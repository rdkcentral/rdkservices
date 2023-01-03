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
#include "TTSConfiguration.h"
#include <ttsproxy.h>

#include <vector>

namespace TTS {

struct Configuration {
    Configuration() : volume(0), rate(0), primVolDuck(25) {};
    ~Configuration() {}

    std::string ttsEndPoint;
    std::string ttsEndPointSecured;
    std::string language;
    std::string voice;
    std::string apiKey;
    FallbackData data{};
    double volume{};
    uint8_t rate{};
    int8_t primVolDuck{};
};

struct SpeechData {
    public:
        uint32_t id;
        std::string text;
};

class TTSEventCallback {
public:
    TTSEventCallback() = default;
    virtual ~TTSEventCallback() = default;

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

class TTSManager: public ttsproxy::TtsEventListener {
public:
    static TTSManager *create(TTSEventCallback *eventCallback);
    TTSManager(TTSEventCallback *eventCallback);
    virtual ~TTSManager();

    // TTS Global APIs
    TTS_Error enableTTS(bool enable);
    bool isTTSEnabled();
    TTS_Error listVoices(std::string language, std::vector<std::string> &voices);
    TTS_Error setConfiguration(Configuration &configuration);
    TTS_Error getConfiguration(Configuration &configuration);

    //Speak APIs
    TTS_Error speak(int& speechId, std::string text);
    TTS_Error pause(uint32_t id);
    TTS_Error resume(uint32_t id);
    TTS_Error shut(uint32_t id);
    TTS_Error isSpeaking(uint32_t id, bool &speaking);
    TTS_Error getSpeechState(uint32_t id, SpeechState &state);
    TTS_Error clearAudioPipeline() {return TTS_OK;} ;
    virtual TTSConfiguration *configuration() {return &m_defaultConfiguration;}  

private:
    void onTtsStateChanged(const bool state) override;
    void onSpeechStart(const unsigned speechId, const std::string& text) override;
    void onSpeechComplete(const unsigned speechId, const std::string& text) override;
    void onSpeechInterrupted(const unsigned speechId) override;
    void onSpeechPause(const unsigned speechId) override;
    void onSpeechResume(const unsigned speechId) override;
    void onNetworkError(const unsigned speechId) override;
    void onPlaybackError(const unsigned speechId) override;

    TTSEventCallback *m_callback;
    TTSConfiguration m_defaultConfiguration;
    ttsproxy::TtsProxy m_proxy;
};

} // namespace TTS

#endif
