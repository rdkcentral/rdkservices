#include "FirmwareVersion.h"

#include <fstream>
#include <regex>
#include <sstream>     
#include <string>
#include "secure_wrapper.h"

// #define MFRUTIL "/usr/bin/mfr_util %s"

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
                    if ((std::regex_match(line, sm, regex)) && (sm.size() > 1)) {
                        response = sm[1];

                        result = Core::ERROR_NONE;
                        break;
                    }
                }
            }

            return result;
        }

    }

    SERVICE_REGISTRATION(FirmwareVersion, 1, 0);

    uint32_t FirmwareVersion::Imagename(string& imagename) const
    {
        return GetFileRegex(_T("/version.txt"), std::regex("^imagename:([^\\n]+)$"), imagename);
    }

    uint32_t FirmwareVersion::Pdri(string& pdri) const
    {
         uint32_t ret = Core::ERROR_GENERAL;
                      
         FILE* fp = v_secure_popen("r", "/usr/bin/mfr_util --PDRIVersion");
         if (!fp) {
            return ret;
         }

         std::ostringstream oss;
         char buffer[64];
         while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
             oss << buffer;
         }
           
         pclose(fp);
         pdri = oss.str();
         TRACE_GLOBAL(Trace::Fatal, (_T("preeja  pdri=%s"), pdri.c_str()));
                  
         // Remove trailing newline if present
         if (!pdri.empty() && pdri.back() == '\n') {
             pdri.pop_back();
         }

         return Core::ERROR_NONE;
    }

    uint32_t FirmwareVersion::Sdk(string& sdk) const
    {
        return GetFileRegex(_T("/version.txt"), std::regex("^SDK_VERSION=([^\\n]+)$"), sdk);
    }

    uint32_t FirmwareVersion::Mediarite(string& mediarite) const
    {
        return GetFileRegex(_T("/version.txt"), std::regex("^MEDIARITE=([^\\n]+)$"), mediarite);
    }

    uint32_t FirmwareVersion::Yocto(string& yocto) const
    {
        return GetFileRegex(_T("/version.txt"), std::regex("^YOCTO_VERSION=([^\\n]+)$"), yocto);
    }
}
}
