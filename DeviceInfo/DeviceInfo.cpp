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

#include "host.hpp"
#include "videoOutputPort.hpp"
#include "videoOutputPortConfig.hpp"

namespace WPEFramework {
namespace Plugin {

    typedef Core::EnumerateType<JsonData::DeviceInfo::DistributoridData::DistributoridType> DistributoridEnum;
    typedef Core::EnumerateType<JsonData::DeviceInfo::DevicetypeData::DevicetypeType> DevicetypeEnum;
    typedef Core::EnumerateType<JsonData::DeviceInfo::MakeData::MakeType> MakeEnum;
    typedef Core::EnumerateType<JsonData::DeviceInfo::ModelidData::SkuType> SkuEnum;
    typedef Core::EnumerateType<JsonData::DeviceInfo::FirmwareversionData::YoctoType> YoctoEnum;
    typedef Core::EnumerateType<JsonData::DeviceInfo::Output_resolutionType> OutputResolutionEnum;

    SERVICE_REGISTRATION(DeviceInfo, 1, 0);

    static Core::ProxyPoolType<Web::JSONBodyType<DeviceInfo::Data>> jsonResponseFactory(4);

    /* virtual */ const string DeviceInfo::Initialize(PluginHost::IShell* service)
    {
        ASSERT(_service == nullptr);
        ASSERT(service != nullptr);

        ASSERT(_subSystem == nullptr);

        _config.FromString(service->ConfigLine());
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

    uint32_t DeviceInfo::FirmwareVersion(JsonData::DeviceInfo::FirmwareversionData& firmwareVersion) const
    {
        uint32_t result = Core::ERROR_GENERAL;

        std::ifstream file(_config.VersionFile.Value());

        if (file) {
            string line;

            while (std::getline(file, line)) {
                if (line.rfind(_T("imagename"), 0) == 0) {
                    firmwareVersion.Imagename = line.substr(line.find(':') + 1);
                    result = Core::ERROR_NONE;
                } else if (line.rfind(_T("YOCTO_VERSION"), 0) == 0) {
                    firmwareVersion.Yocto = YoctoEnum(line.substr(line.find('=') + 1).c_str()).Value();
                } else if (line.rfind(_T("SDK_VERSION"), 0) == 0) {
                    firmwareVersion.Sdk = line.substr(line.find('=') + 1);
                } else if (line.rfind(_T("MEDIARITE"), 0) == 0) {
                    firmwareVersion.Mediarite = line.substr(line.find('=') + 1);
                }
            }
        }

        return result;
    }

    uint32_t DeviceInfo::SerialNumber(Core::JSON::String& serialNumber) const
    {
        uint32_t result = Core::ERROR_GENERAL;

        RFC_ParamData_t param;

        auto status = getRFCParameter(nullptr, _config.RfcSerialNumber.Value().c_str(), &param);

        if (status == WDMP_SUCCESS) {
            serialNumber = param.value;
            result = Core::ERROR_NONE;
        } else {
            std::ifstream file(_config.SerialNumberFile.Value());

            if (file) {
                string line;
                if (std::getline(file, line)) {
                    serialNumber = line;
                    result = Core::ERROR_NONE;
                }
            }
        }

        return result;
    }

    uint32_t DeviceInfo::Sku(SkuJsonEnum& sku) const
    {
        uint32_t result = Core::ERROR_GENERAL;

        RFC_ParamData_t param;

        auto status = getRFCParameter(nullptr, _config.RfcModelName.Value().c_str(), &param);

        if (status == WDMP_SUCCESS) {
            sku = SkuEnum(param.value).Value();
            result = Core::ERROR_NONE;
        } else {
            std::ifstream file(_config.DeviceProperties.Value());

            if (file) {
                string line;
                while (std::getline(file, line)) {
                    if (line.rfind(_T("MODEL_NUM"), 0) == 0) {
                        sku = SkuEnum(line.substr(line.find('=') + 1).c_str()).Value();
                        result = Core::ERROR_NONE;

                        break;
                    }
                }
            }
        }

        return result;
    }

    uint32_t DeviceInfo::Make(MakeJsonEnum& make) const
    {
        uint32_t result = Core::ERROR_GENERAL;

        std::ifstream file(_config.DeviceProperties.Value());

        if (file) {
            string line;
            while (std::getline(file, line)) {
                if (line.rfind(_T("MFG_NAME"), 0) == 0) {
                    make = MakeEnum(line.substr(line.find('=') + 1).c_str()).Value();
                    result = Core::ERROR_NONE;

                    break;
                }
            }
        }

        return result;
    }

    uint32_t DeviceInfo::Model(Core::JSON::String& model) const
    {
        uint32_t result = Core::ERROR_GENERAL;

        std::ifstream file(_config.DeviceProperties.Value());

        if (file) {
            string line;
            while (std::getline(file, line)) {
                if (line.rfind(_T("FRIENDLY_ID"), 0) == 0) {
                    // trim quotes

                    model = std::regex_replace(line, std::regex(_T("^\\w+=(?:\")?([^\"\\n]+)(?:\")?$")), _T("$1"));
                    result = Core::ERROR_NONE;

                    break;
                }
            }
        }

        return result;
    }

    uint32_t DeviceInfo::DeviceType(DevicetypeJsonEnum& deviceType) const
    {
        uint32_t result = Core::ERROR_GENERAL;

        std::ifstream file(_config.AuthserviceConf.Value());

        if (file) {
            string line;
            while (std::getline(file, line)) {
                if (line.rfind(_T("deviceType"), 0) == 0) {
                    deviceType = DevicetypeEnum(line.substr(line.find('=') + 1).c_str()).Value();
                    result = Core::ERROR_NONE;

                    break;
                }
            }
        }

        return result;
    }

    uint32_t DeviceInfo::DistributorId(DistributoridJsonEnum& distributorId) const
    {
        uint32_t result = Core::ERROR_GENERAL;

        RFC_ParamData_t param;

        auto status = getRFCParameter(nullptr, _config.RfcPartnerId.Value().c_str(), &param);

        if (status == WDMP_SUCCESS) {
            distributorId = DistributoridEnum(param.value).Value();
            result = Core::ERROR_NONE;
        } else {
            std::ifstream file(_config.PartnerIdFile.Value());

            if (file) {
                string line;
                if (std::getline(file, line)) {
                    distributorId = DistributoridEnum(line.c_str()).Value();
                    result = Core::ERROR_NONE;
                }
            }
        }

        return result;
    }

    uint32_t DeviceInfo::SupportedAudioPorts(Core::JSON::ArrayType<Core::JSON::String>& supportedAudioPorts) const
    {
        uint32_t result = Core::ERROR_NONE;

        try {
            const auto& aPorts = device::Host::getInstance().getAudioOutputPorts();
            for (size_t i = 0; i < aPorts.size(); i++) {
                supportedAudioPorts.Add() = aPorts.at(i).getName();
            }
        } catch (...) {
            result = Core::ERROR_GENERAL;
        }

        return result;
    }

    uint32_t DeviceInfo::SupportedVideoDisplays(Core::JSON::ArrayType<Core::JSON::String>& supportedVideoDisplays) const
    {
        uint32_t result = Core::ERROR_NONE;

        try {
            const auto& vPorts = device::Host::getInstance().getVideoOutputPorts();
            for (size_t i = 0; i < vPorts.size(); i++) {
                supportedVideoDisplays.Add() = vPorts.at(i).getName();
            }
        } catch (...) {
            result = Core::ERROR_GENERAL;
        }

        return result;
    }

    uint32_t DeviceInfo::HostEDID(Core::JSON::String& edid) const
    {
        uint32_t result = Core::ERROR_NONE;

        vector<uint8_t> edidVec({ 'u', 'n', 'k', 'n', 'o', 'w', 'n' });
        try {
            vector<unsigned char> edidVec2;
            device::Host::getInstance().getHostEDID(edidVec2);
            edidVec = edidVec2;
        } catch (...) {
            result = Core::ERROR_GENERAL;
        }

        if (result == Core::ERROR_NONE) {
            // convert to base64

            uint16_t size = min(edidVec.size(), (size_t)numeric_limits<uint16_t>::max());
            if (edidVec.size() > (size_t)numeric_limits<uint16_t>::max()) {
                result = Core::ERROR_GENERAL;
            } else {
                string base64String;
                Core::ToString((uint8_t*)&edidVec[0], size, true, base64String);
                edid = base64String;
            }
        }

        return result;
    }

    uint32_t DeviceInfo::DefaultResolution(const string& videoDisplay, OutputResolutionJsonEnum& defaultResolution) const
    {
        uint32_t result = Core::ERROR_NONE;

        try {
            auto strVideoPort = videoDisplay.empty() ? device::Host::getInstance().getDefaultVideoPortName() : videoDisplay;
            auto& vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort);
            if (vPort.isDisplayConnected()) {
                defaultResolution = OutputResolutionEnum(vPort.getDefaultResolution().getName().c_str()).Value();
            } else {
                result = Core::ERROR_GENERAL;
            }
        } catch (...) {
            result = Core::ERROR_GENERAL;
        }

        return result;
    }

