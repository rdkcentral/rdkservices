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
#include "MiracastRTSPMsg.h"
#include "MiracastGstPlayer.h"

MiracastGstPlayer *MiracastGstPlayer::mMiracastGstPlayer{nullptr};

MiracastGstPlayer *MiracastGstPlayer::getInstance()
{
    if (mMiracastGstPlayer == nullptr)
    {
        mMiracastGstPlayer = new MiracastGstPlayer();
    }
    return mMiracastGstPlayer;
}

void MiracastGstPlayer::destroyInstance()
{
    MIRACASTLOG_TRACE("Entering...");
    if (mMiracastGstPlayer != nullptr)
    {
        mMiracastGstPlayer->stop();
        if (mMiracastGstPlayer->stop())
        {
            MIRACASTLOG_INFO("Stop miracast player");
        }
        else
        {
            MIRACASTLOG_ERROR("Failed to stop miracast player");
        }
        delete mMiracastGstPlayer;
        mMiracastGstPlayer = nullptr;
    }
    MIRACASTLOG_TRACE("Exiting...");
}

MiracastGstPlayer::MiracastGstPlayer()
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

MiracastGstPlayer::~MiracastGstPlayer()
{
    MIRACASTLOG_TRACE("Entering...");
    stop();
    MIRACASTLOG_TRACE("Exiting...");
}

bool MiracastGstPlayer::setUri(const std::string ipaddr, const std::string port)
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

std::string MiracastGstPlayer::getUri()
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

bool MiracastGstPlayer::setVideoRectangle( VIDEO_RECT_STRUCT video_rect , bool apply )
{
    bool ret = false;

    MIRACASTLOG_TRACE("Entering...");

    m_video_rect_st.startX = video_rect.startX;
    m_video_rect_st.startY = video_rect.startY;
    m_video_rect_st.width = video_rect.width;
    m_video_rect_st.height = video_rect.height;

    if ( true == apply ){
        updateVideoSinkRectangle();
    }
    ret = true;

    MIRACASTLOG_TRACE("Exiting Coords[%d,%d,%d,%d]Apply[%x]...",
                        video_rect.startX,video_rect.startY,video_rect.width,video_rect.height,
                        apply);

    return ret;
}

bool MiracastGstPlayer::updateVideoSinkRectangle(void)
{
    bool ret = false;

    MIRACASTLOG_TRACE("Entering...");

    if (( nullptr != m_video_sink ) && ( 0 < m_video_rect_st.width ) && ( 0 < m_video_rect_st.height ))
    {
        char rectString[64];
        sprintf(rectString,"%d,%d,%d,%d", m_video_rect_st.startX, m_video_rect_st.startY,
                m_video_rect_st.width, m_video_rect_st.height);
        g_object_set(G_OBJECT(m_video_sink), "window-set", rectString, nullptr);
    }

    MIRACASTLOG_TRACE("Exiting...");

    return ret;
}

bool MiracastGstPlayer::launch(std::string localip , std::string streaming_port, MiracastRTSPMsg *rtsp_instance)
{
    bool ret = false;

    if (setUri(localip, streaming_port))
    {
        m_streaming_port = std::stoull(streaming_port.c_str());
        if ( nullptr != rtsp_instance )
        {
            m_rtsp_reference_instance = rtsp_instance;
        }
        ret = createPipeline();
        if ( !ret ){
            m_rtsp_reference_instance = nullptr;
            MIRACASTLOG_ERROR("Failed to create the pipeline");
        }
    }
    return ret;
}

bool MiracastGstPlayer::pause()
{
    return changePipelineState(GST_STATE_PAUSED);
}

bool MiracastGstPlayer::resume()
{
    return changePipelineState(GST_STATE_PLAYING);
}

