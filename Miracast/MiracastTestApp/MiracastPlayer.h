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
#include <glib.h>
#include <pthread.h>
#include <stdint.h>

class MiracastPlayer
{
public:
    static MiracastPlayer *getInstance();
    static void destroyInstance();
    bool launch(std::string localip , std::string streaming_port);
    bool stop();
    bool pause();
    int getPlayerstate();
    bool setUri(std::string ipaddr, std::string port);
    std::string getUri();
    double getDuration();
    double getCurrentPosition();
    bool seekTo(double seconds);
    double get_current_position();
    bool get_player_statistics();
    void print_pipeline_state();

private:
    GstElement *m_pipeline{nullptr};
    std::string m_uri;
    int m_bus_watch_id{-1};
    bool m_bBuffering;
    bool m_is_live;
    bool m_bReady;
    int m_buffering_level;
    double m_currentPosition;
    GstElement *m_video_sink{nullptr};
    pthread_t m_playback_thread;

    static MiracastPlayer *mMiracastPlayer;
    MiracastPlayer();
    virtual ~MiracastPlayer();
    MiracastPlayer &operator=(const MiracastPlayer &) = delete;
    MiracastPlayer(const MiracastPlayer &) = delete;

    bool createPipeline();
    static gboolean busMessageCb(GstBus *bus, GstMessage *msg, gpointer user_data);
    bool changePipelineState(GstState state) const;

    static void *playbackThread(void *ctx);
    GMainLoop *m_main_loop{nullptr};
    GMainContext *m_main_loop_context{nullptr};
    
    pthread_t m_player_statistics_tid;
    static void *monitor_player_statistics_thread(void *ctx);
};

#endif /* MiracastPlayer_hpp */
