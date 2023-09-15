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

#ifndef _TTS_ENGINE_H_
#define _TTS_ENGINE_H_

#include "TTSCommon.h"
#include "TTSSpeaker.h"
#include "TTSConfiguration.h"
#include "TTSDownloader.h"

#include <vector>

namespace TTS {

struct Configuration {
    Configuration() : volume(0), rate(0) {};
    ~Configuration() {}

    std::string ttsEndPoint;
    std::string ttsEndPointSecured;
    std::string language;
    std::string voice;
    std::string speechRate;
    double volume;
    uint8_t rate;
};

class TTSEventCallback {
public:
    TTSEventCallback() {}
    virtual ~TTSEventCallback() {}

    virtual void onTTSStateChanged(bool enabled) { (void)enabled; }
    virtual void onVoiceChanged(std::string voice) { (void)voice; }
    virtual void onWillSpeak(SpeechData &data) { (void)data; }
    virtual void onSpeechStart(SpeechData &data) { (void)data; }
    virtual void onSpeechPause(uint32_t speechId) { (void)speechId; }
    virtual void onSpeechResume(uint32_t speechId) { (void)speechId; }
    virtual void onSpeechCancelled(std::vector<uint32_t> speechIds) { (void)speechIds; }
    virtual void onSpeechInterrupted(uint32_t speechId) { (void)speechId; }
    virtual void onNetworkError(uint32_t speechId) { (void)speechId; }
    virtual void onPlaybackError(uint32_t speechId) { (void)speechId; }
    virtual void onSpeechComplete(SpeechData &data) { (void)data; }
};

class TTSManager : public TTSSpeakerClient{
public:
    static TTSManager *create(TTSEventCallback *eventCallback);
    TTSManager(TTSEventCallback *eventCallback);
    virtual ~TTSManager();

    // TTS Global APIs
    TTS_Error enableTTS(bool enable);
    bool isTTSEnabled();
    void initiateDownload();
    TTS_Error listVoices(std::string language, std::vector<std::string> &voices);
    TTS_Error setConfiguration(Configuration &configuration);
    TTS_Error getConfiguration(Configuration &configuration);
    TTS_Error setFallbackText(FallbackData &data);
    TTS_Error setPrimaryVolDuck(const uint8_t prim);
    TTS_Error setAPIKey(string apikey);

    //Speak APIs
    TTS_Error speak(int speechId, std::string text);
    TTS_Error pause(uint32_t id);
    TTS_Error resume(uint32_t id);
    TTS_Error shut(uint32_t id);
    TTS_Error isSpeaking(uint32_t id, bool &speaking);
    TTS_Error getSpeechState(uint32_t id, SpeechState &state);
    TTS_Error clearAudioPipeline();

    virtual TTSConfiguration *configuration() {return &m_defaultConfiguration;}

    //Speak Events
    virtual void willSpeak(uint32_t speech_id, std::string text);
    virtual void started(uint32_t speech_id, std::string text);
    virtual void spoke(uint32_t speech_id, std::string text);
    virtual void paused(uint32_t speech_id);
    virtual void resumed(uint32_t speech_id);
    virtual void cancelled(std::vector<uint32_t> &speeches);
    virtual void interrupted(uint32_t speech_id);
    virtual void networkerror(uint32_t speech_id);
    virtual void playbackerror(uint32_t speech_id);

private:
    TTSConfiguration m_defaultConfiguration;
    TTSEventCallback *m_callback;
    TTSSpeaker *m_speaker;
    bool m_needsConfigStoreUpdate;
    TTSDownloader *m_downloader;
};

} // namespace TTS

#endif
