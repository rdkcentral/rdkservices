#pragma once

#include "Module.h"
#include <interfaces/IFirmwareVersion.h>

namespace WPEFramework {
namespace Plugin {

    class FirmwareVersion : public Exchange::IFirmwareVersion {
    private:
        FirmwareVersion(const FirmwareVersion&) = delete;
        FirmwareVersion& operator=(const FirmwareVersion&) = delete;

    public:
        FirmwareVersion() = default;

        BEGIN_INTERFACE_MAP(FirmwareVersion)
        INTERFACE_ENTRY(Exchange::IFirmwareVersion)
        END_INTERFACE_MAP

    private:
        //   IFirmwareVersion methods
        // -------------------------------------------------------------------------------------------------------
        uint32_t Imagename(string& imagename) const override;
        uint32_t Sdk(string& sdk) const override;
        uint32_t Mediarite(string& mediarite) const override;
        uint32_t Yocto(string& yocto) const override;
    };

} // namespace Plugin
} // namespace WPEFramework
