#include "FirmwareVersion.h"

#include <fstream>

namespace WPEFramework {
namespace Plugin {

    namespace {
        constexpr auto* kVersionFile = _T("/version.txt");
    }

    SERVICE_REGISTRATION(FirmwareVersion, 1, 0);

    uint32_t FirmwareVersion::Imagename(string& imagename) const
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

    uint32_t FirmwareVersion::Sdk(string& sdk) const
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

    uint32_t FirmwareVersion::Mediarite(string& mediarite) const
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

    uint32_t FirmwareVersion::Yocto(string& yocto) const
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

} // namespace Plugin
} // namespace WPEFramework
