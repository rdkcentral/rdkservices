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
#include <interfaces/IFirmwareVersion.h>
#include <interfaces/json/JsonData_DeviceInfo.h>

namespace WPEFramework {
namespace Plugin {

    class DeviceInfo : public PluginHost::IPlugin,
                       public PluginHost::IWeb,
                       public PluginHost::JSONRPC,
                       public Exchange::IDeviceCapabilities,
                       public Exchange::IFirmwareVersion {

    private:
        class Config : public Core::JSON::Container {
        private:
            Config(const Config&) = delete;
            Config& operator=(const Config&) = delete;

        public:
            Config()
            {
                Add(_T("deviceproperties"), &DeviceProperties);
                Add(_T("authserviceconf"), &AuthserviceConf);
                Add(_T("versionfile"), &VersionFile);
                Add(_T("serialnumberfile"), &SerialNumberFile);
                Add(_T("partneridfile"), &PartnerIdFile);
                Add(_T("rfcpartnerid"), &RfcPartnerId);
                Add(_T("rfcmodelname"), &RfcModelName);
                Add(_T("rfcserialnumber"), &RfcSerialNumber);
                Add(_T("defaultaudioport"), &DefaultAudioPort);
            }
            ~Config()
            {
            }

        public:
            Core::JSON::String DeviceProperties;
            Core::JSON::String AuthserviceConf;
            Core::JSON::String VersionFile;
            Core::JSON::String SerialNumberFile;
            Core::JSON::String PartnerIdFile;
            Core::JSON::String RfcPartnerId;
            Core::JSON::String RfcModelName;
            Core::JSON::String RfcSerialNumber;
            Core::JSON::String DefaultAudioPort;
        };

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
        INTERFACE_ENTRY(Exchange::IDeviceCapabilities)
        INTERFACE_ENTRY(Exchange::IFirmwareVersion)
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
        uint32_t get_supportedaudioports(JsonData::DeviceInfo::SupportedaudioportsData& response) const;
        uint32_t get_supportedvideodisplays(JsonData::DeviceInfo::SupportedvideodisplaysData& response) const;
        uint32_t get_hostedid(JsonData::DeviceInfo::HostedidData& response) const;
        uint32_t endpoint_defaultresolution(const JsonData::DeviceInfo::SupportedresolutionsParamsInfo& params, JsonData::DeviceInfo::DefaultresolutionResultData& response) const;
        uint32_t endpoint_supportedresolutions(const JsonData::DeviceInfo::SupportedresolutionsParamsInfo& params, JsonData::DeviceInfo::SupportedresolutionsResultData& response) const;
        uint32_t endpoint_supportedhdcp(const JsonData::DeviceInfo::SupportedresolutionsParamsInfo& params, JsonData::DeviceInfo::SupportedhdcpResultData& response) const;
        uint32_t endpoint_audiocapabilities(const JsonData::DeviceInfo::AudiocapabilitiesParamsInfo& params, JsonData::DeviceInfo::AudiocapabilitiesResultData& response) const;
        uint32_t endpoint_ms12capabilities(const JsonData::DeviceInfo::AudiocapabilitiesParamsInfo& params, JsonData::DeviceInfo::Ms12capabilitiesResultData& response) const;
        uint32_t endpoint_supportedms12audioprofiles(const JsonData::DeviceInfo::AudiocapabilitiesParamsInfo& params, JsonData::DeviceInfo::Supportedms12audioprofilesResultData& response) const;

        void SysInfo(JsonData::DeviceInfo::SysteminfoData& systemInfo) const;
        void AddressInfo(Core::JSON::ArrayType<JsonData::DeviceInfo::AddressesData>& addressInfo) const;
        void SocketPortInfo(JsonData::DeviceInfo::SocketinfoData& socketPortInfo) const;

    private:
        //   IDeviceCapabilities methods
        // -------------------------------------------------------------------------------------------------------
        uint32_t SerialNumber(string& serialNumber) const override;
        uint32_t Sku(string& sku) const override;
        uint32_t Make(string& make) const override;
        uint32_t Model(string& model) const override;
        uint32_t DeviceType(string& deviceType) const override;
        uint32_t DistributorId(string& distributorId) const override;
        uint32_t SupportedAudioPorts(RPC::IStringIterator*& supportedAudioPorts) const override;
        uint32_t SupportedVideoDisplays(RPC::IStringIterator*& supportedVideoDisplays) const override;
        uint32_t HostEDID(string& edid) const override;
        uint32_t DefaultResolution(const string& videoDisplay, string& defaultResolution) const override;
        uint32_t SupportedResolutions(const string& videoDisplay, RPC::IStringIterator*& supportedResolutions) const override;
        uint32_t SupportedHdcp(const string& videoDisplay, Exchange::IDeviceCapabilities::CopyProtection& supportedHDCPVersion) const override;
        uint32_t AudioCapabilities(const string& audioPort, Exchange::IDeviceCapabilities::IAudioCapabilityIterator*& audioCapabilities) const override;
        uint32_t MS12Capabilities(const string& audioPort, Exchange::IDeviceCapabilities::IMS12CapabilityIterator*& ms12Capabilities) const override;
        uint32_t SupportedMS12AudioProfiles(const string& audioPort, RPC::IStringIterator*& supportedMS12AudioProfiles) const override;

        //   IFirmwareVersion methods
        // -------------------------------------------------------------------------------------------------------
        uint32_t Imagename(string& imagename) const override;
        uint32_t Sdk(string& sdk) const override;
        uint32_t Mediarite(string& mediarite) const override;
        uint32_t Yocto(string& yocto) const override;

    private:
        uint8_t _skipURL;
        PluginHost::IShell* _service;
        PluginHost::ISubSystem* _subSystem;
        string _systemId;
        Config _config;
    };

} // namespace Plugin
} // namespace WPEFramework

#endif // DEVICEINFO_DEVICEINFO_H
