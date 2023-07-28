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

#include "Network.h"
#include <fcntl.h>
#include <string.h>
#include "secure_wrapper.h"
#include "UtilsLogging.h"

#define DEFAULT_IPV4_PACKET_LENGTH   52
#define DEFAULT_IPV6_PACKET_LENGTH   64
#define DEFAULT_WAIT            3
#define DEFAULT_MAX_HOPS        6
#define DEFAULT_QUERIES         3


namespace WPEFramework {
    namespace Plugin {

        bool Network::_doTraceNamedEndpoint(std::string &endpointName, int packets, JsonObject &response)
        {
            std::string interface;
            std::string endpoint = "";

            if (endpointName != "CMTS") // currently we only support CMTS
            {
                response["error"] = "Unsupported named endpoint";
            }
            else if (_getDefaultInterface(interface, endpoint) && !endpoint.empty())
            {
                return _doTrace(endpoint, packets, response);
            }
            else
            {
                response["error"] = "Could not find CMTS gateway";
            }

            return false;
        }

        bool Network::_doTrace(std::string &endpoint, int packets, JsonObject &response)
        {
            std::string error = "";
            std::string interface = "";
            std::string output = "";
            std::string gateway;
            int wait = DEFAULT_WAIT;
            int maxHops = DEFAULT_MAX_HOPS;
            size_t length = 0;
            FILE *pipe = NULL;
            char buff[MAX_OUTPUT_LENGTH];
            memset(buff, 0, sizeof(buff));

            if (packets <= 0)
            {
                packets = DEFAULT_QUERIES;
            }

            if (endpoint.empty())
            {
                error = "Invalid endpoint";
            }
            else if (!NetUtils::isIPV4(endpoint) &&
                     !NetUtils::isIPV6(endpoint) &&
                     !NetUtils::isValidEndpointURL(endpoint))
            {
                error = "Invalid endpoint";
            }
            else if (!_getDefaultInterface(interface, gateway) || interface.empty())
            {
                error = "Could not get default interface";
            }
            else
            {
                if (NetUtils::isIPV6(endpoint))
                {
                    pipe = v_secure_popen("r", "traceroute6 -i %s -w %d -m %d -q %d %s %d 2>&1",
                            interface.c_str(),
                            wait,
                            maxHops,
                            packets,
                            endpoint.c_str(),
                            DEFAULT_IPV6_PACKET_LENGTH);
                }
                else
                {
                   pipe = v_secure_popen("r", "traceroute -w %d -m %d -q %d %s %d 2>&1",
                           wait,
                           maxHops,
                           packets,
                           endpoint.c_str(),
                           DEFAULT_IPV4_PACKET_LENGTH);
                }

                if (pipe == NULL)
                {
                    LOGERR("%s: failed to open file for read mode with result: %s", __FUNCTION__,
                             strerror(errno));
                    error = "Failed to execute traceroute command";
                }
                else
                {
                    while (!feof(pipe) && fgets(buff, MAX_OUTPUT_LENGTH, pipe) != NULL)
                    {
                        output += buff;
                    }
                    // Strip trailing line feed from the output
                    if ((length = output.length()) > 0)
                    {
                        if (output[length-1] == '\n')
                        {
                            output.erase(length-1);
                        }
                    }
                    LOGWARN("traceroute command output  = %s", output.c_str());

                    //check command execution status
                    if (v_secure_pclose(pipe) != 0)
                    {
                       error = "Failed to execute traceroute";
                    }
                }
            }

            if (error.empty())
            {
                // We return the entire output of the trace command but since this contains newlines it is not valid as
                // a json value so we will parse the output into an array of strings, one element for each line.
                JsonArray list;
                if (!output.empty())
                {
                    std::string::size_type last = 0;
                    std::string::size_type next = output.find('\n');
                    while (next != std::string::npos)
                    {
                        list.Add(output.substr(last, next - last));
                        last = next + 1;
                        next = output.find('\n', last);
                    }
                    list.Add(output.substr(last));
                }

                response["target"] = endpoint;
                response["results"] = list;
                response["error"] = "";
                return true;
            }
            else
            {
                response["target"] = endpoint;
                response["results"] = "";
                response["error"] = error;
                return false;
            }
        }
    } // namespace Plugin
} // namespace WPEFramework
