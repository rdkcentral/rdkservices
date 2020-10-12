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

#include "TTSSpeaker.h"
#include "logger.h"

#include <curl/curl.h>
#include <unistd.h>
#include <regex>

#define INT_FROM_ENV(env, default_value) ((getenv(env) ? atoi(getenv(env)) : 0) > 0 ? atoi(getenv(env)) : default_value)

namespace TTS {

std::map<std::string, std::string> TTSConfiguration::m_others;

TTSConfiguration::TTSConfiguration() :
    m_ttsEndPoint(""),
    m_ttsEndPointSecured(""),
    m_language("en-US"),
    m_voice(""),
    m_volume(MAX_VOLUME),
    m_rate(DEFAULT_RATE),
    m_preemptiveSpeaking(true) { }

TTSConfiguration::~TTSConfiguration() {}

void TTSConfiguration::setEndPoint(const std::string endpoint) {
    if(!endpoint.empty())
        m_ttsEndPoint = endpoint;
    else
        TTSLOG_WARNING("Invalid TTSEndPoint input \"%s\"", endpoint.c_str());
}

void TTSConfiguration::setSecureEndPoint(const std::string endpoint) {
    if(!endpoint.empty())
        m_ttsEndPointSecured = endpoint;
    else
        TTSLOG_WARNING("Invalid Secured TTSEndPoint input \"%s\"", endpoint.c_str());
}

void TTSConfiguration::setLanguage(const std::string language) {
    if(!language.empty())
        m_language = language;
    else
        TTSLOG_WARNING("Empty Language input");
}

void TTSConfiguration::setVoice(const std::string voice) {
    if(!voice.empty())
        m_voice = voice;
    else
        TTSLOG_WARNING("Empty Voice input");
}

void TTSConfiguration::setVolume(const double volume) {
    if(volume >= 1 && volume <= 100)
        m_volume = volume;
    else
        TTSLOG_WARNING("Invalid Volume input \"%lf\"", volume);
}

void TTSConfiguration::setRate(const uint8_t rate) {
    if(rate >= 1 && rate <= 100)
        m_rate = rate;
    else
        TTSLOG_WARNING("Invalid Rate input \"%u\"", rate);
}

void TTSConfiguration::setPreemptiveSpeak(const bool preemptive) {
    m_preemptiveSpeaking = preemptive;
}

const std::string TTSConfiguration::voice() {
    std::string str;

    if(!m_voice.empty())
        return m_voice;
    else {
        std::string key = std::string("voice_for_") + m_language.c_str();
        auto it = m_others.find(key);
        if(it != m_others.end())
            str = it->second.c_str();
        return str;
    }
}

void TTSConfiguration::updateWith(TTSConfiguration &nConfig) {
    setEndPoint(nConfig.m_ttsEndPoint);
    setSecureEndPoint(nConfig.m_ttsEndPointSecured);
    setLanguage(nConfig.m_language);
    setVoice(nConfig.m_voice);
    setVolume(nConfig.m_volume);
    setRate(nConfig.m_rate);
}

bool TTSConfiguration::isValid() {
    if((m_ttsEndPoint.empty() && m_ttsEndPointSecured.empty())) {
        TTSLOG_ERROR("TTSEndPointEmpty=%d, TTSSecuredEndPointEmpty=%d",
                m_ttsEndPoint.empty(), m_ttsEndPointSecured.empty());
        return false;
    }
    return true;
}

// --- //

TTSSpeaker::TTSSpeaker(TTSConfiguration &config) :
    m_defaultConfig(config),
    m_clientSpeaking(NULL),
    m_currentSpeech(NULL),
    m_isSpeaking(false),
    m_isPaused(false),
    m_pipeline(NULL),
    m_source(NULL),
    m_audioSink(NULL),
    main_loop(NULL),
    main_context(NULL),
    main_loop_thread(NULL),
    m_pipelineError(false),
    m_networkError(false),
    m_runThread(true),
    m_busThread(true),
    m_flushed(false),
    m_isEOS(false),
    m_pcmAudioEnabled(false),
    m_ensurePipeline(false),
    m_gstThread(new std::thread(GStreamerThreadFunc, this)),
    m_busWatch(0),
    m_duration(0),
    m_pipelineConstructionFailures(0),
    m_maxPipelineConstructionFailures(INT_FROM_ENV("MAX_PIPELINE_FAILURE_THRESHOLD", 1)) {
        setenv("GST_DEBUG", "2", 0);
        if (!gst_is_initialized())
            gst_init(NULL,NULL);
        this->main_loop_thread = g_thread_new("BusWatch", (void* (*)(void*)) event_loop, this);
}

TTSSpeaker::~TTSSpeaker() {
    if(m_isSpeaking)
        m_flushed = true;
    m_runThread = false;
    m_busThread = false;
    m_pcmAudioEnabled = false;
    m_condition.notify_one();

    if(m_gstThread) {
        m_gstThread->join();
        m_gstThread = NULL;
    }

    if(g_main_loop_is_running(this->main_loop))
        g_main_loop_quit(this->main_loop);
    g_thread_join(this->main_loop_thread);
}

void TTSSpeaker::ensurePipeline(bool flag) {
    std::unique_lock<std::mutex> mlock(m_queueMutex);
    TTSLOG_WARNING("%s", __FUNCTION__);
    m_ensurePipeline = flag;
    m_condition.notify_one();
}

int TTSSpeaker::speak(TTSSpeakerClient *client, uint32_t id, std::string text, bool secure) {
    TTSLOG_TRACE("id=%d, text=\"%s\"", id, text.c_str());

    // If force speak is set, clear old queued data & stop speaking
    if(client->configuration()->isPreemptive())
        reset();

    SpeechData data(client, id, text, secure);
    queueData(data);

    return 0;
}

SpeechState TTSSpeaker::getSpeechState(uint32_t id) {
    // See if the speech is in progress i.e Speaking / Paused
    {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        if(m_currentSpeech && id == m_currentSpeech->id) {
            if(m_isPaused)
                return SPEECH_PAUSED;
            else
                return SPEECH_IN_PROGRESS;
        }
    }

    // Or in queue
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        for(auto it = m_queue.begin(); it != m_queue.end(); ++it) {
            if(it->id == id)
                return SPEECH_PENDING;
        }
    }

