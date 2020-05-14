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

namespace WPEFramework
{
    namespace Plugin
    {
        // Not every character can be used for endpoint
        bool is_character_illegal(const int& c)
        {
            //character must be "-./0-9a-zA-Z"
            return (c < 45) || ((c > 58) && (c < 97)) || (c >= 122);
        }

        // Check if valid - consist of only allowed characters
        bool is_endpoint_valid(const std::string& endpoint)
        {
            //return std::find_if(endpoint.begin(), endpoint.end(), is_character_illegal) == endpoint.end();
            return (NetUtils::isIPV4(endpoint) || NetUtils::isIPV6(endpoint));
        }

        /**
         * @ingroup SERVMGR_PING_API
         */
        JsonObject Network::_doPing(std::string endPoint, int packets)
        {
            LOGINFO("PingService calling ping");
            JsonObject pingResult;
            std::string interface = "";
            bool result = false;
            std::string outputFile;
            FILE *fp = NULL;

            pingResult["target"] = endPoint;

            if(!is_endpoint_valid(endPoint))
            {
                LOGERR("%s: Endpoint is not valid string", __FUNCTION__);
                pingResult["success"] = false;
                pingResult["error"] = "invalid input for endpoint: " + endPoint;
                return pingResult;
            }

            if (!m_netUtils.getCMTSInterface(interface))
            {
                LOGERR("%s: Could not get default interface", __FUNCTION__);
                pingResult["success"] = false;
                pingResult["error"] = "Could not get default interface";
                return pingResult;
            }

            char cmd [1000] = {0x0};
            if (NetUtils::isIPV6(endPoint))
            {
                snprintf(cmd, sizeof(cmd), "ping6 -I %s -c %d -W 5 %s 2>&1",
                        interface.c_str(), packets, endPoint.c_str());
            }
            else
            {
                snprintf(cmd, sizeof(cmd), "ping -c %d -W 5 %s 2>&1",
                        packets, endPoint.c_str());
            }

            LOGWARN("ping command: %s", cmd);

            // Run the command and dump the output to /tmp/pingoutput
            if (NetUtils::execCmd(cmd, outputFile, &result, "pingoutput") < 0)
            {
                LOGERR("%s: SERVICEMANAGER_FILE_ERROR: Can't open pipe for command '%s' for read mode: %s"
                        , __FUNCTION__, cmd, strerror(errno));

                pingResult["success"] = false;
                pingResult["error"] = "Could not run command";
            }
            else if (!result) // check the command return status
            {
                pingResult["success"] = false;
                pingResult["error"] = "Could not ping endpoint";
            }
            else if ((fp = fopen(outputFile.c_str(), "r")) == NULL)
            {
                pingResult["success"] = false;
                pingResult["error"] = "Could not read ping result";
            }
            else
            {
                pingResult["success"] = true;
                pingResult["error"] = "";

                char linearray[1000]={0x0};
                while(fgets(linearray, sizeof(linearray), fp) != NULL)
                {
                    std::string line(linearray);
                    LOGINFO("ping result: %s", line.c_str());

                    if( line.find( "packet" ) != std::string::npos ) {
                        //Example: 10 packets transmitted, 10 packets received, 0% packet loss

                        std::stringstream ss( line );
                        int transCount;
                        ss >> transCount;
                        pingResult["packetsTransmitted"] = transCount;

                        std::string token;
                        getline( ss, token, ',' );
                        getline( ss, token, ',' );
                        std::stringstream ss2( token );
                        int rxCount;
                        ss2 >> rxCount;
                        pingResult["packetsReceived"] = rxCount;

                        getline( ss, token, ',' );
                        std::string prefix = token.substr(0, token.find("%"));
                        //double lossFloat = ::atof(prefix.c_str());
                        //pingResult["packetLoss"] = lossFloat;
                        pingResult["packetLoss"] = prefix.c_str();

                    }else if( line.find( "min/avg/max" ) != std::string::npos ) {
                        //Example: round-trip min/avg/max = 17.038/18.310/20.197 ms

                        std::stringstream ss( line );
                        std::string fullpath;
                        getline( ss, fullpath, '=' );
                        getline( ss, fullpath, '=' );

                        std::string prefix;
                        int index = fullpath.find("/");
                        if (index >= 0)
                        {
                            prefix = fullpath.substr(0, fullpath.find("/"));
                            pingResult["tripMin"] = prefix.c_str();
                        }

                        index = fullpath.find("/");
                        if (index >= 0)
                        {
                            fullpath = fullpath.substr(index + 1, fullpath.length());
                            prefix = fullpath.substr(0, fullpath.find("/"));
                            pingResult["tripAvg"] = prefix.c_str();
                        }

                        index = fullpath.find("/");
                        if (index >= 0)
                        {
                            fullpath = fullpath.substr(index + 1, fullpath.length());
                            prefix = fullpath.substr(0, fullpath.find("/"));
                            pingResult["tripMax"] = prefix.c_str();
                        }

                        index = fullpath.find("/");
                        if (index >= 0)
                        {
                            fullpath = fullpath.substr(index + 1, fullpath.length());
                            pingResult["tripStdDev"] = prefix.c_str();
                        }
                    }else if( line.find( "bad" ) != std::string::npos ) {
                        pingResult["success"] = false;
                        pingResult["error"] = "Bad Address";
                    }
                }
                fclose(fp);

                // clear up
                std::remove(outputFile.c_str());
            }

            return pingResult;
        }

        /**
         * @ingroup SERVMGR_PING_API
         */
        JsonObject Network::_doPingNamedEndpoint(std::string endpointName, int packets)
        {
            LOGINFO("PingService calling pingNamedEndpoint for %s", endpointName.c_str());
            std::string error = "";
            JsonObject returnResult;

            if (endpointName == "CMTS")
            {
                std::string gateway;
                if (m_netUtils.getCMTSGateway(gateway))
                {
                    returnResult = _doPing(gateway, packets);
                }
                else
                {
                    LOGERR("%s: Can't get gateway address for interface CMTS", __FUNCTION__);
                    error = "Could not find interface";
                }
            }
            else
            {
                error = "Invalid endpoint name";
            }


            if (error != "")
            {
                returnResult["target"] = endpointName;
                returnResult["success"] = false;
                returnResult["error"] = error;
            }

            return returnResult;
        }
    }
}
