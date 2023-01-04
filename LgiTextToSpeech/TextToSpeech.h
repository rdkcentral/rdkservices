/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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
 */

/**
 * @file TextToSpeech.h
 * @brief Thunder Plugin based Implementation for TTS service API's (RDK-27957).
 */

/**
  @mainpage Text To Speech (TTS)

  <b>TextToSpeech</b> TTS Thunder Service provides APIs for the arbitrators
  * (ex: Native application such as Cobalt) to use TTS resource.
  */

#pragma once

#include "Module.h"
#include "tracing/Logging.h"

#include "TextToSpeechImplementation.h"

namespace WPEFramework {
namespace Plugin {

    class TextToSpeech: public PluginHost::IPlugin, public PluginHost::JSONRPC {
    public:
        class Notification : public RPC::IRemoteConnection::INotification,
                             public Exchange::ITextToSpeech::INotification {
            private:
                Notification() = delete;
                Notification(const Notification&) = delete;
                Notification& operator=(const Notification&) = delete;

            public:
                explicit Notification(TextToSpeech* parent)
                    : _parent(*parent) {
                    ASSERT(parent != nullptr);
                }

                virtual ~Notification() {
                }

            public:
                virtual void StateChanged(const string &data) {
                    _parent.dispatchJsonEvent("onttsstatechanged", data);
                }

                virtual void VoiceChanged(const string &data) {
                    _parent.dispatchJsonEvent("onvoicechanged", data);
                }

                virtual void WillSpeak(const string &data) {
                    _parent.dispatchJsonEvent("onwillspeak", data);
                }

                virtual void SpeechStart(const string &data) {
                    _parent.dispatchJsonEvent("onspeechstart", data);
                }

                virtual void SpeechPause(const string &data) {
                    _parent.dispatchJsonEvent("onspeechpause", data);
                }

                virtual void SpeechResume(const string &data) {
                    _parent.dispatchJsonEvent("onspeechresume", data);
                }

                virtual void SpeechCancelled(const string &data) {
                    _parent.dispatchJsonEvent("onspeechcancelled", data);
                }

                virtual void SpeechInterrupted(const string &data) {
                    _parent.dispatchJsonEvent("onspeechinterrupted", data);
                }

                virtual void NetworkError(const string &data) {
                    _parent.dispatchJsonEvent("onnetworkerror", data);
                }

                virtual void PlaybackError(const string &data) {
                    _parent.dispatchJsonEvent("onplaybackerror", data);
                }

                virtual void SpeechComplete(const string &data) {
                    _parent.dispatchJsonEvent("onspeechcomplete", data);
                }

                virtual void Activated(RPC::IRemoteConnection* /* connection */) final
                {
                    TTSLOG_WARNING("TextToSpeech::Notification::Activated - %p", this);
                }

                virtual void Deactivated(RPC::IRemoteConnection* connection) final
                {
                    TTSLOG_WARNING("TextToSpeech::Notification::Deactivated - %p", this);
                    _parent.Deactivated(connection);
                }

                BEGIN_INTERFACE_MAP(Notification)
                INTERFACE_ENTRY(Exchange::ITextToSpeech::INotification)
                INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
                END_INTERFACE_MAP

            private:
                    TextToSpeech& _parent;
        };

        BEGIN_INTERFACE_MAP(TextToSpeech)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        INTERFACE_AGGREGATE(Exchange::ITextToSpeech, _tts)
        END_INTERFACE_MAP

    public:
        TextToSpeech();
        virtual ~TextToSpeech();
        virtual const string Initialize(PluginHost::IShell* service) override;
        virtual void Deinitialize(PluginHost::IShell* service) override;
        virtual string Information() const override { return {}; }

    private:
        // We do not allow this plugin to be copied !!
        TextToSpeech(const TextToSpeech&) = delete;
        TextToSpeech& operator=(const TextToSpeech&) = delete;

        void RegisterAll();
        bool AddToAccessList(const string &key,const string &value);
        bool HasAccess(const string &method,string &app);

        //TTS Global APIS for Resident application
        uint32_t Enable(const JsonObject& parameters, JsonObject& response);
        uint32_t ListVoices(const JsonObject& parameters, JsonObject& response);
        uint32_t SetConfiguration(const JsonObject& parameters, JsonObject& response);
        uint32_t GetConfiguration(const JsonObject& parameters, JsonObject& response);

        // Mandotory TTS APIs for client application
        uint32_t IsEnabled(const JsonObject& parameters, JsonObject& response);
        uint32_t Speak(const JsonObject& parameters, JsonObject& response);
        uint32_t Cancel(const JsonObject& parameters, JsonObject& response);

        // These extended APIS can be used by Client application if needed
        uint32_t Pause(const JsonObject& parameters, JsonObject& response);
        uint32_t Resume(const JsonObject& parameters, JsonObject& response);
        uint32_t IsSpeaking(const JsonObject& parameters, JsonObject& response);
        uint32_t GetSpeechState(const JsonObject& parameters, JsonObject& response);
        uint32_t SetACL(const JsonObject& parameters, JsonObject& response);

        //version number API's
        uint32_t getapiversion(const JsonObject& parameters, JsonObject& response);

        void dispatchJsonEvent(const char *event, const string &data);
        void Deactivated(RPC::IRemoteConnection* connection);

    private:
        uint8_t _skipURL{};
        uint32_t _connectionId{};
        PluginHost::IShell* _service{};
        Exchange::ITextToSpeech* _tts{};
        Core::Sink<Notification> _notification;
        uint32_t _apiVersionNumber;
        bool m_AclCalled;
        std::map<std::string,std::string> m_AccessList;
        std::mutex m_AccessMutex;

        friend class Notification;
    };

} // namespace Plugin
} // namespace WPEFramework
