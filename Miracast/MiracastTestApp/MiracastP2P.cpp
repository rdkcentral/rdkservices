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

#include <algorithm>
#include <stdio.h>
#include <wpa_ctrl.h>
#include <unistd.h>
#include "libIBus.h"
#include "MiracastLogger.h"
#include "MiracastController.h"
#include "MiracastP2P.h"


/* @TODO: Make a provision to read dynamic i/f files */
#define WPA_SUP_CTRL "/var/run/wpa_supplicant/p2p-dev-wlan0"
#define WPA_SUP_GLOBAL_CTRL "/opt/wpa_supplicant/wlan0-3.global"

using namespace MIRACAST;

MiracastP2P *MiracastP2P::m_miracast_p2p_obj{nullptr};

MiracastP2P::MiracastP2P(void)
{
    MIRACASTLOG_TRACE("Entering..");
    m_p2p_ctrl_monitor_thread_id = 0;
    m_wpa_p2p_cmd_ctrl_iface = nullptr;
    m_wpa_p2p_ctrl_monitor = nullptr;
    m_stop_p2p_monitor = false;
    m_isIARMEnabled = false;
    m_isWiFiDisplayParamsEnabled = false;

    m_authType = MIRACAST_DFLT_CFG_METHOD;
    m_friendly_name = "";

    memset(m_event_buffer, '\0', sizeof(m_event_buffer));
    MIRACASTLOG_TRACE("Exiting..");
}

MiracastP2P::~MiracastP2P()
{
    MIRACASTLOG_TRACE("Entering..");
    if ( false == m_isIARMEnabled )
    {
        p2pUninit();
    }
    MIRACASTLOG_TRACE("Exiting..");
}

MiracastP2P *MiracastP2P::getInstance(MiracastError &error_code)
{
    MiracastError ret_code = MIRACAST_OK;

    MIRACASTLOG_TRACE("Entering..");
    if (nullptr == m_miracast_p2p_obj)
    {
        m_miracast_p2p_obj = new MiracastP2P();
        if (nullptr != m_miracast_p2p_obj){
            ret_code = m_miracast_p2p_obj->Init();
            if ( MIRACAST_OK != ret_code){
                destroyInstance();
            }
        }
    }
    error_code = ret_code;
    MIRACASTLOG_TRACE("Exiting..");
    return m_miracast_p2p_obj;
}

void MiracastP2P::destroyInstance()
{
    MIRACASTLOG_TRACE("Entering..");
    if (nullptr != m_miracast_p2p_obj)
    {
        delete m_miracast_p2p_obj;
        m_miracast_p2p_obj = nullptr;
    }
    MIRACASTLOG_TRACE("Exiting...");
}

static P2P_EVENTS convertIARMtoP2P(IARM_EventId_t eventId)
{
    return (P2P_EVENTS)eventId;
}

static void iarmEvtHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
    MIRACASTLOG_TRACE("Entering..");
    if (strcmp(owner, IARM_BUS_NM_SRV_MGR_NAME) == 0)
    {
        P2P_EVENTS p2pEventId = convertIARMtoP2P(eventId);
        MiracastError error_code = MIRACAST_OK;
        MiracastController *miracast_obj = MiracastController::getInstance(error_code);
        miracast_obj->event_handler(p2pEventId, data, len,true);
    }
    MIRACASTLOG_TRACE("Exiting..");
}

/* The control and monitoring interface is defined and initialized during the init phase */
void p2p_monitor_thread(void *ptr);

int MiracastP2P::p2pWpaCtrlSendCmd(char *cmd, struct wpa_ctrl *wpa_p2p_ctrl_iface, char *ret_buf)
{
    int ret;
    size_t buf_len = sizeof(ret_buf);
    MIRACASTLOG_TRACE("Entering..");
    if (NULL == wpa_p2p_ctrl_iface)
    {
        MIRACASTLOG_ERROR("WIFI_HAL: Control interface is NULL. ");
        MIRACASTLOG_TRACE("Exiting...");
        return -1;
    }

    ret = wpa_ctrl_request(wpa_p2p_ctrl_iface, cmd, strlen(cmd), ret_buf, &buf_len, NULL);

    if (ret == -2)
    {
        MIRACASTLOG_ERROR("WIFI_HAL: cmd=%s timed out ", cmd);
    }
    else if (ret < 0)
    {
        MIRACASTLOG_ERROR("WIFI_HAL: cmd=%s failed ", cmd);
    }
    MIRACASTLOG_TRACE("Exiting...");
    return ret;
}

