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

#include <cjson/cJSON.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include "utils.h"
#include "NetUtilsNetlink.h"

namespace WPEFramework {
    #define MAX_COMMAND_LENGTH      256
    #define MAX_OUTPUT_LENGTH       128

    namespace Plugin {
        class Network;

        class iface_info
        {
        public:
            iface_info() {;}
            virtual ~iface_info() {;}

            int         m_if_index;    // interface index
            std::string m_if_descr;    // interface descriptive name
            std::string m_if_macaddr;  // mac address
            int         m_if_flags;    // SIOCGIFFLAGS

            std::string m_if_name;     // interface name
            std::string m_if_addr;     // ip address v4
            std::string m_if_addrv6;   // ip address v6

            std::string m_if_vname;     // virtual interface name
            std::string m_if_vaddr;     // virtual ip address v4
            std::string m_if_vaddrv6;   // virtual ip address v6

            std::string &ipv6Addr()
            {
                if (m_if_vname.length() && m_if_vaddrv6.length())
                {
                    return m_if_vaddrv6;
                }
                return m_if_addrv6;
            }
            std::string &ipv4Addr()
            {
                if (m_if_vname.length() && m_if_vaddr.length())
                {
                    return m_if_vaddr;
                }
                return m_if_addr;
            }
            std::string &interfaceName(bool physical = false)
            {
                if (physical || (m_if_vname.length() == 0))
                {
                    return m_if_name; // return the name of the physical interface
                }
                else
                {
                    return m_if_vname; // return the name of the virtual interface
                }
            }
        };


        class NetUtils {
        private:
            std::vector<struct iface_info> interfaceList;

            void _startMonitor();
            void _stopMonitor();
            static void _runMonitor(NetUtils *utils);

            void _resetInterfaceList();
            void _updateInterfaceStatus();
            void _interfaceFlagsUpdated(struct iface_info &info);
            void _interfaceAddressUpdated(struct iface_info &info, std::string &addr);
            void _checkPendingDefaultInterface();

            void _displayInterfaceStatus();

            bool _envGetInterfaceDescription(const char *name, std::string &description);
            bool _getDefaultInterfaces(stringList &interfaces, stringList &gateways, stringList &descriptions,
                    bool physical = true, bool async = false);

            void _asyncQueryDefaultInterfaces(indexList &interfaces, stringList &gateways, bool &success);
            static void _queryDefaultInterfaces(indexList &interfaces, stringList &gateways, bool &success);

            void _initialiseInterfaceList();
            bool _parseMACAddress(unsigned char *addr, int addlen, std::string &macAddr);

            void _lock()
            {
                m_dataProtect.lock();
            }

            void _unlock()
            {
                m_dataProtect.unlock();
            }

            void _setPendingDefaultInterface(std::string &interface)
            {
                std::lock_guard<std::recursive_mutex> lock(m_dataProtect);
                m_pendingDefaultInterface = interface;
            }
            void _clearPendingDefaultInterface()
            {
                std::lock_guard<std::recursive_mutex> lock(m_dataProtect);
                m_pendingDefaultInterface.empty();
            }

        public:
            NetUtils();
            virtual ~NetUtils();

            void InitialiseNetUtils();
            bool GetInterfaceMACAddress(const std::string &interfaceDescription, std::string &macAddr);
            bool GetInterfaceConnected(const std::string &interfaceDescription, bool &connected);
            bool GetInterfaceName(const std::string &interfaceDescription, std::string &name, bool physical = false);

            bool GetDefaultInterfaceDescription(std::string &description);
            bool SetDefaultInterface(std::string &interfaceDescription, bool onInit = false);

            bool getCMTSInterface(std::string &interface);
            bool getCMTSGateway(std::string &gateway);

            static bool isIPV4(const std::string &address);
            static bool isIPV6(const std::string &address);
            static bool isConfiguredIPV6();
            static bool isConfiguredIPV4();
            static int execCmd(const char *command, std::string &output, bool *result = NULL, const char *outputfile = NULL);
            static bool setDefaultGatewayPersistent(const char *interface);
            static bool getDefaultGatewayPersistent(std::string &interface);
            static bool getFile(const char *filepath, std::string &contents, bool deleteFile = false);

            static bool envGetValue(const char *key, std::string &value);
            static bool envCheckValue(const char *key, const char *value);
            static bool envCheckBool(const char *key);

            static void getTmpFilename(const char *in, std::string &out);
            static bool isValidEndpointURL(const std::string& endpoint);

        private:
            std::thread             m_netlinkMonitorThread;
            std::recursive_mutex    m_dataProtect;
            std::mutex              m_netlinkProtect;
            int                     m_fdNlMonitorMsgPipe[2];
            bool                    m_monitorRunning;
            std::string             m_pendingDefaultInterface;

            static unsigned int     m_counter;
            static std::mutex       m_counterProtect;

            static bool _createDefaultInterface(std::string &name);
            static bool _deleteDefaultInterface(std::string &name, std::string &gateway);
            static bool _isCharacterIllegal(const int& c);
        };
    } // namespace Plugin
} // namespace WPEFramework
