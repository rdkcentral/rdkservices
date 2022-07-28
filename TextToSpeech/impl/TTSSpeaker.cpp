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
#include "TTSURLConstructer.h"
#include <unistd.h>
#include <regex>

#define INT_FROM_ENV(env, default_value) ((getenv(env) ? atoi(getenv(env)) : 0) > 0 ? atoi(getenv(env)) : default_value)
#define TTS_CONFIGURATION_STORE "/opt/persistent/tts.setting.ini"
#define UPDATE_AND_RETURN(o, n) if(o != n) { o = n; return true; }


namespace WPEFramework {
namespace Plugin {
bool _readFromFile(std::string filename, TTS::TTSConfiguration &ttsConfig);
bool _writeToFile(std::string filename, TTS::TTSConfiguration &ttsConfig);
}//namespace Plugin
}//namespace WPEFramework

namespace TTS {

std::map<std::string, std::string> TTSConfiguration::m_others;

TTSConfiguration::TTSConfiguration() :
    m_ttsEndPoint(""),
    m_ttsEndPointSecured(""),
    m_language("en-US"),
    m_apiKey(""),
    m_voice(""),
    m_volume(MAX_VOLUME),
    m_rate(DEFAULT_RATE),
    m_enabled(false),
    m_preemptiveSpeaking(true) { }

TTSConfiguration::~TTSConfiguration() {}

TTSConfiguration::TTSConfiguration(TTSConfiguration &config)
{
    m_ttsEndPoint = config.m_ttsEndPoint;
    m_ttsEndPointSecured = config.m_ttsEndPointSecured;
    m_language = config.m_language;
    m_apiKey = config.m_apiKey;
    m_voice = config.m_voice;
    m_volume = config.m_volume;
    m_rate = config.m_rate;
    m_enabled = config.m_enabled;
    m_preemptiveSpeaking = config.m_preemptiveSpeaking;
    m_data.scenario = config.m_data.scenario;
    m_data.value = config.m_data.value;
    m_data.path = config.m_data.path;
    m_fallbackenabled = config.m_fallbackenabled;
}
TTSConfiguration& TTSConfiguration::operator = (const TTSConfiguration &config)
{
    m_ttsEndPoint = config.m_ttsEndPoint;
    m_ttsEndPointSecured = config.m_ttsEndPointSecured;
    m_language = config.m_language;
    m_apiKey = config.m_apiKey;
    m_voice = config.m_voice;
    m_volume = config.m_volume;
    m_rate = config.m_rate;
    m_enabled = config.m_enabled;
    m_preemptiveSpeaking = config.m_preemptiveSpeaking;
    m_data.scenario = config.m_data.scenario;
    m_data.value = config.m_data.value;
    m_data.path = config.m_data.path;
    m_fallbackenabled = config.m_fallbackenabled;
    return *this;
}

bool TTSConfiguration::setEndPoint(const std::string endpoint) {
    if(!endpoint.empty())
    {
        UPDATE_AND_RETURN(m_ttsEndPoint, endpoint);
    }
    else
        TTSLOG_VERBOSE("Invalid TTSEndPoint input \"%s\"", endpoint.c_str());
    return false;
}

bool TTSConfiguration::setSecureEndPoint(const std::string endpoint) {
    if(!endpoint.empty())
    {
        UPDATE_AND_RETURN(m_ttsEndPointSecured, endpoint);
    }
    else
        TTSLOG_VERBOSE("Invalid Secured TTSEndPoint input \"%s\"", endpoint.c_str());
    return false;
}

bool TTSConfiguration::setApiKey(const std::string apikey) {
    if(!apikey.empty())
    {
        UPDATE_AND_RETURN(m_apiKey, apikey);
    }
    else
        TTSLOG_VERBOSE("Invalid api key input \"%s\"", apikey.c_str());
    return false;
}

bool TTSConfiguration::setLanguage(const std::string language) {
    if(!language.empty())
    {
        UPDATE_AND_RETURN(m_language, language);
    }
    else
        TTSLOG_VERBOSE("Empty Language input");
    return false;
}

bool TTSConfiguration::setVoice(const std::string voice) {
    if(!voice.empty())
    {
        UPDATE_AND_RETURN(m_voice, voice);  
    }
    else
        TTSLOG_VERBOSE("Empty Voice input");
    return false;
}

bool TTSConfiguration::setVolume(const double volume) {
    if(volume >= 1 && volume <= 100)
    {
        UPDATE_AND_RETURN(m_volume, volume);    
    }
    else
        TTSLOG_VERBOSE("Invalid Volume input \"%lf\"", volume);
    return false;
}



bool TTSConfiguration::setRate(const uint8_t rate) {
    if(rate >= 1 && rate <= 100)
    {
        UPDATE_AND_RETURN(m_rate, rate);    
    }
    else
        TTSLOG_VERBOSE("Invalid Rate input \"%u\"", rate);
    return false;
}

bool TTSConfiguration::setEnabled(const bool enabled) {
    UPDATE_AND_RETURN(m_enabled, enabled);
    return false;
}

void TTSConfiguration::setPreemptiveSpeak(const bool preemptive) {
    m_preemptiveSpeaking = preemptive;
}

bool TTSConfiguration::loadFromConfigStore()
{
    return WPEFramework::Plugin::_readFromFile(TTS_CONFIGURATION_STORE, *this);
}

bool TTSConfiguration::updateConfigStore()
{
    return WPEFramework::Plugin::_writeToFile(TTS_CONFIGURATION_STORE, *this);
}

const std::string TTSConfiguration::voice() {
    std::string str;

    if(!m_voice.empty())
        return m_voice;
    else {
        std::string key = std::string("voice_for_") + m_language;
        auto it = m_others.find(key);
        if(it != m_others.end())
            str = it->second;
        return str;
    }
}

bool TTSConfiguration::updateWith(TTSConfiguration &nConfig) {
    bool updated = false;
    setEndPoint(nConfig.m_ttsEndPoint);
    setSecureEndPoint(nConfig.m_ttsEndPointSecured);
    updated |= setLanguage(nConfig.m_language);
    updated |= setVoice(nConfig.m_voice);
    updated |= setVolume(nConfig.m_volume);
    updated |= setRate(nConfig.m_rate);
    return updated;
}

bool TTSConfiguration::isValid() {
    if((m_ttsEndPoint.empty() && m_ttsEndPointSecured.empty())) {
        TTSLOG_ERROR("TTSEndPointEmpty=%d, TTSSecuredEndPointEmpty=%d",
                m_ttsEndPoint.empty(), m_ttsEndPointSecured.empty());
        return false;
    }
    return true;
}

bool TTSConfiguration::isFallbackEnabled()
{
    return m_fallbackenabled;
}

void TTSConfiguration::saveFallbackPath(std::string path)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_data.path = path;
}

