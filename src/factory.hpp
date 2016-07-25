#ifndef FACTORY_HPP
#define FACTORY_HPP

#include <string>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/ClassLoader.h>
#include <Poco/AutoPtr.h>
#include <Poco/FileChannel.h>
#include "conf.hpp"
#include "ipfilter.hpp"

namespace webcpp {

    class factory : public Poco::Net::HTTPRequestHandlerFactory {
    public:
        factory();
        virtual~factory();
        Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request);
    private:
        webcpp::conf serverConf;
        webcpp::ipfilter ipfilter;
        Poco::AutoPtr<Poco::FileChannel> logger;
        Poco::ClassLoader<Poco::Net::HTTPRequestHandlerFactory> classLoader;
    };
}
#endif /* FACTORY_HPP */