    return SPEECH_NOT_FOUND;
}

bool TTSSpeaker::isSpeaking(uint32_t id) {
    std::lock_guard<std::mutex> lock(m_stateMutex);

    if(m_currentSpeech) {
        if(id == m_currentSpeech->id)
            return m_isSpeaking;
    }

    return false;
}

bool TTSSpeaker::cancelSpeech(uint32_t id) {
    TTSLOG_VERBOSE("Cancelling current speech");
    bool status = false;
    if(m_isSpeaking && m_currentSpeech && ((m_currentSpeech->id == id) || (id == 0))) {
        m_isPaused = false;
        m_flushed = true;
        status = true;
        m_condition.notify_one();
    }
    return status;
}

bool TTSSpeaker::reset() {
    TTSLOG_VERBOSE("Resetting Speaker");
    cancelSpeech();
    flushQueue();

    return true;
}

bool TTSSpeaker::pause(uint32_t id) {
    if(!m_isSpeaking || !m_currentSpeech || (id != m_currentSpeech->id))
        return false;

    if(m_pipeline) {
        if(!m_isPaused) {
            m_isPaused = true;
            gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
            TTSLOG_INFO("Set state to PAUSED");
            return true;
        }
    }
    return false;
}

bool TTSSpeaker::resume(uint32_t id) {
    if(!m_isSpeaking || !m_currentSpeech || (id != m_currentSpeech->id))
        return false;

    if(m_pipeline) {
        if(m_isPaused) {
            gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
            TTSLOG_INFO("Set state to PLAYING");
            return true;
        }
    }
    return false;
}

