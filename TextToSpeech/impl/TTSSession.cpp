/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
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
*/

#include "TTSSession.h"
#include "TTSCommon.h"
#include "logger.h"

#include <sstream>

namespace TTS {

// --- //

#define CHECK_ACTIVENESS() do {\
    if(!m_speaker) { \
        TTSLOG_ERROR("Session \"%u\" is not active to start a speech", m_sessionId); \
        return TTS_SESSION_NOT_ACTIVE; \
    } } while(0)

// --- //


TTSSession::TTSSession(uint32_t appId, std::string appName, uint32_t sessionId, TTSConfiguration configuration, TTSSessionCallback *eventCallbacks) :
    m_speaker(NULL), m_havingConfigToUpdate(false), m_extendedEvents(0) {
    m_appId = appId;
    m_name = appName;
    m_sessionId = sessionId;
    m_configuration = configuration;
    m_callback = eventCallbacks;
}

TTSSession::~TTSSession() {
}

TTS_Error TTSSession::setPreemptiveSpeak(bool preemptive) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_configuration.setPreemptiveSpeak(preemptive);
    TTSLOG_INFO("Preemptive Speech has been %s", preemptive ? "enabled" : "disabled");
    return TTS_OK;
}

TTS_Error TTSSession::getSpeechState(uint32_t id, SpeechState &state) {
    TTSLOG_TRACE("Speak");

    // Check if it is active session
    CHECK_ACTIVENESS();

    state =  m_speaker->getSpeechState(this, id);
    return TTS_OK;
}

TTS_Error TTSSession::speak(uint32_t id, std::string text, bool secure) {
    TTSLOG_TRACE("Speak");

    // Check if it is active session
    CHECK_ACTIVENESS();

    if(!m_configuration.isValid()) {
        TTSLOG_ERROR("Configuration is not set, can't speak");
        return TTS_INVALID_CONFIGURATION;
    }

    m_speaker->speak(this, id, text, secure);

    return TTS_OK;
}

TTS_Error TTSSession::pause(uint32_t id) {
    TTSLOG_TRACE("Pause");

    // Check if it is active session
    CHECK_ACTIVENESS();

    if(m_speaker->isSpeaking(this)) {
        m_speaker->pause(id);
    }

    return TTS_OK;
}

TTS_Error TTSSession::resume(uint32_t id) {
    TTSLOG_TRACE("Resume");

    // Check if it is active session
    CHECK_ACTIVENESS();

    if(m_speaker->isSpeaking(this)) {
        m_speaker->resume(id);
    }

    return TTS_OK;
}

TTS_Error TTSSession::shut() {
    TTSLOG_TRACE("Shut");

    // Check if it is active session
    CHECK_ACTIVENESS();

    if(m_speaker->isSpeaking(this)) {
        m_speaker->cancelCurrentSpeech();
    }

    return TTS_OK;
}

TTS_Error TTSSession::abortAndClearPending() {
    TTSLOG_INFO("Clearing all speeches from session");
    if(m_speaker) {
        std::vector<uint32_t> speechesCancelled;
        m_speaker->clearAllSpeechesFrom(this, speechesCancelled);
        cancelled(speechesCancelled);
    }

    return TTS_OK;
}

TTS_Error TTSSession::requestExtendedEvents(uint32_t eventflags) {
    m_extendedEvents = eventflags;
    return TTS_OK;
}

void  TTSSession::isActive(bool &active) const {
    active = (m_speaker != NULL);
}

void TTSSession::isSpeaking(bool &speaking) const {
    speaking = (m_speaker && m_speaker->isSpeaking(this));
}

void TTSSession::sessionID(uint32_t &sessionid) const {
    sessionid = sessionId();
}

//TTS-ThunderPlugin TODO ***
/*TTS_ERROR TTSSession::getConfiguration(rtObjectRef &configuration) {
    TTSLOG_TRACE("Getting configuration");

    configuration.set("ttsEndPoint", m_configuration.endPoint());
    configuration.set("ttsEndPointSecured", m_configuration.secureEndPoint());
    configuration.set("language", m_configuration.language());
    configuration.set("volume", m_configuration.volume());
    configuration.set("voice", m_configuration.voice());
    configuration.set("rate", m_configuration.rate());

    return TTS_OK;
}*/

