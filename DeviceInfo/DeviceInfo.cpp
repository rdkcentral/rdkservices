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

#include "DeviceInfo.h"

#include <fstream>
#include <regex>

#include "rfcapi.h"

namespace WPEFramework {
namespace Plugin {

    typedef Core::EnumerateType<JsonData::DeviceInfo::DistributoridData::DistributoridType> DistributoridEnum;
    typedef Core::EnumerateType<JsonData::DeviceInfo::DevicetypeData::DevicetypeType> DevicetypeEnum;
    typedef Core::EnumerateType<JsonData::DeviceInfo::MakeData::MakeType> MakeEnum;
    typedef Core::EnumerateType<JsonData::DeviceInfo::ModelidData::SkuType> SkuEnum;
    typedef Core::EnumerateType<JsonData::DeviceInfo::FirmwareversionData::YoctoType> YoctoEnum;

    namespace {
        constexpr auto* kDevicePropsFile = _T("/etc/device.properties");
        constexpr auto* kAuthServiceFile = _T("/etc/authService.conf");
        constexpr auto* kVersionFile = _T("/version.txt");
        constexpr auto* kSerialNumberFile = _T("/proc/device-tree/serial-number");
        constexpr auto* kPartnerIdFile = _T("/opt/www/authService/partnerId3.dat");
        constexpr auto* kRfcPartnerId = _T("Device.DeviceInfo.X_RDKCENTRAL-COM_Syndication.PartnerId");
        constexpr auto* kRfcModelName = _T("Device.DeviceInfo.ModelName");
        constexpr auto* kRfcSerialNumber = _T("Device.DeviceInfo.SerialNumber");
        constexpr auto* kDeviceType = _T("deviceType");
        constexpr auto* kFriendlyId = _T("FRIENDLY_ID");
        constexpr auto* kMfgName = _T("MFG_NAME");
        constexpr auto* kModelNum = _T("MODEL_NUM");
        constexpr auto* kImagename = _T("imagename");
        constexpr auto* kYoctoVersion = _T("YOCTO_VERSION");
        constexpr auto* kSdkVersion = _T("SDK_VERSION");
        constexpr auto* kMediariteVersion = _T("MEDIARITE");
    }

    SERVICE_REGISTRATION(DeviceInfo, 1, 0);

    static Core::ProxyPoolType<Web::JSONBodyType<DeviceInfo::Data>> jsonResponseFactory(4);

    /* virtual */ const string DeviceInfo::Initialize(PluginHost::IShell* service)
    {
        ASSERT(_service == nullptr);
        ASSERT(service != nullptr);

        ASSERT(_subSystem == nullptr);

        Config config;
        config.FromString(service->ConfigLine());
        _skipURL = static_cast<uint8_t>(service->WebPrefix().length());
        _subSystem = service->SubSystems();
        _service = service;
        _systemId = Core::SystemInfo::Instance().Id(Core::SystemInfo::Instance().RawDeviceId(), ~0);

        ASSERT(_subSystem != nullptr);

        // On success return empty, to indicate there is no error text.

        return (_subSystem != nullptr) ? EMPTY_STRING : _T("Could not retrieve System Information.");
    }

    /* virtual */ void DeviceInfo::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);

        if (_subSystem != nullptr) {
            _subSystem->Release();
            _subSystem = nullptr;
        }