void TTSSpeaker::setSpeakingState(bool state, TTSSpeakerClient *client) {
    std::lock_guard<std::mutex> lock(m_stateMutex);

    m_isSpeaking = state;
    m_clientSpeaking = client;
    
    // If thread just completes speaking (called only from GStreamerThreadFunc),
    // it will take the next text from queue, no need to keep
    // m_flushed (as nothing is being spoken, which needs bail out)
    if(state == false)
        m_flushed = false;
}

void TTSSpeaker::queueData(SpeechData data) {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_queue.push_back(data);
    m_condition.notify_one();
}

void TTSSpeaker::flushQueue() {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_queue.clear();
}

SpeechData TTSSpeaker::dequeueData() {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    SpeechData d;
    d = m_queue.front();
    m_queue.pop_front();
    m_flushed = false;
    return d;
}

bool TTSSpeaker::waitForStatus(GstState expected_state, uint32_t timeout_ms) {
    // wait for the pipeline to get to pause so we know we have the audio device
    if(m_pipeline) {
        GstState state;
        GstState pending;

        auto timeout = std::chrono::system_clock::now() + std::chrono::milliseconds(timeout_ms);

        do {
            std::unique_lock<std::mutex> mlock(m_queueMutex);
            m_condition.wait_until(mlock, timeout, [this, &state, &pending, expected_state] () {
                    // Speaker has flushed the data, no need wait for the completion
                    // must break and reset the pipeline
                    if(m_flushed) {
                        TTSLOG_VERBOSE("Bailing out because of forced text queue (m_flushed=true)");
                        return true;
                    }

                    gst_element_get_state(m_pipeline, &state, &pending, GST_CLOCK_TIME_NONE);
                    if(state == expected_state)
                        return true;

                    return false;
                });
        } while(!m_flushed && state != expected_state && timeout > std::chrono::system_clock::now());

        if(state == expected_state) {
            TTSLOG_VERBOSE("Got Status : expected_state = %d, new_state = %d", expected_state, state);
            return true;
        }

        TTSLOG_WARNING("Timed Out waiting for state %s, currentState %s",
                gst_element_state_get_name(expected_state), gst_element_state_get_name(state));
        return false;
    }

    return true;
}