const std::string TTSConfiguration::getFallbackScenario()
{
    return m_data.scenario;
}

const std::string TTSConfiguration::getFallbackPath()
{
    return m_data.path;
}

const std::string TTSConfiguration::getFallbackValue()
{
    return m_data.value;
}

bool TTSConfiguration::setFallBackText(FallbackData &fd)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if((fd.scenario).empty() || (fd.value).empty())
    {
        return false;
    }
    else if(fd.scenario !=  m_data.scenario || fd.value !=  m_data.value)
    {
        m_data.scenario = fd.scenario;
        m_data.value = fd.value;
        m_data.path = fd.path;
        m_fallbackenabled = true;
        return true;
    }
    return false;
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
    m_audioVolume(NULL),
    m_main_loop(NULL),
    m_main_context(NULL),
    m_main_loop_thread(NULL),
    m_pipelineError(false),
    m_networkError(false),
    m_runThread(true),
    m_busThread(true),
    m_flushed(false),
    m_isEOS(false),
    m_pcmAudioEnabled(false),
#if defined(PLATFORM_AMLOGIC)
    m_audio_dev(NULL),
#endif
    m_ensurePipeline(false),
    m_busWatch(0),
    m_duration(0),
    m_pipelineConstructionFailures(0),
    m_maxPipelineConstructionFailures(INT_FROM_ENV("MAX_PIPELINE_FAILURE_THRESHOLD", 1)) {

        setenv("GST_DEBUG", "2", 0);
        setenv("GST_REGISTRY_UPDATE", "no", 0);
        setenv("GST_REGISTRY_FORK", "no", 0);

        m_main_loop_thread = g_thread_new("BusWatch", (void* (*)(void*)) event_loop, this);
        m_gstThread = new std::thread(GStreamerThreadFunc, this);

}

