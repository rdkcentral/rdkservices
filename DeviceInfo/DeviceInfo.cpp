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

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 7

namespace WPEFramework {
namespace {
    static Plugin::Metadata<Plugin::DeviceInfo> metadata(
        // Version (Major, Minor, Patch)
        API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
        // Preconditions
        {},
        // Terminations
        {},
        // Controls
        {}
    );
}

namespace Plugin {

    SERVICE_REGISTRATION(DeviceInfo, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    static Core::ProxyPoolType<Web::JSONBodyType<DeviceInfo::Data>> jsonResponseFactory(4);

    /* virtual */ const string DeviceInfo::Initialize(PluginHost::IShell* service)
    {
        ASSERT(_service == nullptr);
        ASSERT(service != nullptr);

        ASSERT(_subSystem == nullptr);

        _skipURL = static_cast<uint8_t>(service->WebPrefix().length());
        _subSystem = service->SubSystems();
        _service = service;
#ifndef USE_THUNDER_R4
        _systemId = Core::SystemInfo::Instance().Id(Core::SystemInfo::Instance().RawDeviceId(), ~0);
#else
        _systemId = string();
#endif /* USE_THUNDER_R4 */

        ASSERT(_subSystem != nullptr);

        _deviceInfo = service->Root<Exchange::IDeviceInfo>(_connectionId, 2000, _T("DeviceInfoImplementation"));
        _deviceAudioCapabilities = service->Root<Exchange::IDeviceAudioCapabilities>(_connectionId, 2000, _T("DeviceAudioCapabilities"));
        _deviceVideoCapabilities = service->Root<Exchange::IDeviceVideoCapabilities>(_connectionId, 2000, _T("DeviceVideoCapabilities"));
        _firmwareVersion = service->Root<Exchange::IFirmwareVersion>(_connectionId, 2000, _T("FirmwareVersion"));

        ASSERT(_deviceInfo != nullptr);
        ASSERT(_deviceAudioCapabilities != nullptr);
        ASSERT(_deviceVideoCapabilities != nullptr);
        ASSERT(_firmwareVersion != nullptr);

        // On success return empty, to indicate there is no error text.

        return ((_subSystem != nullptr)
                   && (_deviceInfo != nullptr)
                   && (_deviceAudioCapabilities != nullptr)
                   && (_deviceVideoCapabilities != nullptr)
                   && (_firmwareVersion != nullptr))
            ? EMPTY_STRING
            : _T("Could not retrieve System Information.");
    }

    /* virtual */ void DeviceInfo::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);

        _deviceInfo->Release();
        _deviceAudioCapabilities->Release();
        _deviceVideoCapabilities->Release();
        _firmwareVersion->Release();

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
            } else if (index.Current() == "Adresses") {
                AddressInfo(response->Addresses);
            } else if (index.Current() == "System") {
                SysInfo(response->SystemInfo);
            } else if (index.Current() == "Sockets") {
                SocketPortInfo(response->Sockets);
            }
            // TODO RB: I guess we should do something here to return other info (e.g. time) as well.

            result->ContentType = Web::MIMETypes::MIME_JSON;
#ifndef USE_THUNDER_R4
            result->Body(Core::proxy_cast<Web::IBody>(response));
#else
            result->Body(Core::ProxyType<Web::IBody>(response));
#endif /* USE_THUNDER_R4 */
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
        systemInfo.Totalswap = singleton.GetTotalSwap();
        systemInfo.Freeswap = singleton.GetFreeSwap();
        systemInfo.Devicename = singleton.GetHostName();
        systemInfo.Cpuload = Core::NumberType<uint32_t>(static_cast<uint32_t>(singleton.GetCpuLoad())).Text();
        systemInfo.Serialnumber = _systemId;

        auto cpuloadavg = singleton.GetCpuLoadAvg();
        if (cpuloadavg != nullptr) {
            systemInfo.Cpuloadavg.Avg1min = *(cpuloadavg);
            if (++cpuloadavg != nullptr) {
                systemInfo.Cpuloadavg.Avg5min = *(cpuloadavg);
                if (++cpuloadavg != nullptr) {
                    systemInfo.Cpuloadavg.Avg15min = *(cpuloadavg);
                }
            }
        }
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

} // namespace Plugin
} // namespace WPEFramework
