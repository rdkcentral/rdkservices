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
#include <gst/app/gstappsrc.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sys/types.h>
#include <sys/syscall.h>
#include "MiracastLogger.h"
#include "MiracastRTSPMsg.h"
#include "MiracastGstPlayer.h"
#include <SoC_MiracastPlayer.h>

MiracastGstPlayer *MiracastGstPlayer::m_GstPlayer{nullptr};

MiracastGstPlayer *MiracastGstPlayer::getInstance()
{
    if (m_GstPlayer == nullptr)
    {
        m_GstPlayer = new MiracastGstPlayer();
    }
    return m_GstPlayer;
}

void MiracastGstPlayer::destroyInstance()
{
    MIRACASTLOG_TRACE("Entering...");
    if (m_GstPlayer != nullptr)
    {
        m_GstPlayer->stop();
        if (m_GstPlayer->stop())
        {
            MIRACASTLOG_INFO("Stop miracast player");
        }
        else
        {
            MIRACASTLOG_ERROR("Failed to stop miracast player");
        }
        delete m_GstPlayer;
        m_GstPlayer = nullptr;
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
    SoC_ConfigureVideoDecodeErrorPolicy();
    MIRACASTLOG_TRACE("Exiting...");
}

MiracastGstPlayer::~MiracastGstPlayer()
{
    MIRACASTLOG_TRACE("Entering...");
    stop();
    MIRACASTLOG_TRACE("Exiting...");
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

bool MiracastGstPlayer::launch(std::string& localip , std::string& streaming_port, MiracastRTSPMsg *rtsp_instance)
{
    char urlBuffer[128] = {0};
    bool ret = false;

    MIRACASTLOG_TRACE("Entering...");

    snprintf(urlBuffer,sizeof(urlBuffer),"udp://%s:%s",localip.c_str(),streaming_port.c_str());
    m_uri = urlBuffer;

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
    MIRACASTLOG_TRACE("Exiting...");
    return ret;
}

bool MiracastGstPlayer::pause()
{
    changePipelineState(m_append_pipeline,GST_STATE_PAUSED);
    changePipelineState(m_playbin_pipeline,GST_STATE_PAUSED);
    return true;
}

bool MiracastGstPlayer::resume()
{
    changePipelineState(m_append_pipeline,GST_STATE_PLAYING);
    changePipelineState(m_playbin_pipeline,GST_STATE_PLAYING);
    return true;
}

bool MiracastGstPlayer::changePipelineState(GstElement *pipeline, GstState state) const
{
    MIRACASTLOG_TRACE("Entering..!!!");
    GstStateChangeReturn ret;
    bool status = false;
    GstState current, pending;
    current = pending = GST_STATE_VOID_PENDING;
    ret = gst_element_get_state(pipeline, &current, &pending, 0);

    if ((ret != GST_STATE_CHANGE_FAILURE) && (current == state || pending == state))
    {
        status = true;
    }
    MIRACASTLOG_TRACE("Changing state to %s.", gst_element_state_get_name(state));
    if (gst_element_set_state(pipeline, state) != GST_STATE_CHANGE_FAILURE)
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
    MiracastGstPlayer *self = (MiracastGstPlayer *)ctx;
    int elapsed_seconds = 0,
        stats_timeout = 0;
    MIRACASTLOG_TRACE("Entering..!!!");
    self->m_statistics_thread_loop = true;
    struct timespec start_time, current_time;
    std::string opt_flag_buffer = "";

    clock_gettime(CLOCK_REALTIME, &start_time);
    while (true == self->m_statistics_thread_loop)
    {
        clock_gettime(CLOCK_REALTIME, &current_time);
        opt_flag_buffer = MiracastCommon::parse_opt_flag("/opt/miracast_player_stats",true,false);

        if (!opt_flag_buffer.empty())
        {
            stats_timeout = std::atoi(opt_flag_buffer.c_str());
            elapsed_seconds = current_time.tv_sec - start_time.tv_sec;
            if (elapsed_seconds >= stats_timeout)
            {
                self->get_player_statistics();
                // Refresh the Statistics time
                clock_gettime(CLOCK_REALTIME, &start_time);
            }
        }
        usleep(100000);
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

    if ( nullptr != pipeline )
    {
        if (gst_element_query_duration(pipeline, GST_FORMAT_TIME, &duration) && GST_CLOCK_TIME_IS_VALID(duration))
        {
            ret = static_cast<double>(duration) / GST_SECOND;
        }
    }
    MIRACASTLOG_TRACE("Exiting..!!!");
    return ret;
}

double MiracastGstPlayer::getCurrentPosition(GstElement *pipeline)
{
    MIRACASTLOG_TRACE("Entering..!!!");
    gint64 position = 0;

    if ( nullptr != pipeline )
    {
        if (gst_element_query_position(pipeline, GST_FORMAT_TIME, &position))
        {
            position = static_cast<double>(position) / GST_SECOND;
        }
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

    if ( nullptr != pipeline )
    {
        if (!gst_element_seek(pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, cur, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE))
        {
            MIRACASTLOG_VERBOSE("SeekToPos success");
            ret = true;
        }
        else
        {
            MIRACASTLOG_VERBOSE("seekToPos failed");
        }
    }
    MIRACASTLOG_TRACE("Exiting..!!!");
    return ret;
}

void MiracastGstPlayer::print_pipeline_state(GstElement *pipeline)
{
    MIRACASTLOG_TRACE("Entering..!!!");

    if ( nullptr == pipeline )
    {
        MIRACASTLOG_ERROR("pipeline is NULL. Can't proceed with print_pipeline_state(). ");
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
        MIRACASTLOG_ERROR("video-sink is NULL. Can't proceed with getPlayerStatistics(). ");
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
    print_pipeline_state(m_append_pipeline);
    print_pipeline_state(m_playbin_pipeline);
    MIRACASTLOG_INFO("\n=============================================");
    MIRACASTLOG_TRACE("Exiting..!!!");	
    return ret;
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

void MiracastGstPlayer::notifyPlaybackState(eMIRA_GSTPLAYER_STATES gst_player_state, eM_PLAYER_REASON_CODE state_reason_code )
{
    MIRACASTLOG_TRACE("Entering..!!!");
    if ( nullptr != m_rtsp_reference_instance )
    {
        RTSP_HLDR_MSGQ_STRUCT rtsp_hldr_msgq_data = {0};

        rtsp_hldr_msgq_data.state = RTSP_NOTIFY_GSTPLAYER_STATE;
        rtsp_hldr_msgq_data.gst_player_state = gst_player_state;
        rtsp_hldr_msgq_data.state_reason_code = state_reason_code;
        MIRACASTLOG_INFO("!!! GstPlayer to RTSP [%#08X] !!!",gst_player_state);
        m_rtsp_reference_instance->send_msgto_rtsp_msg_hdler_thread(rtsp_hldr_msgq_data);
    }
    MIRACASTLOG_TRACE("Exiting..!!!");
}

GstFlowReturn MiracastGstPlayer::appendPipelineNewSampleHandler(GstElement *elt, gpointer userdata)
{
    MiracastGstPlayer *self = static_cast<MiracastGstPlayer*>(userdata);
    GstSample *sample = NULL;
    GstBuffer *buffer = NULL;

    if (nullptr == self->m_appsrc)
    {
        MIRACASTLOG_WARNING("Yet to get the Appsrc handle");
        return GST_FLOW_OK;
    }

    // Pull the sample from appsink
    sample = gst_app_sink_pull_sample(GST_APP_SINK(elt));
    if (!sample)
    {
        MIRACASTLOG_ERROR("Failed to pull sample from appsink\n");
        return GST_FLOW_ERROR;
    }

    // Get the buffer from the sample
    buffer = gst_sample_get_buffer(sample);
    if (!buffer)
    {
        MIRACASTLOG_ERROR("Failed to get buffer from sample\n");
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

#if 0
    GstMapInfo map;

    // Map the buffer for reading
    if (!gst_buffer_map(buffer, &map, GST_MAP_READ))
    {
        MIRACASTLOG_ERROR("Failed to map buffer\n");
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    GstBuffer *new_buffer = gst_buffer_new_allocate(NULL, map.size, NULL);
    // Copy data from the original buffer to the new buffer
    gst_buffer_fill(new_buffer, 0, map.data, map.size);
    MIRACASTLOG_INFO("==> Received sample size [%u][%x] <==",map.size,new_buffer);
    // Unmap and cleanup
    gst_buffer_unmap(buffer, &map);
#else
    //MIRACASTLOG_INFO("Sending GstBuffer to Q");
    gst_buffer_ref(buffer);
    self->m_customQueueHandle->sendData(static_cast<void*>(buffer));
#endif
    gst_sample_unref(sample);

    return GST_FLOW_OK;
}

/* called when we get a GstMessage from the source pipeline when we get EOS, we
 * notify the appsrc of it. */
gboolean MiracastGstPlayer::appendPipelineBusMessage(GstBus * bus, GstMessage * message, gpointer userdata)
{
    MiracastGstPlayer *self = static_cast<MiracastGstPlayer*>(userdata);
    GstElement *source;
    MIRACASTLOG_TRACE("Entering...");
    switch (GST_MESSAGE_TYPE (message))
    {
        case GST_MESSAGE_EOS:
        {
            MIRACASTLOG_INFO ("The source got dry");
            source = gst_bin_get_by_name (GST_BIN (self->m_append_pipeline), "miracast_appsink");
            gst_app_src_end_of_stream (GST_APP_SRC (source));
            gst_object_unref (source);
        }
        break;
        case GST_MESSAGE_ERROR:
        {
            GError *error;
            gchar *info;
            gst_message_parse_error(message, &error, &info);
            MIRACASTLOG_ERROR("#### GST-FAIL Error received from element [%s | %s | %s] ####", GST_OBJECT_NAME(message->src), error->message, info ? info : "none");
            g_error_free(error);
            g_free(info);
            GST_DEBUG_BIN_TO_DOT_FILE((GstBin *)self->m_append_pipeline, GST_DEBUG_GRAPH_SHOW_ALL, "miracast_udpsrc2appsink_error");
            gst_element_set_state(self->m_append_pipeline, GST_STATE_READY);
            self->notifyPlaybackState(MIRACAST_GSTPLAYER_STATE_STOPPED,MIRACAST_PLAYER_REASON_CODE_GST_ERROR);
        }
        break;
        case GST_MESSAGE_STATE_CHANGED:
        {
            GstState old, now, pending;
            gst_message_parse_state_changed(message, &old, &now, &pending);
            MIRACASTLOG_VERBOSE("[GST_MESSAGE_STATE_CHANGED] Element [%s], Pipeline state change from Old [%s] -> New [%s] and Pending state is [%s]",
                                        GST_ELEMENT_NAME(GST_MESSAGE_SRC(message)),
                                        gst_element_state_get_name(old),
                                        gst_element_state_get_name(now),
                                        gst_element_state_get_name(pending));

            if (GST_MESSAGE_SRC(message) == GST_OBJECT(self->m_append_pipeline))
            {
                char fileName[128] = {0};
                static int playbin_id = 0;
                playbin_id++;
                snprintf( fileName,
                            sizeof(fileName),
                            "MiracastUdpsrcAppsink_%s_%s_%s_%s_DBG",
                            GST_OBJECT_NAME(self->m_append_pipeline),
                            std::to_string(playbin_id).c_str(),
                            gst_element_state_get_name(old),
                            gst_element_state_get_name(now));
                GST_DEBUG_BIN_TO_DOT_FILE((GstBin *)self->m_append_pipeline, GST_DEBUG_GRAPH_SHOW_ALL, fileName);
            }
        }
        break;
        default:
            break;
    }
    MIRACASTLOG_TRACE("Exiting...");
    return TRUE;
}

/* called when we get a GstMessage from the sink pipeline when we get EOS, we
 * exit the mainloop and this testapp. */
gboolean MiracastGstPlayer::playbinPipelineBusMessage (GstBus * bus, GstMessage * message, gpointer userdata)
{
    MiracastGstPlayer *self = static_cast<MiracastGstPlayer*>(userdata);

    switch (GST_MESSAGE_TYPE(message))
    {
        case GST_MESSAGE_EOS:
        case GST_MESSAGE_ERROR:
        {
            if (GST_MESSAGE_EOS == GST_MESSAGE_TYPE(message))
            {
                MIRACASTLOG_INFO("Finished playback");
            }
            else
            {
                GError *error;
                gchar *info;
                gst_message_parse_error(message, &error, &info);
                MIRACASTLOG_ERROR("#### GST-FAIL Error received from element [%s | %s | %s] ####", GST_OBJECT_NAME(message->src), error->message, info ? info : "none");
                g_error_free(error);
                g_free(info);
            }
            GST_DEBUG_BIN_TO_DOT_FILE((GstBin *)self->m_playbin_pipeline, GST_DEBUG_GRAPH_SHOW_ALL, "miracast_playbin2appSrc_error");
            self->notifyPlaybackState(MIRACAST_GSTPLAYER_STATE_STOPPED,MIRACAST_PLAYER_REASON_CODE_GST_ERROR);
        }
        break;
        case GST_MESSAGE_STATE_CHANGED:
        {
            GstState old, now, pending;
            gst_message_parse_state_changed(message, &old, &now, &pending);
            MIRACASTLOG_VERBOSE("[GST_MESSAGE_STATE_CHANGED] Element [%s], Pipeline state change from Old [%s] -> New [%s] and Pending state is [%s]",
                                        GST_ELEMENT_NAME(GST_MESSAGE_SRC(message)),
                                        gst_element_state_get_name(old),
                                        gst_element_state_get_name(now),
                                        gst_element_state_get_name(pending));

            if (GST_MESSAGE_SRC(message) == GST_OBJECT(self->m_playbin_pipeline))
            {
                char fileName[128] = {0};
                static int playbin_id = 0;
                playbin_id++;
                snprintf( fileName,
                            sizeof(fileName),
                            "MiracastPlaybinAppsrc_%s_%s_%s_%s_DBG",
                            GST_OBJECT_NAME(self->m_playbin_pipeline),
                            std::to_string(playbin_id).c_str(),
                            gst_element_state_get_name(old),
                            gst_element_state_get_name(now));
                GST_DEBUG_BIN_TO_DOT_FILE((GstBin *)self->m_playbin_pipeline, GST_DEBUG_GRAPH_SHOW_ALL, fileName);
            }
        }
        break;
        case GST_MESSAGE_BUFFERING:
        {
            gint percent = 0;
            gst_message_parse_buffering(message, &percent);
            MIRACASTLOG_VERBOSE("Buffering [%3d%%].", percent);

            /* Wait until buffering is complete before start/resume playing */
            if (percent < 100)
            {
                gst_element_set_state(self->m_playbin_pipeline, GST_STATE_PAUSED);
            }
            else
            {
                gst_element_set_state(self->m_playbin_pipeline, GST_STATE_PLAYING);
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
            gst_element_set_state(self->m_playbin_pipeline, GST_STATE_PAUSED);
            gst_element_set_state(self->m_playbin_pipeline, GST_STATE_PLAYING);
        }
        break;
        case GST_MESSAGE_QOS:
        {
            MIRACASTLOG_VERBOSE("Received [%s], a buffer was dropped or an element changed its processing strategy for Quality of Service reasons.", gst_message_type_get_name(message->type));
            GstFormat format;
            guint64 processed;
            guint64 dropped;
            gst_message_parse_qos_stats(message, &format, &processed, &dropped);
            MIRACASTLOG_VERBOSE("Format [%s], Processed [%lu], Dropped [%lu].", gst_format_get_name(format), processed, dropped);

            gint64 jitter;
            gdouble proportion;
            gint quality;
            gst_message_parse_qos_values(message, &jitter, &proportion, &quality);
            MIRACASTLOG_VERBOSE("Jitter [%lu], Proportion [%lf],  Quality [%u].", jitter, proportion, quality);

            gboolean live;
            guint64 running_time;
            guint64 stream_time;
            guint64 timestamp;
            guint64 duration;
            gst_message_parse_qos(message, &live, &running_time, &stream_time, &timestamp, &duration);
            MIRACASTLOG_VERBOSE("live stream [%d], runninng_time [%lu], stream_time [%lu], timestamp [%lu], duration [%lu].", live, running_time, stream_time, timestamp, duration);
        }
        break;
        default:
            break;
    }
    return TRUE;
}

gboolean MiracastGstPlayer::pushBufferToAppsrc(gpointer userdata)
{
    MiracastGstPlayer *self = (MiracastGstPlayer *)userdata;
    MIRACASTLOG_TRACE("Entering..!!!");
    void* buffer = nullptr;
    GstBuffer *gstBuffer = nullptr;

    self->m_customQueueHandle->ReceiveData(buffer);

    gstBuffer = static_cast<GstBuffer*>(buffer);

    if (nullptr != gstBuffer)
    {
        //MIRACASTLOG_INFO("Sending GstBuffer to Appsrc");
        // Push the new buffer to appsrc
        GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(self->m_appsrc), gstBuffer);
        if (ret != GST_FLOW_OK)
        {
            MIRACASTLOG_ERROR("Error pushing buffer to appsrc");
        }
    }
    MIRACASTLOG_TRACE("Exiting..!!!");
    return TRUE;
}

// Module functions
void MiracastGstPlayer::gst_bin_need_data(GstAppSrc *src, guint length, gpointer userdata)
{
    MIRACASTLOG_TRACE("Entering...");
    MiracastGstPlayer *self = static_cast<MiracastGstPlayer*>(userdata);
    MIRACASTLOG_TRACE("AppSrc empty");
    if ((self->m_sourceId == 0) && (false == self->m_destroyTimer))
    {
        //MIRACASTLOG_INFO("start feeding\n");
        //self->m_sourceId = g_idle_add((GSourceFunc)pushBufferToAppsrc, self);
    }
    MIRACASTLOG_TRACE("Exiting...");
    return;
}

void MiracastGstPlayer::gst_bin_enough_data(GstAppSrc *src, gpointer userdata)
{
    MiracastGstPlayer *self = static_cast<MiracastGstPlayer*>(userdata);
    MIRACASTLOG_INFO("AppSrc Full!!!!");
    if (self->m_sourceId != 0)
    {
        MIRACASTLOG_INFO("stop feeding\n");
        g_source_remove(self->m_sourceId);
        self->m_sourceId = 0;
    }
    return;
}

/* This function is called when playbin2 has created the appsrc element, so we have
 * a chance to configure it. */
void MiracastGstPlayer::source_setup(GstElement *pipeline, GstElement *source, gpointer userdata)
{
    MiracastGstPlayer *self = static_cast<MiracastGstPlayer*>(userdata);
    MIRACASTLOG_INFO("Entering...");
    MIRACASTLOG_INFO("Source has been created. Configuring [%x]",source);
    self->m_appsrc = source;
    // Set AppSrc parameters
    GstAppSrcCallbacks callbacks = {gst_bin_need_data, gst_bin_enough_data, NULL};
    gst_app_src_set_callbacks(GST_APP_SRC(self->m_appsrc), &callbacks, (gpointer)(self), NULL);
    g_object_set(GST_APP_SRC(self->m_appsrc), "max-bytes", (guint64) 20 * 1024 * 1024, NULL);

    g_object_set(GST_APP_SRC(self->m_appsrc), "format", GST_FORMAT_TIME, NULL);
    g_object_set(GST_APP_SRC(self->m_appsrc), "is-live", true, NULL);
    const gchar *set_cap = "video/mpegts, systemstream=(boolean)true, packetsize=(int)188";
    GstCaps *caps = gst_caps_from_string (set_cap);
    g_object_set(GST_APP_SRC(self->m_appsrc), "caps", caps, NULL);
    if(caps) {
      self->m_capsSrc = caps;
    }
    MIRACASTLOG_INFO("Exiting... ");
}

void MiracastGstPlayer::gstBufferReleaseCallback(void* userParam)
{
    GstBuffer *gstBuffer;
    gstBuffer = static_cast<GstBuffer*>(userParam);

    if (nullptr != gstBuffer)
    {
        MIRACASTLOG_INFO("gstBuffer[%x]",gstBuffer);
        gst_buffer_unref(gstBuffer);
    }
}

bool MiracastGstPlayer::createPipeline()
{
    MIRACASTLOG_TRACE("Entering..!!!");
    GstStateChangeReturn ret;
    GstBus *bus = nullptr;
    bool return_value = true;
    m_customQueueHandle = new MessageQueue(500,gstBufferReleaseCallback);

    if (nullptr == m_customQueueHandle)
    {
        MIRACASTLOG_ERROR("Failed to create MessageQueue");
        return false;
    }

    /* create gst pipeline */
    m_main_loop_context = g_main_context_new();
    g_main_context_push_thread_default(m_main_loop_context);
    m_main_loop = g_main_loop_new(m_main_loop_context, FALSE);

    MIRACASTLOG_INFO("Creating Pipeline...");

    // Create a new pipeline
    m_append_pipeline = gst_pipeline_new("miracast_data_collector");
    // Create elements
    m_udpsrc = gst_element_factory_make("udpsrc", "miracast_udpsrc");
    m_rtpjitterbuffer = gst_element_factory_make("rtpjitterbuffer", "miracast_rtpjitterbuffer");
    m_rtpmp2tdepay = gst_element_factory_make("rtpmp2tdepay", "miracast_rtpmp2tdepay");
    m_tsparse = gst_element_factory_make("tsparse", "miracast_tsparse");
    m_appsink = gst_element_factory_make("appsink", "miracast_appsink");
    m_video_sink = gst_element_factory_make("westerossink", "miracast_westerossink");
    m_audio_sink = SoC_GetAudioSinkProperty();

    if (!m_append_pipeline || !m_udpsrc || !m_rtpjitterbuffer || !m_rtpmp2tdepay ||
        !m_tsparse || !m_appsink || !m_video_sink )
    {
        MIRACASTLOG_ERROR("Append Pipeline[%x]: Element creation failure, check below",m_append_pipeline);
        MIRACASTLOG_WARNING("udpsrc[%x]rtpjitterbuffer[%x]rtpmp2tdepay[%x]",m_udpsrc,m_rtpjitterbuffer,m_rtpmp2tdepay);
        MIRACASTLOG_WARNING("tsparse[%x]appsink[%x]videosink[%x]audiosink[%x]",
                            m_tsparse,m_appsink,m_video_sink,m_audio_sink);
        return -1;
    }

    /*{{{ udpsrc related element configuration*/
    MIRACASTLOG_TRACE(">>>>>>>udpsrc configuration start");
    MIRACASTLOG_TRACE("Set the port[%llu] and to udp source.",m_streaming_port);
    g_object_set(G_OBJECT(m_udpsrc), "port", m_streaming_port, nullptr);

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
    g_object_set(G_OBJECT(m_rtpjitterbuffer), "post-drop-messages", true, "do-lost" , true , nullptr );
    std::string opt_flag_buffer = MiracastCommon::parse_opt_flag("/opt/miracast_faststart-min-packets",true,false);

    if (!opt_flag_buffer.empty())
    {
        uint64_t packetsPerBuffer = std::stoull(opt_flag_buffer);
        MIRACASTLOG_INFO("Set 'faststart-min-packets' to rtpjitterbuffer");
        g_object_set(G_OBJECT(m_rtpjitterbuffer), "faststart-min-packets", packetsPerBuffer, nullptr );
    }
    MIRACASTLOG_TRACE("rtpjitterbuffer configuration end<<<<<<<<");
    
    /*}}}*/

    /*{{{ tsparse related element configuration*/
    MIRACASTLOG_TRACE(">>>>>>>tsparse configuration start");
    MIRACASTLOG_TRACE("Set 'set-timestamps' to tsparse");
    g_object_set(G_OBJECT(m_tsparse), "set-timestamps", true, nullptr );
    opt_flag_buffer = MiracastCommon::parse_opt_flag("/opt/miracast_tsparse",true,false);

    if (!opt_flag_buffer.empty())
    {
        uint64_t packetsPerBuffer = std::stoull(opt_flag_buffer);
        MIRACASTLOG_INFO("Set 'alignment' to tsparse");
        g_object_set(G_OBJECT(m_tsparse), "alignment", packetsPerBuffer, nullptr );
    }
    MIRACASTLOG_TRACE("tsparse configuration end<<<<<<<<");
    /*}}}*/

    /* to be notified of messages from this pipeline, mostly EOS */
    bus = gst_element_get_bus(m_append_pipeline);
    gst_bus_add_watch(bus, (GstBusFunc)appendPipelineBusMessage, this);
    gst_object_unref(bus);
    
    /*{{{ appsink related element configuration*/
    MIRACASTLOG_TRACE(">>>>>>>appsink configuration start");
    // Configure the appsink
    g_object_set(G_OBJECT(m_appsink), "emit-signals", TRUE, "sync", FALSE, NULL);
    g_object_set(G_OBJECT(m_appsink), "async", FALSE, NULL);
    // Set up a signal handler for new buffer signals from appsink
    g_signal_connect(G_OBJECT(m_appsink), "new-sample", G_CALLBACK(appendPipelineNewSampleHandler), this);
    MIRACASTLOG_TRACE("appsink configuration end<<<<<<<<");
    /*}}}*/

    // Add elements to the pipeline
    gst_bin_add_many(GST_BIN(m_append_pipeline), 
                        m_udpsrc,
                        m_rtpjitterbuffer,
                        m_rtpmp2tdepay,
                        m_tsparse,
                        m_appsink,
                        nullptr );

    if (!gst_element_link_many(m_udpsrc,
                                m_rtpjitterbuffer,
                                m_rtpmp2tdepay,
                                m_tsparse,
                                m_appsink,
                                nullptr ))
    {
        MIRACASTLOG_ERROR("Elements (udpsrc->rtpjitterbuffer->queue->rtpmp2tdepay->tsparse->appsink) could not be linked");
        gst_object_unref(m_append_pipeline);
        return -1;
    }

    // Set up pipeline
    m_playbin_pipeline = gst_element_factory_make("playbin", "miracast_playbin");
    if (!m_playbin_pipeline)
    {
        MIRACASTLOG_ERROR( "Failed to create pipeline.");
    }
    else
    {
        gint flags;

        /* Read the state of the current flags */
	    g_object_get(m_playbin_pipeline, "flags", &flags, nullptr);
        MIRACASTLOG_INFO("playbin flags1: 0x%x", flags);

        //flags = GST_PLAY_FLAG_VIDEO | GST_PLAY_FLAG_AUDIO | GST_PLAY_FLAG_NATIVE_AUDIO | GST_PLAY_FLAG_NATIVE_VIDEO; // AudioSink not linked
        flags = GST_PLAY_FLAG_VIDEO | GST_PLAY_FLAG_AUDIO | GST_PLAY_FLAG_NATIVE_VIDEO;
        MIRACASTLOG_INFO("playbin new flags: 0x%x", flags);

        g_object_set(m_playbin_pipeline, "flags", flags, nullptr);

        bus = gst_element_get_bus (m_playbin_pipeline);
        gst_bus_add_watch (bus, (GstBusFunc) playbinPipelineBusMessage, this);
        gst_object_unref (bus);
        // Pipeline created
        g_object_set(m_playbin_pipeline, "uri", "appsrc://", nullptr);

        g_signal_connect(m_playbin_pipeline, "source-setup", G_CALLBACK(source_setup), this);
        
        /*{{{ westerossink related element configuration*/
        MIRACASTLOG_TRACE(">>>>>>>westerossink configuration start");
        updateVideoSinkRectangle();

        g_signal_connect(m_video_sink, "first-video-frame-callback",G_CALLBACK(onFirstVideoFrameCallback), (gpointer)this);
        MIRACASTLOG_TRACE("westerossink configuration end<<<<<<<<");
        g_object_set(m_playbin_pipeline, "video-sink", m_video_sink, nullptr);
        /*}}}*/

	if (m_audio_sink)
        {
	    g_object_set(m_playbin_pipeline, "audio-sink", m_audio_sink, nullptr);
        }
    }

    g_main_context_pop_thread_default(m_main_loop_context);
    pthread_create(&m_playback_thread, nullptr, MiracastGstPlayer::playbackThread, this);
    pthread_create(&m_player_statistics_tid, nullptr, MiracastGstPlayer::monitor_player_statistics_thread, this);

    /* launching things */
    MIRACASTLOG_INFO("m_playbin_pipeline, GST_STATE_PLAYING");
    ret = gst_element_set_state(m_playbin_pipeline, GST_STATE_PLAYING);
    ret = gst_element_set_state(m_append_pipeline, GST_STATE_PLAYING);
    m_sourceId = g_idle_add((GSourceFunc)pushBufferToAppsrc, this);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        MIRACASTLOG_ERROR("Unable to set the pipeline to the playing state.");
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

bool MiracastGstPlayer::stop()
{
    GstStateChangeReturn ret;
    MIRACASTLOG_TRACE("Entering..");

    if (!m_playbin_pipeline)
    {
        MIRACASTLOG_ERROR("Pipeline is NULL");
        return false;
    }
    m_destroyTimer = true;
    if (m_sourceId != 0)
    {
        MIRACASTLOG_INFO("remove Timer");
        g_source_remove(m_sourceId);
        m_sourceId = 0;
    }
    ret = gst_element_set_state(m_playbin_pipeline, GST_STATE_NULL);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        MIRACASTLOG_ERROR("Failed to set gst_element_set_state as NULL");
    }
    ret = gst_element_set_state(m_append_pipeline, GST_STATE_NULL);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        MIRACASTLOG_ERROR("Failed to set gst_element_set_state as NULL");
    }

    if (m_main_loop)
    {
        g_main_loop_quit(m_main_loop);
    }
    if (m_playback_thread)
    {
        pthread_join(m_playback_thread,nullptr);
    }
    if (m_player_statistics_tid)
    {
        m_statistics_thread_loop = false;
        pthread_join(m_player_statistics_tid,nullptr);
    }
    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_append_pipeline));
    if (bus)
    {
        gst_bus_set_sync_handler(bus, nullptr, nullptr, nullptr);
        gst_object_unref(bus);
    }

    bus = gst_pipeline_get_bus(GST_PIPELINE(m_playbin_pipeline));
    if (bus)
    {
        gst_bus_set_sync_handler(bus, nullptr, nullptr, nullptr);
        gst_object_unref(bus);
    }

    if (m_audio_sink)
    {
        SoC_ReleaseAudioSinkProperty(m_audio_sink);
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

    if (m_tsparse)
    {
        gst_bin_remove(GST_BIN(m_append_pipeline), m_tsparse);
        gst_object_unref(m_tsparse);
        m_tsparse = nullptr;
    }
    if (m_rtpmp2tdepay)
    {
        gst_bin_remove(GST_BIN(m_append_pipeline), m_rtpmp2tdepay);
        gst_object_unref(m_rtpmp2tdepay);
        m_rtpmp2tdepay = nullptr;
    }
    if (m_tsdemux)
    {
        gst_bin_remove(GST_BIN(m_append_pipeline), m_tsdemux);
        gst_object_unref(m_tsdemux);
        m_tsdemux = nullptr;
    }
    if (m_rtpjitterbuffer)
    {
        gst_bin_remove(GST_BIN(m_append_pipeline), m_rtpjitterbuffer);
        gst_object_unref(m_rtpjitterbuffer);
        m_tsparse = nullptr;
    }
    if (m_udpsrc)
    {
        gst_bin_remove(GST_BIN(m_append_pipeline), m_udpsrc);
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
    if (m_append_pipeline)
    {
        g_object_unref(m_append_pipeline);
        m_append_pipeline = nullptr;
    }
    if (m_playbin_pipeline)
    {
        g_object_unref(m_playbin_pipeline);
        m_playbin_pipeline = nullptr;
    }
    if (m_capsSrc)
    {
        gst_caps_unref(m_capsSrc);
        m_capsSrc = nullptr;
    }
    if (m_customQueueHandle)
    {
        delete m_customQueueHandle;
        m_customQueueHandle = nullptr;
    }
    MIRACASTLOG_TRACE("Exiting..");
    return true;
}
