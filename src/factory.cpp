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
#include <Poco/FileChannel.h>
#include <Poco/Message.h>
#include <Poco/AutoPtr.h>
#include <Poco/LocalDateTime.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeFormatter.h>



#include "factory.hpp"
#include "error.hpp"
#include "conf.hpp"

namespace webcpp {

	factory::factory() : serverConf(Poco::Util::Application::instance().config())
	, ipfilter(serverConf.getInt("ipDenyExpire", 3600)*1000, serverConf.getInt("ipAccessInterVal", 10)*1000)
	, logger(new Poco::FileChannel(serverConf.getString("logDirectory", "/var/www") + "/webcppd.log"))
	, classLoader()
	{
		this->logger->setProperty(Poco::FileChannel::PROP_ROTATION, this->serverConf.getString("logFileSize", "10M"));
		this->logger->open();

	}

	factory::~factory()
	{
		this->logger->close();
		std::vector<std::string> libpath;
		Poco::ClassLoader<Poco::Net::HTTPRequestHandler>::Iterator it = this->classLoader.begin();

		for (; it != this->classLoader.end(); ++it) {
			libpath.push_back(it->first);
		}
		for (auto &item : libpath) {
			this->classLoader.unloadLibrary(item);
		}


	}

	Poco::Net::HTTPRequestHandler* factory::createRequestHandler(const Poco::Net::HTTPServerRequest& request)
	{
		std::string clientIp = request.clientAddress().host().toString();
		if (this->serverConf.getBool("proxyUsed", false)) {
			std::string realIp = request.get(this->serverConf.getString("proxyServerRealIpHeader", "X-Real-IP"));
			if (!realIp.empty()) {
				clientIp = realIp;
			}
		}
		if (this->serverConf.getBool("ipEnableCheck", false)) {
			//if (!webcpp::checkip(clientIp, this->serverConf.getInt("ipDenyExpire", 3600), this->serverConf.getInt("ipMaxAccessCount", 10), this->serverConf.getInt("ipAccessInterval", 10))) {
			if (this->ipfilter.deny(clientIp, this->serverConf.getInt("ipMaxAccessCount", 10))) {
				return new webcpp::error(Poco::Net::HTTPServerResponse::HTTP_FORBIDDEN);
			}

		}
		std::string uri = Poco::URI(request.getURI()).getPath();
		Poco::Message msg("webcppd.logger", Poco::format("%[3]s %[0]s %[2]s %[4]s %[1]s", clientIp, uri, request.get("User-Agent"), Poco::DateTimeFormatter::format(Poco::LocalDateTime(), Poco::DateTimeFormat::SORTABLE_FORMAT), request.getMethod()), Poco::Message::PRIO_TRACE);
		logger->log(msg);
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

		std::string libPath(this->serverConf.getString("libHandlerDir", "/usr/lib") + "/" + libName + Poco::SharedLibrary::suffix());
		if (!Poco::File(libPath).exists()) {
			return new webcpp::error(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND, libName + " is not found.");
		}
		std::string fullClassName = Poco::format("%[0]s::%[1]s::%[2]s", preName, libName, className);


		if (!this->classLoader.isLibraryLoaded(libPath)) {
			try {
				this->classLoader.loadLibrary(libPath);
			} catch (Poco::LibraryLoadException& e) {
				return new webcpp::error(Poco::Net::HTTPServerResponse::HTTP_BAD_GATEWAY, e.message());
			}
		}

		Poco::Net::HTTPRequestHandler* handler = 0;
		auto finded = this->classLoader.findClass(fullClassName);
		if (finded != 0 && finded->canCreate()) {
			handler = this->classLoader.create(fullClassName);
			this->classLoader.classFor(fullClassName).autoDelete(handler);
		}
		if (handler == 0) {
			return new webcpp::error(Poco::Net::HTTPServerResponse::HTTP_NOT_FOUND, fullClassName + " is not exists.");
		}

		return handler;
	}

}