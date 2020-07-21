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

#include <stdint.h>
#include <syscall.h>
#include "Module.h"
#include "impl/TTSManager.h"
#include "tracing/Logging.h"
#include "utils.h"
#include "AbstractPlugin.h"
#include "TTSServicesCommon.h"
#include <vector>
#include <mutex>

namespace WPEFramework {
    namespace Plugin {

        class TTSEventCallback;
        /**
        * @brief WPEFramework class declaration for TextToSpeech
        **/
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
        class TextToSpeech: public AbstractPlugin {
        private:
            typedef Core::JSON::String JString;
            typedef Core::JSON::ArrayType<JString> JStringArray;
            typedef Core::JSON::Boolean JBool;

            // We do not allow this plugin to be copied !!
            TextToSpeech(const TextToSpeech&) = delete;
            TextToSpeech& operator=(const TextToSpeech&) = delete;

            //TTS Global APIS for Resident application
            uint32_t enabletts(const JsonObject& parameters, JsonObject& response);
            uint32_t listvoices(const JsonObject& parameters, JsonObject& response);
            uint32_t setttsconfiguration(const JsonObject& parameters, JsonObject& response);
            uint32_t getttsconfiguration(const JsonObject& parameters, JsonObject& response);

            // Mandotory TTS APIs for client application
            uint32_t isttsenabled(const JsonObject& parameters, JsonObject& response);
            uint32_t speak(const JsonObject& parameters, JsonObject& response);
            uint32_t cancel(const JsonObject& parameters, JsonObject& response);

            // These extended APIS can be used by Client application if needed
            uint32_t pause(const JsonObject& parameters, JsonObject& response);
            uint32_t resume(const JsonObject& parameters, JsonObject& response);
            uint32_t isspeaking(const JsonObject& parameters, JsonObject& response);
            uint32_t getspeechstate(const JsonObject& parameters, JsonObject& response);

            //version number API's
            uint32_t getapiversion(const JsonObject& parameters, JsonObject& response);

        private:
            static TTS::TTSManager* m_ttsManager;
            static TTSEventCallback* m_eventCallback;
            TTS::Configuration m_config;
            uint32_t m_apiVersionNumber;
            std::mutex  m_mutex;

        public:
            TextToSpeech();
            virtual ~TextToSpeech();
            void notifyClient(std::string eventname, JsonObject& param);
            void setResponseArray(JsonObject& response, const char* key, const std::vector<std::string>& items);
        };

        /**
        *  TTSEventCallback for TTS events
        **/
        class TTSEventCallback : public TTS::TTSEventCallback
        {
            public:
                TTSEventCallback(TextToSpeech* callback)
                {
                    m_eventHandler = callback;
                }

                void onTTSStateChanged(bool state)
                {
                    JsonObject params;
                    params["state"] = JsonValue((bool)state);
                    m_eventHandler->notifyClient("onttsstatechanged", params);
                }

                void onVoiceChanged(std::string voice)
                {
                    JsonObject params;
                    params["voice"] = voice;
                    m_eventHandler->notifyClient("onvoicechanged", params);
                }

                void onWillSpeak(TTS::SpeechData &data)
                {
                    JsonObject params;
                    params["speechid"]  = JsonValue((int)data.id);
                    params["text"]      = data.text;
                    m_eventHandler->notifyClient("onwillspeak", params);
                }

                void onSpeechStart(TTS::SpeechData &data)
                {
                    JsonObject params;
                    params["speechid"]  = JsonValue((int)data.id);
                    params["text"]      = data.text;
                    m_eventHandler->notifyClient("onspeechstart", params);
                }

                void onSpeechPause(uint32_t speechId)
                {
                    JsonObject params;
                    params["speechid"]  = JsonValue((int)speechId);
                    m_eventHandler->notifyClient("onspeechpause", params);
                }

                void onSpeechResume(uint32_t speechId)
                {
                    JsonObject params;
                    params["speechid"]  = JsonValue((int)speechId);
                    m_eventHandler->notifyClient("onspeechresume", params);
                }

                void onSpeechCancelled(std::vector<uint32_t> speechIds)
                {
                    std::stringstream ss;
                    for(auto it = speechIds.begin(); it != speechIds.end(); ++it)
                    {
                        if(it != speechIds.begin())
                            ss << ",";
                        ss << *it;
                    }
                    JsonObject params;
                    params["speechid"]  = ss.str().c_str();
                    m_eventHandler->notifyClient("onspeechcancelled", params);
                }

                void onSpeechInterrupted(uint32_t speechId)
                {
                    JsonObject params;
                    params["speechid"]  = JsonValue((int)speechId);
                    m_eventHandler->notifyClient("onspeechinterrupted", params);
                }

                void onNetworkError(uint32_t speechId)
                {
                    JsonObject params;
                    params["speechid"]  = JsonValue((int)speechId);
                    m_eventHandler->notifyClient("onnetworkerror", params);
                }

                void onPlaybackError(uint32_t speechId)
                {
                    JsonObject params;
                    params["speechid"]  = JsonValue((int)speechId);
                    m_eventHandler->notifyClient("onplaybackerror", params);
                }

                void onSpeechComplete(TTS::SpeechData &data)
                {
                    JsonObject params;
                    params["speechid"]  = JsonValue((int)data.id);
                    params["text"]      = data.text;
                    m_eventHandler->notifyClient("onspeechcomplete", params);
                }

            private:
                TextToSpeech *m_eventHandler;
        };

 } // namespace Plugin
} // namespace WPEFramework
