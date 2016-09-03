
#include <Poco/Util/Application.h>
#include <Poco/Util/ServerApplication.h>


namespace webcpp {

    class server : public Poco::Util::ServerApplication {
    public:
        server();
        ~server();
    protected:
        void initialize(Poco::Util::Application& self);
        void uninitialize();
        void defineOptions(Poco::Util::OptionSet& options);
        void handleOption(const std::string& name, const std::string& value);
        int main(const std::vector<std::string>& args);
    private:
        bool helpRequested;
    };

}