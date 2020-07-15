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
 * @file TTSResource.cpp
 * @brief Thunder Plugin based Implementation for TTS service API's (RDK-RDK-25832).
 */

/**
  @mainpage Text To Speech (TTS)

  <b>TTSResource</b> TTS Resource Service provides APIs for the arbitrators 
  * (ex: Receiver / Optimus App Manager) to reserve the TTS resource for a particular
  * Application. Only when a TTS resource is reserved for an app, the service can be 
  * used by the apps. (i.e., if the app has a session, its session will
  * be made "active". If the app does not have a session, whenever the 
  * session is created, it will be made active.
  */

#pragma once

#include <stdint.h>
#include <syscall.h>
#include "Module.h"
#include "impl/TTSManager.h"
#include "impl/TTSSession.h"
#include "tracing/Logging.h"
#include "utils.h"
#include "AbstractPlugin.h"
#include "TTSServicesCommon.h"
#include "impl/TextToSpeechCommon.h"
#include <vector>

namespace WPEFramework {
    namespace Plugin {

        class TTSConnectionCallback;
        class TTSSessionServiceCallback;
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

            // TTS Global APIs
            uint32_t enableTTS(const JsonObject& parameters, JsonObject& response);
            uint32_t listVoices(const JsonObject& parameters, JsonObject& response);
            uint32_t setTTSConfiguration(const JsonObject& parameters, JsonObject& response);
            uint32_t getTTSConfiguration(const JsonObject& parameters, JsonObject& response);
            bool isTTSEnabled(const JsonObject& parameters ,JsonObject& response);
            uint32_t isSessionActiveForApp(const JsonObject& parameters, JsonObject& response);


            // Resource management APIs
            uint32_t acquireResource(const JsonObject& parameters, JsonObject& response);
            uint32_t claimResource(const JsonObject& parameters, JsonObject& response);
            uint32_t releaseResource(const JsonObject& parameters, JsonObject& response);

            //Session management APIs
            uint32_t createSession(const JsonObject& parameters, JsonObject& response);
            uint32_t destroySession(const JsonObject& parameters, JsonObject& response);
            uint32_t isActiveSession(const JsonObject& parameters, JsonObject& response);
            uint32_t setPreemptiveSpeak(const JsonObject& parameters, JsonObject& response);
            uint32_t requestExtendedEvents(const JsonObject& parameters, JsonObject& response);

            // Speak APIs
            uint32_t speak(const JsonObject& parameters, JsonObject& response);
            uint32_t pause(const JsonObject& parameters, JsonObject& response);
            uint32_t resume(const JsonObject& parameters, JsonObject& response);
            uint32_t abort(const JsonObject& parameters, JsonObject& response);
            uint32_t isSpeaking(const JsonObject& parameters, JsonObject& response);
            uint32_t getSpeechState(const JsonObject& parameters, JsonObject& response);

            //version number API's
            uint32_t getApiVersionNumber();
            void setApiVersionNumber(uint32_t apiVersionNumber);

            TTS::ResourceAllocationPolicy getResourceAllocationPolicy();
            void setResponseArray(JsonObject& response, const char* key, const std::vector<std::string>& items);
            //End methods
        private:
            static uint32_t m_serviceObjCount;
            static TTS::TTSManager* m_ttsManager;
            static TTSConnectionCallback* m_connectionCallback;
            static TTSSessionServiceCallback* m_sessionCallback;
            TTS::Configuration m_config;
            static TTS::ResourceAllocationPolicy m_policy;
            static bool m_ttsEnabled;
            uint32_t m_apiVersionNumber;
            std::map<uint32_t, SessionInfo*> m_sessionMap;

        public:
            TextToSpeech();
            virtual ~TextToSpeech();
            void restoreTextToSpeech();
            void notifyClient(std::string eventname, JsonObject& param);
            void ResourceAcquired(uint32_t sessionId);
        };

        /**
        * @brief WPEFramework class declaration for TTSResourceCallback
        **/
        class TTSConnectionCallback : public TTS::TTSConnectionCallback
        {
            public:
                TTSConnectionCallback(TextToSpeech* callback)
                {
                    m_eventHandler = callback;
                }

                void onTTSServerConnected()
                {
                    LOGINFO("TTS Server Connected");
                    m_eventHandler->restoreTextToSpeech();
                }

                void onTTSServerClosed()
                {
                    LOGINFO("TTS Server Closed");
                }

                void onTTSStateChanged(bool state)
                {
                    JsonObject params;
                    params["state"] = JsonValue((bool)state);
                    LOGINFO("TTS state changed to '%d'\n", state);
                    m_eventHandler->notifyClient("onTTSStateChanged", params);
                }

                void onVoiceChanged(std::string voice)
                {
                    JsonObject params;
                    LOGINFO("TTS voice changed (%s)", voice.c_str());
                    params["voice"] = voice;
                    m_eventHandler->notifyClient("onVoiceChanged", params);
                }
            private:
                TextToSpeech *m_eventHandler;
        };

       class TTSSessionServiceCallback : public TTS::TTSSessionCallback
        {
            public:
                TTSSessionServiceCallback(TextToSpeech* callback )
                {
                    m_eventHandler = callback;
                }

                ~TTSSessionServiceCallback() {}

                void onResourceAcquired(uint32_t appId, uint32_t sessionId)
                {
                    LOGINFO("onResourceAcquired appId(%d) sessionId(%d)", appId, sessionId);
                    m_eventHandler->ResourceAcquired(sessionId);
                    JsonObject params;
                    params["appId"]     = JsonValue((int)appId);
                    params["sessionId"] = JsonValue((int)sessionId);
                    m_eventHandler->notifyClient("onResourceAcquired", params);
                }

