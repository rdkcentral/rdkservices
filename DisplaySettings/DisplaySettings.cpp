/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

//I have put several "TODO(MROLLINS)" in the code below to mark areas of concern I encountered
//  when refactoring the servicemanager's version of displaysettings into this new thunder plugin format

#include "DisplaySettings.h"
#include <algorithm>
#include "dsMgr.h"
#include "host.hpp"
#include "exception.hpp"
#include "videoOutputPort.hpp"
#include "videoOutputPortType.hpp"
#include "videoOutputPortConfig.hpp"
#include "videoResolution.hpp"
#include "audioOutputPort.hpp"
#include "audioOutputPortType.hpp"
#include "audioOutputPortConfig.hpp"
#include "manager.hpp"
#include "dsUtl.h"
#include "dsError.h"
#include "list.hpp"
#include "dsDisplay.h"
#include "rdk/iarmmgrs-hal/pwrMgr.h"
#include "pwrMgr.h"

#include "tr181api.h"

#include "tracing/Logging.h"
#include <syscall.h>

#include "UtilsCStr.h"
#include "UtilsIarm.h"
#include "UtilsJsonRpc.h"
#include "UtilsString.h"
#include "UtilsisValidInt.h"
#include "dsRpc.h"

using namespace std;

#define HDMI_HOT_PLUG_EVENT_CONNECTED 0


#define HDMICECSINK_CALLSIGN "org.rdk.HdmiCecSink"
#define HDMICECSINK_CALLSIGN_VER HDMICECSINK_CALLSIGN".1"
#define HDMICECSINK_ARC_INITIATION_EVENT "arcInitiationEvent"
#define HDMICECSINK_ARC_TERMINATION_EVENT "arcTerminationEvent"
#define HDMICECSINK_ARC_AUDIO_STATUS_EVENT "reportAudioStatusEvent"
#define HDMICECSINK_SHORT_AUDIO_DESCRIPTOR_EVENT "shortAudiodesciptorEvent"
#define HDMICECSINK_SYSTEM_AUDIO_MODE_EVENT "setSystemAudioModeEvent"
#define HDMICECSINK_AUDIO_DEVICE_CONNECTED_STATUS_EVENT "reportAudioDeviceConnectedStatus"
#define HDMICECSINK_CEC_ENABLED_EVENT "reportCecEnabledEvent"
#define HDMICECSINK_AUDIO_DEVICE_POWER_STATUS_EVENT "reportAudioDevicePowerStatus"
#define SERVER_DETAILS  "127.0.0.1:9998"
#define WARMING_UP_TIME_IN_SECONDS 5
#define HDMICECSINK_PLUGIN_ACTIVATION_TIME 2
#define RECONNECTION_TIME_IN_MILLISECONDS 5500
#define AUDIO_DEVICE_CONNECTION_CHECK_TIME_IN_MILLISECONDS 3000
#define SAD_UPDATE_CHECK_TIME_IN_MILLISECONDS 3000
#define ARC_DETECTION_CHECK_TIME_IN_MILLISECONDS 1000
#define AUDIO_DEVICE_POWER_TRANSITION_TIME_IN_MILLISECONDS 1000

#define RFC_PWRMGR2 "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Power.PwrMgr2.Enable"

#define ZOOM_SETTINGS_FILE      "/opt/persistent/rdkservices/zoomSettings.json"
#define ZOOM_SETTINGS_DIRECTORY "/opt/persistent/rdkservices"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 1
#define API_VERSION_NUMBER_PATCH 1

static bool isCecEnabled = false;
static int  hdmiArcPortId = -1;
static int retryPowerRequestCount = 0;
static int  hdmiArcVolumeLevel = 0;
std::vector<int> sad_list;
#ifdef USE_IARM
namespace
{
    /**
     * @struct Mapping
     * @brief Structure that defines members for the display setting service.
     * @ingroup SERVMGR_DISPSETTINGS
     */
    struct Mapping
    {
        const char *IArmBusName;
        const char *SvcManagerName;
    };

    static struct Mapping name_mappings[] = {
        { "Full", "FULL" },
        { "None", "NONE" },
        { "mono", "MONO" },
        { "stereo", "STEREO" },
        { "surround", "SURROUND" },
        { "unknown", "UNKNOWN" },
        // TODO: add your mappings here
        // { <IARM_NAME>, <SVC_MANAGER_API_NAME> },
        { 0,  0 }
    };

    string svc2iarm(const string &name)
    {
        const char *s = name.c_str();

        int i = 0;
        while (name_mappings[i].SvcManagerName)
        {
            if (strcmp(s, name_mappings[i].SvcManagerName) == 0)
                return name_mappings[i].IArmBusName;
            i++;
        }
        return name;
    }

    string iarm2svc(const string &name)
    {
        const char *s = name.c_str();

        int i = 0;
        while (name_mappings[i].IArmBusName)
        {
            if (strcmp(s, name_mappings[i].IArmBusName) == 0)
                return name_mappings[i].SvcManagerName;
            i++;
        }
        return name;
    }
}
#endif

// TODO: remove this
#define registerMethod(...) for (uint8_t i = 1; GetHandler(i); i++) GetHandler(i)->Register<JsonObject, JsonObject>(__VA_ARGS__)

namespace WPEFramework {

    namespace {

        static Plugin::Metadata<Plugin::DisplaySettings> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }

    namespace Plugin {

        namespace {
            // Display Settings should use inter faces
#ifndef USE_THUNDER_R4
            class Job : public Core::IDispatchType<void> {
#else
            class Job : public Core::IDispatch {
#endif /* USE_THUNDER_R4 */
            public:
                Job(std::function<void()> work)
                    : _work(work)
                {
                }
                void Dispatch() override
                {
                    _work();
                }

