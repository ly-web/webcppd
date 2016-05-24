#include <Poco/Format.h>
#include <Poco/LocalDateTime.h>
#include <vector>
#include <libmemcached/memcached.hpp>

#include "checkip.hpp"

namespace webcpp {

	bool checkip(const std::string& ip, int ipDenyExpire, int ipMaxAccessCount, int ipAccessInterval)
	{
		bool check = false;
		std::string denyip = ip + ".deny", countKey("count"), timeKey("time");
		memcache::Memcache cache("localhost", 11211);
		memcached_st *cacheHandler = const_cast<memcached_st*> (cache.getImpl());
		if (memcached_success(memcached_exist(cacheHandler, denyip.c_str(), denyip.size()))) {
			return check;
		}
		if (!memcached_success(memcached_exist_by_key(cacheHandler, ip.c_str(), ip.size(), countKey.c_str(), countKey.size()))) {
			Poco::LocalDateTime now;
			std::vector<char> countValue, timeValue;
			std::string countStr("1"), timeStr = Poco::format("%Li", now.timestamp().epochTime());
			countValue.assign(countStr.begin(), countStr.end());
			cache.setByKey(ip, countKey, countValue, ipAccessInterval, 0);
			timeValue.assign(timeStr.begin(), timeStr.end());
			cache.setByKey(ip, timeKey, timeValue, ipAccessInterval, 0);
			check = true;
		} else {
			Poco::LocalDateTime now;
			std::vector<char> countValue, timeValue;
			std::string countStr, timeStr;
			if (cache.getByKey(ip, timeKey, timeValue) && cache.getByKey(ip, countKey, countValue)) {
				timeStr.assign(timeValue.begin(), timeValue.end());
				time_t tdiff = now.timestamp().epochTime() - atol(timeStr.c_str());
				countStr.assign(countValue.begin(), countValue.end());
				int oldCount = atoi(countStr.c_str());
				if (tdiff <= ipAccessInterval && oldCount >= ipMaxAccessCount) {
					cache.set(denyip, ip.c_str(), ip.size(), ipDenyExpire, 0);
				} else {
					uint64_t newCount;
					if (memcached_success(memcached_increment_by_key(cacheHandler, ip.c_str(), ip.size(), countKey.c_str(), countKey.size(), 1, &newCount))) {
						check = true;
					}
				}
			}
		}
		return check;
	}
}