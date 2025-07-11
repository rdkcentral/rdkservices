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

#define RETRY_DELAY_MS 2000 // Define the delay between retries in milliseconds

using namespace WPEFramework;

namespace TTS {

RFCURLObserver* RFCURLObserver::getInstance() {
    static RFCURLObserver *instance = new RFCURLObserver();
    return instance;
}


void RFCURLObserver::triggerRFC(TTSConfiguration *config,std::string token)
{
   m_defaultConfig = config;
   m_token = token;
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

void RFCURLObserver::registerNotification() {
    if (m_systemService == nullptr && !m_eventRegistered) {
        m_systemService = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(_T(SYSTEMSERVICE_CALLSIGN_VER),"", false, m_token);
        while (!m_eventRegistered) {		
            if (m_systemService->Subscribe<JsonObject>(3000, "onDeviceMgtUpdateReceived",
                &RFCURLObserver::onDeviceMgtUpdateReceivedHandler, this) == Core::ERROR_NONE) {
                m_eventRegistered = true;
                TTSLOG_INFO("Subscribed to notification handler: onDeviceMgtUpdateReceived");
                break;
            } else {
                TTSLOG_ERROR("Failed to subscribe to notification handler: onDeviceMgtUpdateReceived..Retrying");
                std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_DELAY_MS));
            }
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
