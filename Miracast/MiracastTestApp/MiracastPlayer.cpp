/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2023 RDK Management
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

#include <gst/audio/audio.h>
#include <gst/app/gstappsink.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sys/types.h>
#include <sys/syscall.h>
#include "MiracastLogger.h"
#include "MiracastController.h"
#include "MiracastPlayer.h"

MiracastPlayer *MiracastPlayer::mMiracastPlayer{nullptr};

MiracastPlayer *MiracastPlayer::getInstance()
{
    if (mMiracastPlayer == nullptr)
    {
        mMiracastPlayer = new MiracastPlayer();
    }
    return mMiracastPlayer;
}

void MiracastPlayer::destroyInstance()
{
    MIRACASTLOG_TRACE("Entering...");
    if (mMiracastPlayer != nullptr)
    {
        mMiracastPlayer->stop();
        if (mMiracastPlayer->stop())
        {
            MIRACASTLOG_ERROR("Stop miracast player");
        }
        else
        {
            MIRACASTLOG_ERROR("Failed to stop miracast player");
        }
        delete mMiracastPlayer;
        mMiracastPlayer = nullptr;
    }
    MIRACASTLOG_TRACE("Exiting...");
}

MiracastPlayer::MiracastPlayer()
{
    MIRACASTLOG_TRACE("Entering...");
    gst_init(nullptr, nullptr);
    m_bBuffering = false;
    m_bReady = false;
    m_currentPosition = 0.0f;
    m_buffering_level = 100;
    m_player_statistics_tid = 0;
    MIRACASTLOG_TRACE("Exiting...");
}

MiracastPlayer::~MiracastPlayer()
{
    MIRACASTLOG_TRACE("Entering...");
    stop();
    MIRACASTLOG_TRACE("Exiting...");
}

bool MiracastPlayer::setUri(const std::string ipaddr, const std::string port)
{
    if (ipaddr.empty())
    {
        MIRACASTLOG_ERROR("Empty ipaddress");
        return false;
    }
    if (port.empty())
    {
        MIRACASTLOG_ERROR("Empty port");
        return false;
    }

    m_uri = "udp://" + ipaddr + ":" + port;
    return true;
}

std::string MiracastPlayer::getUri()
{
    return m_uri;
}

unsigned getGstPlayFlag(const char *nick)
{
    MIRACASTLOG_TRACE("Entering..!!!");
    static GFlagsClass *flagsClass = static_cast<GFlagsClass *>(g_type_class_ref(g_type_from_name("GstPlayFlags")));
    GFlagsValue *flag = g_flags_get_value_by_nick(flagsClass, nick);
    MIRACASTLOG_TRACE("Exiting..!!!");
    return (flag ? flag->value : 0);
}

bool MiracastPlayer::launch(std::string localip , std::string streaming_port)
{
    bool ret = false;

    if (setUri(localip, streaming_port))
    {
        ret = createPipeline();
    }
    return ret;
}

bool MiracastPlayer::pause()
{
    return changePipelineState(GST_STATE_PAUSED);
}

bool MiracastPlayer::stop()
{
    MIRACASTLOG_TRACE("Entering..");

    if (!m_pipeline)
    {
        MIRACASTLOG_WARNING("Pipeline is NULL");
        return false;
    }
    if (m_player_statistics_tid){
        pthread_cancel(m_player_statistics_tid);
        m_player_statistics_tid = 0;
    }

    if (m_playback_thread)
    {
        pthread_cancel(m_playback_thread);
    }

    GstStateChangeReturn ret;
    ret = gst_element_set_state(m_pipeline, GST_STATE_NULL);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        MIRACASTLOG_ERROR("Failed to set gst_element_set_state as NULL");
    }

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
    if (bus)
    {
        gst_bus_set_sync_handler(bus, nullptr, nullptr, nullptr);
        gst_object_unref(bus);
    }

    if (m_main_loop)
    {
        g_main_loop_unref(m_main_loop);
    }
    if (m_main_loop_context)
    {
        g_main_context_unref(m_main_loop_context);
    }
    if (m_pipeline)
    {
        g_object_unref(m_pipeline);
        m_pipeline = nullptr;
    }
    MIRACASTLOG_TRACE("Exiting..");
    return true;
}

