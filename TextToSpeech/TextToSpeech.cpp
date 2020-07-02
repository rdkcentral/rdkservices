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
 * @brief Thunder Plugin based Implementation for TTS service API's (RDK-RDK-25832).
 */

/**
  @mainpage Text To Speech (TTS)

  <b>TextToSpeech</b> TTS Service provides APIs for the arbitrators 
 * (ex: Receiver / Optimus App Manager) to reserve the TTS resource for a particular
 * Application. Only when a TTS resource is reserved for an app, the service can be 
 * used by the apps. (i.e., if the app has a session, its session will
 * be made "active". If the app does not have a session, whenever the 
 * session is created, it will be made active.
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
        TTSSessionServiceCallback* TextToSpeech::m_sessionCallback = NULL;
        TTS::ResourceAllocationPolicy TextToSpeech::m_policy = TTS::INVALID_POLICY;
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
            registerMethod("isSessionActiveForApp", &TextToSpeech::isSessionActiveForApp, this);
            registerMethod("acquireResource", &TextToSpeech::acquireResource, this);
            registerMethod("claimResource", &TextToSpeech::claimResource, this);
            registerMethod("releaseResource", &TextToSpeech::releaseResource, this);
            registerMethod("createSession", &TextToSpeech::createSession, this);
            registerMethod("destroySession", &TextToSpeech::destroySession, this);
            registerMethod("isActiveSession", &TextToSpeech::isActiveSession, this);
            registerMethod("setPreemptiveSpeak", &TextToSpeech::setPreemptiveSpeak, this);
            registerMethod("speak", &TextToSpeech::speak, this);
            registerMethod("pause", &TextToSpeech::pause, this);
            registerMethod("resume", &TextToSpeech::resume, this);
            registerMethod("abort", &TextToSpeech::abort, this);
            registerMethod("isSpeaking", &TextToSpeech::isSpeaking, this); 
            registerMethod("getSpeechState", &TextToSpeech::getSpeechState, this);
            registerMethod("requestExtendedEvents", &TextToSpeech::requestExtendedEvents, this);
        
            if(!m_sessionCallback)
                m_sessionCallback = new TTSSessionServiceCallback(this);

            if(!m_connectionCallback)
                m_connectionCallback = new TTSConnectionCallback(this);

            if(!m_ttsManager) {
                m_ttsManager = TTS::TTSManager::create(m_connectionCallback);
                if(m_ttsManager)
                {
                    m_policy = getResourceAllocationPolicy();
                    LOGINFO("%d(%s) policy is enforced by TTSEngine", m_policy, policyStr(m_policy));
                    if(m_connectionCallback)
                        m_connectionCallback->onTTSServerConnected();   
                }
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

    	    	if(m_sessionCallback)
    	    	{				
                    delete m_sessionCallback;
                    m_sessionCallback = NULL;
    	        }		
            }
        }

        void TextToSpeech::restoreTextToSpeech()
        {
            if(m_ttsEnabled && m_ttsManager)
            {
                m_ttsManager->enableTTS(m_ttsEnabled);
                LOGWARN("Restored TTS connection and settings");
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
                response["rate"]                = ttsConfig.rate;
                volume = std::to_string(ttsConfig.volume);
                response["volume"]              = volume;
                success = true;
            }
            logResponse(status, response);
            returnResponse(success); 
        }

        bool TextToSpeech::isTTSEnabled(const JsonObject& parameters ,JsonObject& response)
        {
            bool success = false;
            bool force = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            if (parameters.HasLabel("force"))
            {
                force = parameters["force"].Boolean();
            }

            if(!force){
                response["isEnabled"] = JsonValue((bool)m_ttsEnabled);
            } else {
                m_ttsEnabled = m_ttsManager->isTTSEnabled();
                response["isEnabled"] = JsonValue((bool)m_ttsEnabled);
            }

            status = TTS::TTS_OK;
            success = true;
            logResponse(status,response);

            returnResponse(success);
        }

        uint32_t TextToSpeech::isSessionActiveForApp(const JsonObject& parameters, JsonObject& response)
        {
            bool success = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            if (parameters.HasLabel("appId"))
            {
                int app_id = 0;
                getNumberParameter("appId", app_id);
                response["isActive"] = m_ttsManager->isSessionActiveForApp(static_cast<uint32_t>(app_id));
                status = TTS::TTS_OK;
                success = true;
            }
            logResponse(status, response);
            returnResponse(success);
        }

        uint32_t TextToSpeech::acquireResource(const JsonObject& parameters, JsonObject& response)
        {
            bool success = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            if(m_policy != TTS::RESERVATION) {
                LOGERR("Non-Reservation policy (i.e %s) is in effect, declining request", policyStr(m_policy));
                status = TTS::TTS_POLICY_VIOLATION;
                logResponse(status, response);
                returnResponse(success);
            }

            if(parameters.HasLabel("appId"))
            {
                uint32_t app_id = parameters["appId"].Number();
                status = m_ttsManager->reservePlayerResource(app_id);
                if(status != TTS::TTS_OK)
                {
                    LOGERR("Couldn't request reservation of resource, TTS Code = %d", status);
                    logResponse(status, response);
                    returnResponse(success);
                }
                success = true;
            }
            logResponse(status, response);
            returnResponse(success);
        }

        uint32_t TextToSpeech::claimResource(const JsonObject& parameters, JsonObject& response)
        {
            bool success = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            if(m_policy != TTS::RESERVATION) {
                LOGERR("Non-Reservation policy (i.e %s) is in effect, declining request", policyStr(m_policy));
                status = TTS::TTS_POLICY_VIOLATION;
                logResponse(status, response);
                returnResponse(success);
            }

            if(parameters.HasLabel("appId"))
            {
                uint32_t app_id = parameters["appId"].Number();
                status = m_ttsManager->claimPlayerResource(app_id);
                if(status != TTS::TTS_OK)
                {
                    LOGERR("Couldn't claim reservation of resource, TTS Code = %d", status);
                    logResponse(status, response);
                    returnResponse(success);
                }
                success = true;
            }
            logResponse(status, response);
            returnResponse(success);
        }   

        uint32_t TextToSpeech::releaseResource(const JsonObject& parameters, JsonObject& response)
        {
            bool success = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            if(m_policy != TTS::RESERVATION) {
                LOGERR("Non-Reservation policy (i.e %s) is in effect, declining request", policyStr(m_policy));
                status = TTS::TTS_POLICY_VIOLATION;
                logResponse(status, response);
                returnResponse(success);
            }

            if(parameters.HasLabel("appId"))
            {
                uint32_t app_id = parameters["appId"].Number();
                status = m_ttsManager->releasePlayerResource(app_id);
                if(status != TTS::TTS_OK)
                {
                    LOGERR("Resource release didn't succeed, TTS Code = %d", status);
                    logResponse(status, response);
                    returnResponse(success);
                }
                success = true;
            }
            logResponse(status, response);
            returnResponse(success);
        }

        uint32_t TextToSpeech::createSession(const JsonObject& parameters, JsonObject& response)
        {
            bool success = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            TTS::TTSSession *sessionObject=NULL;
            uint32_t sessionId;
            bool ttsEnabled = false;
            string appName = parameters["appName"].String();
            int appId = 0;
            uint32_t app_id = 0;
            getNumberParameter("appId", appId);
            app_id = static_cast<uint32_t>(appId);

            if(app_id && m_sessionCallback)
            {
                SessionInfo *sessionInfo = new SessionInfo();
                sessionInfo->m_appId = app_id;
                sessionInfo->m_appName = appName;
                if(m_policy != TTS::RESERVATION) {
                    sessionInfo->m_gotResource = true;
                }

                sessionObject = m_ttsManager->createSession(app_id, appName, sessionId, ttsEnabled, status, m_sessionCallback);
                if(!sessionObject){
                    LOGWARN("Session couldn't be created for App (\"%u\", \"%s\")", app_id, appName.c_str());
                    delete sessionInfo;
                } else {
                    LOGINFO("Created TTS SessionId = %d, for appId = %d, appname = %s, ttsEnabled = %d", sessionId, app_id, appName.c_str(), ttsEnabled);
                    sessionInfo->m_sessionId = sessionId;
                    sessionInfo->m_session = sessionObject;
                    m_sessionMap[sessionInfo->m_sessionId] = sessionInfo;
                    response["TTSSessionId"] = JsonValue(sessionId);
                    if(m_connectionCallback) {
                        m_ttsEnabled = sessionId;
                        m_connectionCallback->onTTSStateChanged(m_ttsEnabled);
                    }
                    m_sessionCallback->onTTSSessionCreated(sessionInfo->m_appId, sessionInfo->m_sessionId);
                    success = true;
                }
            }

            logResponse(status, response);
            returnResponse(success);
        }

        uint32_t TextToSpeech::destroySession(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            bool success = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            SessionInfo *sessionInfo;
            std::map<uint32_t, SessionInfo*>::iterator sessionItr;

            uint32_t sessionId = parameters["sessionId"].Number();

            CHECK_SESSION_RETURN_ON_FAIL(sessionId, sessionItr, sessionInfo, success);

            status = m_ttsManager->destroySession(sessionId);

            if(status != TTS::TTS_OK) {
                LOGWARN("Failed to destroy TTS SessionId = %d", sessionId);
                logResponse(TTS::TTS_FAIL,response);
                returnResponse(success);            
            }

            LOGINFO("destroy TTS SessionId = %d", sessionId);
            delete sessionInfo;
            m_sessionMap.erase(sessionItr);
            success = true;

            logResponse(status,response);
            returnResponse(success);
        }

        uint32_t TextToSpeech::isActiveSession(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            bool success = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            SessionInfo *sessionInfo;
            std::map<uint32_t, SessionInfo*>::iterator sessionItr;
            bool forcefetch = false;

            if (parameters.HasLabel("forcefetch"))
                forcefetch = parameters["forcefetch"].Boolean();

            uint32_t sessionId = parameters["sessionId"].Number();

            CHECK_SESSION_RETURN_ON_FAIL(sessionId, sessionItr, sessionInfo, success);

            if (forcefetch)
            {
                bool active = false;
                sessionInfo->m_session->isActive(active);
                sessionInfo->m_gotResource = active;
            }

            response["isActive"] = sessionInfo->m_gotResource;

            status = TTS::TTS_OK;
            success = true;
            logResponse(status, response);
            returnResponse(success);
        }


        uint32_t TextToSpeech::setPreemptiveSpeak(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            bool success = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            SessionInfo *sessionInfo;
            std::map<uint32_t, SessionInfo*>::iterator sessionItr;
            bool preemptive=true;

            uint32_t sessionId = parameters["sessionId"].Number();
            CHECK_SESSION_RETURN_ON_FAIL(sessionId, sessionItr, sessionInfo, success);
            if (parameters.HasLabel("preemptive"))
                preemptive = parameters["preemptive"].Boolean();

            status = sessionInfo->m_session->setPreemptiveSpeak(preemptive);
            success = true;
            logResponse(status, response);
            returnResponse(success);
        }

        uint32_t TextToSpeech::requestExtendedEvents(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            bool success = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            SessionInfo *sessionInfo;
            std::map<uint32_t, SessionInfo*>::iterator sessionItr;

            uint32_t sessionId = parameters["sessionId"].Number();
            CHECK_SESSION_RETURN_ON_FAIL(sessionId, sessionItr, sessionInfo, success);

            uint32_t extendedEvents = parameters["extendedEvents"].Number();
            
            SET_UNSET_EXTENDED_EVENT(sessionInfo, extendedEvents, TTS::EXT_EVENT_WILL_SPEAK, "willSpeak");
            SET_UNSET_EXTENDED_EVENT(sessionInfo, extendedEvents, TTS::EXT_EVENT_PAUSED, "paused");
            SET_UNSET_EXTENDED_EVENT(sessionInfo, extendedEvents, TTS::EXT_EVENT_RESUMED, "resumed");
            SET_UNSET_EXTENDED_EVENT(sessionInfo, extendedEvents, TTS::EXT_EVENT_CANCELLED, "cancelled");
            SET_UNSET_EXTENDED_EVENT(sessionInfo, extendedEvents, TTS::EXT_EVENT_INTERRUPTED, "interrupted");
            SET_UNSET_EXTENDED_EVENT(sessionInfo, extendedEvents, TTS::EXT_EVENT_NETWORK_ERROR, "networkerror");
            SET_UNSET_EXTENDED_EVENT(sessionInfo, extendedEvents, TTS::EXT_EVENT_PLAYBACK_ERROR, "playbackerror");

            status = sessionInfo->m_session->requestExtendedEvents(sessionInfo->m_extendedEvents);
            success = true;
            logResponse(status, response);            

            returnResponse(success); 
        }

        uint32_t TextToSpeech::speak(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            bool success = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            SessionInfo *sessionInfo;
            std::map<uint32_t, SessionInfo*>::iterator sessionItr;

            uint32_t sessionId = parameters["sessionId"].Number();
            uint32_t id = parameters["id"].Number();
            std::string text = parameters["text"].String();
            bool secure = parameters["secure"].Boolean();

            CHECK_SESSION_RETURN_ON_FAIL(sessionId, sessionItr, sessionInfo, success);

            if(!m_ttsEnabled) {
                LOGERR("TTS is disabled, can't speak");
                status = TTS::TTS_NOT_ENABLED;
                logResponse(status, response);
                returnResponse(success);
            }

            if(!sessionInfo->m_gotResource) {
                LOGERR("Session is not active, can't speak");
                status = TTS::TTS_SESSION_NOT_ACTIVE;
                logResponse(status, response);
                returnResponse(success);
            }

            status = sessionInfo->m_session->speak(id, text, secure);

            if(status == TTS::TTS_OK)
                success = true;

            logResponse(status, response);
            returnResponse(success);
        }

        uint32_t TextToSpeech::abort(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            bool success = false;
            bool clearPending = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            SessionInfo *sessionInfo;
            std::map<uint32_t, SessionInfo*>::iterator sessionItr;

            uint32_t sessionId = parameters["sessionId"].Number();
            clearPending = parameters["clearPending"].Boolean();

            CHECK_SESSION_RETURN_ON_FAIL(sessionId, sessionItr, sessionInfo, success);

            if(!m_ttsEnabled) {
                LOGERR("TTS is disabled, nothing to pause");
                status = TTS::TTS_NOT_ENABLED;
                logResponse(status, response);
                returnResponse(success);
            }

            if(clearPending)
                status = sessionInfo->m_session->abortAndClearPending();
            else
                status = sessionInfo->m_session->shut();
            
            success = true;
            logResponse(status, response);

            returnResponse(success);
        }

        uint32_t TextToSpeech::pause(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            bool success = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            SessionInfo *sessionInfo;
            std::map<uint32_t, SessionInfo*>::iterator sessionItr;

            uint32_t sessionId = parameters["sessionId"].Number();
            CHECK_SESSION_RETURN_ON_FAIL(sessionId, sessionItr, sessionInfo, success);

            uint32_t speechId = parameters["speechId"].Number();

            if(!m_ttsEnabled) {
                LOGERR("TTS is disabled, nothing to pause");
                status = TTS::TTS_NOT_ENABLED;
                logResponse(status, response);
                returnResponse(success);
            }

            status = sessionInfo->m_session->pause(speechId);
            success = true;
            logResponse(status, response);

            returnResponse(success);
        }

        uint32_t TextToSpeech::resume(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            bool success = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            SessionInfo *sessionInfo;
            std::map<uint32_t, SessionInfo*>::iterator sessionItr;

            uint32_t sessionId = parameters["sessionId"].Number();
            CHECK_SESSION_RETURN_ON_FAIL(sessionId, sessionItr, sessionInfo, success);

            uint32_t speechId = parameters["speechId"].Number();

            if(!m_ttsEnabled) {
                LOGERR("TTS is disabled, nothing to resume");
                status = TTS::TTS_NOT_ENABLED;
                logResponse(status, response);
                returnResponse(success);
            }

            status = sessionInfo->m_session->resume(speechId);
            success = true;
            logResponse(status, response);
            
            returnResponse(success);
        }

        uint32_t TextToSpeech::isSpeaking(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            bool success = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            SessionInfo *sessionInfo;
            std::map<uint32_t, SessionInfo*>::iterator sessionItr;
            bool speaking = false;

            uint32_t sessionId = parameters["sessionId"].Number();
         
            CHECK_SESSION_RETURN_ON_FAIL(sessionId, sessionItr, sessionInfo, success);

            if(sessionInfo->m_gotResource) {
                sessionInfo->m_session->isSpeaking(speaking);
                response["speaking"] = speaking;
                status = TTS::TTS_OK;
                success = true;
            }

            logResponse(status, response);
            returnResponse(true);
        }

        uint32_t TextToSpeech::getSpeechState(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            bool success = false;
            TTS::TTS_Error status = TTS::TTS_FAIL;

            CHECK_CONNECTION_RETURN_ON_FAIL(success);

            SessionInfo *sessionInfo;
            std::map<uint32_t, SessionInfo*>::iterator sessionItr;
            TTS::SpeechState state;

            uint32_t sessionId = parameters["sessionId"].Number();
            CHECK_SESSION_RETURN_ON_FAIL(sessionId, sessionItr, sessionInfo, success);

            uint32_t speechId = parameters["speechId"].Number();


            status = sessionInfo->m_session->getSpeechState(speechId, state);
            if(status == TTS::TTS_OK)
            {
                response["speechState"] = (int) state;
                success = true;
            }

            logResponse(status, response);
            returnResponse(success);
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

         TTS::ResourceAllocationPolicy TextToSpeech::getResourceAllocationPolicy()
         {
             TTS::ResourceAllocationPolicy policy = TTS::INVALID_POLICY;

             if (m_ttsManager)
             {
                 m_ttsManager->getResourceAllocationPolicy(policy);
             }
             return policy;
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
         
         void TextToSpeech::ResourceAcquired(uint32_t sessionId)
         {
            SessionInfo *sessionInfo;
            std::map<uint32_t, SessionInfo*>::iterator sessionItr;

            do {
                sessionItr = m_sessionMap.find(sessionId);
                if(sessionItr == m_sessionMap.end()) {
                    LOGERR("TTS Session is not created");
                    return;
                }
                sessionInfo = sessionItr->second;
            } while(0);

            sessionInfo->m_gotResource = true;
         }

    } // namespace Plugin
} // namespace WPEFramework
