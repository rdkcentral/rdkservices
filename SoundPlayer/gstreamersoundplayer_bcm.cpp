/******************************************************************************
 *  Copyright (c) 2016 RDK Management, LLC. All rights reserved.
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/

#include "gstreamersoundplayer_bcm.h"

#include <string.h>
#include <vector>

#include <utils.h>

#ifdef PANDORA_HACK
#include "uidev.h"
#endif

#if USE_SYSRES_MLT
#include "rpl_new.h"
#endif

#define AAC_DECODING_RFC_CMD ". /lib/rdk/getRFC.sh SOUND_PLAYER_AAC_DECODING"

namespace WPEFramework
{
    namespace Plugin
    {
        static Core::TimerType<ProgressInfo> progressTimer(64 * 1024, "ProgressTimer");

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

        struct GstPooledPipeline
        {
            GstElement          * pipeline;
            GstElement          * source;
            CGStreamerSoundPlayer * owner;
        };

#define GSSOUNDITEM_DEFAULT_VOLUME (100) /* default volume is full scale, no attenuation */

#ifdef USE_SWMP3_DECODER
#define MAX_GST_SOUND_PIPELINES (2)    /* Default max PCM channels is 2 for most BRCM SOCs and max gst pipeline handles is 4;
                                                  This value can be increased, but PCM channel defines in NEXUS will also need to be increased */

        static GstPooledPipeline g_gstSoundPipelines[MAX_GST_SOUND_PIPELINES] = {{NULL, NULL, NULL}, {NULL, NULL, NULL}};
#else
#define MAX_GST_SOUND_PIPELINES (1)
        static GstPooledPipeline g_gstSoundPipelines[MAX_GST_SOUND_PIPELINES] = {{NULL, NULL, NULL}};
