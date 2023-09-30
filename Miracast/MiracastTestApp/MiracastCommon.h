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

#ifndef _MIRACAST_COMMON_H_
#define _MIRACAST_COMMON_H_

#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <glib.h>
#include <semaphore.h>
#include <MiracastTestAppError.h>
#include <MiracastLogger.h>

using namespace std;
using namespace MIRACAST;

#define REMOVE_SPACES(x) x.erase(std::remove(x.begin(), x.end(), ' '), x.end())
#define REMOVE_R(x) x.erase(std::remove(x.begin(), x.end(), '\r'), x.end())
#define REMOVE_N(x) x.erase(std::remove(x.begin(), x.end(), '\n'), x.end())

#define SPACE_CHAR " "

#define ONE_SECOND_IN_MILLISEC (1000)

#define THREAD_RECV_MSG_INDEFINITE_WAIT (-1)

#define WPA_SUP_DFLT_CTRL_PATH "/var/run/wpa_supplicant/p2p-dev-wlan0"

typedef enum controller_framework_states_e
{
    THUNDER_REQ_HLDR_START_DISCOVER = 0x00000001,
    THUNDER_REQ_HLDR_STOP_DISCOVER,
    THUNDER_REQ_HLDR_CONNECT_DEVICE_FROM_CONTROLLER,
    THUNDER_REQ_HLDR_CONNECT_DEVICE_ACCEPTED,
    THUNDER_REQ_HLDR_CONNECT_DEVICE_REJECTED,
    THUNDER_REQ_HLDR_TEARDOWN_CONNECTION,
    THUNDER_REQ_HLDR_SHUTDOWN_APP,
    CONTROLLER_START_DISCOVERING = 0x0000FF00,
    CONTROLLER_STOP_DISCOVERING,
    CONTROLLER_START_STREAMING,
    CONTROLLER_PAUSE_STREAMING,
    CONTROLLER_STOP_STREAMING,
    CONTROLLER_CONNECT_REQ_FROM_THUNDER,
    CONTROLLER_CONNECT_REQ_REJECT,
    CONTROLLER_CONNECT_REQ_TIMEOUT,
    CONTROLLER_TEARDOWN_REQ_FROM_THUNDER,
    CONTROLLER_GO_DEVICE_FOUND,
    CONTROLLER_GO_DEVICE_LOST,
    CONTROLLER_GO_DEVICE_PROVISION,
    CONTROLLER_GO_STOP_FIND,
    CONTROLLER_GO_NEG_REQUEST,
    CONTROLLER_GO_NEG_SUCCESS,
    CONTROLLER_GO_NEG_FAILURE,
    CONTROLLER_GO_GROUP_STARTED,
    CONTROLLER_GO_GROUP_REMOVED,
    CONTROLLER_GO_GROUP_FORMATION_SUCCESS,
    CONTROLLER_GO_GROUP_FORMATION_FAILURE,
    CONTROLLER_GO_EVENT_ERROR,
    CONTROLLER_GO_UNKNOWN_EVENT,
    CONTROLLER_RTSP_MSG_RECEIVED_PROPERLY,
    CONTROLLER_RTSP_MSG_TIMEDOUT,
    CONTROLLER_RTSP_INVALID_MESSAGE,
    CONTROLLER_RTSP_SEND_REQ_RESP_FAILED,
    CONTROLLER_RTSP_TEARDOWN_REQ_RECEIVED,
    CONTROLLER_RTSP_RESTART_DISCOVERING,
    CONTROLLER_SELF_ABORT,
    CONTROLLER_INVALID_STATE,
    RTSP_M1_REQUEST_RECEIVED = 0x000FF0000,
    RTSP_M2_REQUEST_ACK,
    RTSP_M3_REQUEST_RECEIVED,
    RTSP_M4_REQUEST_RECEIVED,
    RTSP_M5_REQUEST_RECEIVED,
    RTSP_M6_REQUEST_ACK,
    RTSP_M7_REQUEST_ACK,
    RTSP_MSG_POST_M1_M7_XCHANGE,
    RTSP_START_RECEIVE_MSGS,
    RTSP_TEARDOWN_FROM_SINK2SRC,
    RTSP_RESTART,
    RTSP_PAUSE_FROM_SINK2SRC,
    RTSP_PLAY_FROM_SINK2SRC,
    RTSP_SELF_ABORT,
    RTSP_INVALID_ACTION
}eCONTROLLER_FW_STATES;

typedef struct group_info
{
    std::string interface;
    bool isGO;
    std::string SSID;
    std::string goDevAddr;
    std::string ipAddr;
    std::string ipMask;
    std::string goIPAddr;
    std::string localIPAddr;
} GroupInfo;

typedef enum msg_type_e
{
    P2P_MSG = 0x01,
    RTSP_MSG,
    CONTRLR_FW_MSG
}eMSG_TYPE;

