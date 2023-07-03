#ifndef _TTS_SATTOKEN_H_
#define _TTS_SATTOKEN_H_
#include "TTSCommon.h"
#include <mutex>

namespace WPEFramework {
namespace Plugin {
namespace TTS {

class SatToken {
public:
    static SatToken* getInstance(const string callsign);
    string getSAT() { 
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_SatToken; 
    };

private:
    SatToken(){};
    SatToken(const string callsign);
    SatToken(const  SatToken&) = delete;
    SatToken& operator=(const  SatToken&) = delete;

    string getSecurityToken();
    void serviceAccessTokenChangedEventHandler (const JsonObject& parameters);
    void getServiceAccessToken();

    WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>* m_authService{nullptr};
    string m_SatToken;
    string m_callsign;
    std::mutex m_mutex;
    bool m_eventRegistered{false};
};
}
}
}
#endif
