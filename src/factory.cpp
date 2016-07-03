#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/SharedLibrary.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Format.h>
#include <Poco/URI.h>
#include <Poco/File.h>
#include <Poco/ClassLoader.h>
#include <Poco/Exception.h>

#include "factory.hpp"
#include "error.hpp"
#include "checkip.hpp"

namespace webcpp {

	factory::factory(webcpp::conf& conf) :
	libHandlerDir(conf.getString("libHandlerDir", "/usr/bin")),
	ipDenyExpire(conf.getInt("ipDenyExpire", 3600)),
	ipMaxAccessCount(conf.getInt("ipMaxAccessCount", 10)),
	ipAccessInterval(conf.getInt("ipAccessInterval", 10)),
	ipEnableCheck(conf.getBool("ipEnableCheck", true))
	{
	}

	Poco::Net::HTTPRequestHandler* factory::createRequestHandler(const Poco::Net::HTTPServerRequest& request)
	{
		if (this->ipEnableCheck&&!webcpp::checkip(request.clientAddress().host().toString(), this->ipDenyExpire, this->ipMaxAccessCount, this->ipAccessInterval)) {
			return new webcpp::error(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN);
		}
		std::string uri = Poco::URI(request.getURI()).getPath();
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

		std::string libPath(this->libHandlerDir + "/" + libName + Poco::SharedLibrary::suffix());
		if (!Poco::File(libPath).exists()) {
			return new webcpp::error(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND, libName + " is not found.");
		}
		std::string fullClassName = Poco::format("%[0]s::%[1]s::%[2]sFactory", preName, libName, className);
		Poco::ClassLoader<Poco::Net::HTTPRequestHandlerFactory> classLoader;

		if (!classLoader.isLibraryLoaded(libPath)) {
			try {
				classLoader.loadLibrary(libPath);
			} catch (Poco::LibraryLoadException& e) {
				return new webcpp::error(Poco::Net::HTTPServerResponse::HTTP_BAD_GATEWAY, e.message());

			}
		}

		Poco::Net::HTTPRequestHandlerFactory* handler = 0;
		auto finded = classLoader.findClass(fullClassName);
		if (finded != 0 && finded->canCreate()) {
			handler = classLoader.create(fullClassName);
			classLoader.classFor(fullClassName).autoDelete(handler);
		}
		if (handler == 0) {
			return new webcpp::error(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND, fullClassName + " is not exists.");
		}

		return handler->createRequestHandler(request);
	}

}