typedef enum miracast_service_states_e
{
    MIRACAST_SERVICE_SHUTDOWN = 0x01,
    MIRACAST_SERVICE_WFD_START,
    MIRACAST_SERVICE_WFD_STOP,
    MIRACAST_SERVICE_ACCEPT_CLIENT,
    MIRACAST_SERVICE_REJECT_CLIENT,
    MIRACAST_SERVICE_STOP_CLIENT_CONNECTION
} MIRACAST_SERVICE_STATES;

enum DEVICEROLE
{
    DEVICEROLE_SOURCE = 0,
    DEVICEROLE_PRIMARY_SINK,
    DEVICEROLE_SECONDARY_SINK,
    DEVICEROLE_DUAL_ROLE
};

typedef struct d_info
{
    string deviceMAC;
    string deviceType;
    string modelName;
    bool isCPSupported;
    enum DEVICEROLE deviceRole;
} DeviceInfo;

typedef struct controller_msgq_st
{
    char msg_buffer[2048];
    eCONTROLLER_FW_STATES state;
    eMSG_TYPE msg_type;
} CONTROLLER_MSGQ_STRUCT;

typedef struct rtsp_hldr_msgq_st
{
    eCONTROLLER_FW_STATES state;
    size_t userdata;
} RTSP_HLDR_MSGQ_STRUCT;

typedef struct thunder_req_hldr_msg_st
{
    char msg_buffer[32];
    char buffer_user_data[32];
    eCONTROLLER_FW_STATES state;
}THUNDER_REQ_HDLR_MSGQ_STRUCT;

#define CONTROLLER_THREAD_NAME ("CONTROL_MSG_HANDLER")
#define CONTROLLER_THREAD_STACK (256 * 1024)
#define CONTROLLER_MSGQ_COUNT (5)
#define CONTROLLER_MSGQ_SIZE (sizeof(CONTROLLER_MSGQ_STRUCT))

#define THUNDER_REQ_HANDLER_THREAD_NAME ("THUNDER_REQ_HLDR")
#define THUNDER_REQ_HANDLER_THREAD_STACK (256 * 1024)
#define THUNDER_REQ_HANDLER_MSGQ_COUNT (1)
#define THUNDER_REQ_HANDLER_MSGQ_SIZE (sizeof(THUNDER_REQ_HDLR_MSGQ_STRUCT))

#define RTSP_HANDLER_THREAD_NAME ("RTSP_MSG_HLDR")
#define RTSP_HANDLER_THREAD_STACK (256 * 1024)
#define RTSP_HANDLER_MSG_COUNT (2)
#define RTSP_HANDLER_MSGQ_SIZE (sizeof(RTSP_HLDR_MSGQ_STRUCT))

#ifdef ENABLE_TEST_NOTIFIER
typedef enum test_notifier_states_e{
    TEST_NOTIFIER_INVALID_STATE = 0x00,
    TEST_NOTIFIER_CLIENT_CONNECTION_REQUESTED,
    TEST_NOTIFIER_CLIENT_STOP_REQUESTED,
    TEST_NOTIFIER_CLIENT_CONNECTION_STARTED,
    TEST_NOTIFIER_CLIENT_CONNECTION_ERROR,
    TEST_NOTIFIER_SHUTDOWN
}
TEST_NOTIFIER_STATES;

#define TEST_NOTIFIER_THREAD_NAME ("TEST_NOTIFIER")
#define TEST_NOTIFIER_THREAD_STACK (20 * 1024)
#define TEST_NOTIFIER_MSG_COUNT (1)
#define TEST_NOTIFIER_MSGQ_SIZE (sizeof(TEST_NOTIFIER_STATES))

#endif /*ENABLE_TEST_NOTIFIER*/

/**
 * Abstract class for Notification.
 */
class MiracastTestAppNotifier
{
public:
    virtual void onMiracastTestAppClientConnectionRequest(string client_mac, string client_name) = 0;
    virtual void onMiracastTestAppClientStopRequest(string client_mac, string client_name) = 0;
    virtual void onMiracastTestAppClientConnectionStarted(string client_mac, string client_name) = 0;
    virtual void onMiracastTestAppClientConnectionError(string client_mac, string client_name) = 0;
};

class MiracastThread
{
public:
    MiracastThread();
    MiracastThread(std::string thread_name, size_t stack_size, size_t msg_size, size_t queue_depth, void (*callback)(void *), void *user_data);
    ~MiracastThread();
    MiracastError start(void);
    void send_message(void *message, size_t msg_size);
    int8_t receive_message(void *message, size_t msg_size, int sem_wait_timedout);

private:
    std::string m_thread_name;
    pthread_t m_pthread_id;
    pthread_attr_t m_pthread_attr;
    sem_t m_empty_msgq_sem_obj;
    GAsyncQueue *m_g_queue;
    size_t m_thread_stacksize;
    size_t m_thread_message_size;
    size_t m_thread_message_count;
    void (*m_thread_callback)(void *);    
    void *m_thread_user_data;
};
#endif