bool MiracastPlayer::changePipelineState(GstState state) const
{
    MIRACASTLOG_TRACE("Entering..!!!");
    GstStateChangeReturn ret;
    bool status = false;
    GstState current, pending;
    current = pending = GST_STATE_VOID_PENDING;
    ret = gst_element_get_state(m_pipeline, &current, &pending, 0);

    if ((ret != GST_STATE_CHANGE_FAILURE) && (current == state || pending == state))
    {
        status = true;
    }
    MIRACASTLOG_INFO("Changing state to %s.", gst_element_state_get_name(state));
    if (gst_element_set_state(m_pipeline, state) != GST_STATE_CHANGE_FAILURE)
    {
        status = true;
    }
    else
    {
        status = false;
    }
    MIRACASTLOG_TRACE("Exiting..!!!");
    return status;
}

bool MiracastPlayer::createPipeline()
{
    MIRACASTLOG_TRACE("Entering..!!!");

    GstStateChangeReturn ret;
    GstBus *bus;
    // gint flags;
    m_bReady = false;
    m_currentPosition = 0.0f;

    /* create gst pipeline */
    m_main_loop_context = g_main_context_new();
    g_main_context_push_thread_default(m_main_loop_context);
    m_main_loop = g_main_loop_new(m_main_loop_context, FALSE);

    m_pipeline = gst_element_factory_make("playbin", "miracast_player");

    if (!m_pipeline)
    {
        MIRACASTLOG_ERROR("Could not load playbin element.");
        return false;
    }

    unsigned flagAudio = getGstPlayFlag("audio");
    unsigned flagVideo = getGstPlayFlag("video");

    g_object_set(m_pipeline, "flags", flagAudio | flagVideo, nullptr);

    MIRACASTLOG_INFO("Miracast playbin uri [ %s ]", m_uri.c_str());

    g_object_set(m_pipeline, "uri", (const gchar *)m_uri.c_str(), nullptr);

    m_video_sink = gst_element_factory_make("westerossink", nullptr);

#if 0
    if(g_object_class_find_property(G_OBJECT_GET_CLASS(m_video_sink), "immediate-output"))
    {
        g_object_set(G_OBJECT(m_video_sink), "immediate-output", FALSE, nullptr);
        MIRACASTLOG_INFO("Set immediate-output as FALSE \n");
    }
    if(g_object_class_find_property(G_OBJECT_GET_CLASS(m_video_sink), "avsync-mode")){
        g_object_set(G_OBJECT(m_video_sink), "avsync-mode", 1, nullptr);
        MIRACASTLOG_INFO("Set avsync-mode as 1 \n");
    }
    if(g_object_class_find_property(G_OBJECT_GET_CLASS(m_video_sink), "avsync-session")){
        g_object_set(G_OBJECT(m_video_sink), "avsync-session", 0, nullptr);
        MIRACASTLOG_INFO("Set avsync-session as 0 \n");
    }
#endif
    g_object_set(m_pipeline, "video-sink", m_video_sink, nullptr);

    bus = gst_element_get_bus(m_pipeline);
    m_bus_watch_id = gst_bus_add_watch(bus, (GstBusFunc)busMessageCb, this);
    gst_object_unref(bus);

    g_main_context_pop_thread_default(m_main_loop_context);
    pthread_create(&m_playback_thread, nullptr, MiracastPlayer::playbackThread, this);
    pthread_create(&m_player_statistics_tid, nullptr, MiracastPlayer::monitor_player_statistics_thread, this);

    MIRACASTLOG_TRACE("Start Playing.");

    /* Start playing */
    ret = gst_element_set_state(m_pipeline, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        MIRACASTLOG_ERROR("Unable to set the pipeline to the playing state.");
        gst_object_unref(m_pipeline);
        return false;
    }
    else if (ret == GST_STATE_CHANGE_NO_PREROLL)
    {
        MIRACASTLOG_INFO("Streaming live");
        m_is_live = true;
    }

    MIRACASTLOG_TRACE("Exit..!!!");
    return true;
}

