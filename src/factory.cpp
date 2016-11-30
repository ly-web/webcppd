#include <fstream>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/SharedLibrary.h>
#include <Poco/StringTokenizer.h>
#include <Poco/RegularExpression.h>
#include <Poco/Format.h>
#include <Poco/URI.h>
#include <Poco/File.h>
#include <Poco/ClassLoader.h>
#include <Poco/Exception.h>
#include <Poco/Util/Application.h>
#include <Poco/LocalDateTime.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DirectoryIterator.h>
#include <Poco/SortedDirectoryIterator.h>
#include <Poco/AsyncChannel.h>
#include <Poco/NumberFormatter.h>

#include "factory.hpp"
#include "error.hpp"
#include "assets.hpp"

namespace webcppd {

    factory::factory() : serverConf(Poco::Util::Application::instance().config())
    , ipfilter(serverConf.getInt("http.ipDenyExpire", 3600)*1000, serverConf.getInt("http.ipAccessInterval", 30)*1000, serverConf.getString("http.ipDenyFile", "/etc/webcppd/ipdeny.conf"))
    , logger(new Poco::FileChannel)
    , classLoader()
    , route() {
        this->logger->setProperty(Poco::FileChannel::PROP_PATH, this->serverConf.getString("http.logDirectory", "/var/webcppd/log") + "/webcppd.log");
        this->logger->setProperty(Poco::FileChannel::PROP_ROTATION, this->serverConf.getString("http.logFileSize", "1 M"));
        this->logger->setProperty(Poco::FileChannel::PROP_COMPRESS, this->serverConf.getBool("http.logCompress", true) ? "true" : "false");
        this->logger->setProperty(Poco::FileChannel::PROP_TIMES, "local");
        this->logger->setProperty(Poco::FileChannel::PROP_ARCHIVE, "number");
        this->logger->setProperty(Poco::FileChannel::PROP_PURGECOUNT, Poco::NumberFormatter::format(this->serverConf.getInt("http.logPurgeCount", 10)));
        Poco::AutoPtr<Poco::AsyncChannel> p(new Poco::AsyncChannel(this->logger));
        Poco::Util::Application::instance().logger().setChannel(p);
        Poco::Util::Application::instance().logger().setLevel("trace");
        Poco::Util::Application::instance().logger().open();

        std::ifstream input(this->serverConf.getString("http.route", "/etc/webcppd/route.conf"));
        if (input) {
            std::string line;
            while (std::getline(input, line)) {
                if (line.front() != '#' && !line.empty()) {
                    Poco::StringTokenizer st(line, ",;", Poco::StringTokenizer::TOK_TRIM | Poco::StringTokenizer::TOK_IGNORE_EMPTY);
                    std::size_t maxIndex = st.count() - 1;
                    if (maxIndex > 0) {
                        for (std::size_t i = 0; i < maxIndex; ++i) {
                            this->route[st[i]] = st[maxIndex];
                        }
                    }
                }
            }
        }

        Poco::Path libDir(this->serverConf.getString("http.libHandlerDir", "/var/webcppd/mod"));
        Poco::SortedDirectoryIterator it(libDir), jt;
        std::string libExt = Poco::SharedLibrary::suffix().substr(1);
        while (it != jt) {
            if (Poco::Path(it->path()).getExtension() == libExt && it->canExecute()) {
                if (!this->classLoader.isLibraryLoaded(it->path())) {
                    try {
                        this->classLoader.loadLibrary(it->path());
                        Poco::Util::Application::instance().logger().notice("%[0]s is loaded", it->path());
                    } catch (Poco::LibraryLoadException& e) {
                        Poco::Util::Application::instance().logger().notice("%[0]s is not loaded", it->path());
                    }
                }
            }
            ++it;
        }

    }

    factory::~factory() {

        std::vector<std::string> libpath;
        Poco::ClassLoader<Poco::Net::HTTPRequestHandler>::Iterator it = this->classLoader.begin();

        for (; it != this->classLoader.end(); ++it) {
            libpath.push_back(it->first);
        }
        for (auto &item : libpath) {

            this->classLoader.unloadLibrary(item);
            Poco::Util::Application::instance().logger().information(item + " is unloaded.");
        }

        Poco::Util::Application::instance().logger().close();
    }

    Poco::Net::HTTPRequestHandler* factory::createRequestHandler(const Poco::Net::HTTPServerRequest& request) {
        if (this->serverConf.getBool("http.enableHotlinking", true) && request.has("Referer")) {
            Poco::RegularExpression hotlinking(this->serverConf.getString("http.matchHotlinking", ".*"));
            if (!hotlinking.match(Poco::URI(request.get("Referer")).getHost())) {
                return new webcppd::error(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN);
            }
        }
        std::string clientIp = request.clientAddress().host().toString();

        if (this->serverConf.getBool("http.proxyUsed", false)) {
            std::string realIp = request.get(this->serverConf.getString("http.proxyServerRealIpHeader", "X-Real-IP"));
            if (!realIp.empty()) {
                clientIp = realIp;
            }
        }
        if (this->ipfilter.kill(clientIp)) {
            Poco::Util::Application::instance().logger().error("IP %[0]s is unwelcome.", clientIp);
            return new webcppd::error(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN);
        }
        if (this->serverConf.getBool("http.ipEnableCheck", false)) {
            if (this->ipfilter.deny(clientIp, this->serverConf.getInt("http.ipMaxAccessCount", 100))) {
                Poco::Util::Application::instance().logger().error("IP %[0]s is denied.", clientIp);
                return new webcppd::error(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN);
            }
        }
        std::string path = Poco::URI(request.getURI()).getPath();
        std::string fullClassName;

        for (auto &item : this->route) {
            Poco::RegularExpression reg(item.first);
            if (reg.match(path)) {
                fullClassName = item.second;
                break;
            }
        }

        Poco::Net::HTTPRequestHandler* handler = 0;
        auto finded = this->classLoader.findClass(fullClassName);
        if (finded != 0 && finded->canCreate()) {
            handler = this->classLoader.create(fullClassName);
            this->classLoader.classFor(fullClassName).autoDelete(handler);
        }

        if (!handler) {
            return new webcppd::assets();
        }
        Poco::Net::HTTPServerResponse & response = request.response();
        if (!response.has("page-type")) {
            response.set("page-type", "dynamic");
        }
        return handler;
    }

}