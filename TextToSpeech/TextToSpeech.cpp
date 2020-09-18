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
 * @file TextToSpeech.cpp
 * @brief Thunder Plugin based Implementation for TTS service API's (RDK-27957).
 */

/**
  @mainpage Text To Speech (TTS)

  <b>TextToSpeech</b> TTS Thunder Service provides APIs for the arbitrators
  * (ex: Native application such as Cobalt) to use TTS resource.
  */

#include "TextToSpeech.h"
#include <iostream>

#define TTS_MAJOR_VERSION 1
#define TTS_MINOR_VERSION 0

#define API_VERSION_NUMBER 1

namespace WPEFramework
{
    namespace Plugin
    {
        /*
         *Register TextToSpeech module as wpeframework plugin
         **/
        SERVICE_REGISTRATION(TextToSpeech, TTS_MAJOR_VERSION, TTS_MINOR_VERSION);

        /**
         * @brief class varibales to handle TTS Engine communication.
         */

        TTS::TTSManager* TextToSpeech::m_ttsManager = NULL;
        TTSEventCallback* TextToSpeech::m_eventCallback = NULL;
        TextToSpeech::TextToSpeech()
                : AbstractPlugin()
                , m_apiVersionNumber(API_VERSION_NUMBER)
        {
            LOGINFO();

            registerMethod("enabletts", &TextToSpeech::enabletts, this);
            registerMethod("listvoices", &TextToSpeech::listvoices, this);
            registerMethod("setttsconfiguration", &TextToSpeech::setttsconfiguration, this);
            registerMethod("getttsconfiguration", &TextToSpeech::getttsconfiguration, this);
            registerMethod("isttsenabled", &TextToSpeech::isttsenabled, this);
            registerMethod("speak", &TextToSpeech::speak, this);
            registerMethod("cancel", &TextToSpeech::cancel, this);
            registerMethod("pause", &TextToSpeech::pause, this);
            registerMethod("resume", &TextToSpeech::resume, this);
            registerMethod("isspeaking", &TextToSpeech::isspeaking, this);
            registerMethod("getspeechstate", &TextToSpeech::getspeechstate, this);
            registerMethod("getapiversion", &TextToSpeech::getapiversion, this);

            if(!m_eventCallback)
                m_eventCallback = new TTSEventCallback(this);

            if(!m_ttsManager)
                m_ttsManager = TTS::TTSManager::create(m_eventCallback);
        }

        TextToSpeech::~TextToSpeech()
        {
            LOGINFO();

            if(m_ttsManager) {
                delete m_ttsManager;
                m_ttsManager = NULL;
            }

            if(m_eventCallback) {
                delete m_eventCallback;
                m_eventCallback = NULL;
            }
        }

        uint32_t nextSpeechId() {
            static uint32_t counter = 0;

            if(counter >= 0xFFFFFFFF)
                counter = 0;

            return ++counter;
        }

        uint32_t TextToSpeech::enabletts(const JsonObject& parameters, JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_mutex);

            LOGINFO();
            CHECK_TTS_MANAGER_RETURN_ON_FAIL();
            CHECK_TTS_PARAMETER_RETURN_ON_FAIL("enabletts");

            auto status = m_ttsManager->enableTTS(parameters["enabletts"].Boolean());

            logResponse(status, response);
            returnResponse(status == TTS::TTS_OK);
        }

        uint32_t TextToSpeech::listvoices(const JsonObject& parameters, JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_mutex);

            LOGINFO();
            CHECK_TTS_MANAGER_RETURN_ON_FAIL();
            CHECK_TTS_PARAMETER_RETURN_ON_FAIL("language");

            std::vector<std::string> voice;
            auto status = m_ttsManager->listVoices(parameters["language"].String(), voice);
            if(status == TTS::TTS_OK)
                setResponseArray(response, "voices", voice);

