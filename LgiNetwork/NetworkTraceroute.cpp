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

//e.g. traceroute -w 3 -m 6 -q 3 192.168.1.100 52
#define CMD_TRACEROUTE "traceroute -w %d -m %d -q %d %s %d 2>&1"
//e.g. traceroute6 -i eth0 -w 3 -m 6 -q 3 fe80::5a19:f8ff:fe37:7a3d 52
#define CMD_TRACEROUTE6 "traceroute6 -i %s -w %d -m %d -q %d %s %d 2>&1"

#define DEFAULT_PACKET_LENGTH   52
#define DEFAULT_WAIT            3
#define DEFAULT_MAX_HOPS        6
#define DEFAULT_QUERIES         3


namespace WPEFramework {
    namespace Plugin {

        bool Network::_doTraceNamedEndpoint(std::string &endpointName, int packets, JsonObject &response)
        {
            std::string interface;
            std::string endpoint = "";
            std::string type = "";

            if (endpointName != "CMTS") // currently we only support CMTS
            {
                response["error"] = "Unsupported named endpoint";
            }
            else if (_getDefaultInterface(interface, endpoint, type) && !endpoint.empty())
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
            std::string output = "";
            std::string error = "";
            std::string interface = "";
            std::string gateway;
            std::string type;
            int wait = DEFAULT_WAIT;
            int maxHops = DEFAULT_MAX_HOPS;
            int packetLen = DEFAULT_PACKET_LENGTH;
            char command[MAX_COMMAND_LENGTH];
            bool result = false;

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
            else if (!_getDefaultInterface(interface, gateway, type) || interface.empty())
            {
                error = "Could not get default interface";
            }
            else
            {
                if (NetUtils::isIPV6(endpoint))
                {
                    snprintf(command, MAX_COMMAND_LENGTH, CMD_TRACEROUTE6,
                            interface.c_str(),
                            wait,
                            maxHops,
                            packets,
                            endpoint.c_str(),
                            packetLen);
                }
                else
                {
                    snprintf(command, MAX_COMMAND_LENGTH, CMD_TRACEROUTE,
                            wait,
                            maxHops,
                            packets,
                            endpoint.c_str(),
                            packetLen);
                }

                if (NetUtils::execCmd(command, output, &result) < 0)
                {
                    error = "Failed to execute traceroute command";
                }
                else if (!result) // check the command return status
                {
                    error = "Failed to execute traceroute";
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
