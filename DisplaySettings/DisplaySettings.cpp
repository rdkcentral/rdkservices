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
#include "libIBusDaemon.h"
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
#include "libIBus.h"
#include "libIBusDaemon.h"
#include "dsDisplay.h"
#include "rdk/iarmmgrs-hal/pwrMgr.h"
#include "pwrMgr.h"

#include "tr181api.h"

#include "tracing/Logging.h"
#include <syscall.h>
#include "utils.h"
#include "dsError.h"

using namespace std;

#define HDMI_HOT_PLUG_EVENT_CONNECTED 0

#define HDMI_IN_ARC_PORT_ID 1


#define HDMICECSINK_CALLSIGN "org.rdk.HdmiCecSink"
#define HDMICECSINK_CALLSIGN_VER HDMICECSINK_CALLSIGN".1"
#define HDMICECSINK_ARC_INITIATION_EVENT "arcInitiationEvent"
#define HDMICECSINK_ARC_TERMINATION_EVENT "arcTerminationEvent"
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

#define ZOOM_SETTINGS_FILE      "/opt/persistent/rdkservices/zoomSettings.json"
#define ZOOM_SETTINGS_DIRECTORY "/opt/persistent/rdkservices"

static bool isCecArcRoutingThreadEnabled = false;
static bool isCecEnabled = false;

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

namespace WPEFramework {

    namespace Plugin {

        SERVICE_REGISTRATION(DisplaySettings, 1, 0);

        DisplaySettings* DisplaySettings::_instance = nullptr;
        IARM_Bus_PWRMgr_PowerState_t DisplaySettings::m_powerState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY;

        DisplaySettings::DisplaySettings()
            : AbstractPlugin(2)
        {
            LOGINFO("ctor");
            DisplaySettings::_instance = this;

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

            registerMethod("getVolumeLeveller", &DisplaySettings::getVolumeLeveller, this, {1});
            registerMethod("getBassEnhancer", &DisplaySettings::getBassEnhancer, this);
            registerMethod("isSurroundDecoderEnabled", &DisplaySettings::isSurroundDecoderEnabled, this);
            registerMethod("getDRCMode", &DisplaySettings::getDRCMode, this);
            registerMethod("getSurroundVirtualizer", &DisplaySettings::getSurroundVirtualizer, this, {1});
            registerMethod("setVolumeLeveller", &DisplaySettings::setVolumeLeveller, this, {1});
            registerMethod("setBassEnhancer", &DisplaySettings::setBassEnhancer, this);
            registerMethod("enableSurroundDecoder", &DisplaySettings::enableSurroundDecoder, this);
            registerMethod("setSurroundVirtualizer", &DisplaySettings::setSurroundVirtualizer, this, {1});
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

	    registerMethod("getVolumeLeveller", &DisplaySettings::getVolumeLeveller2, this, {2});
	    registerMethod("setVolumeLeveller", &DisplaySettings::setVolumeLeveller2, this, {2});
	    registerMethod("getSurroundVirtualizer", &DisplaySettings::getSurroundVirtualizer2, this, {2});
	    registerMethod("setSurroundVirtualizer", &DisplaySettings::setSurroundVirtualizer2, this, {2});
            registerMethod("getVideoFormat", &DisplaySettings::getVideoFormat, this);

            registerMethod("setPreferredColorDepth", &DisplaySettings::setPreferredColorDepth, this);
            registerMethod("getPreferredColorDepth", &DisplaySettings::getPreferredColorDepth, this);
            registerMethod("getColorDepthCapabilities", &DisplaySettings::getColorDepthCapabilities, this);

	    m_subscribed = false; //HdmiCecSink event subscription
	    m_hdmiInAudioDeviceConnected = false;
        m_arcAudioEnabled = false;
	    m_hdmiCecAudioDeviceDetected = false;
            m_hdmiInAudioDevicePowerState = AUDIO_DEVICE_POWER_STATE_UNKNOWN;
	    m_currentArcRoutingState = ARC_STATE_ARC_TERMINATED;
	    m_cecArcRoutingThreadRun = false;
	    isCecArcRoutingThreadEnabled = true;
	    m_arcRoutingThread = std::thread(cecArcRoutingThread);
	    m_timer.connect(std::bind(&DisplaySettings::onTimer, this));
            m_AudioDeviceDetectTimer.connect(std::bind(&DisplaySettings::checkAudioDeviceDetectionTimer, this));
        }

        DisplaySettings::~DisplaySettings()
        {
            LOGINFO("dtor");
            lock_guard<mutex> lck(m_callMutex);
            if ( m_timer.isActive()) {
                m_timer.stop();
            }

            if ( m_AudioDeviceDetectTimer.isActive()) {
                m_AudioDeviceDetectTimer.stop();
            }
        }

