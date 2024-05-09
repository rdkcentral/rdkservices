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

class MiracastGstPlayer
{
public:
    static MiracastGstPlayer *getInstance();
    static void destroyInstance();
    bool launch(std::string& localip , std::string& streaming_port,MiracastRTSPMsg *rtsp_instance);
    bool stop();
    bool pause();
    bool resume();
    bool setVideoRectangle( VIDEO_RECT_STRUCT video_rect , bool apply = false );
    int getPlayerstate();
    double getDuration(GstElement *pipeline = nullptr);
    bool seekTo(double seconds,GstElement *pipeline = nullptr);
    double getCurrentPosition(GstElement *pipeline = nullptr);
    bool get_player_statistics();
    void print_pipeline_state(GstElement *pipeline = nullptr);

private:
    GstElement  *m_pipeline{nullptr};
    GstElement  *m_udpsrc{nullptr};
    GstElement  *m_rtpmp2tdepay{nullptr};
    GstElement  *m_rtpjitterbuffer{nullptr};
    GstElement *m_tsparse{nullptr};
    GstElement *m_tsdemux{nullptr};
    GstElement *m_vQueue{nullptr};
    GstElement *m_h264parse{nullptr};
    GstElement *m_aQueue{nullptr};
    GstElement *m_aacparse{nullptr};
    GstElement *m_avdec_aac{nullptr};
    GstElement *m_audioconvert{nullptr};
    bool m_firstVideoFrameReceived{false};
    MiracastRTSPMsg *m_rtsp_reference_instance{nullptr};

    std::string m_uri;
    guint64 m_streaming_port;
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
    static void onFirstVideoFrameCallback(GstElement* object, guint arg0, gpointer arg1,gpointer userdata);
    void notifyPlaybackState(eMIRA_GSTPLAYER_STATES gst_player_state);
    static gboolean busMessageCb(GstBus *bus, GstMessage *msg, gpointer user_data);
    bool changePipelineState(GstState state) const;

    static void *playbackThread(void *ctx);
    GMainLoop *m_main_loop{nullptr};
    GMainContext *m_main_loop_context{nullptr};

    bool m_statistics_thread_loop{false};
    
    pthread_t m_player_statistics_tid;
    static void *monitor_player_statistics_thread(void *ctx);
    static void pad_added_handler(GstElement *gstelement, GstPad *new_pad, gpointer userdata);
};

#endif /* MiracastGstPlayer_hpp */
