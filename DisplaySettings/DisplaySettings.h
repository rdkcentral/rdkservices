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

#pragma once

#include <mutex>
#include "Module.h"
#include "utils.h"
#include "dsTypes.h"
#include "tptimer.h"
#include "AbstractPlugin.h"
#include "libIBus.h"
#include "libIBusDaemon.h"
#include "irMgr.h"
#include "pwrMgr.h"

namespace WPEFramework {

    namespace Plugin {

		// This is a server for a JSONRPC communication channel.
		// For a plugin to be capable to handle JSONRPC, inherit from PluginHost::JSONRPC.
		// By inheriting from this class, the plugin realizes the interface PluginHost::IDispatcher.
		// This realization of this interface implements, by default, the following methods on this plugin
		// - exists
		// - register
		// - unregister
		// Any other methood to be handled by this plugin  can be added can be added by using the
		// templated methods Register on the PluginHost::JSONRPC class.
		// As the registration/unregistration of notifications is realized by the class PluginHost::JSONRPC,
		// this class exposes a public method called, Notify(), using this methods, all subscribed clients
		// will receive a JSONRPC message as a notification, in case this method is called.
        class DisplaySettings : public AbstractPlugin {
        private:
            typedef Core::JSON::String JString;
            typedef Core::JSON::ArrayType<JString> JStringArray;
            typedef Core::JSON::Boolean JBool;

            // We do not allow this plugin to be copied !!
            DisplaySettings(const DisplaySettings&) = delete;
            DisplaySettings& operator=(const DisplaySettings&) = delete;

            //Begin methods
            uint32_t getConnectedVideoDisplays(const JsonObject& parameters, JsonObject& response);
            uint32_t getConnectedAudioPorts(const JsonObject& parameters, JsonObject& response);
	    uint32_t setEnableAudioPort (const JsonObject& parameters, JsonObject& response);
            uint32_t getSupportedResolutions(const JsonObject& parameters, JsonObject& response);
            uint32_t getSupportedVideoDisplays(const JsonObject& parameters, JsonObject& response);
            uint32_t getSupportedTvResolutions(const JsonObject& parameters, JsonObject& response);
            uint32_t getSupportedSettopResolutions(const JsonObject& parameters, JsonObject& response);
            uint32_t getSupportedAudioPorts(const JsonObject& parameters, JsonObject& response);
            uint32_t getSupportedAudioModes(const JsonObject& parameters, JsonObject& response);
            uint32_t getZoomSetting(const JsonObject& parameters, JsonObject& response);
            uint32_t setZoomSetting(const JsonObject& parameters, JsonObject& response);
            uint32_t getCurrentResolution(const JsonObject& parameters, JsonObject& response);
            uint32_t setCurrentResolution(const JsonObject& parameters, JsonObject& response);
            uint32_t getSoundMode(const JsonObject& parameters, JsonObject& response);
            uint32_t setSoundMode(const JsonObject& parameters, JsonObject& response);
            uint32_t readEDID(const JsonObject& parameters, JsonObject& response);
            uint32_t readHostEDID(const JsonObject& parameters, JsonObject& response);
            uint32_t getActiveInput(const JsonObject& parameters, JsonObject& response);
            uint32_t getTvHDRSupport(const JsonObject& parameters, JsonObject& response);
            uint32_t getSettopHDRSupport(const JsonObject& parameters, JsonObject& response);
            uint32_t setVideoPortStatusInStandby(const JsonObject& parameters, JsonObject& response);
            uint32_t getVideoPortStatusInStandby(const JsonObject& parameters, JsonObject& response);
            uint32_t getCurrentOutputSettings(const JsonObject& parameters, JsonObject& response);
            //End methods
            uint32_t setMS12AudioCompression(const JsonObject& parameters, JsonObject& response);
            uint32_t getMS12AudioCompression(const JsonObject& parameters, JsonObject& response);
            uint32_t setDolbyVolumeMode(const JsonObject& parameters, JsonObject& response);
            uint32_t getDolbyVolumeMode(const JsonObject& parameters, JsonObject& response);
            uint32_t setDialogEnhancement(const JsonObject& parameters, JsonObject& response);
            uint32_t getDialogEnhancement(const JsonObject& parameters, JsonObject& response);
            uint32_t setIntelligentEqualizerMode(const JsonObject& parameters, JsonObject& response);
            uint32_t getIntelligentEqualizerMode(const JsonObject& parameters, JsonObject& response);
            uint32_t setGraphicEqualizerMode(const JsonObject& parameters, JsonObject& response);
            uint32_t getGraphicEqualizerMode(const JsonObject& parameters, JsonObject& response);
	    uint32_t setMS12AudioProfile(const JsonObject& parameters, JsonObject& response);
	    uint32_t getMS12AudioProfile(const JsonObject& parameters, JsonObject& response);
	    uint32_t getSupportedMS12AudioProfiles(const JsonObject& parameters, JsonObject& response);
            uint32_t getAudioDelay(const JsonObject& parameters, JsonObject& response);
            uint32_t setAudioDelay(const JsonObject& parameters, JsonObject& response);
            uint32_t getAudioDelayOffset(const JsonObject& parameters, JsonObject& response);
            uint32_t setAudioDelayOffset(const JsonObject& parameters, JsonObject& response);
            uint32_t getSinkAtmosCapability(const JsonObject& parameters, JsonObject& response);
            uint32_t setAudioAtmosOutputMode(const JsonObject& parameters, JsonObject& response);
            uint32_t getTVHDRCapabilities(const JsonObject& parameters, JsonObject& response);
            uint32_t isConnectedDeviceRepeater(const JsonObject& parameters, JsonObject& response);
            uint32_t getDefaultResolution(const JsonObject& parameters, JsonObject& response);
            uint32_t setScartParameter(const JsonObject& parameters, JsonObject& response);
            uint32_t getVolumeLeveller(const JsonObject& parameters, JsonObject& response);
            uint32_t getBassEnhancer(const JsonObject& parameters, JsonObject& response);
            uint32_t isSurroundDecoderEnabled(const JsonObject& parameters, JsonObject& response);
            uint32_t getDRCMode(const JsonObject& parameters, JsonObject& response);
            uint32_t getSurroundVirtualizer(const JsonObject& parameters, JsonObject& response);
            uint32_t getMISteering(const JsonObject& parameters, JsonObject& response);
            uint32_t setVolumeLeveller(const JsonObject& parameters, JsonObject& response);
            uint32_t setBassEnhancer(const JsonObject& parameters, JsonObject& response);
            uint32_t enableSurroundDecoder(const JsonObject& parameters, JsonObject& response);
            uint32_t setSurroundVirtualizer(const JsonObject& parameters, JsonObject& response);
            uint32_t setMISteering(const JsonObject& parameters, JsonObject& response);
            uint32_t setGain(const JsonObject& parameters, JsonObject& response);
            uint32_t getGain(const JsonObject& parameters, JsonObject& response);
            uint32_t setMuted(const JsonObject& parameters, JsonObject& response);
            uint32_t getMuted(const JsonObject& parameters, JsonObject& response);
            uint32_t setVolumeLevel(const JsonObject& parameters, JsonObject& response);
            uint32_t getVolumeLevel(const JsonObject& parameters, JsonObject& response);
            uint32_t setDRCMode(const JsonObject& parameters, JsonObject& response);
            uint32_t getSettopMS12Capabilities(const JsonObject& parameters, JsonObject& response);
            uint32_t getSettopAudioCapabilities(const JsonObject& parameters, JsonObject& response);
            uint32_t getEnableAudioPort(const JsonObject& parameters, JsonObject& response);

