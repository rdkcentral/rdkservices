/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2022 RDK Management
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

#pragma once

#include <memory>
#include "Module.h"

namespace libmediaplayer //Forward declaration.
{
    class mediaplayer;
}
namespace WPEFramework {
    namespace Plugin {
        class MediaEngineRMF : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        public:

            enum class player_state {
                UNINITIALIZED=0,
                PLATFORM_INITIALIZED=1,
                INSTANTIATED=2,
                PAUSED=3,
                PLAYING=4,
            };

            struct video_rectangle
            {
                unsigned int x;
                unsigned int y;
                unsigned int width;
                unsigned int height;
                video_rectangle(): x(0), y(0), width(1080), height(720){} //Safe defaults
            };

            MediaEngineRMF();
            virtual ~MediaEngineRMF();
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override;
            void onError(libmediaplayer::notification_payload * payload);
            void onEvent(libmediaplayer::notification_payload * payload);

            BEGIN_INTERFACE_MAP(MODULE_NAME)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

        public/*members*/:
            static MediaEngineRMF* _instance;

        public /*constants*/:
            static const string SERVICE_NAME;
            static const string METHOD_PLAY;
            static const string METHOD_STOP;
            static const string METHOD_MUTE;
            static const string METHOD_SET_VOLUME;
            static const string METHOD_SET_VIDEO_RECTANGLE;

            static const string METHOD_INITIALIZE;
            static const string METHOD_DEINITIALIZE;
            static const string EVT_ON_STATUS;
            static const string EVT_ON_ERROR;
            static const string EVT_ON_WARNING;


        private/*registered methods*/:
            //methods
            uint32_t initialize_player(const JsonObject& parameters, JsonObject& response);
            uint32_t deinitialize_player(const JsonObject& parameters, JsonObject& response);
            uint32_t play(const JsonObject& parameters, JsonObject& response);
            uint32_t stop(const JsonObject& parameters, JsonObject& response);
            uint32_t mute(const JsonObject& parameters, JsonObject& response);
            uint32_t setVolume(const JsonObject& parameters, JsonObject& response);
            uint32_t setVideoRectangle(const JsonObject& parameters, JsonObject& response);

        private/*internal methods*/:
            MediaEngineRMF(const MediaEngineRMF&) = delete;
            MediaEngineRMF& operator=(const MediaEngineRMF&) = delete;

        private/*members*/:
            player_state m_player_state;
            std::unique_ptr <libmediaplayer::mediaplayer> m_player;
            video_rectangle m_video_rectangle;
        };
    } // namespace Plugin
} // namespace WPEFramework
