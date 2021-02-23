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

#ifndef DEVICEINFO_DEVICEINFO_H
#define DEVICEINFO_DEVICEINFO_H

#include "Module.h"
#include <interfaces/IDeviceInfo.h>
#include <interfaces/json/JsonData_DeviceInfo.h>

namespace WPEFramework {
namespace Plugin {

    class DeviceInfo : public PluginHost::IPlugin, public PluginHost::IWeb, public PluginHost::JSONRPC {
    public:
        class Data : public Core::JSON::Container {
        public:
            Data()
                : Core::JSON::Container()
                , Addresses()
                , SystemInfo()
            {
                Add(_T("addresses"), &Addresses);
                Add(_T("systeminfo"), &SystemInfo);
                Add(_T("sockets"), &Sockets);
            }

            virtual ~Data()
            {
            }

        public:
            Core::JSON::ArrayType<JsonData::DeviceInfo::AddressesData> Addresses;
            JsonData::DeviceInfo::SysteminfoData SystemInfo;
            JsonData::DeviceInfo::SocketinfoData Sockets;
        };

    private:
        DeviceInfo(const DeviceInfo&) = delete;
        DeviceInfo& operator=(const DeviceInfo&) = delete;

        uint32_t addresses(const Core::JSON::String& parameters, Core::JSON::ArrayType<JsonData::DeviceInfo::AddressesData>& response)
        {
            AddressInfo(response);
            return (Core::ERROR_NONE);
        }
        uint32_t system(const Core::JSON::String& parameters, JsonData::DeviceInfo::SysteminfoData& response)
        {
            SysInfo(response);
            return (Core::ERROR_NONE);
        }
        uint32_t sockets(const Core::JSON::String& parameters, JsonData::DeviceInfo::SocketinfoData& response)
        {
            SocketPortInfo(response);
            return (Core::ERROR_NONE);
        }

    public:
        DeviceInfo()
            : _skipURL(0)
            , _service(nullptr)
            , _subSystem(nullptr)
            , _systemId()
            , _deviceId()
            , _implementation(nullptr)
            , _connectionId(0)
        {
            RegisterAll();
        }

        virtual ~DeviceInfo()
        {
            UnregisterAll();
        }

        BEGIN_INTERFACE_MAP(DeviceInfo)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IWeb)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        INTERFACE_AGGREGATE(Exchange::IDeviceCapabilities, _implementation)
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
        uint32_t get_systeminfo(JsonData::DeviceInfo::SysteminfoData& response) const;
        uint32_t get_addresses(Core::JSON::ArrayType<JsonData::DeviceInfo::AddressesData>& response) const;
        uint32_t get_socketinfo(JsonData::DeviceInfo::SocketinfoData& response) const;
        uint32_t get_capabilities(JsonData::DeviceInfo::CapabilitiesData& response) const;

        void SysInfo(JsonData::DeviceInfo::SysteminfoData& systemInfo) const;
        void AddressInfo(Core::JSON::ArrayType<JsonData::DeviceInfo::AddressesData>& addressInfo) const;
        void SocketPortInfo(JsonData::DeviceInfo::SocketinfoData& socketPortInfo) const;
        void CapabilitiesInfo(JsonData::DeviceInfo::CapabilitiesData& response) const;
        string GetDeviceId() const;

    private:
        uint8_t _skipURL;
        PluginHost::IShell* _service;
        PluginHost::ISubSystem* _subSystem;
        string _systemId;
        mutable string _deviceId;
        Exchange::IDeviceCapabilities* _implementation;
        uint32_t _connectionId;

    };

} // namespace Plugin
} // namespace WPEFramework

#endif // DEVICEINFO_DEVICEINFO_H
