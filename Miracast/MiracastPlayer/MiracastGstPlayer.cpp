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
            MIRACASTLOG_ERROR("Stop miracast player");
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
    m_elts = G_QUEUE_INIT;
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
        //g_object_set(G_OBJECT(m_video_sink), "rectangle", rectString, nullptr);
        g_object_set(G_OBJECT(m_video_sink), "window-set", rectString, nullptr);
    }

    MIRACASTLOG_TRACE("Exiting...");

    return ret;
}

bool MiracastGstPlayer::launch(std::string localip , std::string streaming_port)
{
    bool ret = false;

    if (setUri(localip, streaming_port))
    {
        ret = createPipeline();
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

#if 0
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
#else
bool MiracastGstPlayer::stop()
{
    MIRACASTLOG_TRACE("Entering..");

    if (!m_udpsrc2appsink_pipeline || !m_playbin2appsrc_pipeline)
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
        m_playback_thread = 0;
    }

    GstStateChangeReturn ret;
    ret = gst_element_set_state(m_playbin2appsrc_pipeline, GST_STATE_NULL);
    ret = gst_element_set_state(m_udpsrc2appsink_pipeline, GST_STATE_NULL);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        MIRACASTLOG_ERROR("Failed to set gst_element_set_state as NULL");
    }

    GstBus *bus = nullptr;

    bus = gst_pipeline_get_bus(GST_PIPELINE(m_playbin2appsrc_pipeline));
    if (bus)
    {
        gst_bus_set_sync_handler(bus, nullptr, nullptr, nullptr);
        gst_object_unref(bus);
    }

    bus = gst_pipeline_get_bus(GST_PIPELINE(m_udpsrc2appsink_pipeline));
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
    if (m_playbin2appsrc_pipeline)
    {
        g_object_unref(m_playbin2appsrc_pipeline);
        m_playbin2appsrc_pipeline = nullptr;
    }

    if (m_udpsrc2appsink_pipeline)
    {
        g_object_unref(m_udpsrc2appsink_pipeline);
        m_udpsrc2appsink_pipeline = nullptr;
    }
    MIRACASTLOG_TRACE("Exiting..");
    return true;
}
#endif

