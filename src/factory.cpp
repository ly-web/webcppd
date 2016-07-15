#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/SharedLibrary.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Format.h>
#include <Poco/URI.h>
#include <Poco/File.h>
#include <Poco/ClassLoader.h>
#include <Poco/Exception.h>
#include <Poco/Util/Application.h>

#include "factory.hpp"
#include "error.hpp"
#include "checkip.hpp"
#include "conf.hpp"

namespace webcpp {

	factory::factory() : classLoader()
	{
	}

	factory::~factory()
	{
		std::vector<std::string> libpath;
		Poco::ClassLoader<Poco::Net::HTTPRequestHandlerFactory>::Iterator it = this->classLoader.begin();

		for (; it != this->classLoader.end(); ++it) {
			libpath.push_back(it->first);
		}
		for (auto &item : libpath) {
			this->classLoader.unloadLibrary(item);
		}

	}

	Poco::Net::HTTPRequestHandler* factory::createRequestHandler(const Poco::Net::HTTPServerRequest& request)
	{
		webcpp::conf serverConf(Poco::Util::Application::instance().config());

		if (serverConf.getBool("ipEnableCheck", false) && !webcpp::checkip(request.clientAddress().host().toString(), serverConf.getInt("ipDenyExpire", 3600), serverConf.getInt("ipMaxAccessCount", 10), serverConf.getInt("ipAccessInterval", 10))) {
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

		std::string libPath(serverConf.getString("libHandlerDir", "/usr/lib") + "/" + libName + Poco::SharedLibrary::suffix());
		if (!Poco::File(libPath).exists()) {
			return new webcpp::error(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND, libName + " is not found.");
		}
		std::string fullClassName = Poco::format("%[0]s::%[1]s::%[2]sFactory", preName, libName, className);


		if (!this->classLoader.isLibraryLoaded(libPath)) {
			try {
				this->classLoader.loadLibrary(libPath);
			} catch (Poco::LibraryLoadException& e) {
				return new webcpp::error(Poco::Net::HTTPServerResponse::HTTP_BAD_GATEWAY, e.message());
			}
		}

		Poco::Net::HTTPRequestHandlerFactory* handler = 0;
		auto finded = this->classLoader.findClass(fullClassName);
		if (finded != 0 && finded->canCreate()) {
			handler = this->classLoader.create(fullClassName);
			this->classLoader.classFor(fullClassName).autoDelete(handler);
		}
		if (handler == 0) {
			return new webcpp::error(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND, fullClassName + " is not exists.");
		}

		return handler->createRequestHandler(request);
	}

}