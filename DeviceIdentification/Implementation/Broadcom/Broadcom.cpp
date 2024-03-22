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
        identifier.assign(extractSerialNumber());
        chipset.assign(extractChipId());
        firmwareVersion.assign(extractFirmwareVersion());
    }

    inline std::string extractSerialNumber() const
    {
        std::string serialNumber;
        std::ifstream serialNumberFile("/proc/device-tree/serial-number");

        if (serialNumberFile.is_open())
        {
                getline(serialNumberFile,serialNumber);
                serialNumberFile.close();
        }

        return serialNumber;
    }

    inline std::string extractChipId() const
    {
        std::string chipId;
        std::ifstream chipIdFile("/proc/device-tree/model");

        if (chipIdFile.is_open())
        {
                getline(chipIdFile, chipId);
                chipIdFile.close();
        }

        return chipId;
    }

    inline std::string extractFirmwareVersion() const
    {
        std::string versionIdentifier("VERSION");
        std::ifstream versionFile("/version.txt");
        std::string line;
        std::string firmwareVersion;

        if (versionFile.is_open())
        {
            while (getline(versionFile, line))
            {
                if (0 == line.find(versionIdentifier))
                {
                    std::size_t position = line.find(versionIdentifier) + versionIdentifier.length() + 1; // +1 is to skip '='
                    if (position != std::string::npos)
                    {
                        firmwareVersion.assign(line.substr(position, std::string::npos));
                    }
                }
            }
            versionFile.close();
        }

        return firmwareVersion;
    }

private:
    string _chipset;
    string _firmwareVersion;
    string _identifier;
};

    SERVICE_REGISTRATION(DeviceImplementation, 1, 0);
}
}
