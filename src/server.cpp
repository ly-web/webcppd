#include <iostream>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/ThreadPool.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/SecureServerSocket.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/SharedPtr.h>
#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/Exception.h>

#include "factory.hpp"

#include "server.hpp"

namespace webcppd {

    server::server() : helpRequested(false) {
        if (this->config().getBool("http.enableSSL", true)) {
            Poco::Net::initializeSSL();
        }
    }

    server::~server() {
        if (this->config().getBool("http.enableSSL", true)) {
            Poco::Net::uninitializeSSL();
        }
    }

    void server::initialize(Poco::Util::Application& self) {
        Poco::Util::ServerApplication::initialize(self);
    }

    void server::uninitialize() {
        Poco::Util::ServerApplication::uninitialize();
    }

    void server::displayHelp() {
        Poco::Util::HelpFormatter helpFormatter(options());
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("OPTIONS");
        helpFormatter.setHeader(
                "A web server, based on POCO C++ library."
                "\nThe configuration file(s) must be located in the same directory."
                "\nAnd must have the same base name as the executable,"
                "\nwith one of the following extensions :.properties, .ini or .xml."
                );
        helpFormatter.setFooter(
                "\nEmail:admin@webcpp.net"
                "\nSite:http://www.webcpp.net"
                );
        helpFormatter.format(std::cout);
    }

    void server::defineOptions(Poco::Util::OptionSet& options) {
        Poco::Util::ServerApplication::defineOptions(options);
        options.addOption(
                Poco::Util::Option("help", "h", "display help information on command line arguments")
                .required(false)
                .repeatable(false)
                .callback(Poco::Util::OptionCallback<webcppd::server>(this, &webcppd::server::handleHelp)));

        options.addOption(
                Poco::Util::Option("config", "c", "load configuration data from a file")
                .required(true)
                .repeatable(true)
                .argument("file")
                .callback(Poco::Util::OptionCallback<webcppd::server>(this, &webcppd::server::handleConfig)));

    }

    void server::handleHelp(const std::string& name, const std::string& value) {
        this->helpRequested = true;
        this->displayHelp();
        this->stopOptionsProcessing();
    }

    void server::handleConfig(const std::string& name, const std::string& value) {
        this->loadConfiguration(value);
    }

    int server::main(const std::vector<std::string>& args) {
        if (!helpRequested) {
            Poco::Util::LayeredConfiguration &serverConf = this->config();
            Poco::Net::HTTPServerParams* pars = new Poco::Net::HTTPServerParams;
            pars->setMaxQueued(serverConf.getInt("http.maxQueued", 1000));
            pars->setMaxThreads(serverConf.getInt("http.maxThreads", 1023));
            pars->setSoftwareVersion(serverConf.getString("http.softwareVersion", "webcppd/1.0.3"));
            pars->setKeepAlive(serverConf.getBool("http.keepAlive", true));
            pars->setMaxKeepAliveRequests(serverConf.getInt("http.maxKeepAliveRequests", 0));
            pars->setKeepAliveTimeout(Poco::Timespan(serverConf.getInt("http.keepAliveTimeout", 60), 0));
            pars->setTimeout(Poco::Timespan(serverConf.getInt("http.timeout", 60), 0));

            Poco::ThreadPool &pool = Poco::ThreadPool::defaultPool();
            pool.addCapacity(serverConf.getInt("http.maxThreads", 1023));


            Poco::Net::IPAddress ipAddr(serverConf.getString("http.ip", "127.0.0.1"));
            Poco::Net::SocketAddress socketAddr(ipAddr, serverConf.getUInt("http.port", serverConf.getBool("http.enableSSL", true) ? 443 : 80));


            Poco::SharedPtr<Poco::Net::ServerSocket> serverSocket;

            if (serverConf.getBool("http.enableSSL", true)) {
                try {
                    Poco::Net::Context::Ptr cptr(new Poco::Net::Context(
                            Poco::Net::Context::SERVER_USE
                            , serverConf.getString("http.certPrivateKeyFile", "/var/www/webcppd/cert/server.key")
                            , serverConf.getString("http.certCertificateFile", "/var/www/webcppd/cert/server.crt")
                            , serverConf.getString("http.certRootCertificateFile", "/var/www/webcppd/cert/rootCA.pem")
                            , serverConf.getBool("http.certCheckClient", false) ? Poco::Net::Context::VERIFY_RELAXED : Poco::Net::Context::VERIFY_NONE
                            , 9
                            , false));
                    serverSocket.assign(new Poco::Net::SecureServerSocket(cptr));
                } catch (Poco::Exception& e) {
                    std::cout << e.message() << std::endl;
                }

            } else {
                serverSocket.assign(new Poco::Net::ServerSocket);
            }
            serverSocket->bind(socketAddr, false);
            serverSocket->listen(serverConf.getInt("http.maxQueued", 1000));
            serverSocket->acceptConnection();

            webcppd::factory * factory = new webcppd::factory();

            Poco::Net::HTTPServer httpServer(factory, pool, *serverSocket, pars);

            httpServer.start();

            Poco::Util::Application::instance().logger().information("server start.");
            // wait for CTRL-C or kill
            this->waitForTerminationRequest();

            Poco::Util::Application::instance().logger().information("server stop.");

            httpServer.stop();
        }
        return Poco::Util::Application::EXIT_OK;
    }

}