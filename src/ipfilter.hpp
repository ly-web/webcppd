#ifndef IPFILTER_HPP
#define IPFILTER_HPP


#include <string>
#include <map>
#include <Poco/ExpireCache.h>


namespace webcpp {

    class ipfilter {
    public:
        ipfilter() = delete;
        ipfilter(long ipDenyExpire, long ipAccessInterval);
        ipfilter(const ipfilter&) = delete;
        virtual ~ipfilter();
    public:
        bool deny(const std::string& ip, int ipMaxAccessCount);

    private:
        time_t ipDenyExpire, ipAccessInterval;
        Poco::ExpireCache<std::string, std::pair<long, time_t> > cache;
        Poco::ExpireCache<std::string, int> blackIp;
    };
}

#endif /* IPFILTER_HPP */