            logResponse(status,response);
            returnResponse(status == TTS::TTS_OK);
        }

        uint32_t TextToSpeech::setttsconfiguration(const JsonObject& parameters, JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_mutex);

            LOGINFO();
            CHECK_TTS_MANAGER_RETURN_ON_FAIL();

            if(parameters.HasLabel("ttsendpoint"))
                m_config.ttsEndPoint = parameters["ttsendpoint"].String();

            if(parameters.HasLabel("ttsendpointsecured"))
                m_config.ttsEndPointSecured = parameters["ttsendpointsecured"].String();

            if(parameters.HasLabel("language"))
                m_config.language = parameters["language"].String();

            if(parameters.HasLabel("voice"))
                m_config.voice = parameters["voice"].String();

            if(parameters.HasLabel("volume"))
                m_config.volume = stod(parameters["volume"].String());

            if(parameters.HasLabel("rate")) {
                int rate=0;
                getNumberParameter("rate", rate);
                m_config.rate = static_cast<uint8_t>(rate);
            }

            auto status = m_ttsManager->setConfiguration(m_config);

            logResponse(status, response);
            returnResponse(status == TTS::TTS_OK);
        }

        uint32_t TextToSpeech::getttsconfiguration(const JsonObject& parameters, JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_mutex);

            LOGINFO();
            CHECK_TTS_MANAGER_RETURN_ON_FAIL();

            TTS::Configuration ttsConfig;
            auto status = m_ttsManager->getConfiguration(ttsConfig);
            if(status == TTS::TTS_OK) {
                response["ttsendpoint"]         = ttsConfig.ttsEndPoint;
                response["ttsendpointsecured"]  = ttsConfig.ttsEndPointSecured;
                response["language"]            = ttsConfig.language;
                response["voice"]               = ttsConfig.voice;
                response["rate"]                = (int) ttsConfig.rate;
                response["volume"]              = std::to_string(ttsConfig.volume);
            }

            logResponse(status, response);
            returnResponse(status == TTS::TTS_OK);
        }

        uint32_t TextToSpeech::isttsenabled(const JsonObject& parameters ,JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_mutex);

            LOGINFO();
            CHECK_TTS_MANAGER_RETURN_ON_FAIL();

            response["isenabled"] = JsonValue((bool)m_ttsManager->isTTSEnabled());

            logResponse(TTS::TTS_OK,response);
            returnResponse(true);
        }

        uint32_t TextToSpeech::speak(const JsonObject& parameters, JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_mutex);

            LOGINFO();
            CHECK_TTS_MANAGER_RETURN_ON_FAIL();
            CHECK_TTS_PARAMETER_RETURN_ON_FAIL("text");

            uint32_t speechId = nextSpeechId();
            auto status = m_ttsManager->speak(speechId, parameters["text"].String());
            if(status == TTS::TTS_OK)
                response["speechid"] = (int) speechId;

            logResponse(status, response);
            returnResponse(status == TTS::TTS_OK);
        }

        uint32_t TextToSpeech::cancel(const JsonObject& parameters, JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_mutex);

            LOGINFO();
            CHECK_TTS_MANAGER_RETURN_ON_FAIL();
            CHECK_TTS_PARAMETER_RETURN_ON_FAIL("speechid");

            auto status = m_ttsManager->shut(parameters["speechid"].Number());

            logResponse(status, response);
            returnResponse(status == TTS::TTS_OK);
        }


        uint32_t TextToSpeech::pause(const JsonObject& parameters, JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_mutex);

            LOGINFO();
            CHECK_TTS_MANAGER_RETURN_ON_FAIL();
            CHECK_TTS_PARAMETER_RETURN_ON_FAIL("speechid");

            auto status = m_ttsManager->pause(parameters["speechid"].Number());

            logResponse(status, response);
            returnResponse(status == TTS::TTS_OK);
        }

        uint32_t TextToSpeech::resume(const JsonObject& parameters, JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_mutex);

            LOGINFO();
            CHECK_TTS_MANAGER_RETURN_ON_FAIL();
            CHECK_TTS_PARAMETER_RETURN_ON_FAIL("speechid");

            auto status = m_ttsManager->resume(parameters["speechid"].Number());

            logResponse(status, response);
            returnResponse(status == TTS::TTS_OK);
        }

        uint32_t TextToSpeech::isspeaking(const JsonObject& parameters, JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_mutex);

            LOGINFO();
            CHECK_TTS_MANAGER_RETURN_ON_FAIL();
            CHECK_TTS_PARAMETER_RETURN_ON_FAIL("speechid");

            bool speaking = false;
            auto status = m_ttsManager->isSpeaking(parameters["speechid"].Number(), speaking);
            response["speaking"] = speaking;

            logResponse(status, response);
            returnResponse(status == TTS::TTS_OK);
        }

        uint32_t TextToSpeech::getspeechstate(const JsonObject& parameters, JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_mutex);

            LOGINFO();
            CHECK_TTS_MANAGER_RETURN_ON_FAIL();
            CHECK_TTS_PARAMETER_RETURN_ON_FAIL("speechid");

            TTS::SpeechState state;
            auto status = m_ttsManager->getSpeechState(parameters["speechid"].Number(), state);
            if(status == TTS::TTS_OK)
                response["speechstate"] = (int) state;

            logResponse(status, response);
            returnResponse(status == TTS::TTS_OK);
        }

        uint32_t TextToSpeech::getapiversion(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();
            UNUSED(parameters);

            response["version"] = m_apiVersionNumber;

            returnResponse(true);
        }

        void TextToSpeech::setResponseArray(JsonObject& response, const char* key, const std::vector<std::string>& items)
        {
            JsonArray arr;
            for(auto& i : items) arr.Add(JsonValue(i));

            response[key] = arr;
        }

        void TextToSpeech::notifyClient(string eventname, JsonObject& params)
        {
            sendNotify(eventname.c_str(), params);
        }

    } // namespace Plugin
} // namespace WPEFramework