// GStreamer Releated members
void TTSSpeaker::createPipeline() {
    m_isEOS = false;
    GstCaps *audiocaps=NULL;
    GstElement *capsfilter=NULL;
    m_pcmAudioEnabled = false; //default is cloud

    if(!m_ensurePipeline || m_pipeline) {
        TTSLOG_WARNING("Skipping Pipeline creation");
        return;
    }

    TTSLOG_WARNING("Creating Pipeline...");
    m_pipeline = gst_pipeline_new(NULL);
    if (!m_pipeline) {
        m_pipelineConstructionFailures++;
        TTSLOG_ERROR("Failed to create gstreamer pipeline");
        return;
    }

    m_source = gst_element_factory_make("souphttpsrc", NULL);

    // create soc specific elements
#if defined(PLATFORM_BROADCOM)
    GstElement *decodebin = gst_element_factory_make("brcmmp3decoder", NULL);
    m_audioSink = gst_element_factory_make("brcmpcmsink", NULL);
#elif defined(PLATFORM_AMLOGIC)
    GstElement *parser = gst_element_factory_make("mpegaudioparse", NULL);
    GstElement *decodebin = gst_element_factory_make("avdec_mp3", NULL);
    GstElement *convert = gst_element_factory_make("audioconvert", NULL);
    GstElement *resample = gst_element_factory_make("audioresample", NULL);
    m_audioSink = gst_element_factory_make("amlhalasink", NULL);
#endif

    std::string tts_url =
        !m_defaultConfig.secureEndPoint().empty() ? m_defaultConfig.secureEndPoint().c_str() : m_defaultConfig.endPoint().c_str();
    if(!tts_url.empty()) {
        if(!m_defaultConfig.voice().empty()) {
            tts_url.append("voice=");
            tts_url.append(m_defaultConfig.voice());
        }

        if(!m_defaultConfig.language().empty()) {
            tts_url.append("&language=");
            tts_url.append(m_defaultConfig.language());
        }

        tts_url.append("&text=init");
	std::string nuanceLoopbackAddr = NUANCE_LOOPBACK_ADDR;
	std::string  nuanceLocalhostAddr = NUANCE_LOCALHOST_ADDR;
	//Check if url contains nuance endpoint on localhost, enable PCM audio
	if((tts_url.rfind(nuanceLoopbackAddr,0) != std::string::npos)  || (tts_url.rfind(nuanceLocalhostAddr,0) != std::string::npos)){
	   m_pcmAudioEnabled = true;
	}
        curlSanitize(tts_url);

        g_object_set(G_OBJECT(m_source), "location", tts_url.c_str(), NULL);
    }

    // set the TTS volume to max.
    g_object_set(G_OBJECT(m_audioSink), "volume", (double) (m_defaultConfig.volume() / MAX_VOLUME), NULL);

    // Add elements to pipeline and link
     if(m_pcmAudioEnabled) {
        //Add raw audio caps
       audiocaps = gst_caps_new_simple("audio/x-raw", "format", G_TYPE_STRING, "S16LE", "rate", G_TYPE_INT, 22050,
                                "channels", G_TYPE_INT, 1, NULL);
       if(audiocaps == NULL) {
	   m_pcmAudioEnabled = false;
           TTSLOG_ERROR("Unable to add audio caps for PCM audio.");
           return ;
       }
      capsfilter = gst_element_factory_make ("capsfilter", NULL);
      if (capsfilter) {
         g_object_set (G_OBJECT (capsfilter), "caps", audiocaps, NULL);
         gst_caps_unref(audiocaps);
      }
      else {
          m_pcmAudioEnabled = false;
          TTSLOG_ERROR( "Unable to create capsfilter for PCM audio.");
          return;
      }
     }
    bool result = TRUE;
#if defined(PLATFORM_BROADCOM)
    if(!m_pcmAudioEnabled){
    gst_bin_add_many(GST_BIN(m_pipeline), m_source, decodebin, m_audioSink, NULL);
    result &= gst_element_link (m_source, decodebin);
    result &= gst_element_link (decodebin, m_audioSink);
    }
    else {
        TTSLOG_INFO("PCM audio capsfilter  added to sink");
        gst_bin_add_many(GST_BIN(m_pipeline), m_source, capsfilter, m_audioSink,NULL);
        result = gst_element_link_many (m_source,capsfilter,m_audioSink,NULL);
    }
#elif defined(PLATFORM_AMLOGIC)
    if(!m_pcmAudioEnabled){
    gst_bin_add_many(GST_BIN(m_pipeline), m_source, parser, decodebin, convert, resample, m_audioSink, NULL);
    result &= gst_element_link (m_source, parser);
    result &= gst_element_link (parser, decodebin);
    result &= gst_element_link (decodebin, convert);
    result &= gst_element_link (convert, resample);
    result &= gst_element_link (resample, m_audioSink);
    }
    else {
         //TODO find PCM playback on Amlogic
         TTSLOG_ERROR("PCM playback not Implemented");
         gst_bin_add_many(GST_BIN(m_pipeline), m_source, capsfilter, convert, resample, m_audioSink, NULL);
         result = gst_element_link_many (m_source,capsfilter,convert,resample,m_audioSink,NULL);
    }
#endif

    if(!result) {
        TTSLOG_ERROR("failed to link elements!");
        gst_object_unref(m_pipeline);
        m_pipeline = NULL;
        m_pipelineConstructionFailures++;
        return;
    }

    TTSLOG_WARNING ("gst_element_get_bus\n");
    GstBus *bus = gst_element_get_bus(m_pipeline);
    m_busWatch = gst_bus_add_watch(bus, GstBusCallback, (gpointer)(this));
    gst_object_unref(bus);

    m_pipelineConstructionFailures = 0;

    // wait until pipeline is set to NULL state
    resetPipeline();
}

