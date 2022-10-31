#pragma once

#include <string>
#include <vector>

namespace RdkShell
{
    struct LinuxInputDevice
    {
        LinuxInputDevice() : vendor(0), product(0), deviceType(0), deviceMode(0), devicePath{} {}
        uint16_t vendor;
        uint16_t product;
        uint8_t deviceType;
        uint8_t deviceMode;
        std::string devicePath;
    };

    struct IrInputDeviceTypeMapping
    {
        IrInputDeviceTypeMapping() : filterCode(0), deviceType(0) {}
        uint8_t filterCode;
        uint8_t deviceType;
    };

    void readInputDevicesConfiguration(){return;}
    void inputDeviceTypeAndMode(const uint16_t vendor, const uint16_t product, const std::string& devicePath, uint8_t& type, uint8_t& mode){return;}
    void irDeviceType(const uint8_t filterCode, uint8_t& type){return;}
}
