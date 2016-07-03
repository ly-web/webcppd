#ifndef FACTORY_HPP
#define FACTORY_HPP

#include <string>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/ClassLoader.h>
#include "conf.hpp"

namespace webcpp {

    class factory : public Poco::Net::HTTPRequestHandlerFactory {
    public:
        factory(webcpp::conf& conf);
        virtual~factory();
        Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request);
    private:
        const std::string libHandlerDir;
        bool ipEnableCheck;
        const int ipDenyExpire, ipMaxAccessCount, ipAccessInterval;
        Poco::ClassLoader<Poco::Net::HTTPRequestHandlerFactory> classLoader;
    };
}
#endif /* FACTORY_HPP */

