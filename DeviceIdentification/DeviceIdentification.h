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

        DeviceIdentification()
            : _deviceId()
            , _device(nullptr)
            , _identifier(nullptr)
            , _connectionId(0)
        {
            RegisterAll();
        }

        virtual ~DeviceIdentification()
        {
            UnregisterAll();
        }

        BEGIN_INTERFACE_MAP(DeviceIdentification)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
        END_INTERFACE_MAP

    public:
        //   IPlugin methods
        // -------------------------------------------------------------------------------------------------------
        virtual const string Initialize(PluginHost::IShell* service) override;
        virtual void Deinitialize(PluginHost::IShell* service) override;
        virtual string Information() const override;

    private:
        void RegisterAll();
        void UnregisterAll();
        uint32_t get_deviceidentification(JsonData::DeviceIdentification::DeviceidentificationData& response) const;

        string GetDeviceId() const;
        void Info(JsonData::DeviceIdentification::DeviceidentificationData&) const;

    private:
        string _deviceId;
        Exchange::IDeviceProperties* _device;
        const PluginHost::ISubSystem::IIdentifier* _identifier;

        uint32_t _connectionId;
    };

} // namespace Plugin
} // namespace WPEFramework