// Initializes WiFi - P2P
// Connects to the wpa_supplicant via control interface
// Gets attached to wpa_supplicant to receiver events
// Starts the p2p_monitor thread
MiracastError MiracastP2P::p2pInit()
{
    int retry = 0;
    m_stop_p2p_monitor = false;
    pthread_attr_t thread_attr;
    int ret = 0;
    MIRACASTLOG_TRACE("Entering..");
    MIRACASTLOG_INFO("WIFI_HAL: Initializing P2P WiFi HAL.");
    const char* wpa_supp_ctrl_path_ref_file = "/opt/mcast_wpa_ctrl_path.txt";
    std::string wpa_supp_ctrl_path_name;
    std::ifstream wpa_supp_ctrl_path_ref_fs(wpa_supp_ctrl_path_ref_file);

    if (wpa_supp_ctrl_path_ref_fs.is_open())
    {
        std::getline(wpa_supp_ctrl_path_ref_fs, wpa_supp_ctrl_path_name);
        MIRACASTLOG_INFO("WPA Supplicant ctrl path reading from file [%s], wpa_supp_ctrl_path as [ %s] ", wpa_supp_ctrl_path_ref_file, wpa_supp_ctrl_path_name.c_str());
        wpa_supp_ctrl_path_ref_fs.close();
    }
    else{
        wpa_supp_ctrl_path_name = WPA_SUP_DFLT_CTRL_PATH;
    }

    if (0 != access(wpa_supp_ctrl_path_name.c_str(), F_OK)){
        MIRACASTLOG_ERROR("Unable to find P2P ctrl iface path[%s]", wpa_supp_ctrl_path_name.c_str());
        return MIRACAST_INVALID_P2P_CTRL_IFACE;
    }

    /* Starting wpa_supplicant may take some time, try 10 times before giving up */
    retry = 0;
    while (retry++ < 10)
    {
        m_wpa_p2p_cmd_ctrl_iface = wpa_ctrl_open(wpa_supp_ctrl_path_name.c_str());
        if (m_wpa_p2p_cmd_ctrl_iface != NULL)
            break;
        MIRACASTLOG_ERROR("WIFI_HAL: p2p ctrl_open returned NULL ");
        sleep(1);
    }
    if (m_wpa_p2p_cmd_ctrl_iface == NULL)
    {
        MIRACASTLOG_ERROR("WIFI_HAL: wpa_ctrl_open for p2p failed for control interface ");
        MIRACASTLOG_TRACE("Exiting...");
        return MIRACAST_P2P_INIT_FAILED;
    }
    MIRACASTLOG_INFO("WIFI_HAL: m_wpa_p2p_cmd_ctrl_iface created successfully.");

    m_wpa_p2p_ctrl_monitor = wpa_ctrl_open(wpa_supp_ctrl_path_name.c_str());
    if (m_wpa_p2p_ctrl_monitor == NULL)
    {
        MIRACASTLOG_ERROR("WIFI_HAL: wpa_ctrl_open for p2p failed for monitor interface ");
        wpa_ctrl_close(m_wpa_p2p_cmd_ctrl_iface);
        m_wpa_p2p_cmd_ctrl_iface = nullptr;
        MIRACASTLOG_TRACE("Exiting...");
        return MIRACAST_P2P_INIT_FAILED;
    }
    MIRACASTLOG_INFO("WIFI_HAL: m_wpa_p2p_ctrl_monitor created successfully.");
    if (wpa_ctrl_attach(m_wpa_p2p_ctrl_monitor) != 0)
    {
        MIRACASTLOG_ERROR("WIFI_HAL: p2p wpa_ctrl_attach failed ");
        wpa_ctrl_close(m_wpa_p2p_cmd_ctrl_iface);
        m_wpa_p2p_cmd_ctrl_iface = nullptr;
        wpa_ctrl_close(m_wpa_p2p_ctrl_monitor);
        m_wpa_p2p_ctrl_monitor = nullptr;
        MIRACASTLOG_TRACE("Exiting...");
        return MIRACAST_P2P_INIT_FAILED;
    }
    MIRACASTLOG_INFO("WIFI_HAL: m_wpa_p2p_ctrl_monitor attached successfully.");
    pthread_attr_init(&thread_attr);
    pthread_attr_setstacksize(&thread_attr, 256 * 1024);

    ret = pthread_create(&m_p2p_ctrl_monitor_thread_id, &thread_attr, reinterpret_cast<void *(*)(void *)>(p2p_monitor_thread), this);
    if (ret != 0)
    {
        MIRACASTLOG_ERROR("WIFI_HAL: P2P Monitor thread creation failed ");
        wpa_ctrl_close(m_wpa_p2p_cmd_ctrl_iface);
        m_wpa_p2p_cmd_ctrl_iface = nullptr;
        wpa_ctrl_close(m_wpa_p2p_ctrl_monitor);
        m_wpa_p2p_ctrl_monitor = nullptr;
        MIRACASTLOG_TRACE("Exiting...");
        return MIRACAST_P2P_INIT_FAILED;
    }
    MIRACASTLOG_TRACE("Exiting..");
    return MIRACAST_OK;
}

