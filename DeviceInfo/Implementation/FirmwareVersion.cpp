#include "FirmwareVersion.h"

#include <fstream>
#include <regex>
#include <sstream>     
#include <string>
#include "secure_wrapper.h"

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

        bool GetStringRegex(const string& input, const std::regex& regex) {
            if ((std::regex_search(input, regex))) {
                return true;
            }
            
            return false;
        }
    }

    SERVICE_REGISTRATION(FirmwareVersion, 1, 0);

    uint32_t FirmwareVersion::Imagename(string& imagename) const
    {
        return GetFileRegex(_T("/version.txt"), std::regex("^imagename:([^\\n]+)$"), imagename);
    }

    uint32_t FirmwareVersion::Pdri(string& pdri) const
    {
         uint32_t result = Core::ERROR_GENERAL;
                      
         FILE* fp = v_secure_popen("r", "/usr/bin/mfr_util --PDRIVersion");
         if (!fp) {
            return result;
         }

         std::ostringstream oss;
         char buffer[256];
         while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
             oss << buffer;
         }
           
         v_secure_pclose(fp);
         pdri = oss.str();
         if (pdri.empty())
             return result;
         pdri="IARM_Bus_Call provider returned error (3) for the method mfrGetManufacturerData Call failed for mfrSERIALIZED_TYPE_PDRIVERSION: error code:3\n";                  
         // Remove trailing newline if present
         if (pdri.back() == '\n')
             pdri.pop_back();
        
        // Return empty as PDRI version when device not have pdri image
        if (GetStringRegex(pdri, std::regex("failed"))) {
            pdri = "";
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
