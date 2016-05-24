#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/SharedLibrary.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Format.h>
#include <Poco/URI.h>
#include <Poco/File.h>
#include "factory.hpp"
#include "error.hpp"
#include "checkip.hpp"

namespace webcpp {

	factory::factory(webcpp::conf& conf) :
	libHandlerDir(conf.getString("libHandlerDir", "/usr/bin")),
	ipDenyExpire(conf.getInt("ipDenyExpire", 3600)),
	ipMaxAccessCount(conf.getInt("ipMaxAccessCount", 10)),
	ipAccessInterval(conf.getInt("ipAccessInterval", 10))
	{
	}

	Poco::Net::HTTPRequestHandler* factory::createRequestHandler(const Poco::Net::HTTPServerRequest& request)
	{
		if (!webcpp::checkip(request.clientAddress().host().toString(), this->ipDenyExpire, this->ipMaxAccessCount, this->ipAccessInterval)) {
			return new webcpp::error(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN);
		}
		std::string uri = Poco::URI(request.getURI()).getPath();
		Poco::StringTokenizer tokenizer(uri, "/", Poco::StringTokenizer::TOK_TRIM | Poco::StringTokenizer::TOK_IGNORE_EMPTY);
		int n = tokenizer.count();
		std::string libName("libhome"), funcName("indexImpl");
		switch (n) {
		case 0:
			break;
		case 1:
			libName = "lib" + tokenizer[0];
			break;
		default:
			libName = "lib" + tokenizer[0];
			funcName = tokenizer[1] + "Impl";
		}

		std::string libPath(this->libHandlerDir + "/" + libName + Poco::SharedLibrary::suffix());
		if (!Poco::File(libPath).exists()) {
			return new webcpp::error(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND, libName + " is not found.");
		}
		Poco::SharedLibrary libLoader;
		libLoader.load(libPath);
		if (!libLoader.isLoaded()) {
			return new webcpp::error(Poco::Net::HTTPServerResponse::HTTP_SERVICE_UNAVAILABLE, libName + " is not loaded.");
		}


		if (!libLoader.hasSymbol(funcName)) {
			return new webcpp::error(Poco::Net::HTTPServerResponse::HTTP_NOT_IMPLEMENTED, funcName + " is not found.");
		}

		typedef Poco::Net::HTTPRequestHandler * (*IMPL)();
		IMPL impl = (IMPL) libLoader.getSymbol(funcName);
		Poco::Net::HTTPRequestHandler *handler = 0;
		handler = impl();
		if (!handler) {
			handler = new webcpp::error(Poco::Net::HTTPServerResponse::HTTP_NOT_IMPLEMENTED, funcName + " is not running.");
		}
		return handler;
	}

}