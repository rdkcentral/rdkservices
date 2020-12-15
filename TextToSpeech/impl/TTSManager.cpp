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

#include "TTSManager.h"

namespace TTS {

TTSManager* TTSManager::create(TTSEventCallback *eventCallback)
{
    TTSLOG_TRACE("TTSManager::create");
    return new TTSManager(eventCallback);
}

TTSManager::TTSManager(TTSEventCallback *callback) :
    m_callback(callback),
    m_speaker(NULL),
    m_ttsEnabled(false) {

    TTSLOG_TRACE("TTSManager::TTSManager");

    // Setup Speaker passing the read configuration
    m_speaker = new TTSSpeaker(m_defaultConfiguration);
}

TTSManager::~TTSManager() {
    TTSLOG_TRACE("TTSManager::~TTSManager");
    m_callback = NULL;

    // Clear Speaker Instance
    if(m_speaker) {
        delete m_speaker;
        m_speaker = NULL;
    }
}

TTS_Error TTSManager::enableTTS(bool enable) {

    if(m_ttsEnabled != enable) {
        m_ttsEnabled = enable;
        TTSLOG_INFO("TTS is %s", enable ? "Enabled" : "Disabled");
        m_callback->onTTSStateChanged(m_ttsEnabled);
        m_speaker->ensurePipeline(m_ttsEnabled);
    }

    return TTS_OK;
}

bool TTSManager::isTTSEnabled() {
    return m_ttsEnabled;
}

TTS_Error TTSManager::listVoices(std::string language, std::vector<std::string> &voices) {
    bool returnCurrentConfiguration = false;
    std::string key = std::string("voice_for_"); // return all voices

    if(language.empty()) {
        returnCurrentConfiguration = true; // return voice for the configured language
        key = m_defaultConfiguration.language();
    } else if(language != "*") {
        key += language; // return voices for only the passed language
    }

    if(returnCurrentConfiguration) {
        voices.push_back(m_defaultConfiguration.voice());
    } else {
        auto it = m_defaultConfiguration.m_others.begin();
        while(it != m_defaultConfiguration.m_others.end()) {
            if(it->first.find(key) == 0)
                voices.push_back(it->second);
            ++it;
        }
    }

    return TTS_OK;
}

TTS_Error TTSManager::setConfiguration(Configuration &configuration) {
    TTSLOG_TRACE("Setting Default Configuration");

    std::string v = m_defaultConfiguration.voice();

    m_defaultConfiguration.setEndPoint(configuration.ttsEndPoint);
    m_defaultConfiguration.setSecureEndPoint(configuration.ttsEndPointSecured);
    m_defaultConfiguration.setLanguage(configuration.language);
    m_defaultConfiguration.setVoice(configuration.voice);
    m_defaultConfiguration.setVolume(configuration.volume);
    m_defaultConfiguration.setRate(configuration.rate);

    if(m_defaultConfiguration.endPoint().empty() && !m_defaultConfiguration.secureEndPoint().empty())
        m_defaultConfiguration.setEndPoint(m_defaultConfiguration.secureEndPoint());
    else if(m_defaultConfiguration.secureEndPoint().empty() && !m_defaultConfiguration.endPoint().empty())
        m_defaultConfiguration.setSecureEndPoint(m_defaultConfiguration.endPoint());
    else if(m_defaultConfiguration.endPoint().empty() && m_defaultConfiguration.secureEndPoint().empty())
        TTSLOG_WARNING("TTSEndPoint & SecureTTSEndPoints are empty!!!");

    TTSLOG_INFO("Default config updated, endPoint=%s, secureEndPoint=%s, lang=%s, voice=%s, vol=%lf, rate=%u",
            m_defaultConfiguration.endPoint().c_str(),
            m_defaultConfiguration.secureEndPoint().c_str(),
            m_defaultConfiguration.language().c_str(),
            m_defaultConfiguration.voice().c_str(),
            m_defaultConfiguration.volume(),
            m_defaultConfiguration.rate());

    if(v !=  m_defaultConfiguration.voice())
        m_callback->onVoiceChanged(m_defaultConfiguration.voice());

    return TTS_OK;
}

TTS_Error TTSManager::getConfiguration(Configuration &configuration) {
    TTSLOG_TRACE("Getting Default Configuration");

    configuration.ttsEndPoint = m_defaultConfiguration.endPoint();
    configuration.ttsEndPointSecured = m_defaultConfiguration.secureEndPoint();
    configuration.language = m_defaultConfiguration.language();
    configuration.voice = m_defaultConfiguration.voice();
    configuration.volume = m_defaultConfiguration.volume();
    configuration.rate = m_defaultConfiguration.rate();

    return TTS_OK;
}

TTS_Error TTSManager::speak(int speechId, std::string text) {
    TTSLOG_TRACE("Speak");

    if(!m_defaultConfiguration.isValid()) {
        TTSLOG_ERROR("Configuration is not set, can't speak");
        return TTS_INVALID_CONFIGURATION;
    }

    if(m_speaker) {
        // TODO: Currently 'secure' is set to true. Need to decide about this variable while Resident app integration.
        m_speaker->speak(this, speechId , text, true);
    }

    return TTS_OK;
}

TTS_Error TTSManager::pause(uint32_t id) {
    TTSLOG_TRACE("Pause");

    if(m_speaker && m_speaker->pause(id))
        return TTS_OK;

    return TTS_FAIL;
}

TTS_Error TTSManager::resume(uint32_t id) {
    TTSLOG_TRACE("Resume");

    if(m_speaker && m_speaker->resume(id))
        return TTS_OK;

    return TTS_FAIL;
}

TTS_Error TTSManager::shut(uint32_t id) {
    TTSLOG_TRACE("Shut");

    if((id != 0) && m_speaker && m_speaker->cancelSpeech(id))
        return TTS_OK;

    return TTS_FAIL;
}

TTS_Error TTSManager::isSpeaking(uint32_t id, bool &speaking) {
    speaking = (m_speaker && m_speaker->isSpeaking(id));
    return TTS_OK;
}

TTS_Error TTSManager::getSpeechState(uint32_t id, SpeechState &state) {
    TTSLOG_TRACE("getSpeechState");

    if(m_speaker)
        state =  m_speaker->getSpeechState(id);

    return TTS_OK;
}

void TTSManager::willSpeak(uint32_t speech_id, std::string text) {
    TTSLOG_TRACE(" [%d, %s]", speech_id, text.c_str());

    SpeechData d;
    d.id = speech_id;
    d.text = text;
    m_callback->onWillSpeak(d);
}

void TTSManager::started(uint32_t speech_id, std::string text) {
    TTSLOG_TRACE(" [%d, %s]", speech_id, text.c_str());

    SpeechData d;
    d.id = speech_id;
    d.text = text;
    m_callback->onSpeechStart(d);
}

void TTSManager::spoke(uint32_t speech_id, std::string text) {
    TTSLOG_TRACE(" [%d, %s]", speech_id, text.c_str());

    SpeechData d;
    d.id = speech_id;
    d.text = text;
    m_callback->onSpeechComplete(d);
}

void TTSManager::paused(uint32_t speech_id) {
    TTSLOG_TRACE(" [id=%d]", speech_id);

    m_callback->onSpeechPause(speech_id);
}

void TTSManager::resumed(uint32_t speech_id) {
    TTSLOG_WARNING(" [id=%d]", speech_id);

    m_callback->onSpeechResume(speech_id);
}

void TTSManager::cancelled(std::vector<uint32_t> &speeches) {
    if(speeches.size() <= 0)
        return;

    m_callback->onSpeechCancelled(speeches);
}

void TTSManager::interrupted(uint32_t speech_id) {
    TTSLOG_WARNING(" [id=%d]", speech_id);

    m_callback->onSpeechInterrupted(speech_id);
}

void TTSManager::networkerror(uint32_t speech_id){
    TTSLOG_WARNING(" [id=%d]", speech_id);

    m_callback->onNetworkError(speech_id);
}

void TTSManager::playbackerror(uint32_t speech_id){
    TTSLOG_WARNING(" [id=%d]", speech_id);

    m_callback->onPlaybackError(speech_id);
}
} // namespace TTS
