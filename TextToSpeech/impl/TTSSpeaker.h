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

#ifndef _TTS_SPEAKER_H_
#define _TTS_SPEAKER_H_

#include <gst/gst.h>
#include <gst/audio/audio.h>
#include <gst/app/gstappsink.h>

#include <map>
#include <list>
#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>

#include "TTSCommon.h"
#include "TTSConfiguration.h"

#if defined(PLATFORM_AMLOGIC)
#include "audio_if.h"
#elif defined(PLATFORM_REALTEK)
#include "RtkHALMisc.h" 
#endif
// --- //

namespace TTS {

#define DEFAULT_RATE  50
#define DEFAULT_WPM 200
#define MAX_VOLUME 100

//Local Endpoint
#define LOOPBACK_ENDPOINT "http://127.0.0.1:50050/"
#define LOCALHOST_ENDPOINT "http://localhost:50050/"

// --- //
  
class TTSSpeakerClient {
public:
    virtual TTSConfiguration* configuration() = 0;
    virtual void willSpeak(uint32_t speech_id, std::string text) = 0;
    virtual void started(uint32_t speech_id, std::string text) = 0;
    virtual void spoke(uint32_t speech_id, std::string text) = 0;
    virtual void paused(uint32_t speech_id) = 0;
    virtual void resumed(uint32_t speech_id) = 0;
    virtual void cancelled(std::vector<uint32_t> &speeches) = 0;
    virtual void interrupted(uint32_t speech_id) = 0;
    virtual void networkerror(uint32_t speech_id) = 0;
    virtual void playbackerror(uint32_t speech_id) = 0;
};

struct SpeechData {
    public:
        SpeechData() : client(NULL), secure(false), id(0), text() {}
        SpeechData(TTSSpeakerClient *c, uint32_t i, std::string t, bool s=false) : client(c), secure(s), id(i), text(t) {}
        SpeechData(const SpeechData &n) {
            client = n.client;
            id = n.id;
            text = n.text;
            secure = n.secure;
        }
        ~SpeechData() {}

        TTSSpeakerClient *client;
        bool secure;
        uint32_t id;
        std::string text;
};

class TTSSpeaker {
public:
    TTSSpeaker(TTSConfiguration &config);
    ~TTSSpeaker();

    void ensurePipeline(bool flag=true);

    // Speak Functions
    int speak(TTSSpeakerClient* client, uint32_t id, std::string text, bool secure); // Formalize data to speak API
    bool isSpeaking(uint32_t id);
    SpeechState getSpeechState(uint32_t id);
    bool cancelSpeech(uint32_t id=0);
    bool reset();

    bool pause(uint32_t id = 0);
    bool resume(uint32_t id = 0);

private:

    // Private Data
    TTSConfiguration &m_defaultConfig;
    TTSSpeakerClient *m_clientSpeaking;
    SpeechData *m_currentSpeech;
    bool m_isSpeaking;
    bool m_isPaused;

    std::mutex m_stateMutex;
    std::condition_variable m_condition;

    std::list<SpeechData> m_queue;
    std::mutex m_queueMutex;
    void queueData(SpeechData);
    void flushQueue();
    SpeechData dequeueData();

    // Private functions
    inline void setSpeakingState(bool state, TTSSpeakerClient *client=NULL);

    // GStreamer Releated members
    GstElement  *m_pipeline;
    GstElement  *m_source;
    GstElement  *m_audioSink;
    GstElement  *m_audioVolume;
    GMainLoop   *m_main_loop;
    GMainContext *m_main_context;
    GThread     *m_main_loop_thread;
    bool        m_pipelineError;
    bool        m_networkError;
    bool        m_runThread;
    bool        m_busThread;
    bool        m_flushed;
    bool        m_isEOS;
    bool        m_pcmAudioEnabled;
#if defined(PLATFORM_AMLOGIC)
    audio_hw_device_t *m_audio_dev;
#endif
    enum MixGain {
        MIXGAIN_PRIM,
        MIXGAIN_SYS, //direct-mode=false
        MIXGAIN_TTS //tts=mode=true
    };
    bool        m_ensurePipeline;
    std::thread *m_gstThread;
    guint       m_busWatch;
    gint64      m_duration;
    uint8_t     m_pipelineConstructionFailures;
    const uint8_t     m_maxPipelineConstructionFailures;

#if defined(PLATFORM_AMLOGIC)
    bool loadInitAudioDev();
#endif
    void setMixGain(MixGain gain, int val);
    static void GStreamerThreadFunc(void *ctx);
    void createPipeline();
    void resetPipeline();
    void destroyPipeline();

    // GStreamer Helper functions
    bool needsPipelineUpdate();
    std::string constructURL(TTSConfiguration &config, SpeechData &d);
    void speakText(TTSConfiguration config, SpeechData &data);
    bool waitForStatus(GstState expected_state, uint32_t timeout_ms);
    void waitForAudioToFinishTimeout(float timeout_s);
    bool handleMessage(GstMessage*);
    static int GstBusCallback(GstBus *bus, GstMessage *message, gpointer data);
    static void event_loop(void *data);
};

} // namespace TTS

#endif