void p2p_monitor_thread(void *ptr)
{
    MIRACASTLOG_TRACE("Entering...");
    MiracastP2P *obj = (MiracastP2P *)ptr;
    obj->p2pCtrlMonitorThread();
    MIRACASTLOG_TRACE("Exiting...");
}

// Unintializes WiFi - P2P
// Safely ends the p2p_monitor thread
MiracastError MiracastP2P::p2pUninit()
{
    MIRACASTLOG_TRACE("Entering..");
    MIRACASTLOG_INFO("WIFI_HAL: Stopping P2P Monitor thread");

    if (0!=m_p2p_ctrl_monitor_thread_id){
        m_stop_p2p_monitor = true;
        pthread_join(m_p2p_ctrl_monitor_thread_id, NULL);
    }

    if (NULL!=m_wpa_p2p_cmd_ctrl_iface){
        stop_discover_devices();
        wpa_ctrl_close(m_wpa_p2p_cmd_ctrl_iface);
        m_wpa_p2p_cmd_ctrl_iface = NULL;
    }
    if (NULL!=m_wpa_p2p_ctrl_monitor){
        wpa_ctrl_close(m_wpa_p2p_ctrl_monitor);
        m_wpa_p2p_ctrl_monitor = NULL;
    }

    MIRACASTLOG_TRACE("Exiting..");
    return MIRACAST_OK;
}