#endif
        static int g_nextAvailablePipeline = 0;
        static bool g_useSoundSoupHttpsrc = true;

        static void configure_source(GstElement* source, const char *url, bool useProxy, std::string proxyString, const std::map <std::string, std::string> &headers)
        {
            LOGINFO("configure_source() source=%p", (void*) source);

            std::string location = url;

            LOGINFO("configure_source() location=%s", location.data());

            std::string agent = "Mozilla/5.0 (X11; Linux i686; rv:2.0b10) Gecko/20100101 Firefox/4.0b10";

#ifndef RDK_USE_NXCLIENT
            g_object_set(G_OBJECT(source), "set_playback_ip_probe", FALSE, NULL);
            g_object_set(G_OBJECT(source), "client_trick_mode", TRUE, NULL);
            g_object_set(G_OBJECT(source), "buffer-size",   512, NULL);
#endif
            if (!g_useSoundSoupHttpsrc)
            {
                g_object_set(G_OBJECT(source), "blocksize", 32768, NULL); /* Set the block size to 32K */
                //g_object_set(G_OBJECT(source), "timeout", XRE::Settings::httpSrcConnectionTimeout(), NULL);
                g_object_set(G_OBJECT(source), "timeout", 5, NULL);
                g_object_set(G_OBJECT(source), "redirect-expected",   TRUE, NULL);

                g_object_set(G_OBJECT(source), "disable-process-signaling", strstr(url, "pandora.com") ||
                        (strstr(url, "voice-guidance-tts") && strstr(url, "comcast")), NULL);

            }
            g_object_set(G_OBJECT(source), "location",  location.data(), NULL);
        //    g_object_set(G_OBJECT(source), "user-agent", agent.c_str(), NULL);

#if 0
#define HINT_DURATION_LIVE (0x00007fffffffffff)
            gint64 hint =HINT_DURATION_LIVE;
            DBG_MSG_CB(("G_OBJECT (source), hint_duration, %lld\n ", hint));
            g_object_set(G_OBJECT (source), "hint_duration", hint, NULL);
#endif

            if (useProxy && !proxyString.empty())
            {
                LOGINFO("Using proxy %s for %s", proxyString.c_str(), location.data());
                g_object_set(G_OBJECT (source), "proxy", proxyString.c_str(), NULL);
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

#ifndef USE_SWMP3_DECODER
        static void on_pad_added(GstElement *gstelement, GstPad *pad, gpointer user_data)
        {
            GstCaps *caps = gst_pad_get_caps(pad);
            GstStructure *s = gst_caps_get_structure(caps, 0);

            LOGINFO("on_pad_added() %s", gst_structure_get_name(s));

            if (strncmp (gst_structure_get_name(s), "audio/", 6) == 0)
            {
                CGStreamerSoundPlayer *soundItem = (CGStreamerSoundPlayer *) user_data;
                GstElement *sink = soundItem->m_audioSink;
                GstPad* sinkpad = gst_element_get_static_pad (sink, "sink");
                if (!GST_PAD_IS_LINKED (sinkpad))
                {
                    //brcm gstr need this added later
                    gst_bin_add (GST_BIN (soundItem->m_pipeline), sink);

                    bool linked = GST_PAD_LINK_SUCCESSFUL(gst_pad_link (pad, sinkpad));
                    if (!linked)
                    {
                        LOGINFO("on_pad_added(). Failed to link demux and audio");
                    }
                    else
                    {
                        LOGINFO("on_pad_added(). Audio configured");
                    }
                    gst_object_unref (sinkpad);
                }
            }
        }
#endif // #ifndef USE_SWMP3_DECODER

        CGStreamerSoundPlayer::CGStreamerSoundPlayer(ISoundListener *soundListener)
        : m_progressTimer(this)
        , m_soundListener(soundListener)
        , m_runProgressTimer(false)
        , m_currentGstState(GST_STATE_NULL)
        {
            static bool once = true;
            if (once)
            {
                //g_useSoundSoupHttpsrc = XRE::Settings::useSoundSoupHttpsrc();
                g_useSoundSoupHttpsrc = false;
            }
            once = false;

            m_progressTimerTick = 0;

            LOGWARN("%s %d:  Constructor :: Object pointer = %p", __FUNCTION__ , __LINE__, this);


        /*
            if (XRE::Settings::proxyEnabled())
            {
                QString proxyHost = XRE::Settings::proxyHost();
                int proxyPort = XRE::Settings::proxyPort();
                m_networkProxyFactory = new CNetworkProxyFactory();
                m_networkProxyFactory->addHTTPProxy(proxyHost, proxyPort);
                m_localProxyServerSetting = proxyHost + QString(":") + QString::number(proxyPort);
            }
        */
#ifdef PANDORA_HACK
            //Hack to get mpeos decoder handle
            UIDev_Init((char *) "RI Process");
#endif
        }

        CGStreamerSoundPlayer::~CGStreamerSoundPlayer()
        {
            LOGWARN("%s %d:  Distructor :: Object pointer = %p", __FUNCTION__ , __LINE__, this);

            deleteSoundFile();

            stopProgressTimer();

#ifdef PANDORA_HACK
            //Hack to get mpeos decoder handle
            UIDev_ReleaseResource(UIDEV_RESOURCE_DECODER_1);
#endif
        }


        void CGStreamerSoundPlayer::onResourcePlay()
        {
            LOGINFO("CGStreamerSoundItem::play()");
            GstElement *pipeline = getPipeline();
            if (!pipeline)
            {
                LOGERR("Cannot play sound, no pipeline for %d", m_resourceId);
                return;
            }
            gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);
            startProgressTimer();

            m_progressTimerTick = 0;
            LOGWARN("End of CGStreamerSoundItem::onResourcePlay()");
        }

        void CGStreamerSoundPlayer::pause_privLocked()
        {
            GstState gst_current;
            GstState gst_pending;
            float timeout = 100.0;

            stopProgressTimer();

            GstElement *pipeline = getPipeline();
            if (!pipeline)
            {
                LOGERR("Cannot pause sound, no pipeline for #%d", m_resourceId);
                return;
            }

            if (GST_STATE_CHANGE_FAILURE  == gst_element_get_state(GST_ELEMENT(pipeline), &gst_current, &gst_pending, timeout * GST_MSECOND))
            {
                LOGINFO("CGStreamerSoundItem::pause_privLocked - gst_element_get_state - FAILURE : State = %d, Pending = %d\n", gst_current, gst_pending);
            }
            else
            {
              LOGINFO("CGStreamerSoundItem::pause_privLocked - gst_element_get_state - SUCCESS : State = %d, Pending = %d\n", gst_current, gst_pending);
            }

            if ((gst_current == GST_STATE_PAUSED) || (gst_current == GST_STATE_PLAYING)) {
                GstStateChangeReturn gstRet = gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PAUSED);
                if ((GST_STATE_CHANGE_SUCCESS != gstRet) && (GST_STATE_CHANGE_NO_PREROLL != gstRet))
                {
                    LOGWARN("Did not receive success on state change gstRet=%d", gstRet);
                }
                if (gstRet == GST_STATE_CHANGE_ASYNC)
                {
                    GstState pipelineState;
                    gst_element_get_state(GST_ELEMENT(pipeline), &pipelineState, NULL, timeout * GST_MSECOND);
                }
            }
            LOGINFO("end pause_privLocked()");
        }

        void CGStreamerSoundPlayer::onResourcePause()
        {
            LOGINFO("pause()");
            pause_privLocked();
        }

        void CGStreamerSoundPlayer::onResourceSetVolume(unsigned volumeLevel)
        {
            LOGINFO("CGStreamerSoundItem::onResourceSetVolume(%d)", volumeLevel);

            // save the volumeLevel in case volumeElment is not available
            m_volume = volumeLevel;
#ifdef USE_PIPELINE_FACTORY
            GstElement *volumeElement = PipelineFactory::instance()->getVolumeElement(this);
#else
            GstElement *volumeElement = m_volumeElement;
#endif

            if (!volumeElement)
            {
                LOGINFO("onResourceSetVolume called, element for volume control is null for %d, m_volume is set", m_resourceId);
                return;
            }

#ifndef USE_SWMP3_DECODER
            gfloat gvol = volumeLevel / 100.0 * 10.0; // xre sets values in range [0..100], brcmaudiosink accepts [0.0..10.0]
#else
            gdouble gvol = volumeLevel / 100.0; // xre sets values in range [0..100], brcmaudiosink accepts [0.0..1.0]
#endif // ifndef USE_SWMP3_DECODER
            g_object_set(G_OBJECT(m_volumeElement), "volume", gvol, NULL);
        }

        void CGStreamerSoundPlayer::setSoundFile(std::string soundFile)
        {
            if (soundFile == m_soundUrl) return;

            LOGWARN("setSoundFile(%s) of this = %p", soundFile.c_str(), this);

            m_soundUrl = soundFile;

            /* check if we already have a pipeline set up for this sound */
            if ((m_pipelineIndex>=0) && (m_pipelineIndex<MAX_GST_SOUND_PIPELINES) && (g_gstSoundPipelines[m_pipelineIndex].owner==this))
            {
                LOGINFO("playing from gst sound pipeline %d", m_pipelineIndex);
            }
            else
            {
                LOGINFO("Checking for next available pipeline");
                if (g_gstSoundPipelines[g_nextAvailablePipeline].owner != NULL)
                {
                    LOGWARN("pipeline %d in use; re-opening", g_nextAvailablePipeline);

                    g_gstSoundPipelines[g_nextAvailablePipeline].owner->onError("Max sound pipelines");

                    destroyPipeline(g_nextAvailablePipeline);
                }
                m_pipelineIndex = g_nextAvailablePipeline;
                createPipeline(m_pipelineIndex);
                g_gstSoundPipelines[m_pipelineIndex].owner = this;
                g_nextAvailablePipeline++;
                g_nextAvailablePipeline %= MAX_GST_SOUND_PIPELINES;
            }

            m_soundListener->onItemOpen();
        }

        static void qtdemux_on_pad_added(GstElement *element, GstPad *pad, gpointer data)
        {
            GstElement *brcmaudfilter = (GstElement *) data;

            GstPad *sinkpad = gst_element_get_static_pad (brcmaudfilter, "brcm-audfilter-sink");

            if (GST_PAD_LINK_OK != gst_pad_link (pad, sinkpad))
            {
                LOGWARN("Failed to link qtdemux to brcmaudfilter.");
            }

            gst_object_unref (sinkpad);
        }

        void CGStreamerSoundPlayer::createPipeline(int pipelineIndex)
        {
            LOGINFO("createPipeline %d", pipelineIndex);

            if (!gst_is_initialized())
                gst_init(NULL, NULL);

            if (!aacDecodingEnabled() && (std::string::npos != m_soundUrl.find(".aac") || std::string::npos != m_soundUrl.find(".m4a")))
            {
                m_soundListener->onItemError("400. aac playback has been disabled.");
                return;
            }

#ifdef PANDORA_HACK
            //Hack to get mpeos decoder handle
            UIDev_RequestResource(UIDEV_RESOURCE_DECODER_1);
#endif
            GstElement *gstSoundPipeline = NULL;
            GstElement *decodebin = NULL;

            gstSoundPipeline = gst_pipeline_new(NULL);
            if (!gstSoundPipeline)
            {
                LOGERR("createPipeline(%d) - Failed to create gstreamer pipeline", pipelineIndex);
                return;
            }

#ifdef USE_GST1
            gst_object_ref_sink(GST_OBJECT (gstSoundPipeline));
#else
            gst_object_ref(GST_OBJECT (gstSoundPipeline));
            gst_object_sink(GST_OBJECT (gstSoundPipeline));
#endif

            m_pipeline = gstSoundPipeline;

            if (std::string::npos != m_soundUrl.find(".aac") || std::string::npos != m_soundUrl.find(".m4a"))
            {
                LOGWARN("Using AAC pipeline for '%s'.", m_soundUrl.c_str());

                m_source                    = gst_element_factory_make((g_useSoundSoupHttpsrc ? "souphttpsrc" : "httpsrc"), NULL);
                GstElement *gstqueue        = gst_element_factory_make("queue", NULL);
                GstElement *qtdemux         = gst_element_factory_make("qtdemux", NULL);
                GstElement *brcmaudfilter   = gst_element_factory_make("brcmaudfilter", NULL);
                decodebin                   = gst_element_factory_make("brcmaudiodecoder", NULL);
                m_audioSink                 = gst_element_factory_make("brcmaudiosink", NULL);

                // Add elements to pipeline and link
                gst_bin_add_many(GST_BIN(gstSoundPipeline), m_source, gstqueue, qtdemux, brcmaudfilter, decodebin, m_audioSink, NULL);

                configure_source(m_source, m_soundUrl.c_str(), m_useProxy, m_proxyToUse, m_headers);

                gst_element_link_many (m_source, gstqueue, qtdemux, NULL);

                // qtdemux should be linked to brcmaudfilter as  qtdemux:audio_0 ---> brcmaudfilter:brcm-audfilter-sink,
                // but qtdemux:audio_0 is not available at this point.
                g_signal_connect(qtdemux, "pad_added", G_CALLBACK(qtdemux_on_pad_added), brcmaudfilter);

                gst_element_link_many (brcmaudfilter, decodebin, m_audioSink, NULL);

                m_volumeElement = m_audioSink;
            }
            else
            {
#ifdef USE_SWMP3_DECODER
#ifdef RDK_USE_NXCLIENT
                /* RDK_USE_NXCLIENT define is used to indicate usage of updated brcm-plugins using nxclient which started with URSR 13.2 */
                m_source    = gst_element_factory_make((g_useSoundSoupHttpsrc ? "souphttpsrc" : "httpsrc"), NULL);
                decodebin   = gst_element_factory_make("brcmmp3decoder", NULL);
                m_audioSink = gst_element_factory_make("brcmpcmsink", NULL);

                // Add elements to pipeline and link
                gst_bin_add_many(GST_BIN(gstSoundPipeline), m_source, decodebin, m_audioSink, NULL);

                configure_source(m_source, m_soundUrl.c_str(), m_useProxy, m_proxyToUse, m_headers);

                gst_element_link (m_source, decodebin);
                gst_element_link (decodebin, m_audioSink);
                m_volumeElement = m_audioSink;
#else /*RDK_USE_NXCLIENT*/
                GstElement * parse = NULL;
                GstElement * audioPlay = NULL;

                m_source    = gst_element_factory_make((g_useSoundSoupHttpsrc ? "souphttpsrc" : "httpsrc"), NULL);
                parse       = gst_element_factory_make("mp3parse", NULL);
                decodebin   = gst_element_factory_make("brcmmp3audiodecoder", NULL);
                audioPlay   = gst_element_factory_make("brcmaudioplayback", NULL);
                m_audioSink = gst_element_factory_make("brcmaudiosink", NULL);

                // Add elements to pipeline and link
                gst_bin_add_many(GST_BIN(gstSoundPipeline), m_source, parse, decodebin, audioPlay, m_audioSink, NULL);

                configure_source(m_source, m_soundUrl, m_useProxy, m_proxyToUse);

                g_object_set(G_OBJECT(m_audioSink), "pts_45k_base",   FALSE, NULL);

                gst_element_link (m_source, parse);
                gst_element_link (parse, decodebin);
                gst_element_link (decodebin, audioPlay);
                gst_element_link (audioPlay, m_audioSink);
                m_volumeElement = audioPlay;
#endif /*RDK_USE_NXCLIENT*/
#else  /*USE_SWMP3_DECODER*/
                m_source    = gst_element_factory_make((g_useSoundSoupHttpsrc ? "souphttpsrc" : "httpsrc"), NULL);
                decodebin   = gst_element_factory_make("decodebin2", "decode-bin");
                m_audioSink = gst_element_factory_make("brcmaudiosink", "audio-output");

                // Add elements to pipeline and link
                gst_bin_add_many(GST_BIN(gstSoundPipeline), m_source, decodebin, NULL);

                configure_source(m_source, m_soundUrl, m_useProxy, m_proxyToUse, m_headers);

                gst_element_link (m_source, decodebin);

                // Use GstElement::pad-added instead of this signal.
                g_signal_connect (decodebin, "pad-added", G_CALLBACK (on_pad_added), this);
                m_volumeElement = m_audioSink;
#endif /*USE_SWMP3_DECODER*/
            }

            if (m_volume != GSSOUNDITEM_DEFAULT_VOLUME)
            {
                onResourceSetVolume(m_volume);
            }


            g_gstSoundPipelines[pipelineIndex].source = m_source;
            g_gstSoundPipelines[pipelineIndex].owner = this;
            g_gstSoundPipelines[pipelineIndex].pipeline = gstSoundPipeline;
            }

        void CGStreamerSoundPlayer::destroyPipeline(int pipelineIndex)
        {
            CGStreamerSoundPlayer * pipelineOwner = g_gstSoundPipelines[pipelineIndex].owner;
            LOGINFO("CurObj = %p destroyPipeline(): pipelineOwner %p, pipeline %p pipelineIndex %d", this, pipelineOwner, pipelineOwner->getPipeline(), pipelineIndex);
            pipelineOwner->pause_privLocked();

        /*
            if(GST_STATE_NULL != validateStateWithMsTimeout(GST_STATE_NULL, 10)) { // 10 ms
                LOGWARN("CGStreamerSoundItem - validateStateWithMsTimeout SET TO NULL BEFORE TERM");
            }
        */
            gst_element_set_state(GST_ELEMENT(pipelineOwner->getPipeline()), GST_STATE_NULL);
            gst_object_unref(pipelineOwner->getPipeline());

            pipelineOwner->m_volumeElement = NULL;
            pipelineOwner->m_position = 0;
            pipelineOwner->m_pipelineIndex = -1;
            pipelineOwner->m_audioSink = NULL;
            pipelineOwner->m_pipeline  = NULL;
            pipelineOwner->m_source = NULL;

            g_gstSoundPipelines[pipelineIndex].pipeline = NULL;
            g_gstSoundPipelines[pipelineIndex].source = NULL;
            g_gstSoundPipelines[pipelineIndex].owner = NULL;
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

        void CGStreamerSoundPlayer::deleteSoundFile()
        {
            LOGINFO("CurObj = %p deleteSoundFile()", this);

            stopProgressTimer();

            if ((getPipeline()) && (m_pipelineIndex>=0))
            {
                destroyPipeline(m_pipelineIndex);
            }
        }

        void CGStreamerSoundPlayer::onResourceSetPosition(unsigned int position)
        {
            LOGINFO("setPosition(%d)", position);
            m_position = position;
            if (!getPipeline())
            {
                /* position is stored, apply when pipeline is created */
                LOGWARN("setPosition(). No pipeline, cannot set position for %d", m_resourceId);
                return;
            }
            if (!gst_element_seek(GST_ELEMENT(getPipeline()), 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
                                GST_SEEK_TYPE_SET, position * GST_MSECOND,
                                GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE))
            {
                LOGERR("setPosition() failed for %d", m_resourceId);
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

            if (!getPipeline())
                return;


            {
                GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(getPipeline()));
                GstMessage *msg = NULL;
                while ((msg = gst_bus_pop(bus)) != NULL)
                {
                    processBusMessage(bus, msg);
                    gst_message_unref(msg);
                }
            }

            gint64 pos = -1;
            gint64 dur = -1;
            GstFormat fmt = GST_FORMAT_TIME;

#ifdef USE_GST1
            if (gst_element_query_position(GST_ELEMENT(getPipeline()), fmt, &pos) &&
                gst_element_query_duration(GST_ELEMENT(getPipeline()), fmt, &dur))
#else
            if (gst_element_query_position(GST_ELEMENT(getPipeline()), &fmt, &pos) &&
                gst_element_query_duration(GST_ELEMENT(getPipeline()), &fmt, &dur))
#endif
            {
                LOGINFO("emit progress pos=%lld dur=%lld", pos, dur);
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
                LOGWARN("checkProgress(). Could not query position and duration for %d", m_resourceId);
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
            GstFormat fmt = GST_FORMAT_TIME;
            if (!gst_element_query_position(GST_ELEMENT(msg->src), &fmt, &pos))
#endif
            {
                LOGERR("Could not determine position at EOS");
            }
            LOGWARN("End of stream %lld", pos);

            LOGINFO("OnBusEos()");
            GstElement *pipeline = getPipeline();
            if (pipeline)
            {
                gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);
                m_soundListener->onItemComplete(pos);
            }
            else
            {
                LOGWARN("Pipeline changed at End of Stream");
            }

            stopProgressTimer();
        }

        void CGStreamerSoundPlayer::handleError(GstMessage *msg)
        {
            std::string errorMessage;
            gchar     *gdebug = NULL;
            GError    *gerror = NULL;

            gst_message_parse_error(msg, &gerror, &gdebug);

            errorMessage = gerror->message;

            LOGERR("Bus Error: %s %s", errorMessage.c_str(), gdebug);

            LOGWARN("OnBusError(%s)", errorMessage.c_str());
            m_soundListener->onItemError(errorMessage);

            stopProgressTimer();

            GstElement *pipeline = getPipeline();

            if (pipeline && errorMessage == "Generic mp3 decode error")
                gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);

            g_error_free(gerror);
            g_free(gdebug);
        }

        void CGStreamerSoundPlayer::handleStateChanged(GstMessage *msg)
        {
            GstState newstate, oldstate, pending;
            gst_message_parse_state_changed(msg, &oldstate, &newstate, &pending);
            if (newstate != pending && newstate != m_currentGstState)
            {
                m_currentGstState = newstate;

                //log when we go to or leave the playing state
                if(oldstate == GST_STATE_PLAYING || newstate == GST_STATE_PLAYING)
                    LOGWARN("'%s':'%s' state changed to '%s'", GST_MESSAGE_SRC_NAME(msg), GstStateStr(oldstate), GstStateStr(newstate));
                else
                    LOGINFO("'%s':'%s' state changed to '%s'", GST_MESSAGE_SRC_NAME(msg), GstStateStr(oldstate), GstStateStr(newstate));

                if (!getPipeline())
                {
                    LOGERR("Could not determine state change, no pipeline for %d", m_resourceId);
                    return;
                }

                gint64 pos = -1;
                GstFormat fmt = GST_FORMAT_TIME;

                switch (newstate)
                {
                case GST_STATE_PLAYING:
                    m_soundListener->onItemPlaying();
                    break;
                case GST_STATE_PAUSED:
#ifdef USE_GST1
                    if (!gst_element_query_position(GST_ELEMENT(getPipeline()), fmt, &pos))
#else
                    if (!gst_element_query_position(GST_ELEMENT(getPipeline()), &fmt, &pos))
#endif
                    {
                        LOGERR("OnBusStateChanged(). Could not determine position at pause state change.");
                    }
                    m_soundListener->onItemPaused(pos);
                    break;
                default:
                    break;
                }
            }
        }

        void CGStreamerSoundPlayer::onError(const std::string &error)
        {

        }

        void CGStreamerSoundPlayer::onResourceSetSoundFile(std::string soundFile, bool useProxy, std::string proxyToUse, XREIndex resourceId, const std::map <std::string, std::string> &headers)
        {
            m_audioSink = NULL;
            m_pipeline = NULL;
            m_source = NULL;
            m_volumeElement = NULL;
            m_volume = GSSOUNDITEM_DEFAULT_VOLUME;
            m_soundUrl = "";
            m_pipelineIndex = -1;
            m_position = 0;
            m_localProxyServerSetting = "";
            m_useProxy = useProxy;
            m_proxyToUse = proxyToUse;
            m_headers = headers;
            m_resourceId = resourceId;
            setSoundFile(soundFile);
        }

        void CGStreamerSoundPlayer::releaseCurrentDecoder()
        {
            if (g_gstSoundPipelines[g_nextAvailablePipeline].owner == NULL)
            {
                return;
            }
            g_gstSoundPipelines[g_nextAvailablePipeline].owner->onError("Max sound pipelines");
            CGStreamerSoundPlayer * pipelineOwner = g_gstSoundPipelines[g_nextAvailablePipeline].owner;
            pipelineOwner->pause_privLocked();

            gst_element_set_state(GST_ELEMENT(pipelineOwner->getPipeline()), GST_STATE_NULL);
            gst_object_unref(pipelineOwner->getPipeline());

            pipelineOwner->m_volumeElement = NULL;
            pipelineOwner->m_position = 0;
            pipelineOwner->m_pipelineIndex = -1;
            pipelineOwner->m_audioSink = NULL;
            pipelineOwner->m_pipeline  = NULL;
            pipelineOwner->m_source = NULL;

            g_gstSoundPipelines[g_nextAvailablePipeline].pipeline = NULL;
            g_gstSoundPipelines[g_nextAvailablePipeline].source = NULL;
            g_gstSoundPipelines[g_nextAvailablePipeline].owner = NULL;
            g_nextAvailablePipeline++;
            g_nextAvailablePipeline %= MAX_GST_SOUND_PIPELINES;
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