        void DisplaySettings::AudioPortsReInitialize()
        {
            LOGINFO("Entering DisplaySettings::AudioPortsReInitialize");
            uint32_t ret = Core::ERROR_NONE;
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
                        //Set audio port config. ARC will be set up by onTimer()
                        #ifdef APP_CONTROL_AUDIOPORT_INIT
                        if(isPortPersistenceValEnabled ) {
                            LOGWARN("Audio Port : APP_CONTROL_AUDIOPORT_INIT Enabled\n");
                        #else if
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

			bool isPluginActivated = Utils::isPluginActivated(HDMICECSINK_CALLSIGN);

			if(isPluginActivated) {
			    if(!m_subscribed) {
			        if((subscribeForHdmiCecSinkEvent(HDMICECSINK_ARC_INITIATION_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_ARC_TERMINATION_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_SHORT_AUDIO_DESCRIPTOR_EVENT)== Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_SYSTEM_AUDIO_MODE_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_AUDIO_DEVICE_CONNECTED_STATUS_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_CEC_ENABLED_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_AUDIO_DEVICE_POWER_STATUS_EVENT) == Core::ERROR_NONE)) {
                                    m_subscribed = true;
                                    LOGINFO("%s: HdmiCecSink event subscription completed.\n",__FUNCTION__);
			        }
			    }

			    if(m_subscribed) {
				JsonObject aPortArcEnableResult;
				JsonObject aPortArcEnableParam;
				aPortArcEnableParam.Set(_T("audioPort"),"HDMI_ARC0");
				bool arcEnable = m_audioOutputPortConfig["HDMI_ARC"].Boolean();
                                aPortArcEnableParam.Set(_T("enable"), arcEnable);
                                ret = setEnableAudioPort (aPortArcEnableParam, aPortArcEnableResult);
                                if(ret != Core::ERROR_NONE) {
                                    LOGWARN("%s: Audio Port : [HDMI_ARC0] enable: %d failed ! error code%d\n", __FUNCTION__, arcEnable, ret);
                                }
                                else {
                                    LOGINFO("%s: Audio Port : [HDMI_ARC0] initialized successfully, enable: %d\n", __FUNCTION__, arcEnable);
                                }

				LOGINFO("m_hdmiCecAudioDeviceDetected status [%d] ... \n", m_hdmiCecAudioDeviceDetected);
			     if (m_hdmiCecAudioDeviceDetected)
			     {
                                //Connected Audio Ports status update is necessary on bootup / power state transitions
				sendHdmiCecSinkAudioDevicePowerOn();
				LOGINFO("%s: Audio Port : [HDMI_ARC0] sendHdmiCecSinkAudioDevicePowerOn !!! \n", __FUNCTION__);
                                try {
                                    int types = dsAUDIOARCSUPPORT_NONE;
                                    device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI_ARC0");
                                    aPort.getSupportedARCTypes(&types);
                                    if(types & dsAUDIOARCSUPPORT_eARC) {
                                        m_hdmiInAudioDeviceConnected = true;
                                        connectedAudioPortUpdated(dsAUDIOPORT_TYPE_HDMI_ARC, true);
                                    }
                                    else if (types & dsAUDIOARCSUPPORT_ARC) {
                                        //Dummy ARC intiation request
                                       {
                                        std::lock_guard<std::mutex> lock(m_arcRoutingStateMutex);
                                        if((m_currentArcRoutingState == ARC_STATE_ARC_TERMINATED) && (isCecEnabled == true)) {
                                            LOGINFO("%s: Send dummy ARC initiation request... \n", __FUNCTION__);
                                            m_currentArcRoutingState = ARC_STATE_REQUEST_ARC_INITIATION;
                                            m_cecArcRoutingThreadRun = true;
                                            arcRoutingCV.notify_one();
                                        }
                                       }
                                    }
                                    else {
                                        LOGINFO("%s: Connected Device doesn't have ARC/eARC capability... \n", __FUNCTION__);
                                    }
                                }
                                catch (const device::Exception& err){
                                    LOG_DEVICE_EXCEPTION1(string("HDMI_ARC0"));
                                }
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
                        #else if
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

        const string DisplaySettings::Initialize(PluginHost::IShell* /* service */)
        {
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

        void DisplaySettings::Deinitialize(PluginHost::IShell* /* service */)
        {
	   LOGINFO("Enetering DisplaySettings::Deinitialize");
	   isCecArcRoutingThreadEnabled = false;
	   {
            std::lock_guard<std::mutex> lock(m_arcRoutingStateMutex);
            LOGINFO("DisplaySettings::Deinitialize %d", __LINE__);
            m_currentArcRoutingState = ARC_STATE_ARC_EXIT;
	    m_cecArcRoutingThreadRun = true;
            arcRoutingCV.notify_one();
	   }

            try
            {
                if (m_arcRoutingThread.joinable())
                        m_arcRoutingThread.join();
            }
            catch(const std::system_error& e)
            {
                LOGERR("system_error exception in thread join %s", e.what());
            }
            catch(const std::exception& e)
            {
                LOGERR("exception in thread join %s", e.what());
            }

            DeinitializeIARM();
            DisplaySettings::_instance = nullptr;
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

                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_RX_SENSE) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_ZOOM_SETTINGS) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_RES_PRECHANGE) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG) );
		IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_AUDIO_OUT_HOTPLUG) );
		IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_DSMGR_EVENT_AUDIO_FORMAT_UPDATE) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_AUDIO_PORT_STATE) );
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

		    if(hdmiin_hotplug_port == HDMI_IN_ARC_PORT_ID) { //HDMI ARC/eARC Port Handling
			bool arc_port_enabled =  false;

                        JsonObject audioOutputPortConfig = DisplaySettings::_instance->getAudioOutputPortConfig();
			if (audioOutputPortConfig.HasLabel("HDMI_ARC")) {
                            try {
                                    arc_port_enabled = audioOutputPortConfig["HDMI_ARC"].Boolean();
                            }catch (const device::Exception& err) {
                                    LOGERR("HDMI_ARC not in config object \n");
                                    arc_port_enabled = false;
                            }
			}

			try
			{
                            int types = dsAUDIOARCSUPPORT_NONE;
                            device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI_ARC0");
                            LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG  HDMI_ARC Port, connected:%d \n",  hdmiin_hotplug_conn);
                            if(hdmiin_hotplug_conn) {
                                aPort.getSupportedARCTypes(&types);
                                LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG  HDMI_ARC Port, types: %d \n",  types);
			    }
                            else {
                                if (DisplaySettings::_instance->m_hdmiInAudioDeviceConnected == true) {
                                    DisplaySettings::_instance->m_hdmiInAudioDeviceConnected = false;
                                    DisplaySettings::_instance->m_hdmiInAudioDevicePowerState = AUDIO_DEVICE_POWER_STATE_UNKNOWN;
                                    DisplaySettings::_instance->connectedAudioPortUpdated(dsAUDIOPORT_TYPE_HDMI_ARC, hdmiin_hotplug_conn);
                                    LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG  HDMI_ARC Port disconnected. Notify UI !!!  \n");
                                }

                                if(DisplaySettings::_instance->m_arcAudioEnabled == true) {
                                    aPort.enableARC(dsAUDIOARCSUPPORT_ARC, false);
                                    DisplaySettings::_instance->m_arcAudioEnabled = false;
                                    LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG  HDMI_ARC Port disconnected. Disable ARC/eARC !!!  \n");
                                }

                                {
                                   std::lock_guard<std::mutex> lock(DisplaySettings::_instance->m_arcRoutingStateMutex);
                                   DisplaySettings::_instance->m_currentArcRoutingState = ARC_STATE_ARC_TERMINATED;
                                }

                            }
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
                    LOGINFO("Received IARM_BUS_DSMGR_EVENT_AUDIO_PRIMARY_LANGUAGE_CHANGED. Primary Language: %s \n", pLang);
                    if(DisplaySettings::_instance) {
                        DisplaySettings::_instance->notifyPrimaryLanguageChange(pLang);
                    }
                  }
                  break;
                case IARM_BUS_DSMGR_EVENT_AUDIO_SECONDARY_LANGUAGE_CHANGED:
                  {
                    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                    std::string sLang = eventData->data.AudioLanguageInfo.audioLanguage;
                    LOGINFO("Received IARM_BUS_DSMGR_EVENT_AUDIO_SECONDARY_LANGUAGE_CHANGED. Secondary Language: %s \n", sLang);
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
                    if (aPort.isConnected())
                    {
                        string portName = aPort.getName();
                        if((portName == "HDMI_ARC0") && (m_hdmiInAudioDeviceConnected != true)) {
                            continue;
                        }
                        vectorSet(connectedAudioPorts, portName);
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
                if(tvResolutions & dsTV_RESOLUTION_720p)supportedTvResolutions.emplace_back("720p");
                if(tvResolutions & dsTV_RESOLUTION_1080i)supportedTvResolutions.emplace_back("1080i");
                if(tvResolutions & dsTV_RESOLUTION_1080p)supportedTvResolutions.emplace_back("1080p");
                if(tvResolutions & dsTV_RESOLUTION_2160p30)supportedTvResolutions.emplace_back("2160p30");
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

                            int types = dsAUDIOARCSUPPORT_NONE;
                            aPort.getSupportedARCTypes(&types);

		            if(((mode == device::AudioStereoMode::kSurround) || (mode == device::AudioStereoMode::kPassThru) || (mode == device::AudioStereoMode::kStereo)) && (stereoAuto == false)) {
				    aPort.setStereoAuto(false, persist);

				    if((mode == device::AudioStereoMode::kPassThru) && (types & dsAUDIOARCSUPPORT_ARC) && (m_hdmiInAudioDeviceConnected == true)) {
                                        if (!DisplaySettings::_instance->requestShortAudioDescriptor()) {
                                            success = false;
                                            LOGERR("setSoundMode Passthru: requestShortAudioDescriptor failed !!!\n");;
                                        }
                                        else {
                                            LOGINFO("setSoundMode Passthru: requestShortAudioDescriptor successful\n");
                                        }
                                    }
				    aPort.setStereoMode(mode.toString(), persist);
		            }
			    else { //Auto Mode

				if(types & dsAUDIOARCSUPPORT_eARC) {
				    aPort.setStereoAuto(stereoAuto, persist); //setStereoAuto true
				}
				else if ((types & dsAUDIOARCSUPPORT_ARC) && (m_hdmiInAudioDeviceConnected == true)) {
                                    if (!DisplaySettings::_instance->requestShortAudioDescriptor()) {
                                        success = false;
                                        LOGERR("setSoundMode Auto: requestShortAudioDescriptor failed !!!\n");;
                                    }
                                    else {
                                        LOGINFO("setSoundMode Auto: requestShortAudioDescriptor successful\n");
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
	    if(capabilities & dsHDRSTANDARD_HLG)hdrCapabilities.Add("HLG");
            if(capabilities & dsHDRSTANDARD_DolbyVision)hdrCapabilities.Add("Dolby Vision");
            if(capabilities & dsHDRSTANDARD_TechnicolorPrime)hdrCapabilities.Add("Technicolor Prime");

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
	    if(capabilities & dsHDRSTANDARD_HLG)hdrCapabilities.Add("HLG");
            if(capabilities & dsHDRSTANDARD_DolbyVision)hdrCapabilities.Add("Dolby Vision");
            if(capabilities & dsHDRSTANDARD_TechnicolorPrime)hdrCapabilities.Add("Technicolor Prime");

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
            IARM_Bus_PWRMgr_StandbyVideoState_Param_t param;
            param.isEnabled = enabled;
            strncpy(param.port, portname.c_str(), PWRMGR_MAX_VIDEO_PORT_NAME_LENGTH);
            bool success = true;
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
            returnResponse(success);
        }

        uint32_t DisplaySettings::getVideoPortStatusInStandby(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            returnIfParamNotFound(parameters, "portName");
            string portname = parameters["portName"].String();

            bool success = true;
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
            LOGINFOMETHOD();
            bool success = true;
            float level = 0;

            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                level = aPort.getLevel();
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
                }
                catch (const device::Exception& err)
                {
                    LOG_DEVICE_EXCEPTION2(audioPort, sMuted);
                    success = false;
                }
                returnResponse(success);
        }

        uint32_t DisplaySettings::setVolumeLevel(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();
                returnIfParamNotFound(parameters, "volumeLevel");
                string sLevel = parameters["volumeLevel"].String();
                float level = 0;
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
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                        aPort.setLevel(level);
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
			dsATMOSCapability_t atmosCapability;
            try
            {
                if (device::Host::getInstance().isHDMIOutPortPresent())
                {
                    device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI0");
                    if (aPort.isConnected()) {
                        aPort.getSinkDeviceAtmosCapability (atmosCapability);
                        response["atmos_capability"] = (int)atmosCapability;
                    }
                    else {
                        LOGERR("getSinkAtmosCapability failure: HDMI0 not connected!\n");
                        success = false;
                    }
                }
                else {
                    device::Host::getInstance().getSinkDeviceAtmosCapability (atmosCapability);
                    response["atmos_capability"] = (int)atmosCapability;
                }
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(string("HDMI0"));
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

            if (Utils::isPluginActivated(HDMICECSINK_CALLSIGN)) {
                auto hdmiCecSinkPlugin = getHdmiCecSinkPlugin();
                if (!hdmiCecSinkPlugin) {
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
                    hdmiCecSinkPlugin->Invoke<JsonObject, JsonObject>(2000, "setupARCRouting", param, hdmiCecSinkResult);
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
            bool success = true;
	    bool cecEnable = false;

            if (Utils::isPluginActivated(HDMICECSINK_CALLSIGN)) {
                auto hdmiCecSinkPlugin = getHdmiCecSinkPlugin();
                if (!hdmiCecSinkPlugin) {
                    LOGERR("HdmiCecSink Initialisation failed\n");
                }
                else {
                    JsonObject hdmiCecSinkResult;
                    JsonObject param;

                    hdmiCecSinkPlugin->Invoke<JsonObject, JsonObject>(2000, "getEnabled", param, hdmiCecSinkResult);

		    cecEnable = hdmiCecSinkResult["enabled"].Boolean();
		    LOGINFO("get-cecEnabled [%d]\n",cecEnable);

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
            return cecEnable;
        }

	bool DisplaySettings::getHdmiCecSinkAudioDeviceConnectedStatus ()
        {
            bool success = true;
            bool hdmiAudioDeviceDetected = false;

            if (Utils::isPluginActivated(HDMICECSINK_CALLSIGN)) {
                auto hdmiCecSinkPlugin = getHdmiCecSinkPlugin();
                if (!hdmiCecSinkPlugin) {
                    LOGERR("HdmiCecSink Initialisation failed\n");
                }
                else {
                    JsonObject hdmiCecSinkResult;
                    JsonObject param;

                    hdmiCecSinkPlugin->Invoke<JsonObject, JsonObject>(2000, "getAudioDeviceConnectedStatus", param, hdmiCecSinkResult);

                    hdmiAudioDeviceDetected = hdmiCecSinkResult["connected"].Boolean();
                    LOGINFO("getAudioDeviceConnectedStatus [%d]\n",hdmiAudioDeviceDetected);

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
            return hdmiAudioDeviceDetected;
        }

        bool DisplaySettings::sendHdmiCecSinkAudioDevicePowerOn ()
        {
            bool success = true;

            if (Utils::isPluginActivated(HDMICECSINK_CALLSIGN)) {
                auto hdmiCecSinkPlugin = getHdmiCecSinkPlugin();
                if (!hdmiCecSinkPlugin) {
                    LOGERR("HdmiCecSink Initialisation failed\n");
                }
                else {
                    JsonObject hdmiCecSinkResult;
                    JsonObject param;

                    LOGINFO("%s: Send Audio Device Power On !!!\n");
                    hdmiCecSinkPlugin->Invoke<JsonObject, JsonObject>(2000, "sendAudioDevicePowerOnMessage", param, hdmiCecSinkResult);
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

            if (Utils::isPluginActivated(HDMICECSINK_CALLSIGN)) {
                auto hdmiCecSinkPlugin = getHdmiCecSinkPlugin();
                if (!hdmiCecSinkPlugin) {
                    LOGERR("HdmiCecSink plugin not accessible\n");
                }
                else {
                    JsonObject hdmiCecSinkResult;
                    JsonObject param;

                    LOGINFO("Requesting Short Audio Descriptor \n");
                    hdmiCecSinkPlugin->Invoke<JsonObject, JsonObject>(2000, "requestShortAudioDescriptor", param, hdmiCecSinkResult);
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

            if (Utils::isPluginActivated(HDMICECSINK_CALLSIGN)) {
                auto hdmiCecSinkPlugin = getHdmiCecSinkPlugin();
                if (!hdmiCecSinkPlugin) {
                    LOGERR("HdmiCecSink plugin not accessible\n");
                }
                else {
                    JsonObject hdmiCecSinkResult;
                    JsonObject param;

                    LOGINFO("Requesting Audio Device power Status \n");
                    hdmiCecSinkPlugin->Invoke<JsonObject, JsonObject>(2000, "requestAudioDevicePowerStatus", param, hdmiCecSinkResult);
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
                else {
                    int types = dsAUDIOARCSUPPORT_NONE;

                    device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);

                    aPort.getSupportedARCTypes(&types);
                    if((aPort.isConnected()) && (m_hdmiInAudioDeviceConnected == true)) {
                        LOGINFO("DisplaySettings::setEnableAudioPort Configuring User set Audio mode before starting ARC/eARC Playback...\n");
                        if(aPort.getStereoAuto() == true) {
                            if(types & dsAUDIOARCSUPPORT_eARC) {
                                aPort.setStereoAuto(true,true);
                            }
                            else if (types & dsAUDIOARCSUPPORT_ARC) {
                                if (!DisplaySettings::_instance->requestShortAudioDescriptor()) {
                                    LOGERR("DisplaySettings::setEnableAudioPort (ARC-Auto): requestShortAudioDescriptor failed !!!\n");;
                                }
                                else {
                                    LOGINFO("DisplaySettings::setEnableAudioPort (ARC-Auto): requestShortAudioDescriptor successful\n");
                                }
                            }
                        }
                        else{
                            device::AudioStereoMode mode = device::AudioStereoMode::kStereo;  //default to stereo
                            mode = aPort.getStereoMode(); //get Last User set stereo mode and set
                            if((mode == device::AudioStereoMode::kPassThru) && (types & dsAUDIOARCSUPPORT_ARC)){
                                if (!DisplaySettings::_instance->requestShortAudioDescriptor()) {
                                    LOGERR("DisplaySettings::setEnableAudioPort (ARC-Passthru): requestShortAudioDescriptor failed !!!\n");;
                                }
                                else {
                                    LOGINFO("DisplaySettings::setEnableAudioPort (ARC-Passthru): requestShortAudioDescriptor successful\n");
                                }
                            }
                            aPort.setStereoMode(mode.toString(), true);
                        }
                    }

                    if(types & dsAUDIOARCSUPPORT_eARC) {
                        if(pEnable) {
                            LOGINFO("DisplaySettings::setEnableAudioPort Enable eARC !!!");
                            aPort.enableARC(dsAUDIOARCSUPPORT_eARC, true);
                            m_arcAudioEnabled = true;
                        }
                        else{
                            LOGINFO("DisplaySettings::setEnableAudioPort Disable eARC !!!");
                            aPort.enableARC(dsAUDIOARCSUPPORT_eARC, false);
                            m_arcAudioEnabled = false;
                        }
                    }
                    else if(types & dsAUDIOARCSUPPORT_ARC) {
                       LOGINFO("%s: Device Type ARC. m_hdmiInAudioDeviceConnected: %d , pEnable: %d \n",__FUNCTION__,m_hdmiInAudioDeviceConnected, pEnable);
                       if( m_hdmiInAudioDeviceConnected == true ) {
                           if(pEnable) {
                               LOGINFO("%s: CEC ARC handshake already completed. Enable ARC... \n",__FUNCTION__);
			       // For certain ARC devices, we get ARC initiate message even when ARC device is in standby
			       // Wake up the device always before audio routing
			       sendHdmiCecSinkAudioDevicePowerOn();
                               aPort.enableARC(dsAUDIOARCSUPPORT_ARC, true);
                               m_arcAudioEnabled = true;
			   }
			   else {
                               LOGINFO("%s: Disable ARC \n",__FUNCTION__);
                               aPort.enableARC(dsAUDIOARCSUPPORT_ARC, false);
                               m_arcAudioEnabled = false;
                           }
                       }
                       else {
                            if (pEnable) {
                                LOGINFO("%s: setUpHdmiCecSinkArcRouting true. Audio routing after CEC ARC handshake \n",__FUNCTION__);
                                {
                                    std::lock_guard<std::mutex> lock(m_arcRoutingStateMutex);
                                    if((m_currentArcRoutingState == ARC_STATE_ARC_TERMINATED) && (isCecEnabled == true)) {
                                        m_currentArcRoutingState = ARC_STATE_REQUEST_ARC_INITIATION;
                                        m_cecArcRoutingThreadRun = true;
                                        arcRoutingCV.notify_one();
                                    }
                                    else {
                                        LOGINFO("%s: ARC State is already either initiating/intitiated... \n", __FUNCTION__);
                                    }
                                }
                            }
                            else {
                                LOGINFO("%s: No handling required\n");
                            }
                       }
                    }
                    else {
                        LOGWARN("DisplaySettings::setEnableAudioPort Connected device doesn't have ARC/eARC capability \n");
                    }

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
        std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> DisplaySettings::getHdmiCecSinkPlugin()
        {
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));
            return make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>>("org.rdk.HdmiCecSink.1", "");
        }

        std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> DisplaySettings::getSystemPlugin()
        {
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));
            return make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>>("org.rdk.System.1", "");
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
                  {
		            std::lock_guard<std::mutex> lock(DisplaySettings::_instance->m_arcRoutingStateMutex);
                    LOGINFO("%s: Cleanup ARC/eARC state\n",__FUNCTION__);
                    if(DisplaySettings::_instance->m_currentArcRoutingState != ARC_STATE_ARC_TERMINATED)
                        DisplaySettings::_instance->m_currentArcRoutingState = ARC_STATE_ARC_TERMINATED;

                    if(DisplaySettings::_instance->m_hdmiInAudioDeviceConnected !=  false)
                        DisplaySettings::_instance->m_hdmiInAudioDeviceConnected =  false;
                  }

		  {
                    std::lock_guard<mutex> lck(DisplaySettings::_instance->m_callMutex);
                    if ( DisplaySettings::_instance->m_timer.isActive()) {
                        DisplaySettings::_instance->m_timer.stop();
                    }

                    if ( DisplaySettings::_instance->m_AudioDeviceDetectTimer.isActive()) {
                        DisplaySettings::_instance->m_AudioDeviceDetectTimer.stop();
                    }
                  }

                    if(DisplaySettings::_instance->m_arcAudioEnabled == true) {
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI_ARC0");
                        LOGINFO("%s: Disable ARC/eARC Audio\n",__FUNCTION__);
                        aPort.enableARC(dsAUDIOARCSUPPORT_ARC, false);
                        DisplaySettings::_instance->m_arcAudioEnabled = false;
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


	//Displaysettings ARC Routing thread
	void DisplaySettings::cecArcRoutingThread() {
            LOGINFO("%s: ARC Routing Thread Start\n",__FUNCTION__);
	    bool threadExit = false;
	    int arcState = ARC_STATE_ARC_TERMINATED;
            int audioDevicePowerState = AUDIO_DEVICE_POWER_STATE_STANDBY;

            if(!DisplaySettings::_instance)
                 return;
	    
	    while(isCecArcRoutingThreadEnabled) {

		LOGINFO("%s: Debug:  ARC Routing Thread wait \n",__FUNCTION__);
		{
	    	std::unique_lock<std::mutex> lock(DisplaySettings::_instance->m_arcRoutingStateMutex);
		DisplaySettings::_instance->arcRoutingCV.wait(lock, []{return (DisplaySettings::_instance->m_cecArcRoutingThreadRun == true);});
		arcState = DisplaySettings::_instance->m_currentArcRoutingState;
                audioDevicePowerState = DisplaySettings::_instance->m_hdmiInAudioDevicePowerState;
		}
                if(threadExit == true) {
                    break;
		}

		switch(audioDevicePowerState) {
                    case AUDIO_DEVICE_POWER_STATE_REQUEST:
                        LOGINFO("%s: Send Audio Device Power state request \n",__FUNCTION__);
                        DisplaySettings::_instance->requestAudioDevicePowerStatus();
                        DisplaySettings::_instance->m_hdmiInAudioDevicePowerState = AUDIO_DEVICE_POWER_STATE_UNKNOWN;
                        break;
                    default:
                        LOGINFO("%s: Default case - No Audio device power request sent. Current  audioDevicePowerState : %d \n",__FUNCTION__, audioDevicePowerState);
                        break;
                }
		
                switch(arcState) {

                    case ARC_STATE_REQUEST_ARC_INITIATION:
                        LOGINFO("%s: Send ARC Initiation request \n",__FUNCTION__);
                        DisplaySettings::_instance->setUpHdmiCecSinkArcRouting(true);
                        break;

                    case ARC_STATE_REQUEST_ARC_TERMINATION:
                       LOGINFO("%s: Send ARC Termination request \n",__FUNCTION__);
                       DisplaySettings::_instance->setUpHdmiCecSinkArcRouting(false);
                       break;

                    case ARC_STATE_ARC_EXIT:
                       threadExit = true;
                       break;

                    default:
                        LOGINFO("%s: Default case - No ARC init/terminate request sent. Current arcState : %d \n",__FUNCTION__, arcState);
                        break;
                }

                std::unique_lock<std::mutex> lock(DisplaySettings::_instance->m_arcRoutingStateMutex);
		DisplaySettings::_instance->m_cecArcRoutingThreadRun = false;
	    }

	    LOGINFO("%s: ARC Routing Thread Stop\n",__FUNCTION__);
	}

        // Event management
        // 1.
        uint32_t DisplaySettings::subscribeForHdmiCecSinkEvent(const char* eventName)
        {
            uint32_t err = Core::ERROR_NONE;
            LOGINFO("Attempting to subscribe for event: %s\n", eventName);
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(SERVER_DETAILS)));
            if (nullptr == m_client) {
                m_client = make_shared<WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>>(_T(HDMICECSINK_CALLSIGN_VER), (_T(HDMICECSINK_CALLSIGN_VER)));
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
                } else if(strcmp(eventName, HDMICECSINK_ARC_TERMINATION_EVENT) == 0) {
                    err =m_client->Subscribe<JsonObject>(1000, eventName
                            , &DisplaySettings::onARCTerminationEventHandler, this);
                } else if(strcmp(eventName, HDMICECSINK_SHORT_AUDIO_DESCRIPTOR_EVENT) == 0) {
                    err =m_client->Subscribe<JsonObject>(1000, eventName
                            , &DisplaySettings::onShortAudioDescriptorEventHandler, this);
                } else if(strcmp(eventName, HDMICECSINK_SYSTEM_AUDIO_MODE_EVENT) == 0) {
                    err =m_client->Subscribe<JsonObject>(1000, eventName
                            , &DisplaySettings::onSystemAudioModeEventHandler, this);
                } else if(strcmp(eventName, HDMICECSINK_AUDIO_DEVICE_CONNECTED_STATUS_EVENT) == 0) {
                    err =m_client->Subscribe<JsonObject>(1000, eventName
                            , &DisplaySettings::onAudioDeviceConnectedStatusEventHandler, this);
                } else if(strcmp(eventName, HDMICECSINK_CEC_ENABLED_EVENT) == 0) {
                    err =m_client->Subscribe<JsonObject>(1000, eventName
                            , &DisplaySettings::onCecEnabledEventHandler, this);
                } else if(strcmp(eventName, HDMICECSINK_AUDIO_DEVICE_POWER_STATUS_EVENT) == 0) {
                    err =m_client->Subscribe<JsonObject>(1000, eventName
                            , &DisplaySettings::onAudioDevicePowerStatusEventHandler, this);
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

            if (m_currentArcRoutingState != ARC_STATE_ARC_INITIATED) {
                value = parameters["status"].String();

		if( !value.compare("success") ) {
                    {
                      std::lock_guard<std::mutex> lock(m_arcRoutingStateMutex);
                      m_currentArcRoutingState = ARC_STATE_ARC_INITIATED;
                    }

                    try
                    {
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI_ARC0");
                        JsonObject aPortConfig;
                        aPortConfig = getAudioOutputPortConfig();
			if(m_hdmiInAudioDeviceConnected ==  false) {
                            m_hdmiInAudioDeviceConnected = true;
			    connectedAudioPortUpdated(dsAUDIOPORT_TYPE_HDMI_ARC, true);
			}
			else {
                            LOGINFO("onARCInitiationEventHandler: not notifying the UI as m_hdmiInAudioDeviceConnected = true !!!\n");
                        }

                        if(aPortConfig["HDMI_ARC"].Boolean()) {
                            LOGINFO("onARCInitiationEventHandler: Enable ARC\n");
                            aPort.enableARC(dsAUDIOARCSUPPORT_ARC, true);
                            m_arcAudioEnabled = true;
                        }
                        else {
                           LOGINFO("onARCInitiationEventHandler: HDMI_ARC0 Port not enabled. Skip Audio Routing !!!\n");
                        }
                    }
                    catch (const device::Exception& err)
                    {
                        LOG_DEVICE_EXCEPTION1(string("HDMI_ARC0"));
                    }
		}
		else{
                    LOGERR("CEC ARC Initiaition Failed !!!");
                    {
                      std::lock_guard<std::mutex> lock(m_arcRoutingStateMutex);
                      m_currentArcRoutingState = ARC_STATE_ARC_TERMINATED;
                    }
		}
            } else {
                LOGINFO("%s: The ARC initiation already done", __FUNCTION__);
            }
        }

        // 3.
        void DisplaySettings::onARCTerminationEventHandler(const JsonObject& parameters) {
            string message;
	    string value;

            parameters.ToString(message);
            LOGINFO("[ARC Termination Event], %s : %s", __FUNCTION__, C_STR(message));

            if (parameters.HasLabel("status")) {
                value = parameters["status"].String();
                {
                    std::lock_guard<std::mutex> lock(m_arcRoutingStateMutex);
                    m_currentArcRoutingState = ARC_STATE_ARC_TERMINATED;
                }
                if(!value.compare("success")) {
		    try 
		    {
			if(m_hdmiInAudioDeviceConnected ==  true) {
                            m_hdmiInAudioDeviceConnected = false;
                            connectedAudioPortUpdated(dsAUDIOPORT_TYPE_HDMI_ARC, false);

                            device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI_ARC0");
                            LOGINFO("onARCTerminationEventHandler: Disable ARC\n");
                            aPort.enableARC(dsAUDIOARCSUPPORT_ARC, false);
                            m_arcAudioEnabled = false;
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
            } else {
                LOGERR("Field 'status' could not be found in the event's payload.");
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
                    try
                    {
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI_ARC0");
			LOGINFO("Total Short Audio Descriptors received from connected ARC device: %d\n",shortAudioDescriptorList.Length());
			if(shortAudioDescriptorList.Length() <= 0) {
			    LOGERR("Not setting SAD. No SAD returned by connected ARC device\n");
			    return;
			}

			std::vector<int> sad_list;
			for (int i=0; i<shortAudioDescriptorList.Length(); i++) {
                            LOGINFO("Short Audio Descriptor[%d]: %ld \n",i, shortAudioDescriptorList[i].Number());
                            sad_list.push_back(shortAudioDescriptorList[i].Number());
                        }

		        aPort.setSAD(sad_list);
                        if(aPort.getStereoAuto() == true) {
                            aPort.setStereoAuto(true,true);
                        }
                        else{
                            device::AudioStereoMode mode = device::AudioStereoMode::kStereo;  //default to stereo
                            mode = aPort.getStereoMode(); //get Last User set stereo mode and set
                            aPort.setStereoMode(mode.toString(), true);
                        }
                    }
                    catch (const device::Exception& err)
                    {
                        LOG_DEVICE_EXCEPTION1(string("HDMI_ARC0"));
                    }
            } else {
                LOGERR("Field 'ShortAudioDescriptor' could not be found in the event's payload.");
            }
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
//DD Do not update connection status as it necessarily doesn't mean ARC device connected
//                    m_hdmiInAudioDeviceConnected = true;
//                    connectedAudioPortUpdated(dsAUDIOPORT_TYPE_HDMI_ARC, true);
                    LOGINFO("%s :  audioMode ON !!!\n", __FUNCTION__);
                    try {
                        int types = dsAUDIOARCSUPPORT_NONE;
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI_ARC0");
                        aPort.getSupportedARCTypes(&types);
                        if((types & dsAUDIOARCSUPPORT_eARC) && (m_hdmiInAudioDeviceConnected == false)) {
                            m_hdmiInAudioDeviceConnected = true;
                            LOGINFO("%s: eARC device sent system audio mode ON: Notify UI !!! \n");
                            connectedAudioPortUpdated(dsAUDIOPORT_TYPE_HDMI_ARC, true);
                        }
			else if(types & dsAUDIOARCSUPPORT_ARC) {
                            std::lock_guard<std::mutex> lock(m_arcRoutingStateMutex);
                            if((m_currentArcRoutingState == ARC_STATE_ARC_TERMINATED) && (m_hdmiInAudioDeviceConnected == false) && (isCecEnabled == true)) {
                                LOGINFO("%s :  m_hdmiInAudioDeviceConnected = false. ARC state is terminated.  Trigger ARC Initiation request !!!\n", __FUNCTION__); 
                                m_currentArcRoutingState = ARC_STATE_REQUEST_ARC_INITIATION;
			        m_cecArcRoutingThreadRun = true;
		                arcRoutingCV.notify_one();
		            }
                        }
                    }
                    catch(const device::Exception& err)
                    {
                        LOG_DEVICE_EXCEPTION1(string("HDMI_ARC0"));
                    }
                }
		else if(!value.compare("Off")) {
                    LOGINFO("%s :  audioMode OFF !!!\n", __FUNCTION__);
		    try {
   		        if(m_hdmiInAudioDeviceConnected == true) {
		            m_hdmiInAudioDeviceConnected = false;
		            connectedAudioPortUpdated(dsAUDIOPORT_TYPE_HDMI_ARC, false);
                            device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI_ARC0");
                            LOGINFO("onSystemAudioModeEventHandler: Disable ARC\n");
                            aPort.enableARC(dsAUDIOARCSUPPORT_ARC, false);
                            m_arcAudioEnabled = false;
                            {
                              std::lock_guard<std::mutex> lock(m_arcRoutingStateMutex);
                              m_currentArcRoutingState = ARC_STATE_ARC_TERMINATED;
                            }
		        }
                        else {
                            LOGINFO("onSystemAudioModeEventHandler: Skip Disable ARC and not notifying the UI as  m_hdmiInAudioDeviceConnected = false\n");
                        }
		    }
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

	/* Event handler when Audio Device is Added/Removed     */
	void DisplaySettings::onAudioDeviceConnectedStatusEventHandler(const JsonObject& parameters)
	{
            int types = dsAUDIOARCSUPPORT_NONE;
	    string value;

	    if (parameters.HasLabel("audioDeviceConnected"))
		value = parameters["audioDeviceConnected"].String();
	    
	    if(!value.compare("true")) {
	        m_hdmiCecAudioDeviceDetected = true;
        } else{
	        m_hdmiCecAudioDeviceDetected = false;
        }
	    LOGINFO("updated m_hdmiCecAudioDeviceDetected status [%d] ... \n", m_hdmiCecAudioDeviceDetected);

		if (m_hdmiCecAudioDeviceDetected)
		{
                    LOGINFO("Trigger Audio Device Power State Request status ... \n");
                    {
                       std::lock_guard<std::mutex> lock(m_arcRoutingStateMutex);
                       m_hdmiInAudioDevicePowerState = AUDIO_DEVICE_POWER_STATE_REQUEST;
                       m_cecArcRoutingThreadRun = true;
                       arcRoutingCV.notify_one();
                    }

		} else {
                    LOGINFO("Audio Device is removed \n");
		}
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

             if(pState == 0) {//ON
                m_hdmiInAudioDevicePowerState = AUDIO_DEVICE_POWER_STATE_ON;
                try {
                    int types = dsAUDIOARCSUPPORT_NONE;
                    device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI_ARC0");
                    aPort.getSupportedARCTypes(&types);
                    if((types & dsAUDIOARCSUPPORT_eARC) && (m_hdmiInAudioDeviceConnected == false)) {
                        m_hdmiInAudioDeviceConnected = true;
                        LOGINFO("%s: Triggered from HPD: eARC audio device power on: Notify UI !!! \n");
                        connectedAudioPortUpdated(dsAUDIOPORT_TYPE_HDMI_ARC, true);
                    }
                    else if(types & dsAUDIOARCSUPPORT_ARC) {
                        LOGINFO("ARC_mode: settings... \n");

                        std::lock_guard<std::mutex> lock(m_arcRoutingStateMutex);

                        if((m_currentArcRoutingState == ARC_STATE_ARC_TERMINATED) && (isCecEnabled == true)) {
                           LOGINFO("ARC_mode: Send dummy ARC initiation request... \n");
                           m_currentArcRoutingState = ARC_STATE_REQUEST_ARC_INITIATION;
                           m_cecArcRoutingThreadRun = true;
                           LOGINFO("ARC_mode: Notify Arc routing with m_currentArcRoutingStat [%d] \n", DisplaySettings::_instance->m_currentArcRoutingState );
                           arcRoutingCV.notify_one();
                        }
                    }
                }
                catch(const device::Exception& err)
                {
                    LOG_DEVICE_EXCEPTION1(string("HDMI_ARC0"));
                }
             }
             else if(pState == 1) {
                 m_hdmiInAudioDevicePowerState = AUDIO_DEVICE_POWER_STATE_STANDBY;
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
	      }

              LOGINFO("updated isCecEnabled [%d] ... \n", isCecEnabled);
	}

        // 6.
        void DisplaySettings::onTimer()
        {
            // lock to prevent: parallel onTimer runs, destruction during onTimer
            lock_guard<mutex> lck(m_callMutex);

            bool isPluginActivated = Utils::isPluginActivated(HDMICECSINK_CALLSIGN);

            if (!isPluginActivated) {
                /*HDMICECSINK_CALLSIGN plugin activation moved to onTimer.
                 *To decouple from displyasettings init. Since its time taking*/
                Utils::activatePlugin(HDMICECSINK_CALLSIGN);
                LOGWARN ("DisplaySettings::onTimer after activatePlugin HDMICECSINK_CALLSIGN line:%d", __LINE__);
                sleep(HDMICECSINK_PLUGIN_ACTIVATION_TIME);
            }

            static bool isInitDone = false;
            bool pluginActivated = Utils::isPluginActivated(HDMICECSINK_CALLSIGN);
            LOGWARN ("DisplaySettings::onTimer pluginActivated:%d line:%d", pluginActivated, __LINE__);
            if(!m_subscribed) {
                if (pluginActivated && (subscribeForHdmiCecSinkEvent(HDMICECSINK_ARC_INITIATION_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_ARC_TERMINATION_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_SHORT_AUDIO_DESCRIPTOR_EVENT)== Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_SYSTEM_AUDIO_MODE_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_AUDIO_DEVICE_CONNECTED_STATUS_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_CEC_ENABLED_EVENT) == Core::ERROR_NONE) && (subscribeForHdmiCecSinkEvent(HDMICECSINK_AUDIO_DEVICE_POWER_STATUS_EVENT) == Core::ERROR_NONE))
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
                    if (!pluginActivated)
                    {
                        Utils::activatePlugin(HDMICECSINK_CALLSIGN);
                    }
                }
            } else {
                //Standby ON transitions case
                LOGINFO("Already subscribed. Stopping the timer.");
                if (m_timer.isActive()) {
                    m_timer.stop();
                }
            }

            if(m_subscribed) {
                JsonObject aPortArcEnableResult;
                JsonObject aPortArcEnableParam;
                JsonObject aPortConfig;

                aPortArcEnableParam.Set(_T("audioPort"),"HDMI_ARC0");
                aPortConfig = getAudioOutputPortConfig();
                bool arcEnable = false;
                uint32_t ret = Core::ERROR_NONE;

                if (aPortConfig.HasLabel("HDMI_ARC")) {
                    try {
                            arcEnable = aPortConfig["HDMI_ARC"].Boolean();
                    }catch (...) {
                            LOGERR("HDMI_ARC status read error");
                    }
                }

                aPortArcEnableParam.Set(_T("enable"),arcEnable);
                ret = setEnableAudioPort (aPortArcEnableParam, aPortArcEnableResult);
                if(ret != Core::ERROR_NONE) {
                    LOGWARN("Audio Port : [HDMI_ARC0] enable: %d failed ! error code%d\n", arcEnable, ret);
                }
                else {
                    LOGINFO("Audio Port : [HDMI_ARC0] initialized successfully, enable: %d\n", arcEnable);
                }

		//Connected Audio Ports status update is necessary on bootup / power state transitions
                try {
                    int types = dsAUDIOARCSUPPORT_NONE;
                    device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI_ARC0");
                    aPort.getSupportedARCTypes(&types);
                    if(types & dsAUDIOARCSUPPORT_eARC) {
                        m_hdmiInAudioDeviceConnected = true;
                        connectedAudioPortUpdated(dsAUDIOPORT_TYPE_HDMI_ARC, true);
                    }
                    else if (types & dsAUDIOARCSUPPORT_ARC) {
                        //Dummy ARC intiation request
                      {
                        std::lock_guard<std::mutex> lock(m_arcRoutingStateMutex);
                        if((m_currentArcRoutingState == ARC_STATE_ARC_TERMINATED) && (isCecEnabled == true)) {
                            LOGINFO("%s: Send dummy ARC initiation request... \n", __FUNCTION__);
                            m_currentArcRoutingState = ARC_STATE_REQUEST_ARC_INITIATION;
                            m_cecArcRoutingThreadRun = true;
                            arcRoutingCV.notify_one();
                        }
                      }
                    }
                    else {
                        LOGINFO("%s: Connected Device doesn't have ARC/eARC capability... \n", __FUNCTION__);
                    }
               }
               catch (const device::Exception& err){
                    LOG_DEVICE_EXCEPTION1(string("HDMI_ARC0"));
               }
            }
        }

        void DisplaySettings::checkAudioDeviceDetectionTimer()
        {
            // lock to prevent: parallel onTimer runs, destruction during onTimer
            lock_guard<mutex> lck(m_callMutex);
            if (m_subscribed && m_hdmiCecAudioDeviceDetected)
            {
               //Connected Audio Ports status update is necessary on bootup / power state transitions
               sendHdmiCecSinkAudioDevicePowerOn();
               LOGINFO("%s: Audio Port : [HDMI_ARC0] sendHdmiCecSinkAudioDevicePowerOn !!! \n", __FUNCTION__);
               try {
                   int types = dsAUDIOARCSUPPORT_NONE;
                   device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI_ARC0");
                   aPort.getSupportedARCTypes(&types);
                   if(types & dsAUDIOARCSUPPORT_eARC) {
                       m_hdmiInAudioDeviceConnected = true;
                       connectedAudioPortUpdated(dsAUDIOPORT_TYPE_HDMI_ARC, true);
                   }
                   else if (types & dsAUDIOARCSUPPORT_ARC) {
                       //Dummy ARC intiation request
                      {
                       std::lock_guard<std::mutex> lock(m_arcRoutingStateMutex);
                       if((m_currentArcRoutingState == ARC_STATE_ARC_TERMINATED) && (isCecEnabled == true)) {
                           LOGINFO("%s: Send dummy ARC initiation request... \n", __FUNCTION__);
                           m_currentArcRoutingState = ARC_STATE_REQUEST_ARC_INITIATION;
                           m_cecArcRoutingThreadRun = true;
                           arcRoutingCV.notify_one();
                       }
                      }
                   }
                   else {
                       LOGINFO("%s: Connected Device doesn't have ARC/eARC capability... \n", __FUNCTION__);
                   }
               }
               catch (const device::Exception& err){
                   LOG_DEVICE_EXCEPTION1(string("HDMI_ARC0"));
               }
            }
            if (m_AudioDeviceDetectTimer.isActive()) {
               m_AudioDeviceDetectTimer.stop();
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

            videoFormats.Add("SDR");
            if(capabilities & dsHDRSTANDARD_HDR10)videoFormats.Add("HDR10");
            if(capabilities & dsHDRSTANDARD_HLG)videoFormats.Add("HLG");
            if(capabilities & dsHDRSTANDARD_DolbyVision)videoFormats.Add("DV");
            if(capabilities & dsHDRSTANDARD_TechnicolorPrime)videoFormats.Add("Technicolor Prime");
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
                case dsHDRSTANDARD_NONE:
                    LOGINFO("Video Format: SDR\n");
                    strValue = "SDR";
                    break;
                case dsHDRSTANDARD_HDR10:
                    LOGINFO("Video Format: HDR10\n");
                    strValue = "HDR10";
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
            if(strcmp(strFormat,"SDR")== 0 || strcmp(strFormat,"NONE")== 0 )
                    mode = dsHDRSTANDARD_NONE;
            else if(strcmp(strFormat,"HDR10")== 0)
                    mode = dsHDRSTANDARD_HDR10;
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
