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

#include "NetUtilsNetlink.h"
#include <fcntl.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <string.h>
#include <sys/types.h>

#include "UtilsLogging.h"

namespace WPEFramework {
    namespace Plugin {

        Netlink::Netlink() :
            m_fdNetlink(-1),
            m_netlinkProtect()
        {
        }

        Netlink::~Netlink()
        {
            if (m_fdNetlink != -1)
            {
                close(m_fdNetlink);
                m_fdNetlink = -1;
            }
        }

        /*
         * Create a netlink socket
         * Note - the socket is set to non-blocking so use select/poll to manage waiting and timeout
         *      - only one netlink connection should be made per thread (nl_pid should be unique)
         */
        bool Netlink::connect(int groups)
        {
            std::lock_guard<std::mutex> lock(m_netlinkProtect);

            memset(&m_nlSockaddr, 0, sizeof(m_nlSockaddr));
            m_nlSockaddr.nl_family = AF_NETLINK;
            m_nlSockaddr.nl_groups = groups;
            m_nlSockaddr.nl_pid = pthread_self();
            m_nlSockaddr.nl_pid <<= 16;
            m_nlSockaddr.nl_pid += getpid();

            m_fdNetlink = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
            if (m_fdNetlink == -1)
            {
                LOGERR("Failed to create Netlink socket");
                return false;
            }

            if (bind(m_fdNetlink, (struct sockaddr *)&m_nlSockaddr, sizeof(m_nlSockaddr)) < 0)
            {
                LOGERR("Failed to bind to Netlink socket: %s", strerror(errno));
                close(m_fdNetlink);
                m_fdNetlink = -1;
                return false;
            }

            if (fcntl(m_fdNetlink, F_SETFL, O_NONBLOCK) == -1)
            {
                LOGWARN("Failed to set Netlink socket to non-blocking");
            }

            return true;
        }

        /*
         * Read data from the socket
         * Wait for msTimeout milliseconds for data (do not wait if msTimeout is zero)
         * Note - this will not block if data is not currently available
         */
        int Netlink::read(char *buffer, int size)
        {
            std::lock_guard<std::mutex> lock(m_netlinkProtect);
            return _getMessage(buffer, size, 0);
        }

        /*
         * Get the default interface from netlink
         */
        bool Netlink::getDefaultInterfaces(indexList &interfaceIndex, stringList &gatewayAddress, bool ipv6)
        {
            std::lock_guard<std::mutex> lock(m_netlinkProtect);
            if (!_sendRouteRequest(ipv6))
            {
                LOGERR("Failed to send route information request.");
            }
            else if (!_getRoutesInformation(interfaceIndex, gatewayAddress))
            {
                LOGINFO("Failed to find default route information.");
            }
            else
            {
                return true;
            }

            return false;
        }

        /*
         * DEBUG function to log netlink messages in buffer
         */
        void Netlink::displayMessages(const char* msgBuffer, int msgLength)
        {
            struct nlmsghdr *nlhdr;
            nlhdr = (struct nlmsghdr *) msgBuffer;
            struct ifinfomsg *ifinfo;

            // Display any netlink messages received.
            for (nlhdr = (struct nlmsghdr *)msgBuffer;
                 NLMSG_OK(nlhdr, msgLength);
                 nlhdr = NLMSG_NEXT(nlhdr, msgLength))
            {
                if (nlhdr->nlmsg_type == NLMSG_DONE)
                {
                    break;
                }
                switch (nlhdr->nlmsg_type)
                {
                    case RTM_DELADDR:
                        LOGINFO("Netlink DELADDR EVENT...");
                        break;
                    case RTM_DELLINK:
                        LOGINFO("Netlink DISCONNECTION EVENT...");
                        break;
                    case RTM_NEWLINK:
                        LOGINFO("Netlink NEWLINK EVENT...");
                        break;
                    case RTM_NEWADDR:
                        LOGINFO("Netlink NEWADDR EVENT...");
                        break;
                    default:
                        LOGINFO("Netlink OTHER EVENT (%d)...",nlhdr->nlmsg_type);
                        break;
                }

                if (  nlhdr->nlmsg_type == RTM_NEWLINK )
                {
                    ifinfo = (struct ifinfomsg *)NLMSG_DATA(nlhdr);
                    if ((ifinfo->ifi_flags & IFF_UP) &&
                        (ifinfo->ifi_flags & IFF_RUNNING))
                    {
                        LOGINFO("EVENT TYPE = CONNECTED...");
                    }
                }
            }
        }


        /*
         * Internal functions
         */

        /*
         * Wait for data returned by the socket for specified time
         */
        bool Netlink::_waitForReply(unsigned ms)
        {
            struct timeval timeout;
            fd_set readFDSet;

            FD_ZERO(&readFDSet);
            FD_SET(m_fdNetlink, &readFDSet);

            timeout.tv_sec = (ms / 1000);
            timeout.tv_usec = ((ms % 1000) * 1000);

            if (select(m_fdNetlink + 1, &readFDSet, NULL, NULL, &timeout) > 0)
            {
                return FD_ISSET(m_fdNetlink, &readFDSet);
            }
            return false;
        }

