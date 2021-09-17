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
#include <interfaces/IDeviceIdentification.h>

#include <fstream>

namespace WPEFramework {
namespace Plugin {

class DeviceImplementation : public Exchange::IDeviceProperties, public PluginHost::ISubSystem::IIdentifier {
    static constexpr const TCHAR* PlatformOutputFile= _T("/tmp/.platform_output");

public:
    DeviceImplementation()
    {
        if (system("cat /proc/brcm/platform > /tmp/.platform_output") == 0) {
            UpdateChipset(_chipset);
            UpdateFirmwareVersion(_firmwareVersion);
            UpdateIdentifier(_identifier);
            system("rm -rf /tmp/.platform_output");
        }
    }

    DeviceImplementation(const DeviceImplementation&) = delete;
    DeviceImplementation& operator= (const DeviceImplementation&) = delete;
    virtual ~DeviceImplementation()
    {
    }

public:
    // Device Propertirs interface
    const string Chipset() const override
    {
        return _chipset;
    }
    const string FirmwareVersion() const override
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
        INTERFACE_ENTRY(Exchange::IDeviceProperties)
        INTERFACE_ENTRY(PluginHost::ISubSystem::IIdentifier)
    END_INTERFACE_MAP

private:
    inline void UpdateFirmwareVersion(string& firmwareVersion) const
    {
        std::string line;
        std::ifstream file(PlatformOutputFile);
        if (file.is_open()) {
            while (getline(file, line)) {
                if (line.find("Nexus Release") != std::string::npos) {
                    std::size_t position = line.find("Nexus Release") + sizeof("Nexus Release");
                    if (position != std::string::npos) {
                        firmwareVersion.assign(line.substr(position, std::string::npos));
                    }
                    break;
                }
            }
            file.close();
        }
    }

    inline void UpdateChipset(string& chipset) const
    {
        std::string line;
        std::ifstream file(PlatformOutputFile);

        if (file.is_open()) {
            while (getline(file, line)) {
                if (line.find("Chip ID ") != std::string::npos) {
                    std::size_t position = line.find("Chip ID ") + sizeof("Chip ID ");
                    if (position != std::string::npos) {
                        chipset.assign(line.substr(position, std::string::npos));
                    }
                    break;
               }
            }
            file.close();
        }
    }

   inline void UpdateIdentifier(string &identifier) const
   {
        std::string line;
        std::ifstream file(PlatformOutputFile);

        if (file.is_open()) {
            while (getline(file, line)) {
                if (line.find("CHIPID") != std::string::npos) {
                    std::size_t position = line.find("CHIPID") + sizeof("CHIPID");
                    if (position != std::string::npos) {
                        identifier.assign(line.substr(position, line.find(']')-position));
                    }
                    break;
               }
            }
            file.close();
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
