#include <Poco/LocalDateTime.h>
#include <Poco/String.h>
#include <Poco/File.h>
#include <algorithm>
#include <fstream>
#include "ipfilter.hpp"

namespace webcpp {

    ipfilter::ipfilter(long ipDenyExpire, long ipAccessInterval, const std::string& ipDenyFile) :
    ipDenyExpire(ipDenyExpire)
    , ipAccessInterval(ipAccessInterval)
    , cache(ipAccessInterval)
    , blackIp(ipDenyExpire)
    , denyIp() {
        if (!ipDenyFile.empty() && Poco::File(ipDenyFile).exists()) {
            std::string ip;
            std::ifstream input(ipDenyFile);
            if (input) {
                while (std::getline(input, ip)) {
                    this->denyIp.push_back(ip);
                }
            }
        }
    }

    ipfilter::~ipfilter() {
        this->cache.clear();
        this->blackIp.clear();
        this->denyIp.clear();
    }

    bool ipfilter::deny(const std::string& ip, int ipMaxAccessCount) {
        bool deny = true;
        if (std::find(this->denyIp.begin(), this->denyIp.end(), ip) != this->denyIp.end() || this->blackIp.has(ip)) {
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
