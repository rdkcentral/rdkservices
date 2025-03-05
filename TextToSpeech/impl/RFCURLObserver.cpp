#include <thread>
#include "RFCURLObserver.h"
#include "UtilsgetRFCConfig.h"
#include "UtilsLogging.h"

#if defined(SECURITY_TOKEN_ENABLED) && ((SECURITY_TOKEN_ENABLED == 0) || (SECURITY_TOKEN_ENABLED == false))
#define GetSecurityToken(a, b) 0
#define GetToken(a, b, c) 0
#else
#include <WPEFramework/securityagent/securityagent.h>
#include <WPEFramework/securityagent/SecurityTokenUtil.h>
#endif

#define MAX_SECURITY_TOKEN_SIZE 1024
#define SYSTEMSERVICE_CALLSIGN "org.rdk.System"
#define SYSTEMSERVICE_CALLSIGN_VER SYSTEMSERVICE_CALLSIGN".1"

#define MAX_RETRIES 3 // Define the maximum number of retries
#define RETRY_DELAY_MS 2000 // Define the delay between retries in milliseconds

using namespace WPEFramework;

namespace TTS {

RFCURLObserver* RFCURLObserver::getInstance() {
    static RFCURLObserver *instance = new RFCURLObserver();
    return instance;
}


void RFCURLObserver::triggerRFC(TTSConfiguration *config)
{
   m_defaultConfig = config;
   fetchURLFromConfig();
   std::thread notificationThread(&RFCURLObserver::registerNotification, this);
   notificationThread.detach(); // Detach the thread to run independently
}

void RFCURLObserver::fetchURLFromConfig() {
    bool  m_rfcURLSet = false;
    RFC_ParamData_t param;
    #ifndef UNIT_TESTING
    m_rfcURLSet = Utils::getRFCConfig("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TextToSpeech.URL", param);
    #endif
    if (m_rfcURLSet) {
	TTSLOG_INFO("Received RFC URL %s\n",param.value);
        m_defaultConfig->setRFCEndPoint(param.value);
    } else {
        TTSLOG_ERROR("Error: Reading URL RFC failed.");
    }
}

string RFCURLObserver::getSecurityToken() {
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


void RFCURLObserver::registerNotification() {
    if (m_systemService == nullptr && !m_eventRegistered) {
        std::string token = getSecurityToken();
        if(token.empty()) {
            m_systemService = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(_T(SYSTEMSERVICE_CALLSIGN_VER),"");
        } else {        
            m_systemService = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(_T(SYSTEMSERVICE_CALLSIGN_VER),"", false, token);
        }

	int retries = 0;
        while (retries < MAX_RETRIES) {		
            if (m_systemService->Subscribe<JsonObject>(3000, "onDeviceMgtUpdateReceived",
                &RFCURLObserver::onDeviceMgtUpdateReceivedHandler, this) == Core::ERROR_NONE) {
                m_eventRegistered = true;
                TTSLOG_INFO("Subscribed to notification handler: onDeviceMgtUpdateReceived");
                break;
            } else {
                TTSLOG_ERROR("Failed to subscribe to notification handler: onDeviceMgtUpdateReceived..Retrying");
                std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_DELAY_MS));
                retries++;
            }
        }

       if (!m_eventRegistered) {
            TTSLOG_ERROR("Failed to subscribe to notification handler after maximum retries.");
       }
    }
}

void RFCURLObserver::onDeviceMgtUpdateReceivedHandler(const JsonObject& parameters) {
    if(parameters["source"].String() == "rfc")
    {
       fetchURLFromConfig();
       TTSLOG_INFO("onDeviceMgtUpdateReceived notification received");
    }
}

RFCURLObserver::~RFCURLObserver() {
   // Clean up resources
    if (m_systemService) {
        delete m_systemService;
        m_systemService = nullptr;
    }
}

}
