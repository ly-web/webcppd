#ifndef ASSETS_HPP
#define ASSETS_HPP


#include "root_view.hpp"


namespace webcppd {

    class assets : public root_view {
        void do_get(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);

        void do_post(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
            this->error(request, response);
        }

        void do_delete(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
            this->error(request, response);
        }

        void do_put(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
            this->error(request, response);
        }

    };

}

#endif /* ASSETS_HPP */

