#include "SatToken.h"

#if defined(SECURITY_TOKEN_ENABLED) && ((SECURITY_TOKEN_ENABLED == 0) || (SECURITY_TOKEN_ENABLED == false))
#define GetSecurityToken(a, b) 0
#define GetToken(a, b, c) 0
#else
#include <WPEFramework/securityagent/securityagent.h>
#include <WPEFramework/securityagent/SecurityTokenUtil.h>
#endif

#define MAX_SECURITY_TOKEN_SIZE 1024
#define CALLSIGN_VER ".1"

namespace WPEFramework {
namespace Plugin {
namespace TTS {

using namespace ::TTS;

SatToken* SatToken::getInstance(const string callsign)
{
    static SatToken *instance = new SatToken(callsign);
    return instance;
}

SatToken::SatToken(const string callsign)
{
    m_callsign = callsign;
    m_callsign.append(CALLSIGN_VER);
    string token = getSecurityToken();
    m_authService = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(_T(m_callsign.c_str()),"", false, token);
    getServiceAccessToken();
}

string SatToken::getSecurityToken()
{
    std::string token = "token=";
    int tokenLength = 0;
    unsigned char buffer[MAX_SECURITY_TOKEN_SIZE] = {0};
    static std::string endpoint;

    if(endpoint.empty()) {
        Core::SystemInfo::GetEnvironment(_T("THUNDER_ACCESS"), endpoint);
        TTSLOG_INFO("Thunder RPC Endpoint read from env - %s", endpoint.c_str());
    }

    if(endpoint.empty()) {
        Core::File file("/etc/WPEFramework/config.json");
        if(file.Open(true)) {
            JsonObject config;
            if(config.IElement::FromFile(file)) {
                Core::JSON::String port = config.Get("port");
                Core::JSON::String binding = config.Get("binding");
                if(!binding.Value().empty() && !port.Value().empty())
                    endpoint = binding.Value() + ":" + port.Value();
            }
            file.Close();
        }

        if(endpoint.empty()) {
            endpoint = _T("127.0.0.1:9998");
            TTSLOG_INFO("Thunder RPC Endpoint read from config file - %s", endpoint.c_str());
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), endpoint);
        }
    }

     string payload = "http://localhost";
     if(payload.empty()) {
          tokenLength = GetSecurityToken(sizeof(buffer), buffer);
     } else {
          int buffLength = std::min(sizeof(buffer), payload.length());
          ::memcpy(buffer, payload.c_str(), buffLength);
          tokenLength = GetToken(sizeof(buffer), buffLength, buffer);
     }

     if(tokenLength > 0) {
         token.append((char*)buffer);
     } else {
         token.clear();
     }

     TTSLOG_INFO("Thunder token - %s", token.empty() ? "" : token.c_str());
     return token;
}

void SatToken::getServiceAccessToken()
{
    if(m_authService != nullptr) {
        JsonObject joGetParams, joGetResult;
        auto status = m_authService->Invoke<JsonObject, JsonObject>(1000, "getServiceAccessToken", joGetParams, joGetResult);
        if (status == Core::ERROR_NONE && joGetResult.HasLabel("token")) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_SatToken = joGetResult["token"].String();
        } else {
            TTSLOG_ERROR("Not able to retrieve SAT %s call failed %d",m_callsign.c_str(), status);
        }
    }

    if (!m_eventRegistered) {
        if (m_authService->Subscribe<JsonObject>(1000, "serviceAccessTokenChanged",
                    &SatToken::serviceAccessTokenChangedEventHandler, this) == Core::ERROR_NONE) {
            m_eventRegistered = true;
            TTSLOG_INFO("Subscribed to notification handler : serviceAccessTokenChanged");
        } else {
            TTSLOG_ERROR("Failed to Subscribe notification handler : serviceAccessTokenChanged");
        }
    }
}

void SatToken::serviceAccessTokenChangedEventHandler(const JsonObject& parameters)
{
    TTSLOG_INFO("SAT changed");
    getServiceAccessToken();
}

}
}
}

