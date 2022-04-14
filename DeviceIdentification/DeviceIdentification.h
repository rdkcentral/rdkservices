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
#include <interfaces/IDeviceIdentification.h>
#include <interfaces/json/JsonData_DeviceIdentification.h>

namespace WPEFramework {
namespace Plugin {

    class DeviceIdentification : public PluginHost::IPlugin, public PluginHost::JSONRPC {
    public:
        DeviceIdentification(const DeviceIdentification&) = delete;
        DeviceIdentification& operator=(const DeviceIdentification&) = delete;

        class Notification : public RPC::IRemoteConnection::INotification {
        public:
            Notification(const Notification&) = delete;
            Notification& operator=(const Notification&) = delete;

            explicit Notification(DeviceIdentification* parent)
                : _parent(*parent)
            {
                ASSERT(parent != nullptr);
            }
            ~Notification() override
            {
                TRACE(Trace::Information, (_T("DeviceIdentification::Notification destructed. Line: %d"), __LINE__));
            }

        public:
            void Activated(RPC::IRemoteConnection* /* connection */) override
            {
            }
            void Deactivated(RPC::IRemoteConnection* connectionId) override
            {
                _parent.Deactivated(connectionId);
            }

            BEGIN_INTERFACE_MAP(Notification)
            INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
            END_INTERFACE_MAP

        private:
            DeviceIdentification& _parent;
        };

        DeviceIdentification()
            : _service(nullptr)
            , _deviceId()
            , _identifier(nullptr)
            , _connectionId(0)
            , _notification(this)
        {
        }

        ~DeviceIdentification() override = default;

        BEGIN_INTERFACE_MAP(DeviceIdentification)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        END_INTERFACE_MAP

    public:
        //   IPlugin methods
        // -------------------------------------------------------------------------------------------------------
        const string Initialize(PluginHost::IShell* service) override;
        void Deinitialize(PluginHost::IShell* service) override;
        string Information() const override;

    private:
        void RegisterAll();
        void UnregisterAll();
        uint32_t get_deviceidentification(JsonData::DeviceIdentification::DeviceidentificationData& response) const;

        string GetDeviceId() const;
        void Info(JsonData::DeviceIdentification::DeviceidentificationData&) const;

        void Deactivated(RPC::IRemoteConnection* connection);

    private:
        PluginHost::IShell* _service;
        string _deviceId;
        PluginHost::ISubSystem::IIdentifier* _identifier;

        uint32_t _connectionId;
         Core::Sink<Notification> _notification;
    };

} // namespace Plugin
} // namespace WPEFramework