TTSSpeaker::~TTSSpeaker() {
    if(m_isSpeaking)
        m_flushed = true;
    m_runThread = false;
    m_busThread = false;
    m_pcmAudioEnabled = false;
#if defined(PLATFORM_AMLOGIC)
    if(m_audio_dev){
       audio_hw_unload_interface(m_audio_dev);
       m_audio_dev = NULL;
    }
#endif
    m_condition.notify_one();

    if(m_gstThread) {
        m_gstThread->join();
        m_gstThread = NULL;
    }

    if(g_main_loop_is_running(m_main_loop))
        g_main_loop_quit(m_main_loop);
    g_thread_join(m_main_loop_thread);
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

#if defined(PLATFORM_AMLOGIC)
bool TTSSpeaker::loadInitAudioDev()
{

    int ret = audio_hw_load_interface(&m_audio_dev);
    if (ret) {
        TTSLOG_ERROR("Amlogic audio_hw_load_interface failed:%d, can not control mix gain\n", ret);
        return false;
    }
    int inited = m_audio_dev->init_check(m_audio_dev);
    if (inited) {
        TTSLOG_ERROR("Amlogic audio device not inited, can not control mix gain\n");
        audio_hw_unload_interface(m_audio_dev);
	m_audio_dev = NULL;
        return false;
    }
  TTSLOG_INFO("Amlogic audio device loaded, can control mix gain");
  return true;
}
#endif
/*
 * Control gain of
 * primary audio (direct-mode=true),
 * system audio  (direct-mode=false)
 * app audio     (tts-mode=true)
 * mixgain value from 0 to -96  in db
 * 0   -> Maximum
 * -96 -> Minimum
*/
void TTSSpeaker::setMixGain(MixGain gain, int volume)
{
#if defined(PLATFORM_AMLOGIC)
     //Prim Mix gain
     double volGain = (double)volume/100;
     //convert voltage gain/loss to db
     double dbOut = round(1000000*20*(std::log(volGain)/std::log(10)))/1000000;
     int ret;
     char mixgain_cmd[32];
     if(gain == MIXGAIN_PRIM)
         snprintf(mixgain_cmd, sizeof(mixgain_cmd), "prim_mixgain=%d", (int)round(dbOut));
	else if( gain == MIXGAIN_SYS )
		snprintf(mixgain_cmd, sizeof(mixgain_cmd), "syss_mixgain=%d",(int)round(dbOut));
	else if(gain == MIXGAIN_TTS)
		snprintf(mixgain_cmd, sizeof(mixgain_cmd), "apps_mixgain=%d",(int)round(dbOut));
	 else {
		TTSLOG_ERROR("Unsuported Gain type=%d",gain);
                return;
	}

      if(m_audio_dev) {
         ret = m_audio_dev->set_parameters(m_audio_dev, mixgain_cmd );
         if(!ret) {
             TTSLOG_INFO("Amlogic audio dev  set param=%s success",mixgain_cmd);
         }
	  else {
		TTSLOG_ERROR("Amlogic audio dev  set_param=%s failed  error=%d",mixgain_cmd,ret);
	  }
     }
#elif defined(PLATFORM_REALTEK)
    hal_set_audio_volume(volume);	
#endif
}

// GStreamer Releated members
void TTSSpeaker::createPipeline() {
    m_isEOS = false;

    GstCaps *audiocaps = NULL;
    GstElement *capsfilter = NULL;
    m_pcmAudioEnabled = false;
    
    if(!m_ensurePipeline || m_pipeline) {
        TTSLOG_WARNING("Skipping Pipeline creation");
        return;
    }

#if defined(PLATFORM_AMLOGIC)
    //load Amlogic audio device if not
    if(!m_audio_dev) loadInitAudioDev();
#endif
    TTSLOG_WARNING("Creating Pipeline...");
    m_pipeline = gst_pipeline_new(NULL);
    if (!m_pipeline) {
        m_pipelineConstructionFailures++;
        TTSLOG_ERROR("Failed to create gstreamer pipeline");
        return;
    }


    // create soc specific elements
#if defined(PLATFORM_BROADCOM)
    m_source = gst_element_factory_make("souphttpsrc", NULL);
    m_audioSink = gst_element_factory_make("brcmpcmsink", NULL);
    m_audioVolume = m_audioSink;
#elif defined(PLATFORM_AMLOGIC)
    GstElement *convert = gst_element_factory_make("audioconvert", NULL);
    GstElement *resample = gst_element_factory_make("audioresample", NULL);
    m_audioSink = gst_element_factory_make("amlhalasink", NULL);
    m_audioVolume = m_audioSink;
#elif defined(PLATFORM_REALTEK)
    GstElement *parse = gst_element_factory_make("mpegaudioparse", NULL);
    GstElement *decodebin = gst_element_factory_make("avdec_mp3", NULL);
    GstElement *convert = gst_element_factory_make("audioconvert", NULL);
    GstElement *resample = gst_element_factory_make("audioresample", NULL);
    GstElement *audiofilter = gst_element_factory_make("capsfilter", NULL);
    m_source = gst_element_factory_make("souphttpsrc", NULL);
    m_audioVolume = gst_element_factory_make("volume", NULL);
    m_audioSink = gst_element_factory_make("rtkaudiosink", NULL);
    g_object_set(G_OBJECT(m_audioSink), "media-tunnel",  FALSE, NULL);
    g_object_set(G_OBJECT(m_audioSink), "audio-service",  TRUE, NULL);
#endif

    std::string tts_url =
        !m_defaultConfig.secureEndPoint().empty() ? m_defaultConfig.secureEndPoint() : m_defaultConfig.endPoint();
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
        std::string LoopbackEndPoint = LOOPBACK_ENDPOINT;
        std::string  LocalhostEndPoint = LOCALHOST_ENDPOINT;
        //Check if url contains endpoint on localhost, enable PCM audio
        if((tts_url.rfind(LoopbackEndPoint,0) != std::string::npos)  || (tts_url.rfind(LocalhostEndPoint,0) != std::string::npos)){
            TTSLOG_INFO("PCM audio playback is enabled");
            m_pcmAudioEnabled = true;
        }

#if defined(PLATFORM_AMLOGIC)
        if(m_pcmAudioEnabled) {
            //Raw PCM audio does not work with souphhtpsrc on Amlogic alsaasink
            m_source = gst_element_factory_make("httpsrc", NULL);
        }
        else {
            m_source = gst_element_factory_make("souphttpsrc", NULL);
        }
        g_object_set(G_OBJECT(m_audioSink), "tts-mode", TRUE, NULL);
#endif

        g_object_set(G_OBJECT(m_source), "location", tts_url.c_str(), NULL);
    }

    // set the TTS volume to max.
    #if defined(PLATFORM_REALTEK)
    g_object_set(G_OBJECT(m_audioVolume), "volume", (double) 4.0 * (m_defaultConfig.volume() / MAX_VOLUME), NULL);
    #else
    g_object_set(G_OBJECT(m_audioVolume), "volume", (double) (m_defaultConfig.volume() / MAX_VOLUME), NULL);
    #endif

    // Add elements to pipeline and link
    if(m_pcmAudioEnabled) {
        //Add raw audio caps
        audiocaps = gst_caps_new_simple("audio/x-raw", "format", G_TYPE_STRING, "S16LE", "rate", G_TYPE_INT, 22050,
                                "channels", G_TYPE_INT, 1, "layout", G_TYPE_STRING, "interleaved", NULL);
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
        GstElement *decodebin = gst_element_factory_make("brcmmp3decoder", NULL);
        gst_bin_add_many(GST_BIN(m_pipeline), m_source, decodebin, m_audioSink, NULL);
        result &= gst_element_link (m_source, decodebin);
        result &= gst_element_link (decodebin, m_audioSink);
    }
    else {
        TTSLOG_INFO("PCM audio capsfilter added to sink");
        gst_bin_add_many(GST_BIN(m_pipeline), m_source, capsfilter, m_audioSink,NULL);
        result = gst_element_link_many (m_source,capsfilter,m_audioSink,NULL);
    }
#elif defined(PLATFORM_AMLOGIC)
    if(!m_pcmAudioEnabled) {
        GstElement *parser = gst_element_factory_make("mpegaudioparse", NULL);
        GstElement *decodebin = gst_element_factory_make("avdec_mp3", NULL);
        gst_bin_add_many(GST_BIN(m_pipeline), m_source, parser, decodebin, convert, resample, m_audioSink, NULL);
        result &= gst_element_link (m_source, parser);
        result &= gst_element_link (parser, decodebin);
        result &= gst_element_link (decodebin, convert);
        result &= gst_element_link (convert, resample);
        result &= gst_element_link (resample, m_audioSink);
    }
    else {
        TTSLOG_INFO("PCM audio capsfilter  added to sink");
        gst_bin_add_many(GST_BIN(m_pipeline), m_source, capsfilter, convert, resample, m_audioSink, NULL);
        result = gst_element_link_many (m_source,capsfilter,convert,resample,m_audioSink,NULL);
    }
#elif defined(PLATFORM_REALTEK)
    audiocaps = gst_caps_new_simple("audio/x-raw", "channels", G_TYPE_INT, 2, "rate", G_TYPE_INT, 48000, NULL);
    g_object_set( G_OBJECT(audiofilter),  "caps",  audiocaps, NULL );
    if(!m_pcmAudioEnabled) {
        gst_bin_add_many(GST_BIN(m_pipeline), m_source, parse, convert, resample, audiofilter, decodebin, m_audioSink, m_audioVolume, NULL);
        gst_element_link_many (m_source, parse, decodebin, convert, resample, audiofilter, m_audioVolume, m_audioSink, NULL);
    }
    else {
        TTSLOG_INFO("PCM audio capsfilter added to sink");
        gst_bin_add_many(GST_BIN(m_pipeline), m_source, m_audioVolume, convert, resample, m_audioSink,  NULL);
        gst_element_link_many (m_source, convert, resample, audiofilter, m_audioVolume, m_audioSink, NULL);
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
    TTSURLConstructer url;
    tts_request = url.constructURL(m_defaultConfig,d.text,false);
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
        #if defined(PLATFORM_REALTEK)
        g_object_set(G_OBJECT(m_audioVolume), "volume", (double) 4.0 * (data.client->configuration()->volume() / MAX_VOLUME), NULL);
        #else
        g_object_set(G_OBJECT(m_audioVolume), "volume", (double) (data.client->configuration()->volume() / MAX_VOLUME), NULL);
        #endif
        gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
        #if defined(PLATFORM_AMLOGIC) || defined(PLATFORM_REALTEK)
        setMixGain(MIXGAIN_PRIM,25);
        #endif
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
    speaker->m_main_context = g_main_context_new();
    speaker->m_main_loop = g_main_loop_new(NULL, false);
    g_main_loop_run(speaker->m_main_loop);
}

void TTSSpeaker::GStreamerThreadFunc(void *ctx) {
    TTSLOG_INFO("Starting GStreamerThread");
    TTSSpeaker *speaker = (TTSSpeaker*) ctx;

    if(!gst_is_initialized())
        gst_init(NULL,NULL);

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

	// when not speaking, set primary mixgain back to default.
	if(speaker->m_flushed || speaker->m_networkError || !speaker->m_pipeline || speaker->m_pipelineError) {
           #if defined(PLATFORM_AMLOGIC) || defined(PLATFORM_REALTEK)
	   speaker->setMixGain(MIXGAIN_PRIM,100);
           #endif
        }
        // Inform the client after speaking
        if(speaker->m_flushed)
            data.client->interrupted(data.id);
        else if(speaker->m_networkError)
            data.client->networkerror(data.id);
        else if(!speaker->m_pipeline || speaker->m_pipelineError)
            data.client->playbackerror(data.id);
        else {
            #if defined(PLATFORM_AMLOGIC) || defined(PLATFORM_REALTEK)
	    speaker->setMixGain(MIXGAIN_PRIM,100);
            #endif
            data.client->spoke(data.id, data.text);
	}
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
                if(source.find("httpsrc") != std::string::npos)
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
			    // -12db is almost 25%
                            #if defined(PLATFORM_AMLOGIC) || defined(PLATFORM_REALTEK)
			    setMixGain(MIXGAIN_PRIM,25);
                            #endif
                            m_clientSpeaking->resumed(m_currentSpeech->id);
                            m_condition.notify_one();
                        } else {
                            m_clientSpeaking->started(m_currentSpeech->id, m_currentSpeech->text);
                        }
                    }
                } else if (oldstate == GST_STATE_PLAYING && newstate == GST_STATE_PAUSED) {
                    std::lock_guard<std::mutex> lock(m_stateMutex);
                    if(m_clientSpeaking && m_isPaused) {
                        #if defined(PLATFORM_AMLOGIC) || defined(PLATFORM_REALTEK)
			setMixGain(MIXGAIN_PRIM,100);
                        #endif
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