void TTSSpeaker::resetPipeline() {
    TTSLOG_WARNING("Resetting Pipeline...");

    // Detect pipe line error and destroy the pipeline if any
    if(m_pipelineError) {
        TTSLOG_WARNING("Pipeline error occured, attempting to recover by re-creating pipeline");

        // Try to recover from errors by destroying the pipeline
        destroyPipeline();
    }
    m_pipelineError = false;
    m_networkError = false;
    m_isPaused = false;
    m_isEOS = false;

    if(!m_pipeline) {
        // If pipe line is NULL, create one
        createPipeline();
    } else {
        // If pipeline is present, bring it to NULL state
        gst_element_set_state(m_pipeline, GST_STATE_NULL);
        while(!waitForStatus(GST_STATE_NULL, 60*1000));
    }
}

void TTSSpeaker::destroyPipeline() {
    TTSLOG_WARNING("Destroying Pipeline...");

    if(m_pipeline) {
        gst_element_set_state(m_pipeline, GST_STATE_NULL);
        waitForStatus(GST_STATE_NULL, 1*1000);
        g_source_remove(m_busWatch);
        gst_object_unref(m_pipeline);
    }

    m_busWatch = 0;
    m_pipeline = NULL;
    m_pipelineConstructionFailures = 0;
    m_condition.notify_one();
}

void TTSSpeaker::waitForAudioToFinishTimeout(float timeout_s) {
    TTSLOG_TRACE("timeout_s=%f", timeout_s);

    auto timeout = std::chrono::system_clock::now() + std::chrono::seconds((unsigned long)timeout_s);
    auto startTime = std::chrono::system_clock::now();
    gint64 lastPosition = 0;

    auto playbackInterrupted = [this] () -> bool { return !m_pipeline || m_pipelineError || m_flushed; };
    auto playbackCompleted = [this] () -> bool { return m_isEOS; };

    while(timeout > std::chrono::system_clock::now()) {
        std::unique_lock<std::mutex> mlock(m_queueMutex);
        m_condition.wait_until(mlock, timeout, [this, playbackInterrupted, playbackCompleted] () {
            return playbackInterrupted() || playbackCompleted();
        });

        if(playbackInterrupted() || playbackCompleted()) {
            if(m_flushed)
                TTSLOG_VERBOSE("Bailing out because of forced text queue (m_flushed=true)");
            break;
        } else {
            if(m_isPaused) {
                timeout = std::chrono::system_clock::now() + std::chrono::seconds((unsigned long)timeout_s);
            } else {
                if(m_duration > 0 && m_duration != (gint64)GST_CLOCK_TIME_NONE &&
                    std::chrono::system_clock::now() < startTime + std::chrono::nanoseconds(m_duration)) {
                    timeout = std::chrono::system_clock::now() + std::chrono::seconds((unsigned long)timeout_s);
                    TTSLOG_VERBOSE("Not reached duration");
                } else {
                    // This is a workaround for broken BRCM PCM Sink duration query - To be deleted once that is fixed
                    m_duration = 0;
                    gint64 position = 0;
                    gst_element_query_position(m_pipeline, GST_FORMAT_TIME, &position);
                    if(position > 0 && position != (gint64)GST_CLOCK_TIME_NONE && position > lastPosition) {
                        TTSLOG_VERBOSE("Reached/Invalid duration, last position=%" GST_TIME_FORMAT ", current position=%" GST_TIME_FORMAT,
                                GST_TIME_ARGS(lastPosition), GST_TIME_ARGS(position));
                        timeout = std::chrono::system_clock::now() + std::chrono::seconds((unsigned long)timeout_s);
                        lastPosition = position;
                    }
                }
            }
        }
    }
    TTSLOG_INFO("m_isEOS=%d, m_pipeline=%p, m_pipelineError=%d, m_flushed=%d",
            m_isEOS, m_pipeline, m_pipelineError, m_flushed);

    // Irrespective of EOS / Timeout reset pipeline
    if(m_pipeline)
        gst_element_set_state(m_pipeline, GST_STATE_NULL);

    if(!m_isEOS)
        TTSLOG_ERROR("Stopped waiting for audio to finish without hitting EOS!");
    m_isEOS = false;
}