void *MiracastPlayer::playbackThread(void *ctx)
{
    MIRACASTLOG_TRACE("Entering..!!!");
    MiracastPlayer *self = (MiracastPlayer *)ctx;
    g_main_context_push_thread_default(self->m_main_loop_context);
    g_main_loop_run(self->m_main_loop);
    self->m_playback_thread = 0;
    MIRACASTLOG_TRACE("Exiting..!!!");
    pthread_exit(nullptr);
}

void* MiracastPlayer::monitor_player_statistics_thread(void *ctx)
{
    bool isInteger = true;
    MIRACASTLOG_TRACE("Entering..!!!");
    MiracastPlayer *self = (MiracastPlayer *)ctx;
    
    /* Read Player stats if this /opt/miracast_player_stats flag is present */
    /* Read also confiurable timer from some file */
    int time_interval_sec = 60;
    while (true)
    {
        isInteger = true;
        time_interval_sec = 10;
        std::ifstream player_stats_monitor_delay("/opt/miracast_player_stats"); // Assuming the input file is named "input.txt"

        if (!player_stats_monitor_delay) {
            MIRACASTLOG_ERROR("Failed to open /opt/miracast_player_stats file\n");
        }
        else{
            std::string word;
            player_stats_monitor_delay >> word;
            player_stats_monitor_delay.close();

            if (word.empty())
            {
                isInteger = false;
            }
            else
            {
                for (char c : word) {
                    if (!isdigit(c)) {
                        isInteger = false;
                        break;
                    }
                }
            }

            if (isInteger && (time_interval_sec = std::atoi(word.c_str())))
            {
                self->get_player_statistics();
            }
            else
            {
                MIRACASTLOG_ERROR("The content of the file is not an integer\n");
            }
        }
        sleep(time_interval_sec);
    }
    self->m_player_statistics_tid = 0;
    MIRACASTLOG_TRACE("Exiting..!!!");
    pthread_exit(nullptr);
}

