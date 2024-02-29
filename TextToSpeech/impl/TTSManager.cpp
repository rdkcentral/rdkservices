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
    m_speaker(NULL){

    TTSLOG_TRACE("TTSManager::TTSManager");

    // Setup Speaker passing the read configuration
    m_speaker = new TTSSpeaker(m_defaultConfiguration);
    m_downloader = NULL;
    m_needsConfigStoreUpdate = false;
}

TTSManager::~TTSManager() {
    TTSLOG_TRACE("TTSManager::~TTSManager");
    TTSEventCallback dummyCallback;
    m_callback = &dummyCallback;

    // Clear Speaker Instance
    if(m_speaker) {
        delete m_speaker;
        m_speaker = NULL;
    }
    // Clear Downloader Instance
    if(m_downloader) {
        delete m_downloader;
        m_downloader = NULL;
    }
}

TTS_Error TTSManager::enableTTS(bool enable) {
    static bool force = true; 
    if(force || m_defaultConfiguration.setEnabled(enable)) {
        if(!m_defaultConfiguration.enabled())
            shut(0);
        TTSLOG_INFO("TTS is %s", m_defaultConfiguration.enabled()? "Enabled" : "Disabled");
        m_defaultConfiguration.updateConfigStore();
        m_callback->onTTSStateChanged(m_defaultConfiguration.enabled());
        m_speaker->ensurePipeline(m_defaultConfiguration.enabled());
        force = false;
    }
    return TTS_OK;
}

void TTSManager::initiateDownload()
{
   TTSLOG_INFO("TTSManager::initiate download\n");
   if(m_downloader == NULL)
   {
      TTSLOG_INFO("New TTS fallback downloader object created\n");
      m_downloader = new TTSDownloader(m_defaultConfiguration);
   }
   m_defaultConfiguration.saveFallbackPath("");
   m_downloader->download(m_defaultConfiguration);
}

bool TTSManager::isTTSEnabled() {
    return m_defaultConfiguration.enabled();
}

