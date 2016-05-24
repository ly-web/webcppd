#ifndef CHECKIP_HPP
#define CHECKIP_HPP

#include <string>

namespace webcpp {
    bool checkip(const std::string& ip, int ipDenyExpire, int ipMaxAccessCount, int ipAccessInterval);
}

#endif /* CHECKIP_HPP */