gboolean MiracastPlayer::busMessageCb(GstBus *bus, GstMessage *msg, gpointer user_data)
{
    MIRACASTLOG_TRACE("Entering..!!!");
    MiracastPlayer *self = static_cast<MiracastPlayer *>(user_data);

    switch (GST_MESSAGE_TYPE(msg))
    {
    case GST_MESSAGE_ERROR:
    {
        GError *error;
        gchar *info;
        gst_message_parse_error(msg, &error, &info);
        MIRACASTLOG_ERROR("Error received from element [%s | %s | %s].", GST_OBJECT_NAME(msg->src), error->message, info ? info : "none");
        g_error_free(error);
        g_free(info);
        gst_element_set_state(self->m_pipeline, GST_STATE_READY);
        g_main_loop_quit(self->m_main_loop);
        break;
    }
    case GST_MESSAGE_EOS:
    {
        MIRACASTLOG_INFO("End-Of-Stream reached.");
        self->m_bBuffering = false;
        gst_element_set_state(self->m_pipeline, GST_STATE_READY); // TBD ? (Should be do explicitly or destry automatically.)
        g_main_loop_quit(self->m_main_loop);
        break;
    }
    case GST_MESSAGE_STATE_CHANGED:
    {
        if (GST_MESSAGE_SRC(msg) == GST_OBJECT(self->m_pipeline))
        {
            GstState old, now, pending;
            static int id = 0;
            id++;
            gst_message_parse_state_changed(msg, &old, &now, &pending);
            if (memcmp(GST_OBJECT_NAME(GST_MESSAGE_SRC(msg)), "playbin", 7) == 0)
            {
                MIRACASTLOG_ERROR("Element [%s], Pipeline state change from Old [%s] -> New [%s] and Pending state is [%s]", GST_ELEMENT_NAME(GST_MESSAGE_SRC(msg)),
                                  gst_element_state_get_name(old), gst_element_state_get_name(now), gst_element_state_get_name(pending));
            }
            std::string file_name = "miracast_progressive";
            file_name += (GST_OBJECT_NAME(self->m_pipeline));
            file_name += "_" + std::to_string(id);
            file_name += "_";
            file_name += gst_element_state_get_name(old);
            file_name += "_";
            file_name += gst_element_state_get_name(now);

            GST_DEBUG_BIN_TO_DOT_FILE((GstBin *)self->m_pipeline, GST_DEBUG_GRAPH_SHOW_ALL, file_name.c_str());

            if (self->m_bBuffering)
            {
                self->m_bBuffering = false;
            }
            if (now == GST_STATE_PLAYING && !self->m_bReady)
            {
                self->m_bReady = true;
            }
        }
        break;
    }
    case GST_MESSAGE_BUFFERING:
    {
        /* If the stream is live, no need to do anything. */
        if (self->m_is_live)
        {
            break;
        }
        gint percent = 0;
        gst_message_parse_buffering(msg, &percent);
        MIRACASTLOG_VERBOSE("Buffering [%3d%%].", percent);
        self->m_bBuffering = true;
        /* Wait until buffering is complete before start/resume playing */
        if (percent < 100)
            gst_element_set_state(self->m_pipeline, GST_STATE_PAUSED);
        else
            gst_element_set_state(self->m_pipeline, GST_STATE_PLAYING);
        break;
    }
    case GST_MESSAGE_TAG:
        break;
    case GST_MESSAGE_CLOCK_LOST:
        /* The current clock as selected by the pipeline became unusable, then select a new clock */
        gst_element_set_state(self->m_pipeline, GST_STATE_PAUSED);
        gst_element_set_state(self->m_pipeline, GST_STATE_PLAYING);
        break;
    case GST_MESSAGE_QOS:
    {
        MIRACASTLOG_VERBOSE("Received [%s], a buffer was dropped or an element changed its processing strategy for Quality of Service reasons.", gst_message_type_get_name(msg->type));
        GstFormat format;
        guint64 processed;
        guint64 dropped;
        gst_message_parse_qos_stats(msg, &format, &processed, &dropped);
        MIRACASTLOG_WARNING("Format [%s], Processed [%lu], Dropped [%lu].", gst_format_get_name(format), processed, dropped);

        gint64 jitter;
        gdouble proportion;
        gint quality;
        gst_message_parse_qos_values(msg, &jitter, &proportion, &quality);
        MIRACASTLOG_WARNING("Jitter [%lu], Proportion [%lf],  Quality [%u].", jitter, proportion, quality);

        gboolean live;
        guint64 running_time;
        guint64 stream_time;
        guint64 timestamp;
        guint64 duration;
        gst_message_parse_qos(msg, &live, &running_time, &stream_time, &timestamp, &duration);
        MIRACASTLOG_WARNING("live stream [%d], runninng_time [%lu], stream_time [%lu], timestamp [%lu], duration [%lu].", live, running_time, stream_time, timestamp, duration);
        break;
    }

    default:
        MIRACASTLOG_VERBOSE("Bus msg type: %s\n", gst_message_type_get_name(msg->type));
        break;
    }
    MIRACASTLOG_TRACE("Exiting..!!!");
    return TRUE;
}

double MiracastPlayer::getDuration()
{
    MIRACASTLOG_TRACE("Entering..!!!");
    gint64 duration = 0;
    double ret = 0.0f;
    if (gst_element_query_duration(m_pipeline, GST_FORMAT_TIME, &duration) && GST_CLOCK_TIME_IS_VALID(duration))
    {
        ret = static_cast<double>(duration) / GST_SECOND;
    }
    MIRACASTLOG_TRACE("Exiting..!!!");
    return ret;
}

