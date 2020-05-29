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
#include <fcntl.h>
#include <linux/rtnetlink.h>
#include <linux/if.h>
//#include <net/if.h>
#include <linux/if_arp.h>
#include <string.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include "Network.h"

//Defines
#define DELETE_ROUTE_CMD                "route delete default gw %s 2>&1" //192.168.1.1
#define RESTART_DHCPC_CMD               "/sbin/udhcpc -i %s -p /tmp/udhcpc.%s.pid 2>&1 &" //eth0:0, eth0:0

#define NETUTIL_DEVICE_PROPERTIES_FILE          "/etc/device.properties"
#define NETUTIL_PERSIST_DEFAULT_INTERFACE_FILE  "/opt/persistent/defaultInterface"
#define ESTB_IPV6_FILE_NAME "/tmp/estb_ipv6"
#define ESTB_IPV4_FILE_NAME "/tmp/estb_ipv4"
#define COMMAND_RESULT_FILE "cmdresult"

#define CONNECTED_FLAGS (IFF_UP|IFF_RUNNING)
#define INTERFACE_CONNECTED(flags) (((flags & CONNECTED_FLAGS) == CONNECTED_FLAGS) ? 1 : 0)


namespace WPEFramework {
    namespace Plugin {

        unsigned int NetUtils::m_counter = 0;
        std::mutex NetUtils::m_counterProtect;

        /*
         *
         */
        NetUtils::NetUtils() :
                m_dataProtect(),
                m_netlinkProtect()
        {
        }

        NetUtils::~NetUtils()
        {
            _stopMonitor();
        }

        /*
         * Initialise netutils:
         *      - initialise the interface list
         *      - check (and set) the persistent default interface
         *      - start the network monitor task
         */
        void NetUtils::InitialiseNetUtils()
        {
            std::string defaultInterface;

            // Initialise interface list (this is required for setting the default interface)
            _initialiseInterfaceList();
            //_displayInterfaceStatus();

            // If we are persisting the default interface, try to set it now (on boot this is unlikely
            // to work as the interfaces will probably not be up yet)
            if (NetUtils::getDefaultGatewayPersistent(defaultInterface))
            {
                if (!SetDefaultInterface(defaultInterface, true))
                {
                    LOGINFO("Setting %s as the default interface is pending...", defaultInterface.c_str());

                    // Store the setting to be applied when the interfaces are ready
                    _setPendingDefaultInterface(defaultInterface);
                }
            }

            // Start the network monitor task
            _startMonitor();
        }

        /*
         * Get the MAC address of the interface
         */
        bool NetUtils::GetInterfaceMACAddress(const std::string &interfaceDescription, std::string &macAddr)
        {
            std::lock_guard<std::recursive_mutex> lock(m_dataProtect);

            for ( auto &info : interfaceList)
            {
                if (info.m_if_descr == interfaceDescription)
                {
                    macAddr = info.m_if_macaddr;
                    return true;
                }
            }

            return false;
        }

        /*
         * Get the connection state of the interface
         */
        bool NetUtils::GetInterfaceConnected(const std::string &interfaceDescription, bool &connected)
        {
            std::lock_guard<std::recursive_mutex> lock(m_dataProtect);

            for ( auto &info : interfaceList)
            {
                if (info.m_if_descr == interfaceDescription)
                {
                    connected = INTERFACE_CONNECTED(info.m_if_flags);
                    return true;
                }
            }

            return false;
        }

        /*
         * Get the name of the interface (e.g. wlan0, wlan0:0, eth0, eth0:0)
         * If physical==false then a virtual interface name may be returned if one exists
         */
        bool NetUtils::GetInterfaceName(const std::string &interfaceDescription, std::string &name, bool physical)
        {
            std::lock_guard<std::recursive_mutex> lock(m_dataProtect);

            for ( auto &info : interfaceList)
            {
                if (info.m_if_descr == interfaceDescription)
                {
                    name = info.interfaceName(physical);
                    return true;
                }
            }

            return false;
        }

        /*
         * Get the descriptive name for the default interface (e.g. WIFI, ETHERNET etc.)
         */
        bool NetUtils::GetDefaultInterfaceDescription(std::string &description)
        {
            stringList interfaces;
            stringList gateways;
            stringList descriptions;

            if (!_getDefaultInterfaces(interfaces, gateways, descriptions))
            {
                LOGERR("Failed to get default interface information.");
            }
            else
            {
                description = descriptions[0]; // assume single default interface
                return true;
            }

            return false;
        }