bool MiracastGstPlayer::changePipelineState(GstState state) const
{
    MIRACASTLOG_TRACE("Entering..!!!");
    GstStateChangeReturn ret;
    bool status = false;
    GstState current, pending;
    current = pending = GST_STATE_VOID_PENDING;
    ret = gst_element_get_state(m_playbin2appsrc_pipeline, &current, &pending, 0);

    if ((ret != GST_STATE_CHANGE_FAILURE) && (current == state || pending == state))
    {
        status = true;
    }
    MIRACASTLOG_INFO("[PLAYBIN2APPSRC]Changing state to %s.", gst_element_state_get_name(state));
    if (gst_element_set_state(m_playbin2appsrc_pipeline, state) != GST_STATE_CHANGE_FAILURE)
    {
        status = true;
    }
    else
    {
        status = false;
    }

    ret = gst_element_get_state(m_udpsrc2appsink_pipeline, &current, &pending, 0);

    if ((ret != GST_STATE_CHANGE_FAILURE) && (current == state || pending == state))
    {
        status = true;
    }
    MIRACASTLOG_INFO("[UDPSRC2APPSINK]Changing state to %s.", gst_element_state_get_name(state));
    if (gst_element_set_state(m_udpsrc2appsink_pipeline, state) != GST_STATE_CHANGE_FAILURE)
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

std::string MiracastGstPlayer::parse_opt_flag( std::string file_name , bool integer_check )
{
    std::string return_buffer = "";
    std::ifstream parse_opt_flag_file( file_name.c_str());

    if (!parse_opt_flag_file)
    {
        MIRACASTLOG_ERROR("Failed to open [%s] file\n",file_name.c_str());
    }
    else
    {
        std::string word;
        parse_opt_flag_file >> word;
        parse_opt_flag_file.close();

        return_buffer = word;

        if (integer_check)
        {
            if (word.empty())
            {
                integer_check = false;
            }
            else
            {
                for (char c : word) {
                    if (!isdigit(c))
                    {
                        integer_check = false;
                        break;
                    }
                }
            }

            if ( false == integer_check )
            {
                return_buffer = "";
            }
        }
    }
    return return_buffer;
}

void MiracastGstPlayer::element_setup(GstElement * playbin, GstElement * element, GQueue * elts)
{
    GstElementFactory *eltfact = gst_element_get_factory (element);

    MIRACASTLOG_TRACE("eltfact: [%x]\n",eltfact);
    MIRACASTLOG_TRACE("name: [%s]\n",eltfact ? GST_OBJECT_NAME (eltfact) : GST_OBJECT_NAME (element));

    if ( nullptr != eltfact && ( 0 == g_strcmp0(GST_OBJECT_NAME (eltfact),"tsdemux")))
    {
        g_object_set(G_OBJECT(eltfact), "ignore-pcr", true , nullptr);
        MIRACASTLOG_INFO("set property ignore-pcr to true\n");
    }
    else if (( nullptr != eltfact ) &&
            (( 0 == g_strcmp0(GST_OBJECT_NAME (eltfact),"multiqueue"))||
            ( 0 == g_strcmp0(GST_OBJECT_NAME (eltfact),"queue"))))
    {
        std::string opt_flag_buffer = "";
        uint64_t    max_size_buffers = 0,
                    max_size_bytes = 0,
                    max_size_time = 0;

        opt_flag_buffer = parse_opt_flag( "/opt/miracast_max-size-buffers" , true );
        if (!opt_flag_buffer.empty())
        {
            max_size_buffers = std::stoull(opt_flag_buffer.c_str());
            g_object_set(G_OBJECT(eltfact), "max-size-buffers", max_size_buffers , nullptr);
            MIRACASTLOG_INFO("set max-size-buffers to [%llu]\n",max_size_buffers);
        }

        opt_flag_buffer = parse_opt_flag( "/opt/miracast_max-size-bytes" , true );
        if (!opt_flag_buffer.empty())
        {
            max_size_bytes = std::stoull(opt_flag_buffer.c_str());
            g_object_set(G_OBJECT(eltfact), "max-size-bytes", max_size_bytes , nullptr);
            MIRACASTLOG_INFO("set max-size-bytes to [%llu]\n",max_size_bytes);
        }

        opt_flag_buffer = parse_opt_flag( "/opt/miracast_max-size-time" , true );
        if (!opt_flag_buffer.empty())
        {
            max_size_time = std::stoull(opt_flag_buffer.c_str());
            g_object_set(G_OBJECT(eltfact), "max-size-time", max_size_time , nullptr);
            MIRACASTLOG_INFO("set max-size-time to [%llu]\n",max_size_time);
        }
    }
    g_queue_push_tail (elts, eltfact ? GST_OBJECT_NAME (eltfact) : GST_OBJECT_NAME (element));
}

gboolean MiracastGstPlayer::on_udpsrc2appsink_bus_message(GstBus *bus, GstMessage *msg, gpointer user_data)
{
    MiracastGstPlayer *self = static_cast<MiracastGstPlayer *>(user_data);

    //MIRACASTLOG_TRACE("Entering...\n");
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
            GST_DEBUG_BIN_TO_DOT_FILE((GstBin *)self->m_udpsrc2appsink_pipeline, GST_DEBUG_GRAPH_SHOW_ALL, "miracast_udpsrc2appsink_error");
            gst_element_set_state(self->m_udpsrc2appsink_pipeline, GST_STATE_PAUSED);
            gst_element_set_state(self->m_udpsrc2appsink_pipeline, GST_STATE_PLAYING);
            //gst_element_set_state(self->m_udpsrc2appsink_pipeline, GST_STATE_READY);
            //g_main_loop_quit(self->m_main_loop);
            break;
        }
        case GST_MESSAGE_EOS:
        {
            MIRACASTLOG_INFO("End-Of-Stream reached.");
            self->m_bBuffering = false;
            gst_element_set_state(self->m_udpsrc2appsink_pipeline, GST_STATE_READY); // TBD ? (Should be do explicitly or destry automatically.)
            g_main_loop_quit(self->m_main_loop);
        }
        break;
        case GST_MESSAGE_STATE_CHANGED:
        {
            MIRACASTLOG_INFO("!!!! GST_MESSAGE_STATE_CHANGED !!!!\n");
            if (GST_MESSAGE_SRC(msg) == GST_OBJECT(self->m_udpsrc2appsink_pipeline))
            {
                GstState old, now, pending;
                static int id = 0;
                id++;
                gst_message_parse_state_changed(msg, &old, &now, &pending);
                if (memcmp(GST_OBJECT_NAME(GST_MESSAGE_SRC(msg)), "miracast_udpsrc2appsink" , strlen("miracast_udpsrc2appsink")) == 0)
                {
                    MIRACASTLOG_INFO("Element [%s], Pipeline state change from Old [%s] -> New [%s] and Pending state is [%s]", GST_ELEMENT_NAME(GST_MESSAGE_SRC(msg)),
                                    gst_element_state_get_name(old), gst_element_state_get_name(now), gst_element_state_get_name(pending));
                }
                std::string file_name = "miracast_udpsrc2appsink_";
                file_name += (GST_OBJECT_NAME(self->m_udpsrc2appsink_pipeline));
                file_name += "_" + std::to_string(id);
                file_name += "_";
                file_name += gst_element_state_get_name(old);
                file_name += "_";
                file_name += gst_element_state_get_name(now);

                GST_DEBUG_BIN_TO_DOT_FILE((GstBin *)self->m_udpsrc2appsink_pipeline, GST_DEBUG_GRAPH_SHOW_ALL, file_name.c_str());
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
                gst_element_set_state(self->m_udpsrc2appsink_pipeline, GST_STATE_PAUSED);
            else
                gst_element_set_state(self->m_udpsrc2appsink_pipeline, GST_STATE_PLAYING);
            break;
        }
        case GST_MESSAGE_TAG:
        {
            MIRACASTLOG_VERBOSE("Received GST_MESSAGE_TAG");
        }
        break;
        case GST_MESSAGE_CLOCK_LOST:
        {
            MIRACASTLOG_VERBOSE("Received GST_MESSAGE_CLOCK_LOST");
            /* The current clock as selected by the pipeline became unusable, then select a new clock */
            gst_element_set_state(self->m_udpsrc2appsink_pipeline, GST_STATE_PAUSED);
            gst_element_set_state(self->m_udpsrc2appsink_pipeline, GST_STATE_PLAYING);
        }
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
        break;
        default:
        {
        }
        break;
    }
    MIRACASTLOG_TRACE("Exiting...\n");
    return TRUE;
}

