#include "../../Module.h"
#include <interfaces/IFirmwareVersion.h>

#include <fstream>

namespace WPEFramework {
namespace Plugin {
    namespace {
        constexpr auto* kVersionFile = _T("/version.txt");
    }

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
        // IFirmwareVersion interface
        uint32_t Imagename(string& imagename) const override
        {
            uint32_t result = Core::ERROR_GENERAL;

            std::ifstream file(kVersionFile);
            if (file) {
                string line;
                while (std::getline(file, line)) {
                    if (line.rfind(_T("imagename"), 0) == 0) {
                        imagename = line.substr(line.find(':') + 1);
                        result = Core::ERROR_NONE;
                        break;
                    }
                }
            }

            return result;
        }

        uint32_t Sdk(string& sdk) const override
        {
            uint32_t result = Core::ERROR_GENERAL;

            std::ifstream file(kVersionFile);
            if (file) {
                string line;
                while (std::getline(file, line)) {
                    if (line.rfind(_T("SDK_VERSION"), 0) == 0) {
                        sdk = line.substr(line.find('=') + 1);
                        result = Core::ERROR_NONE;
                        break;
                    }
                }
            }

            return result;
        }

        uint32_t Mediarite(string& mediarite) const override
        {
            uint32_t result = Core::ERROR_GENERAL;

            std::ifstream file(kVersionFile);
            if (file) {
                string line;
                while (std::getline(file, line)) {
                    if (line.rfind(_T("MEDIARITE"), 0) == 0) {
                        mediarite = line.substr(line.find('=') + 1);
                        result = Core::ERROR_NONE;
                        break;
                    }
                }
            }

            return result;
        }

        uint32_t Yocto(string& yocto) const override
        {
            uint32_t result = Core::ERROR_GENERAL;

            std::ifstream file(kVersionFile);
            if (file) {
                string line;
                while (std::getline(file, line)) {
                    if (line.rfind(_T("YOCTO_VERSION"), 0) == 0) {
                        yocto = line.substr(line.find('=') + 1);
                        result = Core::ERROR_NONE;
                        break;
                    }
                }
            }

            return result;
        }
    };

    SERVICE_REGISTRATION(FirmwareVersion, 1, 0);
}
}
