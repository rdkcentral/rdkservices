#include "../../Module.h"
#include <interfaces/IDeviceInfo.h>

#include "UtilsIarm.h"

#include <fstream>
#include <regex>

namespace WPEFramework {
namespace Plugin {
    namespace {
        constexpr auto* kDeviceProperties = _T("/etc/device.properties");
        constexpr auto* kAuthserviceConf = _T("/etc/authService.conf");
        constexpr auto* kSerialNumberFile = _T("/proc/device-tree/serial-number");
        constexpr auto* kPartnerIdFile = _T("/opt/www/authService/partnerId3.dat");
    }

    class DeviceInfoImplementation : public Exchange::IDeviceInfo {
    private:
        DeviceInfoImplementation(const DeviceInfoImplementation&) = delete;
        DeviceInfoImplementation& operator=(const DeviceInfoImplementation&) = delete;

    public:
        DeviceInfoImplementation() = default;

        BEGIN_INTERFACE_MAP(DeviceInfoImplementation)
        INTERFACE_ENTRY(Exchange::IDeviceInfo)
        END_INTERFACE_MAP

    private:
        // IDeviceInfo interface
        uint32_t SerialNumber(string& serialNumber) const
        {
            uint32_t result = Core::ERROR_GENERAL;

            std::ifstream file(kSerialNumberFile);
            if (file) {
                string line;
                if (std::getline(file, line)) {
                    serialNumber = line;
                    result = Core::ERROR_NONE;
                }
            }

            return result;
        }

        uint32_t Sku(string& sku) const
        {
            uint32_t result = Core::ERROR_GENERAL;

            std::ifstream file(kDeviceProperties);
            if (file) {
                string line;
                while (std::getline(file, line)) {
                    if (line.rfind(_T("MODEL_NUM"), 0) == 0) {
                        sku = line.substr(line.find('=') + 1);
                        result = Core::ERROR_NONE;
                        break;
                    }
                }
            }

            return result;
        }

        uint32_t Make(string& make) const
        {
            uint32_t result = Core::ERROR_GENERAL;

            std::ifstream file(kDeviceProperties);
            if (file) {
                string line;
                while (std::getline(file, line)) {
                    if (line.rfind(_T("MFG_NAME"), 0) == 0) {
                        make = line.substr(line.find('=') + 1);
                        result = Core::ERROR_NONE;
                        break;
                    }
                }
            }

            return result;
        }

        uint32_t Model(string& model) const
        {
            uint32_t result = Core::ERROR_GENERAL;

            std::ifstream file(kDeviceProperties);
            if (file) {
                string line;
                while (std::getline(file, line)) {
                    if (line.rfind(_T("FRIENDLY_ID"), 0) == 0) {
                        // trim quotes
                        model = std::regex_replace(line, std::regex(_T("^\\w+=(?:\")?([^\"\\n]+)(?:\")?$")), _T("$1"));
                        result = Core::ERROR_NONE;
                        break;
                    }
                }
            }

            return result;
        }

        uint32_t DeviceType(string& deviceType) const
        {
            uint32_t result = Core::ERROR_GENERAL;

            std::ifstream file(kAuthserviceConf);
            if (file) {
                string line;
                while (std::getline(file, line)) {
                    if (line.rfind(_T("deviceType"), 0) == 0) {
                        deviceType = line.substr(line.find('=') + 1);
                        result = Core::ERROR_NONE;
                        break;
                    }
                }
            }

            return result;
        }

        uint32_t DistributorId(string& distributorId) const
        {
            uint32_t result = Core::ERROR_GENERAL;

            std::ifstream file(kPartnerIdFile);
            if (file) {
                string line;
                if (std::getline(file, line)) {
                    distributorId = line;
                    result = Core::ERROR_NONE;
                }
            }

            return result;
        }
    };

    SERVICE_REGISTRATION(DeviceInfoImplementation, 1, 0);
}
}