gboolean MiracastGstPlayer::on_playbin2appsrc_bus_message(GstBus *bus, GstMessage *msg, gpointer user_data)
{
    MiracastGstPlayer *self = static_cast<MiracastGstPlayer *>(user_data);

    //MIRACASTLOG_TRACE("Entering...\n");
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
            GST_DEBUG_BIN_TO_DOT_FILE((GstBin *)self->m_playbin2appsrc_pipeline, GST_DEBUG_GRAPH_SHOW_ALL, "miracast_playbin2appsrc_error");
            gst_element_set_state(self->m_playbin2appsrc_pipeline, GST_STATE_PAUSED);
            gst_element_set_state(self->m_playbin2appsrc_pipeline, GST_STATE_PLAYING);
            //gst_element_set_state(self->m_playbin2appsrc_pipeline, GST_STATE_READY);
            //g_main_loop_quit(self->m_main_loop);
            break;
        }
        case GST_MESSAGE_EOS:
        {
            MIRACASTLOG_INFO("!!!!GST_MESSAGE_EOS reached !!!!");
            self->m_bBuffering = false;
            gst_element_set_state(self->m_playbin2appsrc_pipeline, GST_STATE_READY); // TBD ? (Should be do explicitly or destry automatically.)
            g_main_loop_quit(self->m_main_loop);
        }
        break;
        case GST_MESSAGE_STATE_CHANGED:
        {
            MIRACASTLOG_INFO("!!!! GST_MESSAGE_STATE_CHANGED !!!!\n");
            if (GST_MESSAGE_SRC(msg) == GST_OBJECT(self->m_playbin2appsrc_pipeline))
            {
                GstState old, now, pending;
                static int id = 0;
                id++;
                gst_message_parse_state_changed(msg, &old, &now, &pending);
                if (memcmp(GST_OBJECT_NAME(GST_MESSAGE_SRC(msg)), "miracast_playbin", strlen("miracast_playbin")) == 0)
                {
                    MIRACASTLOG_INFO("Element [%s], Pipeline state change from Old [%s] -> New [%s] and Pending state is [%s]",
                                        GST_ELEMENT_NAME(GST_MESSAGE_SRC(msg)),
                                        gst_element_state_get_name(old),
                                        gst_element_state_get_name(now),
                                        gst_element_state_get_name(pending));
                }
                std::string file_name = "miracast_playbin2appsrc_";
                file_name += (GST_OBJECT_NAME(self->m_playbin2appsrc_pipeline));
                file_name += "_" + std::to_string(id);
                file_name += "_";
                file_name += gst_element_state_get_name(old);
                file_name += "_";
                file_name += gst_element_state_get_name(now);

                GST_DEBUG_BIN_TO_DOT_FILE((GstBin *)self->m_playbin2appsrc_pipeline, GST_DEBUG_GRAPH_SHOW_ALL, file_name.c_str());
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
                gst_element_set_state(self->m_playbin2appsrc_pipeline, GST_STATE_PAUSED);
            else
                gst_element_set_state(self->m_playbin2appsrc_pipeline, GST_STATE_PLAYING);
        }
        break;
        case GST_MESSAGE_TAG:
            break;
        case GST_MESSAGE_CLOCK_LOST:
        {
            /* The current clock as selected by the pipeline became unusable, then select a new clock */
            gst_element_set_state(self->m_playbin2appsrc_pipeline, GST_STATE_PAUSED);
            gst_element_set_state(self->m_playbin2appsrc_pipeline, GST_STATE_PLAYING);
        }
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
        }
        break;
        default:
        {
        }
        break;
    }
    //MIRACASTLOG_TRACE("Exiting...\n");
    return TRUE;
}

void MiracastGstPlayer::appsrc_need_data(GstAppSrc *src, guint length, gpointer user_data)
{
    MIRACASTLOG_TRACE("Entering...\n");
    MiracastGstPlayer *self = static_cast<MiracastGstPlayer *>(user_data);
    MIRACASTLOG_INFO("AppSrc empty\n");
    self->bPushData = true;
    MIRACASTLOG_TRACE("Exiting...\n");
    return;
}

void MiracastGstPlayer::appsrc_enough_data(GstAppSrc *src, gpointer user_data)
{
    MIRACASTLOG_TRACE("Entering...\n");
    MiracastGstPlayer *self = static_cast<MiracastGstPlayer *>(user_data);
    MIRACASTLOG_INFO("!!!!! AppSrc Full !!!!\n");
    self->bPushData = false;
    MIRACASTLOG_TRACE("Exiting...\n");
    return;
}

/* This function is called when playbin2 has created the appsrc element, so we have
 * a chance to configure it. */
