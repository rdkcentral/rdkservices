/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2023 RDK Management
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
 */

#include "MiracastController.h"

void ControllerThreadCallback(void *args);
#ifdef ENABLE_MIRACAST_SERVICE_TEST_NOTIFIER
void MiracastServiceTestNotifierThreadCallback(void *args);
#endif

MiracastController *MiracastController::m_miracast_ctrl_obj{nullptr};

MiracastController *MiracastController::getInstance(MiracastError &error_code, MiracastServiceNotifier *notifier, std::string p2p_ctrl_iface)
{
    MIRACASTLOG_TRACE("Entering...");
    if (nullptr == m_miracast_ctrl_obj)
    {
        m_miracast_ctrl_obj = new MiracastController();
        if (nullptr != m_miracast_ctrl_obj)
        {
            m_miracast_ctrl_obj->m_notify_handler = notifier;
            error_code = m_miracast_ctrl_obj->create_ControllerFramework(p2p_ctrl_iface);
            if ( MIRACAST_OK != error_code )
            {
                delete m_miracast_ctrl_obj;
                m_miracast_ctrl_obj = nullptr;
            }
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
    return m_miracast_ctrl_obj;
}

void MiracastController::destroyInstance()
{
    MIRACASTLOG_TRACE("Entering...");
    if (nullptr != m_miracast_ctrl_obj)
    {
        m_miracast_ctrl_obj->destroy_ControllerFramework();
        delete m_miracast_ctrl_obj;
        m_miracast_ctrl_obj = nullptr;
    }
    MIRACASTLOG_TRACE("Exiting...");
}

MiracastController::MiracastController(void)
{
    MIRACASTLOG_TRACE("Entering...");

    m_groupInfo = nullptr;
    m_p2p_ctrl_obj = nullptr;
    m_controller_thread = nullptr;
    m_tcpserverSockfd = -1;
    setP2PBackendDiscovery(false);

    MIRACASTLOG_TRACE("Exiting...");
}

MiracastController::~MiracastController()
{
    MIRACASTLOG_TRACE("Entering...");

    while (!m_deviceInfoList.empty())
    {
        delete m_deviceInfoList.back();
        m_deviceInfoList.pop_back();
    }

    if (nullptr != m_groupInfo)
    {
        delete m_groupInfo;
        m_groupInfo = nullptr;
    }

    MIRACASTLOG_TRACE("Exiting...");
}

MiracastError MiracastController::create_ControllerFramework(std::string p2p_ctrl_iface)
{
    MiracastError ret_code = MIRACAST_OK;
    MIRACASTLOG_TRACE("Entering...");

    m_controller_thread = new MiracastThread(CONTROLLER_THREAD_NAME,
                                             CONTROLLER_THREAD_STACK,
                                             CONTROLLER_MSGQ_COUNT,
                                             CONTROLLER_MSGQ_SIZE,
                                             reinterpret_cast<void (*)(void *)>(&ControllerThreadCallback),
                                             this);
    if ((nullptr == m_controller_thread)||
        ( MIRACAST_OK != m_controller_thread->start()))
    {
        ret_code = MIRACAST_CONTROLLER_INIT_FAILED;
    }
    else{
        m_p2p_ctrl_obj = MiracastP2P::getInstance(ret_code,p2p_ctrl_iface);
    }
    if ( MIRACAST_OK != ret_code ){
        destroy_ControllerFramework();
    }
    MIRACASTLOG_TRACE("Exiting...");
    return ret_code;
}

MiracastError MiracastController::destroy_ControllerFramework(void)
{
    MIRACASTLOG_TRACE("Entering...");

#ifdef ENABLE_MIRACAST_SERVICE_TEST_NOTIFIER
    destroy_TestNotifier();
#endif
    CONTROLLER_MSGQ_STRUCT controller_msgq_data = {0};
    MIRACASTLOG_INFO("[MIRACAST_SERVICE_SHUTDOWN]");
    controller_msgq_data.state = CONTROLLER_SELF_ABORT;
    send_thundermsg_to_controller_thread(controller_msgq_data);

    if (nullptr != m_p2p_ctrl_obj)
    {
        MiracastP2P::destroyInstance();
        m_p2p_ctrl_obj = nullptr;
    }
    if (nullptr != m_controller_thread){
        delete m_controller_thread;
        m_controller_thread = nullptr;
    }
    MIRACASTLOG_TRACE("Exiting...");
    return MIRACAST_OK;
}

std::string MiracastController::parse_p2p_event_data(const char *tmpBuff, const char *lookup_data)
{
    char return_buf[1024] = {0};
    const char  *ret = nullptr, 
                *ret_equal = nullptr,
                *ret_space = nullptr,
                *single_quote_start = nullptr,
                *single_quote_end = nullptr;
    ret = strstr(tmpBuff, lookup_data);

    if (nullptr != ret)
    {
        if (0 == strncmp(ret, lookup_data, strlen(lookup_data)))
        {
            ret_equal = strstr(ret, "=");
            ret_space = strstr(ret_equal, " ");

            if (0 == strncmp("name", lookup_data, strlen(lookup_data))){
                single_quote_start = strstr(ret_equal, "'");
                single_quote_end = strstr(single_quote_start + 1, "'");
            }

            if (single_quote_start && single_quote_end) {
                unsigned int length = single_quote_end - single_quote_start;
                if (length < sizeof(return_buf)) {
                    snprintf(return_buf, length, "%s", single_quote_start + 1);
                }
            }
            else if (ret_space)
            {
                snprintf(return_buf, (int)(ret_space - ret_equal), "%s", ret + strlen(lookup_data) + 1);
                MIRACASTLOG_VERBOSE("Parsed Data is - %s", return_buf);
            }
            else
            {
                snprintf(return_buf, strlen(ret_equal - 1), "%s", ret + strlen(lookup_data) + 1);
                MIRACASTLOG_VERBOSE("Parsed Data is - %s", return_buf);
            }
        }
    }
    if (return_buf != nullptr)
        return std::string(return_buf);
    else
        return std::string(" ");
}

std::string MiracastController::getifNameByIPv4(std::string ip_address)
{
    struct ifaddrs *ifaddrList = nullptr, *currentifa = nullptr;
    int family, s;
    char host[MAX_IFACE_NAME_LEN];
    std::string ifaceName = "";

    // Get list of all network interfaces
    if ( -1 == getifaddrs(&ifaddrList))
    {
        MIRACASTLOG_ERROR("getifaddrs failed[%s]",strerror(errno));
    }
    else
    {
        // Iterate through the list of network interfaces
        for ( currentifa = ifaddrList; nullptr != currentifa; currentifa = currentifa->ifa_next)
        {
            if ( nullptr == currentifa->ifa_addr )
            {
                continue;
            }
            family = currentifa->ifa_addr->sa_family;
            // Check for IPv4 address
            if ( AF_INET == family )
            {
                s = getnameinfo(currentifa->ifa_addr, sizeof(struct sockaddr_in), host, MAX_IFACE_NAME_LEN, NULL, 0, NI_NUMERICHOST);
                if (s != 0)
                {
                    MIRACASTLOG_ERROR("getnameinfo failed[%s]",strerror(errno));
                    break;
                }
                // Compare IP address with the given IP
                if ( 0 == strcmp(host, ip_address.c_str()))
                {
                    ifaceName = currentifa->ifa_name;
                }
            }
        }
        if ( nullptr != ifaddrList )
        {
            freeifaddrs(ifaddrList);
            ifaddrList = nullptr;
        }
    }
    return ifaceName;
}

std::string MiracastController::start_DHCPClient(std::string interface, std::string &default_gw_ip_addr)
{
    MIRACASTLOG_TRACE("Entering...");
    char command[128] = {0};
    char sys_cls_file_ifidx[128] = {0};
    std::string local_addr = "",
                gw_ip_addr = "",
                popen_buffer = "",
                system_cmd_buffer = "";
    FILE *popen_file_ptr = nullptr;
    char *current_line_buffer = nullptr;
    std::size_t len = 0;
    unsigned char retry_count = 5;

    sprintf( sys_cls_file_ifidx , "/sys/class/net/%s/ifindex" , interface.c_str());

    std::ifstream ifIndexFile(sys_cls_file_ifidx);

    if (!ifIndexFile.good()) {
        MIRACASTLOG_ERROR("Could not find [%s]\n",sys_cls_file_ifidx);
        return std::string("");
    }

    sprintf(command, "/sbin/udhcpc -v -i ");
    sprintf(command + strlen(command), "%s" , interface.c_str());
    sprintf(command + strlen(command), " -s /etc/netsrvmgr/p2p_udhcpc.script 2>&1");
    MIRACASTLOG_VERBOSE("command : [%s]", command);

    while ( retry_count-- )
    {
        popen_file_ptr = popen(command, "r");
        if (!popen_file_ptr)
        {
            MIRACASTLOG_ERROR("Could not open pipe for output.");
        }
        else
        {
            std::smatch match;
            std::regex localipRegex(R"(lease\s+of\s+(\d+\.\d+\.\d+\.\d+)\s+obtained)");
            std::regex goipRegex1(R"(default\s+gw\s+(\d+\.\d+\.\d+\.\d+)\s+dev)");
            std::regex goipRegex2(R"(Adding\s+DNS\s+(\d+\.\d+\.\d+\.\d+))", std::regex_constants::icase);

            MIRACASTLOG_VERBOSE("udhcpc output as below:\n");

            while (getline(&current_line_buffer, &len, popen_file_ptr) != -1)
            {
                MIRACASTLOG_VERBOSE("[%s]", current_line_buffer);
                popen_buffer = current_line_buffer;

                if ( local_addr.empty() && (std::regex_search(popen_buffer, match, localipRegex)))
                {
                    local_addr = match[1];
                    MIRACASTLOG_INFO("local IP addr obtained is %s\n", local_addr.c_str());
                }

                /* Here retrieved the default gw ip address. Later it can be used as GO IP address if P2P-GROUP started as PERSISTENT */
                if ( gw_ip_addr.empty() && (std::regex_search(popen_buffer, match, goipRegex1)))
                {
                    gw_ip_addr = match[1];
                    MIRACASTLOG_INFO("GO IP addr obtained is %s\n", gw_ip_addr.c_str());
                }
                else if ( gw_ip_addr.empty() && (std::regex_search(popen_buffer, match, goipRegex2)))
                {
                    gw_ip_addr = match[1];
                    MIRACASTLOG_INFO("GO IP addr obtained is %s\n", gw_ip_addr.c_str());
                }
            }
            MIRACASTLOG_VERBOSE("udhcpc output done\n");
            pclose(popen_file_ptr);
            popen_file_ptr = nullptr;

            free(current_line_buffer);
            current_line_buffer = nullptr;

            if (!local_addr.empty()){
                MIRACASTLOG_INFO("%s is success\n", command);
                default_gw_ip_addr = gw_ip_addr;
                break;
            }
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
    return local_addr;
}

std::string MiracastController::start_DHCPServer(std::string interface)
{
    MIRACASTLOG_TRACE("Entering...");
    std::string command = "";

    command = "ifconfig ";
    command.append(interface.c_str());
    command.append(" 192.168.59.1 netmask 255.255.255.0 up");
    MIRACASTLOG_INFO("command : [%s]", command.c_str());
    MiracastCommon::execute_SystemCommand(command.c_str());

    command = "/usr/bin/dnsmasq -p0 -i ";
    command.append(interface.c_str());
    command.append(" -F 192.168.59.50,192.168.59.230,255.255.255.0,24h --log-queries=extra");
    MIRACASTLOG_INFO("command : [%s]", command.c_str());
    MiracastCommon::execute_SystemCommand(command.c_str());

    MIRACASTLOG_TRACE("Exiting...");

    return "192.168.59.1";
}

eCONTROLLER_FW_STATES MiracastController::convertP2PtoSessionActions(P2P_EVENTS eventId)
{
    eCONTROLLER_FW_STATES state = CONTROLLER_INVALID_STATE;

    switch (eventId)
    {
    case EVENT_FOUND:
    {
        state = CONTROLLER_GO_DEVICE_FOUND;
    }
    break;
    case EVENT_PROVISION:
    case EVENT_SHOW_PIN:
    {
        state = CONTROLLER_GO_DEVICE_PROVISION;
    }
    break;
    case EVENT_STOP:
    {
        state = CONTROLLER_GO_STOP_FIND;
    }
    break;
    case EVENT_GO_NEG_REQ:
    {
        state = CONTROLLER_GO_NEG_REQUEST;
    }
    break;
    case EVENT_GO_NEG_SUCCESS:
    {
        state = CONTROLLER_GO_NEG_SUCCESS;
    }
    break;
    case EVENT_GO_NEG_FAILURE:
    {
        state = CONTROLLER_GO_NEG_FAILURE;
    }
    break;
    case EVENT_GROUP_STARTED:
    {
        state = CONTROLLER_GO_GROUP_STARTED;
    }
    break;
    case EVENT_FORMATION_SUCCESS:
    {
        state = CONTROLLER_GO_GROUP_FORMATION_SUCCESS;
    }
    break;
    case EVENT_FORMATION_FAILURE:
    {
        state = CONTROLLER_GO_GROUP_FORMATION_FAILURE;
    }
    break;
    case EVENT_DEVICE_LOST:
    {
        state = CONTROLLER_GO_DEVICE_LOST;
    }
    break;
    case EVENT_GROUP_REMOVED:
    {
        state = CONTROLLER_GO_GROUP_REMOVED;
    }
    break;
    case EVENT_ERROR:
    {
        state = CONTROLLER_GO_EVENT_ERROR;
    }
    break;
    default:
    {
        state = CONTROLLER_GO_UNKNOWN_EVENT;
    }
    break;
    }
    return state;
}

void MiracastController::restart_session(bool start_discovering_enabled)
{
    MIRACASTLOG_TRACE("Entering...");

    reset_WFDSourceMACAddress();
    reset_WFDSourceName();
    stop_session();
    if (start_discovering_enabled){
        discover_devices();
    }
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastController::stop_session(bool stop_streaming_needed)
{
    MIRACASTLOG_TRACE("Entering...");
    stop_discover_devices();
    remove_P2PGroupInstance();
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastController::remove_P2PGroupInstance(void)
{
    MIRACASTLOG_TRACE("Entering...");
    if (m_groupInfo)
    {
        std::string system_cmd_buffer = "";

        if (( true == m_groupInfo->isGO )&&(nullptr != m_p2p_ctrl_obj))
        {
            m_p2p_ctrl_obj->remove_GroupInterface( m_groupInfo->interface );
        }
        if ( true == m_groupInfo->isGO )
        {
            system_cmd_buffer = "ps -ax | awk '/dnsmasq -p0 -i/ && !/grep/ {print $1}' | xargs kill -9";
            MIRACASTLOG_INFO("Terminate old dnsmasq instance: [%s]",system_cmd_buffer.c_str());
        }
        else
        {
            system_cmd_buffer = "ps -ax | awk '/p2p_udhcpc/ && !/grep/ {print $1}' | xargs kill -9";
            MIRACASTLOG_INFO("Terminate old udhcpc p2p instance : [%s]", system_cmd_buffer.c_str());
        }
        MiracastCommon::execute_SystemCommand(system_cmd_buffer.c_str());
        delete m_groupInfo;
        m_groupInfo = nullptr;
    }
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastController::checkAndInitiateP2PBackendDiscovery(void)
{
    if ( m_p2p_backend_discovery )
    {
        MIRACASTLOG_INFO("!!! BACKEND P2P DISCOVERY HAS BEEN STARTED !!!");
        /* Enabled the Device Discovery to allow other device to cast */
        discover_devices();
    }
    else
    {
        MIRACASTLOG_INFO("!!! BACKEND P2P DISCOVERY HAS DISABLED !!!");
        stop_discover_devices();
    }
}

void MiracastController::event_handler(P2P_EVENTS eventId, void *data, size_t len )
{
    CONTROLLER_MSGQ_STRUCT controller_msgq_data = {0};
    std::string event_buffer;
    MIRACASTLOG_TRACE("Entering...");

    event_buffer = (char *)data;
    free(data);

    if ( false == m_start_discovering_enabled )
    {
        MIRACASTLOG_INFO("Miracast discovery not enabled. So no need to notify to MiracastController");
        MIRACASTLOG_TRACE("Exiting...");
        return;
    }

    if (nullptr != m_controller_thread){
        controller_msgq_data.msg_type = P2P_MSG;
        controller_msgq_data.state = convertP2PtoSessionActions(eventId);
        strcpy(controller_msgq_data.msg_buffer, event_buffer.c_str());

        MIRACASTLOG_INFO("event_handler to Controller Action[%#08X] buffer:%s  ", controller_msgq_data.state, event_buffer.c_str());
        m_controller_thread->send_message(&controller_msgq_data, sizeof(controller_msgq_data));
        MIRACASTLOG_VERBOSE("event received : %d buffer:%s  ", eventId, event_buffer.c_str());
    }
    MIRACASTLOG_TRACE("Exiting...");
}

MiracastError MiracastController::set_WFDParameters(void)
{
    MIRACASTLOG_TRACE("Entering...");
    MiracastError ret = MIRACAST_FAIL;
    if (nullptr != m_p2p_ctrl_obj)
    {
        std::string ifName = getifNameByIPv4("192.168.59.1");
        m_p2p_ctrl_obj->remove_GroupInterface(ifName);
        ret = m_p2p_ctrl_obj->set_WFDParameters();
    }
    MIRACASTLOG_TRACE("Exiting...");
    return ret;
}

MiracastError MiracastController::discover_devices(void)
{
    MIRACASTLOG_TRACE("Entering...");
    MiracastError ret = MIRACAST_FAIL;
    if (nullptr != m_p2p_ctrl_obj){
        ret = m_p2p_ctrl_obj->discover_devices();
    }
    MIRACASTLOG_TRACE("Exiting...");
    return ret;
}

MiracastError MiracastController::stop_discover_devices(void)
{
    MIRACASTLOG_TRACE("Entering...");
    MiracastError ret = MIRACAST_FAIL;
    if (nullptr != m_p2p_ctrl_obj){
        ret = m_p2p_ctrl_obj->stop_discover_devices();
    }
    MIRACASTLOG_TRACE("Exiting...");
    return ret;
}

MiracastError MiracastController::connect_device(std::string device_mac , std::string device_name )
{
    MIRACASTLOG_TRACE("Entering...");
    MIRACASTLOG_INFO("Connecting to the MAC - %s", device_mac.c_str());
    MiracastError ret = MIRACAST_FAIL;
    DeviceInfo *device_info_ptr = MiracastController::get_device_details(device_mac);

    if ((nullptr != m_p2p_ctrl_obj) && ( nullptr != device_info_ptr ))
    {
        ret = m_p2p_ctrl_obj->connect_device(device_mac,device_info_ptr->authType);
        if (MIRACAST_OK == ret )
        {
            set_WFDSourceMACAddress(device_mac);
            set_WFDSourceName(device_name);
            MIRACASTLOG_INFO("!!! P2P CONNECT Success for Device[%s - %s] !!!",device_name.c_str(),device_mac.c_str());
        }
        else
        {
            MIRACASTLOG_ERROR("#### MCAST-TRIAGE-NOK P2P CONNECT FAILURE FOR DEVICE[%s - %s] ####",device_name.c_str(),device_mac.c_str());
            if ( nullptr != m_notify_handler )
            {
                m_notify_handler->onMiracastServiceClientConnectionError( device_mac , device_name , MIRACAST_SERVICE_ERR_CODE_P2P_CONNECT_ERROR );
            }
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
    return ret;
}

std::string MiracastController::get_localIp()
{
    MIRACASTLOG_TRACE("Entering...");
    std::string ip_addr = "";
    if (nullptr != m_groupInfo)
    {
        ip_addr = m_groupInfo->localIPAddr;
    }
    MIRACASTLOG_TRACE("Exiting...");
    return ip_addr;
}

std::string MiracastController::get_connected_device_mac()
{
    std::string mac_address = "";
    mac_address = get_WFDSourceMACAddress();
    return mac_address;
}

std::vector<DeviceInfo *> MiracastController::get_allPeers()
{
    return m_deviceInfoList;
}

bool MiracastController::get_connection_status()
{
    return m_connectionStatus;
}

DeviceInfo *MiracastController::get_device_details(std::string MAC)
{
    DeviceInfo *deviceInfo = nullptr;
    std::size_t found;
    MIRACASTLOG_TRACE("Entering...");
    for (auto device : m_deviceInfoList)
    {
        found = device->deviceMAC.find(MAC);
        if (found != std::string::npos)
        {
            deviceInfo = device;
            break;
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
    return deviceInfo;
}

std::string MiracastController::get_device_name(std::string mac_address)
{
    MIRACASTLOG_TRACE("Entering...");
    size_t found;
    std::string device_name = "";
    int i = 0;
    for (auto devices : m_deviceInfoList)
    {
        found = devices->deviceMAC.find(mac_address);
        if (found != std::string::npos)
        {
            device_name = devices->modelName;
            break;
        }
        i++;
    }
    MIRACASTLOG_TRACE("Exiting...");
    return device_name;
}

MiracastError MiracastController::set_FriendlyName(std::string friendly_name , bool apply )
{
    MiracastError ret = MIRACAST_OK;
    MIRACASTLOG_TRACE("Entering..");
    if (nullptr != m_p2p_ctrl_obj){
        ret = m_p2p_ctrl_obj->set_FriendlyName(friendly_name, apply );
    }
    MIRACASTLOG_TRACE("Exiting..");
    return ret;
}

std::string MiracastController::get_FriendlyName(void)
{
    std::string friendly_name = "";
    MIRACASTLOG_TRACE("Entering and Exiting...");

    if (nullptr != m_p2p_ctrl_obj){
        friendly_name = m_p2p_ctrl_obj->get_FriendlyName();
    }
    return friendly_name;
}

void MiracastController::Controller_Thread(void *args)
{
    CONTROLLER_MSGQ_STRUCT controller_msgq_data = {0};
    bool    new_thunder_req_client_connection_sent = false,
            another_thunder_req_client_connection_sent = false,
            session_restart_required = false,
            p2p_group_instance_alive = false;

    MIRACASTLOG_TRACE("Entering...");

    while (nullptr != m_controller_thread)
    {
        std::string event_buffer;
        event_buffer.clear();

        MIRACASTLOG_TRACE("!!! Waiting for Event !!!\n");
        m_controller_thread->receive_message(&controller_msgq_data, CONTROLLER_MSGQ_SIZE, THREAD_RECV_MSG_INDEFINITE_WAIT);

        event_buffer = controller_msgq_data.msg_buffer;

        MIRACASTLOG_TRACE("!!! Received Action[%#08X]Data[%s] !!!\n", controller_msgq_data.state, event_buffer.c_str());

        if (CONTROLLER_SELF_ABORT == controller_msgq_data.state)
        {
            MIRACASTLOG_INFO("CONTROLLER_SELF_ABORT Received.\n");
            break;
        }

        switch (controller_msgq_data.msg_type)
        {
            case P2P_MSG:
            {
                MIRACASTLOG_TRACE("P2P_MSG type received");

                switch (controller_msgq_data.state)
                {
                    case CONTROLLER_GO_DEVICE_FOUND:
                    case CONTROLLER_GO_DEVICE_PROVISION:
                    case CONTROLLER_GO_DEVICE_LOST:
                    {
                        string deviceMAC;
                        string deviceType;
                        string modelName;
                        std::string wfdSubElements;

                        deviceMAC = parse_p2p_event_data(event_buffer.c_str(), "p2p_dev_addr");

                        if ( CONTROLLER_GO_DEVICE_LOST == controller_msgq_data.state )
                        {
                            size_t found;
                            int i = 0;

                            MIRACASTLOG_INFO("CONTROLLER_GO_DEVICE_LOST Received");
                            for (auto devices : m_deviceInfoList)
                            {
                                found = devices->deviceMAC.find(deviceMAC);
                                if (found != std::string::npos)
                                {
                                    delete devices;
                                    m_deviceInfoList.erase(m_deviceInfoList.begin() + i);
                                    break;
                                }
                                i++;
                            }
                        }
                        else
                        {
                            std::string authType = "pbc";
                            bool new_device_entry = false;
                            deviceType = parse_p2p_event_data(event_buffer.c_str(), "pri_dev_type");
                            modelName = parse_p2p_event_data(event_buffer.c_str(), "name");

                            if ( CONTROLLER_GO_DEVICE_FOUND == controller_msgq_data.state )
                            {
                                MIRACASTLOG_INFO("CONTROLLER_GO_DEVICE_FOUND Received");
                            }
                            else
                            {
                                MIRACASTLOG_INFO("CONTROLLER_GO_DEVICE_PROVISION Received");
                                if (std::string::npos != event_buffer.find("P2P-PROV-DISC-SHOW-PIN"))
                                {
                                    std::istringstream iss(event_buffer);
                                    std::string token;

                                    // Ignore the first two tokens (P2P-PROV-DISC-SHOW-PIN and the MAC address)
                                    iss >> token;

                                    iss >> token;

                                    // Get the third token which is PIN
                                    iss >> token;
                                    MIRACASTLOG_INFO("!!!! P2P-PROV-DISC-SHOW-PIN is [%s] !!!!",token.c_str());
                                    authType = token;
                                }
                            }

                            DeviceInfo *cur_device_info_ptr = MiracastController::get_device_details(deviceMAC);

                            if ( nullptr == cur_device_info_ptr )
                            {
                                cur_device_info_ptr = new DeviceInfo;
                                new_device_entry = true;
                            }

                            if ( nullptr != cur_device_info_ptr )
                            {
                                cur_device_info_ptr->deviceMAC = deviceMAC;
                                cur_device_info_ptr->authType = authType;
                                cur_device_info_ptr->modelName = modelName;
                                cur_device_info_ptr->deviceType = deviceType;
                            }

                            if ( new_device_entry && cur_device_info_ptr )
                            {
                                m_deviceInfoList.push_back(cur_device_info_ptr);
                            }
                            wfdSubElements = parse_p2p_event_data(event_buffer.c_str(), "wfd_dev_info");
                            #if 0
                                device->isCPSupported = ((strtol(wfdSubElements.c_str(), nullptr, 16) >> 32) && 256);
                                device->deviceRole = (DEVICEROLE)((strtol(wfdSubElements.c_str(), nullptr, 16) >> 32) && 3);
                            #endif
                            MIRACASTLOG_TRACE("Device data parsed & stored successfully");
                        }
                    }
                    break;
                    case CONTROLLER_GO_NEG_REQUEST:
                    {
                        MIRACASTLOG_INFO("CONTROLLER_GO_NEG_REQUEST Received\n");
                        std::string received_mac_address;
                        size_t space_find = event_buffer.find(" ");
                        size_t dev_str = event_buffer.find("dev_passwd_id");
                        if ((space_find != std::string::npos) && (dev_str != std::string::npos))
                        {
                            received_mac_address = event_buffer.substr(space_find, dev_str - space_find);
                            REMOVE_SPACES(received_mac_address);
                        }

                        std::string device_name = get_device_name(received_mac_address);

                        if ( get_WFDSourceMACAddress().empty())
                        {
                            if ( false == new_thunder_req_client_connection_sent )
                            {
                                new_thunder_req_client_connection_sent = true;
                                notify_ConnectionRequest(device_name,received_mac_address);
                                MIRACASTLOG_INFO("!!! Connection Request reported waiting for user action !!!\n");
                            }
                            else
                            {
                                MIRACASTLOG_WARNING("!!! Another connect request has Received while new connection inprogress !!!\n");
                            }
                        }
                        else
                        {
                            if (0 == (received_mac_address.compare(get_WFDSourceMACAddress())))
                            {
                                MIRACASTLOG_WARNING("Duplicate Connect Request has Received\n");
                            }
                            else
                            {
                                if ( false == another_thunder_req_client_connection_sent )
                                {
                                    notify_ConnectionRequest(device_name,received_mac_address);
                                    another_thunder_req_client_connection_sent = true;
                                    MIRACASTLOG_INFO("!!! New Connection Request reported waiting for user action !!!\n");
                                }
                                else
                                {
                                    //  Need to handle connect request received evenafter connection already established with other client
                                    MIRACASTLOG_ERROR("!!! 3rd connect request has Received while existing is inprogress MAC[%s][%s] !!!\n",
                                                        received_mac_address.c_str(),
                                                        device_name.c_str());
                                }
                            }
                        }
                    }
                    break;
                    case CONTROLLER_GO_GROUP_STARTED:
                    case CONTROLLER_GO_NEG_FAILURE:
                    case CONTROLLER_GO_GROUP_FORMATION_FAILURE:
                    {
                        eMIRACAST_SERVICE_ERR_CODE error_code = MIRACAST_SERVICE_ERR_CODE_GENERIC_FAILURE;

                        if ( CONTROLLER_GO_GROUP_STARTED == controller_msgq_data.state )
                        {
                            std::string remote_address = "",
                                        local_address  = "";
                            std::string src_dev_ip = "",
                                        src_dev_mac = "",
                                        src_dev_name = "",
                                        sink_dev_ip = "";
                            m_groupInfo = new GroupInfo;
                            size_t found = event_buffer.find("client");
                            size_t found_space = event_buffer.find(" ");

                            MIRACASTLOG_TRACE("CONTROLLER_GO_GROUP_STARTED Received\n");

                            if (found != std::string::npos)
                            {
                                MIRACASTLOG_INFO("!!!! P2P GROUP STARTED IN CLIENT MODE !!!!");
                                m_groupInfo->ipAddr = parse_p2p_event_data(event_buffer.c_str(), "ip_addr");
                                m_groupInfo->ipMask = parse_p2p_event_data(event_buffer.c_str(), "ip_mask");
                                m_groupInfo->goIPAddr = parse_p2p_event_data(event_buffer.c_str(), "go_ip_addr");
                                m_groupInfo->goDevAddr = parse_p2p_event_data(event_buffer.c_str(), "go_dev_addr");
                                m_groupInfo->SSID = parse_p2p_event_data(event_buffer.c_str(), "ssid");

                                size_t found_client = event_buffer.find("client");
                                m_groupInfo->interface = event_buffer.substr(found_space, found_client - found_space);
                                REMOVE_SPACES(m_groupInfo->interface);

                                if (getenv("GET_PACKET_DUMP") != nullptr)
                                {
                                    std::string tcpdump;
                                    tcpdump.append("tcpdump -i ");
                                    tcpdump.append(m_groupInfo->interface);
                                    tcpdump.append(" -s 65535 -w /opt/p2p_cli_dump.pcap &");
                                    MIRACASTLOG_VERBOSE("Dump command to execute - %s", tcpdump.c_str());
                                    MiracastCommon::execute_SystemCommand(tcpdump.c_str());
                                }

                                std::string default_gw_ip = "";

                                // STB is a client in the p2p group
                                m_groupInfo->isGO = false;
                                m_groupInfo->localIPAddr = start_DHCPClient(m_groupInfo->interface, default_gw_ip);
                                if (m_groupInfo->localIPAddr.empty())
                                {
                                    MIRACASTLOG_ERROR("Local IP address is not obtained");
                                }
                                else
                                {
                                    if (m_groupInfo->goIPAddr.empty())
                                    {
                                        MIRACASTLOG_INFO("Could be Persistent Group checking default_gw_ip [%s]\n", default_gw_ip.c_str());
                                        m_groupInfo->goIPAddr.append(default_gw_ip);
                                    }
                                    remote_address = m_groupInfo->goIPAddr;
                                    local_address = m_groupInfo->localIPAddr;
                                }
                            }
                            else
                            {
                                MIRACASTLOG_INFO("!!!! P2P GROUP STARTED IN GO MODE !!!!");
                                size_t found_go = event_buffer.find("GO");
                                m_groupInfo->interface = event_buffer.substr(found_space, found_go - found_space);
                                m_groupInfo->goDevAddr = parse_p2p_event_data(event_buffer.c_str(), "go_dev_addr");
                                REMOVE_SPACES(m_groupInfo->interface);

                                if (getenv("GET_PACKET_DUMP") != nullptr)
                                {
                                    std::string tcpdump;
                                    tcpdump.append("tcpdump -i ");
                                    tcpdump.append(m_groupInfo->interface);
                                    tcpdump.append(" -s 65535 -w /opt/p2p_go_dump.pcap &");
                                    MIRACASTLOG_VERBOSE("Dump command to execute - %s", tcpdump.c_str());
                                    MiracastCommon::execute_SystemCommand(tcpdump.c_str());
                                }

                                local_address = start_DHCPServer( m_groupInfo->interface );
                                m_groupInfo->isGO = true;

                                std::string mac_address = get_WFDSourceMACAddress();
                                char data[1024] = {0};
                                char command[128] = {0};
                                std::string popen_buffer = "";
                                FILE *popen_file_ptr = nullptr;
                                char *current_line_buffer = nullptr;
                                std::size_t len = 0;
                                unsigned char retry_count = 15;

                                sprintf( command, "awk '$6 == \"%s\" && $4 !~ /incomplete/ {print $1}' /proc/net/arp", m_groupInfo->interface.c_str());
                                while ( retry_count-- )
                                {
                                    MIRACASTLOG_INFO("command is [%s]\n", command);
                                    popen_file_ptr = popen(command, "r");
                                    if (!popen_file_ptr)
                                    {
                                        MIRACASTLOG_ERROR("Could not open pipe for output.");
                                    }
                                    else
                                    {
                                        memset( data , 0x00 , sizeof(data));
                                        while (getline(&current_line_buffer, &len, popen_file_ptr) != -1)
                                        {
                                            sprintf(data + strlen(data), "%s" ,  current_line_buffer);
                                            MIRACASTLOG_INFO("data : [%s]", data);
                                        }
                                        pclose(popen_file_ptr);
                                        popen_file_ptr = nullptr;

                                        popen_buffer = data;
                                        REMOVE_R(popen_buffer);
                                        REMOVE_N(popen_buffer);

                                        MIRACASTLOG_INFO("popen_buffer is [%s]\n", popen_buffer.c_str());

                                        free(current_line_buffer);
                                        current_line_buffer = nullptr;

                                        if (!popen_buffer.empty()){
                                            MIRACASTLOG_INFO("%s is success and popen_buffer[%s]\n", command,popen_buffer.c_str());
                                            remote_address = popen_buffer;
                                            sleep(1);
                                            memset( command , 0x00 , sizeof(command));
                                            sprintf( command , "arping %s -I %s -c 1" , remote_address.c_str(), m_groupInfo->interface.c_str());
                                            MIRACASTLOG_INFO("### arping is [%s] ###", command);
                                            MiracastCommon::execute_SystemCommand(command);
                                            MIRACASTLOG_INFO("### arping is [%s] done ###", command);
                                            break;
                                        }
                                    }
                                    sleep(1);
                                }
                            }
                            if (!remote_address.empty())
                            {
                                src_dev_ip = remote_address;
                                sink_dev_ip = local_address;
                                src_dev_mac = get_WFDSourceMACAddress();;
                                src_dev_name = get_WFDSourceName();

                                if (!m_connect_req_notified && src_dev_mac.empty() && src_dev_name.empty())
                                {
                                    src_dev_mac = m_groupInfo->goDevAddr;
                                    src_dev_name = get_device_name(src_dev_mac);
                                    set_WFDSourceMACAddress(src_dev_mac);
                                    set_WFDSourceName(src_dev_name);
                                }
                                MIRACASTLOG_INFO("#### MCAST-TRIAGE-OK-LAUNCH LAUNCH REQ FOR SRC_NAME[%s] SRC_MAC[%s] SRC_IP[%s] SINK_IP[%s] ConnectReq[%u]####",
                                                    src_dev_name.c_str(),
                                                    src_dev_mac.c_str(),
                                                    src_dev_ip.c_str(),
                                                    sink_dev_ip.c_str(),
                                                    m_connect_req_notified);
                                if (nullptr != m_notify_handler)
                                {
                                    m_notify_handler->onMiracastServiceLaunchRequest(src_dev_ip, src_dev_mac, src_dev_name, sink_dev_ip, m_connect_req_notified );
                                }
                                checkAndInitiateP2PBackendDiscovery();
                                session_restart_required = false;
                                p2p_group_instance_alive = true;
                                m_connect_req_notified = false;
                            }
                            else
                            {
                                error_code = MIRACAST_SERVICE_ERR_CODE_GENERIC_FAILURE;
                                session_restart_required = true;
                                MIRACASTLOG_ERROR("!!!! Unable to get the Source Device IP and Terminating Group Here !!!!");
                                remove_P2PGroupInstance();
                            }
                        }
                        else
                        {
                            if ( CONTROLLER_GO_GROUP_FORMATION_FAILURE == controller_msgq_data.state )
                            {
                                error_code = MIRACAST_SERVICE_ERR_CODE_P2P_GROUP_FORMATION_ERROR;
                                MIRACASTLOG_ERROR("#### MCAST-TRIAGE-NOK CONTROLLER_GO_GROUP_FORMATION_FAILURE ####");
                            }
                            else if ( CONTROLLER_GO_NEG_FAILURE == controller_msgq_data.state )
                            {
                                error_code = MIRACAST_SERVICE_ERR_CODE_P2P_GROUP_NEGO_ERROR;
                                MIRACASTLOG_ERROR("#### MCAST-TRIAGE-NOK CONTROLLER_GO_NEG_FAILURE ####");
                            }

                            if ( p2p_group_instance_alive )
                            {
                                MIRACASTLOG_ERROR("#### MCAST-TRIAGE-NOK [GROUP_FORMATION/NEG_FAILURE - %#08X] AFTER P2P GROUP STARTED ####",
                                                    controller_msgq_data.state );
                                p2p_group_instance_alive = false;
                            }
                            m_connect_req_notified = false;
                        }

                        if ( true == session_restart_required )
                        {
                            if (nullptr != m_notify_handler)
                            {
                                std::string mac_address = get_WFDSourceMACAddress();
                                std::string device_name = get_WFDSourceName();
                                m_notify_handler->onMiracastServiceClientConnectionError( mac_address , device_name , error_code );
                            }
                            MIRACASTLOG_INFO("!!! Restarting Session !!!");
                            restart_session(m_start_discovering_enabled);
                            session_restart_required = false;
                        }
                    }
                    break;
                    case CONTROLLER_GO_GROUP_REMOVED:
                    {
                        MIRACASTLOG_INFO("CONTROLLER_GO_GROUP_REMOVED Received\n");
                        if ( p2p_group_instance_alive )
                        {
                            std::string device_name = get_NewSourceName(),
                                        mac_address = get_NewSourceMACAddress();

                            if ( ! mac_address.empty())
                            {
                                MIRACASTLOG_INFO("!!!! Cached Connect Request found[%s][%s] and trying to connect it after 5sec !!!!",
                                                    device_name.c_str(),
                                                    mac_address.c_str());
                                sleep(5);
                                connect_device(mac_address,device_name);
                                reset_NewSourceName();
                                reset_NewSourceMACAddress();
                            }
                            else
                            {
                                MIRACASTLOG_INFO("!!!! Cached Connect Request not found !!!!");
                            }
                        }
                        new_thunder_req_client_connection_sent = false;
                        another_thunder_req_client_connection_sent = false;
                        session_restart_required = true;
                        p2p_group_instance_alive = false;
                    }
                    break;
                    case CONTROLLER_GO_STOP_FIND:
                    {
                        MIRACASTLOG_TRACE("[CONTROLLER_GO_STOP_FIND] Received");
                    }
                    break;
                    case CONTROLLER_GO_NEG_SUCCESS:
                    {
                        MIRACASTLOG_INFO("[CONTROLLER_GO_NEG_SUCCESS] Received");
                    }
                    break;
                    case CONTROLLER_GO_GROUP_FORMATION_SUCCESS:
                    {
                        MIRACASTLOG_INFO("[CONTROLLER_GO_GROUP_FORMATION_SUCCESS] Received");
                    }
                    break;
                    case CONTROLLER_GO_EVENT_ERROR:
                    case CONTROLLER_GO_UNKNOWN_EVENT:
                    {
                        MIRACASTLOG_ERROR("[GO_EVENT_ERROR/GO_UNKNOWN_EVENT] Received");
                    }
                    break;
                    default:
                    {
                        MIRACASTLOG_ERROR("!!! Invalid state Received[%#08X]Data[%s] with P2P_MSG !!!\n", controller_msgq_data.state, event_buffer.c_str());
                    }
                    break;
                }
            }
            break;
            case CONTRLR_FW_MSG:
            {
                MIRACASTLOG_TRACE("CONTRLR_FW_MSG type received");
                switch (controller_msgq_data.state)
                {
                // Start and Stop Discovering initiated directly from Thunder request. So this is unused and commenting out here
                #if 0
                    case CONTROLLER_START_DISCOVERING:
                    {
                        MIRACASTLOG_INFO("CONTROLLER_START_DISCOVERING Received\n");
                        set_WFDParameters();
                        discover_devices();
                        m_start_discovering_enabled = true;
                    }
                    break;
                    case CONTROLLER_STOP_DISCOVERING:
                    {
                        MIRACASTLOG_INFO("CONTROLLER_STOP_DISCOVERING Received\n");
                        stop_session(true);
                        m_start_discovering_enabled = false;
                    }
                    break;
                #endif
                    case CONTROLLER_RESTART_DISCOVERING:
                    {
                        std::string cached_mac_address = get_NewSourceMACAddress(),
                                    mac_address = controller_msgq_data.source_dev_mac;
                        MIRACASTLOG_INFO("CONTROLLER_RESTART_DISCOVERING Received\n");
                        m_connectionStatus = false;

                        if ((!cached_mac_address.empty()) && ( 0 == mac_address.compare(cached_mac_address)))
                        {
                            reset_NewSourceMACAddress();
                            reset_NewSourceName();
                            MIRACASTLOG_INFO("[%s] Cached Device info removed...",cached_mac_address.c_str());
                        }
                        restart_session(m_start_discovering_enabled);
                        new_thunder_req_client_connection_sent = false;
                        another_thunder_req_client_connection_sent = false;
                        session_restart_required = true;
                        p2p_group_instance_alive = false;
                        m_connect_req_notified = false;
                    }
                    break;
                    case CONTROLLER_CONNECT_REQ_FROM_THUNDER:
                    {
                        MIRACASTLOG_INFO("CONTROLLER_CONNECT_REQ_FROM_THUNDER Received");
                        std::string mac_address = controller_msgq_data.source_dev_mac;
                        std::string device_name = get_device_name(mac_address);

                        if ( false == p2p_group_instance_alive )
                        {
                            connect_device(mac_address,device_name);
                            new_thunder_req_client_connection_sent = false;
                            another_thunder_req_client_connection_sent = false;
                            session_restart_required = true;
                        }
                        else
                        {
                            if ( get_NewSourceMACAddress().empty())
                            {
                                MIRACASTLOG_INFO("!!! Caching New Connection until P2P Group remove properly !!!");
                                set_NewSourceMACAddress(mac_address);
                                set_NewSourceName(device_name);
                            }
                            else
                            {
                                MIRACASTLOG_ERROR("!!! Unable to Cache Connection[%s - %s] as [%s - %s] was already cached !!!",
                                                    device_name,
                                                    mac_address,
                                                    get_NewSourceName(),
                                                    get_NewSourceMACAddress());
                            }
                        }
                    }
                    break;
                    case CONTROLLER_FLUSH_CURRENT_SESSION:
                    {
                        MIRACASTLOG_INFO("CONTROLLER_FLUSH_CURRENT_SESSION Received\n");
                        remove_P2PGroupInstance();
                    }
                    break;
                    case CONTROLLER_CONNECT_REQ_REJECT:
                    case CONTROLLER_CONNECT_REQ_TIMEOUT:
                    {
                        if ( CONTROLLER_CONNECT_REQ_REJECT == controller_msgq_data.state )
                        {
                            MIRACASTLOG_INFO("CONTROLLER_CONNECT_REQ_REJECT Received\n");
                        }
                        else
                        {
                            MIRACASTLOG_INFO("CONTROLLER_CONNECT_REQ_TIMEOUT Received\n");
                        }
                        new_thunder_req_client_connection_sent = false;
                        another_thunder_req_client_connection_sent = false;
                    }
                    break;
                    case CONTROLLER_SWITCH_LAUNCH_REQ_CTX:
                    {
                        MIRACASTLOG_INFO("Launch Request to be notified from here");
                        if (nullptr != m_notify_handler)
                        {
                            MIRACASTLOG_INFO("#### MCAST-TRIAGE-OK-LAUNCH LAUNCH REQ FOR SRC_NAME[%s] SRC_MAC[%s] SRC_IP[%s] SINK_IP[%s] ####",
                                                    controller_msgq_data.source_dev_name,
                                                    controller_msgq_data.source_dev_mac,
                                                    controller_msgq_data.source_dev_ip,
                                                    controller_msgq_data.sink_dev_ip);
                            m_notify_handler->onMiracastServiceLaunchRequest( controller_msgq_data.source_dev_ip,
                                                                              controller_msgq_data.source_dev_mac,
                                                                              controller_msgq_data.sink_dev_ip,
                                                                              controller_msgq_data.source_dev_name,
                                                                              true );
                        }
                    }
                    break;
                    default:
                    {
                        MIRACASTLOG_ERROR("!!! Invalid state Received[%#08X]Data[%s] with CONTRLR_FW_MSG !!!", controller_msgq_data.state, event_buffer.c_str());
                    }
                    break;
                }
            }
            break;
            default:
            {
                MIRACASTLOG_ERROR("!!! Invalid MsgType Received[%#08X]Data[%s]  !!!", controller_msgq_data.msg_type, event_buffer.c_str());
            }
            break;
        }        
    }
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastController::send_thundermsg_to_controller_thread(CONTROLLER_MSGQ_STRUCT controller_msgq_data)
{
    MIRACASTLOG_TRACE("Entering...");
    if (nullptr != m_controller_thread)
    {
        MIRACASTLOG_INFO("Msg to Controller Action[%#08X]", controller_msgq_data.state);
        controller_msgq_data.msg_type = CONTRLR_FW_MSG;
        m_controller_thread->send_message(&controller_msgq_data, CONTROLLER_MSGQ_SIZE);
    }
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastController::set_enable(bool is_enabled)
{
    MIRACASTLOG_TRACE("Entering...");

    if ( true == is_enabled)
    {
        MIRACASTLOG_INFO("MIRACAST_SERVICE_WFD_START Received");
        set_WFDParameters();
        discover_devices();
        m_start_discovering_enabled = true;
    }
    else
    {
        MIRACASTLOG_INFO("MIRACAST_SERVICE_WFD_STOP Received");
        stop_session(true);
        m_start_discovering_enabled = false;
    }
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastController::setP2PBackendDiscovery(bool is_enabled)
{
    MIRACASTLOG_TRACE("Entering [%x]...",is_enabled);
    m_p2p_backend_discovery = is_enabled;
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastController::restart_session_discovery(std::string& mac_address)
{
    CONTROLLER_MSGQ_STRUCT controller_msgq_data = {0};
    MIRACASTLOG_TRACE("Entering...");
    MIRACASTLOG_INFO("[MIRACAST_SERVICE_WFD_RESTART][%s]",mac_address.c_str());
    if ( !mac_address.empty())
    {
        strncpy(controller_msgq_data.source_dev_mac, mac_address.c_str(),sizeof(controller_msgq_data.source_dev_mac));
        controller_msgq_data.state = CONTROLLER_RESTART_DISCOVERING;
        send_thundermsg_to_controller_thread(controller_msgq_data);
    }
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastController::flush_current_session(void )
{
    CONTROLLER_MSGQ_STRUCT controller_msgq_data = {0};
    MIRACASTLOG_TRACE("Entering...");
    MIRACASTLOG_INFO("[MIRACAST_SERVICE_FLUSH_SESSION]");
    controller_msgq_data.state = CONTROLLER_FLUSH_CURRENT_SESSION;
    send_thundermsg_to_controller_thread(controller_msgq_data);
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastController::accept_client_connection(std::string is_accepted)
{
    CONTROLLER_MSGQ_STRUCT controller_msgq_data = {0};

    MIRACASTLOG_TRACE("Entering...");
    if ("Accept" == is_accepted)
    {
        MIRACASTLOG_INFO("[MIRACAST_SERVICE_ACCEPT_CLIENT]");
        strncpy(controller_msgq_data.source_dev_mac, m_current_device_mac_addr.c_str(),sizeof(controller_msgq_data.source_dev_mac));
        controller_msgq_data.state = CONTROLLER_CONNECT_REQ_FROM_THUNDER;
    }
    else
    {
        MIRACASTLOG_INFO("[MIRACAST_SERVICE_REJECT_CLIENT]");
        controller_msgq_data.state = CONTROLLER_CONNECT_REQ_REJECT;
    }
    MIRACASTLOG_INFO("!!!! DEVICE[%s - %s] Connection Request has '%s' !!!!",
                        m_current_device_name.c_str(),
                        m_current_device_mac_addr.c_str(),
                        is_accepted.c_str());
    m_current_device_name.clear();
    m_current_device_mac_addr.clear();
    send_thundermsg_to_controller_thread(controller_msgq_data);
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastController::switch_launch_request_context(std::string& source_dev_ip,std::string& source_dev_mac,std::string& sink_dev_ip,std::string& source_dev_name)
{
    CONTROLLER_MSGQ_STRUCT controller_msgq_data = {0};
    MIRACASTLOG_TRACE("Entering...");
    if (( !source_dev_ip.empty()) && ( !source_dev_mac.empty()) && ( !sink_dev_ip.empty()) && ( !source_dev_name.empty()))
    {
        MIRACASTLOG_INFO("[MIRACAST_SERVICE_WFD_SWITCH_CTX_FOR_LAUNCH_REQ]source_dev_ip[%s]source_dev_mac[%s]sink_dev_ip[%s]source_dev_name[%s]",
                            source_dev_ip.c_str(),
                            source_dev_mac.c_str(),
                            sink_dev_ip.c_str(),
                            source_dev_name.c_str());
        strncpy(controller_msgq_data.source_dev_ip, source_dev_ip.c_str(),sizeof(controller_msgq_data.source_dev_ip));
        strncpy(controller_msgq_data.source_dev_mac, source_dev_mac.c_str(),sizeof(controller_msgq_data.source_dev_mac));
        strncpy(controller_msgq_data.sink_dev_ip, sink_dev_ip.c_str(),sizeof(controller_msgq_data.sink_dev_ip));
        strncpy(controller_msgq_data.source_dev_name, source_dev_name.c_str(),sizeof(controller_msgq_data.source_dev_name));
        controller_msgq_data.state = CONTROLLER_SWITCH_LAUNCH_REQ_CTX;
        send_thundermsg_to_controller_thread(controller_msgq_data);
    }
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastController::set_WFDSourceMACAddress(std::string MAC_Addr)
{
    m_connected_mac_addr = MAC_Addr;
}

void MiracastController::set_WFDSourceName(std::string device_name)
{
    m_connected_device_name = device_name;
}

std::string MiracastController::get_WFDSourceName(void)
{
    return m_connected_device_name;
}

std::string MiracastController::get_WFDSourceMACAddress(void)
{
    return m_connected_mac_addr;
}

void MiracastController::reset_WFDSourceMACAddress(void)
{
    m_connected_mac_addr.clear();
}

void MiracastController::reset_WFDSourceName(void)
{
    m_connected_device_name.clear();
}

void MiracastController::set_NewSourceMACAddress(std::string mac_address)
{
    m_new_device_mac_addr = mac_address;
}

void MiracastController::set_NewSourceName(std::string device_name)
{
    m_new_device_name = device_name;
}

std::string MiracastController::get_NewSourceName(void)
{
    return m_new_device_name;
}

std::string MiracastController::get_NewSourceMACAddress(void)
{
    return m_new_device_mac_addr;
}

void MiracastController::reset_NewSourceMACAddress(void)
{
    m_new_device_mac_addr.clear();
}

void MiracastController::reset_NewSourceName(void)
{
    m_new_device_name.clear();
}

void MiracastController::notify_ConnectionRequest(std::string device_name,std::string device_mac)
{
    MIRACASTLOG_TRACE("Entering...");
    m_current_device_name = device_name;
    m_current_device_mac_addr = device_mac;

    MIRACASTLOG_INFO("#### MCAST-TRIAGE-OK-CONNECT-REQ DEVICE[%s - %s] CONNECT REQUEST RECEIVED ####",
                        m_current_device_name.c_str(), 
                        m_current_device_mac_addr.c_str());
    m_connect_req_notified = true;
    if (0 == access("/opt/miracast_direct_request", F_OK))
    {
        m_connect_req_notified = false;
    }

    if (nullptr != m_notify_handler)
    {
        m_notify_handler->onMiracastServiceClientConnectionRequest(device_mac, device_name);
    }
    MIRACASTLOG_TRACE("Exiting...");
}

void ControllerThreadCallback(void *args)
{
    MiracastController *miracast_ctrler_obj = (MiracastController *)args;
    MIRACASTLOG_TRACE("Entering...");
    sleep(1);
    if ( nullptr != miracast_ctrler_obj )
    {
        miracast_ctrler_obj->Controller_Thread(nullptr);
    }
    MIRACASTLOG_TRACE("Exiting...");
}

#ifdef ENABLE_MIRACAST_SERVICE_TEST_NOTIFIER

MiracastError MiracastController::create_TestNotifier(void)
{
    MiracastError error_code = MIRACAST_OK;

    m_test_notifier_thread = nullptr;
    MIRACASTLOG_TRACE("Entering...");
    m_test_notifier_thread = new MiracastThread( MIRACAST_SERVICE_TEST_NOTIFIER_THREAD_NAME,
                                                 MIRACAST_SERVICE_TEST_NOTIFIER_THREAD_STACK,
                                                 MIRACAST_SERVICE_TEST_NOTIFIER_MSG_COUNT,
                                                 MIRACAST_SERVICE_TEST_NOTIFIER_MSGQ_SIZE,
                                                 reinterpret_cast<void (*)(void *)>(&MiracastServiceTestNotifierThreadCallback),
                                                 this );
    if ((nullptr == m_test_notifier_thread)||
        ( MIRACAST_OK != m_test_notifier_thread->start()))
    {
        if ( nullptr != m_test_notifier_thread )
        {
            delete m_test_notifier_thread;
            m_test_notifier_thread = nullptr;
        }
        error_code = MIRACAST_FAIL;
    }
    MIRACASTLOG_TRACE("Exiting...");
    return error_code;
}

void MiracastController::destroy_TestNotifier()
{
    MIRACASTLOG_TRACE("Entering...");
    MIRACAST_SERVICE_TEST_NOTIFIER_MSGQ_ST stMsgQ = {0};
    stMsgQ.state = MIRACAST_SERVICE_TEST_NOTIFIER_SHUTDOWN;
    send_msgto_test_notifier_thread(stMsgQ);
    if (nullptr != m_test_notifier_thread)
    {
        delete m_test_notifier_thread;
        m_test_notifier_thread = nullptr;
    }
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastController::TestNotifier_Thread(void *args)
{
    MIRACAST_SERVICE_TEST_NOTIFIER_MSGQ_ST stMsgQ = {0};
    std::string device_name = "",
                mac_address = "",
                source_dev_ip = "",
                sink_dev_ip = "";

    MIRACASTLOG_TRACE("Entering...");

    while ( nullptr != m_test_notifier_thread )
    {
        memset( &stMsgQ , 0x00 , MIRACAST_SERVICE_TEST_NOTIFIER_MSGQ_SIZE );

        MIRACASTLOG_TRACE("!!! WAITING FOR NEW ACTION !!!\n");

        m_test_notifier_thread->receive_message(&stMsgQ, MIRACAST_SERVICE_TEST_NOTIFIER_MSGQ_SIZE , THREAD_RECV_MSG_INDEFINITE_WAIT);

        MIRACASTLOG_TRACE("!!! Received Action[%#08X] !!!\n", stMsgQ.state);

        device_name = stMsgQ.src_dev_name;
        mac_address = stMsgQ.src_dev_mac_addr;

        switch (stMsgQ.state)
        {
            case MIRACAST_SERVICE_TEST_NOTIFIER_CLIENT_CONNECTION_REQUESTED:
            {
                MIRACASTLOG_TRACE("[MIRACAST_SERVICE_TEST_NOTIFIER_CLIENT_CONNECTION_REQUESTED]...");
                m_notify_handler->onMiracastServiceClientConnectionRequest(mac_address, device_name);
            }
            break;
            case MIRACAST_SERVICE_TEST_NOTIFIER_SHUTDOWN:
            {
                MIRACASTLOG_TRACE("[MIRACAST_SERVICE_TEST_NOTIFIER_SHUTDOWN]...");
            }
            break;
            case MIRACAST_SERVICE_TEST_NOTIFIER_LAUNCH_REQUESTED:
            {
                MIRACASTLOG_TRACE("[MIRACAST_SERVICE_TEST_NOTIFIER_LAUNCH_REQUESTED]...");
                source_dev_ip = stMsgQ.src_dev_ip_addr;
                sink_dev_ip = stMsgQ.sink_ip_addr;
                m_notify_handler->onMiracastServiceLaunchRequest( source_dev_ip,
                                                                  mac_address,
                                                                  device_name,
                                                                  sink_dev_ip );
            }
            break;
            case MIRACAST_SERVICE_TEST_NOTIFIER_CLIENT_CONNECTION_ERROR:
            {
                MIRACASTLOG_TRACE("[MIRACAST_SERVICE_TEST_NOTIFIER_CLIENT_CONNECTION_ERROR]...");
                m_notify_handler->onMiracastServiceClientConnectionError( mac_address,
                                                                          device_name ,
                                                                          stMsgQ.error_code );
            }
            break;
            default:
            {
                MIRACASTLOG_ERROR("[UNKNOWN STATE]...");
            }
            break;
        }

        if (MIRACAST_SERVICE_TEST_NOTIFIER_SHUTDOWN == stMsgQ.state)
        {
            break;
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastServiceTestNotifierThreadCallback(void *args)
{
    MiracastController *miracast_ctlr_obj = (MiracastController *)args;
    MIRACASTLOG_TRACE("Entering [%#04X]...",miracast_ctlr_obj);
    if ( nullptr != miracast_ctlr_obj )
    {
        miracast_ctlr_obj->TestNotifier_Thread(nullptr);
    }
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastController::send_msgto_test_notifier_thread(MIRACAST_SERVICE_TEST_NOTIFIER_MSGQ_ST stMsgQ)
{
    MIRACASTLOG_TRACE("Entering...");
    if (nullptr != m_test_notifier_thread)
    {
        m_test_notifier_thread->send_message(&stMsgQ, MIRACAST_SERVICE_TEST_NOTIFIER_MSGQ_SIZE);
    }
    MIRACASTLOG_TRACE("Exiting...");
}
#endif /* ENABLE_MIRACAST_SERVICE_TEST_NOTIFIER */