bool MiracastGstPlayer::stop()
{
    MIRACASTLOG_TRACE("Entering..");

    if (!m_pipeline)
    {
        MIRACASTLOG_ERROR("Pipeline is NULL");
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

    if (m_audio_sink)
    {
        gst_object_unref(m_audio_sink);
        m_audio_sink = nullptr;
    }
    if (m_audioconvert)
    {
        gst_object_unref(m_audioconvert);
        m_audioconvert = nullptr;
    }
    if (m_avdec_aac)
    {
        gst_object_unref(m_avdec_aac);
        m_avdec_aac = nullptr;
    }
    if (m_aacparse)
    {
        gst_object_unref(m_aacparse);
        m_aacparse = nullptr;
    }
    if (m_aQueue)
    {
        gst_object_unref(m_aQueue);
        m_aQueue = nullptr;
    }
    if (m_video_sink)
    {
        gst_object_unref(m_video_sink);
        m_video_sink = nullptr;
    }
    if (m_h264parse)
    {
        gst_object_unref(m_h264parse);
        m_h264parse = nullptr;
    }
    if (m_vQueue)
    {
        gst_object_unref(m_vQueue);
        m_vQueue = nullptr;
    }

    if (m_tsdemux)
    {
        gst_object_unref(m_tsdemux);
        m_tsdemux = nullptr;
    }
    if (m_tsparse)
    {
        gst_object_unref(m_tsparse);
        m_tsparse = nullptr;
    }

    if (m_rtpmp2tdepay)
    {
        gst_object_unref(m_rtpmp2tdepay);
        m_rtpmp2tdepay = nullptr;
    }
    if (m_rtpjitterbuffer)
    {
        gst_object_unref(m_rtpjitterbuffer);
        m_tsparse = nullptr;
    }
    if (m_udpsrc)
    {
        gst_object_unref(m_udpsrc);
        m_udpsrc = nullptr;
    }

    if (m_main_loop)
    {
        g_main_loop_unref(m_main_loop);
        m_main_loop = nullptr;
    }
    if (m_main_loop_context)
    {
        g_main_context_unref(m_main_loop_context);
        m_main_loop_context = nullptr;
    }

    if (m_pipeline)
    {
        g_object_unref(m_pipeline);
        m_pipeline = nullptr;
    }
    MIRACASTLOG_TRACE("Exiting..");
    return true;
}

bool MiracastGstPlayer::changePipelineState(GstState state) const
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
    MIRACASTLOG_TRACE("Changing state to %s.", gst_element_state_get_name(state));
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

void *MiracastGstPlayer::playbackThread(void *ctx)
{
    MIRACASTLOG_TRACE("Entering..!!!");
    MiracastGstPlayer *self = (MiracastGstPlayer *)ctx;
    g_main_context_push_thread_default(self->m_main_loop_context);
    g_main_loop_run(self->m_main_loop);
    self->m_playback_thread = 0;
    MIRACASTLOG_TRACE("Exiting..!!!");
    pthread_exit(nullptr);
}

void* MiracastGstPlayer::monitor_player_statistics_thread(void *ctx)
{
    bool isInteger = true;
    MIRACASTLOG_TRACE("Entering..!!!");
    MiracastGstPlayer *self = (MiracastGstPlayer *)ctx;
    
    int time_interval_sec = 60;
    while (true)
    {
        isInteger = true;
        time_interval_sec = 120;
        std::ifstream player_stats_monitor_delay("/opt/miracast_player_stats"); // Assuming the input file is named "input.txt"

        if (!player_stats_monitor_delay)
        {
            MIRACASTLOG_TRACE("Statistics Disabled as '/opt/miracast_player_stats' file not exists");
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

double MiracastGstPlayer::getDuration( GstElement *pipeline )
{
    MIRACASTLOG_TRACE("Entering..!!!");
    gint64 duration = 0;
    double ret = 0.0f;

    if ( nullptr == pipeline )
    {
        pipeline = m_pipeline;
    }

    if (gst_element_query_duration(pipeline, GST_FORMAT_TIME, &duration) && GST_CLOCK_TIME_IS_VALID(duration))
    {
        ret = static_cast<double>(duration) / GST_SECOND;
    }
    MIRACASTLOG_TRACE("Exiting..!!!");
    return ret;
}

double MiracastGstPlayer::getCurrentPosition(GstElement *pipeline)
{
    MIRACASTLOG_TRACE("Entering..!!!");
    gint64 position = 0;

    if ( nullptr == pipeline )
    {
        pipeline = m_pipeline;
    }

    if (gst_element_query_position(pipeline, GST_FORMAT_TIME, &position))
    {
        position = static_cast<double>(position) / GST_SECOND;
    }
    MIRACASTLOG_TRACE("Exiting..!!!");
    return position;
}

bool MiracastGstPlayer::seekTo(double seconds, GstElement *pipeline )
{
    MIRACASTLOG_TRACE("Entering..!!!");
    bool ret = false;
    gint64 cur = static_cast<gint64>(seconds * GST_SECOND);
    m_currentPosition = seconds;

    MIRACASTLOG_VERBOSE("seekToPos=%f", seconds);

    if ( nullptr == pipeline )
    {
        pipeline = m_pipeline;
    }

    if (!gst_element_seek(pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, cur, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE))
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

void MiracastGstPlayer::print_pipeline_state(GstElement *pipeline)
{
    MIRACASTLOG_TRACE("Entering..!!!");

    if ( nullptr == pipeline )
    {
        MIRACASTLOG_ERROR("pipeline is NULL. Can't proceed with print_pipeline_state(). \n");
    }
    else
    {
        GstState current, pending;
        GstStateChangeReturn ret_state = GST_STATE_CHANGE_FAILURE;
        current = pending = GST_STATE_VOID_PENDING;

        ret_state = gst_element_get_state(pipeline, &current, &pending, 0);
        MIRACASTLOG_VERBOSE("\n[%s]Pipeline State - Current:[%s], Pending:[%s],Ret:[%d]\n",
                            gst_element_get_name(pipeline),
                            gst_element_state_get_name(current),
                            gst_element_state_get_name(pending),
                            ret_state);
    }
    MIRACASTLOG_TRACE("Exiting..!!!");
}

bool MiracastGstPlayer::get_player_statistics()
{
    MIRACASTLOG_TRACE("Entering..!!!");	
    GstStructure *stats = nullptr;
    bool ret = true;

    if (nullptr == m_video_sink )
    {
        MIRACASTLOG_ERROR("video-sink is NULL. Can't proceed with getPlayerStatistics(). \n");
        return false;
    }
    MIRACASTLOG_INFO("============= Player Statistics =============");

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
           MIRACASTLOG_TRACE("!!!! render_frame[%lu] !!!",render_frame);
        }
        /* Get Dropped Frames*/
        value = gst_structure_get_value( stats, (const gchar *)"dropped" );
        if ( value )
        {
           dropped_frame = g_value_get_uint64( value );
           MIRACASTLOG_TRACE("!!!! dropped_frame[%lu] !!!",dropped_frame);
        }
        
        total_video_frames = render_frame + dropped_frame;
        dropped_video_frames = dropped_frame;

        MIRACASTLOG_INFO("Current PTS: [ %f ]",cur_position);
        MIRACASTLOG_INFO("Total Frames: [ %lu], Rendered Frames : [ %lu ], Dropped Frames: [%lu]",
                            total_video_frames,
                            render_frame,
                            dropped_video_frames);
        gst_structure_free( stats );
     }
    print_pipeline_state(m_pipeline);
    MIRACASTLOG_INFO("\n=============================================\n");
    MIRACASTLOG_TRACE("Exiting..!!!");	
    return ret;
}

gboolean MiracastGstPlayer::busMessageCb(GstBus *bus, GstMessage *msg, gpointer userdata)
{
    MiracastGstPlayer *self = static_cast<MiracastGstPlayer*>(userdata);

    MIRACASTLOG_TRACE("Entering...\n");
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
            GST_DEBUG_BIN_TO_DOT_FILE((GstBin *)self->m_pipeline, GST_DEBUG_GRAPH_SHOW_ALL, "miracast_player_error");
            break;
        }
        case GST_MESSAGE_EOS:
        {
            MIRACASTLOG_VERBOSE("!!!!GST_MESSAGE_EOS reached !!!!");
            gst_element_set_state(self->m_pipeline, GST_STATE_READY); // TBD ? (Should be do explicitly or destry automatically.)
            g_main_loop_quit(self->m_main_loop);
        }
        break;
        case GST_MESSAGE_STATE_CHANGED:
        {
            GstState old, now, pending;
            gst_message_parse_state_changed(msg, &old, &now, &pending);
            MIRACASTLOG_VERBOSE("[GST_MESSAGE_STATE_CHANGED] Element [%s], Pipeline state change from Old [%s] -> New [%s] and Pending state is [%s]",
                                        GST_ELEMENT_NAME(GST_MESSAGE_SRC(msg)),
                                        gst_element_state_get_name(old),
                                        gst_element_state_get_name(now),
                                        gst_element_state_get_name(pending));

            if (GST_MESSAGE_SRC(msg) == GST_OBJECT(self->m_pipeline))
            {
                static int id = 0;
                id++;
                #if 0
                gst_message_parse_state_changed(msg, &old, &now, &pending);
                if (memcmp(GST_OBJECT_NAME(GST_MESSAGE_SRC(msg)), "miracast_player", strlen("miracast_player")) == 0)
                {
                    MIRACASTLOG_INFO("Element [%s], Pipeline state change from Old [%s] -> New [%s] and Pending state is [%s]",
                                        GST_ELEMENT_NAME(GST_MESSAGE_SRC(msg)),
                                        gst_element_state_get_name(old),
                                        gst_element_state_get_name(now),
                                        gst_element_state_get_name(pending));
                }
                #endif
                std::string file_name = "miracast_player_";
                file_name += (GST_OBJECT_NAME(self->m_pipeline));
                file_name += "_" + std::to_string(id);
                file_name += "_";
                file_name += gst_element_state_get_name(old);
                file_name += "_";
                file_name += gst_element_state_get_name(now);

                GST_DEBUG_BIN_TO_DOT_FILE((GstBin *)self->m_pipeline, GST_DEBUG_GRAPH_SHOW_ALL, file_name.c_str());
            }
            break;
        }
        case GST_MESSAGE_BUFFERING:
        {
            gint percent = 0;
            gst_message_parse_buffering(msg, &percent);
            MIRACASTLOG_VERBOSE("Buffering [%3d%%].", percent);

            /* Wait until buffering is complete before start/resume playing */
            if (percent < 100)
            {
                gst_element_set_state(self->m_pipeline, GST_STATE_PAUSED);
            }
            else
            {
                gst_element_set_state(self->m_pipeline, GST_STATE_PLAYING);
            }
        }
        break;
        case GST_MESSAGE_TAG:
        {
            MIRACASTLOG_VERBOSE("!!!! GST_MESSAGE_TAG !!!!");
        }
        break;
        case GST_MESSAGE_CLOCK_LOST:
        {
            MIRACASTLOG_VERBOSE("!!!! GST_MESSAGE_CLOCK_LOST !!!!");
            /* The current clock as selected by the pipeline became unusable, then select a new clock */
            gst_element_set_state(self->m_pipeline, GST_STATE_PAUSED);
            gst_element_set_state(self->m_pipeline, GST_STATE_PLAYING);
        }
        break;
        case GST_MESSAGE_QOS:
        {
            MIRACASTLOG_VERBOSE("Received [%s], a buffer was dropped or an element changed its processing strategy for Quality of Service reasons.", gst_message_type_get_name(msg->type));
            GstFormat format;
            guint64 processed;
            guint64 dropped;
            gst_message_parse_qos_stats(msg, &format, &processed, &dropped);
            MIRACASTLOG_VERBOSE("Format [%s], Processed [%lu], Dropped [%lu].", gst_format_get_name(format), processed, dropped);

            gint64 jitter;
            gdouble proportion;
            gint quality;
            gst_message_parse_qos_values(msg, &jitter, &proportion, &quality);
            MIRACASTLOG_VERBOSE("Jitter [%lu], Proportion [%lf],  Quality [%u].", jitter, proportion, quality);

            gboolean live;
            guint64 running_time;
            guint64 stream_time;
            guint64 timestamp;
            guint64 duration;
            gst_message_parse_qos(msg, &live, &running_time, &stream_time, &timestamp, &duration);
            MIRACASTLOG_VERBOSE("live stream [%d], runninng_time [%lu], stream_time [%lu], timestamp [%lu], duration [%lu].", live, running_time, stream_time, timestamp, duration);
        }
        break;
        default:
        {
        }
        break;
    }
    MIRACASTLOG_TRACE("Exiting...\n");
    return TRUE;
}

void MiracastGstPlayer::pad_added_handler(GstElement *gstelement, GstPad *new_pad, gpointer userdata)
{
    MiracastGstPlayer *self = static_cast<MiracastGstPlayer*>(userdata);
    GstCaps *new_pad_caps = NULL;
    GstStructure *new_pad_struct = NULL;

    MIRACASTLOG_TRACE("Entering..!!!");

    if (!self->m_pipeline)
    {
        MIRACASTLOG_ERROR("failed to link elements and Exiting...!!!");
        gst_object_unref(self->m_pipeline);
        self->m_pipeline = NULL;
        return;
    }

    /* Check the new pad's type */
    new_pad_caps = gst_pad_get_current_caps(new_pad);
    new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);

    char *pad_name = (char *)gst_structure_get_name(new_pad_struct);

    MIRACASTLOG_TRACE("Pad Name: %s", pad_name);

    if (strncmp(pad_name, "audio", strlen("audio")) == 0)
    {
        GstElement *sink = (GstElement *)self->m_aQueue;
        GstPad *sinkpad = gst_element_get_static_pad(sink, "sink");
        bool linked = GST_PAD_LINK_SUCCESSFUL(gst_pad_link(new_pad, sinkpad));
        if (!linked)
        {
            MIRACASTLOG_ERROR("Failed to link demux and audio pad (%s)", pad_name);
        }
        else
        {
            MIRACASTLOG_VERBOSE("Configured audio pad");
        }
        gst_object_unref(sinkpad);
    }
    else if (strncmp(pad_name, "video", strlen("video")) == 0)
    {
        GstElement *sink = (GstElement *)self->m_vQueue;
        GstPad *sinkpad = gst_element_get_static_pad(sink, "sink");
        bool linked = GST_PAD_LINK_SUCCESSFUL(gst_pad_link(new_pad, sinkpad));
        if (!linked)
        {
            MIRACASTLOG_ERROR("Failed to link demux and video pad (%s)", pad_name);
        }
        else
        {
            MIRACASTLOG_VERBOSE("Configured video pad");
        }
        gst_object_unref(sinkpad);
    }
    MIRACASTLOG_TRACE("Exiting..!!!");
}