void MiracastGstPlayer::playbin_source_setup(GstElement *pipeline, GstElement *source, gpointer user_data)
{
    MiracastGstPlayer *self = static_cast<MiracastGstPlayer *>(user_data);
    MIRACASTLOG_TRACE("Entering...\n");
    MIRACASTLOG_INFO("Source has been created. Configuring.\n");

    self->m_appsrc = source;

    // Set AppSrc parameters
    GstAppSrcCallbacks callbacks = {appsrc_need_data, appsrc_enough_data, nullptr};
    gst_app_src_set_callbacks(GST_APP_SRC(self->m_appsrc), &callbacks, user_data , nullptr);
    g_object_set(GST_APP_SRC(self->m_appsrc), "format", GST_FORMAT_TIME, nullptr);
    g_object_set(GST_APP_SRC(self->m_appsrc), "is-live", true, nullptr);

    std::string opt_max_bytes = "";
    opt_max_bytes = parse_opt_flag( "/opt/miracast_appsrc_max_bytes" , true );

    guint64 test_max_size = 3*1024*1024;

    if (!opt_max_bytes.empty())
    {
        test_max_size = std::stoull(opt_max_bytes.c_str());
    }
    g_object_set(GST_APP_SRC(self->m_appsrc), "max-bytes", (guint64) test_max_size, nullptr);

    const gchar *set_cap = "video/mpegts, systemstream=(boolean)true, packetsize=(int)188";
    GstCaps *caps = gst_caps_from_string(set_cap);
    if(caps)
    {
        g_object_set(GST_APP_SRC(self->m_appsrc), "caps", caps, nullptr);
        gst_caps_unref(caps);
    }
    MIRACASTLOG_TRACE("Exiting...\n");
}

void MiracastGstPlayer::queue_callback(GstElement* object, gpointer user_data)
{
    if ( nullptr != user_data )
    {
        MIRACASTLOG_INFO("queue [ %s ]", (const char *)user_data);
    }
    else
    {
        MIRACASTLOG_ERROR("queue [!!! userdata NULL !!!]");
    }
}

void MiracastGstPlayer::configure_queue(GstElement * queue)
{
    static const bool enable_queue_signals = false;

    g_object_set(G_OBJECT(queue), "min-threshold-time", (guint64)(1*1000*1000*1000), nullptr);
    guint64 limit = 5*1000*1000*1000UL;
    g_object_set(G_OBJECT(queue), "max-size-time", limit, nullptr);
    g_object_set(G_OBJECT(queue), "max-size-buffers", 0, nullptr);
    g_object_set(G_OBJECT(queue), "max-size-bytes", 0, nullptr);

    if(enable_queue_signals)
    {
        g_signal_connect(queue, "underrun", G_CALLBACK(&queue_callback), (gpointer)"underflow");
        g_signal_connect(queue, "overrun", G_CALLBACK(& queue_callback), (gpointer)"overrun");
    }
    else
    {
        g_object_set(G_OBJECT(queue), "silent", (gboolean)(TRUE), nullptr);
    }
}

GstFlowReturn MiracastGstPlayer::on_new_sample_from_udpsrc(GstElement *element, gpointer user_data)
{
    MIRACASTLOG_TRACE("Entering...\n");
    GstFlowReturn ret = GST_FLOW_OK;
    MiracastGstPlayer *self = static_cast<MiracastGstPlayer *>(user_data);
    GstSample *sample = gst_app_sink_pull_sample(GST_APP_SINK(element));

    if ( nullptr == sample )
    {
        MIRACASTLOG_ERROR("Failed to pull sample");
        MIRACASTLOG_TRACE("Exiting..!!!");
        return GST_FLOW_ERROR;
    }

    GstBuffer *gstbuffer = gst_sample_get_buffer(sample);
    if ( nullptr == gstbuffer )
    {
        MIRACASTLOG_ERROR("Failed to get sample buffer");
        gst_sample_unref(sample);
        MIRACASTLOG_TRACE("Exiting..!!!");
        return GST_FLOW_ERROR;
    }

    GstMapInfo extract_map_info;
    if ( TRUE != gst_buffer_map(gstbuffer, &extract_map_info, GST_MAP_READ))
    {
        MIRACASTLOG_ERROR("Failed to do Extract the Gst Buffer");
        gst_sample_unref(sample);
        MIRACASTLOG_TRACE("Exiting..!!!");
        return GST_FLOW_ERROR;
    }

    if (self->m_current_pushbuffer_size + extract_map_info.size < self->m_max_pushbuffer_size )
    {
        // Append the data to the m_push_buffer_ptr
        memcpy(self->m_current_buffer_ptr, extract_map_info.data , extract_map_info.size );
        MIRACASTLOG_TRACE("=== Received Buffer from [ UDPSrc -> Appsink] Gst Buffer Size : %u\n", extract_map_info.size);
        //self->m_current_buffer_ptr = (((guint8*)self->m_current_buffer_ptr) + extract_map_info.size);
        self->m_current_buffer_ptr += extract_map_info.size;
        self->m_current_pushbuffer_size += extract_map_info.size;
    }
    else
    {
        // Create a new GST buffer wrapped around the CPU buffer
        void *mem = g_memdup(self->m_push_buffer_ptr, self->m_current_pushbuffer_size);
        GstBuffer *gstdatapushbuffer = gst_buffer_new_wrapped(mem, self->m_current_pushbuffer_size);

        if ( nullptr == gstdatapushbuffer )
        {
            MIRACASTLOG_ERROR("Failed to do gst_buffer_new_wrapped");
            ret = GST_FLOW_ERROR;
        }
        else
        {
            // Create a new GST sample using the new buffer
            GstSample *newSample = gst_sample_new(gstdatapushbuffer, nullptr, nullptr, nullptr);

            if ( nullptr == newSample )
            {
                MIRACASTLOG_ERROR("Failed to do gst_buffer_new_wrapped");
                ret = GST_FLOW_ERROR;
            }
            else
            {
                // Push the new sample to the appsrc
                MIRACASTLOG_TRACE("Before : gst_app_src_push_sample ()");
                ret = gst_app_src_push_sample(GST_APP_SRC(self->m_appsrc), newSample);
                MIRACASTLOG_TRACE("After : gst_app_src_push_sample ()");
                // unref the new sample
                gst_sample_unref(newSample);
                MIRACASTLOG_TRACE("New sample pushed to Appsrc, Buffer size [%u]\n ", gst_buffer_get_size(gstdatapushbuffer));
                // Reset the self->m_push_buffer_ptr
                self->m_current_buffer_ptr = self->m_push_buffer_ptr;
                memcpy(self->m_current_buffer_ptr, extract_map_info.data, extract_map_info.size);
                //self->m_current_buffer_ptr = (((guint8*)self->m_current_buffer_ptr) + extract_map_info.size);
                self->m_current_buffer_ptr += extract_map_info.size;
                self->m_current_pushbuffer_size = extract_map_info.size;
            }
            // unref the new buffer
            gst_buffer_unref(gstdatapushbuffer);
        }
    }
    // Unmap the GStreamer buffer
    gst_buffer_unmap(gstbuffer, &extract_map_info);
    // unref the pulled sample
    gst_sample_unref(sample);
    MIRACASTLOG_TRACE("Exiting..!!!");
    return ret;
}

