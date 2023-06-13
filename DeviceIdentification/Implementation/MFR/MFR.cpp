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

extern "C" {
#include <mfrApi.h>
}

namespace WPEFramework {
namespace Plugin {
    class DeviceImplementation : public PluginHost::ISubSystem::IIdentifier {

    public:
        DeviceImplementation()
        {
            UpdateChipset(_chipset);
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
        // IIdentifier interface
        uint8_t Identifier(const uint8_t length, uint8_t buffer[]) const override
        {
            uint8_t result = 0;
            if ((_identity.length()) && (_status == mfrERR_NONE)) {
                result = (_identity.length() > length ? length : _identity.length());
                ::memcpy(buffer, _identity.c_str(), result);
            } else {
                SYSLOG(Logging::Notification, (_T("Cannot determine system identity; Error:[%d]!"), static_cast<uint8_t>(_status)));
            }
            return result;
        }
        string Architecture() const override
        {
            return Core::SystemInfo::Instance().Architecture();
        }
        string Chipset() const override
        {
            return _chipset;
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
            int retVal = -1;
            mfrSerializedData_t mfrSerializedData;
            retVal = mfrGetSerializedData(mfrSERIALIZED_TYPE_SOFTWAREVERSION, &mfrSerializedData);
            if ((mfrERR_NONE == retVal) && mfrSerializedData.bufLen) {
                firmwareVersion = mfrSerializedData.buf;
                if (mfrSerializedData.freeBuf) {
                    mfrSerializedData.freeBuf(mfrSerializedData.buf);
                }
            }
        }
        inline void UpdateChipset(string& chipset) const
        {
            int retVal = -1;
            mfrSerializedData_t mfrSerializedData;
            retVal = mfrGetSerializedData(mfrSERIALIZED_TYPE_CHIPSETINFO, &mfrSerializedData);
            if ((mfrERR_NONE == retVal) && mfrSerializedData.bufLen) {
                chipset = mfrSerializedData.buf;
                if (mfrSerializedData.freeBuf) {
                    mfrSerializedData.freeBuf(mfrSerializedData.buf);
                }
            }
        }
        inline void UpdateIdentifier()
        {
            mfrSerializedData_t mfrSerializedData;
            _status = mfrGetSerializedData(mfrSERIALIZED_TYPE_SERIALNUMBER, &mfrSerializedData);
            if ((mfrERR_NONE == _status) && mfrSerializedData.bufLen) {
                _identity = mfrSerializedData.buf;
                if (mfrSerializedData.freeBuf) {
                    mfrSerializedData.freeBuf(mfrSerializedData.buf);
                }
            }
        }

    private:
        string _chipset;
        string _firmwareVersion;
        string _identity;
        mfrError_t _status;
    };

    SERVICE_REGISTRATION(DeviceImplementation, 1, 0);
}
}
