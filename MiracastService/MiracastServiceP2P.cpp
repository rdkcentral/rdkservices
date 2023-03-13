/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
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

#include <MiracastServicePrivate.h>
#include <string.h>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <wpa_ctrl.h>
#include <unistd.h>
#include "libIBus.h" 

#define WPA_SUP_CTRL                   "/var/run/wpa_supplicant/p2p-dev-wlan0"
#define WPA_SUP_GLOBAL_CTRL            "/opt/wpa_supplicant/wlan0-3.global"

using namespace MIRACAST;

static MiracastPrivate* g_miracastPrivate = NULL;

static enum P2P_EVENTS convertIARMtoP2P(IARM_EventId_t eventId)
{
    return (P2P_EVENTS)eventId;
}

static void iarmEvtHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
    if (strcmp(owner, IARM_BUS_NM_SRV_MGR_NAME)  == 0)
    {
        enum P2P_EVENTS p2pEventId = convertIARMtoP2P(eventId);
        g_miracastPrivate->evtHandler(p2pEventId, data, len);
    }
}

/* The control and monitoring interface is defined and initialized during the init phase */
void monitor_thread(void* ptr);

int MiracastPrivate::p2pWpaCtrlSendCmd(char *cmd, struct wpa_ctrl* wpa_p2p_ctrl_iface, char* ret_buf)
{
    int ret;
    size_t buf_len = sizeof(ret_buf);
    if(NULL == wpa_p2p_ctrl_iface)
    {
        MIRACASTLOG_ERROR("WIFI_HAL: Control interface is NULL. ");
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
    return ret;
}

// Initializes WiFi - P2P 
// Connects to the wpa_supplicant via control interface
// Gets attached to wpa_supplicant to receiver events
// Starts the p2p_monitor thread
int MiracastPrivate::p2pInit()
{
    int retry = 0;
    stop_p2p_monitor=false;
    pthread_attr_t thread_attr;
    int ret;

    MIRACASTLOG_INFO("WIFI_HAL: Initializing P2P WiFi HAL.");
    if(p2p_init_done == true) {
        MIRACASTLOG_INFO("WIFI_HAL: Wifi p2p_init has already been done");
        return RETURN_OK;
    }
    /* Starting wpa_supplicant may take some time, try 10 times before giving up */
    retry = 0;    
    while (retry++ < 10) 
    {
        wpa_p2p_cmd_ctrl_iface = wpa_ctrl_open(WPA_SUP_CTRL);
        if (wpa_p2p_cmd_ctrl_iface != NULL) break;
        MIRACASTLOG_ERROR("WIFI_HAL: p2p ctrl_open returned NULL ");
        sleep(1);
    }
    if (wpa_p2p_cmd_ctrl_iface == NULL) {
        MIRACASTLOG_ERROR("WIFI_HAL: wpa_ctrl_open for p2p failed for control interface ");
        return RETURN_ERR;
    }
    MIRACASTLOG_INFO("WIFI_HAL: wpa_p2p_cmd_ctrl_iface created successfully.");

    wpa_p2p_ctrl_monitor = wpa_ctrl_open(WPA_SUP_CTRL);
    if ( wpa_p2p_ctrl_monitor == NULL )
    {
        MIRACASTLOG_ERROR("WIFI_HAL: wpa_ctrl_open for p2p failed for monitor interface ");
        return RETURN_ERR;
    }
    MIRACASTLOG_INFO("WIFI_HAL: wpa_p2p_ctrl_monitor created successfully.");
    if ( wpa_ctrl_attach(wpa_p2p_ctrl_monitor) != 0)
    {
        MIRACASTLOG_ERROR("WIFI_HAL: p2p wpa_ctrl_attach failed ");
        return RETURN_ERR;
    }
    MIRACASTLOG_INFO("WIFI_HAL: wpa_p2p_ctrl_monitor attached successfully.");

    retry = 0;    
    while (retry++ < 10) 
    {
        wpa_p2p_cmd_global_ctrl_iface = wpa_ctrl_open(WPA_SUP_GLOBAL_CTRL);
        if (wpa_p2p_cmd_global_ctrl_iface != NULL) break;
        MIRACASTLOG_ERROR("p2p global ctrl_open returned NULL ");
        sleep(1);
    }
    if (wpa_p2p_cmd_global_ctrl_iface == NULL)
    {
        MIRACASTLOG_ERROR("WIFI_HAL: wpa_ctrl_open for p2p failed for global control interface ");
        return RETURN_ERR;
    }
    MIRACASTLOG_INFO("WIFI_HAL: wpa_p2p_cmd_global_ctrl_iface created successfully.");

    pthread_attr_init(&thread_attr);
    pthread_attr_setstacksize(&thread_attr, 256*1024);

    ret = pthread_create(&p2p_ctrl_monitor_thread_id, &thread_attr, reinterpret_cast<void* (*)(void*)>(monitor_thread), this);
    if (ret != 0)
    { 
        MIRACASTLOG_ERROR("WIFI_HAL: P2P Monitor thread creation failed ");
        return RETURN_ERR;
    }
    p2p_init_done = true;

    return RETURN_OK;
}

void monitor_thread(void* ptr)
{
    MiracastPrivate *obj = (MiracastPrivate*)ptr;
    obj->p2pCtrlMonitorThread();
}

// Unintializes WiFi - P2P
// Safely ends the p2p_monitor thread
int MiracastPrivate::p2pUninit()
{
    MIRACASTLOG_INFO("WIFI_HAL: Stopping P2P Monitor thread");

    stop_p2p_monitor = true;
    pthread_join (p2p_ctrl_monitor_thread_id, NULL);
    wpa_ctrl_close(wpa_p2p_cmd_global_ctrl_iface);
    wpa_ctrl_close(wpa_p2p_cmd_ctrl_iface);
    wpa_ctrl_close(wpa_p2p_ctrl_monitor);

    p2p_init_done = false;
    return RETURN_OK;
}

/*********Callback thread to send messages to Network Service Manager *********/
void MiracastPrivate::p2pCtrlMonitorThread()
{
    bool goStart = false, goReq = false, goSuc = false;
    while ((stop_p2p_monitor != true) && (wpa_p2p_ctrl_monitor != NULL))
    {
        if (wpa_ctrl_pending(wpa_p2p_ctrl_monitor) > 0)
        {
            memset(event_buffer, '\0', sizeof(event_buffer));
            event_buffer_len = sizeof(event_buffer) - 1;

            if (0 == wpa_ctrl_recv(wpa_p2p_ctrl_monitor, event_buffer, &event_buffer_len))
            {
                MIRACASTLOG_VERBOSE("wpa_ctrl_recv got event_buffer = [%s]\n", event_buffer);
                if(strstr(event_buffer, "P2P-DEVICE-FOUND"))
                {
                    char* evt_buf = strdup(event_buffer);
                    MIRACASTLOG_INFO("P2P Device Found");
                    evtHandler(EVENT_FOUND, (void*)evt_buf, event_buffer_len);
                }
                if(strstr(event_buffer, "P2P-PROV-DISC-PBC-REQ"))
                {
                    char* evt_buf = strdup(event_buffer);
                    MIRACASTLOG_INFO( "P2P Provision discovery");
                    evtHandler(EVENT_PROVISION, (void*)evt_buf, event_buffer_len);
                }
                if(strstr(event_buffer, "P2P-PROV-DISC-SHOW-PIN"))
                {
                    char* evt_buf = strdup(event_buffer);
                    MIRACASTLOG_INFO( "P2P Provision discovery show PIN ");
                    evtHandler(EVENT_SHOW_PIN,(void*)evt_buf, event_buffer_len);
                }
                if(strstr(event_buffer, "P2P-GO-NEG-REQUEST"))
                {
                    if(!goReq)
                    {
                        char* evt_buf = strdup(event_buffer);
                        goReq = true;
                        MIRACASTLOG_INFO("P2P Group owner negotiation request");
                        evtHandler(EVENT_GO_NEG_REQ, (void*)evt_buf, event_buffer_len);
                    }
                }
                if(strstr(event_buffer, "P2P-FIND-STOPPED"))
                {
                    char* evt_buf = strdup(event_buffer);
                    MIRACASTLOG_INFO("P2P find stopped");
                    evtHandler(EVENT_STOP, (void*)evt_buf, event_buffer_len);
                }
                if(strstr(event_buffer, "P2P-GO-NEG-SUCCESS"))
                {
                    if(!goSuc)
                    {
                        char* evt_buf = strdup(event_buffer);
                        goSuc = true;
                        MIRACASTLOG_INFO("P2P Group owner negotiation success");
                        evtHandler(EVENT_GO_NEG_SUCCESS, (void*)evt_buf, event_buffer_len);
                    }
                }
                if(strstr(event_buffer, "P2P-GROUP-FORMATION-SUCCESS"))
                {
                    char* evt_buf = strdup(event_buffer);
                    MIRACASTLOG_INFO("P2P Formation Success");
                    evtHandler(EVENT_FORMATION_SUCCESS, (void*)evt_buf, event_buffer_len);
                }
                if(strstr(event_buffer, "P2P-GROUP-STARTED"))
                {
                    if(!goStart)
                    {
                        goStart = true;
                        char* evt_buf = strdup(event_buffer);
                        MIRACASTLOG_INFO("P2P Group Started");
                        evtHandler(EVENT_GROUP_STARTED, (void*)evt_buf, event_buffer_len);
                    }
                }
                if(strstr(event_buffer, "P2P-GROUP-REMOVED"))
                {
                    char* evt_buf = strdup(event_buffer);
                    MIRACASTLOG_INFO("P2P Group Removed");
                    evtHandler(EVENT_GROUP_REMOVED, (void*)evt_buf, event_buffer_len);
                }
                if(strstr(event_buffer, "P2P-DEVICE-LOST"))
                {
                    char* evt_buf = strdup(event_buffer);
                    MIRACASTLOG_INFO("P2P Device Lost");
                    evtHandler(EVENT_DEVICE_LOST, (void*)evt_buf, event_buffer_len);
                }
                if(strstr(event_buffer, "P2P-GROUP-FORMATION-FAILURE"))
                {
                    char* evt_buf = strdup(event_buffer);
                    MIRACASTLOG_INFO("P2P Group formation failure");
                    evtHandler(EVENT_FORMATION_FAILURE, (void*)evt_buf, event_buffer_len);
                }
                if(strstr(event_buffer, "P2P-GO-NEG-FAILURE"))
                {
                    char* evt_buf = strdup(event_buffer);
                    MIRACASTLOG_INFO("P2P GO negotiation failure");
                    evtHandler(EVENT_GO_NEG_FAILURE, (void*)evt_buf, event_buffer_len);
                }
            }
        }
    }
    MIRACASTLOG_INFO("Exiting ctrl monitor thread");
}

int MiracastPrivate::p2pExecute(char* cmd, enum INTERFACE iface, char* ret_buf)
{
    int ret;
    MIRACASTLOG_INFO("WIFI_HAL: Command to execute - %s", cmd);
    if(iface == NON_GLOBAL_INTERFACE)
        ret = p2pWpaCtrlSendCmd(cmd, wpa_p2p_cmd_ctrl_iface, ret_buf);
    else
        ret = p2pWpaCtrlSendCmd(cmd, wpa_p2p_cmd_global_ctrl_iface, ret_buf);

    return ret;
}

MiracastError MiracastPrivate::executeCommand(std::string command, int interface, std::string& retBuffer)
{
    MIRACASTLOG_INFO("Executing P2P command %s", command.c_str());
    if(m_isIARMEnabled)
    {
        IARM_Result_t retVal = IARM_RESULT_SUCCESS;
        IARM_Bus_WiFiSrvMgr_P2P_Param_t param;
        memset(&param, 0, sizeof(param));
        strcpy(param.cmd, command.c_str());
        param.iface = interface;
        retVal = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_executeP2PCommand, (void *)&param, sizeof(param));
        if(retVal == IARM_RESULT_SUCCESS)
        {
            retBuffer = param.return_buffer;
            return MIRACAST_OK;
        }
        else
            return MIRACAST_FAIL;
    }
    else
    {
        char ret_buffer[2048] = {0};
        p2pExecute((char*)command.c_str(), static_cast<P2P_INTERFACE>(interface), ret_buffer);
        retBuffer = ret_buffer;
        MIRACASTLOG_INFO("command return buffer is - %s", retBuffer.c_str());
    }
    return MIRACAST_OK;
}


void MiracastPrivate::wfdInit(MiracastCallback* Callback)
{
    if(getenv("ENABLE_MIRACAST_IARM") != NULL)
        m_isIARMEnabled = true;

    if(m_isIARMEnabled)
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
        g_miracastPrivate = this;
    }
    else
    {
        if(RETURN_ERR == p2pInit())
            MIRACASTLOG_ERROR("P2P Init failed");
        else
            MIRACASTLOG_INFO("P2P Init succeeded");
    }

    m_eventCallback = Callback;
    std::string command, retBuffer;
    command = "STATUS";
    executeCommand(command, GLOBAL_INTERFACE, retBuffer);
    command = "SET wifi_display 1";
    executeCommand(command, GLOBAL_INTERFACE, retBuffer);
    command = "P2P_PEER FIRST";
    executeCommand(command, GLOBAL_INTERFACE, retBuffer);
    command = "P2P_SET disallow_freq 5180-5900";
    executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer); 
    command = "WFD_SUBELEM_SET 0";
    executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer); 
    command = "WFD_SUBELEM_SET 0 000600111c4400c8";
    executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer);
#if 0 
    command = "P2P_FLUSH";
    executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer); 
    command = "P2P_FIND";
    executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer); 
#endif
}
