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

    factory::factory() : app(Poco::Util::Application::instance())
    , ipfilter(app.config().getInt("http.ipDenyExpire", 3600)*1000, app.config().getInt("http.ipAccessInterval", 30)*1000, app.config().getString("http.ipDenyFile", "/etc/webcppd/ipdeny.conf"))
    , logger(new Poco::FileChannel)
    , classLoader()
    , route()
    , hotlinkRegex(app.config().getString("http.matchHotlinking", "localhost")) {
        this->initialize();
    }

    factory::~factory() {
        this->uninitialize();
    }

    Poco::Net::HTTPRequestHandler* factory::createRequestHandler(const Poco::Net::HTTPServerRequest& request) {
        try {
            if (this->app.config().getBool("http.enableHotlinking", true) && request.has("Referer")) {
                if (!this->hotlinkRegex.match(Poco::URI(request.get("Referer")).getHost())) {
                    return new webcppd::error(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN);
                }
            }
            std::string clientIp = request.clientAddress().host().toString();

            if (this->app.config().getBool("http.proxyUsed", false)) {
                std::string realIp = request.get(this->app.config().getString("http.proxyServerRealIpHeader", "X-Real-IP"));
                if (!realIp.empty()) {
                    clientIp = realIp;
                }
            }
            if (this->ipfilter.kill(clientIp)) {
                this->app.logger().error("IP %[0]s is unwelcome.", clientIp);
                return new webcppd::error(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN);
            }
            if (this->app.config().getBool("http.ipEnableCheck", true)) {
                if (this->ipfilter.deny(clientIp, this->app.config().getInt("http.ipMaxAccessCount", 100))) {
                    this->app.logger().error("IP %[0]s is denied.", clientIp);
                    return new webcppd::error(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN);
                }
            }
            std::string path = Poco::URI(request.getURI()).getPath();
            std::string fullClassName;

            auto route_result = this->get_route(request.getMethod(), path);
            Poco::Net::HTTPRequestHandler* class_handler = 0;
            if (std::get<0>(route_result)) {
                std::string& class_name = std::get<2>(route_result);
                auto finded = this->classLoader.findClass(class_name);
                if (finded && finded->canCreate()) {
                    class_handler = this->classLoader.create(class_name);
                    finded->autoDelete(class_handler);
                }
            }

            if (class_handler) {
                webcppd::root_view* view = dynamic_cast<webcppd::root_view*> (class_handler);
                if (view != nullptr) {
                    view->set_route(std::get<1>(route_result));
                    request.response().set("page-type", "dynamic");
                    return class_handler;
                } else {
                    return new webcppd::error(Poco::Net::HTTPServerResponse::HTTP_NOT_IMPLEMENTED);
                }
            }


            if (request.getMethod() != Poco::Net::HTTPRequest::HTTP_GET) {
                return new webcppd::error(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN);
            }

            return new webcppd::assets();
        } catch (Poco::Exception& e) {
            this->app.logger().error(e.message());
            return new webcppd::error(Poco::Net::HTTPServerResponse::HTTP_EXPECTATION_FAILED);
        }
    }

    void factory::initialize() {
        this->config_log();
        this->config_route();
        this->config_mod();
    }

    void factory::uninitialize() {
        std::vector<std::string> libpath;
        Poco::ClassLoader<Poco::Net::HTTPRequestHandler>::Iterator it = this->classLoader.begin();

        for (; it != this->classLoader.end(); ++it) {
            libpath.push_back(it->first);
        }
        for (auto &item : libpath) {

            this->classLoader.unloadLibrary(item);
            this->app.logger().information(item + " is unloaded.");
        }

        this->app.logger().close();
    }

    void factory::config_log() {
        this->logger->setProperty(Poco::FileChannel::PROP_PATH, this->app.config().getString("http.logDirectory", "/var/webcppd/log") + "/webcppd.log");
        this->logger->setProperty(Poco::FileChannel::PROP_ROTATION, this->app.config().getString("http.logFileSize", "1 M"));
        this->logger->setProperty(Poco::FileChannel::PROP_COMPRESS, this->app.config().getBool("http.logCompress", true) ? "true" : "false");
        this->logger->setProperty(Poco::FileChannel::PROP_TIMES, "local");
        this->logger->setProperty(Poco::FileChannel::PROP_ARCHIVE, "number");
        this->logger->setProperty(Poco::FileChannel::PROP_PURGECOUNT, Poco::NumberFormatter::format(this->app.config().getInt("http.logPurgeCount", 10)));
        Poco::AutoPtr<Poco::AsyncChannel> p(new Poco::AsyncChannel(this->logger));
        this->app.logger().setChannel(p);
        this->app.logger().setLevel("trace");
        this->app.logger().open();
    }

    void factory::config_route() {
        std::ifstream input(this->app.config().getString("http.route", "/etc/webcppd/route.conf"));
        if (input) {
            std::string line;
            while (std::getline(input, line)) {
                if (line.front() != '#' && !line.empty()) {
                    Poco::StringTokenizer st(line, ",;", Poco::StringTokenizer::TOK_TRIM | Poco::StringTokenizer::TOK_IGNORE_EMPTY);
                    if (st.count() == 3) {
                        this->route.push_back(std::make_tuple(st[0], st[1], st[2]));
                    }
                }
            }
        }
    }

    void factory::config_mod() {
        Poco::Path libDir(this->app.config().getString("http.libHandlerDir", "/var/webcppd/mod"));
        Poco::SortedDirectoryIterator it(libDir), jt;
        std::string libExt = Poco::SharedLibrary::suffix().substr(1);
        while (it != jt) {
            if (Poco::Path(it->path()).getExtension() == libExt && it->canExecute()) {
                if (!this->classLoader.isLibraryLoaded(it->path())) {
                    try {
                        this->classLoader.loadLibrary(it->path());
                        this->app.logger().notice("%[0]s is loaded", it->path());
                    } catch (Poco::LibraryLoadException& e) {
                        this->app.logger().notice("%[0]s is not loaded", it->path());
                    }
                }
            }
            ++it;
        }
    }

    std::tuple<bool, std::vector<std::string>, std::string > factory::get_route(const std::string& method, const std::string & path) {
        std::vector<std::string> vec;
        std::string className;
        bool ok = false;
        for (auto &item : this->route) {
            std::string& M = std::get<0>(item);
            std::string& P = std::get<1>(item);
            Poco::RegularExpression regex(P);
            if (method == M && regex.match(path)) {
                regex.split(path, vec);
                className = std::get<2>(item);
                ok = true;
                break;
            }
        }
        return std::make_tuple(ok, vec, className);
    }





}