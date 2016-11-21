#ifndef ROOT_VIEW_HPP
#define ROOT_VIEW_HPP

#include <string>
#include <map>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Util/Application.h>
#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeParser.h>
#include <Poco/URI.h>
#include <Poco/SingletonHolder.h>
#include <Poco/ExpireLRUCache.h>
#include <Poco/Net/NameValueCollection.h>
#include <Poco/Net/HTTPCookie.h>
#include <Poco/MD5Engine.h>
#include <Poco/NumberParser.h>
#include <Poco/File.h>
#include <Poco/ExpirationDecorator.h>
#include <Poco/UniqueExpireCache.h>
#include <Poco/UUIDGenerator.h>
#include <Poco/DynamicAny.h>






namespace webcppd {

    class root_view : public Poco::Net::HTTPRequestHandler {
    public:

        typedef Poco::ExpireLRUCache<std::string, std::map<std::string, Poco::DynamicAny>> root_session_t;
        typedef Poco::ExpireLRUCache<std::string, std::string> root_cache_t;

        void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
            if (!this->check_modified(request, response)) {
                this->log(request, response);
                return;
            }
            this->session_create(request, response);
            if (request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
                this->do_get(request, response);
            } else if (request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
                this->do_post(request, response);
            } else if (request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_PUT) {
                this->do_put(request, response);
            } else if (request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
                this->do_delete(request, response);
            } else {
                this->error(request, response);
            }
            this->log(request, response);
        }




    private:

        bool check_modified(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
            const std::string last_modified("Last-Modified"), if_modified_since("If-Modified-Since"), max_age("max-age");
            const int expire = Poco::Util::Application::instance().config().getInt("http.expires", 3600);
            Poco::DateTime dt;
            if (!request.has(if_modified_since)) {
                request.response().set(last_modified, Poco::DateTimeFormatter::format(dt, Poco::DateTimeFormat::HTTP_FORMAT));
            } else {
                Poco::DateTime modifiedSince;
                int tzd;
                Poco::DateTimeParser::parse(request.get("If-Modified-Since"), modifiedSince, tzd);
                if ((dt.timestamp().epochTime() - modifiedSince.timestamp().epochTime()) <= expire) {
                    request.response().setContentLength(0);
                    request.response().setStatusAndReason(Poco::Net::HTTPResponse::HTTP_NOT_MODIFIED);
                    request.response().send();
                    return false;

                }
            }
            response.add("Cache-Control", Poco::format("max-age=%[0]d", expire));
            response.add("Cache-Control", "must-revalidate");
            return true;
        }

        void log(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
            Poco::Util::Application &app = Poco::Util::Application::instance();
            std::string clientIp = request.clientAddress().host().toString();
            std::string url = Poco::URI(request.getURI()).getPathAndQuery();
            if (app.config().getBool("http.proxyUsed", false)) {
                std::string realIp = request.get(app.config().getString("http.proxyServerRealIpHeader", "X-Real-IP"));
                if (!realIp.empty()) {
                    clientIp = realIp;
                }
            }
            app.logger().notice("%[0]s %[1]s %[2]s %[3]s %[4]s %[5]d"
                    , Poco::DateTimeFormatter::format(Poco::DateTime(), Poco::DateTimeFormat::SORTABLE_FORMAT)
                    , clientIp
                    , request.get("User-Agent")
                    , url
                    , request.getMethod()
                    , static_cast<int> (response.getStatus())
                    );
        }

        std::string session_create(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
            const std::string session_id_key("WEBCPPDSESSIONID");
            const int expire(600);
            Poco::Net::NameValueCollection cookies;
            request.getCookies(cookies);
            if (cookies.has(session_id_key) && root_view::root_session().has(cookies.get(session_id_key))) {
                return cookies.get(session_id_key);
            }
            Poco::Net::HTTPCookie cookie;
            cookie.setName(session_id_key);
            cookie.setValue(Poco::UUIDGenerator::defaultGenerator().createRandom().toString());
            cookie.setMaxAge(expire);
            cookie.setPath("/");
            cookie.setHttpOnly(true);
            if (Poco::Util::Application::instance().config().getBool("http.enableSSL", true)) {
                cookie.setSecure(true);
            }
            response.addCookie(cookie);
            root_view::root_session().add(cookie.getValue(), std::map<std::string, Poco::DynamicAny>());
            return cookie.getValue();
        }

