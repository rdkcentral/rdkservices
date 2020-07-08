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

/*
 * If not stated otherwise in this file or this component's license file the
 * following copyright and licenses apply:
 *
 * Copyright 2018 RDK Management
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

namespace WPEFramework {
    namespace Plugin {

        class TTSConnectionCallback;
        /**
        * @brief WPEFramework class declarioin for TextToSpeech 
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

            // TTS Interfae APIs for client application
            uint32_t isTTSEnabled(const JsonObject& parameters ,JsonObject& response);
            uint32_t speak(const JsonObject& parameters, JsonObject& response);
            uint32_t cancel(const JsonObject& parameters, JsonObject& response);

            //TTS Global APIS for Resident application
            uint32_t enableTTS(const JsonObject& parameters, JsonObject& response);
            uint32_t listVoices(const JsonObject& parameters, JsonObject& response);
            uint32_t setTTSConfiguration(const JsonObject& parameters, JsonObject& response);
            uint32_t getTTSConfiguration(const JsonObject& parameters, JsonObject& response);

            //FIXME- Below Extended APIS will be implemented when it is needed.
            uint32_t pause(const JsonObject& parameters, JsonObject& response);
            uint32_t resume(const JsonObject& parameters, JsonObject& response);
            uint32_t isSpeaking(const JsonObject& parameters, JsonObject& response);
            uint32_t getSpeechState(const JsonObject& parameters, JsonObject& response);
            uint32_t setPreemptiveSpeak(const JsonObject& parameters, JsonObject& response);
            uint32_t requestExtendedEvents(const JsonObject& parameters, JsonObject& response);

            //version number API's
            uint32_t getApiVersionNumber();
            void setApiVersionNumber(uint32_t apiVersionNumber);

            void setResponseArray(JsonObject& response, const char* key, const std::vector<std::string>& items);
            //End methods
        private:
            static uint32_t m_serviceObjCount;
            static TTS::TTSManager* m_ttsManager;
            static TTSConnectionCallback* m_connectionCallback;
            TTS::Configuration m_config;
            static bool m_ttsEnabled;
            uint32_t m_apiVersionNumber;

        public:
            TextToSpeech();
            virtual ~TextToSpeech();
            void notifyClient(std::string eventname, JsonObject& param);
        };

        /**
        * @brief WPEFramework class declaration for TTSConnectionCallback 
        **/
        class TTSConnectionCallback : public TTS::TTSConnectionCallback
        {
            public:
                TTSConnectionCallback(TextToSpeech* callback)
                {
                    m_eventHandler = callback;
                }

                void onTTSStateChanged(bool state)
                {
                    JsonObject params;
                    params["state"] = JsonValue((bool)state);
                    m_eventHandler->notifyClient("onTTSStateChanged", params);
                }

                void onVoiceChanged(std::string voice)
                {
                    JsonObject params;
                    LOGINFO("TTS voice changed (%s)", voice.c_str());
                    params["voice"] = voice;
                    m_eventHandler->notifyClient("onVoiceChanged", params);
                }

                void onWillSpeak(TTS::SpeechData &data)
                {
                    JsonObject params;
                    params["speechId"]  = JsonValue((int)data.id);
                    params["text"]      = data.text;
                    m_eventHandler->notifyClient("onWillSpeak", params);
                }

                void onSpeechStart(TTS::SpeechData &data)
                {
                    JsonObject params;
                    params["speechId"]  = JsonValue((int)data.id);
                    params["text"]      = data.text;
                    m_eventHandler->notifyClient("onSpeechStart", params);
                }

                void onSpeechPause(uint32_t speechId)
                {
                    JsonObject params;
                    params["speechId"]  = JsonValue((int)speechId);
                    m_eventHandler->notifyClient("onSpeechPause", params);
                }

                void onSpeechResume(uint32_t speechId)
                {
                    JsonObject params;
                    params["speechId"]  = JsonValue((int)speechId);
                    m_eventHandler->notifyClient("onSpeechResume", params);                }

                void onSpeechCancelled(std::string id)
                {
                    char *token = strtok((char*)id.c_str(), ",");
                    uint32_t speechId = 0;
                    while(token) {
                        speechId = atol(token);
                        token = strtok(NULL, ",");
                    }
                    JsonObject params;
                    params["speechId"]  = JsonValue((int)speechId);
                    m_eventHandler->notifyClient("onSpeechCancelled", params);
                }

                void onSpeechInterrupted(uint32_t speechId)
                {
                    JsonObject params;
                    params["speechId"]  = JsonValue((int)speechId);
                    m_eventHandler->notifyClient("onSpeechInterrupted", params);
                }

                void onNetworkError(uint32_t speechId)
                {
                    JsonObject params;
                    params["speechId"]  = JsonValue((int)speechId);
                    m_eventHandler->notifyClient("onNetworkError", params);
                }

                void onPlaybackError(uint32_t speechId)
                {
                    JsonObject params;
                    params["speechId"]  = JsonValue((int)speechId);
                    m_eventHandler->notifyClient("onPlaybackError", params);
                }

                void onSpeechComplete(TTS::SpeechData &data)
                {
                    JsonObject params;
                    params["speechId"]  = JsonValue((int)data.id);
                    params["text"]      = data.text;
                    m_eventHandler->notifyClient("onSpeechComplete", params);
                }

            private:
                TextToSpeech *m_eventHandler;
        };

 } // namespace Plugin
} // namespace WPEFramework