void TTSSpeaker::replaceIfIsolated(std::string& text, const std::string& search, const std::string& replace) {
    size_t pos = 0;
    while ((pos = text.find(search, pos)) != std::string::npos) {
        bool punctBefore = (pos == 0 || std::ispunct(text[pos-1]) || std::isspace(text[pos-1]));
        bool punctAfter = (pos+1 == text.length() || std::ispunct(text[pos+1]) || std::isspace(text[pos+1]));

        if(punctBefore && punctAfter) {
            text.replace(pos, search.length(), replace);
            pos += replace.length();
        } else {
            pos += search.length();
        }
    }
}

bool TTSSpeaker::isSilentPunctuation(const char c) {
    static std::string SilentPunctuation = "?!:;-()";
    return (SilentPunctuation.find(c) != std::string::npos);
}

void TTSSpeaker::replaceSuccesivePunctuation(std::string& text) {
    size_t pos = 0;
    while(pos < text.length()) {
        // Remove unwanted characters
        static std::string stray = "\"";
        if(stray.find(text[pos]) != std::string::npos) {
            text.erase(pos,1);
            if(++pos == text.length())
                break;
        }

        if(ispunct(text[pos])) {
            ++pos;
            while(pos < text.length() && (isSilentPunctuation(text[pos]) || isspace(text[pos]))) {
                if(isSilentPunctuation(text[pos]))
                    text.erase(pos,1);
                else
                    ++pos;
            }
        } else {
            ++pos;
        }
    }
}

void TTSSpeaker::curlSanitize(std::string &sanitizedString) {
    CURL *curl = curl_easy_init();
    if(curl) {
      char *output = curl_easy_escape(curl, sanitizedString.c_str(), sanitizedString.size());
      if(output) {
          sanitizedString = output;
          curl_free(output);
      }
    }
    curl_easy_cleanup(curl);
}

void TTSSpeaker::sanitizeString(std::string &input, std::string &sanitizedString) {
    sanitizedString = input.c_str();

    replaceIfIsolated(sanitizedString, "$", "dollar");
    replaceIfIsolated(sanitizedString, "#", "pound");
    replaceIfIsolated(sanitizedString, "&", "and");
    replaceIfIsolated(sanitizedString, "|", "bar");
    replaceIfIsolated(sanitizedString, "/", "or");

    replaceSuccesivePunctuation(sanitizedString);

    curlSanitize(sanitizedString);

    TTSLOG_VERBOSE("In:%s, Out:%s", input.c_str(), sanitizedString.c_str());
}

bool TTSSpeaker::needsPipelineUpdate() {
   return (m_pipelineConstructionFailures < m_maxPipelineConstructionFailures ? true : !m_queue.empty()) &&
       ((m_ensurePipeline && !m_pipeline) || (m_pipeline && !m_ensurePipeline));
}

std::string TTSSpeaker::constructURL(TTSConfiguration &config, SpeechData &d) {
    if(!config.isValid()) {
        TTSLOG_ERROR("Invalid configuration");
        return "";
    }

    // EndPoint URL
    std::string tts_request;
    if(d.secure)
        tts_request.append(config.secureEndPoint());
    else
        tts_request.append(config.endPoint());

    // Voice
    if(!config.voice().empty()) {
        tts_request.append("voice=");
        tts_request.append(config.voice().c_str());
    }

    // Language
    if(!config.language().empty()) {
        tts_request.append("&language=");
        tts_request.append(config.language());
    }

    // Rate / speed
    tts_request.append("&rate=");
    tts_request.append(std::to_string(config.rate() > 100 ? 100 : config.rate()));

    // Sanitize String
    std::string sanitizedString;
    sanitizeString(d.text, sanitizedString);

    tts_request.append("&text=");
    tts_request.append(sanitizedString);

    TTSLOG_WARNING("Constructured final URL is %s", tts_request.c_str());
    return tts_request;
}