#if 0
bool MiracastGstPlayer::createPipeline()
{
    MIRACASTLOG_TRACE("Entering..!!!");

    GstStateChangeReturn ret;
    GstBus *bus;
    // gint flags;
    m_bReady = false;
    m_currentPosition = 0.0f;
    m_elts = G_QUEUE_INIT;

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
    unsigned flagNativeVideo = 0;
    unsigned flagBuffering = 0;

    if (0 == access("/opt/miracast_native_video", F_OK))
    {
        flagNativeVideo = getGstPlayFlag("native-video");
        MIRACASTLOG_INFO("flagNativeVideo [ %#04X ]", flagNativeVideo);
    }
    if (0 == access("/opt/miracast_buffering", F_OK))
    {
        flagBuffering = getGstPlayFlag("buffering");
        MIRACASTLOG_INFO("flagBuffering [ %#04X ]", flagBuffering);
    }

    g_object_set(m_pipeline, "flags", flagAudio | flagVideo | flagNativeVideo | flagBuffering, nullptr);

    m_video_sink = gst_element_factory_make("westerossink", nullptr);

    bool westerossink_immediate_output = false;

    if (0 == access("/opt/miracast_use_immediateoutput", F_OK))
    {
        westerossink_immediate_output = true;
    }

    if(g_object_class_find_property(G_OBJECT_GET_CLASS(m_video_sink), "immediate-output"))
    {
        g_object_set(G_OBJECT(m_video_sink), "immediate-output", westerossink_immediate_output , nullptr);
        MIRACASTLOG_INFO("Set immediate-output as [%x] \n",westerossink_immediate_output);
    }
#if defined(PLATFORM_AMLOGIC)
    if (0 == access("/opt/miracast_use_amlhalasink", F_OK))
    {
        bool amlhalasink_direct_mode = true;

        if (0 == access("/opt/miracast_amlasink_disable_dm", F_OK))
        {
            amlhalasink_direct_mode = false;
        }
        m_audio_sink = gst_element_factory_make("amlhalasink", nullptr);

        if(g_object_class_find_property(G_OBJECT_GET_CLASS(m_audio_sink), "direct-mode"))
        {
            g_object_set(G_OBJECT(m_audio_sink), "direct-mode", amlhalasink_direct_mode , nullptr);
            MIRACASTLOG_INFO("Set direct-mode as [%x] \n",amlhalasink_direct_mode );
        }
    }
#endif
    updateVideoSinkRectangle();

    if (0 != access("/opt/miracast_avoid_element_setup", F_OK))
    {
        MIRACASTLOG_INFO("!!! Configuring element-setup callback !!!\n");
        g_signal_connect (m_pipeline, "element-setup", G_CALLBACK(element_setup),&m_elts);
        MIRACASTLOG_INFO("!!! Configuring element-setup callback DONE !!!\n");
    }

    g_object_set(m_pipeline, "video-sink", m_video_sink, nullptr);

    if ( nullptr != m_audio_sink )
    {
        g_object_set(m_pipeline, "audio-sink", m_audio_sink, nullptr);
    }

    MIRACASTLOG_INFO("Miracast playbin uri [ %s ]", m_uri.c_str());
    g_object_set(m_pipeline, "uri", (const gchar *)m_uri.c_str(), nullptr);

    bus = gst_element_get_bus(m_pipeline);
    m_bus_watch_id = gst_bus_add_watch(bus, (GstBusFunc)busMessageCb, this);
    gst_object_unref(bus);

    g_main_context_pop_thread_default(m_main_loop_context);
    pthread_create(&m_playback_thread, nullptr, MiracastGstPlayer::playbackThread, this);
    pthread_create(&m_player_statistics_tid, nullptr, MiracastGstPlayer::monitor_player_statistics_thread, this);

    /* Start playing */
    if (0 == access("/opt/miracast_player_state", F_OK))
    {
        MIRACASTLOG_TRACE("Pipeline Paused.");
        ret = gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
    }
    else
    {
        MIRACASTLOG_TRACE("Start Playing.");
        ret = gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
    }

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
#else

#if 1
bool MiracastGstPlayer::createPipeline()
{
    GstStateChangeReturn ret;
    GstBus *bus = nullptr;

    /* create gst pipeline */
    m_main_loop_context = g_main_context_new();
    g_main_context_push_thread_default(m_main_loop_context);
    m_main_loop = g_main_loop_new(m_main_loop_context, FALSE);

    // Create a new pipeline
    m_udpsrc2appsink_pipeline = gst_pipeline_new("miracast_udpsrc2appsink");

    // Create elements
    m_udpsrc = gst_element_factory_make("udpsrc", "miracast_udpsrc");
    m_rtpmp2tdepay = gst_element_factory_make("rtpmp2tdepay", "miracast_rtpmp2tdepay");
    m_rtpjitterbuffer = gst_element_factory_make("rtpjitterbuffer", "miracast_rtpjitterbuffer");
    m_appsinkqueue = gst_element_factory_make("queue", "appsink_queue");
    m_appsink = gst_element_factory_make("appsink", "miracast_appsink");

    if (!m_udpsrc2appsink_pipeline || !m_udpsrc || !m_rtpmp2tdepay || !m_rtpjitterbuffer || !m_appsinkqueue || !m_appsink )
    {
        MIRACASTLOG_ERROR("Not all elements could be created.\n");
        return -1;
    }

    // Configuring Appsink Queue
    configure_queue(m_appsinkqueue);

    // Set the UDP source properties
    g_object_set(G_OBJECT(m_udpsrc), "port", 1990, nullptr);

    GstCaps *caps = gst_caps_new_simple("application/x-rtp", "media", G_TYPE_STRING, "video", nullptr);
    g_object_set(G_OBJECT(m_udpsrc), "caps", caps, nullptr);

    guint64 testbuffersize = 0;
    std::string opt_flag_buffer = "";

    opt_flag_buffer = parse_opt_flag( "/opt/miracast_udpsrc_mtu_size" , true );
    testbuffersize = 64*1024;
    if (!opt_flag_buffer.empty())
    {
        testbuffersize = std::stoull(opt_flag_buffer.c_str());
    }

    MIRACASTLOG_INFO("setting mtu value to udpsrc as [%llu]\n",testbuffersize);
    g_object_set(G_OBJECT(m_udpsrc), "mtu", testbuffersize, nullptr);

    opt_flag_buffer = parse_opt_flag( "/opt/miracast_udpsrc_blocksize" , true );
    testbuffersize = 64*1024;
    if (!opt_flag_buffer.empty())
    {
        testbuffersize = std::stoull(opt_flag_buffer.c_str());
    }

    MIRACASTLOG_INFO("setting blocksize value to udpsrc as [%llu]\n",testbuffersize);
    g_object_set(G_OBJECT(m_udpsrc), "blocksize", testbuffersize, nullptr);

    opt_flag_buffer = parse_opt_flag( "/opt/miracast_udpsrc_buffersize" , true );
    testbuffersize = 64*1024;
    if (!opt_flag_buffer.empty())
    {
        testbuffersize = std::stoull(opt_flag_buffer.c_str());
    }

    MIRACASTLOG_INFO("setting buffer-size value to udpsrc as [%llu]\n",testbuffersize);
    g_object_set(G_OBJECT(m_udpsrc), "buffer-size", testbuffersize, nullptr);

    /* to be notified of messages from this pipeline, mostly EOS */
    bus = gst_element_get_bus(m_udpsrc2appsink_pipeline);
    gst_bus_add_watch(bus, (GstBusFunc)on_udpsrc2appsink_bus_message, this);
    gst_object_unref(bus);
    
    opt_flag_buffer = parse_opt_flag( "/opt/miracast_max_pushbuffer_size" , true );

    if (!opt_flag_buffer.empty())
    {
        m_max_pushbuffer_size = std::stoull(opt_flag_buffer.c_str());
        MIRACASTLOG_INFO("Max pushbuffer size configured as [%llu]\n",m_max_pushbuffer_size);
    }
    else
    {
        m_max_pushbuffer_size = DEFAULT_MAX_PUSH_BUFFER_SIZE;
        MIRACASTLOG_INFO("Default Max pushbuffer size configured as [%llu]\n",m_max_pushbuffer_size);
    }
    
    // Configure the appsink
    g_object_set(G_OBJECT(m_appsink), "emit-signals", TRUE, "sync", FALSE, nullptr);
    
    //gst_base_sink_set_sync(GST_BASE_SINK_CAST(appsink), false);
    //MIRACASTLOG_INFO("===> gst_base_sink_set_sync, set to False\n");
    g_object_set(G_OBJECT(m_appsink), "async", FALSE, nullptr);

    m_push_buffer_ptr = (guint8*)(g_malloc0(m_max_pushbuffer_size));

    // Check if memory allocation succeeded
    if ( nullptr == m_push_buffer_ptr )
    {
        MIRACASTLOG_ERROR("Memory allocation failed for [%llu].\n",m_max_pushbuffer_size);
        return 1;
    }

    m_current_buffer_ptr = m_push_buffer_ptr;

    // Set up a signal handler for new buffer signals from appsink
    g_signal_connect(m_appsink, "new-sample", G_CALLBACK(on_new_sample_from_udpsrc), this);

    // Add elements to the pipeline
    // gst_bin_add_many(GST_BIN(m_udpsrc2appsink_pipeline), m_udpsrc, m_rtpmp2tdepay, m_appsink, nullptr);
    // gst_bin_add_many(GST_BIN(m_udpsrc2appsink_pipeline), m_udpsrc, m_rtpjitterbuffer, m_rtpmp2tdepay, m_appsinkqueue, m_appsink, nullptr);
    gst_bin_add_many(GST_BIN(m_udpsrc2appsink_pipeline), m_udpsrc, m_rtpmp2tdepay, m_appsinkqueue , m_appsink, nullptr );

    // Link udpsrc to appsink
    //if (!gst_element_link_many( m_udpsrc, m_rtpmp2tdepay , m_appsink, nullptr ))
    //if (!gst_element_link_many(m_udpsrc, m_rtpjitterbuffer, m_rtpmp2tdepay, m_appsinkqueue, m_appsink, nullptr ))
    if (!gst_element_link_many(m_udpsrc, m_rtpmp2tdepay, m_appsinkqueue, m_appsink, nullptr ))
    {
        MIRACASTLOG_ERROR("Elements could not be linked.\n");
        gst_object_unref(m_udpsrc2appsink_pipeline);
        return -1;
    }

    // Set up pipeline
    m_playbin2appsrc_pipeline = gst_element_factory_make("playbin", "miracast_playbin");
    if (!m_playbin2appsrc_pipeline)
    {
        MIRACASTLOG_ERROR( "Failed to create pipeline.\n");
    }
    else
    {
        bus = gst_element_get_bus (m_playbin2appsrc_pipeline);
        gst_bus_add_watch (bus, (GstBusFunc) on_playbin2appsrc_bus_message, this);
        gst_object_unref (bus);

        // Pipeline created
        g_signal_connect(m_playbin2appsrc_pipeline, "source-setup", G_CALLBACK(playbin_source_setup), this);
        g_object_set(m_playbin2appsrc_pipeline, "uri", "appsrc://", nullptr);
    }
    m_video_sink = gst_element_factory_make("westerossink", nullptr);

    updateVideoSinkRectangle();

    g_object_set(m_playbin2appsrc_pipeline, "video-sink", m_video_sink, nullptr);

    g_main_context_pop_thread_default(m_main_loop_context);
    pthread_create(&m_playback_thread, nullptr, MiracastGstPlayer::playbackThread, this);
    pthread_create(&m_player_statistics_tid, nullptr, MiracastGstPlayer::monitor_player_statistics_thread, this);

    ret = gst_element_set_state(m_udpsrc2appsink_pipeline, GST_STATE_PLAYING);
    ret = gst_element_set_state(m_playbin2appsrc_pipeline, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        MIRACASTLOG_ERROR("Unable to set the pipeline to the playing state.");
        gst_object_unref(m_udpsrc2appsink_pipeline);
        gst_object_unref(m_playbin2appsrc_pipeline);
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
#else
bool MiracastGstPlayer::createPipeline()
{
    MIRACASTLOG_TRACE("Entering..!!!");

    GstStateChangeReturn ret;
    GstBus *bus = nullptr;
    // gint flags;
    m_bReady = false;
    m_currentPosition = 0.0f;
    m_elts = G_QUEUE_INIT;

    m_push_buffer_ptr = nullptr;

    std::string opt_flag_buffer = parse_opt_flag( "/opt/miracast_max_pushbuffer_size" , true );

    if (!opt_flag_buffer.empty())
    {
        m_max_pushbuffer_size = std::stoull(opt_flag_buffer.c_str());
        MIRACASTLOG_INFO("Max pushbuffer size configured as [%llu]\n",m_max_pushbuffer_size);
    }
    else
    {
        m_max_pushbuffer_size = DEFAULT_MAX_PUSH_BUFFER_SIZE;
        MIRACASTLOG_INFO("Default Max pushbuffer size configured as [%llu]\n",m_max_pushbuffer_size);
    }

    m_push_buffer_ptr = g_malloc0(m_max_pushbuffer_size);

    // Check if memory allocation succeeded
    if ( nullptr == m_push_buffer_ptr )
    {
        g_print("Memory allocation failed for [%llu].\n",m_max_pushbuffer_size);
        return 1;
    }

    m_current_buffer_ptr = m_push_buffer_ptr;

    /* create gst pipeline */
    m_main_loop_context = g_main_context_new();
    g_main_context_push_thread_default(m_main_loop_context);
    m_main_loop = g_main_loop_new(m_main_loop_context, FALSE);

    // Create a new pipeline
    m_udpsrc2appsink_pipeline = gst_pipeline_new("miracast_udpsrc2appsink");

    // Create elements
    m_udpsrc = gst_element_factory_make("udpsrc", "miracast_udpsrc");
    m_appsink = gst_element_factory_make("appsink", "miracast_appsink");

    if (!m_udpsrc2appsink_pipeline || !m_udpsrc || !m_appsink )
    {
        MIRACASTLOG_ERROR("Not all elements could be created.\n");
        return -1;
    }

    // Set the UDP source properties
    g_object_set(G_OBJECT(m_udpsrc), "port", 1990, nullptr);

    /* to be notified of messages from this pipeline, mostly EOS */
    bus = gst_element_get_bus(m_udpsrc2appsink_pipeline);
    gst_bus_add_watch(bus, (GstBusFunc)on_udpsrc2appsink_bus_message, this);
    gst_object_unref(bus);

    // Configure the appsink
    g_object_set(G_OBJECT(m_appsink), "emit-signals", TRUE, "sync", FALSE, nullptr);

    //gst_base_sink_set_sync(GST_BASE_SINK_CAST(appsink), false);
    //MIRACASTLOG_INFO("===> gst_base_sink_set_sync, set to False\n");
    g_object_set(G_OBJECT(m_appsink), "async", FALSE, nullptr);

    // Set up a signal handler for new buffer signals from appsink
    g_signal_connect(m_appsink, "new-sample", G_CALLBACK(on_new_sample_from_udpsrc), this);

    // Add elements to the pipeline
    gst_bin_add_many(GST_BIN(m_udpsrc2appsink_pipeline), m_udpsrc, m_appsink, nullptr);

    // Link udpsrc to appsink
    if (!gst_element_link(m_udpsrc, m_appsink))
    {
        MIRACASTLOG_ERROR("Elements could not be linked.\n");
        gst_object_unref(m_udpsrc2appsink_pipeline);
        return false;
    }

    // Set up pipeline
    m_playbin2appsrc_pipeline = gst_element_factory_make("playbin", "miracast_playbin");
    if (!m_playbin2appsrc_pipeline)
    {
        MIRACASTLOG_ERROR( "Failed to create pipeline.\n");
        gst_object_unref(m_udpsrc2appsink_pipeline);
        return false;
    }
    else
    {
        bus = gst_element_get_bus (m_playbin2appsrc_pipeline);
        gst_bus_add_watch (bus, (GstBusFunc) on_playbin2appsrc_bus_message, this);
        gst_object_unref (bus);

        // Pipeline created
        g_object_set(m_playbin2appsrc_pipeline, "uri", "appsrc://", nullptr);
        g_signal_connect(m_playbin2appsrc_pipeline, "source-setup", G_CALLBACK(playbin_source_setup), this);
    }

    m_video_sink = gst_element_factory_make("westerossink", nullptr);

    updateVideoSinkRectangle();

    g_object_set(m_playbin2appsrc_pipeline, "video-sink", m_video_sink, nullptr);

    g_main_context_pop_thread_default(m_main_loop_context);
    pthread_create(&m_playback_thread, nullptr, MiracastGstPlayer::playbackThread, this);
    pthread_create(&m_player_statistics_tid, nullptr, MiracastGstPlayer::monitor_player_statistics_thread, this);

    ret = gst_element_set_state(m_udpsrc2appsink_pipeline, GST_STATE_PLAYING);
    ret = gst_element_set_state(m_playbin2appsrc_pipeline, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        MIRACASTLOG_ERROR("Unable to set the pipeline to the playing state.");
        gst_object_unref(m_udpsrc2appsink_pipeline);
        gst_object_unref(m_playbin2appsrc_pipeline);
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
#endif
#endif

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

gboolean MiracastGstPlayer::busMessageCb(GstBus *bus, GstMessage *msg, gpointer user_data)
{
    MIRACASTLOG_TRACE("Entering..!!!");
    MiracastGstPlayer *self = static_cast<MiracastGstPlayer *>(user_data);

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
            std::string file_name = "miracast_progressive_";
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
    case GST_MESSAGE_ASYNC_DONE:
    {
        MIRACASTLOG_VERBOSE("BusMessageCallback GST_MESSAGE_ASYNC_DONE Received\n");
        if (GST_MESSAGE_SRC(msg) == GST_OBJECT(self->m_pipeline))
        {
            MIRACASTLOG_VERBOSE("===> ASYNC-DONE %s %d\n",gst_element_state_get_name(GST_STATE(self->m_pipeline)));
            MIRACASTLOG_VERBOSE("BusMessageCallback GST_MESSAGE_ASYNC_DONE \n");
            if (0 == access("/opt/miracast_player_state", F_OK))
            {
                MIRACASTLOG_VERBOSE("BusMessageCallback start playing\n");
                gst_element_set_state(self->m_pipeline, GST_STATE_PLAYING);
            }
        }
    }
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
        pipeline = m_pipeline;
    }

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

#if 0
    double cur_position = getCurrentPosition();
#else
    double udpsrc2appsink_cur_pos = getCurrentPosition(m_udpsrc2appsink_pipeline);
    double playbin2appsrc_cur_pos = getCurrentPosition(m_playbin2appsrc_pipeline);
#endif

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

        MIRACASTLOG_INFO("Current UDPSRC2APPSINK-PTS: [ %f ], PLAYBIN2APPSRC-PTS: [ %f ]",
                            udpsrc2appsink_cur_pos,
                            playbin2appsrc_cur_pos);
        MIRACASTLOG_INFO("Total Frames: [ %lu], Rendered Frames : [ %lu ], Dropped Frames: [%lu]",
                            total_video_frames,
                            render_frame,
                            dropped_video_frames);
        gst_structure_free( stats );
     }
#if 0
    print_pipeline_state();
#else
    print_pipeline_state(m_udpsrc2appsink_pipeline);
    print_pipeline_state(m_playbin2appsrc_pipeline);
#endif
    MIRACASTLOG_INFO("\n=============================================\n");
    MIRACASTLOG_TRACE("Exiting..!!!");	
    return ret;
}
