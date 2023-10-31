/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
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
 **/

#ifndef _MIRACAST_PLAYER_H_
#define _MIRACAST_PLAYER_H_

#include <string>
#include <vector>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <glib.h>
#include <pthread.h>
#include <stdint.h>

#define DEFAULT_MAX_PUSH_BUFFER_SIZE    ( 1 * 1024 * 1024 )

class MiracastGstPlayer
{
public:
    static MiracastGstPlayer *getInstance();
    static void destroyInstance();
    bool launch(std::string localip , std::string streaming_port);
    bool stop();
    bool pause();
    bool resume();
    bool setVideoRectangle( VIDEO_RECT_STRUCT video_rect , bool apply = false );
    int getPlayerstate();
    bool setUri(std::string ipaddr, std::string port);
    std::string getUri();
    double getDuration(GstElement *pipeline = nullptr);
    bool seekTo(double seconds,GstElement *pipeline = nullptr);
    double getCurrentPosition(GstElement *pipeline = nullptr);
    bool get_player_statistics();
    void print_pipeline_state(GstElement *pipeline = nullptr);
    GQueue m_elts;
    static void element_setup(GstElement * playbin, GstElement * element, GQueue * elts);
    static std::string parse_opt_flag( std::string file_name , bool integer_check = false );

private:
    GstElement  *m_pipeline{nullptr};

    GstElement  *m_udpsrc2appsink_pipeline{nullptr};
    GstElement  *m_playbin2appsrc_pipeline{nullptr};
    GstElement  *m_udpsrc{nullptr};
    GstElement  *m_rtpmp2tdepay{nullptr};
    GstElement  *m_rtpjitterbuffer{nullptr};
    GstElement  *m_appsinkqueue{nullptr};
    GstElement  *m_appsink{nullptr};
    GstElement  *m_appsrc{nullptr};
    gboolean    bPushData;
    guint64     m_max_pushbuffer_size{DEFAULT_MAX_PUSH_BUFFER_SIZE};
    guint64     m_current_pushbuffer_size{0};
    guint8      *m_push_buffer_ptr{nullptr};
    guint8      *m_current_buffer_ptr{nullptr};

    std::string m_uri;
    int m_bus_watch_id{-1};
    bool m_bBuffering;
    bool m_is_live;
    bool m_bReady;
    int m_buffering_level;
    double m_currentPosition;
    GstElement *m_video_sink{nullptr};
    GstElement *m_audio_sink{nullptr};
    pthread_t m_playback_thread;
    VIDEO_RECT_STRUCT m_video_rect_st;

    static MiracastGstPlayer *mMiracastGstPlayer;
    MiracastGstPlayer();
    virtual ~MiracastGstPlayer();
    MiracastGstPlayer &operator=(const MiracastGstPlayer &) = delete;
    MiracastGstPlayer(const MiracastGstPlayer &) = delete;

    bool createPipeline();
    bool updateVideoSinkRectangle(void);
    static gboolean busMessageCb(GstBus *bus, GstMessage *msg, gpointer user_data);
    bool changePipelineState(GstState state) const;

    static gboolean on_playbin2appsrc_bus_message(GstBus *bus, GstMessage *msg, gpointer user_data);
    static gboolean on_udpsrc2appsink_bus_message(GstBus *bus, GstMessage *msg, gpointer user_data);
    static void playbin_source_setup(GstElement *pipeline, GstElement *source, gpointer user_data);
    static void appsrc_need_data(GstAppSrc *src, guint length, gpointer user_data);
    static void appsrc_enough_data(GstAppSrc *src, gpointer user_data);
    static GstFlowReturn on_new_sample_from_udpsrc(GstElement *element, gpointer user_data);
    static void configure_queue(GstElement * queue);
    static void queue_callback(GstElement* object, gpointer user_data);

    static void *playbackThread(void *ctx);
    GMainLoop *m_main_loop{nullptr};
    GMainContext *m_main_loop_context{nullptr};
    
    pthread_t m_player_statistics_tid;
    static void *monitor_player_statistics_thread(void *ctx);
};

#endif /* MiracastGstPlayer_hpp */
