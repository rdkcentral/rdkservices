#pragma once

#include "../Module.h"
#ifdef USE_THUNDER_R4
#include <interfaces/IDeviceInfo.h>
#else
#include <interfaces/IDeviceInfo2.h>
#endif /* USE_THUNDER_R4 */
#include <interfaces/IConfiguration.h>

namespace WPEFramework {
namespace Plugin {
    class DeviceInfoImplementation : public Exchange::IDeviceInfo , public Exchange::IConfiguration {
    private:
        DeviceInfoImplementation(const DeviceInfoImplementation&) = delete;
        DeviceInfoImplementation& operator=(const DeviceInfoImplementation&) = delete;

    public:
        DeviceInfoImplementation();

        BEGIN_INTERFACE_MAP(DeviceInfoImplementation)
        INTERFACE_ENTRY(Exchange::IDeviceInfo)
        INTERFACE_ENTRY(Exchange::IConfiguration)
        END_INTERFACE_MAP

    private:
        // IDeviceInfo interface
        uint32_t SerialNumber(string& serialNumber) const override;
        uint32_t Sku(string& sku) const override;
        uint32_t Make(string& make) const override;
        uint32_t Model(string& model) const override;
        uint32_t DeviceType(string& deviceType) const override;
        uint32_t DistributorId(string& distributorId) const override;
        uint32_t Brand(string& brand) const override;
        uint32_t Chipset(string& chipset) const override;

        // IConfiguration interface
        uint32_t Configure(PluginHost::IShell* shell);

        PluginHost::IShell* mShell;
    };
}
}