TTS_Error TTSManager::listVoices(std::string language, std::vector<std::string> &voices) {
    bool returnCurrentConfiguration = false;
    std::string key = std::string("voice_for_"); // return all voices

    if(language.empty()) {
        returnCurrentConfiguration = true; // return voice for the configured language
        key = m_defaultConfiguration.language();
    } else if(language != "*") {
        key += language; // return voices for only the passed language
        if(m_defaultConfiguration.m_others.find(key) != m_defaultConfiguration.m_others.end())
        {
            voices.push_back(m_defaultConfiguration.m_others[key]);
        }
        return TTS_OK;
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

TTS_Error TTSManager::listLocalVoices(std::string language, std::vector<std::string> &voices) {
     if(language.empty()) {
        voices.push_back(m_defaultConfiguration.localVoice());
     } else {
         const std::string key = std::string("voice_for_local_") + language;
         auto it = m_defaultConfiguration.m_others.find(key);
         if (it != m_defaultConfiguration.m_others.end()) {
             voices.push_back(it->second);
         }
     }
     return TTS_OK;
}

TTS_Error TTSManager::setConfiguration(Configuration &configuration) {
    std::string v = m_defaultConfiguration.voice();
    
    bool endpointUpdated = false;
    endpointUpdated |= m_defaultConfiguration.setEndPoint(configuration.ttsEndPoint);
    endpointUpdated |= m_defaultConfiguration.setSecureEndPoint(configuration.ttsEndPointSecured);

    /* If new endpoint url provided is using localhost always use that endpoint for VG*/
    if( m_defaultConfiguration.hasValidLocalEndpoint() && endpointUpdated ) {
        string url = m_defaultConfiguration.secureEndPoint();
        if((url.rfind(LOOPBACK_ENDPOINT,0) != std::string::npos)  || (url.rfind(LOCALHOST_ENDPOINT,0) != std::string::npos))
            m_defaultConfiguration.setLocalEndPoint("");
    }

    /* Set default voice for the language only when voice is empty*/
    if(!configuration.language.empty() && configuration.voice.empty()) {
        std::vector<std::string> voices;
        listVoices(configuration.language, voices);
        if(voices.empty()) {
            TTSLOG_WARNING("voice is empty and no voices are defined for the specified language ('%s')!!!", configuration.language.c_str());
            return TTS_FAIL;
        }
        else {
            m_needsConfigStoreUpdate |= m_defaultConfiguration.setVoice(voices.front());
            m_needsConfigStoreUpdate |= m_defaultConfiguration.setLanguage(configuration.language);
        }
    }
    else {
        m_needsConfigStoreUpdate |= m_defaultConfiguration.setVoice(configuration.voice);
        m_needsConfigStoreUpdate |= m_defaultConfiguration.setLanguage(configuration.language);
    }

    bool languageUpdated = m_defaultConfiguration.setLanguage(configuration.language);
    m_needsConfigStoreUpdate |= languageUpdated;

    if( m_defaultConfiguration.hasValidLocalEndpoint() && languageUpdated ) {
        std::vector<std::string> localVoices;
        listLocalVoices(m_defaultConfiguration.language(),localVoices);
        if(localVoices.empty()) {
            TTSLOG_WARNING("Local Voice is empty and no voices are defined for the specified language ('%s')!!!", m_defaultConfiguration.language().c_str());
            return TTS_FAIL;
        } else {
            m_defaultConfiguration.setLocalVoice(localVoices.front());
        }
     }

    m_needsConfigStoreUpdate |= m_defaultConfiguration.setVolume(configuration.volume);
    m_needsConfigStoreUpdate |= m_defaultConfiguration.setRate(configuration.rate);
    m_needsConfigStoreUpdate |= m_defaultConfiguration.setSpeechRate(configuration.speechRate);

    TTSLOG_INFO("Default config updated, endPoint=%s, secureEndPoint=%s, lang=%s, voice=%s, vol=%lf, rate=%u ,speechrate=%s",
            m_defaultConfiguration.endPoint().c_str(),
            m_defaultConfiguration.secureEndPoint().c_str(),
            m_defaultConfiguration.language().c_str(),
            m_defaultConfiguration.voice().c_str(),
            m_defaultConfiguration.volume(),
            m_defaultConfiguration.rate(),
            m_defaultConfiguration.speechRate().c_str());

    if(v !=  m_defaultConfiguration.voice())
        m_callback->onVoiceChanged(m_defaultConfiguration.voice());
    //if any of the configuration attribute changes..config store gets updated
    if(m_needsConfigStoreUpdate)
    {
        if(m_defaultConfiguration.isFallbackEnabled())
        {
            initiateDownload();
        }
        m_defaultConfiguration.updateConfigStore();
        m_needsConfigStoreUpdate = false;
    }
    return TTS_OK;
}

TTS_Error TTSManager::setFallbackText(FallbackData &data)
{
    m_needsConfigStoreUpdate |= m_defaultConfiguration.setFallBackText(data);
    return TTS_OK;
}

TTS_Error TTSManager::setPrimaryVolDuck(const uint8_t prim)
{
    m_needsConfigStoreUpdate |= m_defaultConfiguration.setPrimVolDuck(prim);
    return TTS_OK;
}

TTS_Error TTSManager::setAPIKey(string apikey)
{
    if(m_defaultConfiguration.setApiKey(apikey))
    {
       if(m_defaultConfiguration.isFallbackEnabled() && (m_defaultConfiguration.getFallbackPath()).empty())
        {
           m_needsConfigStoreUpdate = true;
        }
    }
    return TTS_OK;
}

TTS_Error TTSManager::setACL(const std::string method,const std::string apps)
{
   setAccessList(method,apps);
   //abort existing speech if any
   if(m_speaker)
        m_speaker->cancelSpeech(0);
   return TTS_OK;
}

bool TTSManager::setAccessList(const string &key,const string &value)
{
    std::map<std::string,std::string>::iterator itr = m_accessControlList.find(key);
    if (itr !=  m_accessControlList.end())
    {
        TTSLOG_INFO("method %s found in accesslist...replacing value with %s\n",key.c_str(),value.c_str());
        itr->second= value;
        return 1;
    }
    else
    {
        TTSLOG_INFO("method %s not found..inserting to accesslist with value %s\n",key.c_str(),value.c_str());
        m_accessControlList.insert(std::make_pair(key,value));
        return 0;
    }
}

bool TTSManager::checkAccess(const string &method,string &callsign)
{
    if(!m_accessControlList.empty())
    {
        std::map<std::string,std::string>::iterator itr = m_accessControlList.find(method);
        if (itr !=  m_accessControlList.end())
        {
            string app_quote =  '\"' + callsign + '\"';//wrap it with double quote
            TTSLOG_INFO("method %s found in accesslist and can be accessed by %s\n",method.c_str(),(itr->second).c_str());
            if((itr->second).find(app_quote) != std::string::npos)
            {
                TTSLOG_INFO("%s app has access to method  %s\n",callsign.c_str(),method.c_str());
                return true;
            }

            TTSLOG_WARNING("%s app does not have access to method  %s\n",callsign.c_str(),method.c_str());
            return false;
        }
        else
        {
            TTSLOG_WARNING("method :%s not found in accesslist\n",method.c_str());
            return false;
        }
    }
    else
    {
        return true;
    }
}

TTS_Error TTSManager::getConfiguration(Configuration &configuration) {
    TTSLOG_TRACE("Getting Default Configuration");

    configuration.ttsEndPoint = m_defaultConfiguration.endPoint();
    configuration.ttsEndPointSecured = m_defaultConfiguration.secureEndPoint();
    configuration.language = m_defaultConfiguration.language();
    configuration.voice = m_defaultConfiguration.voice();
    configuration.speechRate = m_defaultConfiguration.speechRate();
    configuration.volume = m_defaultConfiguration.volume();
    configuration.rate = m_defaultConfiguration.rate();

    return TTS_OK;
}

TTS_Error TTSManager::speak(int speechId, std::string callsign, std::string text) {
    TTSLOG_TRACE("Speak");

    if(!m_defaultConfiguration.isValid()) {
        TTSLOG_ERROR("Configuration is not set, can't speak");
        return TTS_INVALID_CONFIGURATION;
    }

    if(m_speaker) {
        // TODO: Currently 'secure' is set to true. Need to decide about this variable while Resident app integration.
        if(checkAccess("speak", callsign))
        {
            m_speaker->speak(this, speechId , callsign, text, true, m_defaultConfiguration.primVolDuck());
        }
        else
        {
            TTSLOG_WARNING("No Speak access for callsign %s\n",callsign.c_str());
            return TTS_NO_ACCESS;
        }
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

    if(m_speaker)
        m_speaker->cancelSpeech(id);

    return TTS_OK;
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

void TTSManager::willSpeak(uint32_t speech_id, std::string callsign, std::string text) {
    TTSLOG_TRACE(" [%d, %s]", speech_id, text.c_str());

    SpeechData d;
    d.id = speech_id;
    d.callsign = callsign;
    d.text = text;
    m_callback->onWillSpeak(d);
}

void TTSManager::started(uint32_t speech_id, std::string callsign, std::string text) {
    TTSLOG_TRACE(" [%d, %s]", speech_id, text.c_str());

    SpeechData d;
    d.id = speech_id;
    d.callsign = callsign;
    d.text = text;
    m_callback->onSpeechStart(d);
}

void TTSManager::spoke(uint32_t speech_id, std::string callsign, std::string text) {
    TTSLOG_TRACE(" [%d, %s]", speech_id, text.c_str());

    SpeechData d;
    d.id = speech_id;
    d.callsign = callsign;
    d.text = text;
    m_callback->onSpeechComplete(d);
}

void TTSManager::paused(uint32_t speech_id, std::string callsign) {
    TTSLOG_TRACE(" [id=%d]", speech_id);

    m_callback->onSpeechPause(speech_id, callsign);
}

void TTSManager::resumed(uint32_t speech_id, std::string callsign) {
    TTSLOG_WARNING(" [id=%d]", speech_id);

    m_callback->onSpeechResume(speech_id, callsign);
}

void TTSManager::cancelled(std::vector<uint32_t> &speeches, std::string callsign) {
    if(speeches.size() <= 0)
        return;

    m_callback->onSpeechCancelled(speeches, callsign);
}

void TTSManager::interrupted(uint32_t speech_id, std::string callsign) {
    TTSLOG_WARNING(" [id=%d]", speech_id);

    m_callback->onSpeechInterrupted(speech_id, callsign);
}

void TTSManager::networkerror(uint32_t speech_id, std::string callsign){
    TTSLOG_WARNING(" [id=%d]", speech_id);

    m_callback->onNetworkError(speech_id, callsign);
}

void TTSManager::playbackerror(uint32_t speech_id, std::string callsign){
    TTSLOG_WARNING(" [id=%d]", speech_id);

    m_callback->onPlaybackError(speech_id, callsign);
}

} // namespace TTS
