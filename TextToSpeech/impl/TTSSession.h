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
#ifndef _TTS_SESSION_H_
#define _TTS_SESSION_H_

#include "TTSSpeaker.h"
#include "TTSCommon.h"

#include <mutex>

namespace TTS {

class TTSSessionCallback {
public:
    TTSSessionCallback() {}
    virtual ~TTSSessionCallback() {}
    
    virtual void onTTSSessionCreated(uint32_t appId, uint32_t sessionId) = 0;
    virtual void onResourceAcquired(uint32_t appId, uint32_t sessionId) = 0;
    virtual void onResourceReleased(uint32_t appId, uint32_t sessionId) = 0;
    virtual void onWillSpeak(uint32_t appId, uint32_t sessionId, SpeechData &data) { (void)appId; (void)sessionId; (void)data; }
    virtual void onSpeechStart(uint32_t appId, uint32_t sessionId, SpeechData &data) { (void)appId; (void)sessionId; (void)data; }
    virtual void onSpeechPause(uint32_t appId, uint32_t sessionId, uint32_t speechId) { (void)appId; (void)sessionId; (void)speechId; }
    virtual void onSpeechResume(uint32_t appId, uint32_t sessionId, uint32_t speechId) { (void)appId; (void)sessionId; (void)speechId; }
    virtual void onSpeechCancelled(uint32_t appId, uint32_t sessionId, std::string id) { (void)appId; (void)sessionId; (void)id; }
    virtual void onSpeechInterrupted(uint32_t appId, uint32_t sessionId, uint32_t speechId) { (void)appId; (void)sessionId; (void)speechId; }
    virtual void onNetworkError(uint32_t appId, uint32_t sessionId, uint32_t speechId) { (void)appId; (void)sessionId; (void)speechId; }
    virtual void onPlaybackError(uint32_t appId, uint32_t sessionId, uint32_t speechId) { (void)appId; (void)sessionId; (void)speechId; }
    virtual void onSpeechComplete(uint32_t appId, uint32_t sessionId, SpeechData &data) { (void)appId; (void)sessionId; (void)data; }
};

class TTSSession : public TTSSpeakerClient {
public:
    TTSSession(uint32_t appId, std::string appName, uint32_t sessionId, TTSConfiguration configuration, TTSSessionCallback *eventCallbacks);
    virtual ~TTSSession();

    //TTS_Error getConfiguration(rtObjectRef &configuration); //TTS-ThunderPlugin TODO ***
    TTS_Error setPreemptiveSpeak(bool preemptive);
    TTS_Error getSpeechState(uint32_t id, SpeechState &state);
    TTS_Error speak(uint32_t id, std::string text, bool secure);
    TTS_Error pause(uint32_t id);
    TTS_Error resume(uint32_t id);
    TTS_Error shut();
    TTS_Error abortAndClearPending();
    TTS_Error requestExtendedEvents(uint32_t eventflags);

    void isActive(bool &active) const;
    void sessionID(uint32_t &sessionid) const;
    void isSpeaking(bool &speaking) const;

    void setConfiguration(TTSConfiguration &config);
    void setActive(TTSSpeaker *speaker, bool notifyClient=true);
    void setInactive(bool notifyClient=true);

    uint32_t appId() const { return m_appId; }
    std::string appName() const { return m_name; }
    uint32_t sessionId() const { return m_sessionId; }

protected:
    // Speaker Client Callbacks
    virtual TTSConfiguration *configuration();
    virtual void willSpeak(uint32_t speech_id, std::string text);
    virtual void started(uint32_t speech_id, std::string text);
    virtual void spoke(uint32_t speech_id, std::string text);
    virtual void paused(uint32_t speech_id);
    virtual void resumed(uint32_t speech_id);
    virtual void cancelled(std::vector<uint32_t> &speeches);
    virtual void interrupted(uint32_t speech_id);
    virtual void networkerror(uint32_t speech_id);
    virtual void playbackerror(uint32_t speech_id);

    TTSSpeaker *m_speaker;
    std::mutex m_mutex;

private:
    TTSConfiguration m_tmpConfiguration;
    TTSConfiguration m_configuration;
    bool m_havingConfigToUpdate;
    TTSSessionCallback *m_callback;

    std::string m_name;
    uint32_t m_appId;
    uint32_t m_sessionId;
    uint32_t m_extendedEvents;
};

} // namespace TTS

#endif
