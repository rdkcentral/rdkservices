#include "Module.h"
#include <interfaces/IDeviceInfo.h>

#include "mfrMgr.h"

#include "UtilsIarm.h"

#include <fstream>
#include <regex>

namespace WPEFramework {
namespace Plugin {
    namespace {
        constexpr auto* kAuthserviceConf = _T("/etc/authService.conf");
        constexpr auto* kPartnerIdFile = _T("/opt/www/authService/partnerId3.dat");
    }

    class DeviceInfoImplementation : public Exchange::IDeviceInfo {
    private:
        DeviceInfoImplementation(const DeviceInfoImplementation&) = delete;
        DeviceInfoImplementation& operator=(const DeviceInfoImplementation&) = delete;

    public:
        DeviceInfoImplementation()
        {
            Utils::IARM::init();
        }

        BEGIN_INTERFACE_MAP(DeviceInfoImplementation)
        INTERFACE_ENTRY(Exchange::IDeviceInfo)
        END_INTERFACE_MAP

    private:
        // IDeviceInfo interface
        uint32_t SerialNumber(string& serialNumber) const
        {
            uint32_t result = Core::ERROR_GENERAL;

            IARM_Bus_MFRLib_GetSerializedData_Param_t param;
            param.bufLen = 0;
            param.type = mfrSERIALIZED_TYPE_SERIALNUMBER;
            if (IARM_Bus_Call(IARM_BUS_MFRLIB_NAME, IARM_BUS_MFRLIB_API_GetSerializedData, &param, sizeof(param)) == IARM_RESULT_SUCCESS) {
                serialNumber.assign(param.buffer, param.bufLen);
                result = Core::ERROR_NONE;
            }

            return result;
        }

        uint32_t Sku(string& sku) const
        {
            uint32_t result = Core::ERROR_GENERAL;

            IARM_Bus_MFRLib_GetSerializedData_Param_t param;
            param.bufLen = 0;
            param.type = mfrSERIALIZED_TYPE_MODELNAME;
            if (IARM_Bus_Call(IARM_BUS_MFRLIB_NAME, IARM_BUS_MFRLIB_API_GetSerializedData, &param, sizeof(param)) == IARM_RESULT_SUCCESS) {
                model.assign(param.buffer, param.bufLen);
                result = Core::ERROR_NONE;
            }

            return result;
        }

        uint32_t Make(string& make) const
        {
            uint32_t result = Core::ERROR_GENERAL;

            IARM_Bus_MFRLib_GetSerializedData_Param_t param;
            param.bufLen = 0;
            param.type = mfrSERIALIZED_TYPE_MANUFACTURER;
            if (IARM_Bus_Call(IARM_BUS_MFRLIB_NAME, IARM_BUS_MFRLIB_API_GetSerializedData, &param, sizeof(param)) == IARM_RESULT_SUCCESS) {
                make.assign(param.buffer, param.bufLen);
                result = Core::ERROR_NONE;
            }

            return result;
        }

        uint32_t Model(string& model) const
        {
            uint32_t result = Core::ERROR_GENERAL;

            IARM_Bus_MFRLib_GetSerializedData_Param_t param;
            param.bufLen = 0;
            param.type = mfrSERIALIZED_TYPE_SKYMODELNAME;
            if (IARM_Bus_Call(IARM_BUS_MFRLIB_NAME, IARM_BUS_MFRLIB_API_GetSerializedData, &param, sizeof(param)) == IARM_RESULT_SUCCESS) {
                model.assign(param.buffer, param.bufLen);
                result = Core::ERROR_NONE;
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