    uint32_t DeviceInfo::SupportedResolutions(const string& videoDisplay, Core::JSON::ArrayType<OutputResolutionJsonEnum>& supportedResolutions) const
    {
        uint32_t result = Core::ERROR_NONE;

        try {
            auto strVideoPort = videoDisplay.empty() ? device::Host::getInstance().getDefaultVideoPortName() : videoDisplay;
            auto& vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort);
            const auto resolutions = device::VideoOutputPortConfig::getInstance().getPortType(vPort.getType().getId()).getSupportedResolutions();
            for (size_t i = 0; i < resolutions.size(); i++) {
                supportedResolutions.Add() = OutputResolutionEnum(resolutions.at(i).getName().c_str()).Value();
            }
        } catch (...) {
            result = Core::ERROR_GENERAL;
        }

        return result;
    }

    uint32_t DeviceInfo::SupportedHdcp(const string& videoDisplay, CopyProtectionJsonEnum& supportedHDCPVersion) const
    {
        uint32_t result = Core::ERROR_NONE;

        try {
            auto strVideoPort = videoDisplay.empty() ? device::Host::getInstance().getDefaultVideoPortName() : videoDisplay;
            auto& vPort = device::VideoOutputPortConfig::getInstance().getPort(strVideoPort);
            switch (vPort.getHDCPProtocol()) {
            case dsHDCP_VERSION_2X:
                supportedHDCPVersion = JsonData::DeviceInfo::Copy_protectionType::HDCP_22;
                break;
            case dsHDCP_VERSION_1X:
                supportedHDCPVersion = JsonData::DeviceInfo::Copy_protectionType::HDCP_14;
                break;
            default:
                result = Core::ERROR_GENERAL;
            }
        } catch (...) {
            result = Core::ERROR_GENERAL;
        }

        return result;
    }