        /*
         * Internal method to request the default route(s) information (from netlink) and match
         * it to our return list to get the interface name, description etc.
         * If physical=true return the name of the physical interface (eg. eth0)
         *    else it may be a virtual interface if one exists (eg. eth0:0)
         * If async=true then the netlink request will be run in a separate thread
         */
        bool NetUtils::_getDefaultInterfaces(stringList &interfaces, stringList &gateways, stringList &descriptions,
                bool physical, bool async)
        {
            // Serialise requests so we only open one netlink request socket at a time
            std::lock_guard<std::mutex> lock(m_netlinkProtect);

            indexList interfaceIndexList;
            bool success = false;

            if (async)
            {
                // Async indicates to make the netlink call in a thread (this is to do with how the socket
                // pid is defined - we can only have one netlink socket per thread/process)
                _asyncQueryDefaultInterfaces(interfaceIndexList, gateways, success);
            }
            else
            {
                _queryDefaultInterfaces(interfaceIndexList, gateways, success);
            }

            if (!success)
            {
                LOGERR("Failed to send route information request.");
            }
            else
            {
                std::lock_guard<std::recursive_mutex> lock(m_dataProtect);

                for ( auto &index : interfaceIndexList)
                {
                    for ( auto &info : interfaceList)
                    {
                        if (info.m_if_index == (int)index)
                        {
                            interfaces.push_back(info.interfaceName(physical));
                            descriptions.push_back(info.m_if_descr);
                            LOGINFO("Default route: %s, %s",
                                    info.m_if_descr.c_str(), info.interfaceName().c_str());
                            return true;
                        }
                    }
                }
            }

            return false;
        }

        /*
         * Make a netlink request to get the default interfaces (in a separate thread)
         */
        void NetUtils::_asyncQueryDefaultInterfaces(indexList &interfaces, stringList &gateways, bool &success)
        {
            std::thread asyncTask = std::thread(_queryDefaultInterfaces,
                    std::ref(interfaces), std::ref(gateways), std::ref(success));

            if (asyncTask.joinable())
                asyncTask.join();
        }

        /*
         * Make a netlink request to get the default interfaces
         */
        void NetUtils::_queryDefaultInterfaces(indexList &interfaces, stringList &gateways, bool &success)
        {
            Netlink netlinkRequest;

            success = false;

            if (!netlinkRequest.connect())
            {
                LOGERR("Failed to connect netlink request socket");
            }
            else if (NetUtils::isConfiguredIPV6())
            {
                if (!netlinkRequest.getDefaultInterfaces(interfaces, gateways, true))
                {
                    LOGWARN("Failed to get default IPv6 interfaces");
                }
                else
                {
                    success = true;
                }
            }
            else if (!netlinkRequest.getDefaultInterfaces(interfaces, gateways, false))
            {
                LOGWARN("Failed to get default interfaces");
            }
            else
            {
                success = true;
            }
        }


        /*
         * Set the default interface
         *
         * NOTE - The flag pendingRequest is used to indicate that we are trying to set the default
         * interface at startup, probably at boot time. In this case we will only proceed if the
         * interface is connected and we have a default interface. (As this may be called from the
         * monitor task we will perform the netlink request asynchronously in a separate thread)
         */
        bool NetUtils::SetDefaultInterface(std::string &interfaceDescription, bool onInit)
        {
            std::string interfaceName = "";
            bool connected = false;
            bool success = false;

            // Check the interface exists and is connected (UP and RUNNING)
            if (!GetInterfaceName(interfaceDescription, interfaceName) ||
                !GetInterfaceConnected(interfaceDescription, connected))
            {
                LOGERR("Interface not recognised");
            }
            else if (!connected)
            {
                LOGERR("Interface is not connected");
            }
            else
            {
                stringList names;
                stringList gateways;
                stringList descriptions;

                // Get a list of current default interfaces (routes)
                // We want the name of the virtual interface if it exists
                if (!_getDefaultInterfaces(names, gateways, descriptions, false, onInit))
                {
                    LOGWARN("Could not get current default interface");

                    // If this is on initialisation (we may be waiting for the network connections to
                    // be set up) then exit if we do not have a default interfaces (yet)
                    if (onInit)
                    {
                        return false;
                    }
                }

                // Kill the current udhcpc processes and delete the default interfaces
                for (unsigned i = 0; i < names.size(); i++)
                {
                    if (interfaceDescription == descriptions[i])
                    {
                        LOGINFO("%s is already a default interface", interfaceDescription.c_str());
                        success = true;
                    }
                    else
                    {
                        LOGINFO("Deleting default interface on %s", names[i].c_str());
                        _deleteDefaultInterface(names[i], gateways[i]);
                    }
                }

                if (!success)
                {
                    // Start udhcpc on the requested interface
                    success = _createDefaultInterface(interfaceName);
                }

                if (success)
                {
                    // Make sure any pending setting is cleared
                    _clearPendingDefaultInterface();
                }
            }

            return success;
        }

