#pragma once

#include "Module.h"
#include <interfaces/IDeviceInfo.h>

namespace WPEFramework {
namespace Plugin {

    class DeviceCapabilities : public Exchange::IDeviceCapabilities {
    private:
        DeviceCapabilities(const DeviceCapabilities&) = delete;
        DeviceCapabilities& operator=(const DeviceCapabilities&) = delete;

    public:
        DeviceCapabilities();
        ~DeviceCapabilities() override;

        BEGIN_INTERFACE_MAP(DeviceCapabilities)
        INTERFACE_ENTRY(Exchange::IDeviceCapabilities)
        END_INTERFACE_MAP

    private:
        //   IDeviceCapabilities methods
        // -------------------------------------------------------------------------------------------------------
        uint32_t SerialNumber(string& serialNumber) const override;
        uint32_t Sku(string& sku) const override;
        uint32_t Make(string& make) const override;
        uint32_t Model(string& model) const override;
        uint32_t DeviceType(string& deviceType) const override;
        uint32_t DistributorId(string& distributorId) const override;
        uint32_t SupportedAudioPorts(RPC::IStringIterator*& supportedAudioPorts) const override;
        uint32_t SupportedVideoDisplays(RPC::IStringIterator*& supportedVideoDisplays) const override;
        uint32_t HostEDID(string& edid) const override;
        uint32_t DefaultResolution(const string& videoDisplay, string& defaultResolution) const override;
        uint32_t SupportedResolutions(const string& videoDisplay, RPC::IStringIterator*& supportedResolutions) const override;
        uint32_t SupportedHdcp(const string& videoDisplay, Exchange::IDeviceCapabilities::CopyProtection& supportedHDCPVersion) const override;
        uint32_t AudioCapabilities(const string& audioPort, Exchange::IDeviceCapabilities::IAudioCapabilityIterator*& audioCapabilities) const override;
        uint32_t MS12Capabilities(const string& audioPort, Exchange::IDeviceCapabilities::IMS12CapabilityIterator*& ms12Capabilities) const override;
        uint32_t SupportedMS12AudioProfiles(const string& audioPort, RPC::IStringIterator*& supportedMS12AudioProfiles) const override;
    };

} // namespace Plugin
} // namespace WPEFramework