/**
 * @brief Callback invoked after first video frame decoded
 * @param[in] object pointer to element raising the callback
 * @param[in] arg0 number of arguments
 * @param[in] arg1 array of arguments
 * @param[in] _this pointer to MiracastGstPlayer instance
 */
void MiracastGstPlayer::onFirstVideoFrameCallback(GstElement* object, guint arg0, gpointer arg1,gpointer userdata)
{
    MIRACASTLOG_TRACE("Entering..!!!");
    MiracastGstPlayer *self = static_cast<MiracastGstPlayer*>(userdata);
        self->m_firstVideoFrameReceived = true;
    MIRACASTLOG_INFO("!!! First Video Frame has received !!!");
    self->notifyPlaybackState(MIRACAST_GSTPLAYER_STATE_FIRST_VIDEO_FRAME_RECEIVED);
    MIRACASTLOG_TRACE("Exiting..!!!");
}

void MiracastGstPlayer::notifyPlaybackState(eMIRA_GSTPLAYER_STATES gst_player_state)
{
    MIRACASTLOG_TRACE("Entering..!!!");
    if ( nullptr != m_rtsp_reference_instance )
    {
        RTSP_HLDR_MSGQ_STRUCT rtsp_hldr_msgq_data = {0};

        rtsp_hldr_msgq_data.state = RTSP_NOTIFY_GSTPLAYER_STATE;
        rtsp_hldr_msgq_data.gst_player_state = gst_player_state;
        MIRACASTLOG_INFO("!!! GstPlayer to RTSP [%#08X] !!!",gst_player_state);
        m_rtsp_reference_instance->send_msgto_rtsp_msg_hdler_thread(rtsp_hldr_msgq_data);
    }
    MIRACASTLOG_TRACE("Exiting..!!!");
}