void TTSSpeaker::speakText(TTSConfiguration config, SpeechData &data) {
    m_isEOS = false;
    m_duration = 0;

    if(m_pipeline && !m_pipelineError && !m_flushed) {
        m_currentSpeech = &data;

        g_object_set(G_OBJECT(m_source), "location", constructURL(config, data).c_str(), NULL);
        // PCM Sink seems to be accepting volume change before PLAYING state
        g_object_set(G_OBJECT(m_audioSink), "volume", (double) (data.client->configuration()->volume() / MAX_VOLUME), NULL);
        gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
        TTSLOG_VERBOSE("Speaking.... ( %d, \"%s\")", data.id, data.text.c_str());

        //Wait for EOS with a timeout incase EOS never comes
	if(m_pcmAudioEnabled) {
	  //FIXME, find out way to EOS or position for raw PCM audio
           waitForAudioToFinishTimeout(60);
	}
	else {
              waitForAudioToFinishTimeout(10);
	}
    } else {
        TTSLOG_WARNING("m_pipeline=%p, m_pipelineError=%d", m_pipeline, m_pipelineError);
    }
    m_currentSpeech = NULL;
}

void TTSSpeaker::event_loop(void *data)
{
    TTSSpeaker *speaker= (TTSSpeaker*) data;
    speaker->main_context = g_main_context_new();
    speaker->main_loop = g_main_loop_new(NULL, false);
    g_main_loop_run(speaker->main_loop);
}

void TTSSpeaker::GStreamerThreadFunc(void *ctx) {
    TTSSpeaker *speaker = (TTSSpeaker*) ctx;

    TTSLOG_INFO("Starting GStreamerThread");

    while(speaker && speaker->m_runThread) {
        if(speaker->needsPipelineUpdate()) {
            if(speaker->m_ensurePipeline) {
                speaker->createPipeline();

                // If pipeline creation fails, send playbackerror to the client and remove the req from queue
                if(!speaker->m_pipeline && !speaker->m_queue.empty()) {
                    SpeechData data = speaker->dequeueData();
                    TTSLOG_ERROR("Pipeline creation failed, sending error for speech=%d from client %p\n", data.id, data.client);
                    data.client->playbackerror(data.id);
                    speaker->m_pipelineConstructionFailures = 0;
                }
            } else {
                speaker->destroyPipeline();
            }
        }

        // Take an item from the queue
        TTSLOG_INFO("Waiting for text input");
        while(speaker->m_runThread && speaker->m_queue.empty() && !speaker->needsPipelineUpdate()) {
            std::unique_lock<std::mutex> mlock(speaker->m_queueMutex);
            speaker->m_condition.wait(mlock, [speaker] () {
                    return (!speaker->m_queue.empty() || !speaker->m_runThread || speaker->needsPipelineUpdate());
                });
        }

        // Stop thread on Speaker's cue
        if(!speaker->m_runThread) {
            if(speaker->m_pipeline) {
                gst_element_set_state(speaker->m_pipeline, GST_STATE_NULL);
                speaker->waitForStatus(GST_STATE_NULL, 1*1000);
            }
            TTSLOG_INFO("Stopping GStreamerThread");
            return;
        }

        if(speaker->needsPipelineUpdate()) {
            continue;
        }

        TTSLOG_INFO("Got text input, list size=%d", speaker->m_queue.size());
        SpeechData data = speaker->dequeueData();

        speaker->setSpeakingState(true, data.client);
        // Inform the client before speaking
        if(!speaker->m_flushed)
            data.client->willSpeak(data.id, data.text);

        // Push it to gstreamer for speaking
        if(!speaker->m_flushed) {
            speaker->speakText(*data.client->configuration(), data);
        }

        // Inform the client after speaking
        if(speaker->m_flushed)
            data.client->interrupted(data.id);
        else if(speaker->m_networkError)
            data.client->networkerror(data.id);
        else if(!speaker->m_pipeline || speaker->m_pipelineError)
            data.client->playbackerror(data.id);
        else
            data.client->spoke(data.id, data.text);
        speaker->setSpeakingState(false);

        // stop the pipeline until the next tts string...
        speaker->resetPipeline();
    }

    speaker->destroyPipeline();
}

int TTSSpeaker::GstBusCallback(GstBus *, GstMessage *message, gpointer data) {
    TTSSpeaker *speaker = (TTSSpeaker*)data;
    return speaker->handleMessage(message);
}