    uint32_t DeviceInfo::DisplaySettings(const string& videoDisplay, Core::JSON::DecUInt32& colorSpace, Core::JSON::DecUInt32& colorDepth, Core::JSON::DecUInt32& matrixCoefficients, Core::JSON::DecUInt32& videoEOTF, Core::JSON::DecUInt32& quantizationRange) const
    {
        uint32_t result = Core::ERROR_NONE;

        try {
            auto strVideoPort = videoDisplay.empty() ? device::Host::getInstance().getDefaultVideoPortName() : videoDisplay;
            auto& vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort);
            if (vPort.isDisplayConnected()) {
                int _videoEOTF, _matrixCoefficients, _colorSpace, _colorDepth, _quantizationRange;
                vPort.getCurrentOutputSettings(_videoEOTF, _matrixCoefficients, _colorSpace, _colorDepth, _quantizationRange);
                colorSpace = _colorSpace;
                colorDepth = _colorDepth;
                matrixCoefficients = _matrixCoefficients;
                videoEOTF = _videoEOTF;
                quantizationRange = _quantizationRange;
            } else {
                result = Core::ERROR_GENERAL;
            }
        } catch (...) {
            result = Core::ERROR_GENERAL;
        }

        return result;
    }

    uint32_t DeviceInfo::AudioCapabilities(const string& audioPort, Core::JSON::ArrayType<AudioCapabilityJsonEnum>& audioCapabilities) const
    {
        uint32_t result = Core::ERROR_NONE;

        int capabilities = dsAUDIOSUPPORT_NONE;

        try {
            auto strAudioPort = audioPort.empty() ? "HDMI0" : audioPort;
            auto& aPort = device::Host::getInstance().getAudioOutputPort(strAudioPort);
            aPort.getAudioCapabilities(&capabilities);
        } catch (...) {
            result = Core::ERROR_GENERAL;
        }

        if (!capabilities)
            audioCapabilities.Add() = JsonData::DeviceInfo::AudiocapabilitiesResultData::AudiocapabilityType::NONE;
        if (capabilities & dsAUDIOSUPPORT_ATMOS)
            audioCapabilities.Add() = JsonData::DeviceInfo::AudiocapabilitiesResultData::AudiocapabilityType::ATMOS;
        if (capabilities & dsAUDIOSUPPORT_DD)
            audioCapabilities.Add() = JsonData::DeviceInfo::AudiocapabilitiesResultData::AudiocapabilityType::DD;
        if (capabilities & dsAUDIOSUPPORT_DDPLUS)
            audioCapabilities.Add() = JsonData::DeviceInfo::AudiocapabilitiesResultData::AudiocapabilityType::DDPLUS;
        if (capabilities & dsAUDIOSUPPORT_DAD)
            audioCapabilities.Add() = JsonData::DeviceInfo::AudiocapabilitiesResultData::AudiocapabilityType::DAD;
        if (capabilities & dsAUDIOSUPPORT_DAPv2)
            audioCapabilities.Add() = JsonData::DeviceInfo::AudiocapabilitiesResultData::AudiocapabilityType::DAPV2;
        if (capabilities & dsAUDIOSUPPORT_MS12)
            audioCapabilities.Add() = JsonData::DeviceInfo::AudiocapabilitiesResultData::AudiocapabilityType::MS12;

        return result;
    }

    uint32_t DeviceInfo::MS12Capabilities(const string& audioPort, Core::JSON::ArrayType<Ms12capabilityJsonEnum>& ms12Capabilities) const
    {
        uint32_t result = Core::ERROR_NONE;

        int capabilities = dsMS12SUPPORT_NONE;

        try {
            auto strAudioPort = audioPort.empty() ? "HDMI0" : audioPort;
            auto& aPort = device::Host::getInstance().getAudioOutputPort(strAudioPort);
            aPort.getMS12Capabilities(&capabilities);
        } catch (...) {
            result = Core::ERROR_GENERAL;
        }

        if (!capabilities)
            ms12Capabilities.Add() = JsonData::DeviceInfo::Ms12capabilitiesResultData::Ms12capabilityType::NONE;
        if (capabilities & dsMS12SUPPORT_DolbyVolume)
            ms12Capabilities.Add() = JsonData::DeviceInfo::Ms12capabilitiesResultData::Ms12capabilityType::DOLBYVOLUME;
        if (capabilities & dsMS12SUPPORT_InteligentEqualizer)
            ms12Capabilities.Add() = JsonData::DeviceInfo::Ms12capabilitiesResultData::Ms12capabilityType::INTELIGENTEQUALIZER;
        if (capabilities & dsMS12SUPPORT_DialogueEnhancer)
            ms12Capabilities.Add() = JsonData::DeviceInfo::Ms12capabilitiesResultData::Ms12capabilityType::DIALOGUEENHANCER;

        return result;
    }

    uint32_t DeviceInfo::SupportedMS12AudioProfiles(const string& audioPort, Core::JSON::ArrayType<Core::JSON::String>& supportedMS12AudioProfiles) const
    {
        uint32_t result = Core::ERROR_NONE;

        try {
            auto strAudioPort = audioPort.empty() ? "HDMI0" : audioPort;
            auto& aPort = device::Host::getInstance().getAudioOutputPort(strAudioPort);
            const auto supportedProfiles = aPort.getMS12AudioProfileList();
            for (size_t i = 0; i < supportedProfiles.size(); i++) {
                supportedMS12AudioProfiles.Add() = supportedProfiles.at(i);
            }
        } catch (...) {
            result = Core::ERROR_GENERAL;
        }

        return result;
    }

} // namespace Plugin
} // namespace WPEFramework