bool MiracastGstPlayer::createPipeline()
{
    MIRACASTLOG_TRACE("Entering..!!!");
    GstStateChangeReturn ret;
    GstBus *bus = nullptr;
    bool return_value = true;

    /* create gst pipeline */
    m_main_loop_context = g_main_context_new();
    g_main_context_push_thread_default(m_main_loop_context);
    m_main_loop = g_main_loop_new(m_main_loop_context, FALSE);

    MIRACASTLOG_TRACE("Creating Pipeline...");

    m_pipeline = gst_pipeline_new("miracast_player");
    if (!m_pipeline)
    {
        MIRACASTLOG_ERROR("Failed to create gstreamer pipeline");
        MIRACASTLOG_TRACE("Exiting..!!!");
        return false;
    }

    m_udpsrc = gst_element_factory_make("udpsrc", "udpsrc");

    m_rtpjitterbuffer = gst_element_factory_make("rtpjitterbuffer", "rtpjitterbuffer");
    m_rtpmp2tdepay = gst_element_factory_make("rtpmp2tdepay", "rtpmp2tdepay");

    m_tsparse = gst_element_factory_make("tsparse", "tsparse");
    //GstElement *m_Queue2 = gst_element_factory_make("queue2", "queue2");
    m_tsdemux = gst_element_factory_make("tsdemux", "tsdemux");

    m_vQueue = gst_element_factory_make("queue", "vQueue");
    m_h264parse = gst_element_factory_make("h264parse", "h264parse");
    m_video_sink = gst_element_factory_make("westerossink", "westerossink");

    m_aQueue = gst_element_factory_make("queue", "aQueue");
    m_aacparse = gst_element_factory_make("aacparse", "aacparse");
    m_avdec_aac = gst_element_factory_make("avdec_aac", "avdec_aac");
    m_audioconvert = gst_element_factory_make("audioconvert", "audioconvert");
    m_audio_sink = gst_element_factory_make("amlhalasink", "amlhalasink");

    if ((!m_udpsrc) || (!m_rtpjitterbuffer) || (!m_rtpmp2tdepay)||
        (!m_tsparse) || (!m_tsdemux) ||
        (!m_vQueue) || (!m_h264parse) || (!m_video_sink) ||
        (!m_aQueue) || (!m_aacparse) || (!m_avdec_aac) || (!m_audioconvert) || (!m_audio_sink))
    {
        MIRACASTLOG_ERROR("Element creation failure, check below");
        MIRACASTLOG_WARNING("udpsrc[%x]rtpjitterbuffer[%x]rtpmp2tdepay[%x]",
                            m_udpsrc,m_rtpjitterbuffer,m_rtpmp2tdepay);
        MIRACASTLOG_WARNING("tsparse[%x]tsdemux[%x]",
                            m_tsparse,m_tsdemux);
        MIRACASTLOG_WARNING("vQueue[%x]h264parse[%x]videoSink[%x]",
                            m_vQueue,m_h264parse,m_video_sink);
        MIRACASTLOG_WARNING("aQueue[%x]aacparse[%x]avdec_aac[%x]audioconvert[%x]audioSink[%x]",
                            m_aQueue,m_aacparse,m_avdec_aac,m_audioconvert,m_audio_sink);
        MIRACASTLOG_TRACE("Exiting..!!!");
        return false;
    }
    MIRACASTLOG_INFO("Add all the elements to the Pipeline ");

    /* Add all the elements into the pipeline */
    gst_bin_add_many(GST_BIN(m_pipeline), 
                        m_udpsrc, 
                        m_rtpjitterbuffer,m_rtpmp2tdepay,
                        m_tsparse,m_tsdemux,
                        m_vQueue,m_h264parse,m_video_sink,
                        m_aQueue,m_aacparse,m_avdec_aac,m_audioconvert,m_audio_sink,
                        nullptr );

    MIRACASTLOG_TRACE("Link all the elements together. ");

    /* Link the elements together */
    if (!gst_element_link_many(m_udpsrc, m_rtpjitterbuffer,m_rtpmp2tdepay,m_tsparse,m_tsdemux,nullptr ))
    {
        MIRACASTLOG_ERROR("Elements (udpsrc->rtpjitterbuffer->rtpmp2tdepay->tsparse->tsdemux) could not be linked");
        gst_object_unref(m_pipeline);
        MIRACASTLOG_TRACE("Exiting..!!!");
        return false;
    }

    if (!gst_element_link_many(m_vQueue,m_h264parse,m_video_sink, nullptr))
    {
        MIRACASTLOG_ERROR("Elements (vQueue->h264parse->westerossink) could not be linked");
        gst_object_unref(m_pipeline);
        MIRACASTLOG_TRACE("Exiting..!!!");
        return false;
    }

    if (!gst_element_link_many(m_aQueue,m_aacparse,m_avdec_aac,m_audioconvert,m_audio_sink, nullptr))
    {
        MIRACASTLOG_ERROR("Elements (aQueue->aacparse->avdec_aac->amlhalasink) could not be linked");
        gst_object_unref(m_pipeline);
        MIRACASTLOG_TRACE("Exiting..!!!");
        return false;
    }

    /*{{{ udpsrc related element configuration*/
    MIRACASTLOG_TRACE(">>>>>>>udpsrc configuration start");
    MIRACASTLOG_TRACE("Set the port[%llu] and to udp source.",m_streaming_port);
    g_object_set(G_OBJECT(m_udpsrc), "port", m_streaming_port, nullptr);
    //g_object_set(G_OBJECT(m_udpsrc), "uri", m_uri.c_str(), nullptr);

    GstCaps *caps = gst_caps_new_simple("application/x-rtp", "media", G_TYPE_STRING, "video", nullptr);
    if (caps)
    {
        g_object_set(m_udpsrc, "caps", caps, nullptr);
        gst_caps_unref(caps);
        MIRACASTLOG_TRACE("Set the caps to udp source.");
    }
    else
    {
        MIRACASTLOG_ERROR("Unable to Set caps to udp source.");
    }
    MIRACASTLOG_TRACE("udpsrc configuration end<<<<<<<<");
    /*}}}*/

    /*{{{ rtpjitterbuffer related element configuration*/
    MIRACASTLOG_TRACE(">>>>>>>rtpjitterbuffer configuration start");
    MIRACASTLOG_TRACE("Set the 'post-drop-messages' and 'do-lost' to rtpjitterbuffer.");
    g_object_set(G_OBJECT(m_rtpjitterbuffer), 
                    "post-drop-messages", true, "do-lost" , true , nullptr );
    MIRACASTLOG_TRACE("rtpjitterbuffer configuration end<<<<<<<<");
    /*}}}*/

    /*{{{ tsparse related element configuration*/
    MIRACASTLOG_TRACE(">>>>>>>tsparse configuration start");
    MIRACASTLOG_TRACE("Set 'set-timestamps' to tsparse");
    g_object_set(G_OBJECT(m_tsparse), "set-timestamps", true, nullptr );
    MIRACASTLOG_TRACE("tsparse configuration end<<<<<<<<");
    /*}}}*/

    /*{{{ tsdemux related element configuration*/
    MIRACASTLOG_TRACE(">>>>>>>tsdemux configuration start");
    MIRACASTLOG_TRACE("Connect to the pad-added signal for tsdemux");
    g_signal_connect(m_tsdemux, "pad-added", G_CALLBACK(pad_added_handler), (gpointer)this);
    MIRACASTLOG_TRACE("tsdemux configuration end<<<<<<<<");
    /*}}}*/

    /*{{{ vQueue related element configuration*/
    MIRACASTLOG_TRACE(">>>>>>>vQueue configuration start");
    uint64_t    vQ_max_size_buffers = 2;
    MIRACASTLOG_TRACE("set 'max-size-buffers' as 2 in videoQueue");
    g_object_set(G_OBJECT(m_vQueue), "max-size-buffers", vQ_max_size_buffers, nullptr );
    MIRACASTLOG_TRACE("vQueue configuration end<<<<<<<<");
    /*}}}*/

    /*{{{ westerossink related element configuration*/
    MIRACASTLOG_TRACE(">>>>>>>westerossink configuration start");
    updateVideoSinkRectangle();

    g_signal_connect(m_video_sink, "first-video-frame-callback",G_CALLBACK(onFirstVideoFrameCallback), (gpointer)this);
    MIRACASTLOG_TRACE("westerossink configuration end<<<<<<<<");
    /*}}}*/

    /*{{{ aQueue related element configuration*/
    uint64_t    aQ_max_size_buffers = 2,
                aQ_max_size_time = 0;
    MIRACASTLOG_TRACE(">>>>>>>aQueue configuration start");
    MIRACASTLOG_TRACE("set 'max-size-buffers' as 2 and max-size-time as '0' in audioQueue");
    g_object_set(G_OBJECT(m_aQueue), "max-size-buffers", aQ_max_size_buffers, nullptr );
    g_object_set(G_OBJECT(m_aQueue), "max-size-time", aQ_max_size_time , nullptr );
    MIRACASTLOG_TRACE("aQueue configuration end<<<<<<<<");
    /*}}}*/

    /*{{{ amlhalasink related element configuration*/
    MIRACASTLOG_TRACE(">>>>>>>amlhalasink configuration start");
    
    MIRACASTLOG_TRACE("Set disable-xrun as true to amlhalasink");
    g_object_set(G_OBJECT(m_audio_sink), "disable-xrun" , true, nullptr );

    std::string opt_flag_buffer = MiracastCommon::parse_opt_flag("/opt/miracast_avoid_amlhalasink_iptv_mode");
    if (opt_flag_buffer.empty())
    {
        MIRACASTLOG_INFO("[DEFAULT] Set avsync-mode as 2(IPTV) to amlhalasink");
        g_object_set(G_OBJECT(m_audio_sink), "avsync-mode" , 2, nullptr );
    }
    MIRACASTLOG_TRACE("amlhalasink configuration end<<<<<<<<");
    /*}}}*/

    MIRACASTLOG_TRACE("Listen to the bus.");
    /* Listen to the bus */
    bus = gst_element_get_bus(m_pipeline);
    gst_bus_add_watch(bus, (GstBusFunc)busMessageCb, this );
    gst_object_unref(bus);

    MIRACASTLOG_TRACE("Start Playing....");

    g_main_context_pop_thread_default(m_main_loop_context);
    pthread_create(&m_playback_thread, nullptr, MiracastGstPlayer::playbackThread, this);
    pthread_create(&m_player_statistics_tid, nullptr, MiracastGstPlayer::monitor_player_statistics_thread, this);

    ret = gst_element_set_state(m_pipeline, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        MIRACASTLOG_ERROR("Unable to set the pipeline to the playing state.");
        if(m_audio_sink) gst_object_unref(m_audio_sink);
        if(m_audioconvert) gst_object_unref(m_audioconvert);
        if(m_avdec_aac) gst_object_unref(m_avdec_aac);
        if(m_aacparse) gst_object_unref(m_aacparse);
        if(m_aQueue) gst_object_unref(m_aQueue);

        if(m_video_sink) gst_object_unref(m_video_sink);
        if(m_h264parse) gst_object_unref(m_h264parse);
        if(m_vQueue) gst_object_unref(m_vQueue);

        if(m_tsdemux) gst_object_unref(m_tsdemux);
        if(m_tsparse) gst_object_unref(m_tsparse);

        if(m_rtpmp2tdepay) gst_object_unref(m_rtpmp2tdepay);
        if(m_rtpjitterbuffer) gst_object_unref(m_rtpjitterbuffer);
        if(m_udpsrc) gst_object_unref(m_udpsrc);

        if(m_pipeline) gst_object_unref(m_pipeline);
        return_value = false;
    }
    else if (ret == GST_STATE_CHANGE_NO_PREROLL)
    {
        MIRACASTLOG_TRACE("Streaming live");
        m_is_live = true;
    }

    MIRACASTLOG_TRACE("Exiting..!!!");
    return return_value;
}
