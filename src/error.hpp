#ifndef ERROR_HPP
#define ERROR_HPP

#include "root_view.hpp"

namespace webcppd {

    class error : public root_view {
        void do_get(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);

        void do_post(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
            root_view::error(request, response);
        }

        void do_delete(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
            root_view::error(request, response);
        }

        void do_put(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
            root_view::error(request, response);
        }

    public:
        error(Poco::Net::HTTPServerResponse::HTTPStatus status, const std::string& message = "");


    private:
        const Poco::Net::HTTPServerResponse::HTTPStatus resStatus;
        std::string message;
    };

}

#endif /* ERROR_HPP */

