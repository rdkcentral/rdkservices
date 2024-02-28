/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
 */

#pragma once

#include "Module.h"
#include <interfaces/json/JsonData_PlayerInfo.h>
#include <interfaces/json/JDolbyOutput.h>
#include <interfaces/json/JPlayerProperties.h>

namespace WPEFramework {
namespace Plugin {

    class PlayerInfo : public PluginHost::IPlugin, public PluginHost::IWeb, public PluginHost::JSONRPC {
    private:
        class Notification : public RPC::IRemoteConnection::INotification {
        public:
            Notification() = delete;
            Notification(const Notification&) = delete;
            Notification& operator=(const Notification&) = delete;

            explicit Notification(PlayerInfo* parent)
                : _parent(*parent)
            {
                ASSERT(parent != nullptr);
            }
            ~Notification() override = default;

        public:
            void Activated(RPC::IRemoteConnection* /* connection */) override
            {
            }
            void Deactivated(RPC::IRemoteConnection* connection) override
            {
                _parent.Deactivated(connection);
            }
#ifdef USE_THUNDER_R4
            void Terminated(RPC::IRemoteConnection* /* connection */) override
            {
            }
#endif

            BEGIN_INTERFACE_MAP(Notification)
                INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
            END_INTERFACE_MAP

        private:
            PlayerInfo& _parent;
        };
        class DolbyNotification : protected Exchange::Dolby::IOutput::INotification {
        public:
            DolbyNotification() = delete;
            DolbyNotification(const DolbyNotification&) = delete;
            DolbyNotification& operator=(const DolbyNotification&) = delete;

            explicit DolbyNotification(PlayerInfo* parent)
                : _parent(*parent)
                , _client(nullptr)
            {
                ASSERT(parent != nullptr);
            }
            ~DolbyNotification() override = default;

            void Initialize(Exchange::Dolby::IOutput* client)
            {
                ASSERT(client != nullptr);
                _client = client;
                _client->AddRef();
                _client->Register(this);
            }
            void Deinitialize()
            {
                ASSERT(_client != nullptr);
                if (_client != nullptr) {
                    _client->Unregister(this);
                    _client->Release();
                    _client = nullptr;
                }
            }
            void AudioModeChanged(const Exchange::Dolby::IOutput::SoundModes mode, bool enabled) override
            {
                Exchange::Dolby::JOutput::Event::AudioModeChanged(_parent, mode, enabled);
            }
            BEGIN_INTERFACE_MAP(DolbyNotification)
            INTERFACE_ENTRY(Exchange::Dolby::IOutput::INotification)
            END_INTERFACE_MAP

        private:
            PlayerInfo& _parent;
            Exchange::Dolby::IOutput* _client;
        };

    public:
        PlayerInfo(const PlayerInfo&) = delete;
        PlayerInfo& operator=(const PlayerInfo&) = delete;

        PlayerInfo()
            : _skipURL(0)
            , _connectionId(0)
            , _player(nullptr)
            , _audioCodecs(nullptr)
            , _videoCodecs(nullptr)
            , _dolbyOut(nullptr)
            , _dolbyNotification(this)
            , _notification(this)
            , _service(nullptr)
        {
        }

        ~PlayerInfo() override = default;

        BEGIN_INTERFACE_MAP(PlayerInfo)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IWeb)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        INTERFACE_AGGREGATE(Exchange::IPlayerProperties, _player)
        END_INTERFACE_MAP

    public:
        //   IPlugin methods
        // -------------------------------------------------------------------------------------------------------
        const string Initialize(PluginHost::IShell* service) override;
        void Deinitialize(PluginHost::IShell* service) override;
        string Information() const override;

        //   IWeb methods
        // -------------------------------------------------------------------------------------------------------
        void Inbound(Web::Request& request) override;
        Core::ProxyType<Web::Response> Process(const Web::Request& request) override;

    private:
        void Deactivated(RPC::IRemoteConnection* connection);
        void Info(JsonData::PlayerInfo::CodecsData&) const;

    private:
        uint8_t _skipURL;
        uint32_t _connectionId;

        Exchange::IPlayerProperties* _player;
        Exchange::IPlayerProperties::IAudioCodecIterator* _audioCodecs;
        Exchange::IPlayerProperties::IVideoCodecIterator* _videoCodecs;
        Exchange::Dolby::IOutput* _dolbyOut;
        Core::Sink<DolbyNotification> _dolbyNotification;
        Core::Sink<Notification> _notification;
        PluginHost::IShell* _service;
    };

} // namespace Plugin
} // namespace WPEFramework
