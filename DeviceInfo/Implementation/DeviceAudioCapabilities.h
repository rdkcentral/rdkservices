#pragma once

#include "../Module.h"
#ifdef USE_THUNDER_R4
#include <interfaces/IDeviceInfo.h>
#else
#include <interfaces/IDeviceInfo2.h>
#endif /* USE_THUNDER_R4 */

#include "dsinitmanager/DSManagerPlugin.h"

namespace WPEFramework {
namespace Plugin {
    class DeviceAudioCapabilities : public Exchange::IDeviceAudioCapabilities, public DSManagerPlugin {
    private:
        DeviceAudioCapabilities(const DeviceAudioCapabilities&) = delete;
        DeviceAudioCapabilities& operator=(const DeviceAudioCapabilities&) = delete;

    public:
        DeviceAudioCapabilities();

        BEGIN_INTERFACE_MAP(DeviceAudioCapabilities)
        INTERFACE_ENTRY(Exchange::IDeviceAudioCapabilities)
        END_INTERFACE_MAP

    private:
        // IDeviceAudioCapabilities interface
        uint32_t SupportedAudioPorts(RPC::IStringIterator*& supportedAudioPorts) const override;
        uint32_t AudioCapabilities(const string& audioPort, Exchange::IDeviceAudioCapabilities::IAudioCapabilityIterator*& audioCapabilities) const override;
        uint32_t MS12Capabilities(const string& audioPort, Exchange::IDeviceAudioCapabilities::IMS12CapabilityIterator*& ms12Capabilities) const override;
        uint32_t SupportedMS12AudioProfiles(const string& audioPort, RPC::IStringIterator*& supportedMS12AudioProfiles) const override;
    };
}
}
