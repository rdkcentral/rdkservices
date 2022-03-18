#pragma once

#include "Module.h"

#include <interfaces/IDeviceIdentification.h>

namespace WPEFramework {
namespace Plugin {

    class DeviceImplementation : public Exchange::IDeviceProperties,
                                 public PluginHost::ISubSystem::IIdentifier {
    public:
        DeviceImplementation()
            : _chipset("testChipset")
            , _firmwareVersion("testFirmwareVersion")
            , _identity("testIdentity")
        {
        }

        DeviceImplementation(const DeviceImplementation&) = delete;
        DeviceImplementation& operator=(const DeviceImplementation&) = delete;
        virtual ~DeviceImplementation()
        {
        }

    public:
        // Device Properties interface
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
            uint8_t result = 0;
            if ((_identity.length())) {
                result = (_identity.length() > length ? length : _identity.length());
                ::memcpy(buffer, _identity.c_str(), result);
            } else {
                SYSLOG(Logging::Notification, (_T("Cannot determine system identity")));
            }
            return result;
        }

        BEGIN_INTERFACE_MAP(DeviceImplementation)
        INTERFACE_ENTRY(Exchange::IDeviceProperties)
        INTERFACE_ENTRY(PluginHost::ISubSystem::IIdentifier)
        END_INTERFACE_MAP

    private:
        string _chipset;
        string _firmwareVersion;
        string _identity;
    };

}
}
