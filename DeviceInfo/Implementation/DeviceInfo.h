#pragma once

#include "../Module.h"
#include <interfaces/IDeviceInfo2.h>

namespace WPEFramework {
namespace Plugin {
    class DeviceInfoImplementation : public Exchange::IDeviceInfo {
    private:
        DeviceInfoImplementation(const DeviceInfoImplementation&) = delete;
        DeviceInfoImplementation& operator=(const DeviceInfoImplementation&) = delete;

    public:
        DeviceInfoImplementation();

        BEGIN_INTERFACE_MAP(DeviceInfoImplementation)
        INTERFACE_ENTRY(Exchange::IDeviceInfo)
        END_INTERFACE_MAP

    private:
        // IDeviceInfo interface
        uint32_t SerialNumber(string& serialNumber) const override;
        uint32_t Sku(string& sku) const override;
        uint32_t Make(string& make) const override;
        uint32_t Model(string& model) const override;
        uint32_t DeviceType(string& deviceType) const override;
        uint32_t DistributorId(string& distributorId) const override;
    };
}
}
