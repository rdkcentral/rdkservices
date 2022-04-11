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

#include <fstream>

namespace WPEFramework {
namespace Plugin {

    class DeviceImplementation : public PluginHost::ISubSystem::IIdentifier {
        static constexpr const TCHAR* CPUInfoFile = _T("/proc/cpuinfo");
        static constexpr const TCHAR* VERSIONFile = _T("/version.txt");

    public:
        DeviceImplementation()
        {
            UpdateFirmwareVersion(_firmwareVersion);
            UpdateIdentifier();
        }

        DeviceImplementation(const DeviceImplementation&) = delete;
        DeviceImplementation& operator=(const DeviceImplementation&) = delete;
        virtual ~DeviceImplementation()
        {
            /* Nothing to do here. */
        }

    public:
        // Device Propertirs interface

        // Identifier interface
        uint8_t Identifier(const uint8_t length, uint8_t buffer[]) const override
        {
            uint8_t result = 0;
            if ((_identity.length())) {
                result = (_identity.length() > length ? length : _identity.length());
                ::memcpy(buffer, _identity.c_str(), result);
            } else {
                SYSLOG(Logging::Notification, (_T("Cannot determine system identity")));
            }
            return result;
        }
        string Architecture() const override
        {
            return Core::SystemInfo::Instance().Architecture();
        }
        string Chipset() const override
        {
            return Core::SystemInfo::Instance().Chipset();
        }
        string FirmwareVersion() const override
        {
            return _firmwareVersion;
        }

        BEGIN_INTERFACE_MAP(DeviceImplementation)
        INTERFACE_ENTRY(PluginHost::ISubSystem::IIdentifier)
        END_INTERFACE_MAP

    private:
        inline void UpdateFirmwareVersion(string& firmwareVersion) const
        {
            string line;
            std::ifstream file(VERSIONFile);
            if (file.is_open()) {
                while (getline(file, line)) {
                    if (line.find("SDK_VERSION") != std::string::npos) {
                        std::size_t position = line.find('=');
                        if (position != std::string::npos) {
                            firmwareVersion.assign(line.substr(position + 1, string::npos));
                            break;
                        }
                    }
                }
                file.close();
            }
        }

        inline void UpdateIdentifier()
        {
            string line;
            std::ifstream file(CPUInfoFile);
            if (file.is_open()) {
                while (getline(file, line)) {
                    if (line.find("Serial") != std::string::npos) {
                        std::size_t position = line.find(": ");
                        if (position != std::string::npos) {
                            _identity.assign(line.substr(position + 2, string::npos));
                        }
                    }
                }
                file.close();
            }
        }

    private:
        string _firmwareVersion;
        string _identity;
    };

    SERVICE_REGISTRATION(DeviceImplementation, 1, 0);
}
}
