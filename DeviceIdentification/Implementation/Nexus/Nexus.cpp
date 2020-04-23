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

#include <nexus_config.h>
#include <nexus_platform.h>
#include <nxclient.h>

#if NEXUS_SECURITY_API_VERSION == 2
#include <nexus_otp_key.h>
#else
#include <nexus_otpmsp.h>
#include <nexus_read_otp_id.h>
#endif

namespace WPEFramework {
namespace Plugin {

class DeviceImplementation : public Exchange::IDeviceProperties, public PluginHost::ISubSystem::IIdentifier {
private:
    enum DeviceStatus { ErrorNone, ErrorInit, ErrorFetch, ErrorGeneric };

public:
    DeviceImplementation()
        : _length(0)
        , _status(DeviceStatus::ErrorGeneric)
        , _identity(nullptr)
    {
        NEXUS_Error rc = NxClient_Join(NULL);
        ASSERT(!rc);
        NxClient_UnregisterAcknowledgeStandby(NxClient_RegisterAcknowledgeStandby());

        if (rc == NEXUS_SUCCESS) {
            NEXUS_Platform_GetConfiguration(&_platformConfig);

            UpdateChipset(_chipset);
            UpdateFirmwareVersion(_firmwareVersion);
            UpdateIdentifier();
        } else {
            _status = DeviceStatus::ErrorInit;
        }
    }

    DeviceImplementation(const DeviceImplementation&) = delete;
    DeviceImplementation& operator= (const DeviceImplementation&) = delete;
    virtual ~DeviceImplementation()
    {
        NxClient_Uninit();
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
    /* virtual*/ uint8_t Identifier(const uint8_t length, uint8_t buffer[]) const
    {
        uint8_t result = 0;

        if ((_identity != nullptr) && (_length != 0) && (_status == DeviceStatus::ErrorNone)) {
            result = (_length > length ? length : _length);
            ::memcpy(buffer, _identity, result);
        } else {
            SYSLOG(Logging::Notification, (_T("System identity can not be determined. Error: [%d]!"), static_cast<uint8_t>(_status)));
        }

        return result;
    }

    BEGIN_INTERFACE_MAP(DeviceImplementation)
        INTERFACE_ENTRY(Exchange::IDeviceProperties)
        INTERFACE_ENTRY(PluginHost::ISubSystem::IIdentifier)
    END_INTERFACE_MAP

private:
    inline void UpdateFirmwareVersion(string& firmwareVersion) const
    {
        char version[256];
        sprintf(version, "Nexus Release %d.%d", (NEXUS_P_GET_VERSION(NEXUS_PLATFORM) / NEXUS_PLATFORM_VERSION_UNITS), (NEXUS_P_GET_VERSION(NEXUS_PLATFORM) % NEXUS_PLATFORM_VERSION_UNITS));
        firmwareVersion = version;
    }
    inline void UpdateChipset(string& chipset) const
    {
        NEXUS_PlatformStatus status;
        NEXUS_Error rc = NEXUS_UNKNOWN;

        rc = NEXUS_Platform_GetStatus(&status);
        if (rc == NEXUS_SUCCESS) {
            char chipId[10];
            sprintf(chipId, "%x", status.chipId);
            char revision[10];
            sprintf(revision, "%c%c", (((status.chipRevision >> 4) & 0xF) + 'A' - 1), (((status.chipRevision) & 0xF) + '0'));
            chipset = "BCM" + string(chipId) + " " + string(revision);
        }
    }
    inline void UpdateIdentifier()
    {
        if (_length == 0) {
#if NEXUS_SECURITY_API_VERSION == 2
            if (NEXUS_SUCCESS == NEXUS_OtpKey_GetInfo(0 /*key A*/, &_id)) {
                _length = NEXUS_OTP_KEY_ID_LENGTH;
                _identity = _id.id;
#else
            if (NEXUS_SUCCESS == NEXUS_Security_ReadOtpId(NEXUS_OtpIdType_eA, &_id)) {
                _length = static_cast<unsigned char>(_id.size);
                _identity = _id.otpId;
#endif

                _status = DeviceStatus::ErrorNone;

            } else {
                _status = DeviceStatus::ErrorFetch;
            }
        }
    }

private:
    string _chipset;
    string _firmwareVersion;

    uint8_t _length;
    DeviceStatus _status;
    unsigned char* _identity;

#if NEXUS_SECURITY_API_VERSION == 2
    NEXUS_OtpKeyInfo _id;
#else
    NEXUS_OtpIdOutput _id;
#endif
    NEXUS_PlatformConfiguration _platformConfig;
};

    SERVICE_REGISTRATION(DeviceImplementation, 1, 0);
}
}