	    uint32_t getAudioFormat(const JsonObject& parameters, JsonObject& response);
	    uint32_t getVolumeLeveller2(const JsonObject& parameters, JsonObject& response);
	    uint32_t setVolumeLeveller2(const JsonObject& parameters, JsonObject& response);
	    uint32_t getSurroundVirtualizer2(const JsonObject& parameters, JsonObject& response);
	    uint32_t setSurroundVirtualizer2(const JsonObject& parameters, JsonObject& response);

            void InitAudioPorts();
            //End methods

            //Begin events
            void resolutionPreChange();
            void resolutionChanged(int width, int height);
            void zoomSettingUpdated(const string& zoomSetting);
            void activeInputChanged(bool activeInput);
            void connectedVideoDisplaysUpdated(int hdmiHotPlugEvent);
            void connectedAudioPortUpdated (int iAudioPortType, bool isPortConnected);
	    void notifyAudioFormatChange(dsAudioFormat_t audioFormat);
	    void onARCInitiationEventHandler(const JsonObject& parameters);
            void onARCTerminationEventHandler(const JsonObject& parameters);
	    void onShortAudioDescriptorEventHandler(const JsonObject& parameters);
	    void onSystemAudioModeEventHandler(const JsonObject& parameters);
            //End events
        public:
            DisplaySettings();
            virtual ~DisplaySettings();
            //IPlugin methods
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
        private:
            void InitializeIARM();
            void DeinitializeIARM();
            static void ResolutionPreChange(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            static void ResolutionPostChange(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            static void DisplResolutionHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            static void dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
	    static void audioFormatUpdateEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            static void powerEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            void getConnectedVideoDisplaysHelper(std::vector<string>& connectedDisplays);
	    void audioFormatToString(dsAudioFormat_t audioFormat, JsonObject &response);
            bool checkPortName(std::string& name) const;
            IARM_Bus_PWRMgr_PowerState_t getSystemPowerState();

	    std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> getHdmiCecSinkPlugin();
	    std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> > m_client;
	    std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> getSystemPlugin();
	    uint32_t subscribeForHdmiCecSinkEvent(const char* eventName);
	    bool setUpHdmiCecSinkArcRouting (bool arcEnable);
	    bool requestShortAudioDescriptor();
	    void onTimer();

	    TpTimer m_timer;
            bool m_subscribed;
            std::mutex m_callMutex;
	    bool m_hdmiInAudioDeviceConnected;
	    JsonObject m_audioOutputPortConfig;
            JsonObject getAudioOutputPortConfig() { return m_audioOutputPortConfig; }
            static IARM_Bus_PWRMgr_PowerState_t m_powerState;

        public:
            static DisplaySettings* _instance;

        };
	} // namespace Plugin
} // namespace WPEFramework