        /*
         * Delete the current default interface (route)
         */
        bool NetUtils::_deleteDefaultInterface(std::string &name, std::string &gateway)
        {
            std::string output = "";
            char command[MAX_COMMAND_LENGTH];
            bool success = true;

            // Terminate udhcpc on the interface
            snprintf(command, MAX_COMMAND_LENGTH, "cat /tmp/udhcpc.%s.pid 2>&1", name.c_str());
            if (NetUtils::execCmd(command, output) < 0)
            {
                LOGERR("Failed to get udhcpc pid");
            }
            else if (output.length() > 0) //pid of udhcpc
            {
                snprintf(command, MAX_COMMAND_LENGTH, "kill -9 %s 2>&1", output.c_str());
                NetUtils::execCmd(command, output);
            }

            // Delete the default route
            if (gateway.length() > 0)
            {
                snprintf(command, MAX_COMMAND_LENGTH, DELETE_ROUTE_CMD, gateway.c_str());
                if (NetUtils::execCmd(command, output) < 0)
                {
                    LOGERR("Failed to delete default route");
                    success = false;
                }
            }

            return success;
        }

        /*
         * Set the default interface (route)
         */
        bool NetUtils::_createDefaultInterface(std::string &name)
        {
            std::string output = "";
            char command[MAX_COMMAND_LENGTH];

            // Request a dhcp lease for the new interface
            snprintf(command, MAX_COMMAND_LENGTH, RESTART_DHCPC_CMD,
                    name.c_str(), name.c_str());

            if (NetUtils::execCmd(command, output) < 0)
            {
                LOGERR("Failed to create default route");
                return false;
            }
            else
            {
                return true;
            }
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

        /*
         * See if the device is configured to use ipv6
         */
        bool NetUtils::isConfiguredIPV6()
        {
            struct stat buffer;
            return (stat (ESTB_IPV6_FILE_NAME, &buffer) == 0);
        }

        /*
         * See if the device is configured to use ipv4
         */
        bool NetUtils::isConfiguredIPV4()
        {
            struct stat buffer;
            return (stat (ESTB_IPV4_FILE_NAME, &buffer) == 0);
        }

        /*
         * Get the CMTS (default) interface name
         */
#ifdef USE_NETLINK
        bool NetUtils::getCMTSInterface(std::string &interface)
        {
            stringList interfaces;
            stringList gateways;
            stringList descriptions;

            // Get the name and ip address for the default interface
            if (!_getDefaultInterfaces(interfaces, gateways, descriptions))
            {
                LOGERR("Failed to get default interface information.");
                return false;
            }
            else
            {
                interface = interfaces[0];
                return true;
            }
        }
#else
        bool NetUtils::getCMTSInterface(std::string &interface)
        {
            //Try to use DEFAULT_ESTB_INTERFACE but this may not exist on all devices
            if (!NetUtils::envGetValue("DEFAULT_ESTB_INTERFACE", interface))
            {
                //Query netsrvmgr for the active interface
                std::string interfaceDesription = "";
                if (!Network::getInstance()->_getActiveInterface(interfaceDesription))
                {
                    LOGERR("%s: failed to get active interface", __FUNCTION__);
                    return false;
                }
                //... and convert to interface name
                else if (!GetInterfaceName(interfaceDesription, interface, true)) //ignore virtual interfaces
                {
                    LOGERR("%s: failed to get active interface name", __FUNCTION__);
                    return false;
                }
            }
            return true;
        }
#endif

        /*
         * Get the CMTS gateway from configured information
         */
#ifdef USE_NETLINK
        bool NetUtils::getCMTSGateway(std::string &gateway)
        {
            stringList interfaces;
            stringList gateways;
            stringList descriptions;

            gateway = "";

            // Get the name and ip address for the default interface
            if (!_getDefaultInterfaces(interfaces, gateways, descriptions))
            {
                LOGERR("Failed to get default interface information.");
                return false;
            }
            else
            {
                gateway = gateways[0]; // assume single default interface
                return true;
            }
        }
#else
        bool NetUtils::getCMTSGateway(std::string &gateway)
        {
            std::string interface = "";
            char cmd [1000] = {0x0};

            gateway = "";

            if (getCMTSInterface(interface))
            {
                if (isConfiguredIPV6())
                {
                    snprintf(cmd, sizeof(cmd), "route -A inet6 | grep %s  | grep 'UG' | awk '{print $2}'", interface.c_str());
                    if (execCmd(cmd, gateway) < 0)
                    {
                        LOGERR("%s: failed to get IPv6 gateway address", __FUNCTION__);
                    }
                }

                // If we are not configured for ipv6 or didn't get a gateway address, default to ipv4 and try that
                if (gateway.length() == 0)
                {
                    snprintf(cmd, sizeof(cmd), "route -n | grep %s  | grep 'UG' | awk '{print $2}'", interface.c_str());
                    if (execCmd(cmd, gateway) < 0)
                    {
                        LOGERR("%s: failed to get IPv4 gateway address", __FUNCTION__);
                    }
                }

                if (gateway.length() > 0)
                {
                    // We can get multiple lines matching (especially ipv6) so delete all after the first
                    size_t lfpos = gateway.find('\n');
                    if (lfpos != std::string::npos)
                    {
                        gateway.erase(lfpos);
                    }

                    LOGINFO("gateway = %s", gateway.c_str());
                }
            }

            return (gateway.length() > 0);
        }
#endif

        /*
         * Set the persistence state of the default interface
         */
        bool NetUtils::setDefaultGatewayPersistent(const char *interface)
        {
            bool result = false;
            if (interface)
            {
                std::ofstream ofs(NETUTIL_PERSIST_DEFAULT_INTERFACE_FILE);
                if (ofs.is_open())
                {
                    ofs << interface;
                    result = ofs.fail();
                    ofs.close();
                }
            }
            else
            {
                std::remove(NETUTIL_PERSIST_DEFAULT_INTERFACE_FILE);
                result = true;
            }
            return result;
        }

        /*
         * Get the persistence state of the default interface
         */
        bool NetUtils::getDefaultGatewayPersistent(std::string &interface)
        {
            return getFile(NETUTIL_PERSIST_DEFAULT_INTERFACE_FILE, interface);
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
         * Given the interface name, check the device file so we try to match what the lower layers
         * expect/deliver (i.e stay in sync with existing methods)
         */
        bool NetUtils::_envGetInterfaceDescription(const char *name, std::string &description)
        {
            description.clear();

            if (envCheckBool("WIFI_SUPPORT"))
            {
                if (envCheckValue("WIFI_INTERFACE", name))
                {
                    description = "WIFI";
                    return true;
                }
            }
            if (envCheckBool("MOCA_SUPPORT"))
            {
                if (envCheckValue("MOCA_INTERFACE", name))
                {
                    description = "MOCA";
                    return true;
                }
            }
            if (envCheckValue("ETHERNET_INTERFACE", name))
            {
                description = "ETHERNET";
                return true;
            }

            return false;
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

        bool NetUtils::envCheckValue(const char *key, const char *value)
        {
            std::string envValue = "";
            if (!envGetValue(key, envValue))
            {
                LOGWARN("Could not find property: %s", key);
            }
            return (envValue == value);
        }

        bool NetUtils::envCheckBool(const char *key)
        {
            std::string envValue = "";
            if (!envGetValue(key, envValue))
            {
                LOGWARN("Could not find property: %s", key);
            }
            return (envValue == "true");
        }

        bool NetUtils::_parseMACAddress(unsigned char *addr, int addlen, std::string &macAddr)
        {
            for (int i = 0; i < addlen; i++)
            {
                char buffer[4];
                sprintf(buffer, "%s%02x", i ? ":" : "", addr[i]);
                buffer[3] = '\0';

                macAddr += buffer;
            }

            return macAddr.length() > 0;
        }

        /*
         * Initialise the data that we use to monitor for network changes and provide
         * information on interfaces
         */
        void NetUtils::_initialiseInterfaceList()
        {
            std::lock_guard<std::mutex> lock(m_netlinkProtect);
            struct ifaddrs *pIntfList = NULL;
            struct ifaddrs *pIntf = NULL;
            std::string description;

            getifaddrs(&pIntfList);
            if (pIntfList)
            {
                // Use AF_PACKET items to determine the physical interfaces
                for (pIntf = pIntfList; pIntf != NULL; pIntf = pIntf->ifa_next)
                {
                    if (pIntf->ifa_addr->sa_family == AF_PACKET)
                    {
                        struct sockaddr_ll *hw = (struct sockaddr_ll *)pIntf->ifa_addr;
                        struct iface_info info;
                        info.m_if_name = pIntf->ifa_name;
                        info.m_if_index = hw->sll_ifindex;
                        info.m_if_flags = pIntf->ifa_flags;

                        // We do not need set the address and flags details here
                        info.m_if_addr = "";
                        info.m_if_addrv6 = "";

                        info.m_if_vname = "";
                        info.m_if_vaddr = "";
                        info.m_if_vaddrv6 = "";

                        if ( hw->sll_hatype != ARPHRD_ETHER)
                        {
                            // Ignore non-ethernet ineterfaces (lo, sit0 etc.)
                            LOGINFO("Not ethernet interface: %s", pIntf->ifa_name);
                        }
                        // Look for the interface name in the environment file
                        else if (!_envGetInterfaceDescription(pIntf->ifa_name, info.m_if_descr))
                        {
                            // We expect the interfcaes to be defined in the device.properties file
                            LOGERR("No description for interface %s", pIntf->ifa_name);
                        }
                        else if (!_parseMACAddress(hw->sll_addr, hw->sll_halen, info.m_if_macaddr))
                        {
                            LOGERR("Could not parse mac address for interface %s", pIntf->ifa_name);
                        }
                        else
                        {
                            LOGINFO("Storing interface %s, %s, %s", info.m_if_name.c_str(), info.m_if_descr.c_str(), info.m_if_macaddr.c_str());
                            interfaceList.push_back(info);
                        }
                    }
                }

                // Use Update the interface list with addresses (and virtual interfaces)
                for ( auto &info : interfaceList)
                {
                    for (pIntf = pIntfList; pIntf != NULL; pIntf = pIntf->ifa_next)
                    {
                        if ((pIntf->ifa_addr->sa_family == AF_INET) ||
                            (pIntf->ifa_addr->sa_family == AF_INET6))
                        {
                            if ((0 == strncmp(info.m_if_name.c_str(), pIntf->ifa_name, info.m_if_name.length())) &&
                                (0 != strcmp(info.m_if_name.c_str(), pIntf->ifa_name)))
                            {
                                info.m_if_vname = pIntf->ifa_name;
                            }
                        }
                    }
                }

                freeifaddrs(pIntfList);
            }
        }

        /*
         * Background monitor methods
         */

        /*
         * Start a background network monitor task
         */
        void NetUtils::_startMonitor()
        {
            m_fdNlMonitorMsgPipe[0] = -1;
            m_fdNlMonitorMsgPipe[1] = -1;

            /* Create a pipe for posting a shutdown request */
            if (0 == pipe(m_fdNlMonitorMsgPipe))
            {
                m_netlinkMonitorThread = std::thread(_runMonitor, this);
            }
            else
            {
                LOGERR("ERROR: Failed to create netlink monitor abort pipe.");
            }
        }

        /*
         * Stop the background network monitor task
         */
        void NetUtils::_stopMonitor()
        {
            if (m_fdNlMonitorMsgPipe[1] >= 0)
            {
                if (m_netlinkMonitorThread.joinable())
                {
                    /* Post a shutdown request then wait for thread to terminate */
                    if (write(m_fdNlMonitorMsgPipe[1], &m_fdNlMonitorMsgPipe[1], sizeof(int)) != sizeof(int))
                    {
                        LOGERR("ERROR: Failed to write shutdown request. Netlink monitor task will not terminate.");
                    }
                    LOGINFO("Joining netlink connection monitor thread.");
                    m_netlinkMonitorThread.join();
                    LOGINFO("Netlink connection monitor thread terminated.");
                }

                close(m_fdNlMonitorMsgPipe[0]);
                close(m_fdNlMonitorMsgPipe[1]);
                m_fdNlMonitorMsgPipe[0] = -1;
                m_fdNlMonitorMsgPipe[1] = -1;
            }
        }

        /*
         * Netlink monitor task
         * This is currently used just to detect that a change has occurred, the details will
         * be identified in _updateInterfaceStatus() using getifaddrs
         */
        void NetUtils::_runMonitor(NetUtils *utils)
        {
            char msgBuffer[NETLINK_MESSAGE_BUFFER_SIZE];
            int msgLength;
            struct pollfd pfds[2];
            Netlink netlink;

            LOGWARN("%s : Netlink monitor RUNNING...", __FUNCTION__);

            // Connect a netlink socket to the groups we want to monitor
            if (!netlink.connect(RTMGRP_LINK|RTMGRP_IPV4_IFADDR|RTMGRP_IPV6_IFADDR))
            {
                LOGERR("Could not connect to netlionk socket.");
                return;
            }

            // first fd is the message pipe
            pfds[0].fd = utils->m_fdNlMonitorMsgPipe[0];
            pfds[0].events = POLLIN;
            pfds[0].revents = 0;

            pfds[1].fd = netlink.sockfd();
            pfds[1].events = POLLIN;
            pfds[1].revents = 0;

            utils->_lock();
            utils->_resetInterfaceList();

            while (1)
            {
                utils->_updateInterfaceStatus();
                //utils->_displayInterfaceStatus();

                // Check to see if we are waiting to set the default interface (on startup)
                utils->_checkPendingDefaultInterface();
                utils->_unlock();

                // wait for an event
                if (poll(pfds, 2, -1) <= 0)
                {
                    LOGWARN("Netlink socket poll returned no events");
                    continue;
                }

                utils->_lock();

                if (pfds[0].revents)  // Check for an event on our shutdown pipe
                {
                    LOGINFO("Shutting down netlink monitor");
                    break;
                }
                else if ((msgLength = netlink.read(msgBuffer, NETLINK_MESSAGE_BUFFER_SIZE)) < static_cast<int>(sizeof(struct nlmsghdr)) )
                {
                    // Checking netlink messages. 0=no msg & peer shutdown, -1=error, see errno
                    LOGERR("Invalid netlink message (retval %d)", msgLength);
                    std::this_thread::sleep_for (std::chrono::milliseconds(100));
                }
                else
                {
                    // At least one event relating to ip address or connection status was received. Log what types.
                    netlink.displayMessages(msgBuffer, msgLength);
                }
            }

            utils->_unlock();

            LOGWARN("%s : ENDED.", __FUNCTION__);
        }

        /*
         * Notifications from monitor thread
         */

        void NetUtils::_interfaceFlagsUpdated(struct iface_info &info)
        {
            JsonObject params;

            params["interface"] = info.m_if_descr;
            params["status"] = INTERFACE_CONNECTED(info.m_if_flags) ? "CONNECTED" : "DISCONNECTED";
            Network::_instance->_asyncNotifyConnection(params);
        }

        void NetUtils::_interfaceAddressUpdated(struct iface_info &info, std::string &addr)
        {
            JsonObject params;

            params["interface"] = info.m_if_descr;
            params["ip6Address"] = info.ipv6Addr();
            params["ip4Address"] = info.ipv4Addr();
            params["status"] = (addr.length() == 0) ? "LOST" : "ACQUIRED";
            Network::_instance->_asyncNotifyIPAddr(params);
        }

        /*
         * Reset any information needed to trigger a notification message on update
         */
        void NetUtils::_resetInterfaceList()
        {
            for ( auto &info : interfaceList)
            {
                info.m_if_flags = IFF_RUNNING;
                info.m_if_addr = "";
                info.m_if_addrv6 = "";
            }
        }

        /*
         * Update the interfaces and notify of changes to connection state or ip address
         */
        void NetUtils::_updateInterfaceStatus()
        {
            struct ifaddrs * pIntfList=NULL;
            struct ifaddrs * pIntf=NULL;

            getifaddrs(&pIntfList);

            /* For each interface in our list, see if the address or connection status has changed */
            for ( auto &info : interfaceList)
            {
                int if_flags = 0;
                char ipAddr[INET6_ADDRSTRLEN];
                std::string ipv4Addr = info.ipv4Addr();
                std::string ipv6Addr = info.ipv6Addr();
                if_flags = info.m_if_flags;

                info.m_if_addr = "";
                info.m_if_addrv6 = "";
                info.m_if_vname = "";
                info.m_if_vaddr = "";
                info.m_if_vaddrv6 = "";

                for (pIntf = pIntfList; pIntf != NULL; pIntf = pIntf->ifa_next)
                {
                    // Match the address to the interface via the interface name
                    // Note - we will assume the format of a virtual interface name is just 'name:0' (e.g. 'eth0:0')
                    // so we will match the interface name to the first part of the address interface name
                    if (0 == strncmp(info.m_if_name.c_str(), pIntf->ifa_name, info.m_if_name.length()))
                    {
                        info.m_if_flags = pIntf->ifa_flags;

                        if ((pIntf->ifa_addr->sa_family == AF_INET) ||
                            (pIntf->ifa_addr->sa_family == AF_INET6))
                        {
                            inet_ntop(pIntf->ifa_addr->sa_family, &((struct sockaddr_in *)pIntf->ifa_addr)->sin_addr, ipAddr, INET6_ADDRSTRLEN);

                            // If this is a virtual interface then the names will not match over the full length
                            if (0 != strcmp(info.m_if_name.c_str(), pIntf->ifa_name))
                            {
                                info.m_if_vname = pIntf->ifa_name;

                                if (pIntf->ifa_addr->sa_family == AF_INET)
                                {
                                    info.m_if_vaddr = ipAddr;
                                }
                                else if (pIntf->ifa_addr->sa_family == AF_INET6)
                                {
                                    info.m_if_vaddrv6 = ipAddr;
                                }
                            }
                            else
                            {
                                if (pIntf->ifa_addr->sa_family == AF_INET)
                                {
                                    info.m_if_addr = ipAddr;
                                }
                                else if (pIntf->ifa_addr->sa_family == AF_INET6)
                                {
                                    info.m_if_addrv6 = ipAddr;
                                }
                            }
                        }
                    }
                }

                /*
                 * See if anything has changed
                 */
                if (INTERFACE_CONNECTED(info.m_if_flags) != INTERFACE_CONNECTED(if_flags))
                {
                    _interfaceFlagsUpdated(info);
                }

                if (ipv4Addr != info.ipv4Addr())
                {
                    _interfaceAddressUpdated(info, info.ipv4Addr());
                }
                else if (ipv6Addr != info.ipv6Addr())
                {
                    _interfaceAddressUpdated(info, info.ipv6Addr());
                }
            }
        }

        /*
         * Check to see if we have stored a default interface to set when we are able
         */
        void NetUtils::_checkPendingDefaultInterface()
        {
            if (!NetUtils::isConfiguredIPV6() && //TBD support for default interface on IPv6
                !m_pendingDefaultInterface.empty())
            {
                LOGINFO("Try setting %s as the default interface...", m_pendingDefaultInterface.c_str());
                SetDefaultInterface(m_pendingDefaultInterface, true);
            }
        }

        void NetUtils::getTmpFilename(const char *in, std::string &out)
        {
            std::lock_guard<std::mutex> lock(m_counterProtect);
            out = "/tmp/";
            out += in;
            out += std::to_string(m_counter++);
        }

        /*
         * Debug info
         */
        void NetUtils::_displayInterfaceStatus()
        {
            for ( auto &info : interfaceList)
            {
                LOGINFO ("<%.40s>, %.40s, UP = %s, RUNNING = %s\n",
                        info.m_if_descr.c_str(),
                        info.m_if_macaddr.c_str(),
                        info.m_if_flags & IFF_UP ? "YES" : "NO",
                        info.m_if_flags & IFF_RUNNING ? "YES" : "NO");
                LOGINFO ("> <%.40s>, IP v4 Address = %.40s, IP v6 Address = %.40s.\n",
                        info.m_if_name.c_str(),
                        info.m_if_addr.c_str(),
                        info.m_if_addrv6.c_str());
                if (!info.m_if_name.empty())
                    LOGINFO ("> <%.40s>, IP v4 Address = %.40s, IP v6 Address = %.40s.\n",
                            info.m_if_vname.c_str(),
                            info.m_if_vaddr.c_str(),
                            info.m_if_vaddrv6.c_str());
            }
        }
    } // namespace Plugin
} // namespace WPEFramework
