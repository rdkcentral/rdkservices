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
 * @file TextToSpeech.cpp
 * @brief Thunder Plugin based Implementation for TTS service API's (RDK-27957).
 */

/**
  @mainpage Text To Speech (TTS)

  <b>TextToSpeech</b> TTS Thunder Service provides APIs for the arbitrators
  * (ex: Native application such as Cobalt) to use TTS resource.
  */

#include "TextToSpeech.h"

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
        bool TextToSpeech::m_ttsEnabled= false;
        uint32_t TextToSpeech::m_serviceObjCount = 0;
        TTS::TTSManager* TextToSpeech::m_ttsManager = NULL;
        TTSConnectionCallback* TextToSpeech::m_connectionCallback = NULL;
        TextToSpeech::TextToSpeech()
                : AbstractPlugin()
                , m_apiVersionNumber(API_VERSION_NUMBER)
        {
            LOGINFO();
            registerMethod("enableTTS", &TextToSpeech::enableTTS, this);
            registerMethod("listVoices", &TextToSpeech::listVoices, this);
            registerMethod("setTTSConfiguration", &TextToSpeech::setTTSConfiguration, this);
            registerMethod("getTTSConfiguration", &TextToSpeech::getTTSConfiguration, this);
            registerMethod("isTTSEnabled", &TextToSpeech::isTTSEnabled, this);
            registerMethod("speak", &TextToSpeech::speak, this);
            registerMethod("cancel", &TextToSpeech::cancel, this);
            registerMethod("pause", &TextToSpeech::pause, this);
            registerMethod("resume", &TextToSpeech::resume, this);
            registerMethod("isSpeaking", &TextToSpeech::isSpeaking, this); 
            registerMethod("getSpeechState", &TextToSpeech::getSpeechState, this);
            registerMethod("setPreemptiveSpeak", &TextToSpeech::setPreemptiveSpeak, this);
            registerMethod("requestExtendedEvents", &TextToSpeech::requestExtendedEvents, this);
        
            if(!m_connectionCallback)
                m_connectionCallback = new TTSConnectionCallback(this);

            if(!m_ttsManager) {
                m_ttsManager = TTS::TTSManager::create(m_connectionCallback);

                //FIXME Hardcoding of this enableTTS will be removed once Resident application is implemented.
                m_ttsEnabled = true;
                m_ttsManager->enableTTS(m_ttsEnabled);
            }

            m_serviceObjCount++;
        }

        TextToSpeech::~TextToSpeech()
        {
            LOGINFO();
            m_serviceObjCount--;
            if(m_serviceObjCount == 0)
            {
                if(m_ttsManager)
                {
                    delete m_ttsManager;
                    m_ttsManager = NULL;
                }

                if(m_connectionCallback)
                {
                    delete m_connectionCallback;
                    m_connectionCallback = NULL;
                }

            }
        }

        uint32_t TextToSpeech::enableTTS(const JsonObject& parameters, JsonObject& response)
        {
      	    bool success = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

             CHECK_CONNECTION_RETURN_ON_FAIL(success);

            if (parameters.HasLabel("enableTTS"))
            {
                bool value = parameters["enableTTS"].Boolean();
                m_ttsEnabled = value;
                status = m_ttsManager->enableTTS(m_ttsEnabled);
                success = true;
            }
            logResponse(status, response);
    	    returnResponse(success);
        }

        uint32_t TextToSpeech::listVoices(const JsonObject& parameters, JsonObject& response)
        {
            bool success = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            if(parameters.HasLabel("language"))
            {
                std::vector<std::string> voice;
                std::string lang = parameters["language"].String();
                status = m_ttsManager->listVoices(lang, voice);
                if(status == TTS::TTS_OK)
                {
                    setResponseArray(response, "voices", voice);
                    success = true;
                }
            }

            logResponse(status,response);
            returnResponse(success);
        }

        uint32_t TextToSpeech::setTTSConfiguration(const JsonObject& parameters, JsonObject& response)
        {
            bool success = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);


            if (parameters.HasLabel("ttsEndPoint"))
            {
                m_config.ttsEndPoint = parameters["ttsEndPoint"].String();
            }
            if (parameters.HasLabel("ttsEndPointSecured"))
            {
                m_config.ttsEndPointSecured = parameters["ttsEndPointSecured"].String();
            }
            if (parameters.HasLabel("language"))
            {   
                m_config.language = parameters["language"].String();
            }
            if (parameters.HasLabel("voice"))
            {
                m_config.voice = parameters["voice"].String();
            }
            if (parameters.HasLabel("volume"))
            {   
                m_config.volume = stod(parameters["volume"].String());
            }
            if (parameters.HasLabel("rate"))
            {
                int rate=0;
                getNumberParameter("rate", rate);
                m_config.rate = static_cast<uint8_t>(rate);
            }

            status = m_ttsManager->setConfiguration(m_config);
            if(status == TTS::TTS_OK)
                success = true;
            logResponse(status, response);
            returnResponse(success);
        }

        uint32_t TextToSpeech::getTTSConfiguration(const JsonObject& parameters, JsonObject& response)
        {
            bool success = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            TTS::Configuration ttsConfig;
            status = m_ttsManager->getConfiguration(ttsConfig);
            if(status == TTS::TTS_OK)
            {
                std::string volume;
                response["ttsEndPoint"]         = ttsConfig.ttsEndPoint; 
                response["ttsEndPointSecured"]  = ttsConfig.ttsEndPointSecured;
                response["language"]            = ttsConfig.language;
                response["voice"]               = ttsConfig.voice;
                response["rate"]                = (int) ttsConfig.rate;
                volume = std::to_string(ttsConfig.volume);
                response["volume"]              = volume;
                success = true;
            }
            logResponse(status, response);
            returnResponse(success); 
        }

        uint32_t TextToSpeech::isTTSEnabled(const JsonObject& parameters ,JsonObject& response)
        {
            bool success = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            m_ttsEnabled = m_ttsManager->isTTSEnabled();
            response["isEnabled"] = JsonValue((bool)m_ttsEnabled);

            status = TTS::TTS_OK;
            success = true;
            logResponse(status,response);

            returnResponse(success);
        }

        uint32_t TextToSpeech::speak(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            bool success = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            std::string text = parameters["text"].String();

            if(!m_ttsEnabled) {
                LOGERR("TTS is disabled, can't speak");
                status = TTS::TTS_NOT_ENABLED;
                logResponse(status, response);
                returnResponse(success);
            }

            status = m_ttsManager->speak(text);            

            if(status == TTS::TTS_OK)
                success = true;

            logResponse(status, response);
            returnResponse(success);
        }

        uint32_t TextToSpeech::cancel(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            bool success = false;
            bool clearPending = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            clearPending = parameters["clearPending"].Boolean();

            if(!m_ttsEnabled) {
                LOGERR("TTS is disabled, nothing to pause");
                status = TTS::TTS_NOT_ENABLED;
                logResponse(status, response);
                returnResponse(success);
            }

            if(clearPending)
                status = m_ttsManager->abortAndClearPending();
            else
                status = m_ttsManager->shut();
            
            success = true;
            logResponse(status, response);

            returnResponse(success);
        }

        uint32_t TextToSpeech::pause(const JsonObject& parameters, JsonObject& response)
        {
            // FIXME: implement this function when it is needed
            returnResponse(false);
        }

        uint32_t TextToSpeech::resume(const JsonObject& parameters, JsonObject& response)
        {
            // FIXME: implement this function when it is needed
            returnResponse(false);
        }

        uint32_t TextToSpeech::isSpeaking(const JsonObject& parameters, JsonObject& response)
        {
            // FIXME: implement this function when it is needed
            returnResponse(false);
        }

        uint32_t TextToSpeech::getSpeechState(const JsonObject& parameters, JsonObject& response)
        {
            // FIXME: implement this function when it is needed
            returnResponse(false);
        }

        uint32_t TextToSpeech::setPreemptiveSpeak(const JsonObject& parameters, JsonObject& response)
        {
            // FIXME: implement this function when it is needed
            returnResponse(false);
        }

        uint32_t TextToSpeech::requestExtendedEvents(const JsonObject& parameters, JsonObject& response)
        {
            // FIXME: implement this function when it is needed
            returnResponse(false);
        }

        /* @brief        : To get API version Number.
         * @param[in]     : void
         * @param[out]    : m_apiVersionNumber
         */
         uint32_t TextToSpeech::getApiVersionNumber()
         {
                 return m_apiVersionNumber;
         }    

        /* @brief        : To set API version Number.
         * @param[in]     : apiVersionNumber
         * @param[out]    : void
         */
         void TextToSpeech::setApiVersionNumber(unsigned int apiVersionNumber)
         {
             LOGINFO("Set API Version Number = %d\n", apiVersionNumber);
             m_apiVersionNumber = apiVersionNumber;
         }

         void TextToSpeech::setResponseArray(JsonObject& response, const char* key, const std::vector<std::string>& items)
         {
             JsonArray arr;
             for(auto& i : items) arr.Add(JsonValue(i));

             response[key] = arr;

             string json;
             response.ToString(json);
         }

         void TextToSpeech::notifyClient(string eventname, JsonObject& params)
         {
             //property added in registeredProperty list hence call sendNotify
             sendNotify(eventname.c_str(), params);
         }
         
    } // namespace Plugin
} // namespace WPEFramework
