#include <Poco/Util/Application.h>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/DateTimeParser.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/Format.h>
#include <Poco/RegularExpression.h>
#include <Poco/DirectoryIterator.h>
#include <Poco/SortedDirectoryIterator.h>

#include "mime.hpp"

#include "assets.hpp"

namespace webcppd {

    void assets::do_get(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
        Poco::Util::Application &app = Poco::Util::Application::instance();

        Poco::Path url_path(Poco::URI(request.getURI()).getPath()),
                full_path(app.config().getString("http.docroot", "/var/webcppd/www"));
        full_path.append(url_path);


        Poco::File file(full_path);

        if (file.exists()) {
            if (file.isFile()) {
                Poco::DateTime dt;
                int tz;
                request.response().set("Last-Modified", Poco::DateTimeFormatter::format(file.getLastModified(), Poco::DateTimeFormat::HTTP_FORMAT));
                if (request.has("If-Modified-Since")) {
                    Poco::DateTimeParser::parse(Poco::DateTimeFormat::HTTP_FORMAT, request.get("If-Modified-Since"), dt, tz);
                    if (file.getLastModified() <= dt.timestamp()) {
                        request.response().setContentLength(0);
                        request.response().setStatusAndReason(Poco::Net::HTTPResponse::HTTP_NOT_MODIFIED);
                        request.response().send();
                        return;
                    }
                }

                response.setChunkedTransferEncoding(true);
                response.set("Cache-Control", Poco::format("max-age=%[0]d", app.config().getInt("http.expires", 3600)));
                response.add("Cache-Control", "must-revalidate");
                std::string mimeType = webcppd::mime().getType(full_path.getExtension());
                response.sendFile(full_path.toString(), mimeType);
            } else if (file.isDirectory()) {
                if (!app.config().getBool("http.enableIndex", false)) {
                    response.redirect(url_path.append("/index.html").toString(), Poco::Net::HTTPServerResponse::HTTP_FOUND);
                    return;
                }
                Poco::SortedDirectoryIterator it(full_path), jt;
                response.setContentType("text/html;charset=UTF-8");
                std::ostream& OS = response.send();
                OS << "<html><head>";
                OS << "<title>列出目录</title><style>li{margin:10px 100px;}span{margin:auto 30px;}</style></head><body><ul>";
                while (it != jt) {
                    Poco::Path tmp_path(url_path);
                    tmp_path.append(Poco::Path(it->path()).getFileName());
                    OS << "<li><a href='"
                            << tmp_path.toString()
                            << "'>"
                            << tmp_path.getFileName()
                            << "</a>"
                            << "<span>"
                            << Poco::DateTimeFormatter::format(it->getLastModified(), Poco::DateTimeFormat::HTTP_FORMAT)
                            << "</span>"
                            << "<span>"
                            << it->getSize()
                            << "</span>"
                            << "</li>";
                    ++it;
                }
                OS << "</ul></body></html>";
            } else {
                response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN);
                response.setContentType("text/html;charset=UTF-8");
                response.send() << "<html><head><title>"
                        << static_cast<int> (response.getStatus())
                        << " "
                        << response.getReason()
                        << "</title></head><body bgcolor=\"white\"><center><h1>"
                        << static_cast<int> (response.getStatus())
                        << " "
                        << response.getReason()
                        << "</h1></center><hr/>"
                        << "</body></html>";
            }
        } else {
            response.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND);
            response.setContentType("text/html;charset=UTF-8");
            response.send() << "<html><head><title>"
                    << static_cast<int> (response.getStatus())
                    << " "
                    << response.getReason()
                    << "</title></head><body bgcolor=\"white\"><center><h1>"
                    << static_cast<int> (response.getStatus())
                    << " "
                    << response.getReason()
                    << "</h1></center><hr/>"
                    << "</body></html>";
        }
    }

}
