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
		pars->setMaxQueued(serverConf.getInt("maxQueued", 1024));
		pars->setMaxThreads(serverConf.getInt("maxThreads", 2048));
		pars->setSoftwareVersion(serverConf.getString("softwareVersion", "webcppd/1.0.0"));
		pars->setKeepAlive(serverConf.getBool("keepAlive", true));
		pars->setMaxKeepAliveRequests(serverConf.getInt("maxKeepAliveRequests", 0));
		pars->setKeepAliveTimeout(Poco::Timespan(serverConf.getInt("keepAliveTimeout", 60), 0));
		pars->setTimeout(Poco::Timespan(serverConf.getInt("timeout", 60), 0));

		Poco::ThreadPool &pool = Poco::ThreadPool::defaultPool();
		pool.addCapacity(serverConf.getInt("maxThreads", 2048));

		Poco::Net::ServerSocket serverSocket;
		serverSocket.bind(serverConf.getUInt("port", 8888), false);
		serverSocket.listen(serverConf.getInt("maxQueued", 1024));
		serverSocket.acceptConnection();
		this->server = new Poco::Net::HTTPServer(new webcpp::factory(), pool, serverSocket, pars);
		this->server->start();
	}

	void subsystem::uninitialize()
	{
		this->server->stop();
		delete server;
	}




}