            private:
                std::function<void()> _work;
            };
            uint32_t getServiceState(PluginHost::IShell* shell, const string& callsign, PluginHost::IShell::state& state)
            {
                uint32_t result;
                auto interface = shell->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
                if (interface == nullptr) {
                    result = Core::ERROR_UNAVAILABLE;
                    std::cout << "no IShell for " << callsign << std::endl;
                } else {
                    result = Core::ERROR_NONE;
                    state = interface->State();
                    std::cout << "IShell state " << state << " for " << callsign << std::endl;
                    interface->Release();
                }
                return result;
            }
            uint32_t activate(PluginHost::IShell* shell, const string& callsign)
            {
                uint32_t result = Core::ERROR_ASYNC_FAILED;
                Core::Event event(false, true);
#ifndef USE_THUNDER_R4
                Core::IWorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatchType<void>>(Core::ProxyType<Job>::Create([&]() {
#else
                Core::IWorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatch>(Core::ProxyType<Job>::Create([&]() {
#endif /* USE_THUNDER_R4 */
                    auto interface = shell->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
                    if (interface == nullptr) {
                        result = Core::ERROR_UNAVAILABLE;
                        std::cout << "no IShell for " << callsign << std::endl;
                    } else {
                        result = interface->Activate(PluginHost::IShell::reason::REQUESTED);
                        std::cout << "IShell activate status " << result << " for " << callsign << std::endl;
                        interface->Release();
                    }
                    event.SetEvent();
                })));
                event.Lock();
                return result;
            }
        }

        SERVICE_REGISTRATION(DisplaySettings, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        DisplaySettings* DisplaySettings::_instance = nullptr;
        IARM_Bus_PWRMgr_PowerState_t DisplaySettings::m_powerState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY;

        DisplaySettings::DisplaySettings()
            : PluginHost::JSONRPC()
        {
            LOGINFO("ctor");
            DisplaySettings::_instance = this;
            m_client = nullptr;

            CreateHandler({ 2 });

            registerMethod("getConnectedVideoDisplays", &DisplaySettings::getConnectedVideoDisplays, this);
            registerMethod("getConnectedAudioPorts", &DisplaySettings::getConnectedAudioPorts, this);
            registerMethod("setEnableAudioPort", &DisplaySettings::setEnableAudioPort, this);
            registerMethod("getEnableAudioPort", &DisplaySettings::getEnableAudioPort, this);
            registerMethod("getSupportedResolutions", &DisplaySettings::getSupportedResolutions, this);
            registerMethod("getSupportedVideoDisplays", &DisplaySettings::getSupportedVideoDisplays, this);
            registerMethod("getSupportedTvResolutions", &DisplaySettings::getSupportedTvResolutions, this);
            registerMethod("getSupportedSettopResolutions", &DisplaySettings::getSupportedSettopResolutions, this);
            registerMethod("getSupportedAudioPorts", &DisplaySettings::getSupportedAudioPorts, this);
            registerMethod("getSupportedAudioModes", &DisplaySettings::getSupportedAudioModes, this);
	    registerMethod("getAudioFormat", &DisplaySettings::getAudioFormat, this);
            registerMethod("getZoomSetting", &DisplaySettings::getZoomSetting, this);
            registerMethod("setZoomSetting", &DisplaySettings::setZoomSetting, this);
            registerMethod("getCurrentResolution", &DisplaySettings::getCurrentResolution, this);
            registerMethod("setCurrentResolution", &DisplaySettings::setCurrentResolution, this);
            registerMethod("getSoundMode", &DisplaySettings::getSoundMode, this);
            registerMethod("setSoundMode", &DisplaySettings::setSoundMode, this);
            registerMethod("readEDID", &DisplaySettings::readEDID, this);
            registerMethod("readHostEDID", &DisplaySettings::readHostEDID, this);
            registerMethod("getActiveInput", &DisplaySettings::getActiveInput, this);
            registerMethod("getTvHDRSupport", &DisplaySettings::getTvHDRSupport, this);
            registerMethod("getSettopHDRSupport", &DisplaySettings::getSettopHDRSupport, this);
            registerMethod("setVideoPortStatusInStandby", &DisplaySettings::setVideoPortStatusInStandby, this);
            registerMethod("getVideoPortStatusInStandby", &DisplaySettings::getVideoPortStatusInStandby, this);
            registerMethod("getCurrentOutputSettings", &DisplaySettings::getCurrentOutputSettings, this);

            Register("getVolumeLeveller", &DisplaySettings::getVolumeLeveller, this);
            registerMethod("getBassEnhancer", &DisplaySettings::getBassEnhancer, this);
            registerMethod("isSurroundDecoderEnabled", &DisplaySettings::isSurroundDecoderEnabled, this);
            registerMethod("getDRCMode", &DisplaySettings::getDRCMode, this);
            Register("getSurroundVirtualizer", &DisplaySettings::getSurroundVirtualizer, this);
            Register("setVolumeLeveller", &DisplaySettings::setVolumeLeveller, this);
            registerMethod("setBassEnhancer", &DisplaySettings::setBassEnhancer, this);
            registerMethod("enableSurroundDecoder", &DisplaySettings::enableSurroundDecoder, this);
            Register("setSurroundVirtualizer", &DisplaySettings::setSurroundVirtualizer, this);
            registerMethod("setMISteering", &DisplaySettings::setMISteering, this);
            registerMethod("setGain", &DisplaySettings::setGain, this);
            registerMethod("getGain", &DisplaySettings::getGain, this);
            registerMethod("setMuted", &DisplaySettings::setMuted, this);
            registerMethod("getMuted", &DisplaySettings::getMuted, this);
            registerMethod("setVolumeLevel", &DisplaySettings::setVolumeLevel, this);
            registerMethod("getVolumeLevel", &DisplaySettings::getVolumeLevel, this);
            registerMethod("setDRCMode", &DisplaySettings::setDRCMode, this);
            registerMethod("getMISteering", &DisplaySettings::getMISteering, this);
            registerMethod("setMS12AudioCompression", &DisplaySettings::setMS12AudioCompression, this);
            registerMethod("getMS12AudioCompression", &DisplaySettings::getMS12AudioCompression, this);
            registerMethod("setDolbyVolumeMode", &DisplaySettings::setDolbyVolumeMode, this);
            registerMethod("getDolbyVolumeMode", &DisplaySettings::getDolbyVolumeMode, this);
            registerMethod("setDialogEnhancement", &DisplaySettings::setDialogEnhancement, this);
            registerMethod("getDialogEnhancement", &DisplaySettings::getDialogEnhancement, this);
            registerMethod("setIntelligentEqualizerMode", &DisplaySettings::setIntelligentEqualizerMode, this);
            registerMethod("getIntelligentEqualizerMode", &DisplaySettings::getIntelligentEqualizerMode, this);
            registerMethod("setGraphicEqualizerMode", &DisplaySettings::setGraphicEqualizerMode, this);
            registerMethod("getGraphicEqualizerMode", &DisplaySettings::getGraphicEqualizerMode, this);
            registerMethod("setMS12AudioProfile", &DisplaySettings::setMS12AudioProfile, this);
            registerMethod("getMS12AudioProfile", &DisplaySettings::getMS12AudioProfile, this);
	    registerMethod("getSupportedMS12AudioProfiles", &DisplaySettings::getSupportedMS12AudioProfiles, this);
            registerMethod("resetDialogEnhancement", &DisplaySettings::resetDialogEnhancement, this);
            registerMethod("resetBassEnhancer", &DisplaySettings::resetBassEnhancer, this);
            registerMethod("resetSurroundVirtualizer", &DisplaySettings::resetSurroundVirtualizer, this);
            registerMethod("resetVolumeLeveller", &DisplaySettings::resetVolumeLeveller, this);

            registerMethod("setAssociatedAudioMixing", &DisplaySettings::setAssociatedAudioMixing, this);
            registerMethod("getAssociatedAudioMixing", &DisplaySettings::getAssociatedAudioMixing, this);
            registerMethod("setFaderControl", &DisplaySettings::setFaderControl, this);
            registerMethod("getFaderControl", &DisplaySettings::getFaderControl, this);
            registerMethod("setPrimaryLanguage", &DisplaySettings::setPrimaryLanguage, this);
            registerMethod("getPrimaryLanguage", &DisplaySettings::getPrimaryLanguage, this);
            registerMethod("setSecondaryLanguage", &DisplaySettings::setSecondaryLanguage, this);
            registerMethod("getSecondaryLanguage", &DisplaySettings::getSecondaryLanguage, this);

            registerMethod("getAudioDelay", &DisplaySettings::getAudioDelay, this);
            registerMethod("setAudioDelay", &DisplaySettings::setAudioDelay, this);
            registerMethod("getAudioDelayOffset", &DisplaySettings::getAudioDelayOffset, this);
            registerMethod("setAudioDelayOffset", &DisplaySettings::setAudioDelayOffset, this);
            registerMethod("getSinkAtmosCapability", &DisplaySettings::getSinkAtmosCapability, this);
            registerMethod("setAudioAtmosOutputMode", &DisplaySettings::setAudioAtmosOutputMode, this);
            registerMethod("setForceHDRMode", &DisplaySettings::setForceHDRMode, this);
            registerMethod("getTVHDRCapabilities", &DisplaySettings::getTVHDRCapabilities, this);
            registerMethod("isConnectedDeviceRepeater", &DisplaySettings::isConnectedDeviceRepeater, this);
            registerMethod("getDefaultResolution", &DisplaySettings::getDefaultResolution, this);
            registerMethod("setScartParameter", &DisplaySettings::setScartParameter, this);
            registerMethod("getSettopMS12Capabilities", &DisplaySettings::getSettopMS12Capabilities, this);
            registerMethod("getSettopAudioCapabilities", &DisplaySettings::getSettopAudioCapabilities, this);
            registerMethod("setMS12ProfileSettingsOverride", &DisplaySettings::setMS12ProfileSettingsOverride,this);

            GetHandler(2)->Register<JsonObject, JsonObject>("getVolumeLeveller", &DisplaySettings::getVolumeLeveller2, this);
            GetHandler(2)->Register<JsonObject, JsonObject>("setVolumeLeveller", &DisplaySettings::setVolumeLeveller2, this);
            GetHandler(2)->Register<JsonObject, JsonObject>("getSurroundVirtualizer", &DisplaySettings::getSurroundVirtualizer2, this);
            GetHandler(2)->Register<JsonObject, JsonObject>("setSurroundVirtualizer", &DisplaySettings::setSurroundVirtualizer2, this);
            registerMethod("getVideoFormat", &DisplaySettings::getVideoFormat, this);

            registerMethod("setPreferredColorDepth", &DisplaySettings::setPreferredColorDepth, this);
            registerMethod("getPreferredColorDepth", &DisplaySettings::getPreferredColorDepth, this);
            registerMethod("getColorDepthCapabilities", &DisplaySettings::getColorDepthCapabilities, this);
            

	    m_subscribed = false; //HdmiCecSink event subscription
	    m_hdmiInAudioDeviceConnected = false;// Tells about the device connection state, for eArc will be updated on audio device power status event handler after tinymix command and incase of ARC will be true after ARC Initiation
	    m_arcEarcAudioEnabled = false; // Arc routing enabled/disabled
	    m_hdmiCecAudioDeviceDetected = false;// Audio device detected through cec ping
            m_hdmiInAudioDevicePowerState = AUDIO_DEVICE_POWER_STATE_UNKNOWN;// Power state of AVR
	    m_currentArcRoutingState = ARC_STATE_ARC_TERMINATED; // Maintains the ARC state
            m_isPwrMgr2RFCEnabled = false;
	    m_hdmiInAudioDeviceType = dsAUDIOARCSUPPORT_NONE;// Maintains the Audio device type whether Arc/eArc ocnnected
	    m_AudioDeviceSADState = AUDIO_DEVICE_SAD_UNKNOWN;// maintains the SAD state
	    m_sendMsgThreadExit = false;
	   // m_AudioSentPoweronmsg = false;
        }

        DisplaySettings::~DisplaySettings()
        {
            LOGINFO ("dtor");
        }

        void DisplaySettings::AudioPortsReInitialize()
        {
            LOGINFO("Entering DisplaySettings::AudioPortsReInitialize");
            try
            {
                device::List<device::AudioOutputPort> aPorts = device::Host::getInstance().getAudioOutputPorts();
                for (size_t i = 0; i < aPorts.size(); i++)
                {
                     device::AudioOutputPort &vPort = aPorts.at(i);
                     vPort.reInitializeAudioOutputPort();
                 }
            }
            catch(const device::Exception& err)
            {
                LOGWARN("Audio Port : AudioPortsReInitialize failed\n");
                LOG_DEVICE_EXCEPTION0();
            }
        }
     
        void DisplaySettings::InitAudioPorts() 
        {   //sample servicemanager response: {"success":true,"supportedAudioPorts":["HDMI0"]}
            //LOGINFOMETHOD();
            LOGINFO("Entering DisplaySettings::InitAudioPorts");
            uint32_t ret = Core::ERROR_NONE;
	    m_systemAudioMode_Power_RequestedAndReceived = true; //resetting this variable for bootup for AVR case
            try
            {
                device::List<device::AudioOutputPort> aPorts = device::Host::getInstance().getAudioOutputPorts();
                for (size_t i = 0; i < aPorts.size(); i++)
                {
                    device::AudioOutputPort &vPort = aPorts.at(i);
                    string portName  = vPort.getName();
                    //By default all the ports enabled.
                    bool isPortPersistenceValEnabled = true;
                    LOGINFO("DisplaySettings::InitAudioPorts getting port persistance");
                    try {
                        isPortPersistenceValEnabled = vPort.getEnablePersist ();
                    }
                    catch(const device::Exception& err)
                    {
                        LOGWARN("Audio Port : [%s] Getting enable persist value failed. Proceeding with true\n", portName.c_str());
                    }
                    LOGWARN("Audio Port : [%s] InitAudioPorts isPortPersistenceValEnabled:%d\n", portName.c_str(), isPortPersistenceValEnabled);
                    try {
                        m_hdmiCecAudioDeviceDetected = getHdmiCecSinkAudioDeviceConnectedStatus();
                    }
                    catch (const device::Exception& err){
                        LOG_DEVICE_EXCEPTION1(string("HDMI_ARC0"));
                    } 
                    if (portName == "HDMI_ARC0") {
                        int portId = -1;
                        vPort.getHdmiArcPortId(&portId);
                        if(portId >= 0) {
                           hdmiArcPortId = portId;
                           LOGWARN("HDMI ARC port ID hdmiArcPortId=%d\n",hdmiArcPortId);
                        }

                        //Set audio port config. ARC will be set up by onTimer()
                        #ifdef APP_CONTROL_AUDIOPORT_INIT
                        if(isPortPersistenceValEnabled ) {
                            LOGWARN("Audio Port : APP_CONTROL_AUDIOPORT_INIT Enabled\n");
                        #else
                        if(isPortPersistenceValEnabled &&  m_hdmiCecAudioDeviceDetected) {
                            LOGWARN("Audio Port : APP_CONTROL_AUDIOPORT_INIT Disabled\n");
                        #endif 
                            m_audioOutputPortConfig["HDMI_ARC"] = true;
                        }
                        else {
                            m_audioOutputPortConfig["HDMI_ARC"] = false;
                        }

                        //Stop timer if its already running
                        if(m_timer.isActive()) {
                            m_timer.stop();
                        }

			try {
		    		isCecEnabled = getHdmiCecSinkCecEnableStatus();
			}
			catch (const device::Exception& err){
				LOG_DEVICE_EXCEPTION1(string("HDMI_ARC0"));
			}

            PluginHost::IShell::state state;
            if ((getServiceState(m_service, HDMICECSINK_CALLSIGN, state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED)) {
                LOGINFO("%s is active", HDMICECSINK_CALLSIGN);

                if(!m_subscribed) {
			        if((subscribeForHdmiCecSinkEvent(HDMICECSINK_ARC_INITIATION_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_ARC_TERMINATION_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_SHORT_AUDIO_DESCRIPTOR_EVENT)== Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_SYSTEM_AUDIO_MODE_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_AUDIO_DEVICE_CONNECTED_STATUS_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_CEC_ENABLED_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_AUDIO_DEVICE_POWER_STATUS_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_ARC_AUDIO_STATUS_EVENT) == Core::ERROR_NONE)) {
                                    m_subscribed = true;
                                    LOGINFO("%s: HdmiCecSink event subscription completed.\n",__FUNCTION__);
			        }
			    }

			    if(m_subscribed) {
			     LOGINFO("m_hdmiCecAudioDeviceDetected status [%d] ... \n", m_hdmiCecAudioDeviceDetected);

			     if (m_hdmiCecAudioDeviceDetected)
			     {
	                        m_systemAudioMode_Power_RequestedAndReceived = false; // Means we have not received system audio mode ON or power ON msg from AVR.
                                sendMsgToQueue(SEND_AUDIO_DEVICE_POWERON_MSG, NULL);
				LOGINFO("Audio Port : [HDMI_ARC0] sendHdmiCecSinkAudioDevicePowerOn !!! \n");
				// Some AVR's and SB are not sending response for power on message even though it is in ON state
				// Send power request immediately to query power status of the AVR
				 LOGINFO("[HDMI_ARC0] Starting the timer to check audio device power status after power on msg!!!\n");
				 m_AudioDevicePowerOnStatusTimer.start(AUDIO_DEVICE_POWER_TRANSITION_TIME_IN_MILLISECONDS);
			     } /*m_hdmiCecAudioDeviceDetected */
                             else {
                                 LOGINFO("Starting the timer to recheck audio device connection state after : %d ms\n", AUDIO_DEVICE_CONNECTION_CHECK_TIME_IN_MILLISECONDS);
                                 m_AudioDeviceDetectTimer.start(AUDIO_DEVICE_CONNECTION_CHECK_TIME_IN_MILLISECONDS);
                             }
                            }
			}
			else {
                            //Start the timer only if the device supports HDMI_ARC
                            LOGINFO("Starting the timer");
                            m_timer.start(RECONNECTION_TIME_IN_MILLISECONDS);
			}
                    }
                    else {
                        JsonObject aPortHdmiEnableResult;
                        JsonObject aPortHdmiEnableParam;
  
                        aPortHdmiEnableParam.Set(_T("audioPort"), portName); //aPortHdmiEnableParam.Set(_T("audioPort"),"HDMI0");
                        //Get value from ds srv persistence
                        #ifdef APP_CONTROL_AUDIOPORT_INIT
                        if(isPortPersistenceValEnabled) {
                           LOGWARN("Audio Port : APP_CONTROL_AUDIOPORT_INIT Enabled\n");
                        #else
                        if(isPortPersistenceValEnabled || !m_hdmiCecAudioDeviceDetected) {
                           LOGWARN("Audio Port : APP_CONTROL_AUDIOPORT_INIT Disabled\n");
                        #endif
                            aPortHdmiEnableParam.Set(_T("enable"),true);
                        }
                        else {
                            aPortHdmiEnableParam.Set(_T("enable"),false);
                        }

                        ret = setEnableAudioPort (aPortHdmiEnableParam, aPortHdmiEnableResult);

                        if(ret != Core::ERROR_NONE) {
                            LOGWARN("Audio Port : [%s] enable: %d failed ! error code%d\n", portName.c_str(), isPortPersistenceValEnabled, ret);
                        }
                        else {
                            LOGINFO("Audio Port : [%s] initialized successfully, enable: %d\n", portName.c_str(), isPortPersistenceValEnabled);
                        }
                    }
                }
            }
            catch(const device::Exception& err)
            {
                LOGWARN("Audio Port : InitAudioPorts failed\n");
                LOG_DEVICE_EXCEPTION0();
            }
        }

        const string DisplaySettings::Initialize(PluginHost::IShell* service)
        {
            ASSERT(service != nullptr);
            ASSERT(m_service == nullptr);

            m_service = service;
            m_service->AddRef();

	    m_sendMsgThread = std::thread(sendMsgThread);
	    m_timer.connect(std::bind(&DisplaySettings::onTimer, this));
            m_AudioDeviceDetectTimer.connect(std::bind(&DisplaySettings::checkAudioDeviceDetectionTimer, this));
            m_ArcDetectionTimer.connect(std::bind(&DisplaySettings::checkArcDeviceConnected, this));
            m_SADDetectionTimer.connect(std::bind(&DisplaySettings::checkSADUpdate, this));
	    m_AudioDevicePowerOnStatusTimer.connect(std::bind(&DisplaySettings::checkAudioDevicePowerStatusTimer, this));

            InitializeIARM();

            if (IARM_BUS_PWRMGR_POWERSTATE_ON == getSystemPowerState())
            {
                InitAudioPorts();
            }
            else
            {
                LOGWARN("Current power state %d", m_powerState);
            }
            LOGWARN ("DisplaySettings::Initialize completes line:%d", __LINE__);
            // On success return empty, to indicate there is no error text.
            return (string());
        }

        void DisplaySettings::Deinitialize(PluginHost::IShell* service)
        {
	   LOGINFO("Enetering DisplaySettings::Deinitialize");
	   {
		std::unique_lock<std::mutex> lock(DisplaySettings::_instance->m_sendMsgMutex);
		DisplaySettings::_instance->m_sendMsgThreadExit = true;
                DisplaySettings::_instance->m_sendMsgThreadRun = true;
                DisplaySettings::_instance->m_sendMsgCV.notify_one();
	   }
	   try
	   {
		if (m_sendMsgThread.joinable())
			m_sendMsgThread.join();
	   }
	   catch(const std::system_error& e)
           {
		LOGERR("system_error exception in thread join %s", e.what());
	   }
	   catch(const std::exception& e)
	   {
		LOGERR("exception in thread join %s", e.what());
	   }

            stopCecTimeAndUnsubscribeEvent();

            DeinitializeIARM();
            DisplaySettings::_instance = nullptr;

            ASSERT(service == m_service);

            m_service->Release();
            m_service = nullptr;
        }

        void DisplaySettings::InitializeIARM()
        {
            if (Utils::IARM::init())
            {
                IARM_Result_t res;
                IARM_Bus_PWRMgr_GetPowerState_Param_t param;

                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_RX_SENSE, DisplResolutionHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_ZOOM_SETTINGS, DisplResolutionHandler) );
                //TODO(MROLLINS) localinput.cpp has PreChange guarded with #if !defined(DISABLE_PRE_RES_CHANGE_EVENTS)
                //Can we set it all the time from inside here and let localinput put guards around listening for our event?
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_RES_PRECHANGE,ResolutionPreChange) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE, ResolutionPostChange) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG, dsHdmiEventHandler) );
		IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG, dsHdmiEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_AUDIO_OUT_HOTPLUG, dsHdmiEventHandler) );
		IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_AUDIO_FORMAT_UPDATE, formatUpdateEventHandler) );
		IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_VIDEO_FORMAT_UPDATE, formatUpdateEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_ATMOS_CAPS_CHANGED, checkAtmosCapsEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, powerEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_AUDIO_PORT_STATE, audioPortStateEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_AUDIO_ASSOCIATED_AUDIO_MIXING_CHANGED, dsSettingsChangeEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_AUDIO_FADER_CONTROL_CHANGED, dsSettingsChangeEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_AUDIO_PRIMARY_LANGUAGE_CHANGED, dsSettingsChangeEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_AUDIO_SECONDARY_LANGUAGE_CHANGED, dsSettingsChangeEventHandler) );
 
                res = IARM_Bus_Call(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_API_GetPowerState, (void *)&param, sizeof(param));
                if (res == IARM_RESULT_SUCCESS)
                {
                    m_powerState = param.curState;
                    LOGINFO("DisplaySettings::m_powerState:%d", m_powerState);
                }
            }
            RFC_ParamData_t param = {0};
            WDMP_STATUS status = getRFCParameter(NULL, RFC_PWRMGR2, &param);
            if(WDMP_SUCCESS == status && param.type == WDMP_BOOLEAN && (strncasecmp(param.value,"true",4) == 0))
            {
                m_isPwrMgr2RFCEnabled = true;
            }
            try
            {
                //TODO(MROLLINS) this is probably per process so we either need to be running in our own process or be carefull no other plugin is calling it
                device::Manager::Initialize();
                LOGINFO("device::Manager::Initialize success");
            }
            catch(...)
            {
                LOGINFO("device::Manager::Initialize failed");
            }
        }

        void DisplaySettings::DeinitializeIARM()
        {
            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;

                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_RX_SENSE, DisplResolutionHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_ZOOM_SETTINGS, DisplResolutionHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_RES_PRECHANGE, ResolutionPreChange) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE, ResolutionPostChange) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG, dsHdmiEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG, dsHdmiEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_AUDIO_OUT_HOTPLUG, dsHdmiEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_DSMGR_EVENT_AUDIO_FORMAT_UPDATE, formatUpdateEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_VIDEO_FORMAT_UPDATE, formatUpdateEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, powerEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_AUDIO_PORT_STATE, audioPortStateEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_AUDIO_ASSOCIATED_AUDIO_MIXING_CHANGED, dsSettingsChangeEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_AUDIO_FADER_CONTROL_CHANGED, dsSettingsChangeEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_AUDIO_PRIMARY_LANGUAGE_CHANGED, dsSettingsChangeEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_AUDIO_SECONDARY_LANGUAGE_CHANGED, dsSettingsChangeEventHandler) );
            }

            try
            {
                //TODO(MROLLINS) this is probably per process so we either need to be running in our own process or be carefull no other plugin is calling it
                //No need to call device::Manager::DeInitialize for individual plugin. As it is a singleton instance and shared among all wpeframework plugins
                //Expecting DisplaySettings will be alive for complete run time of wpeframework
                device::Manager::DeInitialize();
                LOGINFO("device::Manager::DeInitialize success");
            }
            catch(...)
            {
                LOGINFO("device::Manager::DeInitialize failed");
            }
        }

        void DisplaySettings::ResolutionPreChange(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if(DisplaySettings::_instance)
            {
                DisplaySettings::_instance->resolutionPreChange();
            }
        }

        void DisplaySettings::ResolutionPostChange(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            int dw = 1280;
            int dh = 720;

            if (strcmp(owner, IARM_BUS_DSMGR_NAME) == 0)
            {
                switch (eventId) {
                    case IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE:
                        IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                        dw = eventData->data.resn.width;
                        dh = eventData->data.resn.height;
                        LOGINFO("width: %d, height: %d", dw, dh);
                        break;
                }
            }

            if(DisplaySettings::_instance)
            {
                DisplaySettings::_instance->resolutionChanged(dw, dh);
            }
        }

        void DisplaySettings::DisplResolutionHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            //TODO(MROLLINS) Receiver has this whole thing guarded by #ifndef HEADLESS_GW
            if (strcmp(owner,IARM_BUS_DSMGR_NAME) == 0)
            {
                switch (eventId)
                {
                case IARM_BUS_DSMGR_EVENT_RES_PRECHANGE:
                    break;
                case IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE:
                    {
                        int dw = 1280;
                        int dh = 720;
                        IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                        dw = eventData->data.resn.width ;
                        dh = eventData->data.resn.height ;
                        if(DisplaySettings::_instance)
                            DisplaySettings::_instance->resolutionChanged(dw,dh);
                    }
                    break;
                case IARM_BUS_DSMGR_EVENT_ZOOM_SETTINGS:
                    {
                        IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                        if(eventData->data.dfc.zoomsettings == dsVIDEO_ZOOM_NONE)
                        {
                            LOGINFO("dsVIDEO_ZOOM_NONE Settings");
                            if(DisplaySettings::_instance)
                                DisplaySettings::_instance->zoomSettingUpdated("NONE");
                        }
                        else if(eventData->data.dfc.zoomsettings == dsVIDEO_ZOOM_FULL)
                        {
                            LOGINFO("dsVIDEO_ZOOM_FULL Settings");
                            if(DisplaySettings::_instance)
                                DisplaySettings::_instance->zoomSettingUpdated("FULL");
                        }
                    }
                    break;
                case IARM_BUS_DSMGR_EVENT_RX_SENSE:
                    {

                        IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                        if(eventData->data.hdmi_rxsense.status == dsDISPLAY_RXSENSE_ON)
                        {
                            LOGINFO("Got dsDISPLAY_RXSENSE_ON -> notifyactiveInputChanged(true)");
                            if(DisplaySettings::_instance)
                                DisplaySettings::_instance->activeInputChanged(true);
                        }
                        else if(eventData->data.hdmi_rxsense.status == dsDISPLAY_RXSENSE_OFF)
                        {
                            LOGINFO("Got dsDISPLAY_RXSENSE_OFF -> notifyactiveInputChanged(false)");
                            if(DisplaySettings::_instance)
                                DisplaySettings::_instance->activeInputChanged(false);
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        }

        void DisplaySettings::dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            switch (eventId)
            {
            case IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG :
                //TODO(MROLLINS) note that there are several services listening for the notifyHdmiHotPlugEvent ServiceManagerNotifier broadcast
                //So if DisplaySettings becomes the owner/originator of this, then those future thunder plugins need to listen to our event
                //But of course, nothing is stopping any thunder plugin for listening to iarm event directly -- this is getting murky
                {
                    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                    int hdmi_hotplug_event = eventData->data.hdmi_hpd.event;
                    LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG  event data:%d ", hdmi_hotplug_event);
                    if(DisplaySettings::_instance)
                        DisplaySettings::_instance->connectedVideoDisplaysUpdated(hdmi_hotplug_event);
                }
                break;
                //TODO(MROLLINS) localinput.cpp was also sending these and they were getting handled by services other then DisplaySettings.  Should DisplaySettings own these as well ?
                /*
            case IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG :
                {
                    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                    int hdmiin_hotplug_port = eventData->data.hdmi_in_connect.port;
                    int hdmiin_hotplug_conn = eventData->data.hdmi_in_connect.isPortConnected;
                    LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG  event data:%d, %d ", hdmiin_hotplug_port);
                    ServiceManagerNotifier::getInstance()->notifyHdmiInputHotPlugEvent(hdmiin_hotplug_port, hdmiin_hotplug_conn);
                }
                break;
            case IARM_BUS_DSMGR_EVENT_HDCP_STATUS :
                {
                    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                    int hdcpStatus = eventData->data.hdmi_hdcp.hdcpStatus;
                    LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDCP_STATUS  event data:%d ", hdcpStatus);
                    ServiceManagerNotifier::getInstance()->notifyHdmiOutputHDCPStatus(hdcpStatus);
                }
                break;
                */
        case IARM_BUS_DSMGR_EVENT_AUDIO_OUT_HOTPLUG: {
            IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
            int iAudioPortType = eventData->data.audio_out_connect.portType;
            bool isPortConnected = eventData->data.audio_out_connect.isPortConnected;
            LOGINFO("Received IARM_BUS_DSMGR_EVENT_AUDIO_OUT_HOTPLUG for audio port %d event data:%d ", iAudioPortType, isPortConnected);
            if(DisplaySettings::_instance) {
                DisplaySettings::_instance->connectedAudioPortUpdated(iAudioPortType, isPortConnected);
            }
            else {
                LOGERR("DisplaySettings::dsHdmiEventHandler DisplaySettings::_instance is NULL\n");
            }

            break;
        }
	    case IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG :
		{
                    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                    int hdmiin_hotplug_port = eventData->data.hdmi_in_connect.port;
                    bool hdmiin_hotplug_conn = eventData->data.hdmi_in_connect.isPortConnected;
                    LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG  Port:%d, connected:%d \n", hdmiin_hotplug_port, hdmiin_hotplug_conn);

		    if(!DisplaySettings::_instance) {
                LOGERR("DisplaySettings::dsHdmiEventHandler DisplaySettings::_instance is NULL\n");
	                return;
            }

		    if(hdmiin_hotplug_port == hdmiArcPortId) { //HDMI ARC/eARC Port Handling

			try
			{
                            LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG  HDMI_ARC Port, connected:%d \n",  hdmiin_hotplug_conn);
                            if(hdmiin_hotplug_conn) {
                                LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG  HDMI_ARC Port \n");
			
			    }
                            else {
		    LOGINFO("Current Arc/eArc states m_currentArcRoutingState = %d, m_hdmiInAudioDeviceConnected =%d, m_arcEarcAudioEnabled =%d, m_hdmiInAudioDeviceType = %d\n", DisplaySettings::_instance->m_currentArcRoutingState, DisplaySettings::_instance->m_hdmiInAudioDeviceConnected, \
                                  DisplaySettings::_instance->m_arcEarcAudioEnabled, DisplaySettings::_instance->m_hdmiInAudioDeviceType);
				std::lock_guard<std::mutex> lock(DisplaySettings::_instance->m_AudioDeviceStatesUpdateMutex);
                                if (DisplaySettings::_instance->m_hdmiInAudioDeviceConnected == true) {
                            	    DisplaySettings::_instance->m_hdmiInAudioDeviceConnected =  false;
                                    DisplaySettings::_instance->m_hdmiInAudioDevicePowerState = AUDIO_DEVICE_POWER_STATE_UNKNOWN;
				 //if(DisplaySettings::_instance->m_arcEarcAudioEnabled == true) // commenting out for the AVR HPD 0 and 1 events instantly for TV standby in/out case
				    {
                                        DisplaySettings::_instance->connectedAudioPortUpdated(dsAUDIOPORT_TYPE_HDMI_ARC, hdmiin_hotplug_conn);
                                        LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG  HDMI_ARC Port disconnected. Notify UI !!!  \n");
				    }
                                }

                                {
                                   DisplaySettings::_instance->m_currentArcRoutingState = ARC_STATE_ARC_TERMINATED;
                                }

                            }// Release Mutex m_AudioDeviceStatesUpdateMutex
			}
                        catch (const device::Exception& err)
                        {
                            LOG_DEVICE_EXCEPTION1(string("HDMI_ARC0"));
                        }
	            }// HDMI_IN_ARC_PORT_ID

		}
	        break;
            default:
                //do nothing
                break;
            }
        }

        void DisplaySettings::formatUpdateEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {

	    LOGINFO("%s \n", __FUNCTION__);
            switch (eventId) {
                case IARM_BUS_DSMGR_EVENT_AUDIO_FORMAT_UPDATE:
                  {
                    dsAudioFormat_t audioFormat = dsAUDIO_FORMAT_NONE;
                    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                    audioFormat = eventData->data.AudioFormatInfo.audioFormat;
                    LOGINFO("Received IARM_BUS_DSMGR_EVENT_AUDIO_FORMAT_UPDATE. Audio format: %d \n", audioFormat);
                    if(DisplaySettings::_instance) {
                        DisplaySettings::_instance->notifyAudioFormatChange(audioFormat);
                    }
		  }
                  break;
                case IARM_BUS_DSMGR_EVENT_VIDEO_FORMAT_UPDATE:
                  {
                    dsHDRStandard_t videoFormat = dsHDRSTANDARD_NONE;
                    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                    videoFormat = eventData->data.VideoFormatInfo.videoFormat;
                    LOGINFO("Received IARM_BUS_DSMGR_EVENT_VIDEO_FORMAT_UPDATE. Video format: %d \n", videoFormat);
                    if(DisplaySettings::_instance) {
                        DisplaySettings::_instance->notifyVideoFormatChange(videoFormat);
                    }
		  }
                  break;
		default:
		    LOGERR("Invalid event ID\n");
		    break;
           }
        }

	void DisplaySettings::checkAtmosCapsEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {

        dsATMOSCapability_t atmosCaps = dsAUDIO_ATMOS_NOTSUPPORTED;
        bool atmosCapsChangedstatus;
        IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
        atmosCaps = eventData->data.AtmosCapsChange.caps;
        atmosCapsChangedstatus = eventData->data.AtmosCapsChange.status;
        LOGINFO("Received IARM_BUS_DSMGR_EVENT_ATMOS_CAPS_CHANGED: %d \n", atmosCaps);
        if(DisplaySettings::_instance && atmosCapsChangedstatus) {
        DisplaySettings::_instance->notifyAtmosCapabilityChange(atmosCaps);
        }
    }        
        void DisplaySettings::audioPortStateEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            dsAudioPortState_t audioPortState = dsAUDIOPORT_STATE_UNINITIALIZED;
            LOGINFO("%s \n", __FUNCTION__);
            switch (eventId) {
                case IARM_BUS_DSMGR_EVENT_AUDIO_PORT_STATE:
                {
                   IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                   audioPortState = eventData->data.AudioPortStateInfo.audioPortState;
                   LOGINFO("Received IARM_BUS_DSMGR_EVENT_AUDIO_PORT_STATE. Audio Port Init State: %d \n", audioPortState);
                   try
                   {   if( audioPortState == dsAUDIOPORT_STATE_INITIALIZED)
                       {
                           DisplaySettings::_instance->AudioPortsReInitialize();
                           DisplaySettings::_instance->InitAudioPorts();
                       }
                  }
                  catch(const device::Exception& err)
                  {
                     LOG_DEVICE_EXCEPTION0();
                  }
                }
                break;
                default:
                  LOGERR("Invalid event ID\n");
                  break;
           }  
        }  

        void DisplaySettings::dsSettingsChangeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {

            LOGINFO("%s \n", __FUNCTION__);
            if (data == NULL) {
                LOGERR("data is NULL, return !!!\n");
                return;
            }
            switch (eventId) {
                case IARM_BUS_DSMGR_EVENT_AUDIO_ASSOCIATED_AUDIO_MIXING_CHANGED:
                  {
                    bool mixing = false;
                    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                    mixing = eventData->data.AssociatedAudioMixingInfo.mixing;
                    LOGINFO("Received IARM_BUS_DSMGR_EVENT_AUDIO_ASSOCIATED_AUDIO_MIXING_CHANGED. Associated Audio Mixing: %d \n", mixing);
                    if(DisplaySettings::_instance) {
                        DisplaySettings::_instance->notifyAssociatedAudioMixingChange(mixing);
                    }
                  }
                  break;
                case IARM_BUS_DSMGR_EVENT_AUDIO_FADER_CONTROL_CHANGED:
                  {
                    int mixerbalance = 0;
                    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                    mixerbalance = eventData->data.FaderControlInfo.mixerbalance;
                    LOGINFO("Received IARM_BUS_DSMGR_EVENT_AUDIO_FADER_CONTROL_CHANGED. Fader Control: %d \n", mixerbalance);
                    if(DisplaySettings::_instance) {
                        DisplaySettings::_instance->notifyFaderControlChange(mixerbalance);
                    }
                  }
                  break;
                case IARM_BUS_DSMGR_EVENT_AUDIO_PRIMARY_LANGUAGE_CHANGED:
                  {
                    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                    std::string pLang = eventData->data.AudioLanguageInfo.audioLanguage;
                    LOGINFO("Received IARM_BUS_DSMGR_EVENT_AUDIO_PRIMARY_LANGUAGE_CHANGED. Primary Language: %s \n", pLang.c_str());
                    if(DisplaySettings::_instance) {
                        DisplaySettings::_instance->notifyPrimaryLanguageChange(pLang);
                    }
                  }
                  break;
                case IARM_BUS_DSMGR_EVENT_AUDIO_SECONDARY_LANGUAGE_CHANGED:
                  {
                    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                    std::string sLang = eventData->data.AudioLanguageInfo.audioLanguage;
                    LOGINFO("Received IARM_BUS_DSMGR_EVENT_AUDIO_SECONDARY_LANGUAGE_CHANGED. Secondary Language: %s \n", sLang.c_str());
                    if(DisplaySettings::_instance) {
                        DisplaySettings::_instance->notifySecondaryLanguageChange(sLang);
                    }
                  }
                  break;
                default:
                    LOGERR("Unhandled Event... \n");
                    break;
           }
        }

        void setResponseArray(JsonObject& response, const char* key, const vector<string>& items)
        {
            JsonArray arr;
            for(auto& i : items) arr.Add(JsonValue(i));

            response[key] = arr;

            string json;
            response.ToString(json);
        }

        //Begin methods
        uint32_t DisplaySettings::getConnectedVideoDisplays(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response: {"connectedVideoDisplays":["HDMI0"],"success":true}
            //this                          : {"connectedVideoDisplays":["HDMI0"]}
            LOGINFOMETHOD();

            vector<string> connectedVideoDisplays;
            getConnectedVideoDisplaysHelper(connectedVideoDisplays);
            setResponseArray(response, "connectedVideoDisplays", connectedVideoDisplays);
            returnResponse(true);
        }

        uint32_t DisplaySettings::getConnectedAudioPorts(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response: {"success":true,"connectedAudioPorts":["HDMI0"]}
            LOGINFOMETHOD();
            vector<string> connectedAudioPorts;
            try
            {
                device::List<device::AudioOutputPort> aPorts = device::Host::getInstance().getAudioOutputPorts();
                for (size_t i = 0; i < aPorts.size(); i++)
                {
                    device::AudioOutputPort &aPort = aPorts.at(i);
                    string portName = aPort.getName();
                    if (aPort.isConnected())
                    {
                        if((portName == "HDMI_ARC0") && (m_hdmiInAudioDeviceConnected != true)) {
                            continue;
                        }
                        vectorSet(connectedAudioPorts, portName);
                    }
		    else if (portName == "HDMI_ARC0" && m_hdmiInAudioDeviceConnected == true && m_arcEarcAudioEnabled == false)
		    {
	               /* This is the case where we get ARC initiation or eARC detection done before HPD.Send connectedport update as ARC disconnected and Restart the ARC-eARC again */
			m_hdmiInAudioDeviceConnected = false;
			m_hdmiInAudioDevicePowerState = AUDIO_DEVICE_POWER_STATE_UNKNOWN;
			m_currentArcRoutingState = ARC_STATE_ARC_TERMINATED;
			m_hdmiInAudioDeviceType = dsAUDIOARCSUPPORT_NONE;
			m_AudioDeviceSADState = AUDIO_DEVICE_SAD_UNKNOWN;
			DisplaySettings::_instance->connectedAudioPortUpdated(dsAUDIOPORT_TYPE_HDMI_ARC, false);
			LOGINFO("[HDMI_ARC0] sendHdmiCecSinkAudioDevicePowerOn !!! \n");
			sendMsgToQueue(SEND_AUDIO_DEVICE_POWERON_MSG, NULL);
		    }
                }
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
            }
            setResponseArray(response, "connectedAudioPorts", connectedAudioPorts);
            returnResponse(true);
        }

        uint32_t DisplaySettings::getSupportedResolutions(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:{"success":true,"supportedResolutions":["720p","1080i","1080p60"]}
            LOGINFOMETHOD();
            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
            string videoDisplay = parameters.HasLabel("videoDisplay") ? parameters["videoDisplay"].String() : strVideoPort;
            vector<string> supportedResolutions;
            try
            {
                device::VideoOutputPort &vPort = device::Host::getInstance().getVideoOutputPort(videoDisplay);
                const device::List<device::VideoResolution> resolutions = device::VideoOutputPortConfig::getInstance().getPortType(vPort.getType().getId()).getSupportedResolutions();
                for (size_t i = 0; i < resolutions.size(); i++) {
                    const device::VideoResolution &resolution = resolutions.at(i);
                    string supportedResolution = resolution.getName();
                    vectorSet(supportedResolutions,supportedResolution);
                }
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(videoDisplay);
            }
            setResponseArray(response, "supportedResolutions", supportedResolutions);
            returnResponse(true);
        }

        uint32_t DisplaySettings::getSupportedVideoDisplays(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response: {"supportedVideoDisplays":["HDMI0"],"success":true}
            LOGINFOMETHOD();
            vector<string> supportedVideoDisplays;
            try
            {
                device::List<device::VideoOutputPort> vPorts = device::Host::getInstance().getVideoOutputPorts();
                for (size_t i = 0; i < vPorts.size(); i++)
                {
                    device::VideoOutputPort &vPort = vPorts.at(i);
                    string videoDisplay = vPort.getName();
                    vectorSet(supportedVideoDisplays, videoDisplay);
                }
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
            }
            setResponseArray(response, "supportedVideoDisplays", supportedVideoDisplays);
            returnResponse(true);
        }

        uint32_t DisplaySettings::getSupportedTvResolutions(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:{"success":true,"supportedTvResolutions":["480i","480p","576i","720p","1080i","1080p"]}
            LOGINFOMETHOD();
            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
            string videoDisplay = parameters.HasLabel("videoDisplay") ? parameters["videoDisplay"].String() : strVideoPort;
            vector<string> supportedTvResolutions;
            try
            {
                int tvResolutions = 0;
                device::VideoOutputPort &vPort = device::Host::getInstance().getVideoOutputPort(videoDisplay);
                vPort.getSupportedTvResolutions(&tvResolutions);
                if(!tvResolutions)supportedTvResolutions.emplace_back("none");
                if(tvResolutions & dsTV_RESOLUTION_480i)supportedTvResolutions.emplace_back("480i");
                if(tvResolutions & dsTV_RESOLUTION_480p)supportedTvResolutions.emplace_back("480p");
                if(tvResolutions & dsTV_RESOLUTION_576i)supportedTvResolutions.emplace_back("576i");
                if(tvResolutions & dsTV_RESOLUTION_576p)supportedTvResolutions.emplace_back("576p");
		if(tvResolutions & dsTV_RESOLUTION_576p50)supportedTvResolutions.emplace_back("576p50");
                if(tvResolutions & dsTV_RESOLUTION_720p)supportedTvResolutions.emplace_back("720p");
		if(tvResolutions & dsTV_RESOLUTION_720p50)supportedTvResolutions.emplace_back("720p50");
                if(tvResolutions & dsTV_RESOLUTION_1080i)supportedTvResolutions.emplace_back("1080i");
                if(tvResolutions & dsTV_RESOLUTION_1080p)supportedTvResolutions.emplace_back("1080p");
		if(tvResolutions & dsTV_RESOLUTION_1080p24)supportedTvResolutions.emplace_back("1080p24");
		if(tvResolutions & dsTV_RESOLUTION_1080i25)supportedTvResolutions.emplace_back("1080i25");
		if(tvResolutions & dsTV_RESOLUTION_1080p30)supportedTvResolutions.emplace_back("1080p30");
		if(tvResolutions & dsTV_RESOLUTION_1080i50)supportedTvResolutions.emplace_back("1080i50");
		if(tvResolutions & dsTV_RESOLUTION_1080p50)supportedTvResolutions.emplace_back("1080p50");
                if(tvResolutions & dsTV_RESOLUTION_1080p60)supportedTvResolutions.emplace_back("1080p60");
                if(tvResolutions & dsTV_RESOLUTION_2160p30)supportedTvResolutions.emplace_back("2160p30");
		if(tvResolutions & dsTV_RESOLUTION_2160p50)supportedTvResolutions.emplace_back("2160p50");
                if(tvResolutions & dsTV_RESOLUTION_2160p60)supportedTvResolutions.emplace_back("2160p60");
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(videoDisplay);
            }
            setResponseArray(response, "supportedTvResolutions", supportedTvResolutions);
            returnResponse(true);
        }

        uint32_t DisplaySettings::getSupportedSettopResolutions(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:{"success":true,"supportedSettopResolutions":["720p","1080i","1080p60"]}
            LOGINFOMETHOD();
            vector<string> supportedSettopResolutions;
            try
            {
                if (device::Host::getInstance().getVideoDevices().size() < 1)
                {
                    LOGINFO("DSMGR_NOT_RUNNING");
                    returnResponse(false);
                }

                device::VideoDevice &device = device::Host::getInstance().getVideoDevices().at(0);
                list<string> resolutions;
                device.getSettopSupportedResolutions(resolutions);
                for (list<string>::const_iterator ci = resolutions.begin(); ci != resolutions.end(); ++ci)
                {
                      string supportedResolution = *ci;
                      vectorSet(supportedSettopResolutions, supportedResolution);
                }
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
            }
            setResponseArray(response, "supportedSettopResolutions", supportedSettopResolutions);
            returnResponse(true);
        }

        uint32_t DisplaySettings::getSupportedAudioPorts(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response: {"success":true,"supportedAudioPorts":["HDMI0"]}
            LOGINFOMETHOD();
            vector<string> supportedAudioPorts;
            try
            {
                device::List<device::AudioOutputPort> aPorts = device::Host::getInstance().getAudioOutputPorts();
                for (size_t i = 0; i < aPorts.size(); i++)
                {
                    device::AudioOutputPort &vPort = aPorts.at(i);
                    string portName  = vPort.getName();
                    vectorSet(supportedAudioPorts,portName);
                }
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
            }
            setResponseArray(response, "supportedAudioPorts", supportedAudioPorts);
            returnResponse(true);
        }

        uint32_t DisplaySettings::getSupportedAudioModes(const JsonObject& parameters, JsonObject& response)
        {   //sample response: {"success":true,"supportedAudioModes":["STEREO","PASSTHRU","AUTO (Dolby Digital 5.1)"]}
            LOGINFOMETHOD();
            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "";
            vector<string> supportedAudioModes;
            try
            {
                bool HAL_hasSurround = false;

                device::List<device::VideoOutputPort> vPorts = device::Host::getInstance().getVideoOutputPorts();
                for (size_t i = 0; i < vPorts.size(); i++) {
                    device::AudioOutputPort &aPort = vPorts.at(i).getAudioOutputPort();
                    for (size_t j = 0; j < aPort.getSupportedStereoModes().size(); j++) {
                        if (audioPort.empty() || Utils::String::stringContains(aPort.getName(), audioPort))
                        {
                            string audioMode = aPort.getSupportedStereoModes().at(j).getName();

                            // Starging Version 5, "Surround" mode is replaced by "Auto Mode"
                            if (strcasecmp(audioMode.c_str(),"SURROUND") == 0)
                            {
                                HAL_hasSurround = true;
                                continue;
                            }

                            vectorSet(supportedAudioModes,audioMode);
                        }
                    }
                }

                if (Utils::String::stringContains(audioPort, "HDMI0"))
                {
                    std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                    device::VideoOutputPort vPort = device::VideoOutputPortConfig::getInstance().getPort(strVideoPort.c_str());
                    int surroundMode = false;
                    try{
                        surroundMode = vPort.getDisplay().getSurroundMode();
                    }
                    catch(const device::Exception& err)
                    {
                        surroundMode = false;
                        LOG_DEVICE_EXCEPTION1(audioPort);
                    }
                    if (vPort.isDisplayConnected() && surroundMode)
                    {
                        if(surroundMode & dsSURROUNDMODE_DDPLUS )
                        {
                            LOGINFO("HDMI0 has surround DD Plus ");
                            supportedAudioModes.emplace_back("AUTO (Dolby Digital Plus)");
                        }
                        else if(surroundMode & dsSURROUNDMODE_DD )
                        {
                            LOGINFO("HDMI0 has surround DD5.1 ");
                            supportedAudioModes.emplace_back("AUTO (Dolby Digital 5.1)");
                        }
                    }
                    else {
                        LOGINFO("HDMI0 does not have surround");
                        supportedAudioModes.emplace_back("AUTO (Stereo)");
                    }
                }
		else if (audioPort.empty() || Utils::String::stringContains(audioPort, "SPDIF0") || Utils::String::stringContains(audioPort, "HDMI_ARC0"))
                {
                    if (HAL_hasSurround) {
                        supportedAudioModes.emplace_back("SURROUND");
                    }
                }
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(audioPort);
            }
            setResponseArray(response, "supportedAudioModes", supportedAudioModes);
            returnResponse(true);
        }

        uint32_t DisplaySettings::getZoomSetting(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();
            string zoomSetting = "unknown";

            bool success = true;
            try
            {
                if (device::Host::getInstance().getVideoDevices().size() < 1)
                {
                    LOGINFO("DSMGR_NOT_RUNNING");
                    returnResponse(false);
                }

                // TODO: why is this always the first one in the list
                device::VideoDevice &decoder = device::Host::getInstance().getVideoDevices().at(0);
                zoomSetting = decoder.getDFC().getName();
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
                success = false;
            }
#ifdef USE_IARM
            zoomSetting = iarm2svc(zoomSetting);
#endif
            response["zoomSetting"] = zoomSetting;
            returnResponse(success);
        }

        uint32_t DisplaySettings::setZoomSetting(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();

            returnIfParamNotFound(parameters, "zoomSetting");
            string zoomSetting = parameters["zoomSetting"].String();

            bool success = true;
            try
            {
#ifdef USE_IARM
                zoomSetting = svc2iarm(zoomSetting);
#endif
                if (device::Host::getInstance().getVideoDevices().size() < 1)
                {
                    LOGINFO("DSMGR_NOT_RUNNING");
                    returnResponse(false);
                }

                // TODO: why is this always the first one in the list?
                device::VideoDevice &decoder = device::Host::getInstance().getVideoDevices().at(0);
                decoder.setDFC(zoomSetting);
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(zoomSetting);
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::getCurrentResolution(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:{"success":true,"resolution":"720p"}
            LOGINFOMETHOD();
            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
            string videoDisplay = parameters.HasLabel("videoDisplay") ? parameters["videoDisplay"].String() : strVideoPort;
            bool success = true;
            try
            {
                device::VideoOutputPort &vPort = device::Host::getInstance().getVideoOutputPort(videoDisplay);
                response["resolution"] = vPort.getResolution().getName();
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(videoDisplay);
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::setCurrentResolution(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();
            returnIfParamNotFound(parameters, "videoDisplay");
            returnIfParamNotFound(parameters, "resolution");

            string videoDisplay = parameters["videoDisplay"].String();
            string resolution = parameters["resolution"].String();

            bool hasPersist = parameters.HasLabel("persist");
            bool persist = hasPersist ? parameters["persist"].Boolean() : true;
            if (!hasPersist) LOGINFO("persist: true");
 
            bool isIgnoreEdidArg = parameters.HasLabel("ignoreEdid");
            bool isIgnoreEdid = isIgnoreEdidArg ? parameters["ignoreEdid"].Boolean() : false;
            if (!isIgnoreEdidArg) LOGINFO("isIgnoreEdid: false"); else LOGINFO("isIgnoreEdid: %d", isIgnoreEdid);

            bool success = true;
            try
            {
                device::VideoOutputPort &vPort = device::Host::getInstance().getVideoOutputPort(videoDisplay);
                vPort.setResolution(resolution, persist, isIgnoreEdid);
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION2(videoDisplay, resolution);
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::getSoundMode(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:{"success":true,"soundMode":"AUTO (Dolby Digital 5.1)"}
            LOGINFOMETHOD();
            string audioPort = parameters["audioPort"].String();//empty value will browse all ports

            if (!checkPortName(audioPort))
                audioPort = "HDMI0";

            string modeString("");
            device::AudioStereoMode mode = device::AudioStereoMode::kStereo;  //default to stereo

            try
            {
                /* Return the sound mode of the audio ouput connected to the specified videoDisplay */
                /* Check if HDMI is connected - Return (default) Stereo Mode if not connected */
                if (audioPort.empty())
                {
                    std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                    if (device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str()).isDisplayConnected())
                    {
                        audioPort = "HDMI0";
                    }
                    else
                    {
                        /*  * If HDMI is not connected
                            * Get the SPDIF if it is supported by platform
                            * If Platform does not have connected ports. Default to HDMI.
                        */
                        audioPort = "HDMI0";
                        device::List<device::VideoOutputPort> vPorts = device::Host::getInstance().getVideoOutputPorts();
                        for (size_t i = 0; i < vPorts.size(); i++)
                        {
                            device::VideoOutputPort &vPort = vPorts.at(i);
                            if (vPort.isDisplayConnected())
                            {
                                audioPort = "SPDIF0";
                                break;
                            }
                        }
                    }
                }

                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);

                if (aPort.isConnected())
                {
                    mode = aPort.getStereoMode();
                    if (aPort.getType().getId() == device::AudioOutputPortType::kHDMI)
                    {
                        /* In DS5, "Surround" implies "Auto" */
                        if (aPort.getStereoAuto() || mode == device::AudioStereoMode::kSurround)
                        {
                            LOGINFO("HDMI0 is in Auto Mode");
                            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                            int surroundMode = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str()).getDisplay().getSurroundMode();
                            if ( surroundMode & dsSURROUNDMODE_DDPLUS)
                            {
                                LOGINFO("HDMI0 has surround DDPlus");
                                modeString.append("AUTO (Dolby Digital Plus)");
                            }
                            else if (surroundMode & dsSURROUNDMODE_DD)
                            {
                                LOGINFO("HDMI0 has surround DD 5.1");
                                modeString.append("AUTO (Dolby Digital 5.1)");
                            }
                            else
                            {
                                LOGINFO("HDMI0 does not surround");
                                modeString.append("AUTO (Stereo)");
                            }
                        }
                        else if ( mode == device::AudioStereoMode::kDD)
                        {
                            LOGINFO("HDMI0 is in dolby digital Mode");
                            modeString.append("DOLBYDIGITAL");
                        }
                        else if ( mode == device::AudioStereoMode::kDDPlus)
                        {
                            LOGINFO("HDMI0 is in dolby digital Plus Mode");
                            modeString.append("DOLBYDIGITALPLUS");
                        }
                        else
                            modeString.append(mode.toString());
                    }
		    else if((aPort.getType().getId() == device::AudioOutputPortType::kARC) || (aPort.getType().getId() == device::AudioOutputPortType::kSPDIF)){
                        if (aPort.getStereoAuto()) {
                            LOGINFO("%s output mode Auto", audioPort.c_str());
                            modeString.append("AUTO");
			}
			else{
			    modeString.append(mode.toString());
			}
		    }
                    else
                    {
                            modeString.append(mode.toString());
                    }
                }
                else
                {
		    if((aPort.getType().getId() == device::AudioOutputPortType::kARC)){
                        if (aPort.getStereoAuto()) {
                            LOGINFO("%s output mode Auto", audioPort.c_str());
                            modeString.append("AUTO");
                        }
                        else{
                            mode = aPort.getStereoMode();
                            modeString.append(mode.toString());
                        }
                    }
                    else if((aPort.getType().getId() == device::AudioOutputPortType::kHDMI)){
                        mode = aPort.getStereoMode();
                        if (aPort.getStereoAuto() || mode == device::AudioStereoMode::kSurround)
                        {
                            LOGINFO("%s output mode Auto", audioPort.c_str());
                            modeString.append("AUTO (Stereo)");
                        }
                        else{
                            modeString.append(mode.toString());
                        }
                    }
                    else {
                        /*
                        * VideoDisplay is not connected. Its audio mode is unknown. Return
                        * "Stereo" as safe default;
                        */
                        mode = device::AudioStereoMode::kStereo;
                        modeString.append(mode.toString());
                    }
                }
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
                //
                // Exception
                // "Stereo" as safe default;
                //
                mode = device::AudioStereoMode::kStereo;
		modeString.append(mode.toString());
            }

            LOGWARN("audioPort = %s, mode = %s!", audioPort.c_str(), modeString.c_str());
#ifdef USE_IARM
            modeString = iarm2svc(modeString);
#endif
            response["soundMode"] = modeString;
            returnResponse(true);
        }

        uint32_t DisplaySettings::setSoundMode(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();
            string audioPort = parameters["audioPort"].String();//missing or empty string and we will set all ports

            returnIfParamNotFound(parameters, "soundMode");
            string soundMode = parameters["soundMode"].String();
            Utils::String::toLower(soundMode);

            bool hasPersist = parameters.HasLabel("persist");
            bool persist = hasPersist ? parameters["persist"].Boolean() : true;
            if (!hasPersist) LOGINFO("persist: true");

            bool success = true;
            device::AudioStereoMode mode = device::AudioStereoMode::kStereo;  //default to stereo
            bool stereoAuto = false;

            if (soundMode == "mono" || soundMode == "MONO")
                mode = device::AudioStereoMode::kMono;
            else if (soundMode == "stereo" || soundMode == "STEREO")
                mode = device::AudioStereoMode::kStereo;
            else if (soundMode == "surround" || soundMode == "SURROUND")
                mode = device::AudioStereoMode::kSurround;
            else if (soundMode == "passthru" || soundMode == "PASSTHRU")
                mode = device::AudioStereoMode::kPassThru;
            else if (soundMode == "dolbydigital" || soundMode == "DOLBYDIGITAL")
                mode = device::AudioStereoMode::kDD;
            else if (soundMode == "dolbydigitalplus" || soundMode == "DOLBYDIGITALPLUS")
                mode = device::AudioStereoMode::kDDPlus;
            else if (soundMode == "auto" || soundMode == "auto " || soundMode == "AUTO" || soundMode == "AUTO ")
            {
                /*
                 * anything after "auto" is only descriptive, and can be ignored.
                 * use kSurround in this case.
                 */
                if (audioPort.empty())
                    audioPort = "HDMI0";

                stereoAuto = true;
                mode = device::AudioStereoMode::kSurround;
            }
            else if (soundMode == "dolby digital 5.1")
                mode = device::AudioStereoMode::kSurround;
            else
            {
                LOGWARN("Sound mode '%s' is empty or incompatible with known values, hence sound mode will not changed!", soundMode.c_str());
                returnResponse(success);
            }

            if (!checkPortName(audioPort))
            {
                LOGERR("Invalid port Name: display = %s, mode = %s!", audioPort.c_str(), soundMode.c_str());
                returnResponse(false);
            }

            LOGWARN("display = %s, mode = %s!", audioPort.c_str(), soundMode.c_str());

            try
            {
                //now setting the sound mode for specified video display types
                if (!audioPort.empty())
                {
                    device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                    if (aPort.isConnected())
                    {
                        /* Auto mode is only for HDMI and DS5 and non-Passthru*/
                        if (aPort.getType().getId() == device::AudioOutputPortType::kHDMI && (!(mode == device::AudioStereoMode::kPassThru)))
                        {
                            aPort.setStereoAuto(stereoAuto, persist);
                            if (stereoAuto)
                            {
                                std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                                if (device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str()).getDisplay().getSurroundMode())
                                    mode = device::AudioStereoMode::kSurround;
                                else
                                    mode = device::AudioStereoMode::kStereo;
                            }
                            //TODO: if mode has not changed, we can skip the extra call
                            aPort.setStereoMode(mode.toString(), persist);
                        }
                        else if (aPort.getType().getId() == device::AudioOutputPortType::kHDMI)
                        {
                            LOGERR("Reset auto on %s for mode = %s!", audioPort.c_str(), soundMode.c_str());
                            aPort.setStereoAuto(false, persist);
                            //TODO: if mode has not changed, we can skip the extra call
                            aPort.setStereoMode(mode.toString(), persist);
                        }
			else if (aPort.getType().getId() == device::AudioOutputPortType::kARC) {


		            if(((mode == device::AudioStereoMode::kSurround) || (mode == device::AudioStereoMode::kPassThru) || (mode == device::AudioStereoMode::kStereo)) && (stereoAuto == false)) {
				    aPort.setStereoAuto(false, persist);

				    if((m_hdmiInAudioDeviceType == dsAUDIOARCSUPPORT_ARC) && (m_hdmiInAudioDeviceConnected == true)) {
					if (mode == device::AudioStereoMode::kPassThru) {
					    if (m_AudioDeviceSADState  == AUDIO_DEVICE_SAD_CLEARED || \
							    m_AudioDeviceSADState  == AUDIO_DEVICE_SAD_UNKNOWN) {
						LOGINFO("%s: sending SAD request\n", __FUNCTION__);
			                        sendMsgToQueue(REQUEST_SHORT_AUDIO_DESCRIPTOR, NULL);
                                                m_AudioDeviceSADState  = AUDIO_DEVICE_SAD_REQUESTED; 
                                                LOGINFO("setSoundMode Passthru: SAD Requested\n");
					    }
					} else if (mode == device::AudioStereoMode::kStereo) {
					    if (m_AudioDeviceSADState  != AUDIO_DEVICE_SAD_CLEARED) {
						LOGINFO("%s: Clearing the SAD since audio mode is changed to PCM\n", __FUNCTION__);
						m_AudioDeviceSADState  = AUDIO_DEVICE_SAD_CLEARED;
						//clear the SAD list
						sad_list.clear();
					    }
					}
                                    }
				    aPort.setStereoMode(mode.toString(), persist);
		            }
			    else { //Auto Mode

				if(m_hdmiInAudioDeviceType == dsAUDIOARCSUPPORT_eARC) {
				    aPort.setStereoAuto(stereoAuto, persist); //setStereoAuto true
				}
				else if ((m_hdmiInAudioDeviceType == dsAUDIOARCSUPPORT_ARC) && (m_hdmiInAudioDeviceConnected == true)) {
				    if (m_AudioDeviceSADState  == AUDIO_DEVICE_SAD_CLEARED ||\
						    m_AudioDeviceSADState  == AUDIO_DEVICE_SAD_UNKNOWN) {
					LOGINFO("%s: sending SAD request\n", __FUNCTION__);
			                sendMsgToQueue(REQUEST_SHORT_AUDIO_DESCRIPTOR, NULL);
                                        m_AudioDeviceSADState  = AUDIO_DEVICE_SAD_REQUESTED;
                                        LOGINFO("setSoundMode Auto: SAD Requested\n");
				    }
				    aPort.setStereoAuto(stereoAuto, persist); //setStereoAuto true
				}
			   }
			}
                        else if (aPort.getType().getId() == device::AudioOutputPortType::kSPDIF)
                        {
			    if(stereoAuto == false) {
                                aPort.setStereoAuto(false, persist);
                                aPort.setStereoMode(mode.toString(), persist);
			    }
			    else{
			        aPort.setStereoAuto(true, persist);
			    }
                        }

                    }
		    else {
                        if (aPort.getType().getId() == device::AudioOutputPortType::kARC) {
                            if(((mode == device::AudioStereoMode::kPassThru) || (mode == device::AudioStereoMode::kStereo) || (mode == device::AudioStereoMode::kSurround)) && (stereoAuto == false)) {
                                aPort.setStereoAuto(false, persist);
                                aPort.setStereoMode(mode.toString(), persist);
                            }
                            else { //Auto Mode
                                aPort.setStereoAuto(stereoAuto, persist);
                            }
                        }else if (aPort.getType().getId() == device::AudioOutputPortType::kHDMI) {
                            if (!(mode == device::AudioStereoMode::kPassThru))
                            {
                                aPort.setStereoAuto(stereoAuto, persist);
                                LOGINFO("setting stereoAuto= %d  \n ",stereoAuto);
                            }
                            else
                            {
                                aPort.setStereoAuto(false, persist);
                            }
                            LOGINFO("setting sound mode = %s  \n ", mode.toString().c_str());
                            aPort.setStereoMode(mode.toString(), persist);
                        } else {
                            LOGERR("setSoundMode failed !! Device Not Connected...\n");
                            success = false;
                        }
		    }
                }
                else
                {
                    /* No videoDisplay is specified, setMode to all connected ports */
                    JsonObject params;
                    params["videoDisplay"] = "HDMI0";
                    params["soundMode"] = soundMode;
                    JsonObject unusedResponse;
                    setSoundMode(params, response);
                    params["videoDisplay"] = "SPDIF0";
                    setSoundMode(params, unusedResponse);
                }
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
                success = false;
            }
            //TODO(MROLLINS) -- so this is interesting.  ServiceManager had a settingChanged event that I guess handled settings from many services.
            //Does that mean we need to save our setting back to another plugin that would own settings (and this settingsChanged event) ?
            //ServiceManager::getInstance()->saveSetting(this, SETTING_DISPLAY_SERVICE_SOUND_MODE, soundMode);

            returnResponse(success);
        }

        uint32_t DisplaySettings::readEDID(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response: {"EDID":"AP///////wBSYgYCAQEBAQEXAQOAoFp4CvCdo1VJmyYPR0ovzgCBgIvAAQEBAQEBAQEBAQEBAjqAGHE4LUBYLEUAQIRjAAAeZiFQsFEAGzBAcDYAQIRjAAAeAAAA/ABUT1NISUJBLVRWCiAgAAAA/QAXSw9EDwAKICAgICAgAbECAytxSpABAgMEBQYHICImCQcHEQcYgwEAAGwDDAAQADgtwBUVHx/jBQMBAR2AGHEcFiBYLCUAQIRjAACeAR0AclHQHiBuKFUAQIRjAAAejArQiiDgLRAQPpYAsIRDAAAYjAqgFFHwFgAmfEMAsIRDAACYAAAAAAAAAAAAAAAA9w=="
            //sample this thunder plugin    : {"EDID":"AP///////wBSYgYCAQEBAQEXAQOAoFp4CvCdo1VJmyYPR0ovzgCBgIvAAQEBAQEBAQEBAQEBAjqAGHE4LUBYLEUAQIRjAAAeZiFQsFEAGzBAcDYAQIRjAAAeAAAA/ABUT1NISUJBLVRWCiAgAAAA/QAXSw9EDwAKICAgICAgAbECAytxSpABAgMEBQYHICImCQcHEQcYgwEAAGwDDAAQADgtwBUVHx/jBQMBAR2AGHEcFiBYLCUAQIRjAACeAR0AclHQHiBuKFUAQIRjAAAejArQiiDgLRAQPpYAsIRDAAAYjAqgFFHwFgAmfEMAsIRDAACYAAAAAAAAAAAAAAAA9w"}
            LOGINFOMETHOD();

            bool success = true;
            vector<uint8_t> edidVec({'u','n','k','n','o','w','n' });
            try
            {
                vector<uint8_t> edidVec2;
                std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
                if (vPort.isDisplayConnected())
                {
                    vPort.getDisplay().getEDIDBytes(edidVec2);
                    edidVec = edidVec2;//edidVec must be "unknown" unless we successfully get to this line

                    //convert to base64
                    uint16_t size = min(edidVec.size(), (size_t)numeric_limits<uint16_t>::max());
                    if(edidVec.size() > (size_t)numeric_limits<uint16_t>::max())
                        LOGERR("Size too large to use ToString base64 wpe api");
                    string edidbase64;
                    Core::ToString((uint8_t*)&edidVec[0], size, true, edidbase64);
                    response["EDID"] = edidbase64;

                }
                else
                {
                    LOGWARN("failure: HDMI0 not connected!");
                    success = false;
                }
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
                success = false;
            }

            returnResponse(success);
        }

        uint32_t DisplaySettings::readHostEDID(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();

            vector<uint8_t> edidVec({'u','n','k','n','o','w','n' });
            try
            {
                vector<unsigned char> edidVec2;
                device::Host::getInstance().getHostEDID(edidVec2);
                edidVec = edidVec2;//edidVec must be "unknown" unless we successfully get to this line
                LOGINFO("getHostEDID size is %d.", int(edidVec2.size()));
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
            }
            //convert to base64
            string base64String;
            uint16_t size = min(edidVec.size(), (size_t)numeric_limits<uint16_t>::max());
            if(edidVec.size() > (size_t)numeric_limits<uint16_t>::max())
                LOGINFO("size too large to use ToString base64 wpe api");

            Core::ToString((uint8_t*)&edidVec[0], size, true, base64String);
            response["EDID"] = base64String;
            returnResponse(true);
        }

        uint32_t DisplaySettings::getActiveInput(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();

            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
            string videoDisplay = parameters.HasLabel("videoDisplay") ? parameters["videoDisplay"].String() : strVideoPort;
            bool active = true;
            try
            {
                device::VideoOutputPort &vPort = device::Host::getInstance().getVideoOutputPort(videoDisplay);
                active = (vPort.isDisplayConnected() && vPort.isActive());
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(videoDisplay);
                response["activeInput"] = JsonValue(false);
                returnResponse(false);
            }
            response["activeInput"] = JsonValue(active);
            returnResponse(true);
        }

        uint32_t DisplaySettings::getTvHDRSupport(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:{"standards":["none"],"supportsHDR":false}
            LOGINFOMETHOD();

            JsonArray hdrCapabilities;
            int capabilities = dsHDRSTANDARD_NONE;

            try
            {
                std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                device::VideoOutputPort vPort = device::VideoOutputPortConfig::getInstance().getPort(strVideoPort.c_str());
                if (vPort.isDisplayConnected())
                    vPort.getTVHDRCapabilities(&capabilities);
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
            }

            if(!capabilities)hdrCapabilities.Add("none");
            if(capabilities & dsHDRSTANDARD_HDR10)hdrCapabilities.Add("HDR10");
            if(capabilities & dsHDRSTANDARD_HDR10PLUS)hdrCapabilities.Add("HDR10PLUS");
	    if(capabilities & dsHDRSTANDARD_HLG)hdrCapabilities.Add("HLG");
            if(capabilities & dsHDRSTANDARD_DolbyVision)hdrCapabilities.Add("Dolby Vision");
            if(capabilities & dsHDRSTANDARD_TechnicolorPrime)hdrCapabilities.Add("Technicolor Prime");
            if(capabilities & dsHDRSTANDARD_SDR)hdrCapabilities.Add("SDR");

            if(capabilities)
            {
                response["supportsHDR"] = true;
            }
            else
            {
                response["supportsHDR"] = false;
            }
            response["standards"] = hdrCapabilities;
            for (uint32_t i = 0; i < hdrCapabilities.Length(); i++)
            {
               LOGINFO("capabilities: %s", hdrCapabilities[i].String().c_str());
            }
            returnResponse(true);
        }

        uint32_t DisplaySettings::getSettopHDRSupport(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:{"standards":["HDR10"],"supportsHDR":true}
            LOGINFOMETHOD();

            JsonArray hdrCapabilities;
            int capabilities = dsHDRSTANDARD_NONE;

            try
            {
                if (device::Host::getInstance().getVideoDevices().size() < 1)
                {
                    LOGINFO("DSMGR_NOT_RUNNING");
                    returnResponse(false);
                }

                device::VideoDevice &device = device::Host::getInstance().getVideoDevices().at(0);
                device.getHDRCapabilities(&capabilities);
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
            }

            if(!capabilities)hdrCapabilities.Add("none");
            if(capabilities & dsHDRSTANDARD_HDR10)hdrCapabilities.Add("HDR10");
            if(capabilities & dsHDRSTANDARD_HDR10PLUS)hdrCapabilities.Add("HDR10PLUS");
	    if(capabilities & dsHDRSTANDARD_HLG)hdrCapabilities.Add("HLG");
            if(capabilities & dsHDRSTANDARD_DolbyVision)hdrCapabilities.Add("Dolby Vision");
            if(capabilities & dsHDRSTANDARD_TechnicolorPrime)hdrCapabilities.Add("Technicolor Prime");
            if(capabilities & dsHDRSTANDARD_SDR)hdrCapabilities.Add("SDR");

            if(capabilities)
            {
                response["supportsHDR"] = true;
            }
            else
            {
                response["supportsHDR"] = false;
            }
            response["standards"] = hdrCapabilities;
            for (uint32_t i = 0; i < hdrCapabilities.Length(); i++)
            {
               LOGINFO("capabilities: %s", hdrCapabilities[i].String().c_str());
            }
            returnResponse(true);
        }
        uint32_t DisplaySettings::getSettopAudioCapabilities(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:{"AudioCapabilities":["ATMOS","DOLBY DIGITAL","DOLBYDIGITAL PLUS","MS12"]}
            LOGINFOMETHOD();

            JsonArray audioCapabilities;
            int capabilities = dsAUDIOSUPPORT_NONE;

            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                aPort.getAudioCapabilities(&capabilities);
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
            }

            if(!capabilities)audioCapabilities.Add("none");
            if(capabilities & dsAUDIOSUPPORT_ATMOS)audioCapabilities.Add("ATMOS");
            if(capabilities & dsAUDIOSUPPORT_DD)audioCapabilities.Add("DOLBY DIGITAL");
            if(capabilities & dsAUDIOSUPPORT_DDPLUS)audioCapabilities.Add("DOLBY DIGITAL PLUS");
            if(capabilities & dsAUDIOSUPPORT_DAD)audioCapabilities.Add("Dual Audio Decode");
            if(capabilities & dsAUDIOSUPPORT_DAPv2)audioCapabilities.Add("DAPv2");
            if(capabilities & dsAUDIOSUPPORT_MS12)audioCapabilities.Add("MS12");

            response["AudioCapabilities"] = audioCapabilities;
            for (uint32_t i = 0; i < audioCapabilities.Length(); i++)
            {
               LOGINFO("capabilities: %s", audioCapabilities[i].String().c_str());
            }
            returnResponse(true);
        }

        uint32_t DisplaySettings::getSettopMS12Capabilities(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:{"MS12Capabilities":["Dolby Volume","Inteligent Equalizer","Dialogue Enhancer"]}
            LOGINFOMETHOD();

            JsonArray ms12Capabilities;
            int capabilities = dsMS12SUPPORT_NONE;
            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                aPort.getMS12Capabilities(&capabilities);
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
            }

            if(!capabilities)ms12Capabilities.Add("none");
            if(capabilities & dsMS12SUPPORT_DolbyVolume)ms12Capabilities.Add("Dolby Volume");
            if(capabilities & dsMS12SUPPORT_InteligentEqualizer)ms12Capabilities.Add("Inteligent Equalizer");
            if(capabilities & dsMS12SUPPORT_DialogueEnhancer)ms12Capabilities.Add("Dialogue Enhancer");

            response["MS12Capabilities"] = ms12Capabilities;
            for (uint32_t i = 0; i < ms12Capabilities.Length(); i++)
            {
               LOGINFO("capabilities: %s", ms12Capabilities[i].String().c_str());
            }
            returnResponse(true);
        }

        uint32_t DisplaySettings::setVideoPortStatusInStandby(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            returnIfParamNotFound(parameters, "portName");
            string portname = parameters["portName"].String();

            bool enabled = parameters["enabled"].Boolean();
            bool success = true;
            if(!m_isPwrMgr2RFCEnabled)
            {
                IARM_Bus_PWRMgr_StandbyVideoState_Param_t param;
                param.isEnabled = enabled;
                strncpy(param.port, portname.c_str(), PWRMGR_MAX_VIDEO_PORT_NAME_LENGTH);
                if(IARM_RESULT_SUCCESS != IARM_Bus_Call(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_API_SetStandbyVideoState, &param, sizeof(param)))
                {
                    LOGERR("Port: %s. enable: %d", param.port, param.isEnabled);
                    response["error_message"] = "Bus failure";
                    success = false;
                }
                else if(0 != param.result)
                {
                    LOGERR("Result %d. Port: %s. enable:%d", param.result, param.port, param.isEnabled);
                    response["error_message"] = "internal error";
                    success = false;
                }
            }
            else
            {
                dsMgrStandbyVideoStateParam_t param;
                param.isEnabled = enabled;
                strncpy(param.port, portname.c_str(), PWRMGR_MAX_VIDEO_PORT_NAME_LENGTH);
                if(IARM_RESULT_SUCCESS != IARM_Bus_Call(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_API_SetStandbyVideoState, &param, sizeof(param)))
                {
                    LOGERR("Port: %s. enable: %d", param.port, param.isEnabled);
                    response["error_message"] = "Bus failure";
                    success = false;
                }
                else if(0 != param.result)
                {
                    LOGERR("Result %d. Port: %s. enable:%d", param.result, param.port, param.isEnabled);
                    response["error_message"] = "internal error";
                    success = false;
                }
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::getVideoPortStatusInStandby(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            returnIfParamNotFound(parameters, "portName");
            string portname = parameters["portName"].String();

            bool success = true;
            if(!m_isPwrMgr2RFCEnabled)
            {
                IARM_Bus_PWRMgr_StandbyVideoState_Param_t param;
                strncpy(param.port, portname.c_str(), PWRMGR_MAX_VIDEO_PORT_NAME_LENGTH);
                if(IARM_RESULT_SUCCESS != IARM_Bus_Call(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_API_GetStandbyVideoState, &param, sizeof(param)))
                {
                    LOGERR("Port: %s. enable:%d", param.port, param.isEnabled);
                    response["error_message"] = "Bus failure";
                    success = false;
                }
                else if(0 != param.result)
                {
                    LOGERR("Result %d. Port: %s. enable:%d", param.result, param.port, param.isEnabled);
                    response["error_message"] = "internal error";
                    success = false;
                }
                else
                {
                    bool enabled(0 != param.isEnabled);
                    LOGINFO("video port is %s", enabled ? "enabled" : "disabled");
                    response["videoPortStatusInStandby"] = enabled;
                }
            }
            else
            {
                dsMgrStandbyVideoStateParam_t param;
                strncpy(param.port, portname.c_str(), PWRMGR_MAX_VIDEO_PORT_NAME_LENGTH);
                if(IARM_RESULT_SUCCESS != IARM_Bus_Call(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_API_GetStandbyVideoState, &param, sizeof(param)))
                {
                    LOGERR("Port: %s. enable:%d", param.port, param.isEnabled);
                    response["error_message"] = "Bus failure";
                    success = false;
                }
                else if(0 != param.result)
                {
                    LOGERR("Result %d. Port: %s. enable:%d", param.result, param.port, param.isEnabled);
                    response["error_message"] = "internal error";
                    success = false;
                }
                else
                {
                    bool enabled(0 != param.isEnabled);
                    LOGINFO("video port is %s", enabled ? "enabled" : "disabled");
                    response["videoPortStatusInStandby"] = enabled;
                }

            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::getCurrentOutputSettings(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success = true;
            try
            {
                std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
                if (vPort.isDisplayConnected())
                {
                    int videoEOTF, matrixCoefficients, colorSpace, colorDepth, quantizationRange;
                    vPort.getCurrentOutputSettings(videoEOTF, matrixCoefficients, colorSpace, colorDepth, quantizationRange);

                    response["colorSpace"] = colorSpace;
                    response["colorDepth"] = colorDepth;
                    response["matrixCoefficients"] = matrixCoefficients;
                    response["videoEOTF"] = videoEOTF;
                    response["quantizationRange"] = quantizationRange;
                }
                else
                {
                    LOGERR("HDMI0 not connected!");
                    success = false;
                }
            }
            catch (const device::Exception& err)
            {
                LOGINFO("caught an exception: %d, %s", err.getCode(), err.what());
                success = false;
            }

            LOGINFO("Leaving_ DisplaySettings::%s\n", __FUNCTION__);
            returnResponse(success);
        }

        uint32_t DisplaySettings::getVolumeLeveller(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();

                bool success = true;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                dsVolumeLeveller_t leveller;

                try
                {
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                                if (aPort.isConnected())
                                {
                                        leveller= aPort.getVolumeLeveller();
                                        response["enable"] = (leveller.mode ? true : false);
                                        response["level"] = leveller.level;
                                }
                }
                catch (const device::Exception& err)
                {
                        LOG_DEVICE_EXCEPTION1(audioPort);
                        success = false;
                        response["enable"] = false;
                        response["level"] = 0;
                }
                returnResponse(success);
        }

        uint32_t DisplaySettings::getVolumeLeveller2(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();

                bool success = true;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                dsVolumeLeveller_t leveller;

                try
                {
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                                if (aPort.isConnected())
                                {
                                        leveller= aPort.getVolumeLeveller();
                                        response["mode"] = leveller.mode;
                                        response["level"] = leveller.level;
                                }
                }
                catch (const device::Exception& err)
                {
                        LOG_DEVICE_EXCEPTION1(audioPort);
                        success = false;
                        response["mode"] = 0; //Off
                        response["level"] = 0;
                }
                returnResponse(success);
        }

        void DisplaySettings::audioFormatToString(dsAudioFormat_t audioFormat, JsonObject & response)
        {
            std::vector<string> supportedAudioFormat = {"NONE", "PCM", "AAC","VORBIS","WMA", "DOLBY AC3", "DOLBY EAC3",
                                                         "DOLBY AC4", "DOLBY MAT", "DOLBY TRUEHD",
                                                         "DOLBY EAC3 ATMOS","DOLBY TRUEHD ATMOS",
                                                         "DOLBY MAT ATMOS","DOLBY AC4 ATMOS","UNKNOWN"};
            switch (audioFormat)
            {
                   case dsAUDIO_FORMAT_NONE:
                       response["currentAudioFormat"] = "NONE";
                       break;
                   case dsAUDIO_FORMAT_PCM:
                       response["currentAudioFormat"] = "PCM";
                       break;
                   case dsAUDIO_FORMAT_AAC:
                       response["currentAudioFormat"] = "AAC";
                       break;
                   case dsAUDIO_FORMAT_VORBIS:
                       response["currentAudioFormat"] = "VORBIS";
                       break;
                   case dsAUDIO_FORMAT_WMA:
                       response["currentAudioFormat"] = "WMA";
                       break;
                   case dsAUDIO_FORMAT_DOLBY_AC3:
                       response["currentAudioFormat"] = "DOLBY AC3";
                       break;
                   case dsAUDIO_FORMAT_DOLBY_EAC3:
                       response["currentAudioFormat"] = "DOLBY EAC3";
                       break;
                   case dsAUDIO_FORMAT_DOLBY_AC4:
                       response["currentAudioFormat"] = "DOLBY AC4";
                       break;
                   case dsAUDIO_FORMAT_DOLBY_MAT:
                       response["currentAudioFormat"] = "DOLBY MAT";
                       break;
                   case dsAUDIO_FORMAT_DOLBY_TRUEHD:
                       response["currentAudioFormat"] = "DOLBY TRUEHD";
                       break;
                   case dsAUDIO_FORMAT_DOLBY_EAC3_ATMOS:
                       response["currentAudioFormat"] = "DOLBY EAC3 ATMOS";
                       break;
                   case dsAUDIO_FORMAT_DOLBY_TRUEHD_ATMOS:
                       response["currentAudioFormat"] = "DOLBY TRUEHD ATMOS";
                       break;
                   case dsAUDIO_FORMAT_DOLBY_MAT_ATMOS:
                       response["currentAudioFormat"] = "DOLBY MAT ATMOS";
                       break;
                   case dsAUDIO_FORMAT_DOLBY_AC4_ATMOS:
                       response["currentAudioFormat"] = "DOLBY AC4 ATMOS";
                       break;
                   default:
                       response["currentAudioFormat"] = "UNKNOWN";
                       break;
            }
            setResponseArray(response, "supportedAudioFormat", supportedAudioFormat);
	}

        uint32_t DisplaySettings::getAudioFormat(const JsonObject& parameters, JsonObject& response)
        {
             LOGINFOMETHOD();
	     bool success = true;
             dsAudioFormat_t audioFormat = dsAUDIO_FORMAT_NONE;
             try
             {
                 device::Host::getInstance().getCurrentAudioFormat(audioFormat);
                 LOGINFO("current audio format: %d \n", audioFormat);
                 audioFormatToString(audioFormat, response);
                 success = true;
             }
             catch (const device::Exception& err)
             {
                 LOG_DEVICE_EXCEPTION0();
		 success = false;
		 audioFormatToString(dsAUDIO_FORMAT_NONE, response);
             }
	     returnResponse(success);
        }

	void DisplaySettings::notifyAudioFormatChange(dsAudioFormat_t audioFormat)
	{
	     JsonObject params;
	     audioFormatToString(audioFormat, params);
             sendNotify("audioFormatChanged", params);
	}

    void DisplaySettings::notifyAtmosCapabilityChange(dsATMOSCapability_t atmosCaps)
    {
         JsonObject params;
         switch (atmosCaps) {
        case dsAUDIO_ATMOS_ATMOSMETADATA:
            params["currentAtmosCapability"] = "ATMOS_SUPPORTED";
            break;
        case dsAUDIO_ATMOS_NOTSUPPORTED:
            params["currentAtmosCapability"] = "ATMOS_NOT_SUPPORTED";
            break;
        default:
            LOGINFO("Atmos capability unknown, not notifying");
            break;
         }
             sendNotify("AtmosCapabilityChanged", params);
    }
	void DisplaySettings::notifyVideoFormatChange(dsHDRStandard_t videoFormat)
	{
            JsonObject params;
            params["currentVideoFormat"] = getVideoFormatTypeToString(videoFormat);

            params["supportedVideoFormat"] = getSupportedVideoFormats();
            sendNotify("videoFormatChanged", params);
        }

        void DisplaySettings::notifyAssociatedAudioMixingChange(bool mixing)
        {
             JsonObject params;
             params["mixing"] = mixing;
             sendNotify("associatedAudioMixingChanged", params);
        }

        void DisplaySettings::notifyFaderControlChange(bool mixerbalance)
        {
             JsonObject params;
             params["mixerBalance"] = mixerbalance;
             sendNotify("faderControlChanged", params);
        }

        void DisplaySettings::notifyPrimaryLanguageChange(std::string pLang)
        {
             JsonObject params;
             params["primaryLanguage"] = pLang;
             sendNotify("primaryLanguageChanged", params);
        }

        void DisplaySettings::notifySecondaryLanguageChange(std::string sLang)
        {
             JsonObject params;
             params["secondaryLanguage"] = sLang;
             sendNotify("secondaryLanguageChanged", params);
        }

        uint32_t DisplaySettings::getBassEnhancer(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();
                bool success = true;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                int boost = 0;
                try
                {
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                                if (aPort.isConnected())
                                {
                                        boost = aPort.getBassEnhancer();
                                        response["enable"] = boost ? true : false ;
                                        response["bassBoost"] = boost;
                                }
                                else
                                {
                                        LOGERR("aport is not connected!");
                                        success = false;
                                }
                }
                catch (const device::Exception& err)
                {
                        LOG_DEVICE_EXCEPTION1(audioPort);
                        success = false;
                        response["enable"] = false;
                }
                returnResponse(success);
        }

        uint32_t DisplaySettings::isSurroundDecoderEnabled(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();
                bool success = true;
                bool surroundDecoderEnable = false;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";

                try
                {
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                                if (aPort.isConnected())
                                {
                                        surroundDecoderEnable = aPort.isSurroundDecoderEnabled();
                                        response["surroundDecoderEnable"] = surroundDecoderEnable;
                                }
                                else
                                {
                                        LOGERR("aport is not connected!");
                                        success = false;
                                }
                }
                catch (const device::Exception& err)
                {
                        LOG_DEVICE_EXCEPTION1(audioPort);
                        success = false;
                }
                returnResponse(success);
        }

        uint32_t DisplaySettings::getGain (const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool success = true;
            float gain = 0;

            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                gain = aPort.getGain();
                response["gain"] = to_string(gain);
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(audioPort);
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::getMuted (const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool success = true;
            bool muted = false;

            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                muted = aPort.isMuted();
                response["muted"] = muted;
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(string(audioPort));
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::getVolumeLevel (const JsonObject& parameters, JsonObject& response)
        {
            //LOGINFOMETHOD();
            bool success = true;
            float level = 0;

            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                if( audioPort != "HDMI_ARC0")
                {
                     device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                     level = aPort.getLevel();
                }
                else
                {
                    level = hdmiArcVolumeLevel;
                }
                response["volumeLevel"] = to_string(level);
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(audioPort);
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::getDRCMode(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();
                bool success = true;
                int mode = 0;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                try
                {
                       device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                                if (aPort.isConnected())
                                {
                                        mode = aPort.getDRCMode();
                                        response["DRCMode"] = mode ? "RF" : "line" ;
                                }
                                else
                                {
                                        LOGERR("aport is not connected!");
                                        success = false;
                                }
                }
                catch (const device::Exception& err)
                {
                        LOG_DEVICE_EXCEPTION1(audioPort);
                        success = false;
                }
                returnResponse(success);
        }

        uint32_t DisplaySettings::getSurroundVirtualizer(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();
                bool success = true;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                dsSurroundVirtualizer_t virtualizer;

                try
                {
                       device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                                if (aPort.isConnected())
                                {
                                        virtualizer = aPort.getSurroundVirtualizer();
                                        response["enable"] = virtualizer.mode ? true : false ;
                                        response["boost"] = virtualizer.boost;
                                }
                                else
                                {
                                        LOGERR("aport is not connected!");
                                        success = false;
                                }
                }
                catch (const device::Exception& err)
                {
                        LOG_DEVICE_EXCEPTION1(audioPort);
                        success = false;
                }
                returnResponse(success);
        }

        uint32_t DisplaySettings::getSurroundVirtualizer2(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();
                bool success = true;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                dsSurroundVirtualizer_t virtualizer;

                try
                {
                       device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                                if (aPort.isConnected())
                                {
                                        virtualizer = aPort.getSurroundVirtualizer();
                                        response["mode"] = virtualizer.mode;
                                        response["boost"] = virtualizer.boost;
                                }
                                else
                                {
                                        LOGERR("aport is not connected!");
                                        success = false;
                                }
                }
                catch (const device::Exception& err)
                {
                        LOG_DEVICE_EXCEPTION1(audioPort);
                        success = false;
                        response["mode"] = 0; //Off
                        response["boost"] = 0;

                }
                returnResponse(success);
        }
	
        uint32_t DisplaySettings::getMISteering(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();
                bool success = true;
                bool enable = false;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                try
                {
                       device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                                if (aPort.isConnected())
                                {
                                        enable = aPort.getMISteering();
                                        response["MISteeringEnable"] = enable;
                                }
                                else
                                {
                                        LOGERR("aport is not connected!");
                                        success = false;
                                }
                }
                catch (const device::Exception& err)
                {
                        LOG_DEVICE_EXCEPTION1(audioPort);
                        success = false;
                }
                returnResponse(success);
        }

        uint32_t DisplaySettings::setVolumeLeveller(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();
                returnIfParamNotFound(parameters, "level");
                string sVolumeLeveller = parameters["level"].String();
                dsVolumeLeveller_t VolumeLeveller;
                bool isIntiger = Utils::isValidUnsignedInt ((char*)sVolumeLeveller.c_str());
                if (false == isIntiger) {
                    LOGWARN("level should be an unsigned integer");
                    returnResponse(false);
                }

                try {
                    VolumeLeveller.level = stoi(sVolumeLeveller);
                    if(VolumeLeveller.level == 0) {
                        VolumeLeveller.mode = 0; //Off
                    }
                    else {
                        VolumeLeveller.mode = 1; //On
                    }
                }catch (const device::Exception& err) {
                        LOG_DEVICE_EXCEPTION1(sVolumeLeveller);
                        returnResponse(false);
                }
                bool success = true;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                try
                {
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                        aPort.setVolumeLeveller(VolumeLeveller);
                }
                catch (const device::Exception& err)
                {
                        LOG_DEVICE_EXCEPTION2(audioPort, sVolumeLeveller);
                        success = false;
                }
                returnResponse(success);
        }

        uint32_t DisplaySettings::setVolumeLeveller2(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();
                returnIfParamNotFound(parameters, "mode");
		string sMode = parameters["mode"].String();
                string sLevel = parameters["level"].String();
                dsVolumeLeveller_t volumeLeveller;
                if ((Utils::isValidUnsignedInt ((char*)sMode.c_str()) == false) || (Utils::isValidUnsignedInt ((char*)sMode.c_str()) == false)) {
                    LOGWARN("mode and level should be an unsigned integer");
                    returnResponse(false);
                }

                try {
                        int mode = stoi(sMode);
                        if (mode == 0) {
                                volumeLeveller.mode = 0; //Off
				volumeLeveller.level = 0;
                        }
                        else if (mode == 1){
                                volumeLeveller.mode = 1; //On
				volumeLeveller.level = stoi(sLevel);
                        }
			else if (mode == 2) {
				volumeLeveller.mode = 2; //Auto
				volumeLeveller.level = 0;
			}
			else {
				LOGERR("Invalid volume leveller mode \n");
				returnResponse(false);
			}
                }catch (const device::Exception& err) {
                        LOG_DEVICE_EXCEPTION1(sMode);
                        returnResponse(false);
                }
                bool success = true;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                try
                {
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                        aPort.setVolumeLeveller(volumeLeveller);
                }
                catch (const device::Exception& err)
                {
                        LOG_DEVICE_EXCEPTION2(audioPort, sMode);
                        success = false;
                }
                returnResponse(success);
        }


        uint32_t DisplaySettings::enableSurroundDecoder(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();
                returnIfParamNotFound(parameters, "surroundDecoderEnable");
                string sEnableSurroundDecoder = parameters["surroundDecoderEnable"].String();
                bool enableSurroundDecoder = false;
                try {
                        enableSurroundDecoder= parameters["surroundDecoderEnable"].Boolean();
                }catch (const device::Exception& err) {
                        LOG_DEVICE_EXCEPTION1(sEnableSurroundDecoder);
                        returnResponse(false);
                }
                bool success = true;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                try
                {
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                        aPort.enableSurroundDecoder(enableSurroundDecoder);
                }
                catch (const device::Exception& err)
                {
                        LOG_DEVICE_EXCEPTION2(audioPort, sEnableSurroundDecoder);
                        success = false;
                }
                returnResponse(success);
        }

        uint32_t DisplaySettings::setBassEnhancer(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();
                returnIfParamNotFound(parameters, "bassBoost");
                string sBassBoost = parameters["bassBoost"].String();
                int bassBoost = 0;
                bool isIntiger = Utils::isValidUnsignedInt ((char*)sBassBoost.c_str());
                if (false == isIntiger) {
                    LOGWARN("bassBoost should be an unsigned integer");
                    returnResponse(false);
                }
                try {
                        bassBoost = stoi(sBassBoost);
                }catch (const device::Exception& err) {
                        LOG_DEVICE_EXCEPTION1(sBassBoost);
                        returnResponse(false);
                }
                bool success = true;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                try
                {
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                        aPort.setBassEnhancer(bassBoost);
                }
                catch (const device::Exception& err)
                {
                        LOG_DEVICE_EXCEPTION2(audioPort, sBassBoost);
                        success = false;
                }
                returnResponse(success);
        }

        uint32_t DisplaySettings::setSurroundVirtualizer(const JsonObject& parameters, JsonObject& response)
        {
               LOGINFOMETHOD();
               returnIfParamNotFound(parameters, "boost");
               string sSurroundVirtualizer = parameters["boost"].String();
               dsSurroundVirtualizer_t surroundVirtualizer;
               bool isIntiger = Utils::isValidUnsignedInt ((char*)sSurroundVirtualizer.c_str());
               if (false == isIntiger) {
                   LOGWARN("boost should be an unsigned integer");
                   returnResponse(false);
               }

               try {
                  surroundVirtualizer.boost = stoi(sSurroundVirtualizer);
		  if(surroundVirtualizer.boost == 0) {
			  surroundVirtualizer.mode = 0; //Off
		  }
		  else {
			  surroundVirtualizer.mode = 1; //On
		  }
               }catch (const device::Exception& err) {
                  LOG_DEVICE_EXCEPTION1(sSurroundVirtualizer);
                              returnResponse(false);
               }
               bool success = true;
               string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
               try
               {
                   device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                   aPort.setSurroundVirtualizer(surroundVirtualizer);
               }
               catch (const device::Exception& err)
               {
                   LOG_DEVICE_EXCEPTION2(audioPort, sSurroundVirtualizer);
                   success = false;
               }
               returnResponse(success);
        }

        uint32_t DisplaySettings::setSurroundVirtualizer2(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();
                returnIfParamNotFound(parameters, "mode");
                string sMode = parameters["mode"].String();
                string sBoost = parameters["boost"].String();
                dsSurroundVirtualizer_t surroundVirtualizer;

                if ((Utils::isValidUnsignedInt ((char*)sMode.c_str()) == false) || (Utils::isValidUnsignedInt ((char*)sBoost.c_str()) == false)) {
                    LOGWARN("mode and boost value should be an unsigned integer");
                    returnResponse(false);
                }

                try {
                        int mode = stoi(sMode);
                        if (mode == 0) {
                                surroundVirtualizer.mode = 0; //Off
                                surroundVirtualizer.boost = 0;
                        }
                        else if (mode == 1){
                                surroundVirtualizer.mode = 1; //On
                                surroundVirtualizer.boost = stoi(sBoost);
                        }
                        else if (mode == 2) {
                                surroundVirtualizer.mode = 2; //Auto
                                surroundVirtualizer.boost = 0;
                        }
                        else {
                                LOGERR("Invalid surround virtualizer mode \n");
                                returnResponse(false);
                        }
                }catch (const device::Exception& err) {
                        LOG_DEVICE_EXCEPTION1(sMode);
                        returnResponse(false);
                }
                bool success = true;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                try
                {
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                        aPort.setSurroundVirtualizer(surroundVirtualizer);
                }
                catch (const device::Exception& err)
                {
                        LOG_DEVICE_EXCEPTION2(audioPort, sMode);
                        success = false;
                }
                returnResponse(success);
        }

        uint32_t DisplaySettings::setMISteering(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();
                returnIfParamNotFound(parameters, "MISteeringEnable");
                string sMISteering = parameters["MISteeringEbnable"].String();
                bool MISteering = false;
                try {
                        MISteering = parameters["MISteeringEnable"].Boolean();
                }catch (const device::Exception& err) {
                        LOG_DEVICE_EXCEPTION1(sMISteering);
                        returnResponse(false);
                }
                bool success = true;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                try
                {
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                        aPort.setMISteering(MISteering);
                }
                catch (const device::Exception& err)
                {
                        LOG_DEVICE_EXCEPTION2(audioPort, sMISteering);
                        success = false;
                }
                returnResponse(success)
        }

        uint32_t DisplaySettings::setGain(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();
                returnIfParamNotFound(parameters, "gain");
                string sGain = parameters["gain"].String();
                float newGain = 0;
                try {
                        newGain = stof(sGain);
                        if ((newGain < -2080) || (newGain > 480)) {
                            LOGERR("Gain value being set to an invalid value newGain: %f \n",newGain);
                            returnResponse(false);
                        }
                }catch (const device::Exception& err) {
                        LOG_DEVICE_EXCEPTION1(sGain);
                        returnResponse(false);
                }
                bool success = true;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                try
                {
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                        aPort.setGain(newGain);
                        success= true;
                }
                catch (const device::Exception& err)
                {
                        LOG_DEVICE_EXCEPTION2(audioPort, sGain);
                        success = false;
                }
                returnResponse(success);
        }

        uint32_t DisplaySettings::setMuted (const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();
                returnIfParamNotFound(parameters, "muted");
                string sMuted = parameters["muted"].String();
                bool muted = false;
                static bool cache_muted = false;
                try {
                        muted = parameters["muted"].Boolean();
                }catch (const device::Exception& err) {
                        LOG_DEVICE_EXCEPTION1(sMuted);
                        returnResponse(false);
                }

                bool success = true;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                LOGWARN("DisplaySettings::setMuted called Audio Port :%s muted:%d\n", audioPort.c_str(), muted);
                try
                {
                    device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                    aPort.setMuted(muted);
                    if(cache_muted != muted)
                    {
                        cache_muted = muted;
                        JsonObject params;
                        params["muted"] = muted;
                        sendNotify("muteStatusChanged", params);
                    }
                }
                catch (const device::Exception& err)
                {
                    LOG_DEVICE_EXCEPTION2(audioPort, sMuted);
                    success = false;
                }
                returnResponse(success);
        }
	#define KEY_PRESSES_AT_ONCE 5
        void DisplaySettings::setVolumeThread( int level)
        {
            LOGINFO("Entry  %s:  level: %d hdmiArcVolumeLevel:%d \n",__FUNCTION__,level,hdmiArcVolumeLevel);
	    do
	    {
                JsonObject hdmiCecSinkResult;
                JsonObject params;
		// 5 is logical address for audio system
                params["logicalAddress"] = 5;
		// key code for volume up and down key event
                params["keyCode"] =(level > hdmiArcVolumeLevel)?0x41:0x42;
		int loopCount  = abs(level-hdmiArcVolumeLevel);
                loopCount=(loopCount>KEY_PRESSES_AT_ONCE)?KEY_PRESSES_AT_ONCE:loopCount;

                for(int i =0; i< loopCount;i++  )
	        {
                    _instance->m_client->Invoke<JsonObject, JsonObject>(2000, "sendKeyPressEvent", params, hdmiCecSinkResult);
                    if (!hdmiCecSinkResult["success"].Boolean()) {
                        LOGERR("%s HdmiCecSink Plugin sendKeyPressEvent returned error\n",__FUNCTION__);
                    }
	        }
		sleep(3);
                LOGINFO("After iteration %s:  level: %d hdmiArcVolumeLevel:%d loopCount:%d \n",__FUNCTION__,level,hdmiArcVolumeLevel,loopCount);
	    }while( abs(level-hdmiArcVolumeLevel) > KEY_PRESSES_AT_ONCE/2 );
            LOGINFO("Exit  %s:  level: %d hdmiArcVolumeLevel:%d \n",__FUNCTION__,level,hdmiArcVolumeLevel);
        }

        uint32_t DisplaySettings::setVolumeLevel(const JsonObject& parameters, JsonObject& response)
        {
                //LOGINFOMETHOD();
                returnIfParamNotFound(parameters, "volumeLevel");
                string sLevel = parameters["volumeLevel"].String();
                float level = 0;
                int cache_volumelevel = 0;
                try {
                        level = stof(sLevel);
                }catch (const device::Exception& err) {
                        LOG_DEVICE_EXCEPTION1(sLevel);
                        returnResponse(false);
                }

                bool success = true;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                try
                {
                    if( audioPort != "HDMI_ARC0")
                    {
                         device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                         aPort.setLevel(level);
                    }
                    else
                    {
                        std::thread t_setVolThread = std::thread(setVolumeThread,(int)level);
                        t_setVolThread.detach();
                    }
                    if(cache_volumelevel != (int)level)
                    {
                        cache_volumelevel = (int)level;
                        JsonObject params;
                        params["volumeLevel"] = (int)level;
                        sendNotify("volumeLevelChanged", params);
                    }
                    success= true;
                }
                catch (const device::Exception& err)
                {
                        LOG_DEVICE_EXCEPTION2(audioPort, sLevel);
                        success = false;
                }
                returnResponse(success);
        }

        uint32_t DisplaySettings::setDRCMode(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();
                returnIfParamNotFound(parameters, "DRCMode");
                string sDRCMode = parameters["DRCMode"].String();
                int DRCMode = 0;
                bool isIntiger = Utils::isValidUnsignedInt ((char*)sDRCMode.c_str());
                if (false == isIntiger) {
                    LOGWARN("DRCMode should be an unsigned integer");
                    returnResponse(false);
                }
                try {
                        DRCMode = stoi(sDRCMode);
                }catch (const device::Exception& err) {
                        LOG_DEVICE_EXCEPTION1(sDRCMode);
                        returnResponse(false);
                }
                bool success = true;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                try
                {
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                        aPort.setDRCMode(DRCMode);
                }
                catch (const device::Exception& err)
                {
                        LOG_DEVICE_EXCEPTION2(audioPort, sDRCMode);
                        success = false;
                }
                returnResponse(success);
        }

        uint32_t DisplaySettings::setMS12AudioCompression (const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();
            returnIfParamNotFound(parameters, "compresionLevel");

            string sCompresionLevel = parameters["compresionLevel"].String();
                       int compresionLevel = 0;
            try {
                compresionLevel = stoi(sCompresionLevel);
            }catch (const std::exception &err) {
               LOGERR("Failed to parse compresionLevel '%s'", sCompresionLevel.c_str());
                          returnResponse(false);
            }

            bool success = true;
            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                aPort.setCompression (compresionLevel);
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION2(audioPort, sCompresionLevel);
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::getMS12AudioCompression (const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();
                       bool success = true;
                       int compressionlevel = 0;

            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                compressionlevel = aPort.getCompression();
                response["compressionlevel"] = compressionlevel;
                               response["enable"] = (compressionlevel ? true : false);
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(audioPort);
                response["compressionlevel"] = 0;
                               response["enable"] = false;
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::setDolbyVolumeMode (const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();
            returnIfParamNotFound(parameters, "dolbyVolumeMode");

            string sDolbyVolumeMode = parameters["dolbyVolumeMode"].String();
            bool dolbyVolumeMode = false;

            try
            {
                dolbyVolumeMode = parameters["dolbyVolumeMode"].Boolean();
            }
            catch (const std::exception &err)
            {
               LOGERR("Failed to parse dolbyVolumeMode '%s'", sDolbyVolumeMode.c_str());
               returnResponse(false);
            }

            bool success = true;
            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                aPort.setDolbyVolumeMode (dolbyVolumeMode);
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION2(audioPort, sDolbyVolumeMode);
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::getDolbyVolumeMode (const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();
                       bool success = true;

            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                response["dolbyVolumeMode"] = aPort.getDolbyVolumeMode();
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(audioPort);
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::setDialogEnhancement (const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();
            returnIfParamNotFound(parameters, "enhancerlevel");

            string sEnhancerlevel = parameters["enhancerlevel"].String();
                       int enhancerlevel = 0;
            try {
                enhancerlevel = stoi(sEnhancerlevel);
            }catch (const std::exception &err) {
               LOGERR("Failed to parse enhancerlevel '%s'", sEnhancerlevel.c_str());
                          returnResponse(false);
            }

            bool success = true;
            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                aPort.setDialogEnhancement (enhancerlevel);
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION2(audioPort, sEnhancerlevel);
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::getDialogEnhancement (const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();
                       bool success = true;
                       int enhancerlevel = 0;

            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                enhancerlevel = aPort.getDialogEnhancement();
                response["enable"] = (enhancerlevel ? true : false);
                response["enhancerlevel"] = enhancerlevel;
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(string(audioPort));
                response["enable"] = false;
                response["enhancerlevel"] = 0;
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::setIntelligentEqualizerMode (const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();
            returnIfParamNotFound(parameters, "intelligentEqualizerMode");

            string sIntelligentEqualizerMode = parameters["intelligentEqualizerMode"].String();
                       int intelligentEqualizerMode = 0;
            try {
                intelligentEqualizerMode = stoi(sIntelligentEqualizerMode);
            }catch (const std::exception &err) {
               LOGERR("Failed to parse intelligentEqualizerMode '%s'", sIntelligentEqualizerMode.c_str());
                          returnResponse(false);
            }

            bool success = true;
            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                aPort.setIntelligentEqualizerMode (intelligentEqualizerMode);
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION2(audioPort, sIntelligentEqualizerMode);
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::getIntelligentEqualizerMode (const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();
                       bool success = true;
                       int intelligentEqualizerMode = 0;

            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                intelligentEqualizerMode = aPort.getIntelligentEqualizerMode ();
                response["enable"] = (intelligentEqualizerMode ? true : false);
                response["mode"] = intelligentEqualizerMode;
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(audioPort);
                response["enable"] = false;
                response["mode"] = 0;
                success = false;
            }
            returnResponse(success);
        }


        uint32_t DisplaySettings::setGraphicEqualizerMode (const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();
            returnIfParamNotFound(parameters, "graphicEqualizerMode");

            string sGraphicEqualizerMode = parameters["graphicEqualizerMode"].String();
                       int graphicEqualizerMode = 0;
            try {
                graphicEqualizerMode = stoi(sGraphicEqualizerMode);
            }catch (const std::exception &err) {
               LOGERR("Failed to parse graphicEqualizerMode '%s'", sGraphicEqualizerMode.c_str());
                          returnResponse(false);
            }

            bool success = true;
            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                aPort.setGraphicEqualizerMode (graphicEqualizerMode);
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION2(audioPort, sGraphicEqualizerMode);
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::getGraphicEqualizerMode (const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();
                       bool success = true;
                       int graphicEqualizerMode = 0;

            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                graphicEqualizerMode = aPort.getGraphicEqualizerMode ();
                response["enable"] = (graphicEqualizerMode ? true : false);
                response["mode"] = graphicEqualizerMode;
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(audioPort);
                response["enable"] = false;
                response["mode"] = 0;
                success = false;
            }
            returnResponse(success);
        }


        uint32_t DisplaySettings::setMS12AudioProfile (const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success = true;

            returnIfParamNotFound(parameters, "ms12AudioProfile");
            string audioProfileName = parameters["ms12AudioProfile"].String();

            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                aPort.setMS12AudioProfile(audioProfileName);
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION2(audioPort, audioProfileName);
                success = false;
            }

	    returnResponse(success);
        }

        uint32_t DisplaySettings::setMS12ProfileSettingsOverride(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool success = true;

            returnIfParamNotFound(parameters, "operation");
            string audioProfileState = parameters["operation"].String();

            returnIfParamNotFound(parameters, "profileName");
            string audioProfileName = parameters["profileName"].String();

            returnIfParamNotFound(parameters, "ms12SettingsName");
            string audioProfileSettingsName = parameters["ms12SettingsName"].String();

            returnIfParamNotFound(parameters, "ms12SettingsValue");
            string audioProfileSettingValue = parameters["ms12SettingsValue"].String();


            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                aPort.setMS12AudioProfileSetttingsOverride(audioProfileState,audioProfileName,audioProfileSettingsName, audioProfileSettingValue);
            }
            catch (const device::Exception& err)
            {
                success = false;
            }

            returnResponse(success);
        }


        uint32_t DisplaySettings::getMS12AudioProfile (const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool success = true;

	    string audioProfileName;
            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                audioProfileName = aPort.getMS12AudioProfile();
                response["ms12AudioProfile"] = audioProfileName;
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(audioPort);
                response["ms12AudioProfile"] = "None";
                success = false;
            }
            returnResponse(success);
        }


        uint32_t DisplaySettings::getSupportedMS12AudioProfiles(const JsonObject& parameters, JsonObject& response)
        {   //sample response: {"success":true,"supportedMS12AudioProfiles":["Off","Music","Movie","Game","Voice","Night","User"]}
            LOGINFOMETHOD();
            vector<string> supportedProfiles;
	    string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
		device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
		supportedProfiles = aPort.getMS12AudioProfileList();
                for (size_t i = 0; i < supportedProfiles.size(); i++)
                {
		    LOGINFO("Profile[%d]:  %s\n",i,supportedProfiles.at(i).c_str());
                }
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
            }
            setResponseArray(response, "supportedMS12AudioProfiles", supportedProfiles);
            returnResponse(true);
        }


        uint32_t DisplaySettings::setAssociatedAudioMixing(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();
                returnIfParamNotFound(parameters, "mixing");
                string sMixing = parameters["mixing"].String();
                bool mixing = false;
                try {
                        mixing = parameters["mixing"].Boolean();
                }catch (const device::Exception& err) {
                        LOG_DEVICE_EXCEPTION1(sMixing);
                        returnResponse(false);
                }
                bool success = true;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                try
                {
                    if (device::Host::getInstance().isHDMIOutPortPresent())
                    {
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                        aPort.setAssociatedAudioMixing(mixing);
                    }
                    else {
                        device::Host::getInstance().setAssociatedAudioMixing(mixing);
                    }
                }
                catch (const device::Exception& err)
                {
                        LOG_DEVICE_EXCEPTION2(audioPort, sMixing);
                        success = false;
                }
                returnResponse(success)
        }



        uint32_t DisplaySettings::getAssociatedAudioMixing(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();
                bool success = true;
                bool mixing = false;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                try
                {
                    if (device::Host::getInstance().isHDMIOutPortPresent())
                    {
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                        aPort.getAssociatedAudioMixing(&mixing);
                    }
                    else {
                        device::Host::getInstance().getAssociatedAudioMixing(&mixing);
                    }
                    response["mixing"] = mixing;
                }
                catch (const device::Exception& err)
                {
                    LOG_DEVICE_EXCEPTION1(audioPort);
                    success = false;
                }
                returnResponse(success);
        }

        uint32_t DisplaySettings::setFaderControl(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();
                returnIfParamNotFound(parameters, "mixerBalance");
                string sMixerBalance = parameters["mixerBalance"].String();
                int mixerBalance = 0;
                bool isIntiger = Utils::isValidInt ((char*)sMixerBalance.c_str());
                if (false == isIntiger) {
                    LOGWARN("mixerBalance should be an integer");
                    returnResponse(false);
                }
                try {
                        mixerBalance = stoi(sMixerBalance);
                }catch (const device::Exception& err) {
                        LOG_DEVICE_EXCEPTION1(sMixerBalance);
                        returnResponse(false);
                }
                bool success = true;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                try
                {
                    if (device::Host::getInstance().isHDMIOutPortPresent())
                    {
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                        aPort.setFaderControl(mixerBalance);
                    }
                    else {
                        device::Host::getInstance().setFaderControl(mixerBalance);
                    }
                }
                catch (const device::Exception& err)
                {
                        LOG_DEVICE_EXCEPTION2(audioPort, sMixerBalance);
                        success = false;
                }
                returnResponse(success);
        }


        uint32_t DisplaySettings::getFaderControl(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();
                bool success = true;
                string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
                int mixerBalance = 0;
                try
                {
                    if (device::Host::getInstance().isHDMIOutPortPresent())
                    {
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                        aPort.getFaderControl(&mixerBalance);
                    }
                    else {
                        device::Host::getInstance().getFaderControl(&mixerBalance);
                    }
                    response["mixerBalance"] = mixerBalance;
                }
                catch (const device::Exception& err)
                {
                        LOG_DEVICE_EXCEPTION1(audioPort);
                        success = false;
                }
                returnResponse(success);
        }

        uint32_t DisplaySettings::setPrimaryLanguage (const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success = true;

            returnIfParamNotFound(parameters, "lang");
            string primaryLanguage = parameters["lang"].String();

            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                if (device::Host::getInstance().isHDMIOutPortPresent())
                {
                    device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                    aPort.setPrimaryLanguage(primaryLanguage);
		}
                else {
                    device::Host::getInstance().setPrimaryLanguage(primaryLanguage);
                }
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION2(audioPort, primaryLanguage);
                success = false;
            }

            returnResponse(success);
        }


        uint32_t DisplaySettings::getPrimaryLanguage (const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool success = true;

            string primaryLanguage;
            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                if (device::Host::getInstance().isHDMIOutPortPresent())
                {
                    device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                    aPort.getPrimaryLanguage(primaryLanguage);
                }
                else {
                    device::Host::getInstance().getPrimaryLanguage(primaryLanguage);
                }
                response["lang"] = primaryLanguage;
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(audioPort);
                response["lang"] = "None";
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::setSecondaryLanguage (const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success = true;

            returnIfParamNotFound(parameters, "lang");
            string secondaryLanguage = parameters["lang"].String();

            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                if (device::Host::getInstance().isHDMIOutPortPresent())
                {
                    device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                    aPort.setSecondaryLanguage(secondaryLanguage);
                }
                else {
                    device::Host::getInstance().setSecondaryLanguage(secondaryLanguage);
                }
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION2(audioPort, secondaryLanguage);
                success = false;
            }

            returnResponse(success);
        }


        uint32_t DisplaySettings::getSecondaryLanguage (const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool success = true;

            string secondaryLanguage;
            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                if (device::Host::getInstance().isHDMIOutPortPresent())
                {
                    device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                    aPort.getSecondaryLanguage(secondaryLanguage);
                }
                else {
                    device::Host::getInstance().getSecondaryLanguage(secondaryLanguage);
                }
                response["lang"] = secondaryLanguage;
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(audioPort);
                response["lang"] = "None";
                success = false;
            }
            returnResponse(success);
        }


        uint32_t DisplaySettings::getAudioDelay (const JsonObject& parameters, JsonObject& response) 
        {   //sample servicemanager response:
            LOGINFOMETHOD();
            bool success = true;
            string audioPort = parameters["audioPort"].String();//empty value will browse all ports

            if (!checkPortName(audioPort))
                audioPort = "HDMI0";

            uint32_t audioDelayMs = 0;
            try
            {
                /* Return the sound mode of the audio ouput connected to the specified audioPort */
                /* Check if HDMI is connected - Return (default) Stereo Mode if not connected */
                if (audioPort.empty())
                {
                    std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                    if (device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str()).isDisplayConnected())
                    {
                        audioPort = "HDMI0";
                    }
                    else
                    {
                        /*  * If HDMI is not connected
                            * Get the SPDIF if it is supported by platform
                            * If Platform does not have connected ports. Default to HDMI.
                        */
                        audioPort = "HDMI0";
                        device::List<device::VideoOutputPort> vPorts = device::Host::getInstance().getVideoOutputPorts();
                        for (size_t i = 0; i < vPorts.size(); i++)
                        {
                            device::VideoOutputPort &vPort = vPorts.at(i);
                            if (vPort.isDisplayConnected())
                            {
                                audioPort = "SPDIF0";
                                break;
                            }
                        }
                    }
                }

                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                aPort.getAudioDelay(audioDelayMs);
				response["audioDelay"] = std::to_string(audioDelayMs);
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(audioPort);
                success = false;
            }

            returnResponse(success);
        }

        uint32_t DisplaySettings::setAudioDelay (const JsonObject& parameters, JsonObject& response) 
        {   //sample servicemanager response:
            LOGINFOMETHOD();

			returnIfParamNotFound(parameters, "audioDelay");

            string sAudioDelayMs = parameters["audioDelay"].String();
			int audioDelayMs = 0;
            try {
                audioDelayMs = stoi(sAudioDelayMs);
            } catch (const std::exception &err) {
                LOGERR("Failed to parse audioDelay '%s'", sAudioDelayMs.c_str());
                returnResponse(false);
            }
            
            if ( audioDelayMs < 0 )
            {
                LOGERR("audioDelay '%s', Should be a postiive value", sAudioDelayMs.c_str());
                returnResponse(false);
            }

            bool success = true;
            string audioPort = parameters["audioPort"].String();//empty value will browse all ports

            if (!checkPortName(audioPort))
                audioPort = "HDMI0";

            try
            {
                /* Return the sound mode of the audio ouput connected to the specified audioPort */
                /* Check if HDMI is connected - Return (default) Stereo Mode if not connected */
                if (audioPort.empty())
                {
                    std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                    if (device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str()).isDisplayConnected())
                        audioPort = "HDMI0";
                    else
                    {
                        /*  * If HDMI is not connected
                            * Get the SPDIF if it is supported by platform
                            * If Platform does not have connected ports. Default to HDMI.
                        */
                        audioPort = "HDMI0";
                        device::List<device::VideoOutputPort> vPorts = device::Host::getInstance().getVideoOutputPorts();
                        for (size_t i = 0; i < vPorts.size(); i++)
                        {
                            device::VideoOutputPort &vPort = vPorts.at(i);
                            if (vPort.isDisplayConnected())
                            {
                                audioPort = "SPDIF0";
                                break;
                            }
                        }
                    }
                }

                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                aPort.setAudioDelay (audioDelayMs);
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION2(audioPort, sAudioDelayMs);
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::getAudioDelayOffset (const JsonObject& parameters, JsonObject& response) 
        {   //sample servicemanager response:
            LOGINFOMETHOD();

            bool success = true;
            string audioPort = parameters["audioPort"].String();//empty value will browse all ports

            if (!checkPortName(audioPort))
                audioPort = "HDMI0";

            uint32_t audioDelayOffsetMs = 0;
            try
            {
                /* Return the sound mode of the audio ouput connected to the specified audioPort */
                /* Check if HDMI is connected - Return (default) Stereo Mode if not connected */
                if (audioPort.empty())
                {
                    std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                    if (device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str()).isDisplayConnected())
                    {
                        audioPort = "HDMI0";
                    }
                    else
                    {
                        /*  * If HDMI is not connected
                            * Get the SPDIF if it is supported by platform
                            * If Platform does not have connected ports. Default to HDMI.
                        */
                        audioPort = "HDMI0";
                        device::List<device::VideoOutputPort> vPorts = device::Host::getInstance().getVideoOutputPorts();
                        for (size_t i = 0; i < vPorts.size(); i++)
                        {
                            device::VideoOutputPort &vPort = vPorts.at(i);
                            if (vPort.isDisplayConnected())
                            {
                                audioPort = "SPDIF0";
                                break;
                            }
                        }
                    }
                }

                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                aPort.getAudioDelayOffset (audioDelayOffsetMs);
				response["audioDelayOffset"] = std::to_string(audioDelayOffsetMs);
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(audioPort);
                success = false;
            }

            returnResponse(success);
        }

        uint32_t DisplaySettings::setAudioDelayOffset (const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();

            returnIfParamNotFound(parameters, "audioDelayOffset");

            string sAudioDelayOffsetMs = parameters["audioDelayOffset"].String();
            int audioDelayOffsetMs = 0;
            try
            {
                audioDelayOffsetMs = stoi(sAudioDelayOffsetMs);
            }
            catch (const std::exception &err)
            {
                LOGERR("Failed to parse audioDelayOffset '%s'", sAudioDelayOffsetMs.c_str());
                returnResponse(false);
            }

            bool success = true;
            string audioPort = parameters["audioPort"].String();//empty value will browse all ports

            if (!checkPortName(audioPort))
                audioPort = "HDMI0";

            try
            {
                /* Return the sound mode of the audio ouput connected to the specified audioPort */
                /* Check if HDMI is connected - Return (default) Stereo Mode if not connected */
                if (audioPort.empty())
                {
                    std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                    if (device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str()).isDisplayConnected())
                    {
                        audioPort = "HDMI0";
                    }
                    else
                    {
                        /*  * If HDMI is not connected
                            * Get the SPDIF if it is supported by platform
                            * If Platform does not have connected ports. Default to HDMI.
                        */
                        audioPort = "HDMI0";
                        device::List<device::VideoOutputPort> vPorts = device::Host::getInstance().getVideoOutputPorts();
                        for (size_t i = 0; i < vPorts.size(); i++)
                        {
                            device::VideoOutputPort &vPort = vPorts.at(i);
                            if (vPort.isDisplayConnected())
                            {
                                audioPort = "SPDIF0";
                                break;
                            }
                        }
                    }
                }

                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                aPort.setAudioDelayOffset (audioDelayOffsetMs);
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION2(audioPort, sAudioDelayOffsetMs);
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::getSinkAtmosCapability (const JsonObject& parameters, JsonObject& response) 
        {   //sample servicemanager response:
            LOGINFOMETHOD();
            bool success = true;
            bool isValidAudioPort =  false;
            dsATMOSCapability_t atmosCapability;
            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "NULL";
            try
            {
                if(audioPort != "NULL") {
                    device::List<device::AudioOutputPort> aPorts = device::Host::getInstance().getAudioOutputPorts();
                    for (size_t i = 0; i < aPorts.size(); i++)
                    {
                        device::AudioOutputPort port = aPorts.at(i);
                        if(audioPort == port.getName()) {
                            isValidAudioPort = true;
                            break;
                        }
                    }

                    if(isValidAudioPort != true) {
                         success = false;
                         LOGERR("getSinkAtmosCapability failure: Unsupported Audio Port!!!\n");
                         returnResponse(success);
                    }
		}

                if (device::Host::getInstance().isHDMIOutPortPresent()) //STB
                {
                    device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI0");
                    if(isValidAudioPort) {
                        aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                    }
                    if (aPort.isConnected()) {
                        aPort.getSinkDeviceAtmosCapability (atmosCapability);
                        response["atmos_capability"] = (int)atmosCapability;
                    }
                    else {
                        LOGERR("getSinkAtmosCapability failure: %s not connected!\n", aPort.getName().c_str());
                        success = false;
                    }
                }
                else { //TV
                    if(isValidAudioPort) {
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                        if ( (aPort.getName() == "HDMI_ARC0" && aPort.isConnected() && m_arcEarcAudioEnabled == true) || (aPort.getName() != "HDMI_ARC0" && aPort.isConnected()) )  {
                            aPort.getSinkDeviceAtmosCapability (atmosCapability);
                            response["atmos_capability"] = (int)atmosCapability;
                        }
                        else {
                            LOGERR("getSinkAtmosCapability failure: %s not connected!\n", audioPort.c_str());
                            success = false;
                        }
                    }
                    else {
                        device::Host::getInstance().getSinkDeviceAtmosCapability (atmosCapability);
                        response["atmos_capability"] = (int)atmosCapability;
                    }
                }
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(audioPort);
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::setAudioAtmosOutputMode (const JsonObject& parameters, JsonObject& response) 
        {   //sample servicemanager response:
            LOGINFOMETHOD();
            returnIfParamNotFound(parameters, "enable");

            string sEnable = parameters["enable"].String();
            int enable = parameters["enable"].Boolean();

            bool success = true;
            try
            {
                if (device::Host::getInstance().isHDMIOutPortPresent())
                {
                    device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI0");
                    if (aPort.isConnected()) {
                        aPort.setAudioAtmosOutputMode (enable);
                    }
                    else {
					    LOGERR("setAudioAtmosOutputMode failure: HDMI0 not connected!\n");
                        success = false;
                    }
                }
                else {
                    device::Host::getInstance().setAudioAtmosOutputMode (enable);
                }
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION2(string("HDMI0"), sEnable);
                success = false;
            }
            returnResponse(success);
        }

	uint32_t DisplaySettings::setForceHDRMode (const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();
            returnIfParamNotFound(parameters, "hdr_mode");

            string sMode = parameters["hdr_mode"].String();
            dsHDRStandard_t mode = getVideoFormatTypeFromString(sMode.c_str());
            LOGINFO("setForceHDRMode entry hdr_mode :%s mode:%d  !\n",sMode.c_str(),mode);

            bool success = false;
            try
            {
		std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
                if (vPort.isDisplayConnected()) {
                   if(vPort.setForceHDRMode (mode) == true)
		    {
                        success = true;
			LOGINFO("setForceHDRMode set successfully \n");
		    }
                }
                else {
                    LOGERR("setForceHDRMode failure: HDMI0 not connected!\n");
                }
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION2(string("HDMI0"), sMode);
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::getPreferredColorDepth(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:{"colorDepth":"10 Bit","success":true}
            LOGINFOMETHOD();
            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
            string videoDisplay = parameters.HasLabel("videoDisplay") ? parameters["videoDisplay"].String() : strVideoPort;
            bool persist = parameters.HasLabel("persist") ? parameters["persist"].Boolean() : true;

            bool success = true;
            try
            {
                device::VideoOutputPort &vPort = device::Host::getInstance().getVideoOutputPort(videoDisplay);
                unsigned int colorDepth = vPort.getPreferredColorDepth(persist);
		switch (colorDepth) {
			case dsDISPLAY_COLORDEPTH_8BIT:
				response["colorDepth"] = "8 Bit";
				break;
			case dsDISPLAY_COLORDEPTH_10BIT:
				response["colorDepth"] = "10 Bit";
				break;
			case dsDISPLAY_COLORDEPTH_12BIT:
				response["colorDepth"] = "12 Bit";
				break;
			case dsDISPLAY_COLORDEPTH_AUTO:
				response["colorDepth"] = "Auto";
				break;
			default :
				success = false;
				break;
		}
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(videoDisplay);
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::setPreferredColorDepth(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response: {"success":true}
            LOGINFOMETHOD();
            returnIfParamNotFound(parameters, "videoDisplay");
            returnIfParamNotFound(parameters, "colorDepth");

            string videoDisplay = parameters["videoDisplay"].String();
            string strColorDepth = parameters["colorDepth"].String();

            bool persist = parameters.HasLabel("persist") ? parameters["persist"].Boolean() : true;

            bool success = true;
            try
            {
                dsDisplayColorDepth_t colorDepth = dsDISPLAY_COLORDEPTH_UNKNOWN;
                device::VideoOutputPort &vPort = device::Host::getInstance().getVideoOutputPort(videoDisplay);
		if (0==strncmp(strColorDepth.c_str(), "8 Bit", 5)){
                        colorDepth = dsDISPLAY_COLORDEPTH_8BIT;
		} else if (0==strncmp(strColorDepth.c_str(), "10 Bit", 6)){
                        colorDepth = dsDISPLAY_COLORDEPTH_10BIT;
		} else if (0==strncmp(strColorDepth.c_str(), "12 Bit", 6)){
                        colorDepth = dsDISPLAY_COLORDEPTH_12BIT;
		} else if (0==strncmp(strColorDepth.c_str(), "Auto", 4)){
                        colorDepth = dsDISPLAY_COLORDEPTH_AUTO;
		} else {
			//UNKNOWN color depth
			LOGERR("UNKNOWN color depth: %s", strColorDepth.c_str());
			success = false;
		}
                if (dsDISPLAY_COLORDEPTH_UNKNOWN!=colorDepth) {
                    vPort.setPreferredColorDepth(colorDepth, persist);
                }
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION2(videoDisplay, strColorDepth);
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::getColorDepthCapabilities(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:{"success":true,"capabilities":["8 Bit","10 Bit","12 Bit","Auto"]}
            LOGINFOMETHOD();
            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
            string videoDisplay = parameters.HasLabel("videoDisplay") ? parameters["videoDisplay"].String() : strVideoPort;
            vector<string> colorDepthCapabilities;
            try
            {
                unsigned int capabilities = 0;
                device::VideoOutputPort &vPort = device::Host::getInstance().getVideoOutputPort(videoDisplay);
                vPort.getColorDepthCapabilities(&capabilities);
                if(!capabilities) colorDepthCapabilities.emplace_back("none");
                if(capabilities & dsDISPLAY_COLORDEPTH_8BIT)colorDepthCapabilities.emplace_back("8 Bit");
                if(capabilities & dsDISPLAY_COLORDEPTH_10BIT)colorDepthCapabilities.emplace_back("10 Bit");
                if(capabilities & dsDISPLAY_COLORDEPTH_12BIT)colorDepthCapabilities.emplace_back("12 Bit");
                if(capabilities & dsDISPLAY_COLORDEPTH_AUTO)colorDepthCapabilities.emplace_back("Auto");
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(videoDisplay);
            }
            setResponseArray(response, "capabilities", colorDepthCapabilities);
            returnResponse(true);
        }

        bool DisplaySettings::setUpHdmiCecSinkArcRouting (bool arcEnable)
        {
            bool success = true;

            PluginHost::IShell::state state;
            if ((getServiceState(m_service, HDMICECSINK_CALLSIGN, state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED)) {
                LOGINFO("%s is active", HDMICECSINK_CALLSIGN);

                getHdmiCecSinkPlugin();
                if (!m_client) {
                    LOGERR("HdmiCecSink Initialisation failed\n");
                }
                else {
                    JsonObject hdmiCecSinkResult;
                    JsonObject param;

                    if(arcEnable) {
                        param["enabled"] = true;
                    }else {
                        param["enabled"] = false;
                    }

                    LOGINFO("ARC Routing - %d \n", arcEnable);
                    m_client->Invoke<JsonObject, JsonObject>(2000, "setupARCRouting", param, hdmiCecSinkResult);
                    if (!hdmiCecSinkResult["success"].Boolean()) {
			success = false;
                        LOGERR("HdmiCecSink Plugin returned error\n");
                    }
                }
            }
	    else {
		success = false;
                LOGERR("HdmiCecSink plugin not ready\n");
            }

            return success;
	}
	
	bool DisplaySettings::getHdmiCecSinkCecEnableStatus ()
        {
            bool cecEnable = false;

            PluginHost::IShell::state state;
            if ((getServiceState(m_service, HDMICECSINK_CALLSIGN, state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED)) {
                LOGINFO("%s is active", HDMICECSINK_CALLSIGN);

                getHdmiCecSinkPlugin();
                if (!m_client) {
                    LOGERR("HdmiCecSink Initialisation failed\n");
                }
                else {
                    JsonObject hdmiCecSinkResult;
                    JsonObject param;

                    m_client->Invoke<JsonObject, JsonObject>(2000, "getEnabled", param, hdmiCecSinkResult);

		    cecEnable = hdmiCecSinkResult["enabled"].Boolean();
		    LOGINFO("get-cecEnabled [%d]\n",cecEnable);

                    if (!hdmiCecSinkResult["success"].Boolean()) {
                        LOGERR("HdmiCecSink Plugin returned error\n");
                    }
                }
            }
            else {
                LOGERR("HdmiCecSink plugin not ready\n");
            }
            return cecEnable;
        }

	bool DisplaySettings::getHdmiCecSinkAudioDeviceConnectedStatus ()
        {
            bool hdmiAudioDeviceDetected = false;

            PluginHost::IShell::state state;
            if ((getServiceState(m_service, HDMICECSINK_CALLSIGN, state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED)) {
                LOGINFO("%s is active", HDMICECSINK_CALLSIGN);

                getHdmiCecSinkPlugin();
                if (!m_client) {
                    LOGERR("HdmiCecSink Initialisation failed\n");
                }
                else {
                    JsonObject hdmiCecSinkResult;
                    JsonObject param;

                    m_client->Invoke<JsonObject, JsonObject>(2000, "getAudioDeviceConnectedStatus", param, hdmiCecSinkResult);

                    hdmiAudioDeviceDetected = hdmiCecSinkResult["connected"].Boolean();
                    LOGINFO("getAudioDeviceConnectedStatus [%d]\n",hdmiAudioDeviceDetected);

                    if (!hdmiCecSinkResult["success"].Boolean()) {
                        LOGERR("HdmiCecSink Plugin returned error\n");
                    }
                }
            }
            else {
                LOGERR("HdmiCecSink plugin not ready\n");
            }
            return hdmiAudioDeviceDetected;
        }

        bool DisplaySettings::sendHdmiCecSinkAudioDevicePowerOn ()
        {
            bool success = true;

            PluginHost::IShell::state state;
            if ((getServiceState(m_service, HDMICECSINK_CALLSIGN, state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED)) {
                LOGINFO("%s is active", HDMICECSINK_CALLSIGN);

                getHdmiCecSinkPlugin();
                if (!m_client) {
                    LOGERR("HdmiCecSink Initialisation failed\n");
                }
                else {
                    JsonObject hdmiCecSinkResult;
                    JsonObject param;

                    LOGINFO("Send Audio Device Power On !!!\n");
                    m_client->Invoke<JsonObject, JsonObject>(2000, "sendAudioDevicePowerOnMessage", param, hdmiCecSinkResult);
                    if (!hdmiCecSinkResult["success"].Boolean()) {
                        success = false;
                        LOGERR("HdmiCecSink Plugin returned error\n");
                    }
                }
            }
            else {
                success = false;
                LOGERR("HdmiCecSink plugin not ready\n");
            }

            return success;
        }

        bool DisplaySettings::requestShortAudioDescriptor()
        {
            bool success = true;

            PluginHost::IShell::state state;
            if ((getServiceState(m_service, HDMICECSINK_CALLSIGN, state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED)) {
                LOGINFO("%s is active", HDMICECSINK_CALLSIGN);

                getHdmiCecSinkPlugin();
                if (!m_client) {
                    LOGERR("HdmiCecSink plugin not accessible\n");
                }
                else {
                    JsonObject hdmiCecSinkResult;
                    JsonObject param;

                    LOGINFO("Requesting Short Audio Descriptor \n");
                    m_client->Invoke<JsonObject, JsonObject>(2000, "requestShortAudioDescriptor", param, hdmiCecSinkResult);
                    if (!hdmiCecSinkResult["success"].Boolean()) {
                        success = false;
                        LOGERR("HdmiCecSink Plugin returned error\n");
                    }
                }
            }
            else {
                success = false;
                LOGERR("HdmiCecSink plugin not ready\n");
            }

            return success;
        }

        bool DisplaySettings::requestAudioDevicePowerStatus()
        {
            bool success = true;

            PluginHost::IShell::state state;
            if ((getServiceState(m_service, HDMICECSINK_CALLSIGN, state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED)) {
                LOGINFO("%s is active", HDMICECSINK_CALLSIGN);

                getHdmiCecSinkPlugin();
                if (!m_client) {
                    LOGERR("HdmiCecSink plugin not accessible\n");
                }
                else {
                    JsonObject hdmiCecSinkResult;
                    JsonObject param;

                    LOGINFO("Requesting Audio Device power Status \n");
                    m_client->Invoke<JsonObject, JsonObject>(2000, "requestAudioDevicePowerStatus", param, hdmiCecSinkResult);
                    if (!hdmiCecSinkResult["success"].Boolean()) {
                        success = false;
                        LOGERR("HdmiCecSink Plugin returned error\n");
                    }
                }
            }
            else {
                success = false;
                LOGERR("HdmiCecSink plugin not ready\n");
            }

            return success;
        }

        uint32_t DisplaySettings::setEnableAudioPort (const JsonObject& parameters, JsonObject& response)
        {   //TODO: Handle other audio ports. Currently only supports HDMI ARC/eARC
            LOGINFOMETHOD();
            returnIfParamNotFound(parameters, "audioPort");

            bool success = true;
            string audioPort = parameters["audioPort"].String();

            returnIfParamNotFound(parameters, "enable");
            string spEnable = parameters["enable"].String();
            bool pEnable = false;
            try {
                    pEnable = parameters["enable"].Boolean();
            }catch (const device::Exception& err) {
                    LOG_DEVICE_EXCEPTION1(spEnable);
                    returnResponse(false);
            }

            if (true == pEnable && IARM_BUS_PWRMGR_POWERSTATE_STANDBY == getSystemPowerState()) {
                LOGWARN("Ignoring the setEnableAudioPort(true) request based on the power state");
                returnResponse(false);
            }

            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                //Save the user settings irrespective of actual call passed or failed.
                aPort.setEnablePersist(pEnable);
                dsError_t eRet = dsERR_GENERAL;
                LOGWARN("Calling DisplaySettings::setEnableAudioPort audioPort:%s pEnable:%d \n", audioPort.c_str(), pEnable);
                //if not HDMI_ARC port
                if(audioPort.compare("HDMI_ARC0")) {
                    eRet = aPort.setEnablePort (pEnable);                    
                    if (dsERR_NONE != eRet) {
                        LOGWARN("DisplaySettings::setEnableAudioPort aPort.setEnablePort retuned %04x \n", eRet);
                        success = false;
                    } else if (aPort.isMuted()) {
                        LOGWARN("DisplaySettings::setEnableAudioPort aPort.isMuted()\n");
                        aPort.setMuted(true);
                    }
                }
                else /* for HDMI_ARC0 audio port */ 
		{
			LOGINFO("%s: m_hdmiInAudioDeviceConnected: %d , pEnable: %d \n",__FUNCTION__,m_hdmiInAudioDeviceConnected, pEnable);

			device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
			device::AudioStereoMode mode = device::AudioStereoMode::kStereo;  //default to stereo
                    	mode = aPort.getStereoMode(); //get Last User set stereo mode and set
			if(pEnable) 
			{
                            if(m_hdmiInAudioDeviceConnected == true)
			    {
			       if(m_hdmiInAudioDeviceType == dsAUDIOARCSUPPORT_eARC)
			       {  // EARC case
				  if(m_arcEarcAudioEnabled == false) 
				  {
				     LOGINFO("DisplaySettings::setEnableAudioPort Enable eARC !!!");
                                     aPort.enableARC(dsAUDIOARCSUPPORT_eARC, true);
                                     m_arcEarcAudioEnabled = true;
				  } else {
				     LOGINFO("eARC is already enabled. Value of m_arcEarcAudioEnabled is %d: \n", m_arcEarcAudioEnabled);
				  }
			       }/* EARC case end */
			       else if (m_hdmiInAudioDeviceType == dsAUDIOARCSUPPORT_ARC) 
			       {
				   if(m_arcEarcAudioEnabled == false ) 
			 	   {
                                        LOGINFO("%s: Audio Port : [HDMI_ARC0] sendHdmiCecSinkAudioDevicePowerOn !!! \n", __FUNCTION__);
                                        sendMsgToQueue(SEND_AUDIO_DEVICE_POWERON_MSG, NULL);
					/* Check SAD for passthru and Auto mode only */
					if ((mode == device::AudioStereoMode::kPassThru)  || (aPort.getStereoAuto() == true))
					{
					  {
					    std::lock_guard<std::mutex> lock(m_SadMutex);
					    /* Take actions according to SAD udpate state */
					    switch(m_AudioDeviceSADState)
					    {
						case  AUDIO_DEVICE_SAD_UPDATED: 						   
						{
							LOGINFO("%s: Enable ARC... \n",__FUNCTION__);
					 	        aPort.enableARC(dsAUDIOARCSUPPORT_ARC, true);
							m_arcEarcAudioEnabled = true;
						}
						break;

						case AUDIO_DEVICE_SAD_RECEIVED: 
						{
							LOGINFO("%s: Update Audio device SAD\n", __FUNCTION__);
							m_AudioDeviceSADState = AUDIO_DEVICE_SAD_UPDATED;
							aPort.setSAD(sad_list);

							if(aPort.getStereoAuto() == true) {
								aPort.setStereoAuto(true,true);
							} else {
								/* passthrough */
								aPort.setStereoMode(mode.toString(), true);
							}
												
							LOGINFO("%s: Enable ARC... \n",__FUNCTION__);
							aPort.enableARC(dsAUDIOARCSUPPORT_ARC, true); // enable  ARC
							m_arcEarcAudioEnabled = true;
						}
						break; 
											
						case AUDIO_DEVICE_SAD_REQUESTED: 
						{
							// SAD is not yet received so start a timer to wait for SAD update
							if ( !(m_SADDetectionTimer.isActive()))
							{ 			    
								m_SADDetectionTimer.start(SAD_UPDATE_CHECK_TIME_IN_MILLISECONDS);
							        LOGINFO("%s: Audio device SAD is not received yet, so starting timer for %d seconds", \
									__FUNCTION__, SAD_UPDATE_CHECK_TIME_IN_MILLISECONDS);
						        }
							LOGINFO("%s: Audio Device SAD is pending, Route audio after SAD update\n", __FUNCTION__);
						}
						break;
											
						default: 
						{
							LOGINFO("Incorrect Audio Deivce SAD state %d\n", m_AudioDeviceSADState); // should not hit this case
						}
						break;
					    }
					  } /* End of sad mutex scope */
					}/*End of mode == passthru and Auto mode operation */
					else
					{
					    //PCM
					    aPort.enableARC(dsAUDIOARCSUPPORT_ARC, true);
                                            m_arcEarcAudioEnabled = true;
					    LOGINFO("%s: Enable ARC... \n",__FUNCTION__);
					}
				   } 
				   else /* m_arcEarcAudioEnabled == true */
				   {
					// audio already routed.
                                        LOGINFO("ARC/eARC is audio already enabled. Value of m_arcEarcAudioEnabled is %d: \n", m_arcEarcAudioEnabled);
				   }/*End of if(m_arcEarcAudioEnabled == false ) */
				}/* ARC Case end*/
			        else
				{
				    LOGWARN("DisplaySettings::setEnableAudioPort Connected device doesn't have ARC/eARC capability \n");
				}
			    }
			    else /*m_hdmiInAudioDeviceConnected == false */
			    {
				LOGWARN("Audio device may be unplugged, not routing returning failure, m_hdmiInAudioDeviceConnected is %d: \n", m_hdmiInAudioDeviceConnected);
                                success = false; /* Returning failure since m_hdmiInAudioDeviceConnected=flase (HPD unplug case), but received call to enable audio*/
			    }/* End of (m_hdmiInAudioDeviceConnected == true) */

			}/* End of pEnable = true */			
			else
			{   //pEnable == false
			    if(m_arcEarcAudioEnabled == true ) 
			    {
				/* stop ARC/eARC */
				if(m_hdmiInAudioDeviceType == dsAUDIOARCSUPPORT_eARC)
				{
					aPort.enableARC(dsAUDIOARCSUPPORT_eARC, false);
					m_arcEarcAudioEnabled = false;
					LOGINFO("Disable eARC \n");
	                                if (m_hdmiInAudioDeviceConnected == false) {
					   /* Update Arctype only when device is disconneced */
					   m_hdmiInAudioDeviceType = dsAUDIOARCSUPPORT_NONE;
					}
				}
				else if (m_hdmiInAudioDeviceType == dsAUDIOARCSUPPORT_ARC)
				{				   
					aPort.enableARC(dsAUDIOARCSUPPORT_ARC, false);
					m_arcEarcAudioEnabled = false;	
					LOGINFO("Disable ARC \n");
	                                if (m_hdmiInAudioDeviceConnected == false) {
					   /* Update Arctype only when device is disconnected */
					   m_hdmiInAudioDeviceType = dsAUDIOARCSUPPORT_NONE;
					}
				}else {
					LOGWARN("DisplaySettings::setEnableAudioPort Connected device doesn't have ARC/eARC capability to Disable \n");
				}
			    }	
			    else
			    {
				LOGWARN(" ARC/eARC Audio already disbaled m_arcEarcAudioEnabled is %d: \n", m_arcEarcAudioEnabled);
			    }
			}/* End of pEnable*/
			
			m_audioOutputPortConfig["HDMI_ARC"] = pEnable;
		}

	    }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(audioPort);
                success = false;
            }
            returnResponse(success);
        }
        
	void  DisplaySettings::checkSADUpdate() {
		//Timer will be invoked if SAD update is delayed
		LOGINFO("Inside checkSADUpdate\n");
		std::lock_guard<std::mutex> lock(m_SadMutex);
		device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI_ARC0");
		LOGINFO("m_AudioDeviceSADState = %d, m_arcEarcAudioEnabled = %d, m_hdmiInAudioDeviceConnected = %d\n",m_AudioDeviceSADState, m_arcEarcAudioEnabled, m_hdmiInAudioDeviceConnected);
		if (m_arcEarcAudioEnabled == false && m_hdmiInAudioDeviceConnected == true){
			if (m_AudioDeviceSADState == AUDIO_DEVICE_SAD_RECEIVED)
			{
                           m_AudioDeviceSADState = AUDIO_DEVICE_SAD_UPDATED;
			   aPort.setSAD(sad_list);
        		   if(aPort.getStereoAuto() == true) {
            			aPort.setStereoAuto(true,true);
        		   }
        		   else{
            			device::AudioStereoMode mode = device::AudioStereoMode::kStereo;  //default to stereo
            			mode = aPort.getStereoMode(); //get Last User set stereo mode and set
            			aPort.setStereoMode(mode.toString(), true);
        		   }
                           LOGINFO("SAD is updated m_AudioDeviceSADState = %d\n", m_AudioDeviceSADState);
			}else{
				//Still SAD is not received, route audio with out SAD update.
                        	LOGINFO("Not recieved SAD update after 3sec timeout, proceeding with default SAD\n");
			}
			LOGINFO("%s: Enable ARC... \n",__FUNCTION__);
                        aPort.enableARC(dsAUDIOARCSUPPORT_ARC, true);
                        m_arcEarcAudioEnabled = true;
		}

		if (m_SADDetectionTimer.isActive()) {
			m_SADDetectionTimer.stop();
		}
	}

        uint32_t DisplaySettings::getEnableAudioPort (const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();
                       bool success = true;

            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
		bool isEnabled =  false;
		//Devicesettings returns exact HDMI ARC audio routing enable status
		//From thunder plugin's perspective HDMI ARC status must be the last user set value
		// even if ARC device is not connected. Audio routing will automatically start when the device is connected.
		if(!audioPort.compare("HDMI_ARC0")) {
                    JsonObject aPortConfig;
                    aPortConfig = getAudioOutputPortConfig();
		    isEnabled = aPortConfig["HDMI_ARC"].Boolean();
	        }
		else {
                    device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                    isEnabled = aPort.isEnabled();
		}
                response["enable"] = isEnabled;
                LOGWARN ("Thunder sending response to get state enable for audioPort %s is: %s", audioPort.c_str(), (isEnabled?("TRUE"):("FALSE"))); 
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(audioPort);
                success = false;
            }
            returnResponse(success);
        }


        // Thunder plugins communication
        void DisplaySettings::getHdmiCecSinkPlugin()
        {
            if(m_client == nullptr)
            {
                string token;

                // TODO: use interfaces and remove token
                auto security = m_service->QueryInterfaceByCallsign<PluginHost::IAuthenticate>("SecurityAgent");
                if (security != nullptr) {
                    string payload = "http://localhost";
                    if (security->CreateToken(
                            static_cast<uint16_t>(payload.length()),
                            reinterpret_cast<const uint8_t*>(payload.c_str()),
                            token)
                        == Core::ERROR_NONE) {
                        std::cout << "DisplaySettings got security token" << std::endl;
                    } else {
                        std::cout << "DisplaySettings failed to get security token" << std::endl;
                    }
                    security->Release();
                } else {
                    std::cout << "No security agent" << std::endl;
                }

                string query = "token=" + token;
                Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));
                m_client = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(_T(HDMICECSINK_CALLSIGN_VER), (_T(HDMICECSINK_CALLSIGN_VER)), false, query);
                LOGINFO("DisplaySettings getHdmiCecSinkPlugin init m_client\n");
            }
        }


        IARM_Bus_PWRMgr_PowerState_t DisplaySettings::getSystemPowerState()
        {
            IARM_Result_t res;
            IARM_Bus_PWRMgr_GetPowerState_Param_t param;

            res = IARM_Bus_Call(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_API_GetPowerState, (void *)&param, sizeof(param));
            if (res == IARM_RESULT_SUCCESS)
            {
                m_powerState = param.curState;
                LOGWARN("DisplaySettings::m_powerState: %d", m_powerState);
            }
            else
            {
                LOGWARN("GetPowerState failed");
            }

            return m_powerState;
        }

        void DisplaySettings::initAudioPortsWorker(void)
        {
            DisplaySettings::_instance->InitAudioPorts();
        }

        void DisplaySettings::powerEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if(!DisplaySettings::_instance)
                 return;
            if (strcmp(owner, IARM_BUS_PWRMGR_NAME) != 0)
                 return;

            switch (eventId) {
            case  IARM_BUS_PWRMGR_EVENT_MODECHANGED:
            {
                IARM_Bus_PWRMgr_EventData_t *eventData = (IARM_Bus_PWRMgr_EventData_t *)data;
                LOGWARN("Event IARM_BUS_PWRMGR_EVENT_MODECHANGED: State Changed %d --> %d\r",
                             eventData->data.state.curState, eventData->data.state.newState);
                m_powerState = eventData->data.state.newState;
                if (eventData->data.state.newState == IARM_BUS_PWRMGR_POWERSTATE_ON) {
	            try
                    {
		        LOGWARN("creating worker thread for initAudioPortsWorker ");
		        std::thread audioPortInitThread = std::thread(initAudioPortsWorker);
			audioPortInitThread.detach();
                    }
                    catch(const std::system_error& e)
                    {
                        LOGERR("system_error exception in thread creation: %s", e.what());
                    }
                    catch(const std::exception& e)
                    {
                        LOGERR("exception in thread creation : %s", e.what());
                    }
                }

		else {
		    LOGINFO("%s: Current Power state: %d\n",__FUNCTION__,eventData->data.state.newState);
            try
            {
                device::List<device::AudioOutputPort> aPorts = device::Host::getInstance().getAudioOutputPorts();
                bool hdmi_arc_supported = false;
                for (size_t i = 0; i < aPorts.size(); i++)
                {
                    device::AudioOutputPort &aPort = aPorts.at(i);
                    string portName  = aPort.getName();
                    if(portName == "HDMI_ARC0") {
                        hdmi_arc_supported = true;
                        break;    
                    }
                }

                if(hdmi_arc_supported) {
		  LOGINFO("Current Arc/eArc states m_currentArcRoutingState = %d, m_hdmiInAudioDeviceConnected =%d, m_arcEarcAudioEnabled =%d, m_hdmiInAudioDeviceType = %d\n", DisplaySettings::_instance->m_currentArcRoutingState, DisplaySettings::_instance->m_hdmiInAudioDeviceConnected, \
				  DisplaySettings::_instance->m_arcEarcAudioEnabled, DisplaySettings::_instance->m_hdmiInAudioDeviceType);
                  {
		        std::lock_guard<std::mutex> lock(DisplaySettings::_instance->m_AudioDeviceStatesUpdateMutex);
                    	LOGINFO("%s: Cleanup ARC/eARC state\n",__FUNCTION__);
                    	if(DisplaySettings::_instance->m_currentArcRoutingState != ARC_STATE_ARC_TERMINATED)
                            DisplaySettings::_instance->m_currentArcRoutingState = ARC_STATE_ARC_TERMINATED;
		      {
                    	if(DisplaySettings::_instance->m_hdmiInAudioDeviceConnected !=  false) {
                            DisplaySettings::_instance->m_hdmiInAudioDeviceConnected =  false;
			    DisplaySettings::_instance->connectedAudioPortUpdated(dsAUDIOPORT_TYPE_HDMI_ARC, false);
			    DisplaySettings::_instance->m_hdmiInAudioDevicePowerState = AUDIO_DEVICE_POWER_STATE_UNKNOWN;
			 }
                    
		    	if(DisplaySettings::_instance->m_arcEarcAudioEnabled == true) {
                            device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI_ARC0");
                            LOGINFO("%s: Disable ARC/eARC Audio\n",__FUNCTION__);
                            aPort.enableARC(dsAUDIOARCSUPPORT_ARC, false);
                            DisplaySettings::_instance->m_arcEarcAudioEnabled = false;
                        }
			if((DisplaySettings::_instance->m_hdmiInAudioDeviceType != dsAUDIOARCSUPPORT_NONE))
				DisplaySettings::_instance->m_hdmiInAudioDeviceType = dsAUDIOARCSUPPORT_NONE;

		      }
                  }//Release Mutex m_AudioDeviceStatesUpdateMutex

		  {
                    std::lock_guard<mutex> lck(DisplaySettings::_instance->m_callMutex);
                    if ( DisplaySettings::_instance->m_timer.isActive()) {
                        DisplaySettings::_instance->m_timer.stop();
                    }

                    if ( DisplaySettings::_instance->m_AudioDeviceDetectTimer.isActive()) {
                        DisplaySettings::_instance->m_AudioDeviceDetectTimer.stop();
                    }
                    if ( DisplaySettings::_instance->m_SADDetectionTimer.isActive()) {
                        DisplaySettings::_instance->m_SADDetectionTimer.stop();
                    }
                    if ( DisplaySettings::_instance->m_ArcDetectionTimer.isActive()) {
                        DisplaySettings::_instance->m_ArcDetectionTimer.stop();
                    }
                    if ( DisplaySettings::_instance->m_AudioDevicePowerOnStatusTimer.isActive()) {
                        DisplaySettings::_instance->m_AudioDevicePowerOnStatusTimer.stop();
                    }
                  }

                }
              }
              catch(const device::Exception& err)
              {
                LOG_DEVICE_EXCEPTION0();
              }
	     }
            }
            break;

            default: break;
            }
        }

	/* Message wrapper function to push the message to queue  */
	void DisplaySettings::sendMsgToQueue(msg_t msg, void *param )
	{
		SendMsgInfo msgInfo;

                msgInfo.msg = msg;
		msgInfo.param = param;
		std::unique_lock<std::mutex> lock(DisplaySettings::_instance->m_sendMsgMutex);
        	DisplaySettings::_instance->m_sendMsgQueue.push(msgInfo);
        	DisplaySettings::_instance->m_sendMsgThreadRun = true;
        	DisplaySettings::_instance->m_sendMsgCV.notify_one();
	}
		
void DisplaySettings::sendMsgThread()
{
	LOGINFO("%s: message Thread Start\n",__FUNCTION__);
	bool result = false;
        SendMsgInfo msgInfo;
	
	if(!DisplaySettings::_instance)
                 return;

	while(!_instance->m_sendMsgThreadExit) 
	{
		msgInfo.msg = -1;
        	msgInfo.param = NULL;
		{

                       LOGINFO("%s: Debug: Wait for message \n",__FUNCTION__);
		       std::unique_lock<std::mutex> lock(DisplaySettings::_instance->m_sendMsgMutex);
		       _instance->m_sendMsgCV.wait(lock, []{return (_instance->m_sendMsgThreadRun == true);});
		
		}

		if (_instance->m_sendMsgThreadExit == true)
        	{
            		LOGINFO(" sendCecMessageThread Exiting");
            		_instance->m_sendMsgThreadRun = false;
            		break;
        	}

        	if (_instance->m_sendMsgQueue.empty()) {
            		_instance->m_sendMsgThreadRun = false;
            		continue;
        	}
		
		msgInfo = DisplaySettings::_instance->m_sendMsgQueue.front();
		
			switch(msgInfo.msg)
			{
				case SEND_AUDIO_DEVICE_POWERON_MSG:
				{
					LOGINFO(" sendHdmiCecSinkAudioDevicePowerOn");
					result = DisplaySettings::_instance->sendHdmiCecSinkAudioDevicePowerOn();
				}
				break;
				
				case REQUEST_SHORT_AUDIO_DESCRIPTOR:
				{
					LOGINFO(" Request Short Audio descriptor");
					result = DisplaySettings::_instance->requestShortAudioDescriptor();
				}
				break;
				
				case REQUEST_AUDIO_DEVICE_POWER_STATUS:
				{
					LOGINFO(" Request Audio Device Power Status");
					result = DisplaySettings::_instance->requestAudioDevicePowerStatus();
				}
				break;

				case SEND_REQUEST_ARC_INITIATION: // spearte initiation and termination cases
				{
					LOGINFO(" Send request for ARC INITIATION");
					result = DisplaySettings::_instance->setUpHdmiCecSinkArcRouting(true);
				}
				break;
				
				case SEND_REQUEST_ARC_TERMINATION:
				{
					LOGINFO(" Send request for ARC TERMINATION");
					result = DisplaySettings::_instance->setUpHdmiCecSinkArcRouting(false);
				}
		
				default:
				{
					LOGINFO(" Requested invalid message");
				}
				break;
				
			}
			
			if (result == true) {
			    LOGINFO(" send cec msg [%d] success \n",msgInfo.msg);
			}else{
			    LOGERR(" send cec msg [%d] failed \n",msgInfo.msg);
			}
						
		DisplaySettings::_instance->m_sendMsgQueue.pop();
	}
}

        // Event management
        // 1.
        uint32_t DisplaySettings::subscribeForHdmiCecSinkEvent(const char* eventName)
        {
            uint32_t err = Core::ERROR_NONE;
            LOGINFO("Attempting to subscribe for event: %s\n", eventName);
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(SERVER_DETAILS)));
            if (nullptr == m_client) {
                getHdmiCecSinkPlugin();
                if (nullptr == m_client) {
                    LOGERR("JSONRPC: %s: client initialization failed", HDMICECSINK_CALLSIGN_VER);
                    err = Core::ERROR_UNAVAILABLE;
                } 
            }

	    if(err == Core::ERROR_NONE) {
                /* Register handlers for Event reception. */
                if(strcmp(eventName, HDMICECSINK_ARC_INITIATION_EVENT) == 0) {
                    err =m_client->Subscribe<JsonObject>(1000, eventName
                            , &DisplaySettings::onARCInitiationEventHandler, this);
                    m_clientRegisteredEventNames.push_back(eventName);
                } else if(strcmp(eventName, HDMICECSINK_ARC_TERMINATION_EVENT) == 0) {
                    err =m_client->Subscribe<JsonObject>(1000, eventName
                            , &DisplaySettings::onARCTerminationEventHandler, this);
                    m_clientRegisteredEventNames.push_back(eventName);
                } else if(strcmp(eventName, HDMICECSINK_SHORT_AUDIO_DESCRIPTOR_EVENT) == 0) {
                    err =m_client->Subscribe<JsonObject>(1000, eventName
                            , &DisplaySettings::onShortAudioDescriptorEventHandler, this);
                    m_clientRegisteredEventNames.push_back(eventName);
                } else if(strcmp(eventName, HDMICECSINK_SYSTEM_AUDIO_MODE_EVENT) == 0) {
                    err =m_client->Subscribe<JsonObject>(1000, eventName
                            , &DisplaySettings::onSystemAudioModeEventHandler, this);
                    m_clientRegisteredEventNames.push_back(eventName);
                } else if(strcmp(eventName, HDMICECSINK_ARC_AUDIO_STATUS_EVENT) == 0) {
                    err =m_client->Subscribe<JsonObject>(1000, eventName
                            , &DisplaySettings::onArcAudioStatusEventHandler, this);
                    m_clientRegisteredEventNames.push_back(eventName);
                } else if(strcmp(eventName, HDMICECSINK_AUDIO_DEVICE_CONNECTED_STATUS_EVENT) == 0) {
                    err =m_client->Subscribe<JsonObject>(1000, eventName
                            , &DisplaySettings::onAudioDeviceConnectedStatusEventHandler, this);
                    m_clientRegisteredEventNames.push_back(eventName);
                } else if(strcmp(eventName, HDMICECSINK_CEC_ENABLED_EVENT) == 0) {
                    err =m_client->Subscribe<JsonObject>(1000, eventName
                            , &DisplaySettings::onCecEnabledEventHandler, this);
                    m_clientRegisteredEventNames.push_back(eventName);
                } else if(strcmp(eventName, HDMICECSINK_AUDIO_DEVICE_POWER_STATUS_EVENT) == 0) {
                    err =m_client->Subscribe<JsonObject>(1000, eventName
                            , &DisplaySettings::onAudioDevicePowerStatusEventHandler, this);
                    m_clientRegisteredEventNames.push_back(eventName);
		} else {
                     err = Core::ERROR_UNAVAILABLE;
                     LOGERR("Unsupported Event: %s ", eventName);
                }
                if ( err  == Core::ERROR_NONE) {
                    LOGINFO("Subscribed for %s", eventName);
                } else {
                    LOGERR("Failed to subscribe for %s with code %d", eventName, err);
                }
            }
            return err;
        }

        // 2.
        void DisplaySettings::onARCInitiationEventHandler(const JsonObject& parameters) {
            string message;
	    string value;

            parameters.ToString(message);
            LOGINFO("[ARC Initiation Event], %s : %s", __FUNCTION__, C_STR(message));

            if (!parameters.HasLabel("status")) {
                LOGERR("Field 'status' could not be found in the event's payload.");
                return;
            }
	    LOGINFO("ARC routing state before update m_currentArcRoutingState=%d\n ", m_currentArcRoutingState);
	    // AVR power status is not checked here assuming that ARC init request will happen only when AVR is in ON state
            if ((m_currentArcRoutingState != ARC_STATE_ARC_INITIATED) && (m_systemAudioMode_Power_RequestedAndReceived == true)) {
                value = parameters["status"].String();

		if( !value.compare("success") ) {
		    //Update Arc state
                    std::lock_guard<std::mutex> lock(m_AudioDeviceStatesUpdateMutex);
                    m_currentArcRoutingState = ARC_STATE_ARC_INITIATED;
		    //Request SAD
		    // We will get Arc initiation request only if port is connected and Audio device is detected
		    // So no need to explicitly check for that
	            LOGINFO("ARC routing state after update m_currentArcRoutingState=%d\n ", m_currentArcRoutingState);
		    device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI_ARC0");
		    device::AudioStereoMode mode = device::AudioStereoMode::kStereo;  //default to stereo
                    mode = aPort.getStereoMode(); //get Last User set stereo mode and set
		    if ((m_AudioDeviceSADState == AUDIO_DEVICE_SAD_CLEARED || m_AudioDeviceSADState == AUDIO_DEVICE_SAD_UNKNOWN) && \
				    ((mode == device::AudioStereoMode::kPassThru) || aPort.getStereoAuto() == true)) {
			   LOGINFO("Initiate SAD request\n");
			   m_AudioDeviceSADState = AUDIO_DEVICE_SAD_REQUESTED;
			   sendMsgToQueue(REQUEST_SHORT_AUDIO_DESCRIPTOR, NULL);
		    } else {
			    LOGINFO("SAD not requested m_AudioDeviceSADState =%d, soundmode = %s", m_AudioDeviceSADState, mode.toString().c_str());
		    }
		    //update device type in case we receive ARC init before power ON request
		    if (m_hdmiInAudioDeviceType == dsAUDIOARCSUPPORT_NONE) {
			    LOGINFO("Updating Audio device type to Arc\n");
			    m_hdmiInAudioDeviceType = dsAUDIOARCSUPPORT_ARC;
		    } else {
			    LOGINFO("m_hdmiInAudioDeviceType is already updated %d\n", m_hdmiInAudioDeviceType);
		    }
                    try
                    {
			if(m_hdmiInAudioDeviceConnected ==  false) {
                            m_hdmiInAudioDeviceConnected = true;
			    if (m_arcEarcAudioEnabled == false) {
				LOGINFO("Arc Initiation sucess, Notify UI\n");
			        connectedAudioPortUpdated(dsAUDIOPORT_TYPE_HDMI_ARC, true);
			    } else {
				    LOGINFO("not notified to UI since m_arcEarcAudioEnabled =%d\n",m_arcEarcAudioEnabled);
			    }
			}
			else {
                            LOGINFO("onARCInitiationEventHandler: not notifying the UI as m_hdmiInAudioDeviceConnected = true !!!\n");
                        }

                    }
                    catch (const device::Exception& err)
                    {
                        LOG_DEVICE_EXCEPTION1(string("HDMI_ARC0"));
                    }
		} //Release Mutex m_AudioDeviceStatesUpdateMutex if Arc is Success
		else{
                    LOGERR("CEC ARC Initiaition Failed !!!");
                    {
                      std::lock_guard<std::mutex> lock(m_AudioDeviceStatesUpdateMutex);
                      m_currentArcRoutingState = ARC_STATE_ARC_TERMINATED;
                    }//Release Mutex m_AudioDeviceStatesUpdateMutex if Arc failure
		}
            } else {
                LOGINFO("%s: The ARC initiation already done or m_systemAudioMode_Power_RequestedAndReceived [%d]", __FUNCTION__, m_systemAudioMode_Power_RequestedAndReceived);
            }
        }

        // 3.
        void DisplaySettings::onARCTerminationEventHandler(const JsonObject& parameters) {
            string message;
	    string value;

            parameters.ToString(message);
            LOGINFO("[ARC Termination Event], %s : %s", __FUNCTION__, C_STR(message));

	    if (m_AudioDeviceSADState != AUDIO_DEVICE_SAD_CLEARED) {
		m_AudioDeviceSADState = AUDIO_DEVICE_SAD_CLEARED;
		LOGINFO("%s: Clearing Audio device SAD\n", __FUNCTION__);
		//clear the SAD list
		sad_list.clear();
	    } else {
		LOGINFO("SAD already cleared\n");
	    }

	    LOGINFO("Current ARC routing state before update m_currentArcRoutingState=%d\n ", m_currentArcRoutingState);
	    if (m_currentArcRoutingState != ARC_STATE_ARC_TERMINATED) {
                if (parameters.HasLabel("status")) {
                    value = parameters["status"].String();
                    std::lock_guard<std::mutex> lock(m_AudioDeviceStatesUpdateMutex);
                    m_currentArcRoutingState = ARC_STATE_ARC_TERMINATED;
	            LOGINFO("Current ARC routing state after update m_currentArcRoutingState=%d\n ", m_currentArcRoutingState);
                    if(!value.compare("success")) {
		        try 
		        {
			    if(m_hdmiInAudioDeviceConnected ==  true) {
				m_hdmiInAudioDeviceConnected = false;
				if (m_arcEarcAudioEnabled == true) {
                                    connectedAudioPortUpdated(dsAUDIOPORT_TYPE_HDMI_ARC, false);
				} else {
				    LOGINFO("Not notifying UI since m_arcEarcAudioEnabled = %d", m_arcEarcAudioEnabled);
                                }
			    }
			    else {
			        LOGINFO("onARCTerminationEventHandler: Skip Disable ARC and not notifying the UI as  m_hdmiInAudioDeviceConnected = false\n");
			    }
	                }
                        catch (const device::Exception& err)
                        {
                            LOG_DEVICE_EXCEPTION1(string("HDMI_ARC0"));
                        }
                    }
                    else{
                        LOGERR("CEC onARCTerminationEventHandler Failed !!!");
                    }
                }//Release mutex m_AudioDeviceStatesUpdateMutex 
		else {
                    LOGERR("Field 'status' could not be found in the event's payload.");
                }
	    }
        }

        // 4.
        void DisplaySettings::onShortAudioDescriptorEventHandler(const JsonObject& parameters) {
            string message;

            parameters.ToString(message);
	    JsonArray shortAudioDescriptorList;
            LOGINFO("[Short Audio Descriptor Event], %s : %s", __FUNCTION__, C_STR(message));

            if (parameters.HasLabel("ShortAudioDescriptor")) {
                shortAudioDescriptorList = parameters["ShortAudioDescriptor"].Array();
		if (m_AudioDeviceSADState == AUDIO_DEVICE_SAD_REQUESTED) {
                    try
                    {
		        std::lock_guard<std::mutex> lock(m_SadMutex);
			m_AudioDeviceSADState = AUDIO_DEVICE_SAD_RECEIVED;
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI_ARC0");
			LOGINFO("Total Short Audio Descriptors received from connected ARC device: %d\n",shortAudioDescriptorList.Length());
			if(shortAudioDescriptorList.Length() <= 0) {
			    LOGERR("Not setting SAD. No SAD returned by connected ARC device\n");
			    return;
			}

			for (int i=0; i<shortAudioDescriptorList.Length(); i++) {
                            LOGINFO("Short Audio Descriptor[%d]: %lld \n",i, shortAudioDescriptorList[i].Number());
                            sad_list.push_back(shortAudioDescriptorList[i].Number());
                        }

			bool wasSADTimerActive = false;

			if (m_currentArcRoutingState == ARC_STATE_ARC_INITIATED) {
			    if (m_SADDetectionTimer.isActive()) {
			        //Timer is active, so stop the timer and if audio is not routed set SAD and route the audio
			        LOGINFO("%s: Stopping the SAD timer\n", __FUNCTION__);
			        m_SADDetectionTimer.stop();
				
				wasSADTimerActive = true;
			    }

			    if (wasSADTimerActive == true && m_arcEarcAudioEnabled == false ) { /*setEnableAudioPort is called, Timer has started, got SAD before Timer Expiry*/
			        LOGINFO("%s: Updating SAD \n", __FUNCTION__);
                                m_AudioDeviceSADState = AUDIO_DEVICE_SAD_UPDATED;
                                aPort.setSAD(sad_list);
                                if(aPort.getStereoAuto() == true) {
                                    aPort.setStereoAuto(true,true);
                                }
                                else{
                                    device::AudioStereoMode mode = device::AudioStereoMode::kStereo;  //default to stereo
                                    mode = aPort.getStereoMode(); //get Last User set stereo mode and set
                                    aPort.setStereoMode(mode.toString(), true);
                                }
				LOGINFO("%s: Routing the audio since m_arcEarcAudioEnabled = %d\n", __FUNCTION__, m_arcEarcAudioEnabled);
				LOGINFO("%s: Enable ARC... \n",__FUNCTION__);
				aPort.enableARC(dsAUDIOARCSUPPORT_ARC, true);
                        	m_arcEarcAudioEnabled = true;
			    } else if (m_arcEarcAudioEnabled == true) { /*setEnableAudioPort is called,Timer started and Expired, arc is routed -- or for both wasSADTimerActive == true/false*/
				LOGINFO("%s: Updating SAD since audio is already routed and ARC is initiated\n", __FUNCTION__);
				 m_AudioDeviceSADState = AUDIO_DEVICE_SAD_UPDATED;
				    aPort.setSAD(sad_list);
                        	    if(aPort.getStereoAuto() == true) {
                    	            	aPort.setStereoAuto(true,true);
                            	    }
                            	    else{
                                	device::AudioStereoMode mode = device::AudioStereoMode::kStereo;  //default to stereo
                                	mode = aPort.getStereoMode(); //get Last User set stereo mode and set
                                	aPort.setStereoMode(mode.toString(), true);
                            	    }
			      } else { // SAD received before setEnableAudioPort
			            LOGINFO("%s: Not updating SAD now since arc routing has not yet happened and SAD timer is not active -> Routing and SAD is updated when setEnableAudioPort is called \n", __FUNCTION__);
			      }
			}else {
				LOGINFO("%s: m_currentArcRoutingState = %d, m_arcEarcAudioEnabled = %d", __FUNCTION__, m_currentArcRoutingState, m_arcEarcAudioEnabled);
			}/*End of m_currentArcRoutingState check */
                    }
                    catch (const device::Exception& err)
                    {
                        LOG_DEVICE_EXCEPTION1(string("HDMI_ARC0"));
                    }
            }  else {
                LOGERR("Invalid SAD state m_AudioDeviceSADState =%d", m_AudioDeviceSADState);
               }/*End of (m_AudioDeviceSADState == AUDIO_DEVICE_SAD_REQUESTED) */
			}
		    else {
                LOGERR("Field 'ShortAudioDescriptor' could not be found in the event's payload.");
            }/*End of (m_AudioDeviceSADState == AUDIO_DEVICE_SAD_REQUESTED) */
        }

        // 5.
        void DisplaySettings::onSystemAudioModeEventHandler(const JsonObject& parameters) {
            string message;
            string value;

            parameters.ToString(message);
            LOGINFO("[System Audio Mode Event], %s : %s", __FUNCTION__, C_STR(message));

            if (parameters.HasLabel("audioMode")) {
                value = parameters["audioMode"].String();
                if(!value.compare("On")) {
	                m_systemAudioMode_Power_RequestedAndReceived = true; // system audio mode ON is received
			LOGINFO("Requesting power status of AVR as system audio mode is %s\n", C_STR(message));
	                m_hdmiInAudioDevicePowerState = AUDIO_DEVICE_POWER_STATE_REQUEST;//Should we send power request irrespective of System audio mode status
                        sendMsgToQueue(REQUEST_AUDIO_DEVICE_POWER_STATUS, NULL);
                }
		else if(!value.compare("Off")) {
                    LOGINFO("%s :  audioMode OFF !!!\n", __FUNCTION__);
		    try {
			std::lock_guard<std::mutex> lock(m_AudioDeviceStatesUpdateMutex);
                        m_hdmiInAudioDevicePowerState = AUDIO_DEVICE_POWER_STATE_UNKNOWN;

   		        if(m_hdmiInAudioDeviceConnected == true) {
			    LOGINFO("SystemAudio mode off disable Arc\n");
			    m_hdmiInAudioDeviceConnected = false;
			    if (m_arcEarcAudioEnabled == true) {
				LOGINFO("System Audio mode is off and arc Enable is %d, Notify UI to disbale Arc", m_arcEarcAudioEnabled);
		            	connectedAudioPortUpdated(dsAUDIOPORT_TYPE_HDMI_ARC, false);
			    } else {
				    LOGINFO("Not notifying UI since m_arcEarcAudioEnabled =%d\n", m_arcEarcAudioEnabled);
			    }
                            {
			      // Arc termination happens from HdmiCecSink plugin so just update the state here
                              m_currentArcRoutingState = ARC_STATE_ARC_TERMINATED;
			      LOGINFO("Updating ARC routing state to ARC terminated\n");
                            }

		        }
                        else {
                            LOGINFO("onSystemAudioModeEventHandler: Skip Disable ARC and not notifying the UI as  m_hdmiInAudioDeviceConnected = false\n");
                        }
		    }//Release mutex m_AudioDeviceStatesUpdateMutex
		    catch(const device::Exception& err)
                    {
		        LOG_DEVICE_EXCEPTION1(string("HDMI_ARC0"));
                    }
                }
                else{
                    LOGERR("%s: Invalid audio mode sent by HdmiCecSink !!!\n",__FUNCTION__);
                }
            } else {
                LOGERR("Field 'audioMode' could not be found in the event's payload.");
            }
        }

        void DisplaySettings::onArcAudioStatusEventHandler(const JsonObject& parameters) {
            string message;
            parameters.ToString(message);
            LOGINFO("[ARC Audio Status Event], %s : %s", __FUNCTION__, C_STR(message));

            if (parameters.HasLabel("muteStatus") && parameters.HasLabel("volumeLevel")) {
                hdmiArcVolumeLevel =  stoi(parameters["volumeLevel"].String());
            } else {
                LOGERR("Field 'muteStatus' and 'volumeLevel' could not be found in the event's payload.");
            }
        }

	/* Event handler when Audio Device is Added/Removed     */
	void DisplaySettings::onAudioDeviceConnectedStatusEventHandler(const JsonObject& parameters)
	{
	    string value;

	    if (parameters.HasLabel("audioDeviceConnected"))
		value = parameters["audioDeviceConnected"].String();
	    
	    if(!value.compare("true")) {
	        m_hdmiCecAudioDeviceDetected = true;
            } else{
	            m_hdmiCecAudioDeviceDetected = false;
		        if (m_hdmiInAudioDeviceConnected == true) {
					LOGINFO("Audio device removed event Handler, clearing the states m_hdmiInAudioDeviceConnected =%d, m_currentArcRoutingState =%d", \
                    m_hdmiInAudioDeviceConnected, m_currentArcRoutingState);
				    m_hdmiInAudioDeviceConnected = false;	
		    	    m_hdmiInAudioDevicePowerState = AUDIO_DEVICE_POWER_STATE_UNKNOWN;
                    m_currentArcRoutingState = ARC_STATE_ARC_TERMINATED;
				    connectedAudioPortUpdated(dsAUDIOPORT_TYPE_HDMI_ARC, false);
			    }
		        if (m_AudioDeviceSADState != AUDIO_DEVICE_SAD_CLEARED && m_AudioDeviceSADState != AUDIO_DEVICE_SAD_UNKNOWN) {
		            LOGINFO("%s: Clearing Audio device SAD previous state= %d current state = %d\n", __FUNCTION__, m_AudioDeviceSADState, AUDIO_DEVICE_SAD_CLEARED);
		            //clear the SAD list
		            sad_list.clear();
		            m_AudioDeviceSADState = AUDIO_DEVICE_SAD_CLEARED;
		        } else {
		            LOGINFO("SAD already cleared\n");
	            }
                //if m_arcEarcAudioEnabled == true(case where arc/earc is already routed) we will not reset device type because it will be done from setEnableAudioPort during disable from the connectedAudioPort update
				if (m_arcEarcAudioEnabled == false && m_hdmiInAudioDeviceType != dsAUDIOARCSUPPORT_NONE) {
					LOGINFO("Reset m_hdmiInAudioDeviceType since m_arcEarcAudioEnabled = %d", m_arcEarcAudioEnabled);
					m_hdmiInAudioDeviceType = dsAUDIOARCSUPPORT_NONE;
				}

            }
	    LOGINFO("updated m_hdmiCecAudioDeviceDetected status [%d] ... \n", m_hdmiCecAudioDeviceDetected);

		if (m_hdmiCecAudioDeviceDetected)
		{
                    LOGINFO("Trigger Audio Device Power State Request status ... \n");
		    m_hdmiInAudioDevicePowerState = AUDIO_DEVICE_POWER_STATE_REQUEST;
                    sendMsgToQueue(REQUEST_AUDIO_DEVICE_POWER_STATUS, NULL);

		} else {
                    LOGINFO("Audio Device is removed \n");
		}
                hdmiArcVolumeLevel = 0;
        }

	void DisplaySettings::onAudioDevicePowerStatusEventHandler(const JsonObject& parameters) {
            string value;
            if (parameters.HasLabel("powerStatus"))
                value = parameters["powerStatus"].String();

             int pState = 1;//STANDBY
             bool isIntiger = Utils::isValidInt ((char*)value.c_str());
             if (false == isIntiger) {
                 LOGWARN("powerStatus is not a valid int\n");
                 return;
             }
             try {
                 pState = stoi(value);
             }catch (const device::Exception& err) {
                 LOG_DEVICE_EXCEPTION1(value);
                 return;
	     }

	     LOGINFO("Audio Device Power State [%d] ... \n", pState);

             if(pState == AVR_POWER_STATE_ON) {//ON
                m_hdmiInAudioDevicePowerState = AUDIO_DEVICE_POWER_STATE_ON;
	        m_systemAudioMode_Power_RequestedAndReceived = true; // received power ON msg from AVR.

                if (m_AudioDevicePowerOnStatusTimer.isActive()) {
	           LOGINFO("Stopping timer, Audio Device power status - m_hdmiInAudioDevicePowerState [%d]!!!\n", m_hdmiInAudioDevicePowerState);
                   retryPowerRequestCount = 0;
                   m_AudioDevicePowerOnStatusTimer.stop();
                }

                try {
                    int types = dsAUDIOARCSUPPORT_NONE;
                    device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI_ARC0");
                    aPort.getSupportedARCTypes(&types);
                    if((types & dsAUDIOARCSUPPORT_eARC) && (m_hdmiInAudioDeviceConnected == false)) {
			    LOGINFO("%s: Audio device is eArc m_hdmiInAudioDeviceConnected =%d",__FUNCTION__,m_hdmiInAudioDeviceConnected);
                        m_hdmiInAudioDeviceConnected = true;
			m_hdmiInAudioDeviceType = dsAUDIOARCSUPPORT_eARC;
			if (m_arcEarcAudioEnabled == false) {
			    // Notify UI that Audio device is connected and is in ON state
                            LOGINFO("Triggered from HPD: eARC audio device power on: Notify UI !!! \n");
                            connectedAudioPortUpdated(dsAUDIOPORT_TYPE_HDMI_ARC, true);
			} else {
				LOGINFO("arc already enabled m_arcEarcAudioEnabled =%d", m_arcEarcAudioEnabled);
			}
                    } else {
			if ((m_hdmiInAudioDeviceConnected == false) && !(m_ArcDetectionTimer.isActive())) {
			    // tinymix commad to detect eArc is failed, start the timer for 3 seconds
			    LOGINFO("Starting timer to detect eArc for %d milli seconds", ARC_DETECTION_CHECK_TIME_IN_MILLISECONDS);
		            m_ArcDetectionTimer.start(ARC_DETECTION_CHECK_TIME_IN_MILLISECONDS);
			}
		    }
                }
                catch(const device::Exception& err)
                {
                    LOG_DEVICE_EXCEPTION1(string("HDMI_ARC0"));
                }
             }
             else if(pState == AVR_POWER_STATE_STANDBY) {
                 m_hdmiInAudioDevicePowerState = AUDIO_DEVICE_POWER_STATE_STANDBY;
             } else if (pState == AVR_POWER_STATE_STANDBY_TO_ON_TRANSITION) {
		     //Start a timer to re check the power status of AVR?
		     LOGINFO("Audio device  power status IN TRANSITION from STANDBY to ON, Requesting power status again pState=%d\n", pState);
		     m_hdmiInAudioDevicePowerState = AUDIO_DEVICE_POWER_STATE_REQUEST;
                     sendMsgToQueue(REQUEST_AUDIO_DEVICE_POWER_STATUS, NULL);
	     }
        }

	/*Arc detection timer*/
        void DisplaySettings::checkArcDeviceConnected() {
	    //Timer is invoked in case of delayed HPD
	    LOGINFO("Inside checkArcDeviceConnected\n");
	    static int retryArcCount = 0;
	    std::lock_guard<std::mutex> lock(m_callMutex);
            int types = dsAUDIOARCSUPPORT_NONE;
            device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI_ARC0");
            aPort.getSupportedARCTypes(&types);
	    if(m_currentArcRoutingState != ARC_STATE_ARC_INITIATED) {
	       if((types & dsAUDIOARCSUPPORT_eARC) && (m_hdmiInAudioDeviceConnected == false)) {
                   m_hdmiInAudioDeviceConnected = true;
		   m_hdmiInAudioDeviceType = dsAUDIOARCSUPPORT_eARC;
		   if (m_arcEarcAudioEnabled == false) {
                       LOGINFO("Triggered from HPD: eARC audio device power on: Notify UI !!! \n");
                       connectedAudioPortUpdated(dsAUDIOPORT_TYPE_HDMI_ARC, true);
		   } else {
		       LOGINFO("Arc enabled already m_arcEarcAudioEnabled =%d", m_arcEarcAudioEnabled);
		   }
               } else if(m_hdmiInAudioDeviceConnected == false) {
		    std::lock_guard<std::mutex> lock(m_AudioDeviceStatesUpdateMutex);
		    retryArcCount ++;
		    LOGINFO("device Type is ARC, checking if eARC - retryArcCount [%d]", retryArcCount);
		    if (retryArcCount >= 3 )
		    {
		        m_hdmiInAudioDeviceType = dsAUDIOARCSUPPORT_ARC;
                        if((m_currentArcRoutingState == ARC_STATE_ARC_TERMINATED) && (isCecEnabled == true)) {
			    LOGINFO("ARC_mode: Send dummy ARC initiation request... \n");
                            LOGINFO("ARC_mode: Notify Arc routing with m_currentArcRoutingStat [%d] \n", DisplaySettings::_instance->m_currentArcRoutingState );
                            m_currentArcRoutingState = ARC_STATE_REQUEST_ARC_INITIATION;
                            sendMsgToQueue(SEND_REQUEST_ARC_INITIATION, NULL);
                        } else {
			    LOGINFO("Arc initiation request not sent\n");
		        }
	            }
	       }//Release Mutex m_AudioDeviceStatesUpdateMutex
	    } else {
		    LOGINFO("Arc is already initiated m_currentArcRoutingState =%d", m_currentArcRoutingState);
	    }

	    if ( m_ArcDetectionTimer.isActive() && ((retryArcCount >= 3) || (m_currentArcRoutingState == ARC_STATE_ARC_INITIATED) || (m_hdmiInAudioDeviceType != dsAUDIOARCSUPPORT_NONE)) ) {
	            retryArcCount = 0; /* reset counter */
		    LOGINFO("Stopping the eArc detection timer retryArcCount = %d, m_currentArcRoutingState = %d, m_hdmiInAudioDeviceType = %d",\
				    retryArcCount, m_currentArcRoutingState, m_hdmiInAudioDeviceType);
                    m_ArcDetectionTimer.stop();
            }
	}	
	/* DisplaaySettings gets notified whenever CEC is made Enable or Disable  */
	void DisplaySettings::onCecEnabledEventHandler(const JsonObject& parameters)
	{
             string value;

             LOGINFO(" CEC Enable-Disable Event... \n");
	     if (parameters.HasLabel("cecEnable"))
                 value = parameters["cecEnable"].String();

	     if(!value.compare("true")) {
		isCecEnabled = true;
	      } else{
		isCecEnabled = false;
		try
                    {
                        //if m_arcEarcAudioEnabled == true(case where arc/earc is already routed) we will not reset device type because it will be done from setEnableAudioPort during disable from the connectedAudioPort update
                        if (m_arcEarcAudioEnabled == false && m_hdmiInAudioDeviceType != dsAUDIOARCSUPPORT_NONE) {
                           LOGINFO("Reset m_hdmiInAudioDeviceType since m_arcEarcAudioEnabled = %d", m_arcEarcAudioEnabled);
                           m_hdmiInAudioDeviceType = dsAUDIOARCSUPPORT_NONE;
                        }
                        if(m_hdmiInAudioDeviceConnected ==  true) {
                            m_hdmiInAudioDeviceConnected = false;
                            connectedAudioPortUpdated(dsAUDIOPORT_TYPE_HDMI_ARC, false);
                            m_hdmiInAudioDevicePowerState = AUDIO_DEVICE_POWER_STATE_UNKNOWN;
                        }
                        else {
                            LOGINFO("Skip Disable ARC and not notifying the UI as  m_hdmiInAudioDeviceConnected = false\n");
                        }
                    }
                    catch (const device::Exception& err)
                    {
                        LOG_DEVICE_EXCEPTION1(string("HDMI_ARC0"));
                    }
	      }

              LOGINFO("updated isCecEnabled [%d] ... \n", isCecEnabled);
	}

        void DisplaySettings::stopCecTimeAndUnsubscribeEvent() {
            LOGINFO ("de-init cec timer and subscribbed event \n");
            {
                lock_guard<mutex> lck(m_callMutex);
                if ( m_timer.isActive()) {
                    m_timer.stop();
                }

                if ( m_AudioDeviceDetectTimer.isActive()) {
                    m_AudioDeviceDetectTimer.stop();
                }
		if ( m_SADDetectionTimer.isActive()) {
                        m_SADDetectionTimer.stop();
                }
                if ( m_ArcDetectionTimer.isActive()) {
                    m_ArcDetectionTimer.stop();
                }
                if ( m_AudioDevicePowerOnStatusTimer.isActive()) {
                    m_AudioDevicePowerOnStatusTimer.stop();
                }

                if (nullptr != m_client) {
                    for (std::string eventName : m_clientRegisteredEventNames) {
                        m_client->Unsubscribe(1000, _T(eventName));
                        LOGINFO ("Unsubscribing event %s\n", eventName.c_str());
                    }
                    m_clientRegisteredEventNames.clear();

                    LOGINFO ("deleting m_client \n");
                    delete m_client; m_client = nullptr;
                }
            }
        }

        // 6.
        void DisplaySettings::onTimer()
        {
            // lock to prevent: parallel onTimer runs, destruction during onTimer
            lock_guard<mutex> lck(m_callMutex);

            bool isPluginActivated = false;

            PluginHost::IShell::state state;
            if ((getServiceState(m_service, HDMICECSINK_CALLSIGN, state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED)) {
                LOGINFO("%s is active", HDMICECSINK_CALLSIGN);
                isPluginActivated = true;
            }

            if (!isPluginActivated) {
                /*HDMICECSINK_CALLSIGN plugin activation moved to onTimer.
                 *To decouple from displyasettings init. Since its time taking*/

                activate(m_service, HDMICECSINK_CALLSIGN);

                LOGWARN ("DisplaySettings::onTimer after activatePlugin HDMICECSINK_CALLSIGN line:%d", __LINE__);
                sleep(HDMICECSINK_PLUGIN_ACTIVATION_TIME);
            }

            bool pluginActivated = false;

            if ((getServiceState(m_service, HDMICECSINK_CALLSIGN, state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED)) {
                LOGINFO("%s is active", HDMICECSINK_CALLSIGN);
                pluginActivated = true;
            }

            LOGWARN ("DisplaySettings::onTimer pluginActivated:%d line:%d", pluginActivated, __LINE__);
            if(!m_subscribed) {
                if (pluginActivated && (subscribeForHdmiCecSinkEvent(HDMICECSINK_ARC_INITIATION_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_ARC_TERMINATION_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_SHORT_AUDIO_DESCRIPTOR_EVENT)== Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_SYSTEM_AUDIO_MODE_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_AUDIO_DEVICE_CONNECTED_STATUS_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_CEC_ENABLED_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_AUDIO_DEVICE_POWER_STATUS_EVENT) == Core::ERROR_NONE)&& (subscribeForHdmiCecSinkEvent(HDMICECSINK_ARC_AUDIO_STATUS_EVENT) == Core::ERROR_NONE))
                {
                    m_subscribed = true;
                    if (m_timer.isActive()) {
                        m_timer.stop();
                        LOGINFO("Timer stopped.");
                    }
                    LOGINFO("Subscription completed.");
		    sleep(WARMING_UP_TIME_IN_SECONDS);

                } else {
                    LOGERR("Could not subscribe this time, one more attempt in %d msec. Plugin is %s", RECONNECTION_TIME_IN_MILLISECONDS, pluginActivated ? "ACTIVE" : "BLOCKED");
                }
            } else {
                //Standby ON transitions case
                LOGINFO("Already subscribed. Stopping the timer.");
                if (m_timer.isActive()) {
                    m_timer.stop();
                }
            }

            if(m_subscribed) {
         	//Need to send power on request as this timer might have started based on standby out or boot up scenario
                LOGINFO("%s: Audio Port : [HDMI_ARC0] sendHdmiCecSinkAudioDevicePowerOn !!! \n", __FUNCTION__);
                sendMsgToQueue(SEND_AUDIO_DEVICE_POWERON_MSG, NULL);
		// Some AVR's and SB are not sending response for power on message even though it is in ON state
                // Send power request immediately to query power status of the AVR
                m_hdmiInAudioDevicePowerState = AUDIO_DEVICE_POWER_STATE_REQUEST;
                sendMsgToQueue(REQUEST_AUDIO_DEVICE_POWER_STATUS, NULL);
                LOGINFO("[HDMI_ARC0] sendAudioDevicePowerStatusRequestMsg!!!\n");
            }
        }

        void DisplaySettings::checkAudioDeviceDetectionTimer()
        {
            // lock to prevent: parallel onTimer runs, destruction during onTimer
            lock_guard<mutex> lck(m_callMutex);
            if (m_subscribed && m_hdmiCecAudioDeviceDetected)
            {
               //Connected Audio Ports status update is necessary on bootup / power state transitions
	       m_systemAudioMode_Power_RequestedAndReceived = false;
               LOGINFO("%s: Audio Port : [HDMI_ARC0] sendHdmiCecSinkAudioDevicePowerOn !!! \n", __FUNCTION__);
               sendMsgToQueue(SEND_AUDIO_DEVICE_POWERON_MSG, NULL);
	       LOGINFO("[HDMI_ARC0] Starting the timer to check audio device power status after power on msg!!!\n");
	       m_AudioDevicePowerOnStatusTimer.start(AUDIO_DEVICE_POWER_TRANSITION_TIME_IN_MILLISECONDS);
            } else {
		    LOGINFO("%s: No Audio device detected even after timeout\n", __FUNCTION__);
	    }

            if (m_AudioDeviceDetectTimer.isActive()) {
               m_AudioDeviceDetectTimer.stop();
            }
        }

 void DisplaySettings::checkAudioDevicePowerStatusTimer()
 {

    lock_guard<mutex> lck(m_callMutex);
           if (m_subscribed && m_hdmiCecAudioDeviceDetected)
           {
                // Some AVR's and SB are not sending response for power on message even though it is in ON state
                // Send power request immediately to query power status of the AVR
                LOGINFO("[HDMI_ARC0] m_hdmiInAudioDevicePowerState [%d] \n", m_hdmiInAudioDevicePowerState);
		if (m_hdmiInAudioDevicePowerState != AUDIO_DEVICE_POWER_STATE_ON)
		{
                   m_hdmiInAudioDevicePowerState = AUDIO_DEVICE_POWER_STATE_REQUEST;
		   if ((retryPowerRequestCount == 2) || (retryPowerRequestCount == 4)) // Send Power On msg again for 3rd and 4th iteration
		   {
                        LOGINFO("[HDMI_ARC0] sendHdmiCecSinkAudioDevicePowerOn !!! \n");
                        sendMsgToQueue(SEND_AUDIO_DEVICE_POWERON_MSG, NULL);
		   }
                   sendMsgToQueue(REQUEST_AUDIO_DEVICE_POWER_STATUS, NULL);
		   retryPowerRequestCount++;
                   LOGINFO("[HDMI_ARC0] sendAudioDevicePowerStatusRequestMsg, retryPowerRequestCount [%d]\n", retryPowerRequestCount);
		}
            } else {
                LOGINFO("%s: No Audio device detected\n", __FUNCTION__);
            }

//            if (((m_hdmiInAudioDevicePowerState == AUDIO_DEVICE_POWER_STATE_ON) || (retryPowerRequestCount >= 5)) && m_AudioDevicePowerOnStatusTimer.isActive()) {
            if ((retryPowerRequestCount >= 5) && m_AudioDevicePowerOnStatusTimer.isActive()) {
	       m_systemAudioMode_Power_RequestedAndReceived = true; // resetting the Variable if power status not received.
	       LOGINFO("Stopping timer, Audio Device power status - m_hdmiInAudioDevicePowerState [%d]!!!\n", m_hdmiInAudioDevicePowerState);
               retryPowerRequestCount = 0;
               m_AudioDevicePowerOnStatusTimer.stop();
            }
 }
        // Event management end

        // Thunder plugins communication end


        uint32_t DisplaySettings::getTVHDRCapabilities (const JsonObject& parameters, JsonObject& response) 
        {   //sample servicemanager response:
            LOGINFOMETHOD();
			bool success = true;
			int capabilities = dsHDRSTANDARD_NONE;
            try
            {
                std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
                if (vPort.isDisplayConnected()) {
                    vPort.getTVHDRCapabilities(&capabilities);
                    response["capabilities"] = capabilities;
                }
                else {
					LOGERR("getTVHDRCapabilities failure: HDMI0 not connected!\n");
                    success = false;
                }
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(string("HDMI0"));
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::isConnectedDeviceRepeater (const JsonObject& parameters, JsonObject& response) 
        {   //sample servicemanager response:
            LOGINFOMETHOD();
            bool success = true;
            bool isConnectedDeviceRepeater = false;
            try
            {
                std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
                if (vPort.isDisplayConnected()) {
                    isConnectedDeviceRepeater = vPort.getDisplay().isConnectedDeviceRepeater();
                }
                else {
                    LOGERR("isConnectedDeviceRepeater failure: HDMI0 not connected!\n");
                    success = false;
                }
                response["HdcpRepeater"] = isConnectedDeviceRepeater;
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(string("HDMI0"));
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::getDefaultResolution (const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();
			bool success = true;
            try
            {
                std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
                if (vPort.isDisplayConnected()) {
                    response["defaultResolution"] = vPort.getDefaultResolution().getName();
                }
                else {
					LOGERR("getDefaultResolution failure: HDMI0 not connected!\n");
                    success = false;
                }
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(string("HDMI0"));
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::setScartParameter (const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
            LOGINFOMETHOD();
            returnIfParamNotFound(parameters, "scartParameter");
            returnIfParamNotFound(parameters, "scartParameterData");

            string sScartParameter = parameters["scartParameter"].String();
            string sScartParameterData = parameters["scartParameterData"].String();

            bool success = true;
            try
            {
                device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort("SCART0");
                success = vPort.setScartParameter(sScartParameter, sScartParameterData);
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION2(sScartParameter, sScartParameterData);
                success = false;
            }
            returnResponse(success);
        }
        //End methods

        //Begin events
        void DisplaySettings::resolutionPreChange()
        {
            sendNotify("resolutionPreChange", JsonObject());
        }

        void DisplaySettings::resolutionChanged(int width, int height)
        {
            vector<string> connectedDisplays;
            getConnectedVideoDisplaysHelper(connectedDisplays);

            string firstDisplay = "";
            string firstResolution = "";
            bool firstResolutionSet = false;
            for (int i = 0; i < (int)connectedDisplays.size(); i++)
            {
                string resolution;
                string display = connectedDisplays.at(i);
                try
                {
                    resolution = device::Host::getInstance().getVideoOutputPort(display).getResolution().getName();
                }
                catch(const device::Exception& err)
                {
                    LOG_DEVICE_EXCEPTION1(display);
                }
                if (!resolution.empty())
                {
                    std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                    std::string videoPortName = strVideoPort.substr(0, strVideoPort.size()-1);
                    if (Utils::String::stringContains(display, videoPortName.c_str()))
                    {
                        // only report first HDMI connected device is HDMI is connected
                        JsonObject params;
                        params["width"] = width;
                        params["height"] = height;
                        params["videoDisplayType"] = display;
                        params["resolution"] = resolution;
                        sendNotify("resolutionChanged", params);
                        return;
                    }
                    else if (!firstResolutionSet)
                    {
                        firstDisplay = display;
                        firstResolution = resolution;
                        firstResolutionSet = true;
                    }
                }
            }
            if (firstResolutionSet)
            {
                //if HDMI is not connected then notify the server of first connected device
                JsonObject params;
                params["width"] = width;
                params["height"] = height;
                params["videoDisplayType"] = firstDisplay;
                params["resolution"] = firstResolution;
                sendNotify("resolutionChanged", params);
            }
        }

        void DisplaySettings::zoomSettingUpdated(const string& zoomSetting)
        {//servicemanager sample: {"name":"zoomSettingUpdated","params":{"zoomSetting":"None","success":true,"videoDisplayType":"all"}
         //servicemanager sample: {"name":"zoomSettingUpdated","params":{"zoomSetting":"Full","success":true,"videoDisplayType":"all"}
            JsonObject params;
            params["zoomSetting"] = zoomSetting;
            params["videoDisplayType"] = "all";
            sendNotify("zoomSettingUpdated", params);
        }

        void DisplaySettings::activeInputChanged(bool activeInput)
        {
            JsonObject params;
            params["activeInput"] = activeInput;
            sendNotify("activeInputChanged", params);
        }

        void DisplaySettings::connectedVideoDisplaysUpdated(int hdmiHotPlugEvent)
        {
            static int previousStatus = HDMI_HOT_PLUG_EVENT_CONNECTED;
            static int firstTime = 1;

            if (firstTime || previousStatus != hdmiHotPlugEvent)
            {
                firstTime = 0;
                JsonArray connectedDisplays;
                if (HDMI_HOT_PLUG_EVENT_CONNECTED == hdmiHotPlugEvent)
                {
                    connectedDisplays.Add("HDMI0");
                }
                else
                {
                    /* notify Empty list on HDMI-output-disconnect hotplug */
                }

                JsonObject params;
                params["connectedVideoDisplays"] = connectedDisplays;
                sendNotify("connectedVideoDisplaysUpdated", params);
            }
            previousStatus = hdmiHotPlugEvent;
        }

        void DisplaySettings::connectedAudioPortUpdated (int iAudioPortType, bool isPortConnected)
        {
            JsonObject params;
            string sPortName;
            string sPortStatus;
            switch (iAudioPortType)
            {
                case dsAUDIOPORT_TYPE_HDMI_ARC:
                    params["HotpluggedAudioPort"] = "HDMI_ARC0";
                    sPortName.assign ("HDMI_ARC0");
                    break;
                case dsAUDIOPORT_TYPE_HEADPHONE:
                    params["HotpluggedAudioPort"] = "HEADPHONE0";
                    sPortName.assign ("HEADPHONE0");
                    break;

                default:
                    //do nothing
                	break;
            }

            if (1 == isPortConnected)
            {
                params["isConnected"] = "connected";
                sPortStatus.assign ("connected");
            }
            else
            {
                params["isConnected"] = "disconnected";
                sPortStatus.assign ("disconnected");
            }
            LOGWARN ("Thunder sends notification %s audio port hotplug status %s", sPortName.c_str(), sPortStatus.c_str());
            sendNotify("connectedAudioPortUpdated", params);
        }

        //End events

        void DisplaySettings::getConnectedVideoDisplaysHelper(vector<string>& connectedDisplays)
        {
            try
            {
                device::List<device::VideoOutputPort> vPorts = device::Host::getInstance().getVideoOutputPorts();
                for (size_t i = 0; i < vPorts.size(); i++)
                {
                    device::VideoOutputPort &vPort = vPorts.at(i);
                    if (vPort.isDisplayConnected())
                    {
                        string displayName = vPort.getName();
                        if (strncasecmp(displayName.c_str(), "hdmi", 4)==0)
                        {
                            connectedDisplays.clear();
                            connectedDisplays.emplace_back(displayName);
                            break;
                        }
                        else
                        {
                            vectorSet(connectedDisplays, displayName);
                        }
                    }
                }
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
            }
        }

        bool DisplaySettings::checkPortName(std::string& name) const
        {
            if (Utils::String::stringContains(name,"HDMI")) {
		if(Utils::String::stringContains(name,"HDMI_ARC"))
                    name = "HDMI_ARC0";
		else
		    name = "HDMI0";
            }
            else if (Utils::String::stringContains(name,"SPDIF"))
                name = "SPDIF0";
            else if (Utils::String::stringContains(name,"IDLR"))
                name = "IDLR0";
            else if (Utils::String::stringContains(name,"SPEAKER"))
                name = "SPEAKER0";
            else if (!name.empty()) // Empty is allowed
                return false;

            return true;
        }

        uint32_t DisplaySettings::resetDialogEnhancement(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool success = true;
            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                aPort.resetDialogEnhancement();
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(audioPort);
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::resetBassEnhancer(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool success = true;
            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                aPort.resetBassEnhancer();
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(audioPort);
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::resetSurroundVirtualizer(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool success = true;
            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                aPort.resetSurroundVirtualizer();
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(audioPort);
                success = false;
            }
            returnResponse(success);
        }

        uint32_t DisplaySettings::resetVolumeLeveller(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool success = true;
            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                aPort.resetVolumeLeveller();
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(audioPort);
                success = false;
            }
            returnResponse(success);
        }
        uint32_t DisplaySettings::getVideoFormat(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:{"currentVideoFormat":"SDR","supportedVideoFormat":["SDR","HDR10","HLG","DV","Technicolor Prime"],"success":true}
            LOGINFOMETHOD();

            try
            {
                std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
                if (vPort.isDisplayConnected())
                {
                    int _eotf = vPort.getVideoEOTF();
                    response["currentVideoFormat"] = getVideoFormatTypeToString((dsHDRStandard_t)_eotf);
                }
                else
                {
                    response["currentVideoFormat"] = "NONE";
                }

	    }
	    catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
                response["currentVideoFormat"] = "NONE";
            }


            response["supportedVideoFormat"] = getSupportedVideoFormats();
            returnResponse(true);
        }

        JsonArray DisplaySettings::getSupportedVideoFormats()
        {
            JsonArray videoFormats;
            int capabilities = dsHDRSTANDARD_NONE;

            try
            {
                if (device::Host::getInstance().getVideoDevices().size() < 1)
                {
                    LOGINFO("DSMGR_NOT_RUNNING");
                    return videoFormats;
                }

                device::VideoDevice &device = device::Host::getInstance().getVideoDevices().at(0);
                device.getHDRCapabilities(&capabilities);
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
                return videoFormats;
            }

            if(capabilities & dsHDRSTANDARD_HDR10)videoFormats.Add("HDR10");
            if(capabilities & dsHDRSTANDARD_HLG)videoFormats.Add("HLG");
            if(capabilities & dsHDRSTANDARD_DolbyVision)videoFormats.Add("DV");
            if(capabilities & dsHDRSTANDARD_TechnicolorPrime)videoFormats.Add("Technicolor Prime");
            if(capabilities & dsHDRSTANDARD_HDR10PLUS)videoFormats.Add("HDR10PLUS");
            if(capabilities & dsHDRSTANDARD_SDR)videoFormats.Add("SDR");
            for (uint32_t i = 0; i < videoFormats.Length(); i++)
            {
               LOGINFO("capabilities: %s", videoFormats[i].String().c_str());
            }
            return videoFormats;
        }

        const char *DisplaySettings::getVideoFormatTypeToString(dsHDRStandard_t format)
        {
            const char *strValue = "NONE";
            switch (format)
            {
                case dsHDRSTANDARD_SDR:
                    LOGINFO("Video Format: SDR\n");
                    strValue = "SDR";
                    break;
                case dsHDRSTANDARD_HDR10:
                    LOGINFO("Video Format: HDR10\n");
                    strValue = "HDR10";
                    break;
                case dsHDRSTANDARD_HDR10PLUS:
                    LOGINFO("Video Format: HDR10PLUS\n");
                    strValue = "HDR10PLUS";
                    break;
                case dsHDRSTANDARD_HLG:
                    LOGINFO("Video Format: HLG\n");
                    strValue = "HLG";
                    break;
                case dsHDRSTANDARD_DolbyVision:
                    LOGINFO("Video Format: DV\n");
                    strValue = "DV";
                    break;
                case dsHDRSTANDARD_TechnicolorPrime:
                    LOGINFO("Video Format: TechnicolorPrime\n");
                    strValue = "TechnicolorPrime";
                    break;
                default:
                    LOGINFO("Video Format: NONE\n");
                    strValue = "NONE";
                    break;
            }
            return strValue;

        }
	dsHDRStandard_t DisplaySettings::getVideoFormatTypeFromString(const char *strFormat)
        {
           dsHDRStandard_t mode = dsHDRSTANDARD_NONE;
            if(strcmp(strFormat,"SDR")== 0 )
                    mode = dsHDRSTANDARD_SDR;
            else if(strcmp(strFormat,"NONE")== 0)
                    mode = dsHDRSTANDARD_NONE;
            else if(strcmp(strFormat,"HDR10")== 0)
                    mode = dsHDRSTANDARD_HDR10;
            else if(strcmp(strFormat,"HDR10PLUS")== 0)
                    mode = dsHDRSTANDARD_HDR10PLUS;
            else if(strcmp(strFormat,"DV")== 0)
                    mode = dsHDRSTANDARD_DolbyVision;
            else if(strcmp(strFormat,"HLG")== 0)
                    mode = dsHDRSTANDARD_TechnicolorPrime;
            else if(strcmp(strFormat,"TechnicolorPrime")== 0)
                    mode = dsHDRSTANDARD_NONE;
	    else
		    mode = dsHDRSTANDARD_Invalid;

	    return mode;
        }
    } // namespace Plugin
} // namespace WPEFramework