bool TTSSpeaker::handleMessage(GstMessage *message) {
    GError* error = NULL;
    gchar* debug = NULL;

    if(!m_pipeline) {
        TTSLOG_WARNING("NULL Pipeline");
        return false;
    }

    switch (GST_MESSAGE_TYPE(message)){
        case GST_MESSAGE_ERROR: {
                gst_message_parse_error(message, &error, &debug);
                TTSLOG_ERROR("error! code: %d, %s, Debug: %s", error->code, error->message, debug);
                GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(m_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "error-pipeline");
                std::string source = GST_MESSAGE_SRC_NAME(message);
                if(source.find("souphttpsrc") == 0)
                    m_networkError = true;
                m_pipelineError = true;
                m_condition.notify_one();
            }
            break;

        case GST_MESSAGE_WARNING: {
                gst_message_parse_warning(message, &error, &debug);
                TTSLOG_WARNING("warning! code: %d, %s, Debug: %s", error->code, error->message, debug);
            }
            break;

        case GST_MESSAGE_EOS: {
                TTSLOG_INFO("Audio EOS message received");
                m_isEOS = true;
                m_condition.notify_one();
            }
            break;


        case GST_MESSAGE_DURATION_CHANGED: {
                gst_element_query_duration(m_pipeline, GST_FORMAT_TIME, &m_duration);
                TTSLOG_INFO("Duration %" GST_TIME_FORMAT, GST_TIME_ARGS(m_duration));
            }
            break;

        case GST_MESSAGE_STATE_CHANGED: {
                gchar* filename;
                GstState oldstate, newstate, pending;
                gst_message_parse_state_changed (message, &oldstate, &newstate, &pending);

                // Ignore messages not coming directly from the pipeline.
                if (GST_ELEMENT(GST_MESSAGE_SRC(message)) != m_pipeline)
                    break;

                filename = g_strdup_printf("%s-%s", gst_element_state_get_name(oldstate), gst_element_state_get_name(newstate));
                GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(m_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, filename);
                g_free(filename);

                // get the name and state
                TTSLOG_VERBOSE("%s old_state %s, new_state %s, pending %s",
                        GST_MESSAGE_SRC_NAME(message) ? GST_MESSAGE_SRC_NAME(message) : "",
                        gst_element_state_get_name (oldstate), gst_element_state_get_name (newstate), gst_element_state_get_name (pending));

                if (oldstate == GST_STATE_NULL && newstate == GST_STATE_READY) {
                } else if (oldstate == GST_STATE_READY && newstate == GST_STATE_PAUSED) {
                    GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(m_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "paused-pipeline");
                } else if (oldstate == GST_STATE_PAUSED && newstate == GST_STATE_PAUSED) {
                } else if (oldstate == GST_STATE_PAUSED && newstate == GST_STATE_PLAYING) {
                    GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(m_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "playing-pipeline");
                    std::lock_guard<std::mutex> lock(m_stateMutex);
                    if(m_clientSpeaking) {
                        if(m_isPaused) {
                            m_isPaused = false;
                            m_clientSpeaking->resumed(m_currentSpeech->id);
                            m_condition.notify_one();
                        } else {
                            m_clientSpeaking->started(m_currentSpeech->id, m_currentSpeech->text);
                        }
                    }
                } else if (oldstate == GST_STATE_PLAYING && newstate == GST_STATE_PAUSED) {
                    std::lock_guard<std::mutex> lock(m_stateMutex);
                    if(m_clientSpeaking && m_isPaused) {
                        m_clientSpeaking->paused(m_currentSpeech->id);
                        m_condition.notify_one();
                    }
                } else if (oldstate == GST_STATE_PAUSED && newstate == GST_STATE_READY) {
                } else if (oldstate == GST_STATE_READY && newstate == GST_STATE_NULL) {
                }
            }
            break;

        default:
            break;
    }

    if(error)
        g_error_free(error);

    if(debug)
        g_free(debug);

    return true;
}

} // namespace TTS
