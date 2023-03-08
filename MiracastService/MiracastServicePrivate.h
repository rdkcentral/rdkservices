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

#ifndef _MIRACAST_PRIVATE_H_
#define _MIRACAST_PRIVATE_H_

#include <MiracastService.h>
#include <string>
#include <vector>
#include <logger.h> 
#include <mqueue.h>
#include <glib.h>
#include "wifiSrvMgrIarmIf.h"
#include <semaphore.h>

using namespace std;
using namespace MIRACAST;

enum INTERFACE
{
    NON_GLOBAL_INTERFACE = 0,
    GLOBAL_INTERFACE
};

enum P2P_EVENTS
{
    EVENT_FOUND = 0, 
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
};

typedef enum rtsp_send_response_code_e
{
	RTSP_SEND_REQ_RESPONSE_NOK=0x01,
	RTSP_INVALID_MSG_RECEIVED,
	RTSP_VALID_MSG_OR_SEND_REQ_RESPONSE_OK,
	RTSP_RECV_TIMEDOUT
}RTSP_SEND_RESPONSE_CODE;

#define SESSION_MGR_NAME		( "MIRA_SESSION_MGR" )
#define SESSION_MGR_STACK		( 256 * 1024 )
#define SESSION_MGR_MSG_COUNT		( 5 )
#define SESSION_MGR_MSGQ_SIZE		(sizeof(SessionMgrMsg))

#define RTSP_HANDLER_NAME		( "MIRA_RTSP_MSG_HLDR" )
#define RTSP_HANDLER_STACK		( 256 * 1024 )
#define RTSP_HANDLER_MSG_COUNT		( 2 )
#define RTSP_HANDLER_MSGQ_SIZE		(sizeof(RTSPHldrMsg))

#define CLIENT_REQ_HANDLER_NAME		( "MIRA_CLIENT_REQ_HLDR" )
#define CLIENT_REQ_HANDLER_STACK	( 256 * 1024 )
#define MIRACAST_THREAD_MSG_Q_DIR	( "/tmp/miracast/" )

#define RTSP_M1_RESPONSE_START_TAG	"RTSP/1.0 200 OK\r\nPublic: \""
#define RTSP_M1_RESPONSE_END_TAG	", GET_PARAMETER, SET_PARAMETER\"\r\nCSeq: 1\r\n\r\n"
#define RTSP_M2_REQUEST_BUFFER		"OPTIONS * RTSP/1.0\r\nRequire: org.wfa.wfd1.0\r\nCSeq: 1\r\n\r\n"
#define RTSP_M3_RESPONSE_TAG		"RTSP/1.0 200 OK\r\nContent-Length: 210\r\nContent-Type: text/parameters\r\nCSeq: 2\r\n\r\nwfd_content_protection: none\r\nwfd_video_formats: 00 00 03 10 0001ffff 1fffffff 00001fff 00 0000 0000 10 none none\r\nwfd_audio_codecs: AAC 00000007 00\r\nwfd_client_rtp_ports: RTP/AVP/UDP;unicast 1991 0 mode=play\r\n"
#define RTSP_M4_RESPONSE_TAG		"RTSP/1.0 200 OK\r\nCSeq: 3\r\n\r\n"
#define RTSP_M5_RESPONSE_TAG		"RTSP/1.0 200 OK\r\nCSeq: 4\r\n\r\n"
#define RTSP_M6_REQUEST_BUFFER		"SETUP rtsp://192.168.49.1/wfd1.0/streamid=0 RTSP/1.0\r\nTransport: RTP/AVP/UDP;unicast;client_port=1991\r\nCSeq: 2\r\n\r\n"
#define RTSP_M7_REQUEST_START_TAG	"PLAY rtsp://192.168.49.1/wfd1.0/streamid=0 RTSP/1.0\r\nSession: "
#define RTSP_M7_REQUEST_END_TAG		"\r\nCSeq: 3\r\n\r\n"