        _service = nullptr;
    }

    /* virtual */ string DeviceInfo::Information() const
    {
        // No additional info to report.
        return (string());
    }

    /* virtual */ void DeviceInfo::Inbound(Web::Request& /* request */)
    {
    }

    /* virtual */ Core::ProxyType<Web::Response> DeviceInfo::Process(const Web::Request& request)
    {
        ASSERT(_skipURL <= request.Path.length());

        Core::ProxyType<Web::Response> result(PluginHost::IFactories::Instance().Response());

        // By default, we assume everything works..
        result->ErrorCode = Web::STATUS_OK;
        result->Message = "OK";

        // <GET> - currently, only the GET command is supported, returning system info
        if (request.Verb == Web::Request::HTTP_GET) {

            Core::ProxyType<Web::JSONBodyType<Data>> response(jsonResponseFactory.Element());

            Core::TextSegmentIterator index(Core::TextFragment(request.Path, _skipURL, static_cast<uint32_t>(request.Path.length()) - _skipURL), false, '/');

            // Always skip the first one, it is an empty part because we start with a '/' if there are more parameters.
            index.Next();

            if (index.Next() == false) {
                AddressInfo(response->Addresses);
                SysInfo(response->SystemInfo);
                SocketPortInfo(response->Sockets);
                FirmwareVersion(response->FirmwareVersion);
                SerialNumber(response->SerialNumber);
                Sku(response->Sku);
                Make(response->Make);
                Model(response->Model);
                DeviceType(response->DeviceType);
                DistributorId(response->DistributorId);
            } else if (index.Current() == "Adresses") {
                AddressInfo(response->Addresses);
            } else if (index.Current() == "System") {
                SysInfo(response->SystemInfo);
            } else if (index.Current() == "Sockets") {
                SocketPortInfo(response->Sockets);
            }
            // TODO RB: I guess we should do something here to return other info (e.g. time) as well.

            result->ContentType = Web::MIMETypes::MIME_JSON;
            result->Body(Core::proxy_cast<Web::IBody>(response));
        } else {
            result->ErrorCode = Web::STATUS_BAD_REQUEST;
            result->Message = _T("Unsupported request for the [DeviceInfo] service.");
        }

        return result;
    }

    void DeviceInfo::SysInfo(JsonData::DeviceInfo::SysteminfoData& systemInfo) const
    {
        Core::SystemInfo& singleton(Core::SystemInfo::Instance());

        systemInfo.Time = Core::Time::Now().ToRFC1123(true);
        systemInfo.Version = _service->Version() + _T("#") + _subSystem->BuildTreeHash();
        systemInfo.Uptime = singleton.GetUpTime();
        systemInfo.Freeram = singleton.GetFreeRam();
        systemInfo.Totalram = singleton.GetTotalRam();
        systemInfo.Devicename = singleton.GetHostName();
        systemInfo.Cpuload = Core::NumberType<uint32_t>(static_cast<uint32_t>(singleton.GetCpuLoad())).Text();
        systemInfo.Serialnumber = _systemId;
    }

    void DeviceInfo::AddressInfo(Core::JSON::ArrayType<JsonData::DeviceInfo::AddressesData>& addressInfo) const
    {
        // Get the point of entry on WPEFramework..
        Core::AdapterIterator interfaces;

        while (interfaces.Next() == true) {

            JsonData::DeviceInfo::AddressesData newElement;
            newElement.Name = interfaces.Name();
            newElement.Mac = interfaces.MACAddress(':');
            JsonData::DeviceInfo::AddressesData& element(addressInfo.Add(newElement));

            // get an interface with a public IP address, then we will have a proper MAC address..
            Core::IPV4AddressIterator selectedNode(interfaces.IPV4Addresses());

            while (selectedNode.Next() == true) {
                Core::JSON::String nodeName;
                nodeName = selectedNode.Address().HostAddress();

                element.Ip.Add(nodeName);
            }
        }
    }

    void DeviceInfo::SocketPortInfo(JsonData::DeviceInfo::SocketinfoData& socketPortInfo) const
    {
        socketPortInfo.Runs = Core::ResourceMonitor::Instance().Runs();
    }

    void DeviceInfo::FirmwareVersion(JsonData::DeviceInfo::FirmwareversionData& firmwareVersion) const
    {
        std::ifstream file(kVersionFile);

        if (file) {
            string line;

            while (std::getline(file, line)) {
                if (line.rfind(kImagename, 0) == 0) {
                    firmwareVersion.Imagename = line.substr(line.find(':') + 1);
                } else if (line.rfind(kYoctoVersion, 0) == 0) {
                    firmwareVersion.Yocto = YoctoEnum(line.substr(line.find('=') + 1).c_str()).Value();
                } else if (line.rfind(kSdkVersion, 0) == 0) {
                    firmwareVersion.Sdk = line.substr(line.find('=') + 1);
                } else if (line.rfind(kMediariteVersion, 0) == 0) {
                    firmwareVersion.Mediarite = line.substr(line.find('=') + 1);
                }
            }
        }
    }

    void DeviceInfo::SerialNumber(Core::JSON::String& serialNumber) const
    {
        RFC_ParamData_t param;

        auto status = getRFCParameter(nullptr, kRfcSerialNumber, &param);

        if (status == WDMP_SUCCESS) {
            serialNumber = param.value;
        } else {
            std::ifstream file(kSerialNumberFile);

            if (file) {
                string line;
                if (std::getline(file, line)) {
                    serialNumber = line;
                }
            }
        }
    }

    void DeviceInfo::Sku(SkuJsonEnum& sku) const
    {
        RFC_ParamData_t param;

        auto status = getRFCParameter(nullptr, kRfcModelName, &param);

        if (status == WDMP_SUCCESS) {
            sku = SkuEnum(param.value).Value();
        } else {
            std::ifstream file(kDevicePropsFile);

            if (file) {
                string line;
                while (std::getline(file, line)) {
                    if (line.rfind(kModelNum, 0) == 0) {
                        sku = SkuEnum(line.substr(line.find('=') + 1).c_str()).Value();

                        break;
                    }
                }
            }
        }
    }

    void DeviceInfo::Make(MakeJsonEnum& make) const
    {
        std::ifstream file(kDevicePropsFile);

        if (file) {
            string line;
            while (std::getline(file, line)) {
                if (line.rfind(kMfgName, 0) == 0) {
                    make = MakeEnum(line.substr(line.find('=') + 1).c_str()).Value();

                    break;
                }
            }
        }
    }

    void DeviceInfo::Model(Core::JSON::String& model) const
    {
        std::ifstream file(kDevicePropsFile);

        if (file) {
            string line;
            while (std::getline(file, line)) {
                if (line.rfind(kFriendlyId, 0) == 0) {
                    // trim quotes

                    model = std::regex_replace(line, std::regex(_T("^\\w+=(?:\")?([^\"\\n]+)(?:\")?$")), _T("$1"));

                    break;
                }
            }
        }
    }

    void DeviceInfo::DeviceType(DevicetypeJsonEnum& deviceType) const
    {
        std::ifstream file(kAuthServiceFile);

        if (file) {
            string line;
            while (std::getline(file, line)) {
                if (line.rfind(kDeviceType, 0) == 0) {
                    deviceType = DevicetypeEnum(line.substr(line.find('=') + 1).c_str()).Value();

                    break;
                }
            }
        }
    }

    void DeviceInfo::DistributorId(DistributoridJsonEnum& distributorId) const
    {
        RFC_ParamData_t param;

        auto status = getRFCParameter(nullptr, kRfcPartnerId, &param);

        if (status == WDMP_SUCCESS) {
            distributorId = DistributoridEnum(param.value).Value();
        } else {
            std::ifstream file(kPartnerIdFile);

            if (file) {
                string line;
                if (std::getline(file, line)) {
                    distributorId = DistributoridEnum(line.c_str()).Value();
                }
            }
        }
    }

} // namespace Plugin
} // namespace WPEFramework
