#include <thread>
#include "RFCURLObserver.h"
#include "UtilsgetRFCConfig.h"
#include "UtilsLogging.h"
#include "UtilsController.h"

#define MAX_SECURITY_TOKEN_SIZE 1024
#define SYSTEMSERVICE_CALLSIGN "org.rdk.System"
#define SYSTEMSERVICE_CALLSIGN_VER SYSTEMSERVICE_CALLSIGN".1"

#define MAX_RETRIES 3 // Define the maximum number of retries
#define RETRY_DELAY_MS 2000 // Define the delay between retries in milliseconds

namespace TTS {

RFCURLObserver* RFCURLObserver::getInstance() {
    static RFCURLObserver *instance = new RFCURLObserver();
    return instance;
}


void RFCURLObserver::triggerRFC(TTSConfiguration *config)
{
   m_defaultConfig = config;
   fetchURLFromConfig();
   //registerNotification();

   std::thread notificationThread(&RFCURLObserver::registerNotification, this);
   notificationThread.detach(); // Detach the thread to run independently
}

void RFCURLObserver::fetchURLFromConfig() {
    bool  m_rfcURLSet;
    RFC_ParamData_t param;
    m_rfcURLSet = Utils::getRFCConfig("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TextToSpeech.URL", param);
    
    if (m_rfcURLSet) {
	TTSLOG_INFO("Received RFC URL %s\n",param.value);
        m_defaultConfig->setRFCEndPoint(param.value);
    } else {
        TTSLOG_ERROR("Error: Reading URL RFC failed.");
    }
}

void RFCURLObserver::registerNotification() {
    if (m_systemService == nullptr && !m_eventRegistered) {
        std::string token;
        Utils::SecurityToken::getSecurityToken(token);
        /* 
        //Method1
        if(token.empty()) {
            m_systemService = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(_T(SYSTEMSERVICE_CALLSIGN_VER),"");
        } else {        
            m_systemService = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(_T(SYSTEMSERVICE_CALLSIGN_VER),"", false, token);
        }
        */
        //Method2
        string query = "token=" + token;
        Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), _T(SERVER_DETAILS));
        m_systemService = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(_T(SYSTEMSERVICE_CALLSIGN_VER),"", false, query);

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
