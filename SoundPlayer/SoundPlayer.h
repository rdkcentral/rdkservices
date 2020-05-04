/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2020 RDK Management
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

#include "Module.h"
#include "utils.h"
#include "AbstractPlugin.h"

namespace WPEFramework {

    namespace Plugin {

        class CGStreamerSoundPlayer;

        class ISoundListener
        {
        public:
            virtual void onItemOpen() = 0;
            virtual void onItemError(std::string error) = 0;
            virtual void onItemProgress(int64_t duration, int64_t time) = 0;
            virtual void onItemComplete(int64_t position) = 0;
            virtual void onItemPlaying() = 0;
            virtual void onItemPaused(int64_t position) = 0;
        };


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
        class SoundPlayer : public AbstractPlugin, public ISoundListener {
        private:

            // We do not allow this plugin to be copied !!
            SoundPlayer(const SoundPlayer&) = delete;
            SoundPlayer& operator=(const SoundPlayer&) = delete;

            //Begin methods
            uint32_t setPropertiesWrapper(const JsonObject& parameters, JsonObject& response);

            uint32_t setUrlWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setVolumeWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setPositionWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setAutoPlayWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getPropertiesWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getUrlWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getVolumeWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getPositionWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getAutoPlayWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t playWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t pauseWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getHeadersWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setHeadersWrapper(const JsonObject& parameters, JsonObject& response);
            //End methods

        public:
            SoundPlayer();
            virtual ~SoundPlayer();

            void terminate();

        public:
            static SoundPlayer* _instance;
        private:

            std::string m_url;
            unsigned m_volume;
            unsigned m_position;
            unsigned m_currentPosition;
            bool m_autoPlay;
            std::map <std::string, std::string> m_headers;

            bool m_itemCreated;
            bool m_itemReady;

            CGStreamerSoundPlayer* m_item;

            void createSoundItem();
            void onServiceSetUrl(const std::string& url);
            void onServiceSetVolume(unsigned volume);
            void onServiceSetPosition(unsigned position);
            void onServiceSetAutoPlay(bool autoPlay);
            void onServicePlay();
            void onServicePause();

            void onItemOpen();
            void onItemError(std::string error);
            void onItemProgress(int64_t duration, int64_t time);
            void onItemComplete(int64_t position);
            void onItemPlaying();
            void onItemPaused(int64_t position);
        };
	} // namespace Plugin
} // namespace WPEFramework
