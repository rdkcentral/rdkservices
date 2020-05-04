/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/

#include "gstreamersoundplayer_intel.h"
#include <unistd.h>

#include <gst/gst.h>

#include <string.h>

#include <utils.h>

#if USE_SYSRES_MLT
#include "rpl_new.h"
#endif

#define AAC_DECODING_RFC_CMD ". /lib/rdk/getRFC.sh SOUND_PLAYER_AAC_DECODING"

#define MAKE_ELEMENT(variable, name)                            \
    do {                                                        \
        variable = gst_element_factory_make(name, #variable);   \
        if (!variable) {                                        \
            LOGERR(\
                         "Failed to create "#variable"\n");     \
            deleteSoundFile();                                  \
            return;                                             \
        } else {                                                \
            gst_bin_add(GST_BIN(m_pipeline), variable);           \
        }                                                       \
    } while(0);

namespace WPEFramework
{

    namespace Plugin
    {
        static Core::TimerType<ProgressInfo>& progressTimer = Core::SingletonType < Core::TimerType<ProgressInfo> >::Instance(64 * 1024, "ProgressTimer");

        static bool g_useSoundSoupHttpsrc = true;

        static const char* GstStateStr(GstState st)
        {
          switch (st)
          {
            case GST_STATE_VOID_PENDING:  return "GST_STATE_VOID_PENDING";
            case GST_STATE_NULL:          return "GST_STATE_NULL";
            case GST_STATE_READY:         return "GST_STATE_READY";
            case GST_STATE_PAUSED:        return "GST_STATE_PAUSED";
            case GST_STATE_PLAYING:       return "GST_STATE_PLAYING";
            default:                      return "GST_STATE:unknown";
          }
        }

        static void configure_source(GstElement* source, const char *url, bool useProxy, std::string proxyString, const std::map <std::string, std::string> &headers)
        {
            LOGINFO("configure_source() source=%p", (void*) source);

            std::string location = url;

            LOGINFO("configure_source() location=%s", location.data());

            std::string agent = "Mozilla/5.0 (X11; Linux i686; rv:2.0b10) Gecko/20100101 Firefox/4.0b10";

            if (!g_useSoundSoupHttpsrc)
            {
                g_object_set(G_OBJECT(source), "blocksize", 32768, NULL); /* Set the block size to 32K */
                g_object_set(G_OBJECT(source), "timeout", 5, NULL);
                g_object_set(G_OBJECT(source), "redirect-expected", true, NULL);

                g_object_set(G_OBJECT(source), "disable-process-signaling", strstr(url, "pandora.com") ||
                        (strstr(url, "voice-guidance-tts") && strstr(url, "comcast")), NULL);

            }
            g_object_set(G_OBJECT(source), "location",   location.data(), NULL);
            g_object_set(G_OBJECT(source), "user-agent", agent.c_str(), NULL);

            if (useProxy && !proxyString.empty())
            {
                LOGWARN("Using proxy %s for %s", proxyString.c_str(), location.data());
                g_object_set(G_OBJECT(source), "proxy", proxyString.c_str(), NULL);
            }

            if (headers.size() > 0)
            {
                GstStructure *new_headers = gst_structure_new("extra-headers", NULL);

                for (std::map <std::string, std::string>::const_iterator it = headers.cbegin(); it != headers.cend(); it++)
                {
                    GValue hVal = G_VALUE_INIT;
                    g_value_init (&hVal, G_TYPE_STRING);
                    g_value_set_string (&hVal, it->second.c_str());

                    gst_structure_set_value(new_headers, it->first.c_str(), &hVal);
                }

                g_object_set(G_OBJECT(source), "extra-headers", new_headers, NULL);

                gst_structure_free(new_headers);
            }
        }

        static void on_have_type(GstElement *typefind, guint probability, GstCaps *srcPadCaps, gpointer user_data)
        {
            if ((srcPadCaps == NULL) || (user_data == NULL))
            {
                LOGERR ("Typefind SRC Pad Caps NULL");
                return;
            }

            GstStructure *s = gst_caps_get_structure(srcPadCaps, 0);
            LOGWARN("on_have_type %s", gst_structure_get_name(s));

            if (strncmp (gst_structure_get_name(s), "audio/", 6) == 0)
            {
#ifdef USE_MPEG_AUDIO_PARSE
                GstElement  *sink    = (GstElement *)(((CGStreamerSoundPlayer*)user_data)->getParser());
#else
                GstElement  *sink    = (GstElement *)(((CGStreamerSoundPlayer*)user_data)->getAudioSink());
#endif
                GstPad      *sinkpad = gst_element_get_static_pad (sink, "sink");
                GstPad      *srcpad  = gst_element_get_static_pad (typefind, "src");

                bool linked = GST_PAD_LINK_SUCCESSFUL(gst_pad_link (srcpad, sinkpad));
                if (!linked)
                {
                    LOGINFO("Failed to link typefind and audio");
                }
                else
                {
                    LOGINFO("Configured audio");
                }
                gst_object_unref (sinkpad);
                gst_object_unref (srcpad);
            }
            else if (strncmp (gst_structure_get_name(s), "application/x-id3", 17) == 0)
            {
#ifdef USE_MPEG_AUDIO_PARSE
                GstElement  *sink    = (GstElement *)(((CGStreamerSoundPlayer*)user_data)->getParser());
#else
                GstElement  *sink    = (GstElement *)(((CGStreamerSoundPlayer*)user_data)->getAudioSink());
#endif
                GstPad      *sinkpad = gst_element_get_static_pad (sink, "sink");
                GstPad      *srcpad  = gst_element_get_static_pad (typefind, "src");

                GstElement  *id3demux = (GstElement *)(((CGStreamerSoundPlayer*)user_data)->getID3Demux());

                GstPad      *id3Sinkpad = gst_element_get_static_pad (id3demux, "sink");
                GstPad      *id3Srcpad  = gst_element_get_static_pad (id3demux, "src");

                bool linkedid3Sink  = GST_PAD_LINK_SUCCESSFUL(gst_pad_link (srcpad, id3Sinkpad));
                bool linkedid3Src   = GST_PAD_LINK_SUCCESSFUL(gst_pad_link (id3Srcpad, sinkpad));
                if (!linkedid3Sink || !linkedid3Src)
                {
                    LOGINFO("Failed to link typefind and audio");
                }
                else
                {
                    LOGINFO("Configured audio");
                }

                gst_object_unref (id3Sinkpad);
                gst_object_unref (id3Srcpad);

                gst_object_unref (sinkpad);
                gst_object_unref (srcpad);
            }
        }

        CGStreamerSoundPlayer::CGStreamerSoundPlayer(ISoundListener *soundListener)
        : m_progressTimer(this)
        , m_soundListener(soundListener)
        , m_runProgressTimer(false)
        , m_currentGstState(GST_STATE_NULL)
        {
            static bool once = true;
            if (once)
            {
                g_useSoundSoupHttpsrc = false;
            }
            once = false;
            m_id3demux = NULL;
#ifdef USE_MPEG_AUDIO_PARSE
            m_parse = NULL;
#endif
            m_audioSink = NULL;
            m_pipeline = NULL;
            m_useProxy = false;
            m_startedPlayback = false;
            m_proxyToUse = "";
            m_resourceId = 0;
            m_soundFile = "";
            m_progressTimerTick = 0;

            LOGWARN("%s %d:  Constructor :: Object pointer = %p", __FUNCTION__ , __LINE__, this);
        }

        CGStreamerSoundPlayer::~CGStreamerSoundPlayer()
        {
            LOGWARN("%s %d:  Distructor :: Object pointer = %p", __FUNCTION__ , __LINE__, this);
            deleteSoundFile();

            stopProgressTimer();
        }

        void CGStreamerSoundPlayer::onResourcePlay()
        {
            LOGINFO("CGStreamerSoundItem::onResourcePlay()");
            gst_element_set_state(GST_ELEMENT(m_pipeline), GST_STATE_PLAYING);
            startProgressTimer();
            m_progressTimerTick = 0;

            LOGWARN("%s %d : Timer started - m_progressTimer (%p) connected to object (%p)", __FILE__ , __LINE__ , m_progressTimer, this);
            m_soundListener->onItemPlaying();
            LOGWARN("End of CGStreamerSoundItem::onResourcePlay()");
        }

        void CGStreamerSoundPlayer::onResourcePause()
        {
            GstState gst_current;
            GstState gst_pending;
            float timeout = 100.0;

            stopProgressTimer();
            LOGWARN("%s %d : Timer stopped - m_progressTimer (%p) connected to object (%p)", __FILE__ , __LINE__ , m_progressTimer, this);

#ifdef USE_PIPELINE_FACTORY
            GstElement *pipeline = PipelineFactory::instance()->getPipeline(this);
#else
            GstElement *pipeline = getPipeline();
#endif
            if (!pipeline)
            {
                LOGERR("Cannot pause sound, no pipeline for #%d", m_resourceId);
                return;
            }

            if (GST_STATE_CHANGE_FAILURE  == gst_element_get_state(GST_ELEMENT(pipeline), &gst_current, &gst_pending, timeout * GST_MSECOND))
            {
                LOGINFO("CGStreamerSoundItem::onResourcePause - gst_element_get_state - FAILURE : State = %d, Pending = %d\n", gst_current, gst_pending);
            }
            else
            {
                LOGINFO("CGStreamerSoundItem::onResourcePause - gst_element_get_state - SUCCESS : State = %d, Pending = %d\n", gst_current, gst_pending);
            }

            if ((gst_current == GST_STATE_PAUSED) || (gst_current == GST_STATE_PLAYING))
            {
                GstStateChangeReturn gstRet = gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PAUSED);
                if ((GST_STATE_CHANGE_SUCCESS != gstRet) && (GST_STATE_CHANGE_NO_PREROLL != gstRet))
                {
                    LOGWARN("Did not receive success on state change gstRet=%d", gstRet);
                }
                if (gstRet == GST_STATE_CHANGE_ASYNC)
                {
                    GstState pipelineState;
                    gst_element_get_state(GST_ELEMENT(pipeline), &pipelineState, NULL, GST_CLOCK_TIME_NONE);
                }
            }
        }

        void CGStreamerSoundPlayer::onResourceSetVolume(unsigned volumeLevel)
        {
            LOGINFO("CGStreamerSoundItem::onResourceSetVolume(%d)", volumeLevel);

#ifdef USE_PIPELINE_FACTORY
            GstElement *sink = PipelineFactory::instance()->getVolumeElement(this);
#else
            GstElement *sink = m_audioSink;
#endif
            if (!sink)
            {
                LOGERR("setVolume called, but audioSink control is null for %d", m_resourceId);
                return;
            }

            gfloat gvol = volumeLevel / 100.0 * 8.0; // xre sets values in range [0..100], ismd_audio_sink accepts [0.0..8.0]
            g_object_set(G_OBJECT(sink), "volume", gvol, NULL);
        }

        static void qtdemux_on_pad_added(GstElement *element, GstPad *pad, gpointer data)
        {
            GstElement *brcmaudfilter = (GstElement *) data;

            GstPad *sinkpad = gst_element_get_static_pad (brcmaudfilter, "sink");

            if (GST_PAD_LINK_OK != gst_pad_link (pad, sinkpad))
                LOGWARN("Failed to link qtdemux to brcmaudfilter.");

            gst_object_unref (sinkpad);
        }

        void CGStreamerSoundPlayer::setSoundFile(std::string soundFile) {
            if (m_soundFile == soundFile) return;

            LOGWARN("CGStreamerSoundItem::setSoundFile('%s') of this = %p", soundFile.c_str(), this);

            if (!gst_is_initialized())
                gst_init(NULL, NULL);

            m_soundFile = soundFile;

            //QUrl url = QUrl();
            //url.setEncodedUrl(soundFile.toUtf8());

            /*if (!url.isValid())
            {
                QString msg = QString("Error: Invalid sound uri: '") + soundFile + "'";

                //QTextStream(&msg) << "Error: Invalid sound uri: '" << soundFile << "'";
                //LOGERR(msg.toAscii().constData());

                //LOGERR(msg.toUtf8().constData());
                LOGERR("%s", msg.toUtf8().constData());

                emit error(msg);
                m_progressTimer->stop();
                LOGWARN("%s %d : Timer stopped - m_progressTimer (%p) connected to object (%p)", __FILE__ , __LINE__ , m_progressTimer, this);
                return;
            }*/

            if (!aacDecodingEnabled() && (std::string::npos != soundFile.find(".aac") || std::string::npos != soundFile.find(".m4a")))
            {
                m_soundListener->onItemError("400. aac playback has been disabled.");
                return;
            }

#ifdef USE_PIPELINE_FACTORY
            GstElement *pipeline = PipelineFactory::instance()->getPipeline(this, true);
#else
            m_pipeline = gst_pipeline_new("SoundPipe");
            if (!m_pipeline)
            {
                std::string msg = "Error: Failed to create gstreamer pipeline";
                LOGERR("%s", msg.c_str());
                m_soundListener->onItemError(msg);
                return;
            }

            GstElement *typefind = NULL;
            GstElement *source = NULL;

            if (0 == soundFile.find("file://"))
            {
                MAKE_ELEMENT(source, "filesrc");
            }
            else
            {
                if (g_useSoundSoupHttpsrc)
                {
                    MAKE_ELEMENT(source, "souphttpsrc");
                }
                else
                {
                    MAKE_ELEMENT(source, "httpsrc");
                }
            }

            if (std::string::npos != soundFile.find(".aac") || std::string::npos != soundFile.find(".m4a"))
            {
                LOGWARN("Using AAC pipeline for '%s'.", soundFile.c_str());

                GstElement *gstqueue = NULL;
                GstElement *qtdemux = NULL;

                MAKE_ELEMENT(gstqueue,      "queue");
                MAKE_ELEMENT(qtdemux,       "qtdemux");
                MAKE_ELEMENT(m_audioSink,   "ismd_audio_sink");

                configure_source(source, soundFile.c_str(), m_useProxy, m_proxyToUse, m_headers);
                g_object_set(G_OBJECT(m_audioSink), "sync", FALSE, NULL);

                LOGWARN("CGStreamerSoundItem - Audio Set Input As Primary = %d\n", FALSE);
                g_object_set(G_OBJECT(m_audioSink), "audio-input-set-as-primary", FALSE, NULL);

                gst_element_link_many(source, gstqueue, qtdemux, NULL);

                // qtdemux should be linked to ismd_audio_sink as  qtdemux:audio_0 ---> ismd_audio_sink:sink,
                // but qtdemux:audio_0 is not available at this point.
                g_signal_connect(qtdemux, "pad_added", G_CALLBACK(qtdemux_on_pad_added), m_audioSink);
            }
            else
            {
                MAKE_ELEMENT(typefind,      "typefind");

                MAKE_ELEMENT(m_id3demux, "id3demux");

#ifdef USE_MPEG_AUDIO_PARSE
                MAKE_ELEMENT(m_parse,      "mpegaudioparse"); // provides seek
#endif

                MAKE_ELEMENT(m_audioSink,  "ismd_audio_sink");

                configure_source(source, soundFile.c_str(), m_useProxy, m_proxyToUse, m_headers);
                g_object_set(G_OBJECT(m_audioSink), "sync", FALSE, NULL);

                LOGWARN("CGStreamerSoundItem - Audio Set Input As Primary = %d\n", FALSE);
                g_object_set(G_OBJECT(m_audioSink), "audio-input-set-as-primary", FALSE, NULL);

                gst_element_link (source, typefind);

                m_startedPlayback = false;

                g_signal_connect (typefind, "have-type", G_CALLBACK (on_have_type), this);
#ifdef USE_MPEG_AUDIO_PARSE
                gst_element_link (m_parse, m_audioSink);
#endif

            }

#endif // USE_PIPELINE_FACTORY

            LOGWARN("CGStreamerSoundItem - emit open");
            m_soundListener->onItemOpen();

            LOGWARN("CGStreamerSoundItem, end of setSoundFile");
        }

        void CGStreamerSoundPlayer::deleteSoundFile()
        {
#ifdef USE_PIPELINE_FACTORY
            PipelineFactory::instance()->destroyPipeline(this);
#else
            GstElement *pipeline = getPipeline();

            if (!pipeline)
                return;

            LOGWARN("CGStreamerSoundItem::deleteSoundFile() for %s", m_soundFile.c_str());

            if(GST_STATE_NULL != validateStateWithMsTimeout(GST_STATE_NULL, 10)) { // 10 ms
                LOGWARN("CGStreamerSoundItem - validateStateWithMsTimeout SET TO NULL BEFORE TERM");
                gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);
            }
        //    gst_element_set_state(GST_ELEMENT(getPipeline()), GST_STATE_NULL);
            gst_object_unref(pipeline);
            m_pipeline  = NULL;
            m_audioSink = NULL;
#ifdef USE_MPEG_AUDIO_PARSE
            m_parse = NULL;
#endif
            m_id3demux = NULL;

#endif
            stopProgressTimer();
            LOGWARN("%s %d : Timer stopped - m_progressTimer (%p) connected to object (%p)", __FILE__ , __LINE__ , m_progressTimer, this);
        }

        GstState CGStreamerSoundPlayer::validateStateWithMsTimeout (GstState stateToValidate, guint msTimeOut)
        {
            GstState    gst_current;
            GstState    gst_pending;
            float       timeout = 50.0;
            gint        gstGetStateCnt = 5;

            do {
                if ((GST_STATE_CHANGE_SUCCESS == gst_element_get_state(getPipeline(), &gst_current, &gst_pending, timeout * GST_MSECOND)) && (gst_current == stateToValidate)) {
                    LOGWARN("CGStreamerSoundItem::validateStateWithMsTimeout - PIPELINE gst_element_get_state - SUCCESS : State = %d, Pending = %d\n", gst_current, gst_pending);
                    return gst_current;
                }
                // will be removing this method because of this trying not to call it
                usleep(msTimeOut * 1000); // Let pipeline safely transition to required state
            } while ((gst_current != stateToValidate) && (gstGetStateCnt-- != 0)) ;

            LOGWARN("CGStreamerSoundItem::validateStateWithMsTimeout - PIPELINE gst_element_get_state - FAILURE : State = %d, Pending = %d\n", gst_current, gst_pending);
            return gst_current;
        }

        void CGStreamerSoundPlayer::onResourceSetPosition(unsigned int position)
        {
            LOGINFO("CGStreamerSoundItem::onResourceSetPosition(%d)", position);
#ifdef USE_PIPELINE_FACTORY
            GstElement *pipeline = PipelineFactory::instance(this);
#else
            GstElement *pipeline = getPipeline();
#endif
            if (pipeline)
            {
                LOGERR("No pipeline, cannot set position for %d", m_resourceId);
                return;
            }
            if (!gst_element_seek(GST_ELEMENT(pipeline), 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
                                  GST_SEEK_TYPE_SET, position * GST_MSECOND,
                                  GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE))
            {
                LOGERR("setPosition failed for %d", m_resourceId);
                return;
            }
            LOGINFO("setPosition succeeded");
        }

        void CGStreamerSoundPlayer::checkProgress()
        {
            {
                std::lock_guard<std::mutex> lock(m_progressMutex);

                if (!m_runProgressTimer)
                    return;
            }

#ifdef USE_PIPELINE_FACTORY
            GstElement *pipeline = PipelineFactory::instance(this);
#else
            GstElement *pipeline = getPipeline();
#endif
            if (!pipeline)
            {
                LOGERR("No pipeline");
                return;
            }

            {
                GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(getPipeline()));
                GstMessage *msg = NULL;
                while ((msg = gst_bus_pop(bus)) != NULL)
                {
                    processBusMessage(bus, msg);
                    gst_message_unref(msg);
                }
            }

            if (m_startedPlayback == true)
            {
                int64_t pos = -1;
                int64_t dur = -1;
                GstFormat fmt = GST_FORMAT_TIME;
#ifdef USE_GST1
                if (gst_element_query_position(GST_ELEMENT(pipeline), fmt, &pos) &&
                    gst_element_query_duration(GST_ELEMENT(pipeline), fmt, &dur))
#else
                if (gst_element_query_position(GST_ELEMENT(pipeline), &fmt, &pos) &&
                    gst_element_query_duration(GST_ELEMENT(pipeline), &fmt, &dur))
#endif
                {
                    m_soundListener->onItemProgress(dur / 1000000, pos / 1000000);

                    //log progress at certain intervals 0s, 1s, 5s, 30s, 2min, every 20min
                    if(m_progressTimerTick == 0 ||
                            m_progressTimerTick == 1 * 2 ||
                            m_progressTimerTick == 5 * 2 ||
                            m_progressTimerTick == 30 * 2 ||
                            m_progressTimerTick == 2 * 60 * 2 ||
                            !(m_progressTimerTick % (20 * 60 * 2))
                            )
                        LOGWARN("duration:%f position:%f", dur / 1000000.0, pos / 1000000.0);

                    m_progressTimerTick++;
                }
                else
                {
                    LOGERR("Could not query position and duration for %d", m_resourceId);
                }
            }

            progressTimer.Schedule(Core::Time::Now().Add(500), m_progressTimer);
        }

        void CGStreamerSoundPlayer::processBusMessage(GstBus *bus, GstMessage *msg)
        {
            LOGINFO("Message type: '%s', source name: '%s'", GST_MESSAGE_TYPE_NAME(msg), GST_MESSAGE_SRC_NAME(msg));

            switch (GST_MESSAGE_TYPE(msg))
            {
            case GST_MESSAGE_EOS:
                handleEoS(msg);
                break;
            case GST_MESSAGE_ERROR:
                handleError(msg);
                break;
            case GST_MESSAGE_STATE_CHANGED:
                handleStateChanged(msg);
                break;
            default:
                break;
            }

        }

        void CGStreamerSoundPlayer::handleEoS(GstMessage *msg)
        {
            int64_t     pos = -1;

#ifdef USE_GST1
            if (!gst_element_query_position(GST_ELEMENT(msg->src), GST_FORMAT_TIME, &pos))
#else
            GstFormat  fmt = GST_FORMAT_TIME;
            if (!gst_element_query_position(GST_ELEMENT(msg->src), &fmt, &pos))
#endif
            {
                LOGERR("Could not determine position at EOS");
            }

            LOGWARN("End of stream %lld", pos);

            LOGINFO("OnBusEos()");
#ifdef USE_PIPELINE_FACTORY
            GstElement *pipeline = PipelineFactory::instance()->getPipeline(this);
#else
            GstElement *pipeline = getPipeline();
#endif
            if (!pipeline)
            {
                LOGERR("Could not send complete event, no pipeline for %d", m_resourceId);
                return;
            }

            gst_element_set_state(GST_ELEMENT(getPipeline()), GST_STATE_NULL);

            m_soundListener->onItemComplete(pos);

            stopProgressTimer();
            LOGWARN("%s %d : Timer stopped - m_progressTimer (%p) connected to object (%p)", __FILE__ , __LINE__ , m_progressTimer, this);
        }

        void CGStreamerSoundPlayer::handleError(GstMessage *msg)
        {
            std::string   errorMessage;
            gchar     *gdebug = NULL;
            GError    *gerror = NULL;

            gst_message_parse_error(msg, &gerror, &gdebug);

            errorMessage = gerror->message;

            LOGERR("Bus Error: %s %s", gerror->message, gdebug);

            m_soundListener->onItemError(errorMessage);
            stopProgressTimer();
            LOGWARN("%s %d : Timer stopped - m_progressTimer (%p) connected to object (%p)", __FILE__ , __LINE__ , m_progressTimer, this);

            g_error_free(gerror);
            g_free(gdebug);
        }

        void CGStreamerSoundPlayer::handleStateChanged(GstMessage *msg)
        {
            GstState newstate, oldstate, pending;
            gst_message_parse_state_changed(msg, &oldstate, &newstate, &pending);

            if (newstate != pending && newstate != m_currentGstState)
            {
                //log when we go to or leave the playing state
                if(oldstate == GST_STATE_PLAYING || newstate == GST_STATE_PLAYING)
                    LOGWARN("'%s':'%s' state changed to '%s'", GST_MESSAGE_SRC_NAME(msg), GstStateStr(oldstate), GstStateStr(newstate));
                else
                    LOGINFO("'%s':'%s' state changed to '%s'", GST_MESSAGE_SRC_NAME(msg), GstStateStr(oldstate), GstStateStr(newstate));

                m_currentGstState = newstate;

                int64_t pos = -1;
                GstFormat fmt = GST_FORMAT_TIME;

#ifdef USE_PIPELINE_FACTORY
                GstElement *pipeline = PipelineFactory::instance()->getPipeline(this);
#else
                GstElement *pipeline = getPipeline();
#endif
                if (!pipeline)
                {
                    LOGERR("Could not determine state change, no pipeline for %d", m_resourceId);
                    return;
                }
                switch (newstate)
                {
                case GST_STATE_PLAYING:
                    m_soundListener->onItemPlaying();
                    m_startedPlayback = true;
                    break;
                case GST_STATE_PAUSED:
                    if (m_startedPlayback == true) {
#ifdef USE_GST1
                        if (!gst_element_query_position(GST_ELEMENT(pipeline), fmt, &pos))
#else
                        if (!gst_element_query_position(GST_ELEMENT(pipeline), &fmt, &pos))
#endif
                        {
                            LOGERR("Could not determine position at pause state change");
                        }
                    }
                    m_soundListener->onItemPaused(pos);
                    break;
                default:
                    break;
                }
            }
        }

        void CGStreamerSoundPlayer::onResourceDeleteSoundItem()
        {
            LOGWARN("onResourceDeleteSoundItem");
            deleteSoundFile();

            stopProgressTimer();
        }

        void CGStreamerSoundPlayer::onResourceSetSoundFile(std::string soundFile, bool useProxy, std::string proxyToUse, XREIndex resourceId, const std::map <std::string, std::string> &headers)
        {
            if (m_soundFile == soundFile) return;

            LOGWARN("%s %d:  Reset the old pipeline of this = %p", __FUNCTION__ , __LINE__, this);
            deleteSoundFile();

            m_useProxy = useProxy;
            m_proxyToUse = proxyToUse;
            m_resourceId = resourceId;
            m_headers = headers;
            setSoundFile(soundFile);
        }

        bool CGStreamerSoundPlayer::aacDecodingEnabled()
        {
            static bool calledRFC = false;
            static bool isEnabled = true;

            if (!calledRFC)
            {
                calledRFC = true;

                FILE *p = popen(AAC_DECODING_RFC_CMD, "r");

                if(p)
                {
                    char buf[256];
                    if (fgets(buf, sizeof(buf), p))
                    {
                        if (0 == strncmp(buf, "0", 1) || 0 == strncmp(buf, "false", 5))
                        {
                            isEnabled = false;
                        }
                    }
                    else
                    {
                        LOGWARN("Empty output from %s", AAC_DECODING_RFC_CMD);
                    }
                }
                else
                {
                    LOGWARN("CSoundResource failed to exec %s:%s", AAC_DECODING_RFC_CMD, strerror(errno));
                }

                pclose(p);
            }

            return isEnabled;
        }

        void CGStreamerSoundPlayer::startProgressTimer()
        {
            {
                std::lock_guard<std::mutex> lock(m_progressMutex);
                m_runProgressTimer = true;
            }

            progressTimer.Schedule(Core::Time::Now().Add(500), m_progressTimer);
        }

        void CGStreamerSoundPlayer::stopProgressTimer()
        {
            progressTimer.Revoke(m_progressTimer);

            {
                std::lock_guard<std::mutex> lock(m_progressMutex);
                m_runProgressTimer = false;
            }
        }

        uint64_t ProgressInfo::Timed(const uint64_t scheduledTime)
        {
            uint64_t result = 0;
            m_soundPlayer->checkProgress();
            return(result);
        }
    }
}