typedef enum session_manager_actions_e
{
	SESSION_MGR_START_DISCOVERING = 0x01,
	SESSION_MGR_STOP_DISCOVERING,
	SESSION_MGR_GO_DEVICE_FOUND,
	SESSION_MGR_GO_DEVICE_LOST,
	SESSION_MGR_GO_DEVICE_PROVISION,
	SESSION_MGR_GO_STOP_FIND,
	SESSION_MGR_GO_NEG_REQUEST,
	SESSION_MGR_GO_NEG_SUCCESS,
	SESSION_MGR_GO_NEG_FAILURE,
	SESSION_MGR_CONNECT_REQ_FROM_HANDLER,
	SESSION_MGR_GO_GROUP_STARTED,
	SESSION_MGR_GO_GROUP_REMOVED,
	SESSION_MGR_GO_GROUP_FORMATION_SUCCESS,
	SESSION_MGR_GO_GROUP_FORMATION_FAILURE,
	SESSION_MGR_START_STREAMING,
	SESSION_MGR_PAUSE_STREAMING,
	SESSION_MGR_STOP_STREAMING,
	SESSION_MGR_RTSP_MSG_RECEIVED_PROPERLY,
	SESSION_MGR_RTSP_MSG_TIMEDOUT,
	SESSION_MGR_RTSP_INVALID_MESSAGE,
	SESSION_MGR_RTSP_SEND_REQ_RESP_FAILED,
	SESSION_MGR_GO_EVENT_ERROR,
	SESSION_MGR_GO_UNKNOWN_EVENT,
	SESSION_MGR_SELF_ABORT,
	SESSION_MGR_INVALID_ACTION
}SESSION_MANAGER_ACTIONS;

typedef enum rtsp_msg_handler_actions_e
{
	M1_REQUEST_RECEIVED = 0x01,
	M2_REQUEST_ACK,
	M3_REQUEST_RECEIVED,
	M4_REQUEST_RECEIVED,
	M5_REQUEST_RECEIVED,
	M6_REQUEST_ACK,
	M7_REQUEST_ACK,
	RTSP_ACTIVATE,
	RTSP_SELF_ABORT
}RTSP_MSG_HANDLER_ACTIONS;

typedef enum client_req_handler_actions_e
{
	CLIENT_REQ_HLDR_START_DISCOVER = 0x01,
	CLIENT_REQ_HLDR_STOP_DISCOVER,
	CLIENT_REQ_HLDR_CONNECT_DEVICE_FROM_SESSION_MGR,
	CLIENT_REQ_HLDR_STOP_APPLICATION
}
CLIENT_MSG_HANDLER_ACTIONS;

typedef struct group_info 
{
	std::string interface;
	bool isGO;
	std::string SSID;
	std::string goDevAddr;
	std::string ipAddr;
	std::string ipMask;
	std::string goIPAddr;
}GroupInfo;

typedef struct session_mgr_msg
{
	char event_buffer[2048];
	SESSION_MANAGER_ACTIONS action;
}SessionMgrMsg;

typedef struct rtsp_hldr_msg
{
	RTSP_MSG_HANDLER_ACTIONS action;
	size_t userdata;
}RTSPHldrMsg;

typedef struct client_req_ldr_msg
{
	char action_buffer[32];
	char buffer_user_data[32];
	CLIENT_MSG_HANDLER_ACTIONS action;
}
ClientReqHldrMsg;

typedef struct common_thread_msg
{
	std::string event_data;
	size_t event_type;
}CommonThreadMsg;

class MiracastRTSPMessages
{
	public:
		MiracastRTSPMessages();
		~MiracastRTSPMessages();

		std::string m1_msg_req_from_client;
		std::string m1_msg_resp_to_client;
		std::string m2_msg_req_to_client;
		std::string m2_msg_req_ack_from_client;
		std::string m3_msg_req_from_client;
		std::string m3_msg_resp_to_client;
		std::string m4_msg_req_from_client;
		std::string m4_msg_resp_to_client;
		std::string m5_msg_req_from_client;
		std::string m5_msg_resp_to_client;
		std::string m6_msg_req_to_client;
		std::string m6_msg_req_ack_from_client;
		std::string m7_msg_req_to_client;
		std::string m7_msg_req_ack_from_client;
};

class MiracastThread
{
	public:
		MiracastThread();
		MiracastThread(std::string thread_name, size_t stack_size, size_t msg_size, size_t queue_depth , void (*callback)(void*) , void* user_data );
		~MiracastThread();
		void start( void );
		void send_message( void* message , size_t msg_size );
		void receive_message( void* message , size_t msg_size );
		void* thread_user_data;

	private:
		pthread_t thread_;
		pthread_attr_t attr_;
		sem_t sem_object;
		GAsyncQueue* g_queue;
		size_t thread_stacksize;
		size_t thread_message_size;
		size_t thread_message_count;
		void (*thread_callback)(void *);
};

class MiracastThread;
class MiracastRTSPMessages;

class MiracastPrivate
{
	public:

		MiracastPrivate();
		~MiracastPrivate();
		MiracastPrivate(MiracastCallback* xreCallback);

		void CommonThreadCallBack( void* args );

		//Global APIs
		MiracastError discoverDevices();
		MiracastError selectDevice();
		MiracastError connectDevice(std::string MAC);
		MiracastError startStreaming();

		string getConnectedMAC();
		vector<DeviceInfo*> getAllPeers();
		bool getConnectionStatus();
		DeviceInfo* getDeviceDetails(std::string deviceMAC);
		void evtHandler(enum P2P_EVENTS eventId, void* data, size_t len);

		//APIs to disconnect
		bool stopStreaming();
		bool disconnectDevice();    

		/*members for interacting with wpa_supplicant*/
		void p2pCtrlMonitorThread();

		void SendMessageToClientReqHandler( size_t action );
		//Session Manager
		void SessionManagerThread(void* args);
		//RTSP Message Handler
		void RTSPMessageHandlerThread(void* args);
		// Client Request Handler
		void ClientRequestHandlerThread(void* args);
		
		bool ReceiveBufferTimedOut( void* buffer , size_t buffer_len );
		bool SendBufferTimedOut(std::string rtsp_response_buffer );
		RTSP_SEND_RESPONSE_CODE validate_rtsp_msg_response_back(std::string rtsp_msg_buffer , RTSP_MSG_HANDLER_ACTIONS action_id );
		RTSP_SEND_RESPONSE_CODE validate_rtsp_m1_msg_m2_send_request(std::string rtsp_m1_msg_buffer );
		RTSP_SEND_RESPONSE_CODE validate_rtsp_m2_request_ack(std::string rtsp_m1_response_ack_buffer );
		RTSP_SEND_RESPONSE_CODE validate_rtsp_m3_response_back(std::string rtsp_m3_msg_buffer );
		RTSP_SEND_RESPONSE_CODE validate_rtsp_m4_response_back(std::string rtsp_m4_msg_buffer );
		RTSP_SEND_RESPONSE_CODE validate_rtsp_m5_msg_m6_send_request(std::string rtsp_m5_msg_buffer );
		RTSP_SEND_RESPONSE_CODE validate_rtsp_m6_ack_m7_send_request(std::string rtsp_m6_ack_buffer );
		RTSP_SEND_RESPONSE_CODE validate_rtsp_m7_request_ack(std::string rtsp_m7_ack_buffer );
		SESSION_MANAGER_ACTIONS convertP2PtoSessionActions( enum P2P_EVENTS eventId );
		MiracastError stopDiscoverDevices();
		void RestartSession( void );
		void StopSession( void );

	private:

		MiracastError executeCommand(std::string command, int interface, std::string& retBuffer);
		std::string storeData(const char* tmpBuff, const char* lookup_data);
		std::string startDHCPClient(std::string interface);
		bool initiateTCP(std::string goIP);
		bool connectSink();
		void wfdInit(MiracastCallback* Callback);

		MiracastCallback* m_eventCallback;
		vector<DeviceInfo*> m_deviceInfo;
		GroupInfo* m_groupInfo;
		std::string m_authType;
		std::string m_iface;
		bool m_connectionStatus;
		int m_tcpSockfd;

		/*members for interacting with wpa_supplicant*/
		int p2pInit();
		int p2pUninit();
		int p2pExecute(char* cmd, enum INTERFACE iface, char* status);
		int p2pWpaCtrlSendCmd(char *cmd, struct wpa_ctrl* wpa_p2p_ctrl_iface, char* ret_buf);

		struct wpa_ctrl *wpa_p2p_cmd_ctrl_iface;
		struct wpa_ctrl *wpa_p2p_cmd_global_ctrl_iface;
		struct wpa_ctrl *wpa_p2p_ctrl_monitor;
		struct wpa_ctrl *wpa_p2p_global_ctrl_monitor;
		bool p2p_init_done;
		bool stop_p2p_monitor;
		char event_buffer[2048];
		size_t event_buffer_len;
		bool m_isIARMEnabled;
		sem_t session_client_req_sync;
		sem_t session_rtsp_req_sync;
		pthread_t p2p_ctrl_monitor_thread_id;
		MiracastRTSPMessages* m_rtsp_msg;
	        MiracastThread* m_client_req_handler_thread;
	        MiracastThread* m_session_manager_thread;
	        MiracastThread* m_rtsp_msg_handler_thread;
};

#endif
