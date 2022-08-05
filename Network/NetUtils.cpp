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

#include "NetUtils.h"
#include <string.h>
#include "Network.h"

#include "UtilsLogging.h"

#include <fstream>

//Defines

#define NETUTIL_DEVICE_PROPERTIES_FILE          "/etc/device.properties"
#define COMMAND_RESULT_FILE "cmdresult"


namespace WPEFramework {
    namespace Plugin {

        unsigned int NetUtils::m_counter = 0;
        std::mutex NetUtils::m_counterProtect;

        /*
         *
         */
        NetUtils::NetUtils()
        {
        }

        NetUtils::~NetUtils()
        {
        }

        /*
         * Initialise netutils:
         *      - load interface descriptions
         */
        void NetUtils::InitialiseNetUtils()
        {
            _loadInterfaceDescriptions();
        }

        void NetUtils::_loadInterfaceDescriptions()
        {
            string value;

            if (envGetValue("WIFI_SUPPORT", value) && (strncasecmp(value.c_str(), "true", 4)==0) && envGetValue("WIFI_INTERFACE", value))
                interface_descriptions.insert({value, "WIFI"});
            if (envGetValue("MOCA_SUPPORT", value) && (strncasecmp(value.c_str(), "true", 4)==0) && envGetValue("MOCA_INTERFACE", value))
                interface_descriptions.insert({value, "MOCA"});
            if (envGetValue("ETHERNET_INTERFACE", value))
                interface_descriptions.insert({value, "ETHERNET"});

            for (const auto& e : interface_descriptions)
                LOGINFO ("%s %s", e.first.c_str(), e.second.c_str());
        }

        const std::string& NetUtils::getInterfaceDescription(const std::string interface)
        {
            static std::string empty("");
            auto it = interface_descriptions.find(interface.substr(0, interface.find(':'))); // look up "eth0" (real interface) for "eth0:0" (virtual interface) input also
            return (it != interface_descriptions.end()) ? it->second : empty;
        }

        /*
         * Returns >= 0 on success
         * Blocking process so run in background thread
         *
         * If 'result' is not NULL then it will be set to the return status of the command
         *
         * If 'outputfile' is not NULL then it should contain a name for the outpur file. The output of the command will
         *     be dumped to '/tmp/[outputfile]x' where x is a unique number.
         *     The filepath will be returned in 'output' and the file should be deleted by the calling process after use.
         */
        int NetUtils::execCmd(const char *command, std::string &output, bool *result, const char *outputfile)
        {
            std::string commandString;
            std::string resultFile;
            char buffer[MAX_OUTPUT_LENGTH];
            FILE *pipe = NULL;
            size_t length = 0;

            output.clear();

            commandString.assign(command);

            // If we have an output file then pipe the output from the command to /tmp/outpufile
            if (outputfile)
            {
                getTmpFilename(outputfile, output); //append a count to get a unique filename and return the filename in output
                commandString += " > ";
                commandString += output;
            }

            // If we have requested a result, store the result of the command in /tmp/cmdresult
            if (result)
            {
                getTmpFilename(COMMAND_RESULT_FILE, resultFile); //append a count to get a unique filename
                commandString += "; echo $? > ";
                commandString += resultFile;
            }

            pipe = popen(commandString.c_str(), "r");
            if (pipe == NULL)
            {
                LOGERR("%s: failed to open file '%s' for read mode with result: %s", __FUNCTION__,
                        commandString.c_str(), strerror(errno));
                return -1;
            }

            LOGWARN("%s: opened file '%s' for read mode", __FUNCTION__,
                    commandString.c_str());

            while (!feof(pipe) && fgets(buffer, MAX_OUTPUT_LENGTH, pipe) != NULL)
            {
                // If we are not dumping it to file, store the output
                if (!outputfile)
                {
                    output += buffer;
                }
            }

            // Strip trailing line feed from the output
            if ((length = output.length()) > 0)
            {
                if (output[length-1] == '\n')
                {
                    output.erase(length-1);
                }
            }

            // If we have requested a result, query the contents of the result file
            if (result)
            {
                std::string commandResult = "1";
                if (!getFile(resultFile.c_str(), commandResult, true)) //delete file after reading
                {
                    LOGERR("%s: failed to get command result '%s'", __FUNCTION__,resultFile.c_str());
                }
                else
                {
                    *result =  (commandResult == "0");
                    LOGINFO("%s: command result '%s'", __FUNCTION__,((*result)?"true":"false"));
                }
            }

            return pclose(pipe);
        }

