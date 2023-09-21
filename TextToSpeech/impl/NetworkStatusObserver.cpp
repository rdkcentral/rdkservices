#include "NetworkStatusObserver.h"

#if defined(SECURITY_TOKEN_ENABLED) && ((SECURITY_TOKEN_ENABLED == 0) || (SECURITY_TOKEN_ENABLED == false))
#define GetSecurityToken(a, b) 0
#define GetToken(a, b, c) 0
#else
#include <WPEFramework/securityagent/securityagent.h>
#include <WPEFramework/securityagent/SecurityTokenUtil.h>
#endif

#define MAX_SECURITY_TOKEN_SIZE 1024
#define NETWORK_CALLSIGN "org.rdk.Network"
#define NETWORK_CALLSIGN_VER NETWORK_CALLSIGN".1"

namespace WPEFramework {
namespace Plugin {
namespace TTS {

using namespace ::TTS;

NetworkStatusObserver* NetworkStatusObserver::getInstance() {
    static NetworkStatusObserver *instance = new NetworkStatusObserver();
    return instance;
}

string NetworkStatusObserver::getSecurityToken() {
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
        if(endpoint.empty()) 
            endpoint = _T("127.0.0.1:9998");
            
        TTSLOG_INFO("Thunder RPC Endpoint read from config file - %s", endpoint.c_str());
        Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), endpoint);        
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

bool NetworkStatusObserver::isConnected() {
    if (m_networkService == nullptr) {
        string token = getSecurityToken();
        if(token.empty()) {
            m_networkService = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(_T(NETWORK_CALLSIGN_VER),"");
        } else {        
            m_networkService = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(_T(NETWORK_CALLSIGN_VER),"", false, token);
        }

        JsonObject joGetParams, joGetResult;
        auto status = m_networkService->Invoke<JsonObject, JsonObject>(1000, "isConnectedToInternet", joGetParams, joGetResult);
        if (status == Core::ERROR_NONE && joGetResult.HasLabel("connectedToInternet")) {
            m_isConnected = joGetResult["connectedToInternet"].Boolean();
        } else {
            m_isConnected = false;
            TTSLOG_ERROR("%s call failed %d",NETWORK_CALLSIGN_VER, status);
        }
    }

    if (!m_eventRegistered) {
        if (m_networkService->Subscribe<JsonObject>(1000, "onInternetStatusChange",
                    &NetworkStatusObserver::onConnectionStatusChangedEventHandler, this) == Core::ERROR_NONE) {
            m_eventRegistered = true;
            TTSLOG_INFO("Subscribed to notification handler : onInternetStatusChange");
        } else {
            TTSLOG_ERROR("Failed to Subscribe notification handler : onInternetStatusChange");
        }
    }

    TTSLOG_INFO("connectedToInternet status %s",(m_isConnected ? "true":"false"));
    return m_isConnected;
}

void NetworkStatusObserver::onConnectionStatusChangedEventHandler(const JsonObject& parameters) {
    m_isConnected = (parameters.HasLabel("status") && parameters["status"].String() == "FULLY_CONNECTED");
    TTSLOG_INFO("Internet Interface State changed to %s",parameters["status"].String().c_str());
}

}
}
}
