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
#include <interfaces/IMediaPlayer.h>

namespace WPEFramework {
    namespace Plugin {

        class FireboltMediaPlayer : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        private:
            FireboltMediaPlayer(const FireboltMediaPlayer&) = delete;
            FireboltMediaPlayer& operator=(const FireboltMediaPlayer&) = delete;

            class Notification : public RPC::IRemoteConnection::INotification {
            private:
                Notification() = delete;
                Notification(const Notification&) = delete;
                Notification& operator=(const Notification&) = delete;

            public:
                explicit Notification(FireboltMediaPlayer* parent)
                : _parent(*parent)
                {
                    ASSERT(parent != nullptr);
                }
                virtual ~Notification()
                {
                }

            public:
                virtual void Activated(RPC::IRemoteConnection* connection)
                {
                }
                virtual void Deactivated(RPC::IRemoteConnection* connection)
                {
                    _parent.Deactivated(connection);
                }

                BEGIN_INTERFACE_MAP(Notification)
                INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
                END_INTERFACE_MAP

            private:
                FireboltMediaPlayer& _parent;
            };

            public:

            /**
             * @brief A proxy for the out-of-process implementation of IMediaStream.
             *
             * The reference count of this instance is effectively the reference count of the out-of-process
             * instance.
             *
             */
            class MediaStreamProxy
            {

            private:
                class MediaStreamSink : public Exchange::IMediaPlayer::IMediaStream::INotification
                {
                private:
                    MediaStreamSink() = delete;
                    MediaStreamSink(const MediaStreamSink&) = delete;
                    MediaStreamSink& operator=(const MediaStreamSink&) = delete;

                public:
                    MediaStreamSink(MediaStreamProxy* parent)
                    : _parent(*parent)
                    {
                            ASSERT(parent != nullptr);
                    }

                    ~MediaStreamSink() override
                    {

                    }

                    void Event(const string &eventName, const string &parametersJson) override
                    {
                        _parent.OnEvent(eventName, parametersJson);
                    }

                    BEGIN_INTERFACE_MAP(MediaStreamSink)
                    INTERFACE_ENTRY(Exchange::IMediaPlayer::IMediaStream::INotification)
                    END_INTERFACE_MAP

                private:
                    MediaStreamProxy& _parent;
                };

            public:
                MediaStreamProxy() = delete;
                MediaStreamProxy(const MediaStreamProxy&) = delete;
                MediaStreamProxy& operator=(const MediaStreamProxy&) = delete;

                MediaStreamProxy(FireboltMediaPlayer& parent, const string& id,
                        Exchange::IMediaPlayer::IMediaStream* implementation)
                : _parent(parent), _id(id), _implementation(implementation),
                  _mediaPlayerSink(this)
                {
                    ASSERT(implementation != nullptr);
                   _implementation->Register(&_mediaPlayerSink);
                }

                virtual ~MediaStreamProxy();

                Exchange::IMediaPlayer::IMediaStream* Stream() {
                    return (_implementation);
                }
                const Exchange::IMediaPlayer::IMediaStream* Stream() const {
                    return (_implementation);
                }

                uint32_t Release();

                void OnEvent(const string &eventName, const string &parametersJson)
                {
                    _parent.onMediaStreamEvent(_id, eventName, parametersJson);
                }

           private:
                FireboltMediaPlayer& _parent;
                string _id;
                Exchange::IMediaPlayer::IMediaStream *_implementation;
                Core::Sink<MediaStreamSink> _mediaPlayerSink;
            };

            typedef std::map<string, MediaStreamProxy*> MediaStreams;

            public:
            FireboltMediaPlayer();
            ~FireboltMediaPlayer() override;

            // JsonRpc handlers
            uint32_t create(const JsonObject& parameters, JsonObject& response);
            uint32_t release(const JsonObject& parameters, JsonObject& response);
            uint32_t load(const JsonObject& parameters, JsonObject& response);
            uint32_t play(const JsonObject& parameters, JsonObject& response);
            uint32_t pause(const JsonObject& parameters, JsonObject& response);
            uint32_t seek(const JsonObject& parameters, JsonObject& response);
            uint32_t stop(const JsonObject& parameters, JsonObject& response);
            uint32_t initConfig(const JsonObject& parameters, JsonObject& response);
            uint32_t setDRMConfig(const JsonObject& parameters, JsonObject& response);

            void onMediaStreamEvent(const string& id, const string &eventName, const string &parameters);

            public:
            BEGIN_INTERFACE_MAP(FireboltMediaPlayer)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

            // IPlugin methods
            const string Initialize(PluginHost::IShell* service) override;
            void Deinitialize(PluginHost::IShell* service) override;
            string Information() const override;

            private:
            void Deactivated(RPC::IRemoteConnection* connection);

            // JsonRpc
            void RegisterAll();
            void UnregisterAll();

            private:

            PluginHost::IShell* _service;
            Core::Sink<Notification> _notification;

            // Media Player with AAMP implementation running in separate process
            //TODO: consider list of different MediaPlayers
            uint32_t _aampMediaPlayerConnectionId;
            Exchange::IMediaPlayer* _aampMediaPlayer;
            //* The current MediaStreamProxy instances by name. When their reference count reaches zero they are removed from this map.
            MediaStreams _mediaStreams;
        };

    } //namespace Plugin
} //namespace WPEFramework
