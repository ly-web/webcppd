#ifndef MIME_HPP
#define MIME_HPP

#include <string>
#include <map>

namespace webcpp {

    class mime : protected std::map<std::string, std::string> {
    public:
        mime();
        std::string getType(const std::string &ext);
        virtual ~mime() = default;
    private:
        std::map<std::string, std::string> data;
        static const char *src[][2];
    };
}

#endif /* MIME_HPP */

