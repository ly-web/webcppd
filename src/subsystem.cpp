#include <Poco/Util/Application.h>
#include <Poco/ThreadPool.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Timespan.h>
#include <iostream>

#include "subsystem.hpp"
#include "conf.hpp"
#include "factory.hpp"

namespace webcpp {

	const char* subsystem::name() const
	{
		return "http::WebcppdServer";
	}

	void subsystem::initialize(Poco::Util::Application& app)
	{
		webcpp::conf serverConf(app.config());
		Poco::Net::HTTPServerParams* pars = new Poco::Net::HTTPServerParams;
		pars->setMaxQueued(serverConf.getInt("maxQueued", 100));
		pars->setMaxThreads(serverConf.getInt("maxThreads", 50));
		pars->setSoftwareVersion(serverConf.getString("softwareVersion", "webcppd/1.0.0"));
		pars->setKeepAlive(serverConf.getBool("keepAlive", true));
		pars->setMaxKeepAliveRequests(serverConf.getInt("maxKeepAliveRequests", 0));
		pars->setKeepAliveTimeout(Poco::Timespan(serverConf.getInt("keepAliveTimeout", 75), 0));
		pars->setTimeout(Poco::Timespan(serverConf.getInt("timeout", 60), 0));
		this->server = new Poco::Net::HTTPServer(new webcpp::factory(), serverConf.getUInt("port", 8888), pars);
		this->server->start();

	}

	void subsystem::uninitialize()
	{
		this->server->stop();
		delete server;
	}




}
