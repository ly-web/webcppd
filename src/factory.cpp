#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/SharedLibrary.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Format.h>
#include <Poco/URI.h>
#include <Poco/File.h>
#include <Poco/ClassLoader.h>
#include <Poco/Exception.h>
#include <Poco/Util/Application.h>
#include <Poco/FileChannel.h>
#include <Poco/Message.h>
#include <Poco/AutoPtr.h>
#include <Poco/LocalDateTime.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DirectoryIterator.h>
#include <Poco/SortedDirectoryIterator.h>

#include "factory.hpp"
#include "error.hpp"

namespace webcpp {

    factory::factory() : serverConf(Poco::Util::Application::instance().config())
    , ipfilter(serverConf.getInt("http.ipDenyExpire", 3600)*1000, serverConf.getInt("http.ipAccessInterval", 30)*1000, serverConf.getString("http.ipDenyFile", ""))
    , logger(new Poco::FileChannel(serverConf.getString("http.logDirectory", "/var/www") + "/webcppd.log"))
    , classLoader() {
        Poco::Util::Application::instance().logger().setChannel(this->logger);
        Poco::Util::Application::instance().logger().setLevel("trace");
        Poco::Util::Application::instance().logger().open();

        Poco::Path libDir(this->serverConf.getString("http.libHandlerDir", "/usr/lib"));
        Poco::SortedDirectoryIterator it(libDir), jt;
        std::string libExt = Poco::SharedLibrary::suffix().substr(1);
        while (it != jt) {
            if (Poco::Path(it->path()).getExtension() == libExt && it->canExecute()) {
                if (!this->classLoader.isLibraryLoaded(it->path())) {
                    try {
                        this->classLoader.loadLibrary(it->path());
                        Poco::Message msg("webcppd.logger", Poco::format("%[0]s is loaded", it->path()), Poco::Message::PRIO_TRACE);
                        Poco::Util::Application::instance().logger().log(msg);
                    } catch (Poco::LibraryLoadException& e) {

                        Poco::Message msg("webcppd.logger", Poco::format("%[0]s is not loaded", it->path()), Poco::Message::PRIO_TRACE);
                        Poco::Util::Application::instance().logger().log(msg);
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
        std::string clientIp = request.clientAddress().host().toString();

        if (this->serverConf.getBool("http.proxyUsed", false)) {
            std::string realIp = request.get(this->serverConf.getString("http.proxyServerRealIpHeader", "X-Real-IP"));
            if (!realIp.empty()) {
                clientIp = realIp;
            }
        }
        if (this->serverConf.getBool("http.ipEnableCheck", true)) {
            if (this->ipfilter.deny(clientIp, this->serverConf.getInt("http.ipMaxAccessCount", 100))) {
                return new webcpp::error(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN);
            }
        }
        std::string uri = Poco::URI(request.getURI()).getPath();
        Poco::Message msg("webcppd.logger", Poco::format("%[3]s %[0]s %[2]s %[4]s %[1]s", clientIp, uri, request.get("User-Agent"), Poco::DateTimeFormatter::format(Poco::LocalDateTime(), Poco::DateTimeFormat::SORTABLE_FORMAT), request.getMethod()), Poco::Message::PRIO_TRACE);
        Poco::Util::Application::instance().logger().log(msg);

        Poco::StringTokenizer tokenizer(uri, "/", Poco::StringTokenizer::TOK_TRIM | Poco::StringTokenizer::TOK_IGNORE_EMPTY);
        int n = tokenizer.count();
        std::string preName("webcpp"), libName("home"), className("index");
        switch (n) {
            case 0:
                break;
            case 1:
                libName = tokenizer[0];
                className = "index";
                break;
            default:
                libName = tokenizer[0];
                className = tokenizer[1];
        }

        std::string fullClassName = Poco::format("%[0]s::%[1]s::%[2]s", preName, libName, className);

        Poco::Net::HTTPRequestHandler* handler = 0;
        auto finded = this->classLoader.findClass(fullClassName);
        if (finded != 0 && finded->canCreate()) {
            handler = this->classLoader.create(fullClassName);
            this->classLoader.classFor(fullClassName).autoDelete(handler);
        }
        if (handler == 0) {
            return new webcpp::error(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND, fullClassName + " is not exists.");
        }

        return handler;
    }

}