        /*
         * See if an address is IPV4 format
         */
        bool NetUtils::isIPV4(const std::string &address)
        {
            struct in_addr ipv4address;
            return (inet_pton(AF_INET, address.c_str(), &ipv4address) > 0);
        }

        /*
         * See if an address is IPV6 format
         */
        bool NetUtils::isIPV6(const std::string &address)
        {
            struct in6_addr ipv6address;
            return (inet_pton(AF_INET6, address.c_str(), &ipv6address) > 0);
        }

        /*
         * Get the contents of a file
         * if deleteFile is true, remove the file after it is read
         *    (default == false)
         */
        bool NetUtils::getFile(const char *filepath, std::string &contents, bool deleteFile)
        {
            bool result = false;
            if (filepath)
            {
                std::ifstream ifs(filepath);
                if (ifs.is_open())
                {
                    std::getline(ifs, contents);
                    result = contents.length() > 0;
                    ifs.close();

                    if (deleteFile)
                    {
                        std::remove(filepath);
                    }
                }
            }
            return result;
        }

        /*
         * Get the value of the given key from the environment (device properties file)
         */
        bool NetUtils::envGetValue(const char *key, std::string &value)
        {
            std::ifstream fs(NETUTIL_DEVICE_PROPERTIES_FILE, std::ifstream::in);
            std::string::size_type delimpos;
            std::string line;

            if (!fs.fail())
            {
                while (std::getline(fs, line))
                {
                    if (!line.empty() &&
                        ((delimpos = line.find('=')) > 0))
                    {
                        std::string itemKey = line.substr(0, delimpos);
                        if (itemKey == key)
                        {
                            value = line.substr(delimpos + 1, std::string::npos);
                            return true;
                        }
                    }
                }
            }

            return false;
        }

        void NetUtils::getTmpFilename(const char *in, std::string &out)
        {
            std::lock_guard<std::mutex> lock(m_counterProtect);
            out = "/tmp/";
            out += in;
            out += std::to_string(m_counter++);
        }

        bool NetUtils::isIPV6LinkLocal(const std::string& address)
        {
            struct sockaddr_in6 sa6;

            if (inet_pton(AF_INET6, address.c_str(), &(sa6.sin6_addr)) == 0)
                return false;
            else
                return IN6_IS_ADDR_LINKLOCAL(&sa6.sin6_addr);
        }

        bool NetUtils::isIPV4LinkLocal(const std::string& address)
        {
            struct sockaddr_in sa;

            if (inet_pton(AF_INET, address.c_str(), &(sa.sin_addr)) == 0)
                return false;
            else
                return IN_IS_ADDR_LINKLOCAL(sa.sin_addr.s_addr);
        }

        // Not every character can be used for endpoint
        bool NetUtils::_isCharacterIllegal(const int& c)
        {
            //character must be "-./0-9a-zA-Z"
            return (c < 45) || ((c > 57) && (c < 65)) || ((c > 90) && (c < 97)) || (c > 122);
        }

        // Check if valid - consist of only allowed characters
        bool NetUtils::isValidEndpointURL(const std::string& endpoint)
        {
            return std::find_if(endpoint.begin(), endpoint.end(), _isCharacterIllegal) == endpoint.end();
        }
    } // namespace Plugin
} // namespace WPEFramework
