#include <iostream>
#include <Poco/ThreadPool.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>

#include "factory.hpp"

#include "server.hpp"

namespace webcpp {

	server::server() : helpRequested(false), httpServer(0)
	{

	}

	server::~server()
	{


	}

	void server::initialize(Poco::Util::Application& self)
	{

		this->loadConfiguration();

		Poco::Util::ServerApplication::initialize(self);
	}

	void server::uninitialize()
	{
		if (this->httpServer) {
			this->httpServer->stopAll(true);
			delete this->httpServer;
		}
		Poco::Util::ServerApplication::uninitialize();
	}

	void server::defineOptions(Poco::Util::OptionSet& options)
	{
		Poco::Util::ServerApplication::defineOptions(options);
		options.addOption(
			Poco::Util::Option("help", "h",
			"display help information on command line arguments")
			.required(false)
			.repeatable(false));
	}

	void server::handleOption(const std::string& name, const std::string& value)
	{
		Poco::Util::ServerApplication::handleOption(name, value);
		if (name == "help") {
			Poco::Util::HelpFormatter hf(this->options());
			hf.setCommand(commandName());
			hf.setUsage("OPTIONS");
			hf.setHeader("A web server, based on POCO C++ library."
				"\nThe configuration file(s) must be located in the same directory."
				"\nAnd must have the same base name as the executable,"
				"\nwith one of the following extensions :.properties, .ini or .xml."
				"\nEmail:admin@webcpp.net"
				"\nSite:http://www.webcpp.net");

			hf.setUnixStyle(true);

			hf.setFooter("");
			hf.format(std::cout);
			this->stopOptionsProcessing();
			this->helpRequested = true;
		}
	}

	int server::main(const std::vector<std::string>& args)
	{
		if (!helpRequested) {
			Poco::Util::LayeredConfiguration &serverConf = this->config();
			Poco::Net::HTTPServerParams* pars = new Poco::Net::HTTPServerParams;
			pars->setMaxQueued(serverConf.getInt("http.maxQueued", 1024));
			pars->setMaxThreads(serverConf.getInt("http.maxThreads", 2048));
			pars->setSoftwareVersion(serverConf.getString("http.softwareVersion", "webcppd/1.0.0"));
			pars->setKeepAlive(serverConf.getBool("http.keepAlive", true));
			pars->setMaxKeepAliveRequests(serverConf.getInt("http.maxKeepAliveRequests", 0));
			pars->setKeepAliveTimeout(Poco::Timespan(serverConf.getInt("http.keepAliveTimeout", 60), 0));
			pars->setTimeout(Poco::Timespan(serverConf.getInt("http.timeout", 60), 0));

			Poco::ThreadPool &pool = Poco::ThreadPool::defaultPool();
			pool.addCapacity(serverConf.getInt("http.maxThreads", 2048));

			Poco::Net::ServerSocket serverSocket;
			Poco::Net::IPAddress ipAddr(serverConf.getString("http.ip", "127.0.0.1"));
			Poco::Net::SocketAddress socketAddr(ipAddr, serverConf.getUInt("http.port", 8888));
			serverSocket.bind(socketAddr, false);
			serverSocket.listen(serverConf.getInt("http.maxQueued", 1024));
			serverSocket.acceptConnection();
			this->httpServer = new Poco::Net::HTTPServer(new webcpp::factory(), pool, serverSocket, pars);
			this->httpServer->start();

			// wait for CTRL-C or kill
			this->waitForTerminationRequest();
		}
		return Poco::Util::Application::EXIT_OK;
	}

}