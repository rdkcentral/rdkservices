#pragma once

#include "Module.h"
#include "libIBus.h"

#include "ctrlm_ipc.h"
#include "ctrlm_ipc_voice.h"

#define IARM_VOICECONTROL_PLUGIN_NAME       "Voice_Control"

namespace WPEFramework {

    namespace Plugin {

        class VoiceControl : public PluginHost::IPlugin, public PluginHost::JSONRPC {
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
            virtual string Information() const override { return {}; }

            BEGIN_INTERFACE_MAP(VoiceControl)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

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
