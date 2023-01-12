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

#pragma once

#include "Module.h"

#include <mutex>

namespace WPEFramework {
    namespace Plugin {
        #define NETLINK_MESSAGE_BUFFER_SIZE     8192
        #define NETLINK_MESSAGE_TIMEOUT_MS      500

        typedef std::vector<std::string> stringList;
        typedef std::vector<unsigned> indexList;

        /*
         * This is a helper class to manage a netlink socket used to get info about various network
         * parameters
         */
        class Netlink
        {
            public:
                Netlink();
                virtual ~Netlink();

                bool connect(int groups = 0);
                int read(char *buffer, int size);
                bool getDefaultInterfaces(indexList &interfaceIndex, stringList &gatewayAddress, bool ipv6 = false);
                int sockfd() { return m_fdNetlink;}

                void displayMessages(const char* msgBuffer, int msgLength);

            private:
                int                 m_fdNetlink;
                struct sockaddr_nl  m_nlSockaddr;
                std::mutex          m_netlinkProtect;

                bool _waitForReply(unsigned ms);
                bool _sendRouteRequest(bool ipv6 = false);
                int _getMessage(char *buffer, int size, unsigned msTimeout);
                bool _getRoutesInformation(indexList &defaultInterfaceIndex, stringList &gatewayAddress);
                bool _parseRoute(void *msg, unsigned &index, std::string &destination, std::string &gateway);
        };
    } // namespace Plugin
} // namespace WPEFramework
