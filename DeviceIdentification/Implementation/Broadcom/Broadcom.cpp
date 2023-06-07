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
 
#include "../../Module.h"
#ifndef USE_THUNDER_R4
#include <interfaces/IDeviceIdentification.h>
#endif /* USE_THUNDER_R4 */

#include <fstream>

namespace WPEFramework {
namespace Plugin {

class DeviceImplementation : public PluginHost::ISubSystem::IIdentifier {
    static constexpr const TCHAR* PlatformFile = _T("/proc/brcm/platform");

public:
    DeviceImplementation()
        : _chipset()
        , _firmwareVersion()
        , _identifier()
    {
        UpdateDeviceInfo(_identifier, _chipset, _firmwareVersion);
    }

    DeviceImplementation(const DeviceImplementation&) = delete;
    DeviceImplementation& operator= (const DeviceImplementation&) = delete;
    virtual ~DeviceImplementation()
    {
    }

public:
    // Device Propertirs interface
    string Architecture() const override
    {
        return Core::SystemInfo::Instance().Architecture();;
    }
    string Chipset() const override
    {
        return _chipset;
    }
    string FirmwareVersion() const override
    {
        return _firmwareVersion;
    }

    // Identifier interface
    uint8_t Identifier(const uint8_t length, uint8_t buffer[]) const override
    {
        uint8_t ret = 0;
        if (_identifier.length()) {
            ret = (_identifier.length() > length ? length : _identifier.length());
            ::memcpy(buffer, _identifier.c_str(), ret);
        } 
        return ret;
    }

    BEGIN_INTERFACE_MAP(DeviceImplementation)
        INTERFACE_ENTRY(PluginHost::ISubSystem::IIdentifier)
    END_INTERFACE_MAP

private:
    inline void UpdateDeviceInfo(string& identifier, string& chipset, string& firmwareVersion) const
    {
        static constexpr uint8_t MaxInfoCollection = 3;
        static constexpr const TCHAR ChipsetKey[] = _T("Chip ID");
        static constexpr const TCHAR IdentifierKey[] = _T("CHIPID");
        static constexpr const TCHAR FirmwareVersionKey[] = _T("Nexus Release");

        std::string line;
        std::ifstream file(PlatformFile);
        uint32_t collectedInfo = 0;

        if (file.is_open()) {
            while (getline(file, line) && (collectedInfo < MaxInfoCollection)) {
                if ((identifier.empty() == true) && (line.find(IdentifierKey) != std::string::npos)) {
                    std::size_t position = line.find(IdentifierKey) + sizeof(IdentifierKey);
                    if (position != std::string::npos) {
                        identifier.assign(line.substr(position, line.find(']')-position));
                        collectedInfo++;
                    }
                } else if ((chipset.empty() == true) && (line.find(ChipsetKey) != std::string::npos)) {
                    std::size_t position = line.find(ChipsetKey) + sizeof(ChipsetKey);
                    if (position != std::string::npos) {
                        chipset.assign(line.substr(position, std::string::npos));
                        collectedInfo++;
                    }
                } else if ((firmwareVersion.empty() == true) &&
                           (line.find(FirmwareVersionKey) != std::string::npos)) {
                    std::size_t position = line.find(FirmwareVersionKey) + sizeof(FirmwareVersionKey);
                    if (position != std::string::npos) {
                        firmwareVersion.assign(line.substr(position, std::string::npos));
                        collectedInfo++;
                    }
                }
            }
        }
    }

private:
    string _chipset;
    string _firmwareVersion;
    string _identifier;
};

    SERVICE_REGISTRATION(DeviceImplementation, 1, 0);
}
}
