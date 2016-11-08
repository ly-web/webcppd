#ifndef IPFILTER_HPP
#define IPFILTER_HPP


#include <string>
#include <map>
#include <vector>
#include <Poco/ExpireCache.h>


namespace webcppd {

    class ipfilter {
    public:
        ipfilter() = delete;
        ipfilter(long ipDenyExpire, long ipAccessInterval, const std::string& ipDenyFile = "");
        ipfilter(const ipfilter&) = delete;
        virtual ~ipfilter();
    public:
        bool deny(const std::string& ip, int ipMaxAccessCount);
        bool kill(const std::string& ip);
    private:
        time_t ipDenyExpire, ipAccessInterval;
        Poco::ExpireCache<std::string, std::pair<long, time_t> > cache;
        Poco::ExpireCache<std::string, int> blackIp;
        std::vector<std::string> denyIp;
    };
}

#endif /* IPFILTER_HPP */

