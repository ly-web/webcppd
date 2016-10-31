#include <Poco/Util/Application.h>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/DateTimeParser.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/Format.h>
#include <Poco/RegularExpression.h>

#include "mime.hpp"

#include "assets.hpp"

namespace webcpp {

    void assets::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
        Poco::Util::Application &app = Poco::Util::Application::instance();
        std::string clientIp = request.clientAddress().host().toString();
        if (app.config().getBool("http.proxyUsed", false)) {
            std::string realIp = request.get(app.config().getString("http.proxyServerRealIpHeader", "X-Real-IP"));
            if (!realIp.empty()) {
                clientIp = realIp;
            }
        }

        std::string url = Poco::URI(request.getURI()).getPath();
        if (url.size() == 1) {
            url = "/index.html";
        }

        Poco::Path path(app.config().getString("http.docroot", "/var/www/webcppd/www") + url);

        if (path.isFile()) {
            Poco::File file(path);
            if (file.exists()) {
                Poco::DateTime dt;
                int tz;
                request.response().set("Last-Modified", Poco::DateTimeFormatter::format(file.getLastModified(), Poco::DateTimeFormat::HTTP_FORMAT));
                if (request.has("If-Modified-Since")) {
                    Poco::DateTimeParser::parse(Poco::DateTimeFormat::HTTP_FORMAT, request.get("If-Modified-Since"), dt, tz);
                    if (file.getLastModified() <= dt.timestamp()) {
                        request.response().setContentLength(0);
                        request.response().setStatusAndReason(Poco::Net::HTTPResponse::HTTP_NOT_MODIFIED);
                        request.response().send();
                        app.logger().notice("%[0]s %[1]s %[2]s %[3]s %[4]s %[5]d"
                                , Poco::DateTimeFormatter::format(dt, Poco::DateTimeFormat::SORTABLE_FORMAT)
                                , clientIp
                                , request.get("User-Agent")
                                , url
                                , request.getMethod()
                                , static_cast<int> (response.getStatus())
                                );
                        return;
                    }
                }

                response.setChunkedTransferEncoding(true);
                response.set("Cache-Control", Poco::format("max-age=%[0]d", app.config().getInt("http.expires", 3600)));
                response.add("Cache-Control", "must-revalidate");
                std::string mimeType = webcpp::mime().getType(path.getExtension());
                response.sendFile(path.toString(), mimeType);
                app.logger().notice("%[0]s %[1]s %[2]s %[3]s %[4]s %[5]d"
                        , Poco::DateTimeFormatter::format(dt, Poco::DateTimeFormat::SORTABLE_FORMAT)
                        , clientIp
                        , request.get("User-Agent")
                        , url
                        , request.getMethod()
                        , static_cast<int> (response.getStatus())
                        );
            } else {
                response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);

                response.setContentType("text/html;charset=UTF-8");
                response.send() << "<html><head><title>"
                        << static_cast<int> (response.getStatus())
                        << " "
                        << response.getReason()
                        << "</title><body bgcolor=\"white\"><center><h1>"
                        << static_cast<int> (response.getStatus())
                        << " "
                        << response.getReason()
                        << "</h1></center><hr/>"
                        << "</body></html>";
            }
        } else {
            response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN);
            response.setContentType("text/html;charset=UTF-8");
            response.send() << "<html><head><title>"
                    << static_cast<int> (response.getStatus())
                    << " "
                    << response.getReason()
                    << "</title><body bgcolor=\"white\"><center><h1>"
                    << static_cast<int> (response.getStatus())
                    << " "
                    << response.getReason()
                    << "</h1></center><hr/>"
                    << "</body></html>";
        }

    }




}