/*********Callback thread to send messages to Network Service Manager *********/
void MiracastP2P::p2pCtrlMonitorThread()
{
    MiracastError error_code = MIRACAST_OK;
    MIRACASTLOG_TRACE("Entering..");
    bool goStart = false;
    MiracastController *miracast_obj = MiracastController::getInstance(error_code);

    while ((m_stop_p2p_monitor != true) && (m_wpa_p2p_ctrl_monitor != NULL))
    {
        if (wpa_ctrl_pending(m_wpa_p2p_ctrl_monitor) > 0)
        {
            memset(m_event_buffer, '\0', sizeof(m_event_buffer));
            m_event_buffer_len = sizeof(m_event_buffer) - 1;

            if (0 == wpa_ctrl_recv(m_wpa_p2p_ctrl_monitor, m_event_buffer, &m_event_buffer_len))
            {
                MIRACASTLOG_TRACE("wpa_ctrl_recv got m_event_buffer = [%s]\n", m_event_buffer);
                if (strstr(m_event_buffer, "P2P-DEVICE-FOUND"))
                {
                    char *evt_buf = strdup(m_event_buffer);
                    MIRACASTLOG_INFO("P2P Device Found");
                    miracast_obj->event_handler(EVENT_FOUND, (void *)evt_buf, m_event_buffer_len);
                }
                if (strstr(m_event_buffer, "P2P-PROV-DISC-PBC-REQ"))
                {
                    char *evt_buf = strdup(m_event_buffer);
                    MIRACASTLOG_INFO("P2P Provision discovery");
                    miracast_obj->event_handler(EVENT_PROVISION, (void *)evt_buf, m_event_buffer_len);
                }
                if (strstr(m_event_buffer, "P2P-PROV-DISC-SHOW-PIN"))
                {
                    char *evt_buf = strdup(m_event_buffer);
                    MIRACASTLOG_INFO("P2P Provision discovery show PIN ");
                    miracast_obj->event_handler(EVENT_SHOW_PIN, (void *)evt_buf, m_event_buffer_len);
                }
                if (strstr(m_event_buffer, "P2P-GO-NEG-REQUEST"))
                {
                    char *evt_buf = strdup(m_event_buffer);
                    MIRACASTLOG_INFO("P2P Group owner negotiation request");
                    miracast_obj->event_handler(EVENT_GO_NEG_REQ, (void *)evt_buf, m_event_buffer_len);
                }
                if (strstr(m_event_buffer, "P2P-FIND-STOPPED"))
                {
                    char *evt_buf = strdup(m_event_buffer);
                    MIRACASTLOG_INFO("P2P find stopped");
                    miracast_obj->event_handler(EVENT_STOP, (void *)evt_buf, m_event_buffer_len);
                }
                if (strstr(m_event_buffer, "P2P-GO-NEG-SUCCESS"))
                {
                    char *evt_buf = strdup(m_event_buffer);
                    MIRACASTLOG_INFO("P2P Group owner negotiation success");
                    miracast_obj->event_handler(EVENT_GO_NEG_SUCCESS, (void *)evt_buf, m_event_buffer_len);
                }
                if (strstr(m_event_buffer, "P2P-GROUP-FORMATION-SUCCESS"))
                {
                    char *evt_buf = strdup(m_event_buffer);
                    MIRACASTLOG_INFO("P2P Formation Success");
                    miracast_obj->event_handler(EVENT_FORMATION_SUCCESS, (void *)evt_buf, m_event_buffer_len);
                }
                if (strstr(m_event_buffer, "P2P-GROUP-STARTED"))
                {
                    if (!goStart)
                    {
                        char *evt_buf = strdup(m_event_buffer);
                        MIRACASTLOG_INFO("P2P Group Started");
                        goStart = true;
                        miracast_obj->event_handler(EVENT_GROUP_STARTED, (void *)evt_buf, m_event_buffer_len);
                    }
                }
                if (strstr(m_event_buffer, "P2P-GROUP-REMOVED"))
                {
                    char *evt_buf = strdup(m_event_buffer);
                    MIRACASTLOG_INFO("P2P Group Removed");
                    miracast_obj->event_handler(EVENT_GROUP_REMOVED, (void *)evt_buf, m_event_buffer_len);
                    goStart = false;
                }
                if (strstr(m_event_buffer, "P2P-DEVICE-LOST"))
                {
                    char *evt_buf = strdup(m_event_buffer);
                    MIRACASTLOG_INFO("P2P Device Lost");
                    miracast_obj->event_handler(EVENT_DEVICE_LOST, (void *)evt_buf, m_event_buffer_len);
                }
                if (strstr(m_event_buffer, "P2P-GROUP-FORMATION-FAILURE"))
                {
                    char *evt_buf = strdup(m_event_buffer);
                    MIRACASTLOG_INFO("P2P Group formation failure");
                    miracast_obj->event_handler(EVENT_FORMATION_FAILURE, (void *)evt_buf, m_event_buffer_len);
                }
                if (strstr(m_event_buffer, "P2P-GO-NEG-FAILURE"))
                {
                    char *evt_buf = strdup(m_event_buffer);
                    MIRACASTLOG_INFO("P2P GO negotiation failure");
                    miracast_obj->event_handler(EVENT_GO_NEG_FAILURE, (void *)evt_buf, m_event_buffer_len);
                }
            }
        }
    }
    MIRACASTLOG_TRACE("Exiting ctrl monitor thread");
}

int MiracastP2P::p2pExecute(char *cmd, enum INTERFACE iface, char *ret_buf)
{
    int ret;
    MIRACASTLOG_TRACE("Entering...");
    MIRACASTLOG_INFO("WIFI_HAL: Command to execute - %s", cmd);
    ret = p2pWpaCtrlSendCmd(cmd, m_wpa_p2p_cmd_ctrl_iface, ret_buf);
    MIRACASTLOG_TRACE("Exiting...");
    return ret;
}

