#pragma once

#include "Module.h"
#include "utils.h"
#include "AbstractPlugin.h"
#include "libIBus.h"

#include "ctrlm_ipc.h"
#include "ctrlm_ipc_voice.h"

#define IARM_VOICECONTROL_PLUGIN_NAME       "Voice_Control"

namespace WPEFramework {

    namespace Plugin {

		// This is a server for a JSONRPC communication channel.
		// For a plugin to be capable to handle JSONRPC, inherit from PluginHost::JSONRPC.
		// By inheriting from this class, the plugin realizes the interface PluginHost::IDispatcher.
		// This realization of this interface implements, by default, the following methods on this plugin
		// - exists
		// - register
		// - unregister
		// Any other method to be handled by this plugin  can be added can be added by using the
		// templated methods Register on the PluginHost::JSONRPC class.
		// As the registration/unregistration of notifications is realized by the class PluginHost::JSONRPC,
		// this class exposes a public method called, Notify(), using this methods, all subscribed clients
		// will receive a JSONRPC message as a notification, in case this method is called.
        // Note that most of the above is now inherited from the AbstractPlugin class.
        class VoiceControl : public AbstractPlugin {
        private:
            // We do not allow this plugin to be copied !!
            VoiceControl(const VoiceControl&) = delete;
            VoiceControl& operator=(const VoiceControl&) = delete;

            //Begin methods
            uint32_t getApiVersionNumber(const JsonObject& parameters, JsonObject& response);

            uint32_t voiceStatus(const JsonObject& parameters, JsonObject& response);
            uint32_t configureVoice(const JsonObject& parameters, JsonObject& response);
            uint32_t setVoiceInit(const JsonObject& parameters, JsonObject& response);
            uint32_t sendVoiceMessage(const JsonObject& parameters, JsonObject& response);
            uint32_t voiceSessionByText(const JsonObject& parameters, JsonObject& response);
            //End methods

            //Begin events
            void onSessionBegin(ctrlm_voice_iarm_event_json_t* eventData);
            void onStreamBegin(ctrlm_voice_iarm_event_json_t* eventData);
            void onKeywordVerification(ctrlm_voice_iarm_event_json_t* eventData);
            void onServerMessage(ctrlm_voice_iarm_event_json_t* eventData);
            void onStreamEnd(ctrlm_voice_iarm_event_json_t* eventData);
            void onSessionEnd(ctrlm_voice_iarm_event_json_t* eventData);
            //End events

        public:
            VoiceControl();
            virtual ~VoiceControl();
            //IPlugin methods
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
        private:
            void InitializeIARM();
            void DeinitializeIARM();
            // Handlers for ControlMgr BT Remote events
            static void voiceEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            void iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

            // Local utility methods
            void setApiVersionNumber(uint32_t apiVersionNumber);
        public:
            static VoiceControl* _instance;
        private:
            uint32_t m_apiVersionNumber;
            bool m_hasOwnProcess;
        };
	} // namespace Plugin
} // namespace WPEFramework
