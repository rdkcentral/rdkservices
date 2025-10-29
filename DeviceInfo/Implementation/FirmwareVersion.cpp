#include "FirmwareVersion.h"

#include <fstream>
#include <regex>
#include <sstream>     
#include <string>

extern "C" {
    #include <v_secure_popen.h>
}

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

       static uint32_t RunCommand(const std::string& command, std::string& result) {
           uint32_t ret = Core::ERROR_GENERAL;
           
           FILE* fp = v_secure_popen(command.c_str(), "r");
           if (!fp) {
              return ret;
           }

           std::ostringstream oss;
           char buffer[64];
           while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
              oss << buffer;
           }
           
           pclose(fp);
           result = oss.str();
           if (result.empty()) {
             return ret;
           }
        
           // Remove trailing newline if present
           if (result.back() == '\n') {
              result.pop_back();
           }

           return Core::ERROR_NONE;
       }

    }

    SERVICE_REGISTRATION(FirmwareVersion, 1, 0);

    uint32_t FirmwareVersion::Imagename(string& imagename) const
    {
        return GetFileRegex(_T("/version.txt"), std::regex("^imagename:([^\\n]+)$"), imagename);
    }

    uint32_t FirmwareVersion::Pdri(string& pdri) const
    {
         return RunCommand("/usr/bin/mfr_util --PDRIVersion", pdri);
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