MiracastError MiracastP2P::executeCommand(std::string command, int interface, std::string &retBuffer)
{
    MIRACASTLOG_TRACE("Entering..");
    MIRACASTLOG_INFO("Executing P2P command %s", command.c_str());
    if (m_isIARMEnabled)
    {
        IARM_Result_t retVal = IARM_RESULT_SUCCESS;
        IARM_Bus_WiFiSrvMgr_P2P_Param_t param;
        memset(&param, 0, sizeof(param));
        strcpy(param.cmd, command.c_str());
        param.iface = interface;
        retVal = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_executeP2PCommand, (void *)&param, sizeof(param));
        if (retVal == IARM_RESULT_SUCCESS)
        {
            retBuffer = param.return_buffer;
            MIRACASTLOG_TRACE("Exiting...");
            return MIRACAST_OK;
        }
        else{
            MIRACASTLOG_TRACE("Exiting...");
            return MIRACAST_FAIL;
        }
    }
    else
    {
        char ret_buffer[2048] = {0};
        p2pExecute((char *)command.c_str(), static_cast<P2P_INTERFACE>(interface), ret_buffer);
        retBuffer = ret_buffer;
        MIRACASTLOG_INFO("command return buffer is - %s", retBuffer.c_str());
    }
    MIRACASTLOG_TRACE("Exiting..");
    return MIRACAST_OK;
}

MiracastError MiracastP2P::Init( void )
{
    MiracastError ret_code = MIRACAST_OK;

    MIRACASTLOG_TRACE("Entering..");

    if (getenv("ENABLE_MIRACAST_IARM") != NULL)
        m_isIARMEnabled = true;

    if (m_isIARMEnabled)
    {
        IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, (IARM_Bus_NMgr_P2P_EventId_t)IARM_BUS_WIFI_P2P_EVENT_onFound, iarmEvtHandler);
        IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, (IARM_Bus_NMgr_P2P_EventId_t)IARM_BUS_WIFI_P2P_EVENT_Provision, iarmEvtHandler);
        IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, (IARM_Bus_NMgr_P2P_EventId_t)IARM_BUS_WIFI_P2P_EVENT_onStop, iarmEvtHandler);
        IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, (IARM_Bus_NMgr_P2P_EventId_t)IARM_BUS_WIFI_P2P_EVENT_onGoNegReq, iarmEvtHandler);
        IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, (IARM_Bus_NMgr_P2P_EventId_t)IARM_BUS_WIFI_P2P_EVENT_onGoNegSuccess, iarmEvtHandler);
        IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, (IARM_Bus_NMgr_P2P_EventId_t)IARM_BUS_WIFI_P2P_EVENT_onGoNegFailure, iarmEvtHandler);
        IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, (IARM_Bus_NMgr_P2P_EventId_t)IARM_BUS_WIFI_P2P_EVENT_onGroupStarted, iarmEvtHandler);
        IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, (IARM_Bus_NMgr_P2P_EventId_t)IARM_BUS_WIFI_P2P_EVENT_onFormationSuccess, iarmEvtHandler);
        IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, (IARM_Bus_NMgr_P2P_EventId_t)IARM_BUS_WIFI_P2P_EVENT_onFormationFailure, iarmEvtHandler);
        IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, (IARM_Bus_NMgr_P2P_EventId_t)IARM_BUS_WIFI_P2P_EVENT_onDeviceLost, iarmEvtHandler);
        IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, (IARM_Bus_NMgr_P2P_EventId_t)IARM_BUS_WIFI_P2P_EVENT_onGroupRemoved, iarmEvtHandler);
        IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, (IARM_Bus_NMgr_P2P_EventId_t)IARM_BUS_WIFI_P2P_EVENT_onError, iarmEvtHandler);
    }
    else
    {
        ret_code = p2pInit();
        if (MIRACAST_OK != ret_code){
            MIRACASTLOG_ERROR("P2P Init failed");
        }
        else{
            MIRACASTLOG_INFO("P2P Init succeeded");
            set_FriendlyName(MIRACAST_DFLT_NAME , false);
        }
    }
    MIRACASTLOG_TRACE("Exiting..");
    return ret_code;
}

