#include "server.hpp"

int main(int argc, char** argv)
{
	webcppd::server server;
	return server.run(argc, argv);
}

