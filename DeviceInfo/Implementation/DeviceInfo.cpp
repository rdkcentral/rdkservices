#include "DeviceInfo.h"

#include "mfrMgr.h"
#include "rfcapi.h"

#include "UtilsIarm.h"

#include <fstream>
#include <regex>

namespace WPEFramework {
namespace Plugin {
    namespace {
        uint32_t GetFileRegex(const char* filename, const std::regex& regex, string& response)
        {
            uint32_t result = Core::ERROR_GENERAL;

            std::ifstream file(filename);
            if (file) {
                string line;
                while (std::getline(file, line)) {
                    std::smatch sm;
                    if (std::regex_match(line, sm, regex)) {
                        ASSERT(sm.size() == 2);
                        response = sm[1];
                        result = Core::ERROR_NONE;
                        break;
                    }
                }
            }

            return result;
        }

        uint32_t GetMFRData(mfrSerializedType_t type, string& response)
        {
            uint32_t result = Core::ERROR_GENERAL;

            IARM_Bus_MFRLib_GetSerializedData_Param_t param;
            param.bufLen = 0;
            param.type = type;
            auto status = IARM_Bus_Call(
                IARM_BUS_MFRLIB_NAME, IARM_BUS_MFRLIB_API_GetSerializedData, &param, sizeof(param));
            if ((status == IARM_RESULT_SUCCESS) && param.bufLen) {
                response.assign(param.buffer, param.bufLen);
                result = Core::ERROR_NONE;
            } else {
                TRACE_GLOBAL(Trace::Information, (_T("MFR error [%d] for %d"), status, type));
            }

            return result;
        }

        uint32_t GetRFCData(const char* name, string& response)
        {
            uint32_t result = Core::ERROR_GENERAL;

            RFC_ParamData_t param;
            auto status = getRFCParameter(nullptr, name, &param);
            if ((status == WDMP_SUCCESS) && param.value[0]) {
                response = param.value;
                result = Core::ERROR_NONE;
            } else {
                TRACE_GLOBAL(Trace::Information, (_T("RFC error [%d] for %s"), status, name));
            }

            return result;
        }
    }

    SERVICE_REGISTRATION(DeviceInfoImplementation, 1, 0);

    DeviceInfoImplementation::DeviceInfoImplementation()
    {
        Utils::IARM::init();
    }

    uint32_t DeviceInfoImplementation::SerialNumber(string& serialNumber) const
    {
        return (GetMFRData(mfrSERIALIZED_TYPE_SERIALNUMBER, serialNumber)
                   == Core::ERROR_NONE)
            ? Core::ERROR_NONE
            : GetRFCData(_T("Device.DeviceInfo.SerialNumber"), serialNumber);
    }

    uint32_t DeviceInfoImplementation::Sku(string& sku) const
    {
        return (GetFileRegex(_T("/etc/device.properties"),
                    std::regex("^MODEL_NUM(?:\\s*)=(?:\\s*)(?:\"{0,1})([^\"\\n]+)(?:\"{0,1})(?:\\s*)$"), sku)
                   == Core::ERROR_NONE)
            ? Core::ERROR_NONE
            : ((GetMFRData(mfrSERIALIZED_TYPE_MODELNAME, sku)
                   == Core::ERROR_NONE)
                    ? Core::ERROR_NONE
                    : GetRFCData(_T("Device.DeviceInfo.ModelName"), sku));
    }

    uint32_t DeviceInfoImplementation::Make(string& make) const
    {
        return (GetFileRegex(_T("/etc/device.properties"),
                    std::regex("^MFG_NAME(?:\\s*)=(?:\\s*)(?:\"{0,1})([^\"\\n]+)(?:\"{0,1})(?:\\s*)$"), make)
                   == Core::ERROR_NONE)
            ? Core::ERROR_NONE
            : GetMFRData(mfrSERIALIZED_TYPE_MANUFACTURER, make);
    }

    uint32_t DeviceInfoImplementation::Model(string& model) const
    {
        return
        // TYPE_SKYMODELNAME exists for oem-sky-realtek
#ifdef ENABLE_DEVICE_MANUFACTURER_INFO
            (GetMFRData(mfrSERIALIZED_TYPE_PROVISIONED_MODELNAME, model) == Core::ERROR_NONE)
            ? Core::ERROR_NONE
            :
#endif
            GetFileRegex(_T("/etc/device.properties"),
                std::regex("^FRIENDLY_ID(?:\\s*)=(?:\\s*)(?:\"{0,1})([^\"\\n]+)(?:\"{0,1})(?:\\s*)$"), model);
    }

    uint32_t DeviceInfoImplementation::DeviceType(string& deviceType) const
    {
#ifdef BUILD_FOR_COMMUNITY
        return GetFileRegex(_T("/etc/device.properties"),
                std::regex("^DEVICE_TYPE(?:\\s*)=(?:\\s*)(?:\"{0,1})([^\"\\n]+)(?:\"{0,1})(?:\\s*)$"), deviceType);
#endif

        return GetFileRegex(_T("/etc/authService.conf"),
            std::regex("^deviceType(?:\\s*)=(?:\\s*)(?:\"{0,1})([^\"\\n]+)(?:\"{0,1})(?:\\s*)$"), deviceType);
    }

    uint32_t DeviceInfoImplementation::DistributorId(string& distributorId) const
    {
        return (GetFileRegex(_T("/opt/www/authService/partnerId3.dat"),
                    std::regex("^([^\\n]+)$"), distributorId)
                   == Core::ERROR_NONE)
            ? Core::ERROR_NONE
            : GetRFCData(_T("Device.DeviceInfo.X_RDKCENTRAL-COM_Syndication.PartnerId"), distributorId);
    }
}
}