    private:

        class mysql_connection_config {
        public:

            mysql_connection_config() : nvc() {
                Poco::Util::Application& app = Poco::Util::Application::instance();
                this->nvc["host"] = app.config().getString("mysql.host", "localhost");
                this->nvc["port"] = app.config().getString("mysql.port", "3306");
                this->nvc["user"] = app.config().getString("mysql.user", "root");
                this->nvc["password"] = app.config().getString("mysql.password", "123456");
                this->nvc["db"] = app.config().getString("mysql.db", "test");
                this->nvc["default-character-set"] = app.config().getString("mysql.default-character", "utf8");
                this->nvc["compress"] = app.config().getString("mysql.compress", "true");
                this->nvc["auto-reconnect"] = app.config().getString("mysql.auto-reconnect", "true");
            }
            virtual ~mysql_connection_config() = default;

            std::string get() {
                std::string connection_string;
                for (auto& item : this->nvc) {
                    connection_string += (item.first + "=" + item.second + ";");
                }
                return connection_string;
            }
        private:
            std::map<std::string, std::string> nvc;

        };
    protected:

        void session_set(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response, const std::string& key, const Poco::DynamicAny& value) {
            root_view::root_session().get(this->session_create(request, response))->operator[](key) = value;
        }

        bool session_has(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response, const std::string& key) {
            auto tmp = root_view::root_session().get(this->session_create(request, response));
            auto item = tmp->find(key);
            return item != tmp->end();
        }

        Poco::DynamicAny session_get(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response, const std::string& key) {
            return root_view::root_session().get(this->session_create(request, response))->operator[](key);
        }

        void session_remove(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response, const std::string& key) {
            root_view::root_session().get(this->session_create(request, response))->erase(key);
        }

        void session_clear(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
            root_view::root_session().get(this->session_create(request, response))->clear();
        }

        void session_expired(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
            std::string old_session_id = this->session_create(request, response);
            root_view::root_session().remove(old_session_id);
            const std::string session_id_key("WEBCPPDSESSIONID");
            const int expire(600);
            Poco::Net::HTTPCookie cookie;
            cookie.setName(session_id_key);
            cookie.setValue(old_session_id);
            cookie.setMaxAge(expire);
            cookie.setPath("/");
            cookie.setHttpOnly(true);
            if (Poco::Util::Application::instance().config().getBool("http.enableSSL", true)) {
                cookie.setSecure(true);
            }
            response.addCookie(cookie);
            root_view::root_session().add(old_session_id, std::map<std::string, Poco::DynamicAny>());
        }

        void expired(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
            request.response().set("Last-Modified", Poco::DateTimeFormatter::format(Poco::DateTime(1970, 1, 1), Poco::DateTimeFormat::HTTP_FORMAT));
        }
    protected:

        virtual void do_get(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) = 0;

        virtual void do_post(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) = 0;

        virtual void do_put(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) = 0;

        virtual void do_delete(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) = 0;

        virtual void error(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
            request.response().setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN);
            request.response().send();
        }
    protected:

        static root_cache_t& root_cache() {
            static Poco::SingletonHolder<root_cache_t> cache;
            return *cache.get();
        }

        static root_session_t& root_session() {
            static Poco::SingletonHolder<root_session_t> session;
            return *session.get();
        }

        static std::string mysql_connection_string() {
            static Poco::SingletonHolder<mysql_connection_config> mysql_config;
            return mysql_config.get()->get();
        }



    };


}

#endif /* ROOT_VIEW_HPP */

