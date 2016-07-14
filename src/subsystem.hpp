#ifndef SUBSYSTEM_HPP
#define SUBSYSTEM_HPP

#include <Poco/Util/Subsystem.h>
#include <Poco/Net/HTTPServer.h>

namespace webcpp {

	class subsystem : public Poco::Util::Subsystem {
	public:

		virtual const char *name() const;

		void initialize(Poco::Util::Application& app);

		void uninitialize();

	private:
		Poco::Net::HTTPServer *server;
	};
}
#endif /* SUBSYSTEM_HPP */

