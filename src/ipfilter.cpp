#include <Poco/LocalDateTime.h>


#include "ipfilter.hpp"

namespace webcpp {

	ipfilter::ipfilter(long ipDenyExpire, long ipAccessInterval) :
	ipDenyExpire(ipDenyExpire)
	, ipAccessInterval(ipAccessInterval)
	, cache(ipAccessInterval)
	, blackIp(ipDenyExpire)
	{

	}

	ipfilter::~ipfilter()
	{
		this->cache.clear();
		this->blackIp.clear();
	}

	bool ipfilter::deny(const std::string& ip, int ipMaxAccessCount)
	{
		bool deny = true;
		if (this->blackIp.has(ip)) {
			return deny;
		}

		if (!this->cache.has(ip)) {
			this->cache.add(ip, std::make_pair(1, Poco::LocalDateTime().timestamp().epochTime()));
			deny = false;
		} else {
			auto old = this->cache.get(ip);
			time_t tdiff = Poco::LocalDateTime().timestamp().epochTime() - old->second;
			if (old->first >= ipMaxAccessCount && tdiff <= this->ipAccessInterval) {
				this->blackIp.add(ip, 0);
			} else {
				this->cache.update(ip, std::make_pair(old->first + 1, old->second));
				deny = false;
			}
		}
		return deny;
	}



}
