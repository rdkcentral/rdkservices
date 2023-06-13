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

/**
 * @file SystemAudioPlayer.h
 * @brief Thunder Plugin based Implementation for SAP service API's (RDK-27957).
 */

/**
  @mainpage SystemAudioPlayer (SAP)

  <b>SystemAudioPlayer</b> SAP Thunder Service provides APIs for the arbitrators
  
  */

#pragma once

#include "Module.h"
#include "tracing/Logging.h"

#include "SystemAudioPlayerImplementation.h"

namespace WPEFramework {
namespace Plugin {

    class SystemAudioPlayer: public PluginHost::IPlugin, public PluginHost::JSONRPC {
    public:
        class Notification : public RPC::IRemoteConnection::INotification,
                             public Exchange::ISystemAudioPlayer::INotification {
            private:
                Notification() = delete;
                Notification(const Notification&) = delete;
                Notification& operator=(const Notification&) = delete;

            public:
                explicit Notification(SystemAudioPlayer* parent)
                    : _parent(*parent) {
                    ASSERT(parent != nullptr);
                }

                virtual ~Notification() {
                }

            public:
                
                virtual void OnSAPEvents(const string &data) {
                    _parent.dispatchJsonEvent("onsapevents", data);
                }

                virtual void Activated(RPC::IRemoteConnection* /* connection */) final
                {
                    SAPLOG_WARNING("SystemAudioPlayer::Notification::Activated - %p", this);
                }

                virtual void Deactivated(RPC::IRemoteConnection* connection) final
                {
                    SAPLOG_WARNING("SystemAudioPlayer::Notification::Deactivated - %p", this);
                    _parent.Deactivated(connection);
                }

                BEGIN_INTERFACE_MAP(Notification)
                INTERFACE_ENTRY(Exchange::ISystemAudioPlayer::INotification)
                INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
                END_INTERFACE_MAP

            private:
                    SystemAudioPlayer& _parent;
        };

        BEGIN_INTERFACE_MAP(SystemAudioPlayer)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        INTERFACE_AGGREGATE(Exchange::ISystemAudioPlayer, _sap)
        END_INTERFACE_MAP

    public:
        SystemAudioPlayer();
        virtual ~SystemAudioPlayer();
        virtual const string Initialize(PluginHost::IShell* service) override;
        virtual void Deinitialize(PluginHost::IShell* service) override;
        virtual string Information() const override { return {}; }

    private:
        // We do not allow this plugin to be copied !!
        SystemAudioPlayer(const SystemAudioPlayer&) = delete;
        SystemAudioPlayer& operator=(const SystemAudioPlayer&) = delete;

        void RegisterAll();

        uint32_t Open(const JsonObject& parameters, JsonObject& response);
        uint32_t Play(const JsonObject& parameters, JsonObject& response);
        uint32_t PlayBuffer(const JsonObject& parameters, JsonObject& response);
        uint32_t Pause(const JsonObject& parameters, JsonObject& response);
        uint32_t Resume(const JsonObject& parameters, JsonObject& response);
        uint32_t Stop(const JsonObject& parameters, JsonObject& response);
        uint32_t Close(const JsonObject& parameters, JsonObject& response);
        uint32_t SetMixerLevels(const JsonObject& parameters, JsonObject& response);
        uint32_t SetSmartVolControl(const JsonObject& parameters, JsonObject& response);
        uint32_t IsPlaying(const JsonObject& parameters, JsonObject& response);
	uint32_t Config(const JsonObject& parameters, JsonObject& response);
        uint32_t GetPlayerSessionId(const JsonObject& parameters, JsonObject& response);

        //version number API's
        uint32_t getapiversion(const JsonObject& parameters, JsonObject& response);

        void dispatchJsonEvent(const char *event, const string &data);
        void Deactivated(RPC::IRemoteConnection* connection);

    private:
        uint8_t _skipURL{};
        uint32_t _connectionId{};
        PluginHost::IShell* _service{};
        Exchange::ISystemAudioPlayer* _sap{};
        Core::Sink<Notification> _notification;
        uint32_t _apiVersionNumber;
        
        friend class Notification;
    };

} // namespace Plugin
} // namespace WPEFramework