double MiracastPlayer::getCurrentPosition()
{
    MIRACASTLOG_TRACE("Entering..!!!");
    gint64 position = 0;

    if (gst_element_query_position(m_pipeline, GST_FORMAT_TIME, &position))
    {
        m_currentPosition = static_cast<double>(position) / GST_SECOND;
    }
    MIRACASTLOG_TRACE("Exiting..!!!");
    return m_currentPosition;
}

bool MiracastPlayer::seekTo(double seconds)
{
    MIRACASTLOG_TRACE("Entering..!!!");
    bool ret = false;
    gint64 cur = static_cast<gint64>(seconds * GST_SECOND);
    m_currentPosition = seconds;

    MIRACASTLOG_VERBOSE("seekToPos=%f", seconds);

    if (!gst_element_seek(m_pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, cur, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE))
    {
        MIRACASTLOG_VERBOSE("SeekToPos success\n");
        ret = true;
    }
    else
    {
        MIRACASTLOG_VERBOSE("seekToPos failed");
    }
    MIRACASTLOG_TRACE("Exiting..!!!");
    return ret;
}

void MiracastPlayer::print_pipeline_state()
{
    MIRACASTLOG_TRACE("Entering..!!!");
    if ( nullptr == m_pipeline )
    {
        MIRACASTLOG_ERROR("m_pipeline is NULL. Can't proceed with print_pipeline_state(). \n");
    }
    else
    {
        GstState current, pending;
        GstStateChangeReturn ret_state = GST_STATE_CHANGE_FAILURE;
        current = pending = GST_STATE_VOID_PENDING;

        ret_state = gst_element_get_state(m_pipeline, &current, &pending, 0);
        MIRACASTLOG_VERBOSE("\nPipeline State - Current:[%s], Pending:[%s],Ret:[%d]\n",
                            gst_element_state_get_name(current),gst_element_state_get_name(pending),ret_state);
    }
    MIRACASTLOG_TRACE("Exiting..!!!");
}

bool MiracastPlayer::get_player_statistics()
{
    MIRACASTLOG_TRACE("Entering..!!!");	
    GstStructure *stats = nullptr;
    bool ret = true;

    if (nullptr == m_video_sink )
    {
        MIRACASTLOG_ERROR("video-sink is NULL. Can't proceed with getPlayerStatistics(). \n");
        return false;
    }
    MIRACASTLOG_VERBOSE("\n============= Player Statistics =============\n");

    double cur_position = getCurrentPosition();

    g_object_get( G_OBJECT(m_video_sink), "stats", &stats, nullptr );

    if ( stats )
    {
        const GValue *value = nullptr;
        guint64 render_frame = 0,
                dropped_frame = 0,
                total_video_frames = 0,
                dropped_video_frames = 0;

        /* Get Rendered Frames*/
        value = gst_structure_get_value( stats, (const gchar *)"rendered" );
        if ( value )
        {
           render_frame = g_value_get_uint64( value );
        }
        /* Get Dropped Frames*/
        value = gst_structure_get_value( stats, (const gchar *)"dropped" );
        if ( value )
        {
           dropped_frame = g_value_get_uint64( value );
        }
        gst_structure_free( stats );
        
        total_video_frames = render_frame + dropped_frame;
        dropped_video_frames = dropped_frame;

        MIRACASTLOG_VERBOSE("\nCurrent PTS: [ %f ], Total Frames: [ %lu], Rendered Frames : [ %lu ], Dropped Frames: [%lu]\n",
                            cur_position,
                            total_video_frames,
                            render_frame,
                            dropped_video_frames);
     }
     print_pipeline_state();
     MIRACASTLOG_VERBOSE("\n=============================================\n");
     MIRACASTLOG_TRACE("Exiting..!!!");	
     return ret;
}
