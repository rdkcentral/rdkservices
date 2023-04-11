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

namespace {
    std::string convertDialectToLanguage(const std::string &dialect) {
        std::map<std::string, std::string> dialectToLanguage = {
            {"eng-GBR", "en-GB"},
            {"nld-NLD", "nl-NL"},
            {"fra-FRA", "fr-FR"},
            {"deu-DEU", "de-DE"},
            {"ita-ITA", "it-IT"},
        };
        auto it = dialectToLanguage.find(dialect);
        if(it != dialectToLanguage.end()) {
            return it->second;
        }

        TTSLOG_ERROR("%s no mapping for %s!!", __func__, dialect.c_str());
        return "";
    }
}

namespace TTS {

std::map<std::string, std::string> TTSConfiguration::m_others;

TTSManager* TTSManager::create(TTSEventCallback *eventCallback)
{
    TTSLOG_TRACE("TTSManager::create");
    return new TTSManager(eventCallback);
}

TTSManager::TTSManager(TTSEventCallback *callback) :
    m_callback(callback),
    m_proxy(*this)
{
    TTSLOG_TRACE("TTSManager::TTSManager");
}

TTSManager::~TTSManager() {
    TTSLOG_TRACE("TTSManager::~TTSManager");
    m_callback = NULL;

}

TTS_Error TTSManager::enableTTS(bool enable) {
    (void) enable;
    TTSLOG_ERROR("%s is not supported", __func__);
    return TTS_FAIL;
}

bool TTSManager::isTTSEnabled() {
    return m_proxy.isTtsEnabled();
}

TTS_Error TTSManager::listVoices(std::string language, std::vector<std::string> &voices) {
    TTSLOG_ERROR("%s is not supported", __func__);
    return TTS_FAIL;
}

TTS_Error TTSManager::setConfiguration(Configuration &configuration) {
    TTSLOG_ERROR("%s is not supported", __func__);
    return TTS_FAIL;
}

TTS_Error TTSManager::getConfiguration(Configuration &configuration) {
    std::string dialect;
    m_proxy.getLanguageConfiguration(dialect);
    configuration.language = convertDialectToLanguage(dialect);
    if(configuration.language.empty()) {
        configuration.language = dialect;
    }
    return TTS_OK;
}

TTS_Error TTSManager::speak(int& speechId, std::string text) {
    ttsproxy::SpeechResult res{};
    const bool force = true;
    bool ret = m_proxy.speak(text, force, res);
    if(!ret || !res.result)
        return TTS_FAIL;
    speechId = res.speechId;
    return TTS_OK;
}

TTS_Error TTSManager::pause(uint32_t id) {
    
    if(!m_proxy.pause(id))
        return TTS_FAIL;
    return TTS_OK;
}

TTS_Error TTSManager::resume(uint32_t id) {
    if(!m_proxy.resume(id))
        return TTS_FAIL;
    return TTS_OK;

}

TTS_Error TTSManager::shut(uint32_t id) {
    if(!m_proxy.cancel(id))
        return TTS_FAIL;
    return TTS_OK;

}

TTS_Error TTSManager::isSpeaking(uint32_t id, bool &speaking) {
    if(!m_proxy.isSpeaking(id, speaking))
        return TTS_FAIL;
    return TTS_OK;

}

TTS_Error TTSManager::getSpeechState(uint32_t id, SpeechState &state) {
    if(!m_proxy.getSpeechState(id, state))
        return TTS_FAIL;
    return TTS_OK;
}

void TTSManager::onTtsStateChanged(const bool state) 
{
    m_callback->onTTSStateChanged(state);
}

void TTSManager::onSpeechStart(const unsigned speechId, const std::string& text)  
{
    SpeechData data{speechId, text};
    m_callback->onSpeechStart(data);
}

void TTSManager::onSpeechComplete(const unsigned speechId, const std::string& text)  
{
    SpeechData data{speechId, text};
    m_callback->onSpeechComplete(data);
}

void TTSManager::onSpeechInterrupted(const unsigned speechId)  
{
    m_callback->onSpeechInterrupted(speechId);
}

void TTSManager::onSpeechPause(const unsigned speechId)  
{
    m_callback->onSpeechPause(speechId);
}

void TTSManager::onSpeechResume(const unsigned speechId)  
{
    m_callback->onSpeechResume(speechId);
}

void TTSManager::onNetworkError(const unsigned speechId)
{
    m_callback->onNetworkError(speechId);
}

void TTSManager::onPlaybackError(const unsigned speechId)  
{
    m_callback->onPlaybackError(speechId);
}

} // namespace TTS
