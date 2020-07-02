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
#include "TTSCommon.h"
#include "TTSSession.h"
#include "utils.h"

#define CHECK_CONNECTION_RETURN_ON_FAIL(ret) do {\
    if(!m_ttsManager) { \
        LOGERR("TTS manager is not establised"); \
        logResponse(TTS::TTS_NOT_ENABLED,response); \
        returnResponse(ret); \
    } } while(0)

#define CHECK_SESSION_RETURN_ON_FAIL(id, sessionitr, sessioninfo, ret) do { \
    sessionitr = m_sessionMap.find(id); \
    if(sessionitr == m_sessionMap.end()) { \
        LOGERR("TTS Session is not created"); \
        logResponse(TTS::TTS_NO_SESSION_FOUND,response); \
        returnResponse(ret); \
    } \
    sessioninfo = sessionitr->second; \
    } while(0)

#define SET_UNSET_EXTENDED_EVENT(sessionInfo, input_event_list, event_flag, event_name) do { \
    uint32_t event = (uint32_t)(event_flag); \
    if((input_event_list & event) && !(sessionInfo->m_extendedEvents & event)) { \
        LOGINFO("Installing the event \"%s\"", event_name); \
        response["ExtendedEvent"] = event_name; \
        sessionInfo->m_extendedEvents |= event; \
    } else if(!(input_event_list & event) && (sessionInfo->m_extendedEvents & event)) { \
        LOGINFO("UnInstalling the event \"%s\"", event_name); \
        sessionInfo->m_extendedEvents &= ~event; \
    } } while(0)

namespace WPEFramework {
    namespace Plugin {
        struct SessionInfo {
            SessionInfo() :
                m_appId(0),
                m_sessionId(0),
                m_extendedEvents(0),
                m_gotResource(false),
                m_callback(NULL) {}

            ~SessionInfo() {
                m_callback = NULL;
                m_gotResource = 0;
                m_sessionId = 0;
                m_appId = 0;
            }

            uint32_t m_appId;
            uint32_t m_sessionId;
            uint32_t m_extendedEvents;
            std::string m_appName;
            bool m_gotResource;
            TTS::TTSSession *m_session;
            TTS::TTSSessionCallback *m_callback;
        };

static inline const char *policyStr(TTS::ResourceAllocationPolicy policy) {
    switch(policy) {
        case TTS::RESERVATION: return "Reservation";
        case TTS::PRIORITY: return "Priority";
        case TTS::OPEN: return "Open";
        default: return "*Invalid*";
    }
}
    }//namespace Plugin
}//namespace WPEFramework