MiracastError MiracastP2P::set_WFDParameters(void)
{
    MiracastError ret = MIRACAST_OK;
    MIRACASTLOG_TRACE("Entering..");
    if (false == m_isWiFiDisplayParamsEnabled)
    {
        std::string command, retBuffer;
        command = "STATUS";
        executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer);
        command = "SET wifi_display 1";
        executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer);
        command = "P2P_PEER FIRST";
        executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer);
        command = "P2P_SET disallow_freq 5180-5900";
        executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer);
        command = "WFD_SUBELEM_SET 0";
        executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer);
        command = "WFD_SUBELEM_SET 0 000600111c4400c8";
        executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer);

        command = "SET config_methods pbc";
        executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer);

        set_FriendlyName(get_FriendlyName() , true);

        m_isWiFiDisplayParamsEnabled = true;
    }
    MIRACASTLOG_TRACE("Exiting..");
    return ret;
}

void MiracastP2P::reset_WFDParameters(void)
{
    MIRACASTLOG_TRACE("Entering...");
    m_isWiFiDisplayParamsEnabled = false;
    MIRACASTLOG_TRACE("Exiting...");
}

MiracastError MiracastP2P::discover_devices(void)
{
    MiracastError ret = MIRACAST_FAIL;
    std::string command, retBuffer;
    MIRACASTLOG_TRACE("Entering..");

    command = "P2P_FIND";
    ret = executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer);
    if (ret != MIRACAST_OK)
    {
        MIRACASTLOG_ERROR("Failed to discovering devices");
    }
    MIRACASTLOG_TRACE("Exiting..");
    return ret;
}

MiracastError MiracastP2P::stop_discover_devices(void)
{
    MiracastError ret = MIRACAST_FAIL;
    std::string command, retBuffer;
    MIRACASTLOG_TRACE("Entering...");

    command = "P2P_STOP_FIND";
    ret = executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer);
    if (ret != MIRACAST_OK)
    {
        MIRACASTLOG_ERROR("Failed to Stop discovering devices");
    }
    MIRACASTLOG_TRACE("Exiting...");
    return ret;
}

MiracastError MiracastP2P::connect_device(std::string MAC)
{
    MIRACASTLOG_TRACE("Entering...");
    MiracastError ret = MIRACAST_FAIL;
    std::string command("P2P_CONNECT"), retBuffer;
    command.append(SPACE_CHAR);
    command.append(MAC);
    command.append(SPACE_CHAR);
    command.append(m_authType);
#if 0
    // configuring go_intent as 0 to make our device as p2p_client insteadof getting p2p_group_owner
    command.append(SPACE_CHAR);
    command.append("go_intent=0");
#endif
    ret = (MiracastError)executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer);
    MIRACASTLOG_TRACE("Exiting...");
    return ret;
}

MiracastError MiracastP2P::set_FriendlyName(std::string friendly_name , bool apply )
{
    MiracastError ret = MIRACAST_OK;
    MIRACASTLOG_TRACE("Entering..");

    if (friendly_name.empty()){
        MIRACASTLOG_ERROR("Empty Friendly name has passed..");
        ret = MIRACAST_FAIL;
    }
    else{
        m_friendly_name = friendly_name;
        if (apply){
            std::string command, retBuffer;
            command = "SET device_name " + m_friendly_name;
            ret = executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer);
        }
    }

    MIRACASTLOG_TRACE("Exiting..");
    return ret;
}

std::string MiracastP2P::get_FriendlyName(void)
{
    return m_friendly_name;
}

MiracastError MiracastP2P::remove_GroupInterface(std::string group_iface_name )
{
    MiracastError ret = MIRACAST_OK;
    MIRACASTLOG_TRACE("Entering..");

    if (group_iface_name.empty()){
        MIRACASTLOG_ERROR("Empty group iface name has passed..");
        ret = MIRACAST_FAIL;
    }
    else{
        std::string command, retBuffer;
        command = "P2P_GROUP_REMOVE " + group_iface_name;
        ret = executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer);
    }

    MIRACASTLOG_TRACE("Exiting..");
    return ret;
}