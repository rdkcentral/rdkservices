#include "FirmwareVersion.h"

#include <fstream>
#include <regex>
#include <cstdio>      
#include <sstream>     
#include <string>

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

        uint32_t GetStringRegex(const string& input, const std::regex& regex) {
            if ((std::regex_search(input, regex))) {
                return Core::ERROR_GENERAL;
            }
            TRACE_GLOBAL(Trace::Fatal, (_T("preeja3 return success")));
            return Core::ERROR_NONE;
        }

       static bool RunCommand(const std::string& command, std::string& result) {
           FILE* fp = popen(command.c_str(), "r");
           if (!fp) {
              return false;
           }

           std::ostringstream oss;
           char buffer[64];
           while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
              oss << buffer;
           }
           
           pclose(fp);
           result = oss.str();
           TRACE_GLOBAL(Trace::Fatal, (_T("preeja2 result %s"), result.c_str()));
           if (result.empty()) {
             return false;
           }
           
           return true;
       }

    }

    SERVICE_REGISTRATION(FirmwareVersion, 1, 0);

    uint32_t FirmwareVersion::Imagename(string& imagename) const
    {
        return GetFileRegex(_T("/version.txt"), std::regex("^imagename:([^\\n]+)$"), imagename);
    }

    uint32_t FirmwareVersion::NewImage(string& pdri) const
    {
       if (RunCommand("/usr/bin/mfr_util --PDRIVersion", pdri)) {
           return GetStringRegex(pdri, std::regex("failed", std::regex_constants::icase));
       }
        
       return Core::ERROR_GENERAL;
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