        int Netlink::_getMessage(char *buffer, int size, unsigned msTimeout)
        {
            struct iovec iov;
            struct msghdr msg;
            int replyLength = -1;

            if (msTimeout)
            {
                if (!_waitForReply(msTimeout))
                {
                    LOGERR("No message received");
                    return false;
                }
            }

            iov.iov_base = buffer;
            iov.iov_len = size;

            msg.msg_name = &m_nlSockaddr;
            msg.msg_namelen = sizeof(m_nlSockaddr);
            msg.msg_iov = &iov;
            msg.msg_iovlen = 1;
            msg.msg_control = NULL;
            msg.msg_controllen = 0;
            msg.msg_flags = 0;

            if ((replyLength = recvmsg(m_fdNetlink, &msg, 0)) < static_cast<int>(sizeof(struct nlmsghdr)))
            {
                LOGERR("Unable to read message");
                replyLength = -1;
            }

            return replyLength;
        }

        /*
         * Functions for requesting and processing network route information
         */

        /*
         * Send a netlink request for all network route information
         */
        bool Netlink::_sendRouteRequest(bool ipv6)
        {
            struct messageBuffer {
                struct nlmsghdr netlinkRequesthdr;
                struct rtmsg request;
            } requestMessage;

            memset(&requestMessage, 0, sizeof(struct messageBuffer));

            requestMessage.netlinkRequesthdr.nlmsg_type = RTM_GETROUTE;
            requestMessage.netlinkRequesthdr.nlmsg_len = sizeof(requestMessage);
            requestMessage.netlinkRequesthdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
            requestMessage.netlinkRequesthdr.nlmsg_seq = 1;
            requestMessage.request.rtm_family = ipv6 ? AF_INET6 : AF_INET;

            if (send(m_fdNetlink, &requestMessage, sizeof(requestMessage), 0) < 0)
            {
                LOGERR("Failed to send socket message: %s", strerror(errno));
                return false;
            }

            return true;
        }

        /*
         * Retrieve the route information data and parse it to retrieve the default route
         */
        bool Netlink::_getRoutesInformation(indexList &defaultInterfaceIndex, stringList &gatewayAddress)
        {
            char msgBuffer[NETLINK_MESSAGE_BUFFER_SIZE];
            struct nlmsghdr *nlhdr;
            int msgLength = -1;
            unsigned index = 0;
            std::string destination = "";
            std::string gateway = "";

            defaultInterfaceIndex.clear();
            gatewayAddress.clear();

            /* Retrieve Netlink messages */
            if ((msgLength = _getMessage(msgBuffer, NETLINK_MESSAGE_BUFFER_SIZE, NETLINK_MESSAGE_TIMEOUT_MS)) < 0 )
            {
                LOGERR("Unable to read message");
                return false;
            }

            nlhdr = (struct nlmsghdr *)msgBuffer;

            for (nlhdr = (struct nlmsghdr *)msgBuffer;
                 NLMSG_OK(nlhdr, msgLength);
                 nlhdr = NLMSG_NEXT(nlhdr, msgLength))
            {
                if (nlhdr->nlmsg_type == NLMSG_DONE)
                {
                    break;
                }
                else if (nlhdr->nlmsg_type == RTM_NEWROUTE)
                {
                    if (_parseRoute(nlhdr, index, destination, gateway))
                    {
                        if (destination.length() == 0) // default route has no destination address
                        {
                            LOGINFO("Default interface found: %d, %s", index, gateway.c_str());
                            gatewayAddress.push_back(gateway);
                            defaultInterfaceIndex.push_back(index);
                        }
                    }
                }
                else
                {
                    // Ignore other message types
                }
            }

            return !defaultInterfaceIndex.empty();
        }

        /*
         * Parse the route message returning the interface index and the destination address
         */
        bool Netlink::_parseRoute(void *msg, unsigned &index, std::string &destination, std::string &gateway)
        {
            struct nlmsghdr *nlhdr = (struct nlmsghdr *)msg;
            struct rtmsg *routeMsg;
            struct rtattr *attribute;
            int attrLength = -1;
            char ipAddress[INET6_ADDRSTRLEN];

            routeMsg = (struct rtmsg *)NLMSG_DATA(nlhdr);

            if (routeMsg->rtm_table != RT_TABLE_MAIN)
            {
                // we only want the main routing table information
                return false;
            }

            if ((routeMsg->rtm_family != AF_INET) && (routeMsg->rtm_family != AF_INET6))
            {
                return false;
            }

            index = 0;

            attrLength = nlhdr->nlmsg_len - NLMSG_LENGTH(sizeof(struct rtmsg));
            for (attribute = RTM_RTA(routeMsg);
                    RTA_OK(attribute, attrLength);
                    attribute = RTA_NEXT(attribute, attrLength))
            {
                if (attribute->rta_type == RTA_OIF)
                {
                    index = *(unsigned *)RTA_DATA(attribute);
                }
                else if (attribute->rta_type == RTA_DST)
                {
                    inet_ntop(routeMsg->rtm_family, RTA_DATA(attribute), ipAddress, INET6_ADDRSTRLEN);
                    destination = ipAddress;
                }
                else if (attribute->rta_type == RTA_GATEWAY)
                {
                    inet_ntop(routeMsg->rtm_family, RTA_DATA(attribute), ipAddress, INET6_ADDRSTRLEN);
                    gateway = ipAddress;
                }
                else
                {
                    // ignore the rest for now
                }
            }

            //TBD - how to correctly determine the IPV6 gateway / default route?
            if (routeMsg->rtm_family == AF_INET6)
            {
                if ((routeMsg->rtm_scope == RT_SCOPE_UNIVERSE) &&
                    (routeMsg->rtm_type == RTN_UNICAST) &&
                    (routeMsg->rtm_protocol == RTPROT_RA))
                {
                    destination = "";
                }
            }

            return index > 0;
        }

    } // namespace Plugin
} // namespace WPEFramework
