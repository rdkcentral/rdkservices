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

#include <bcm_host.h>
#include <fstream>

namespace WPEFramework {
namespace Plugin {

class DeviceImplementation : public Exchange::IDeviceProperties, public PluginHost::ISubSystem::IIdentifier {
    static constexpr const TCHAR* CPUInfoFile= _T("/proc/cpuinfo");

public:
    DeviceImplementation()
    {
        bcm_host_init();

        UpdateChipset(_chipset);
        UpdateFirmwareVersion(_firmwareVersion);
    }

    DeviceImplementation(const DeviceImplementation&) = delete;
    DeviceImplementation& operator= (const DeviceImplementation&) = delete;
    virtual ~DeviceImplementation()
    {
        bcm_host_deinit();
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
        return 0;
    }

    BEGIN_INTERFACE_MAP(DeviceImplementation)
        INTERFACE_ENTRY(Exchange::IDeviceProperties)
        INTERFACE_ENTRY(PluginHost::ISubSystem::IIdentifier)
    END_INTERFACE_MAP

private:
    inline void UpdateFirmwareVersion(string& firmwareVersion) const
    {
        Command("version", firmwareVersion);
        if (firmwareVersion.length() > 0) {

            string::size_type i = 0;
            while (i < firmwareVersion.length()) {
                i = firmwareVersion.find_first_of("\n\r", i);
                if (i != std::string::npos) {
                    firmwareVersion.replace(i, 1, ", ");
                }
            }
        }
    }
    inline void UpdateChipset(string& chipset) const
    {
        string line;
        std::ifstream file(CPUInfoFile);
        if (file.is_open()) {
            while (getline(file, line)) {
                if (line.find("Hardware") != std::string::npos) {
                    std::size_t position = line.find(':');
                    if (position != std::string::npos) {
                        chipset.assign(line.substr(position + 1, string::npos));
                    }
                }
            }
            file.close();
        }
    }
    void Command(const char request[], string& value) const
    {
        char buffer[512];

        // Reset the string
        buffer[0] = '\0';

        int VARIABLE_IS_NOT_USED status = vc_gencmd(buffer, sizeof(buffer), &request[0]);
        assert((status == 0) && "Error: vc_gencmd failed.\n");

        // Make sure it is null-terminated
        buffer[sizeof(buffer) - 1] = '\0';

        // We do not need the stuff that is before the '=', we know what we requested :-)
        char* equal = strchr(buffer, '=');
        if (equal != nullptr) {
            equal++;
        }
        else {
            equal = buffer;
        }

        // Create string from buffer.
        Core::ToString(equal, value);
    }

private:
    string _chipset;
    string _firmwareVersion;
};

    SERVICE_REGISTRATION(DeviceImplementation, 1, 0);
}
}