void TTSSession::setConfiguration(TTSConfiguration &config) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if(m_speaker && m_speaker->isSpeaking(this)) {
        TTSLOG_WARNING("Session \"%u\" is speaking now, will update configuration once speaking is done", m_sessionId);
        m_tmpConfiguration = config;
        TTSLOG_INFO("tmpConfiguration, endPoint=%s, secureEndPoint=%s, lang=%s, voice=%s, vol=%lf, rate=%u",
                m_tmpConfiguration.endPoint().c_str(),
                m_tmpConfiguration.secureEndPoint().c_str(),
                m_tmpConfiguration.language().c_str(),
                m_tmpConfiguration.voice().c_str(),
                m_tmpConfiguration.volume(),
                m_tmpConfiguration.rate());
        m_havingConfigToUpdate = true;
    } else {
        m_configuration.updateWith(config);
        TTSLOG_INFO("configuration, endPoint=%s, secureEndPoint=%s, lang=%s, voice=%s, vol=%lf, rate=%u",
                m_configuration.endPoint().c_str(),
                m_configuration.secureEndPoint().c_str(),
                m_configuration.language().c_str(),
                m_configuration.voice().c_str(),
                m_configuration.volume(),
                m_configuration.rate());
    }
}

void TTSSession::setActive(TTSSpeaker *speaker, bool notifyClient) {
    TTSLOG_TRACE("Activating Session");

    if(m_speaker) {
        TTSLOG_ERROR("Session \"%u\" is already active", sessionId());
        return;
    }

    m_speaker = speaker;

    if(notifyClient) {
        m_callback->onResourceAcquired(appId(), sessionId());
    }
}

void TTSSession::setInactive(bool notifyClient) {
    TTSLOG_TRACE("Deactivating Session");

    // If active session, reset speaker
    if(m_speaker) {
        abortAndClearPending();
        m_speaker = NULL;

        if(notifyClient) {
            m_callback->onResourceReleased(appId(), sessionId());
        }
    }
}

TTSConfiguration *TTSSession::configuration() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return &m_configuration;
}

void TTSSession::willSpeak(uint32_t speech_id, std::string text) {
    if(!(m_extendedEvents & EXT_EVENT_WILL_SPEAK))
        return;

    TTSLOG_VERBOSE(" [%d, %s]", speech_id, text.c_str());

    SpeechData d;
    d.id = speech_id;
    d.text = text.c_str();
    m_callback->onWillSpeak(appId(), sessionId(), d);
}

void TTSSession::started(uint32_t speech_id, std::string text) {
    TTSLOG_WARNING(" [%d, %s]", speech_id, text.c_str());

    SpeechData d;
    d.id = speech_id;
    d.text = text.c_str();
    m_callback->onSpeechStart(appId(), sessionId(), d);
}

void TTSSession::spoke(uint32_t speech_id, std::string text) {
    TTSLOG_VERBOSE(" [%d, %s]", speech_id, text.c_str());

    if(m_havingConfigToUpdate) {
        m_configuration.updateWith(m_tmpConfiguration);
        m_havingConfigToUpdate = false;
    }

    SpeechData d;
    d.id = speech_id;
    d.text = text.c_str();
    m_callback->onSpeechComplete(appId(), sessionId(), d);
}

void TTSSession::paused(uint32_t speech_id) {
    if(!(m_extendedEvents & EXT_EVENT_PAUSED))
        return;

    TTSLOG_WARNING(" [id=%d]", speech_id);

    m_callback->onSpeechPause(appId(), sessionId(), speech_id);
}

void TTSSession::resumed(uint32_t speech_id) {
    if(!(m_extendedEvents & EXT_EVENT_RESUMED))
        return;

    TTSLOG_WARNING(" [id=%d]", speech_id);

    m_callback->onSpeechResume(appId(), sessionId(), speech_id);
}

void TTSSession::cancelled(std::vector<uint32_t> &speeches) {
    if(!(m_extendedEvents & EXT_EVENT_CANCELLED))
        return;

    if(speeches.size() <= 0)
        return;

    std::stringstream ss;
    for(auto it = speeches.begin(); it != speeches.end(); ++it) {
        if(it != speeches.begin())
            ss << ",";
        ss << *it;
    }
    TTSLOG_WARNING(" [ids=%s]", ss.str().c_str());

    m_callback->onSpeechCancelled(appId(), sessionId(), ss.str().c_str());
}

void TTSSession::interrupted(uint32_t speech_id) {
    if(!(m_extendedEvents & EXT_EVENT_INTERRUPTED))
        return;

    TTSLOG_WARNING(" [id=%d]", speech_id);

    m_callback->onSpeechInterrupted(appId(), sessionId(), speech_id);
}

void TTSSession::networkerror(uint32_t speech_id){
    if(!(m_extendedEvents & EXT_EVENT_NETWORK_ERROR))
        return;

    TTSLOG_WARNING(" [id=%d]", speech_id);

    m_callback->onNetworkError(appId(), sessionId(), speech_id);
}

void TTSSession::playbackerror(uint32_t speech_id){
    if(!(m_extendedEvents & EXT_EVENT_PLAYBACK_ERROR))
        return;

    TTSLOG_WARNING(" [id=%d]", speech_id);

    m_callback->onPlaybackError(appId(), sessionId(), speech_id);
}

} // namespace TTS

