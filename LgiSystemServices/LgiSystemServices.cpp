/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2020 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/
#include "LgiSystemServices.h"

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <utility>
#include <sstream>
#include <vector>
#include <algorithm>

#include "UtilsString.h"
#include "UtilsJsonRpc.h"
#include "UtilsfileExists.h"
#include "SystemServicesHelper.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 1
#define API_VERSION_NUMBER_PATCH 5

#ifndef LGISYSTEMSERVICE_DEVICEINFO_FILENAME
#define LGISYSTEMSERVICE_DEVICEINFO_FILENAME "/etc/WPEFramework/device.info"
#endif

#ifndef LGISYSTEMSERVICE_DEVICEINFO_ENVVAR_PREFIX
#define LGISYSTEMSERVICE_DEVICEINFO_ENVVAR_PREFIX "RDK_ENV_"
#endif

static std::vector<std::string> getDeviceInfoFromFile()
{
    std::vector<std::string> lines;
    std::ifstream ifs = std::ifstream(LGISYSTEMSERVICE_DEVICEINFO_FILENAME, std::ios_base::in);
    if (!ifs.is_open())
    {
        LOGWARN("could not read data file: %s", LGISYSTEMSERVICE_DEVICEINFO_FILENAME);
        return lines;
    }

    std::stringstream ss;
    ss << ifs.rdbuf();
    ifs.close();

    std::string line;
    while (std::getline(ss, line, '\n')) 
    {
        lines.push_back(line);
    }
    
    return lines;
}

static std::vector<std::string> getDeviceInfoFromEnv(char** environ_ = environ)
{
    std::vector<std::string> lines;
    char** env = environ_;
    while (*env)
    {
        std::string s(*env);
        env++;
        
        size_t n = s.find(LGISYSTEMSERVICE_DEVICEINFO_ENVVAR_PREFIX);
        if (n == std::string::npos || n > 0)
            continue;
        
        lines.push_back(s.substr(strlen(LGISYSTEMSERVICE_DEVICEINFO_ENVVAR_PREFIX)));
    }

    return lines;
}

static bool parseLine(const std::string& line, std::map<std::string, std::string>& map)
{
    size_t n = line.find_first_of("=");

    if (n == std::string::npos)
    {
        LOGERR("no '=' found in: %s", line.c_str());
        return false;
    }

    std::string key = line.substr(0, n);
    std::string val = line.substr(n + 1);

    Utils::String::trim(key);
    Utils::String::trim(val);

    if (val.empty())
    {
        LOGWARN("found empty value in: %s", line.c_str());
    }

    if (key.empty())
    {
        LOGERR("empty key is forbidden: %s", line.c_str());
        return false;
    }

    map[key] = val;

    return true;
}

static void populateDeviceInfo(std::map<std::string, std::string>& map)
{
    std::vector<std::string> lines = getDeviceInfoFromFile();
    std::vector<std::string> envLines  = getDeviceInfoFromEnv();
    lines.insert(lines.end(), envLines.begin(), envLines.end());

    map.clear();
    std::for_each(lines.begin(), lines.end(), [&](const string& l){parseLine(l, map);});
}


/**
 * @brief WPEFramework class for LgiSystemServices
 */
namespace WPEFramework {

    namespace {

        static Plugin::Metadata<Plugin::LgiSystemServices> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }

    namespace Plugin {
        SERVICE_REGISTRATION(LgiSystemServices, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        /**
         * Register SystemService module as wpeframework plugin
         */
        LgiSystemServices::LgiSystemServices()
            : PluginHost::JSONRPC()
        {
            Register("getDeviceInfo", &LgiSystemServices::getDeviceInfo, this);
            Register("getTimeZoneDST", &LgiSystemServices::getTimeZoneDST, this);
        }


        LgiSystemServices::~LgiSystemServices()
        {
        }

        const string LgiSystemServices::Initialize(PluginHost::IShell* service)
        {            
            populateDeviceInfo(m_deviceInfo);
            
            LOGINFO("Device info includes %zu items:", m_deviceInfo.size());
            for (const auto& p : m_deviceInfo)
                LOGINFO("'%s' : '%s'", p.first.c_str(), p.second.c_str());

            return std::string();
        }

        void LgiSystemServices::Deinitialize(PluginHost::IShell*)
        {
        }


        uint32_t LgiSystemServices::getDeviceInfo(const JsonObject& parameters,
                JsonObject& response)
        {
            if (!parameters.HasLabel("params") //there is no list of keys
                || parameters["params"].Content() != WPEFramework::Core::JSON::Variant::type::ARRAY //keys list is not a list
                || !parameters["params"].Array().IsSet()) //keys list is a list, but is empty
            {
                LOGINFO("Keys list is empty - sending full map");
                std::for_each(m_deviceInfo.begin(), m_deviceInfo.end(),
                    [&](const std::pair<std::string, std::string>& p){response[p.first.c_str()] = p.second;});
                returnResponse(true);    
            }

            const auto& keys = parameters["params"].Array();
            for (int i = 0; i < keys.Length(); i++)
            {
                if (keys[i].Content() != WPEFramework::Core::JSON::Variant::type::STRING)
                {
                    LOGWARN("key have to be 'string' type: %s", keys[i].String().c_str());
                    returnResponse(false);
                }

                auto e = m_deviceInfo.find(keys[i].String());
                if (e == m_deviceInfo.end())
                {
                    LOGWARN("no key found in map: %s", keys[i].String().c_str());
                    returnResponse(false);
                }

                response[e->first.c_str()] = e->second; 
            }

            returnResponse(true);
        }

        uint32_t LgiSystemServices::getTimeZoneDST(const JsonObject& parameters,
                JsonObject& response)
        {
            std::string timezone;
            bool resp = false;

            if (Utils::fileExists(TZ_FILE)) {
                if(readFromFile(TZ_FILE, timezone)) {
                    LOGWARN("Fetch TimeZone: %s\n", timezone.c_str());
                    for(const char c : timezone) {
                        LOGWARN("Fetch TimeZone: %c\n", c);
                    }
                    response["timeZone"] = timezone;
                    response["accuracy"] = "FINAL";
                    resp = true;
                } else {
                    LOGERR("Unable to open %s file.\n", TZ_FILE);
                    response["timeZone"] = "null";
                    resp = false;
                }
            } else {
                LOGERR("File not found %s.\n", TZ_FILE);
                populateResponseWithError(SysSrv_FileAccessFailed, response);
                resp = false;
            }
            returnResponse(resp);
        }
    }
}
