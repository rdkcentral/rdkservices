/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2021 RDK Management
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

#include <string>

#include "Module.h"
#include "AbstractPlugin.h"
#include "LinchpinPluginRPC.h"
#include "ILinchpinPluginEvents.h"

namespace WPEFramework {

    namespace Plugin {
        struct paramInfo_t {
            char* paramName;
            WPEFramework::Core::JSON::Variant::type paramType;
        };

        typedef std::list<std::reference_wrapper<const paramInfo_t>> paramInfoList_t;

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
        class PlaybackSync : public AbstractPlugin, public ILinchpinPluginEvents {
        private:

            // We do not allow this plugin to be copied !!
            PlaybackSync(const PlaybackSync&) = delete;
            PlaybackSync& operator=(const PlaybackSync&) = delete;

            //Begin methods
            uint32_t ConnectToServer(const JsonObject& parameters, JsonObject& response);
            uint32_t setRoomID(const JsonObject& parameters, JsonObject& response);
            uint32_t getRoomID(const JsonObject& parameters, JsonObject& response);
            uint32_t setDeviceDetails(const JsonObject& parameters, JsonObject& response);
            uint32_t getDeviceDetails(const JsonObject& parameters, JsonObject& response);
            // uint32_t setLeader(const JsonObject& parameters, JsonObject& response);
            // uint32_t getLeader(const JsonObject& parameters, JsonObject& response);
            // uint32_t isLeader(const JsonObject& parameters, JsonObject& response);
            uint32_t setPlaybackURL(const JsonObject& parameters, JsonObject& response);
            uint32_t getPlaybackURL(const JsonObject& parameters, JsonObject& response);
            uint32_t setPlaybackServiceKey(const JsonObject& parameters, JsonObject& response);
            uint32_t getPlaybackServiceKey(const JsonObject& parameters, JsonObject& response);
            uint32_t setPlaybackString(string& storage, uint8_t labelIndex, const JsonObject& parameters, JsonObject& response);
            uint32_t getPlaybackString(string& storage, uint8_t labelIndex, const JsonObject& parameters, JsonObject& response);
            uint32_t requestLeadership(const JsonObject& parameters, JsonObject& response);
            uint32_t publishPlaybackMessage(const JsonObject& parameters, JsonObject& response);
            uint32_t publishPlaybackPause(const JsonObject& parameters, JsonObject& response);
            uint32_t publishPlaybackSpeed(const JsonObject& parameters, JsonObject& response);
            uint32_t publishPlaybackStop(const JsonObject& parameters, JsonObject& response);
            uint32_t publishPlaybackPosition(const JsonObject& parameters, JsonObject& response);
            uint32_t publishPlaybackStarted(const JsonObject& parameters, JsonObject& response);
            uint32_t publishPlaybackError(const JsonObject& parameters, JsonObject& response);
            //End methods

            bool ValidPlaybackSyncEvent(string& eventName, JsonObject& params);
            void PopulateCachedParameters(const paramInfoList_t& paramInfoList, JsonObject& packParams);
        public:
            PlaybackSync();
            ~PlaybackSync();
            void onNotifyMessageReceived(const string& room);
            void onConnectionClosed(void);
            void Deinitialize(PluginHost::IShell* service) override;
            const string Initialize(PluginHost::IShell* service) override;

        public:
            static PlaybackSync* _instance;
        private:
            PluginHost::IShell* m_messageShell;
            std::string         m_roomID;
            std::string         m_playbackURL;
            std::string         m_playbackServiceKey;
            std::string         m_deviceDetailsString;
            JsonObject          m_deviceDetails;
            // JsonObject          m_leaderDetails;
            // bool                m_isLeader {false};
            LinchpinPluginRPC   m_linchpinRPC;
        };
	} // namespace Plugin
} // namespace WPEFramework
