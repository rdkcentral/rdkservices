#include "TTSCommon.h"

namespace WPEFramework {
namespace Plugin {
namespace TTS {

class NetworkStatusObserver {
public:
    static NetworkStatusObserver* getInstance();
    bool isConnected();

private:
    NetworkStatusObserver(){};
    NetworkStatusObserver(const  NetworkStatusObserver&) = delete;
    NetworkStatusObserver& operator=(const  NetworkStatusObserver&) = delete;

    string getSecurityToken();
    void onConnectionStatusChangedEventHandler(const JsonObject& parameters);

    WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>* m_networkService{nullptr};
    bool m_isConnected {false};
    bool m_eventRegistered {false};
};

}
}
}
