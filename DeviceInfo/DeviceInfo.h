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
#include <interfaces/json/JsonData_DeviceInfo.h>

namespace WPEFramework {
namespace Plugin {

    class DeviceInfo : public PluginHost::IPlugin, public PluginHost::IWeb, public PluginHost::JSONRPC {
    private:
        typedef Core::JSON::EnumType<JsonData::DeviceInfo::DistributoridData::DistributoridType> DistributoridJsonEnum;
        typedef Core::JSON::EnumType<JsonData::DeviceInfo::DevicetypeData::DevicetypeType> DevicetypeJsonEnum;
        typedef Core::JSON::EnumType<JsonData::DeviceInfo::MakeData::MakeType> MakeJsonEnum;
        typedef Core::JSON::EnumType<JsonData::DeviceInfo::ModelidData::SkuType> SkuJsonEnum;

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
                Add(_T("firmwareversion"), &FirmwareVersion);
                Add(_T("serialnumber"), &SerialNumber);
                Add(_T("sku"), &Sku);
                Add(_T("make"), &Make);
                Add(_T("model"), &Model);
                Add(_T("devicetype"), &DeviceType);
                Add(_T("distributorid"), &DistributorId);
            }

            virtual ~Data()
            {
            }

        public:
            Core::JSON::ArrayType<JsonData::DeviceInfo::AddressesData> Addresses;
            JsonData::DeviceInfo::SysteminfoData SystemInfo;
            JsonData::DeviceInfo::SocketinfoData Sockets;
            JsonData::DeviceInfo::FirmwareversionData FirmwareVersion;
            Core::JSON::String SerialNumber;
            SkuJsonEnum Sku;
            MakeJsonEnum Make;
            Core::JSON::String Model;
            DevicetypeJsonEnum DeviceType;
            DistributoridJsonEnum DistributorId;
        };

    private:
        DeviceInfo(const DeviceInfo&) = delete;
        DeviceInfo& operator=(const DeviceInfo&) = delete;

    public:
        DeviceInfo()
            : _skipURL(0)
            , _service(nullptr)
            , _subSystem(nullptr)
            , _systemId()
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
        uint32_t get_firmwareversion(JsonData::DeviceInfo::FirmwareversionData& response) const;
        uint32_t get_serialnumber(JsonData::DeviceInfo::SerialnumberData& response) const;
        uint32_t get_modelid(JsonData::DeviceInfo::ModelidData& response) const;
        uint32_t get_make(JsonData::DeviceInfo::MakeData& response) const;
        uint32_t get_modelname(JsonData::DeviceInfo::ModelnameData& response) const;
        uint32_t get_devicetype(JsonData::DeviceInfo::DevicetypeData& response) const;
        uint32_t get_distributorid(JsonData::DeviceInfo::DistributoridData& response) const;

        void SysInfo(JsonData::DeviceInfo::SysteminfoData& systemInfo) const;
        void AddressInfo(Core::JSON::ArrayType<JsonData::DeviceInfo::AddressesData>& addressInfo) const;
        void SocketPortInfo(JsonData::DeviceInfo::SocketinfoData& socketPortInfo) const;
        void FirmwareVersion(JsonData::DeviceInfo::FirmwareversionData& firmwareVersion) const;
        void SerialNumber(Core::JSON::String& serialNumber) const;
        void Sku(SkuJsonEnum& sku) const;
        void Make(MakeJsonEnum& make) const;
        void Model(Core::JSON::String& model) const;
        void DeviceType(DevicetypeJsonEnum& deviceType) const;
        void DistributorId(DistributoridJsonEnum& distributorId) const;

    private:
        uint8_t _skipURL;
        PluginHost::IShell* _service;
        PluginHost::ISubSystem* _subSystem;
        string _systemId;
    };

} // namespace Plugin
} // namespace WPEFramework

#endif // DEVICEINFO_DEVICEINFO_H
