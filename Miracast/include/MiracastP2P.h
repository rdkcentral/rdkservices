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

#ifndef _MIRACAST_P2P_H_
#define _MIRACAST_P2P_H_

#include <string.h>
#include <MiracastServiceError.h>
#include <MiracastLogger.h>

using namespace std;
using namespace MIRACAST;

#define SPACE_CHAR " "

typedef enum INTERFACE
{
    NON_GLOBAL_INTERFACE = 0,
    GLOBAL_INTERFACE
}P2P_INTERFACE;

typedef enum p2p_events_e
{
    EVENT_FOUND = 0x00,
    EVENT_PROVISION,
    EVENT_STOP,
    EVENT_GO_NEG_REQ,
    EVENT_GO_NEG_SUCCESS,
    EVENT_GO_NEG_FAILURE,
    EVENT_GROUP_STARTED,
    EVENT_FORMATION_SUCCESS,
    EVENT_FORMATION_FAILURE,
    EVENT_DEVICE_LOST,
    EVENT_GROUP_REMOVED,
    EVENT_SHOW_PIN,
    EVENT_ERROR
}
P2P_EVENTS;

#define ENABLE_DELAY_FOR_MIRACAST_P2P_SCAN

#define MIRACAST_DFLT_NAME "Miracast-Generic"
#define MIRACAST_DFLT_CFG_METHOD "pbc"
#define MIRACAST_DFLT_P2P_SCAN_INTERVAL_MSEC "3000"

class MiracastP2P
{
private:
    static MiracastP2P *m_miracast_p2p_obj;
    MiracastP2P *m_ctrler_evt_hdlr;
    MiracastP2P();
    virtual ~MiracastP2P();
    MiracastP2P &operator=(const MiracastP2P &) = delete;
    MiracastP2P(const MiracastP2P &) = delete;
    
    std::string m_friendly_name;
    std::string m_authType;
    struct wpa_ctrl *m_wpa_p2p_cmd_ctrl_iface;
    struct wpa_ctrl *m_wpa_p2p_ctrl_monitor;
    bool m_stop_p2p_monitor;
    char m_event_buffer[2048];
    size_t m_event_buffer_len;
    bool m_isWiFiDisplayParamsEnabled;
    pthread_t m_p2p_ctrl_monitor_thread_id;

    MiracastError p2pInit(std::string p2p_ctrl_iface);
    MiracastError p2pUninit();
    MiracastError executeCommand(std::string command, int interface, std::string &retBuffer);
    int p2pExecute(char *cmd, enum INTERFACE iface, char *status);
    int p2pWpaCtrlSendCmd(char *cmd, struct wpa_ctrl *wpa_p2p_ctrl_iface, char *ret_buf);
    void Release_P2PCtrlInterface(void);

public:
    static MiracastP2P *getInstance(MiracastError &error_code,std::string p2p_ctrl_iface);
    static void destroyInstance();

    /*members for interacting with wpa_supplicant*/
    MiracastError Init(std::string p2p_ctrl_iface);
    void p2pCtrlMonitorThread();
    MiracastError set_WFDParameters(void);
    void reset_WFDParameters();
    MiracastError discover_devices(void);
    MiracastError stop_discover_devices(void);
    MiracastError connect_device(std::string MAC);

    MiracastError set_FriendlyName(std::string friendly_name , bool apply=false );
    std::string get_FriendlyName(void);
    MiracastError remove_GroupInterface(std::string group_iface_name);
};

#endif