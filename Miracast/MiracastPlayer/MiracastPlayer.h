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

#pragma once

#include <mutex>
#include <vector>

#include "Module.h"
#include "MiracastRTSPMsg.h"
#include "SoC_GstPlayer.h"

using std::vector;
namespace WPEFramework
{
    namespace Plugin
    {
        // This is a server for a JSONRPC communication channel.
        // For a plugin to be capable to handle JSONRPC, inherit from PluginHost::JSONRPC.
        // By inheriting from this class, the plugin realizes the interface PluginHost::IDispatcher.
        // This realization of this interface implements, by default, the following methods on this plugin
        // - exists
        // - register
        // - unregister
        // Any other methood to be handled by this plugin  can be added can be added by using the
        // templated methods Register on the PluginHost::JSONRPC class.
        // As the registration/unregistration of notifications is realized by the class PluginHost::JSONRPC,
        // this class exposes a public method called, Notify(), using this methods, all subscribed clients
        // will receive a JSONRPC message as a notification, in case this method is called.
        class MiracastPlayer : public PluginHost::IPlugin, public PluginHost::JSONRPC, public MiracastPlayerNotifier
        {
        public:
            // constants
            static const short API_VERSION_NUMBER_MAJOR;
            static const short API_VERSION_NUMBER_MINOR;
            static const string SERVICE_NAME;

            // methods
            static const string METHOD_MIRACAST_PLAYER_PLAY_REQUEST;
            static const string METHOD_MIRACAST_PLAYER_STOP_REQUEST;
            static const string METHOD_MIRACAST_PLAYER_SET_PLAYER_STATE;
            static const string METHOD_MIRACAST_PLAYER_SET_VIDEO_RECTANGLE;
            static const string METHOD_MIRACAST_SET_VIDEO_FORMATS;
            static const string METHOD_MIRACAST_SET_AUDIO_FORMATS;
            static const string METHOD_MIRACAST_SET_RTSP_WAITTIMEOUT;
            static const string METHOD_MIRACAST_PLAYER_SET_LOG_LEVEL;

#ifdef ENABLE_MIRACAST_PLAYER_TEST_NOTIFIER
            static const string METHOD_MIRACAST_TEST_NOTIFIER;
            uint32_t testNotifier(const JsonObject &parameters, JsonObject &response);
            bool m_isTestNotifierEnabled;
#endif /* ENABLE_MIRACAST_PLAYER_TEST_NOTIFIER */

            MiracastPlayer();
            virtual ~MiracastPlayer();
            virtual const string Initialize(PluginHost::IShell *shell) override;
            virtual void Deinitialize(PluginHost::IShell *service) override;
            virtual string Information() const override;

            void onStateChange(const std::string& client_mac, const std::string& client_name, eMIRA_PLAYER_STATES player_state, eM_PLAYER_REASON_CODE reason_code ) override;

            BEGIN_INTERFACE_MAP(MiracastPlayer)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

            PluginHost::IShell *m_CurrentService;
            static MiracastPlayer *_instance;
            static MiracastRTSPMsg *m_miracast_rtsp_obj;

        private:
            VIDEO_RECT_STRUCT m_video_sink_rect;
            bool m_isServiceInitialized;
            bool m_isServiceEnabled;
            SoC_GstPlayer *m_GstPlayer;
            MiracastRTSPMsg *m_rtsp_msg;

            uint32_t playRequest(const JsonObject &parameters, JsonObject &response);
            uint32_t stopRequest(const JsonObject &parameters, JsonObject &response);
            uint32_t setPlayerState(const JsonObject &parameters, JsonObject &response);
            uint32_t setVideoRectangle(const JsonObject &parameters, JsonObject &response);
            uint32_t setVideoFormats(const JsonObject &parameters, JsonObject &response);
            uint32_t setAudioFormats(const JsonObject &parameters, JsonObject &response);
            uint32_t setRTSPWaitTimeout(const JsonObject &parameters, JsonObject &response);
            uint32_t setLogging(const JsonObject &parameters, JsonObject &response);

            std::string reasonDescription(eM_PLAYER_REASON_CODE);
            std::string stateDescription(eMIRA_PLAYER_STATES);

            // We do not allow this plugin to be copied !!
            MiracastPlayer(const MiracastPlayer &) = delete;
            MiracastPlayer &operator=(const MiracastPlayer &) = delete;
        };
    } // namespace Plugin
} // namespace WPEFramework