                void onResourceReleased(uint32_t appId, uint32_t sessionId)
                {
                    LOGINFO("onResourceReleased appId(%d) sessionId(%d)", appId, sessionId);
                    JsonObject params;
                    params["appId"]     = JsonValue((int)appId);
                    params["sessionId"] = JsonValue((int)sessionId);
                    m_eventHandler->notifyClient("onResourceReleased", params);

                }

                void onTTSSessionCreated(uint32_t appId, uint32_t sessionId)
                {
                    LOGINFO("onTTSSessionCreated appId(%d) sessionId(%d)", appId, sessionId);
                    JsonObject params;
                    params["appId"]     = JsonValue((int)appId);
                    params["sessionId"] = JsonValue((int)sessionId);
                    m_eventHandler->notifyClient("onTTSSessionCreated", params);
                }


                void onWillSpeak(uint32_t appId, uint32_t sessionId, TTS::SpeechData &data)
                {
                    LOGINFO("onWillSpeak appId(%d) sessionId(%d)", appId, sessionId);
                    JsonObject params;
                    params["appId"]     = JsonValue((int)appId);
                    params["sessionId"] = JsonValue((int)sessionId);
                    params["speechId"]  = JsonValue((int)data.id);
                    params["text"]      = data.text;
                    m_eventHandler->notifyClient("onWillSpeak", params);
                }

                void onSpeechStart(uint32_t appId, uint32_t sessionId, TTS::SpeechData &data)
                {
                    LOGINFO("onSpeechStart(%d) sessionId(%d)", appId, sessionId);
                    JsonObject params;
                    params["appId"]     = JsonValue((int)appId);
                    params["sessionId"] = JsonValue((int)sessionId);
                    params["speechId"]  = JsonValue((int)data.id);
                    params["text"]      = data.text;
                    m_eventHandler->notifyClient("onSpeechStart", params);
                }

                void onSpeechPause(uint32_t appId, uint32_t sessionId, uint32_t speechId)
                {
                    LOGINFO("onSpeechPause appId(%d) sessionId(%d) speechId(%d)", appId, sessionId, speechId);
                    JsonObject params;
                    params["appId"]     = JsonValue((int)appId);
                    params["sessionId"] = JsonValue((int)sessionId);
                    params["speechId"]  = JsonValue((int)speechId);
                    m_eventHandler->notifyClient("onSpeechPause", params);
                }

                void onSpeechResume(uint32_t appId, uint32_t sessionId, uint32_t speechId) 
                {
                    LOGINFO("onSpeechResume appId(%d) sessionId(%d) speechId(%d)", appId, sessionId, speechId);
                    JsonObject params;
                    params["appId"]     = JsonValue((int)appId);
                    params["sessionId"] = JsonValue((int)sessionId);
                    params["speechId"]  = JsonValue((int)speechId);
                    m_eventHandler->notifyClient("onSpeechResume", params);                }

                void onSpeechCancelled(uint32_t appId, uint32_t sessionId, std::string id)
                {
                    char *token = strtok((char*)id.c_str(), ",");
                    uint32_t speechId = 0;
                    while(token) {
                        speechId = atol(token);
                        token = strtok(NULL, ",");
                    }
                    LOGINFO("onSpeechCancelled appId(%d) sessionId(%d) speechId(%d)", appId, sessionId, speechId);
                    JsonObject params;
                    params["appId"]     = JsonValue((int)appId);
                    params["sessionId"] = JsonValue((int)sessionId);
                    params["speechId"]  = JsonValue((int)speechId);
                    m_eventHandler->notifyClient("onSpeechCancelled", params);
                }

                void onSpeechInterrupted(uint32_t appId, uint32_t sessionId, uint32_t speechId)
                {
                    LOGINFO("onSpeechInterrupted appId(%d) sessionId(%d) speechId(%d)", appId, sessionId, speechId);
                    JsonObject params;
                    params["appId"]     = JsonValue((int)appId);
                    params["sessionId"] = JsonValue((int)sessionId);
                    params["speechId"]  = JsonValue((int)speechId);
                    m_eventHandler->notifyClient("onSpeechInterrupted", params);
                }

                void onNetworkError(uint32_t appId, uint32_t sessionId, uint32_t speechId)
                {
                    LOGINFO("onNetworkError appId(%d) sessionId(%d) speechId(%d)", appId, sessionId, speechId);
                    JsonObject params;
                    params["appId"]     = JsonValue((int)appId);
                    params["sessionId"] = JsonValue((int)sessionId);
                    params["speechId"]  = JsonValue((int)speechId);
                    m_eventHandler->notifyClient("onNetworkError", params);
                }

                void onPlaybackError(uint32_t appId, uint32_t sessionId, uint32_t speechId)
                {
                    LOGINFO("onPlaybackError appId(%d) sessionId(%d) speechId(%d)", appId, sessionId, speechId);
                    JsonObject params;
                    params["appId"]     = JsonValue((int)appId);
                    params["sessionId"] = JsonValue((int)sessionId);
                    params["speechId"]  = JsonValue((int)speechId);
                    m_eventHandler->notifyClient("onPlaybackError", params);
                }

                void onSpeechComplete(uint32_t appId, uint32_t sessionId, TTS::SpeechData &data)
                {
                    LOGINFO("onSpeechComplete appId(%d) sessionId(%d)", appId, sessionId);
                    JsonObject params;
                    params["appId"]     = JsonValue((int)appId);
                    params["sessionId"] = JsonValue((int)sessionId);
                    params["speechId"]  = JsonValue((int)data.id);
                    params["text"]      = data.text;
                    m_eventHandler->notifyClient("onSpeechComplete", params);
                }

            private:
                TextToSpeech *m_eventHandler;
        };

 } // namespace Plugin
} // namespace WPEFramework
