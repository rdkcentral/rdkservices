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
#include <interfaces/IPlayerInfo.h>
#include <interfaces/json/JsonData_PlayerInfo.h>

namespace WPEFramework {
namespace Plugin {

    class PlayerInfo : public PluginHost::IPlugin, public PluginHost::IWeb, public PluginHost::JSONRPC {
    public:
        PlayerInfo(const PlayerInfo&) = delete;
        PlayerInfo& operator=(const PlayerInfo&) = delete;

        PlayerInfo()
            : _skipURL(0)
            , _connectionId(0)
            , _player(nullptr)
            , _audioCodecs(nullptr)
            , _videoCodecs(nullptr)
        {
            RegisterAll();
        }

        virtual ~PlayerInfo()
        {
            UnregisterAll();
        }

        BEGIN_INTERFACE_MAP(PlayerInfo)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IWeb)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        END_INTERFACE_MAP

    public:
        //   IPlugin methods
        // -------------------------------------------------------------------------------------------------------
        virtual const string Initialize(PluginHost::IShell* service) override;
        virtual void Deinitialize(PluginHost::IShell* service) override;
        virtual string Information() const override;

        //   IWeb methods
        // -------------------------------------------------------------------------------------------------------
        virtual void Inbound(Web::Request& request) override;
        virtual Core::ProxyType<Web::Response> Process(const Web::Request& request) override;

    private:
        // JsonRpc
        void RegisterAll();
        void UnregisterAll();
        uint32_t get_playerinfo(JsonData::PlayerInfo::CodecsData&) const;

        void Info(JsonData::PlayerInfo::CodecsData&) const;

    private:
        uint8_t _skipURL;
        uint32_t _connectionId;
        Exchange::IPlayerProperties* _player;

        Exchange::IPlayerProperties::IAudioIterator* _audioCodecs;
        Exchange::IPlayerProperties::IVideoIterator* _videoCodecs;
    };

} // namespace Plugin
} // namespace WPEFramework
