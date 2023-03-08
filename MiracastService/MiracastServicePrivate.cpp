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
#include "libIBus.h"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <glib.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fstream>

#define REMOVE_SPACES(x) x.erase(std::remove(x.begin(), x.end(), ' '), x.end())
#define REMOVE_R(x) x.erase(std::remove(x.begin(), x.end(), '\r'), x.end())
#define REMOVE_N(x) x.erase(std::remove(x.begin(), x.end(), '\n'), x.end())

#define MAX_EPOLL_EVENTS 64
#define SPACE " "

//static sem_t session_client_req_sync;
static MiracastPrivate* g_miracastPrivate = NULL;

void ClientRequestHandlerCallback( void* args );
void SessionMgrThreadCallback( void* args );
void RTSPMsgHandlerCallback( void* args );

std::string MiracastPrivate::storeData(const char* tmpBuff, const char* lookup_data)
{
    char return_buf[1024] = {0};
    const char* ret = NULL, *ret_equal = NULL, *ret_space = NULL;
    ret = strstr(tmpBuff, lookup_data);
    if (NULL != ret)
    {
        if(0 == strncmp(ret, lookup_data, strlen(lookup_data)))
        {
            ret_equal = strstr(ret, "=");
            ret_space = strstr(ret_equal, " ");
            if(ret_space)
            {
                snprintf(return_buf, (int)(ret_space - ret_equal), "%s", ret+strlen(lookup_data)+1);
                MIRACASTLOG_VERBOSE("Parsed Data is - %s", return_buf);
            }
            else
            {
                snprintf(return_buf, strlen(ret_equal-1), "%s", ret+strlen(lookup_data)+1);
                MIRACASTLOG_VERBOSE("Parsed Data is - %s", return_buf);
            }
        }
    }
    if(return_buf != NULL)
        return std::string(return_buf);
    else
        return std::string(" ");
}

std::string MiracastPrivate::startDHCPClient(std::string interface)
{
    FILE *fp;
    std::string IP;
    char command[128] = {0};
    char data[1024] = {0};

    size_t len = 0;
    char *ln = NULL;

    sprintf(command, "/sbin/udhcpc -i ");
    sprintf(command+strlen(command), interface.c_str());
    sprintf(command+strlen(command), " 2>&1 | grep -i \"lease of\" | cut -d ' ' -f 4");

    MIRACASTLOG_VERBOSE("command : [%s]", command);

    fp = popen(command,"r");

    if(!fp){
        MIRACASTLOG_ERROR("Could not open pipe for output.");
        return std::string();
    }

    while (getline(&ln, &len, fp) != -1)
    {
        sprintf(data+strlen(data), ln);
        MIRACASTLOG_VERBOSE("data : [%s]", data);
        fputs(ln, stdout);
    }
    std::string local_addr(data);
    MIRACASTLOG_VERBOSE("local IP addr obtained is %s", local_addr.c_str());
    REMOVE_SPACES(local_addr);
    free(ln);
    return local_addr;
}

#if 0
bool MiracastPrivate::connectSink()
{
    std::string response;
    char buf[4096] = {0};
    size_t found;
    MIRACASTLOG_INFO("sink established. Commencing RTSP transactions.");

    int read_ret;
    size_t buf_size = sizeof(buf);

    bzero(buf, sizeof(buf));
    read_ret = read(m_tcpSockfd, buf, buf_size-1);
    if(read_ret < 0)
    {
        MIRACASTLOG_INFO("read error received (%d)%s", errno, strerror(errno));
        return false;
    }

    if(strlen(buf) > 1)
    {
        MIRACASTLOG_INFO("Buffer read ");
    }

    std::string buffer = buf;
    found = buffer.find("OPTIONS");
    MIRACASTLOG_INFO("received length is %d", read_ret);
    if(read_ret < 0)
    {
        MIRACASTLOG_INFO("returning as nothing is received");
        return false;
    }
    MIRACASTLOG_INFO("received string(%d) - %s", buffer.length(), buffer.c_str());
    MIRACASTLOG_INFO("M1 request received");

    if(found!=std::string::npos)
    {
        MIRACASTLOG_INFO("M1 OPTIONS packet received");
        size_t found_str = buffer.find("Require");
        std::string req_str;
        if(found_str != std::string::npos)
        {
            req_str = buffer.substr(found_str+9);
            REMOVE_R(req_str);
            REMOVE_N(req_str);
        }
        response = "RTSP/1.0 200 OK\r\nPublic: \"";
        response.append(req_str);
        response.append(", GET_PARAMETER, SET_PARAMETER\"\r\nCSeq: 1\r\n\r\n");
        MIRACASTLOG_INFO("%s", response.c_str());
        read_ret = 0;
        read_ret = send(m_tcpSockfd, response.c_str(), response.length(), 0);
        if(read_ret < 0)
        {
            MIRACASTLOG_INFO("write error received (%d)%s", errno, strerror(errno));
            return false;
        }

        MIRACASTLOG_INFO("Sending the M1 response %d", read_ret);
    }
    std::string m2_msg("OPTIONS * RTSP/1.0\r\nRequire: org.wfa.wfd1.0\r\nCSeq: 1\r\n\r\n");
    MIRACASTLOG_INFO("%s", m2_msg.c_str());
    read_ret = 0;
    read_ret = send(m_tcpSockfd, m2_msg.c_str(), m2_msg.length(), 0);
    if(read_ret < 0)
    {
        MIRACASTLOG_INFO("write error received (%d)%s", errno, strerror(errno));
        return false;
    }

    MIRACASTLOG_INFO("Sending the M2 request %d", read_ret);

    bzero(buf, sizeof(buf));
    read_ret = read(m_tcpSockfd, buf, buf_size-1);
    if(read_ret < 0)
    {
        MIRACASTLOG_INFO("read error received (%d)%s", errno, strerror(errno));
        return false;
    }

    if(strlen(buf) > 1)
    {
        MIRACASTLOG_INFO("Buffer read is - %s ", buf);
    }
    else
    {
        MIRACASTLOG_INFO("Buffer read is empty (M2 resp)");
        return false;
    }
    MIRACASTLOG_INFO("M2 response received");
    bzero(buf, sizeof(buf));
    read_ret = read(m_tcpSockfd, buf, buf_size-1);
    if(read_ret < 0)
    {
        MIRACASTLOG_INFO("read error received (%d)%s", errno, strerror(errno));
        return false;
    }

    if(strlen(buf) > 1)
    {
        MIRACASTLOG_INFO("Buffer read is - %s ", buf);
    }
    else
    {
        MIRACASTLOG_INFO("Buffer read is empty (M3 req)");
        return false;
    }
    MIRACASTLOG_INFO("M3 request received");
    buffer.clear();
    buffer = buf;
    if(buffer.find("wfd_video_formats") != std::string::npos)
    {
        MIRACASTLOG_INFO("In else case ");
        response.clear();
        response = "RTSP/1.0 200 OK\r\nContent-Length: 210\r\nContent-Type: text/parameters\r\nCSeq: 2\r\n\r\nwfd_content_protection: none\r\nwfd_video_formats: 00 00 03 10 0001ffff 1fffffff 00001fff 00 0000 0000 10 none none\r\nwfd_audio_codecs: AAC 00000007 00\r\nwfd_client_rtp_ports: RTP/AVP/UDP;unicast 1991 0 mode=play\r\n";
        MIRACASTLOG_INFO("%s", response.c_str());
        read_ret = 0;
        read_ret = send(m_tcpSockfd, response.c_str(), response.length(), 0);
        if(read_ret < 0)
        {
            MIRACASTLOG_INFO("write error received (%d)(%s)", errno, strerror(errno));
            return false;
        }
        MIRACASTLOG_INFO("Sending the M3 response %d", read_ret);
    }
    bzero(buf, sizeof(buf));
    read_ret = read(m_tcpSockfd, buf, buf_size-1);
    if(read_ret < 0)
    {
        MIRACASTLOG_INFO("read error received (%d)%s", errno, strerror(errno));
        return false;
    }

    if(strlen(buf) > 1)
    {
        MIRACASTLOG_INFO("Buffer read is - %s ", buf);
    }
    else
    {
        MIRACASTLOG_INFO("Buffer read is empty (M4 req)");
        return false;
    }
    MIRACASTLOG_INFO("M4 packet received");
    if(read_ret < 0)
    {
        MIRACASTLOG_INFO("returning as nothing is received");
        return false;
    }
    buffer.clear();
    buffer = buf;
    if(buffer.find("SET_PARAMETER") != std::string::npos)
    {
        response.clear();
        response = "RTSP/1.0 200 OK\r\nCSeq: 3\r\n\r\n";
        MIRACASTLOG_INFO("%s", response.c_str());
        read_ret = 0;
        read_ret = send(m_tcpSockfd, response.c_str(), response.length(), 0);
        if(read_ret < 0)
        {
            MIRACASTLOG_INFO("write error received (%d)(%s)", errno, strerror(errno));
            return false;
        }
        MIRACASTLOG_INFO("Sending the M4 response %d", read_ret);

    }
    bzero(buf, sizeof(buf));
    read_ret = read(m_tcpSockfd, buf, buf_size-1);
    if(read_ret < 0)
    {
        MIRACASTLOG_INFO("read error received (%d)%s", errno, strerror(errno));
        return false;
    }

    if(strlen(buf) > 1)
    {
        MIRACASTLOG_INFO("Buffer read is - %s ", buf);
    }
    else
    {
        MIRACASTLOG_INFO("Buffer read is empty (M5 req)");
        return false;
    }
    MIRACASTLOG_INFO("M5 packet received");
    buffer.clear();
    if(read_ret < 0)
    {
        MIRACASTLOG_INFO("returning as nothing is received");
        return false;
    }
    buffer = buf;
    if(buffer.find("wfd_trigger_method: SETUP") != std::string::npos)
    {
        response.clear();
        response = "RTSP/1.0 200 OK\r\nCSeq: 4\r\n\r\n";
        MIRACASTLOG_INFO("%s", response.c_str());
        read_ret = 0;
        read_ret = send(m_tcpSockfd, response.c_str(), response.length(), 0);
        if(read_ret < 0)
        {
            MIRACASTLOG_INFO("write error received (%d) (%s)", errno, strerror(errno));
            return false;
        }
        MIRACASTLOG_INFO("Sending the M5 response %d", read_ret);
    }

    response.clear();
    response = "SETUP rtsp://192.168.49.1/wfd1.0/streamid=0 RTSP/1.0\r\nTransport: RTP/AVP/UDP;unicast;client_port=1991\r\nCSeq: 2\r\n\r\n";
    MIRACASTLOG_INFO("%s", response.c_str());
    read_ret = 0;
    read_ret = send(m_tcpSockfd, response.c_str(), response.length(), 0);
    if(read_ret < 0)
    {
        MIRACASTLOG_INFO("write error received (%d) (%s)", errno, strerror(errno));
        return false;
    }
    MIRACASTLOG_INFO("Sending the M6 request %d", read_ret);

    bzero(buf, sizeof(buf));
    read_ret = read(m_tcpSockfd, buf, buf_size-1);
    if(read_ret < 0)
    {
        MIRACASTLOG_INFO("read error received (%d)%s", errno, strerror(errno));
        return false;
    }

    if(strlen(buf) > 1)
    {
        MIRACASTLOG_INFO("Buffer read is - %s ", buf);
    }
    else
    {
        MIRACASTLOG_INFO("Buffer read is empty (M6 resp)");
        return false;
    }
    MIRACASTLOG_INFO("M6 packet(resp) received");
    buffer.clear();
    if(read_ret < 0)
    {
        MIRACASTLOG_INFO("returning as nothing is received");
        return false;
    }
    buffer = buf;
    size_t pos_ses = buffer.find("Session");
    std::string session = buffer.substr(pos_ses+strlen("Session: "));
    pos_ses = session.find(";");
    std::string session_number = session.substr(0, pos_ses);

    if(buffer.find("client_port") != std::string::npos)
    {
        response.clear();

        response = "PLAY rtsp://192.168.49.1/wfd1.0/streamid=0 RTSP/1.0\r\nSession: ";
        response.append(session_number);
        response.append("\r\nCSeq: 3\r\n\r\n");
        MIRACASTLOG_INFO("%s", response.c_str());
        read_ret = 0;
        read_ret = send(m_tcpSockfd, response.c_str(), response.length(), 0);
        if(read_ret < 0)
        {
            MIRACASTLOG_INFO("write error received (%d) (%s)", errno, strerror(errno));
            return false;
        }
        MIRACASTLOG_INFO("Sending the M7 request %d", read_ret);
    }
    bzero(buf, sizeof(buf));
    read_ret = read(m_tcpSockfd, buf, buf_size-1);
    if(read_ret < 0)
    {
        MIRACASTLOG_INFO("read error received (%d)%s", errno, strerror(errno));
        return false;
    }

    if(strlen(buf) > 1)
    {
        MIRACASTLOG_INFO("Buffer read is - %s ", buf);
    }
    else
    {
        MIRACASTLOG_INFO("Buffer read is empty (M7 resp)");
        return false;
    }
    MIRACASTLOG_INFO("M7 packet(resp) received");
    return TRUE;
}
#endif

bool MiracastPrivate::ReceiveBufferTimedOut( void* buffer , size_t buffer_len )
{
	int recv_return = 0;
	bool status = true;

	recv_return = read(m_tcpSockfd, buffer , buffer_len );
//	recv_return = recv( m_tcpSockfd , buffer, buffer_len , MSG_WAITALL);

	if ( recv_return <= 0 ){
		status = false;

		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			printf("error: recv timed out\n");
		} else {
			printf("error: recv failed, or connection closed\n");
		}
	}
	MIRACASTLOG_INFO("received string(%d) - %s\n", recv_return, buffer);
	printf("received string(%d) - %s\n", recv_return, buffer);
	return status;
}

bool MiracastPrivate::initiateTCP(std::string goIP)
{
    int r, i, num_ready=0;
    size_t addr_size;
    struct epoll_event events[MAX_EPOLL_EVENTS];
    struct sockaddr_in addr = {0};
    struct sockaddr_storage str_addr = {0};
    std::string defaultgoIP = "192.168.49.1";

    system("iptables -I INPUT -p tcp -s 192.168.0.0/16 --dport 7236 -j ACCEPT");
    system("iptables -I OUTPUT -p tcp -s 192.168.0.0/16 --dport 7236 -j ACCEPT");
    addr.sin_family = AF_INET;
    addr.sin_port = htons(7236);
    
    if (goIP.empty()){
	    r = inet_pton(AF_INET, goIP.c_str(), &addr.sin_addr);
    }
    else
    {
	    r = inet_pton(AF_INET, goIP.c_str(), &addr.sin_addr);
    }
    
    if (r != 1)
    {
	    MIRACASTLOG_ERROR("inet_issue");
	    return false;
    }


    memcpy(&str_addr, &addr, sizeof(addr));
    addr_size = sizeof(addr);

    struct sockaddr_storage in_addr = str_addr;

    m_tcpSockfd = socket(in_addr.ss_family, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (m_tcpSockfd < 0)
    {
        MIRACASTLOG_ERROR("TCP Socket creation error %s", strerror(errno));
        return false;
    }
    /*---Add socket to epoll---*/
    int epfd = epoll_create(1);
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLOUT;
    event.data.fd = m_tcpSockfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, m_tcpSockfd, &event);

    r = connect(m_tcpSockfd, (struct sockaddr*)&in_addr, addr_size);
    if(r < 0)
    {
        if(errno != EINPROGRESS)
        {
            MIRACASTLOG_INFO("Event %s received(%d)", strerror(errno), errno);
        }
        else
        {
            MIRACASTLOG_INFO("Event (%s) received", strerror(errno));
        }
    }

    /*---Wait for socket connect to complete---*/
    num_ready = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, 1000/*timeout*/);
    for(i = 0; i < num_ready; i++)
    {
        if(events[i].events & EPOLLOUT)
        {
            MIRACASTLOG_INFO("Socket(%d) %d connected (EPOLLOUT)", i, events[i].data.fd);
        }
    }

    num_ready = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, 1000/*timeout*/);
    for(i = 0; i < num_ready; i++)
    {
        if(events[i].events & EPOLLOUT)
        {
            MIRACASTLOG_INFO("Socket %d got some data via EPOLLOUT", events[i].data.fd);
            return true;
        }
    }
    return true; 
}

SESSION_MANAGER_ACTIONS MiracastPrivate::convertP2PtoSessionActions( enum P2P_EVENTS eventId )
{
	SESSION_MANAGER_ACTIONS action_type = SESSION_MGR_INVALID_ACTION;

	switch(eventId)
	{
		case EVENT_FOUND:
		{
			action_type = SESSION_MGR_GO_DEVICE_FOUND;
		}
		break;
		case EVENT_PROVISION:
		{
			action_type = SESSION_MGR_GO_DEVICE_PROVISION;
		}
		break;
		case EVENT_STOP:
		{
			action_type = SESSION_MGR_GO_STOP_FIND;
		}
		break;
		case EVENT_GO_NEG_REQ:
		{
			action_type = SESSION_MGR_GO_NEG_REQUEST;
		}
		break;
		case EVENT_GO_NEG_SUCCESS:
		{
			action_type = SESSION_MGR_GO_NEG_SUCCESS;
		}
		break;
		case EVENT_GO_NEG_FAILURE:
		{
			action_type = SESSION_MGR_GO_NEG_FAILURE;
		}
		break;
		case EVENT_GROUP_STARTED:
		{
			action_type = SESSION_MGR_GO_GROUP_STARTED;
		}
		break;
		case EVENT_FORMATION_SUCCESS:
		{
			action_type = SESSION_MGR_GO_GROUP_FORMATION_SUCCESS;
		}
		break;
		case EVENT_FORMATION_FAILURE:
		{
			action_type = SESSION_MGR_GO_GROUP_FORMATION_FAILURE;
		}
		break;
		case EVENT_DEVICE_LOST:
		{
			action_type = SESSION_MGR_GO_DEVICE_LOST;
		}
		break;
		case EVENT_GROUP_REMOVED:
		{
			action_type = SESSION_MGR_GO_GROUP_REMOVED;
		}
		break;
		case EVENT_ERROR:
		{
			action_type = SESSION_MGR_GO_EVENT_ERROR;
		}
		break;
		default:
		{
			action_type = SESSION_MGR_GO_UNKNOWN_EVENT;
		}
		break;
	}
	return action_type;
}

void MiracastPrivate::RestartSession( void )
{
	MIRACASTLOG_INFO("Restarting the Session ...\n");
	StopSession();
	discoverDevices();
}

void MiracastPrivate::StopSession( void )
{
	MIRACASTLOG_INFO("Stopping the Session ...\n");
	stopDiscoverDevices();
	if ( m_groupInfo ){
		delete m_groupInfo;
		m_groupInfo = NULL;
	}
}

#if 1 
void MiracastPrivate::evtHandler(enum P2P_EVENTS eventId, void* data, size_t len)
{
	SessionMgrMsg msg_buffer = {0};
	std::string event_buffer;
	MIRACASTLOG_VERBOSE("event received");
	if(m_isIARMEnabled)
	{
		IARM_BUS_WiFiSrvMgr_P2P_EventData_t* EventData = (IARM_BUS_WiFiSrvMgr_P2P_EventData_t*)data;
		event_buffer = EventData->event_data;
	}
	else
	{
		event_buffer = (char*) data;
		free(data);
	}
	msg_buffer.action = convertP2PtoSessionActions( eventId );
	strcpy( msg_buffer.event_buffer , event_buffer.c_str());
	m_session_manager_thread->send_message( &msg_buffer , sizeof(msg_buffer));
	MIRACASTLOG_VERBOSE("event received : %d buffer:%s  ",eventId, event_buffer.c_str());
}
#else
void MiracastPrivate::evtHandler(enum P2P_EVENTS eventId, void* data, size_t len)
{
	std::string event_buffer;
	MIRACASTLOG_VERBOSE("event received");
	if(m_isIARMEnabled)
	{
		IARM_BUS_WiFiSrvMgr_P2P_EventData_t* EventData = (IARM_BUS_WiFiSrvMgr_P2P_EventData_t*)data;
		event_buffer = EventData->event_data;
	}
	else
	{
		event_buffer = (char*) data;
		free(data);
	}
	MIRACASTLOG_VERBOSE("event received : %d buffer:%s  ",eventId, event_buffer.c_str());

	switch(eventId)
	{
		case EVENT_FOUND:
		{
			MIRACASTLOG_INFO("Application received device found %s", event_buffer.c_str());
			std::string wfdSubElements;
			DeviceInfo* device = new DeviceInfo;
			device->deviceMAC = storeData(event_buffer.c_str(), "p2p_dev_addr"); 
			device->deviceType = storeData(event_buffer.c_str(), "pri_dev_type"); 
			device->modelName = storeData(event_buffer.c_str(), "name"); 
			wfdSubElements = storeData(event_buffer.c_str(), "wfd_dev_info"); 
			device->isCPSupported = ((strtol(wfdSubElements.c_str(), NULL, 16) >> 32) && 256);
			device->deviceRole = (DEVICEROLE)((strtol(wfdSubElements.c_str(), NULL, 16) >> 32) && 3); 
			MIRACASTLOG_VERBOSE("Device data parsed & stored successfully");
			m_eventCallback->onDeviceDiscovery(device); 
			MIRACASTLOG_VERBOSE("onDeviceDiscovery Callback initiated");
			m_deviceInfo.push_back(device);
			break;
		}
		case EVENT_PROVISION:
		{
			MIRACASTLOG_INFO("Application received event provision %s", event_buffer.c_str());
			m_authType = "pbc";
			std::string MAC = storeData(event_buffer.c_str(), "p2p_dev_addr");
			m_eventCallback->onProvisionReq(m_authType, MAC);
			break;
		}
		case EVENT_STOP:
		{
			MIRACASTLOG_INFO("Stopping P2P find");
			break;
		}
		case EVENT_GO_NEG_REQ:
		{
			MIRACASTLOG_INFO("Handler received GoNegReq");
			std::string MAC;
			size_t space_find = event_buffer.find(" ");
			size_t dev_str = event_buffer.find("dev_passwd_id");
			if((space_find != std::string::npos) && (dev_str != std::string::npos))
			{
				MAC = event_buffer.substr(space_find, dev_str-space_find);
				REMOVE_SPACES(MAC);
				m_eventCallback->onGoNegReq(MAC);
			}
			break;
		}
		case EVENT_SHOW_PIN:
		{
			MIRACASTLOG_INFO("Application received event EVENT_SHOW_PIN %s", event_buffer.c_str());
			m_authType =event_buffer.substr(44,8);
			std::string MAC = storeData(event_buffer.c_str(), "p2p_dev_addr");
			MIRACASTLOG_INFO("Application received event MAC %s m_authType :%s  ", MAC.c_str(), m_authType.c_str());
			m_eventCallback->onProvisionReq(m_authType, MAC);
			break;
		}
		case EVENT_GO_NEG_SUCCESS:
		{
			break;
		}
		case EVENT_GO_NEG_FAILURE:
		{
			break;
		}
		case EVENT_GROUP_STARTED:
		{
			MIRACASTLOG_INFO("Group stated");
			m_groupInfo = new GroupInfo;
			int ret = -1;
			size_t found = event_buffer.find("client");
			size_t found_space = event_buffer.find(" ");
			if(found != std::string::npos)
			{
				m_groupInfo->ipAddr =  storeData(event_buffer.c_str(), "ip_addr");
				m_groupInfo->ipMask =  storeData(event_buffer.c_str(), "ip_mask");
				m_groupInfo->goIPAddr =  storeData(event_buffer.c_str(), "go_ip_addr");
				size_t found_client = event_buffer.find("client");
				m_groupInfo->interface =  event_buffer.substr(found_space, found_client-found_space);
				REMOVE_SPACES(m_groupInfo->interface);

				if(getenv("GET_PACKET_DUMP") != NULL)
				{
					std::string tcpdump;
					tcpdump.append("tcpdump -i ");
					tcpdump.append(m_groupInfo->interface);
					tcpdump.append(" -s 65535 -w /opt/dump.pcap &");
					MIRACASTLOG_VERBOSE("Dump command to execute - %s", tcpdump.c_str());
					system(tcpdump.c_str());
				}
				//STB is a client in the p2p group
				m_groupInfo->isGO = false;
				std::string localIP = startDHCPClient(m_groupInfo->interface);
				if(localIP.empty())
				{
					MIRACASTLOG_ERROR("Local IP address is not obtained");
					break;
				}
				else
				{
						ret = initiateTCP(m_groupInfo->goIPAddr);
				}
				if(ret == true)
				{
					ret = false;
					ret = connectSink();
					if(ret == true)
						MIRACASTLOG_INFO("Sink Connected successfully");
					else
					{
						MIRACASTLOG_FATAL("Sink Connection failure");
						return;
					}
				}
				else
				{
					MIRACASTLOG_FATAL("TCP connection Failed");
					return;
				}
			}
			else
			{
				size_t found_go = event_buffer.find("GO");
				m_groupInfo->interface =  event_buffer.substr(found_space, found_go-found_space);
				//STB is the GO in the p2p group
				m_groupInfo->isGO = true;
			}
			m_groupInfo->SSID =  storeData(event_buffer.c_str(), "ssid");
			m_groupInfo->goDevAddr =  storeData(event_buffer.c_str(), "go_dev_addr");
			m_connectionStatus = true;
			m_eventCallback->onGroupStarted();
			break;
		}
		case EVENT_FORMATION_SUCCESS:
		{
			MIRACASTLOG_INFO("Group formation successful");
			m_eventCallback->onGroupFormationSuccess();
			break;
		}
		case EVENT_FORMATION_FAILURE:
		{
			break;
		}
		case EVENT_DEVICE_LOST:
		{
			std::string lostMAC = storeData(event_buffer.c_str(), "p2p_dev_addr");
			size_t found;
			int i = 0;
			for(auto devices : m_deviceInfo)
			{
				found = devices->deviceMAC.find(lostMAC);
				if(found != std::string::npos) 
				{
					delete devices;
					m_deviceInfo.erase(m_deviceInfo.begin()+i);
					break;
				}
				i++;
			}
			m_eventCallback->onDeviceLost(lostMAC);
			break;
		}
		case EVENT_GROUP_REMOVED:
		{
			std::string reason = storeData(event_buffer.c_str(), "reason");
			m_eventCallback->onGroupRemoved(reason);
			break;
		}
		case EVENT_ERROR:
		{
			break;
		}
		default:
		break;
	}
}
#endif

MiracastError MiracastPrivate::discoverDevices()
{
	MiracastError ret = MIRACAST_FAIL;
	MIRACASTLOG_INFO("Discovering Devices");
	std::string command, retBuffer;

#if 0
	command = "P2P_FLUSH";
	ret = executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer);
	if(ret != MIRACAST_OK)
	{
		MIRACASTLOG_ERROR("Unable to Flush ");
	}
	command.clear();
#endif

	command = "P2P_FIND";
	ret = executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer);
	if(ret != MIRACAST_OK)
	{
		MIRACASTLOG_ERROR("Unable to discover devices");
	}
	return ret;
}

MiracastError MiracastPrivate::stopDiscoverDevices()
{
	MiracastError ret = MIRACAST_FAIL;
	MIRACASTLOG_INFO("Stop Discovering Devices");
	std::string command, retBuffer;

	command = "P2P_STOP_FIND";
	ret = executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer);
	if(ret != MIRACAST_OK)
	{
		MIRACASTLOG_ERROR("Failed to Stop discovering devices");
	}
	return ret;
}


MiracastPrivate::MiracastPrivate(MiracastCallback* Callback)
{
	//To delete the rules so that it would be duplicated in this program execution
	system("iptables -D INPUT -p udp -s 192.168.0.0/16 --dport 1991 -j ACCEPT");
	system("iptables -D INPUT -p tcp -s 192.168.0.0/16 --dport 7236 -j ACCEPT");
	system("iptables -D OUTPUT -p tcp -s 192.168.0.0/16 --dport 7236 -j ACCEPT");
	p2p_init_done = false;
	m_tcpSockfd = -1;
	m_authType = "pbc";
	memset(event_buffer, '\0', sizeof(event_buffer));
	MIRACASTLOG_INFO("Private instance instantiated");

	int returnval = sem_init( &session_client_req_sync , 0, 0 );
	g_miracastPrivate = this;

	m_client_req_handler_thread = new MiracastThread( CLIENT_REQ_HANDLER_NAME , CLIENT_REQ_HANDLER_STACK , 0 , 0 , (void*)&ClientRequestHandlerCallback , NULL );
	m_client_req_handler_thread->start();

	m_session_manager_thread = new MiracastThread( SESSION_MGR_NAME , SESSION_MGR_STACK , SESSION_MGR_MSG_COUNT , SESSION_MGR_MSGQ_SIZE , (void*)&SessionMgrThreadCallback , NULL );
        m_session_manager_thread->start();
	
	m_rtsp_msg_handler_thread = new MiracastThread( RTSP_HANDLER_NAME , RTSP_HANDLER_STACK , RTSP_HANDLER_MSG_COUNT , RTSP_HANDLER_MSGQ_SIZE , (void*)&RTSPMsgHandlerCallback , NULL );
	m_rtsp_msg_handler_thread->start();

	m_rtsp_msg = new MiracastRTSPMessages();

	wfdInit(Callback);
}

MiracastPrivate::~MiracastPrivate()
{
	MIRACASTLOG_INFO("Destructor...");
	if(m_isIARMEnabled)
	{
		p2pUninit();
	}

	delete m_rtsp_msg_handler_thread;
	delete m_rtsp_msg;
	delete m_session_manager_thread;
	delete m_client_req_handler_thread;

	while(!m_deviceInfo.empty())
	{
		delete m_deviceInfo.back();
		m_deviceInfo.pop_back();
	}

	if ( m_groupInfo ){
		delete m_groupInfo;
		m_groupInfo = NULL;
	}

	system("iptables -D INPUT -p udp -s 192.168.0.0/16 --dport 1991 -j ACCEPT");
	system("iptables -D INPUT -p tcp -s 192.168.0.0/16 --dport 7236 -j ACCEPT");
	system("iptables -D OUTPUT -p tcp -s 192.168.0.0/16 --dport 7236 -j ACCEPT");
}

MiracastError MiracastPrivate::connectDevice(std::string MAC)
{
	MIRACASTLOG_VERBOSE("Connecting to the MAC - %s", MAC.c_str());
	MiracastError ret = MIRACAST_FAIL;
	std::string command("P2P_CONNECT"), retBuffer;
	command.append(SPACE);
	command.append(MAC);
	command.append(SPACE);
	command.append(m_authType);

	ret = (MiracastError)executeCommand(command, NON_GLOBAL_INTERFACE, retBuffer);
	MIRACASTLOG_VERBOSE("Establishing P2P connection with authType - %s", m_authType.c_str());
	m_eventCallback->onConnected();
	return ret;
}

MiracastError MiracastPrivate::startStreaming()
{
	MiracastError ret = MIRACAST_FAIL;

	const char* mcastIptableFile = "/opt/mcast_iptable.txt";
	std::ifstream mIpfile (mcastIptableFile);
	std::string mcast_iptable;
	if(mIpfile.is_open())
	{
		std::getline (mIpfile, mcast_iptable);
		MIRACASTLOG_INFO("Iptable reading from file [%s] as [ %s] ", mcastIptableFile , mcast_iptable.c_str());
		system(mcast_iptable.c_str());
		mIpfile.close();
	}
	else
	{
		system("iptables -I INPUT -p udp -s 192.168.0.0/16 --dport 1991 -j ACCEPT");
	}
	MIRACASTLOG_INFO("Casting started. Player initiated");
	std::string gstreamerPipeline;

	const char* mcastfile = "/opt/mcastgstpipline.txt";
	std::ifstream mcgstfile (mcastfile);

	if ( mcgstfile.is_open() )
	{
		std::getline (mcgstfile, gstreamerPipeline);
        MIRACASTLOG_INFO("gstpipeline reading from file [%s], gstreamerPipeline as [ %s] ", mcastfile, gstreamerPipeline.c_str());
        mcgstfile.close();
    }
    else 
    {
        gstreamerPipeline = "GST_DEBUG=3 gst-launch-1.0 -vvv udpsrc  port=1991 caps=\"application/x-rtp, media=video\" ! rtpmp2tdepay ! tsdemux name=demuxer demuxer. ! queue max-size-buffers=0 max-size-time=0 ! brcmvidfilter ! brcmvideodecoder ! brcmvideosink demuxer. ! queue max-size-buffers=0 max-size-time=0 ! brcmaudfilter ! brcmaudiodecoder ! brcmaudiosink";
    }

    MIRACASTLOG_INFO("pipeline constructed is --> %s", gstreamerPipeline.c_str());
    if(0 == system(gstreamerPipeline.c_str()))
        MIRACASTLOG_INFO("Pipeline created successfully ");
    else
    {
        MIRACASTLOG_INFO("Pipeline creation failure");
        return MIRACAST_FAIL;
    }

   // m_eventCallback->onStreamingStarted();
    return MIRACAST_OK;
}

bool MiracastPrivate::stopStreaming()
{
    system("iptables -D INPUT -p udp -s 192.168.0.0/16 --dport 1991 -j ACCEPT");
    return true;
}

bool MiracastPrivate::disconnectDevice()
{
    return true;
}

std::string MiracastPrivate::getConnectedMAC()
{
    return m_groupInfo->goDevAddr;
}

std::vector<DeviceInfo*> MiracastPrivate::getAllPeers()
{
    return m_deviceInfo;
}

bool MiracastPrivate::getConnectionStatus()
{
    return m_connectionStatus;
}

DeviceInfo* MiracastPrivate::getDeviceDetails(std::string MAC)
{
    DeviceInfo* deviceInfo;
    std::size_t found;
    memset(deviceInfo, 0, sizeof(deviceInfo));
    for(auto device : m_deviceInfo)
    {
        found = device->deviceMAC.find(MAC);
        if(found != std::string::npos)
        {
            deviceInfo = device;
            break;
        }
    }
    return deviceInfo;
}

bool MiracastPrivate::SendBufferTimedOut(std::string rtsp_response_buffer )
{
	int read_ret = 0;

	read_ret = send( m_tcpSockfd, rtsp_response_buffer.c_str(), rtsp_response_buffer.length(), 0 );

	if( 0 > read_ret )
	{
		MIRACASTLOG_INFO("Send Failed (%d)%s", errno, strerror(errno));
		return false;
	}

	MIRACASTLOG_INFO("Sending the RTSP response %d Data[%s]\n", read_ret,rtsp_response_buffer.c_str());
	return true;
}

RTSP_SEND_RESPONSE_CODE MiracastPrivate::validate_rtsp_m1_msg_m2_send_request(std::string rtsp_m1_msg_buffer )
{
	RTSP_SEND_RESPONSE_CODE response_code = RTSP_INVALID_MSG_RECEIVED;
	size_t found = rtsp_m1_msg_buffer.find("OPTIONS");

	MIRACASTLOG_INFO("M1 request received");
	if(found!=std::string::npos)
	{
		MIRACASTLOG_INFO("M1 OPTIONS packet received");
		size_t found_str = rtsp_m1_msg_buffer.find("Require");
		std::string req_str;
		if(found_str != std::string::npos)
		{
			req_str = rtsp_m1_msg_buffer.substr(found_str+9);
			REMOVE_R(req_str);
			REMOVE_N(req_str);
		}
		m_rtsp_msg->m1_msg_req_from_client.append(rtsp_m1_msg_buffer);

		m_rtsp_msg->m1_msg_resp_to_client.clear();
		m_rtsp_msg->m1_msg_resp_to_client.append(RTSP_M1_RESPONSE_START_TAG);
		m_rtsp_msg->m1_msg_resp_to_client.append(req_str);
		m_rtsp_msg->m1_msg_resp_to_client.append(RTSP_M1_RESPONSE_END_TAG);

		MIRACASTLOG_INFO("%s", m_rtsp_msg->m1_msg_resp_to_client.c_str());

		if ( true == SendBufferTimedOut( m_rtsp_msg->m1_msg_resp_to_client )){
			response_code = RTSP_VALID_MSG_OR_SEND_REQ_RESPONSE_OK;
			m_rtsp_msg->m2_msg_req_to_client.clear();
			m_rtsp_msg->m2_msg_req_to_client.append(RTSP_M2_REQUEST_BUFFER);

			MIRACASTLOG_INFO("%s", m_rtsp_msg->m2_msg_req_to_client.c_str());
			if ( true == SendBufferTimedOut( m_rtsp_msg->m2_msg_req_to_client )){
				MIRACASTLOG_INFO("Sending the M2 request \n");
			}
			else{
				response_code = RTSP_SEND_REQ_RESPONSE_NOK;
			}
		}
		else{
			response_code = RTSP_SEND_REQ_RESPONSE_NOK;
		}
	}
	return ( response_code );
}


RTSP_SEND_RESPONSE_CODE MiracastPrivate::validate_rtsp_m2_request_ack(std::string rtsp_m1_response_ack_buffer )
{
	//Yet to check and validate the response
	return RTSP_VALID_MSG_OR_SEND_REQ_RESPONSE_OK;
}

RTSP_SEND_RESPONSE_CODE MiracastPrivate::validate_rtsp_m3_response_back(std::string rtsp_m3_msg_buffer )
{
	RTSP_SEND_RESPONSE_CODE response_code = RTSP_INVALID_MSG_RECEIVED;

	MIRACASTLOG_INFO("M3 request received");
	
	if ( rtsp_m3_msg_buffer.find("wfd_video_formats") != std::string::npos)
	{
		m_rtsp_msg->m3_msg_req_from_client.clear();
		m_rtsp_msg->m3_msg_req_from_client.append(rtsp_m3_msg_buffer);

		m_rtsp_msg->m3_msg_resp_to_client.clear();
		m_rtsp_msg->m3_msg_resp_to_client.append(RTSP_M3_RESPONSE_TAG);
		
		MIRACASTLOG_INFO("%s", m_rtsp_msg->m3_msg_resp_to_client.c_str());
		
		if ( true == SendBufferTimedOut( m_rtsp_msg->m3_msg_resp_to_client )){
			response_code = RTSP_VALID_MSG_OR_SEND_REQ_RESPONSE_OK;
			MIRACASTLOG_INFO("Sending the M3 response \n");
		}
		else{
			response_code = RTSP_SEND_REQ_RESPONSE_NOK;
		}
	}

	return ( response_code );
}

RTSP_SEND_RESPONSE_CODE MiracastPrivate::validate_rtsp_m4_response_back(std::string rtsp_m4_msg_buffer )
{
	RTSP_SEND_RESPONSE_CODE response_code = RTSP_INVALID_MSG_RECEIVED;

	if( rtsp_m4_msg_buffer.find("SET_PARAMETER") != std::string::npos)
	{
		m_rtsp_msg->m4_msg_req_from_client.clear();
		m_rtsp_msg->m4_msg_req_from_client.append(rtsp_m4_msg_buffer);

		m_rtsp_msg->m4_msg_resp_to_client.clear();
		m_rtsp_msg->m4_msg_resp_to_client.append(RTSP_M4_RESPONSE_TAG);
		
		if ( true == SendBufferTimedOut( m_rtsp_msg->m4_msg_resp_to_client )){
			response_code = RTSP_VALID_MSG_OR_SEND_REQ_RESPONSE_OK;
			MIRACASTLOG_INFO("Sending the M4 response \n");
		}
		else{
			response_code = RTSP_SEND_REQ_RESPONSE_NOK;
		}

	}

	return ( response_code );
}

RTSP_SEND_RESPONSE_CODE MiracastPrivate::validate_rtsp_m5_msg_m6_send_request(std::string rtsp_m5_msg_buffer )
{
	RTSP_SEND_RESPONSE_CODE response_code = RTSP_INVALID_MSG_RECEIVED;

	if( rtsp_m5_msg_buffer.find("wfd_trigger_method: SETUP") != std::string::npos)
	{
		m_rtsp_msg->m5_msg_req_from_client.clear();
		m_rtsp_msg->m5_msg_req_from_client.append(rtsp_m5_msg_buffer);

		m_rtsp_msg->m5_msg_resp_to_client.clear();
		m_rtsp_msg->m5_msg_resp_to_client.append(RTSP_M5_RESPONSE_TAG);
		
		if ( true == SendBufferTimedOut( m_rtsp_msg->m5_msg_resp_to_client )){
			MIRACASTLOG_INFO("Sending the M5 response \n");

			m_rtsp_msg->m6_msg_req_to_client.clear();
			m_rtsp_msg->m6_msg_req_to_client.append(RTSP_M6_REQUEST_BUFFER);

			if ( true == SendBufferTimedOut( m_rtsp_msg->m6_msg_req_to_client )){
				response_code = RTSP_VALID_MSG_OR_SEND_REQ_RESPONSE_OK;
				MIRACASTLOG_INFO("Sending the M6 Request\n");
			}
			else{
				response_code = RTSP_SEND_REQ_RESPONSE_NOK;
			}
		}
		else{
			response_code = RTSP_SEND_REQ_RESPONSE_NOK;
		}

	}

	return ( response_code );
}

RTSP_SEND_RESPONSE_CODE MiracastPrivate::validate_rtsp_m6_ack_m7_send_request(std::string rtsp_m6_ack_buffer )
{
	RTSP_SEND_RESPONSE_CODE response_code = RTSP_INVALID_MSG_RECEIVED;

	if( !rtsp_m6_ack_buffer.empty())
	{
		m_rtsp_msg->m6_msg_req_ack_from_client.clear();
		m_rtsp_msg->m6_msg_req_ack_from_client.append(rtsp_m6_ack_buffer);

		size_t pos_ses = rtsp_m6_ack_buffer.find("Session");
		std::string session = rtsp_m6_ack_buffer.substr(pos_ses+strlen("Session: "));
		pos_ses = session.find(";");
		std::string session_number = session.substr(0, pos_ses);

		if(rtsp_m6_ack_buffer.find("client_port") != std::string::npos)
		{
			m_rtsp_msg->m7_msg_req_to_client.clear();
			m_rtsp_msg->m7_msg_req_to_client.append(RTSP_M7_REQUEST_START_TAG);
			m_rtsp_msg->m7_msg_req_to_client.append(session_number);
			m_rtsp_msg->m7_msg_req_to_client.append(RTSP_M7_REQUEST_END_TAG);
			if ( true == SendBufferTimedOut( m_rtsp_msg->m7_msg_req_to_client )){
				response_code = RTSP_VALID_MSG_OR_SEND_REQ_RESPONSE_OK;
				MIRACASTLOG_INFO("Sending the M7 Request\n");
			}
			else{
				response_code = RTSP_SEND_REQ_RESPONSE_NOK;
			}
		}
	}

	return ( response_code );
}

RTSP_SEND_RESPONSE_CODE MiracastPrivate::validate_rtsp_m7_request_ack(std::string rtsp_m7_ack_buffer )
{
	// Yet to check and validate the M7 acknowledgement
	return RTSP_VALID_MSG_OR_SEND_REQ_RESPONSE_OK;
}

RTSP_SEND_RESPONSE_CODE MiracastPrivate::validate_rtsp_msg_response_back(std::string rtsp_msg_buffer , RTSP_MSG_HANDLER_ACTIONS action_id )
{
	RTSP_SEND_RESPONSE_CODE response_code = RTSP_INVALID_MSG_RECEIVED;

	switch ( action_id )
	{
		case M1_REQUEST_RECEIVED:
		{
			response_code = validate_rtsp_m1_msg_m2_send_request( rtsp_msg_buffer );
		}
		break;
		case M2_REQUEST_ACK:
		{
			response_code = validate_rtsp_m2_request_ack( rtsp_msg_buffer );
		}
		break;
		case M3_REQUEST_RECEIVED:
		{
			response_code = validate_rtsp_m3_response_back( rtsp_msg_buffer );
		}
		break;
		case M4_REQUEST_RECEIVED:
		{
			response_code = validate_rtsp_m4_response_back( rtsp_msg_buffer );
		}
		break;
		case M5_REQUEST_RECEIVED:
		{
			response_code = validate_rtsp_m5_msg_m6_send_request( rtsp_msg_buffer );
		}
		break;
		case M6_REQUEST_ACK:
		{
			response_code = validate_rtsp_m6_ack_m7_send_request( rtsp_msg_buffer );
		}
		break;
		case M7_REQUEST_ACK:
		{
			response_code = validate_rtsp_m7_request_ack( rtsp_msg_buffer );
		}
		break;
	}
	MIRACASTLOG_INFO("Validating RTSP Msg => ACTION[%#04X] Resp[%#04X]\n",action_id,response_code);

	return response_code;
}

MiracastRTSPMessages::MiracastRTSPMessages()
{
}

MiracastRTSPMessages::~MiracastRTSPMessages()
{
}

MiracastThread::MiracastThread(std::string thread_name, size_t stack_size, size_t msg_size, size_t queue_depth , void (*callback)(void*) , void* user_data )
{
	thread_name = thread_name;
	
	thread_stacksize = stack_size;
	thread_message_size = msg_size;
	thread_message_count = queue_depth;

	thread_user_data = user_data;

	thread_callback = callback;

	// Create message queue
	g_queue = g_async_queue_new();
	//g_async_queue_ref( g_queue );
	
	sem_init( &sem_object , 0 , 0 );
	
	// Create thread
	pthread_attr_init(&attr_);
	pthread_attr_setstacksize(&attr_, stack_size); 
}

MiracastThread::~MiracastThread()
{
	// Join thread
	pthread_join(thread_, NULL);

	// Close message queue
	g_async_queue_unref( g_queue );	
}

void MiracastThread::start(void)
{
	pthread_create( &thread_, &attr_, (void*)thread_callback , thread_user_data );
}

void MiracastThread::send_message( void* message , size_t msg_size )
{
	void* buffer = malloc( msg_size );
	if ( NULL == buffer ){
		return;
	}
	memset( buffer , 0x00 , msg_size );
	// Send message to queue

	memcpy( buffer , message , msg_size );
	g_async_queue_push( this->g_queue, buffer );
	sem_post( &this->sem_object );
}

void MiracastThread::receive_message( void* message , size_t msg_size )
{
	unsigned int priority = 0;

	sem_wait( &this->sem_object );
	if ( NULL != this->g_queue ){
		void* data_ptr = static_cast<void*>(g_async_queue_pop(this->g_queue));
		if (( NULL != message ) && ( NULL != data_ptr )){
			memcpy(message , data_ptr , msg_size );
			free(data_ptr);
		}	

	}
}

void MiracastPrivate::SessionManagerThread(void* args)
{
	SessionMgrMsg session_message_data = { 0 };
	RTSPHldrMsg rtsp_message_data = {0};

	while( true ){
		std::string event_buffer;
		event_buffer.clear();

		MIRACASTLOG_INFO("[%s] Waiting for Event .....\n",__FUNCTION__);
		m_session_manager_thread->receive_message( &session_message_data , SESSION_MGR_MSGQ_SIZE);
		
		event_buffer = session_message_data.event_buffer;

		MIRACASTLOG_INFO("[%s] Received Action[%#04X]Data[%s]\n",__FUNCTION__,session_message_data.action,event_buffer.c_str());

		if ( SESSION_MGR_SELF_ABORT == session_message_data.action ){
			MIRACASTLOG_INFO("SESSION_MGR_SELF_ABORT Received.\n");
			rtsp_message_data.action = RTSP_SELF_ABORT;
			m_rtsp_msg_handler_thread->send_message( &rtsp_message_data , RTSP_HANDLER_MSGQ_SIZE);
			StopSession();
			break;
		}

		switch ( session_message_data.action )
		{
			case SESSION_MGR_START_DISCOVERING:
			{
				MIRACASTLOG_INFO("SESSION_MGR_START_DISCOVERING Received\n");
				discoverDevices();
			}
			break;
			case SESSION_MGR_STOP_DISCOVERING:
			{
				MIRACASTLOG_INFO("SESSION_MGR_STOP_DISCOVERING Received\n");
				StopSession();
			}
			break;
			case SESSION_MGR_GO_DEVICE_FOUND:
			{
				MIRACASTLOG_INFO("SESSION_MGR_GO_DEVICE_FOUND Received\n");
				std::string wfdSubElements;
				DeviceInfo* device = new DeviceInfo;
				device->deviceMAC = storeData(event_buffer.c_str(), "p2p_dev_addr"); 
				device->deviceType = storeData(event_buffer.c_str(), "pri_dev_type"); 
				device->modelName = storeData(event_buffer.c_str(), "name"); 
				wfdSubElements = storeData(event_buffer.c_str(), "wfd_dev_info"); 
				device->isCPSupported = ((strtol(wfdSubElements.c_str(), NULL, 16) >> 32) && 256);
				device->deviceRole = (DEVICEROLE)((strtol(wfdSubElements.c_str(), NULL, 16) >> 32) && 3); 
				MIRACASTLOG_VERBOSE("Device data parsed & stored successfully");
				
				m_deviceInfo.push_back(device);
			}
			break;
			case SESSION_MGR_GO_DEVICE_LOST:
			{
				MIRACASTLOG_INFO("SESSION_MGR_GO_DEVICE_LOST Received\n");
				std::string lostMAC = storeData(event_buffer.c_str(), "p2p_dev_addr");
				size_t found;
				int i = 0;
				for(auto devices : m_deviceInfo)
				{
					found = devices->deviceMAC.find(lostMAC);
					if(found != std::string::npos) 
					{
						delete devices;
						m_deviceInfo.erase(m_deviceInfo.begin()+i);
						break;
					}
					i++;
				}
			}
			break;
			case SESSION_MGR_GO_DEVICE_PROVISION:
			{
				MIRACASTLOG_INFO("SESSION_MGR_GO_DEVICE_PROVISION Received\n");
				m_authType = "pbc";
				std::string MAC = storeData(event_buffer.c_str(), "p2p_dev_addr");
			}
			break;
			case SESSION_MGR_GO_NEG_REQUEST:
			{
				ClientReqHldrMsg client_req_msg_data = {0};
				MIRACASTLOG_INFO("SESSION_MGR_GO_NEG_REQUEST Received\n");
				MIRACASTLOG_INFO("Handler received GoNegReq");
				std::string MAC;
				size_t space_find = event_buffer.find(" ");
				size_t dev_str = event_buffer.find("dev_passwd_id");
				if((space_find != std::string::npos) && (dev_str != std::string::npos))
				{
					MAC = event_buffer.substr(space_find, dev_str-space_find);
					REMOVE_SPACES(MAC);
				}

				size_t found;
				std::string device_name;
				int i = 0;
				for(auto devices : m_deviceInfo)
				{
					found = devices->deviceMAC.find(MAC);
					if(found != std::string::npos)
					{
						device_name = devices->modelName;
						break;
					}
					i++;
				}

				client_req_msg_data.action = CLIENT_REQ_HLDR_CONNECT_DEVICE_FROM_SESSION_MGR;
				strcpy( client_req_msg_data.action_buffer , MAC.c_str());
				strcpy( client_req_msg_data.buffer_user_data , device_name.c_str());

				m_client_req_handler_thread->send_message( &client_req_msg_data , sizeof(client_req_msg_data));
			}
			break;
			case SESSION_MGR_CONNECT_REQ_FROM_HANDLER:
			{
				MIRACASTLOG_INFO("SESSION_MGR_CONNECT_REQ_FROM_HANDLER Received\n");
				std::string mac_address = event_buffer;

				connectDevice( mac_address );
			}
			break;
			case SESSION_MGR_GO_GROUP_STARTED:
			{
				MIRACASTLOG_INFO("SESSION_MGR_GO_GROUP_STARTED Received\n");
				m_groupInfo = new GroupInfo;
				int ret = -1;
				size_t found = event_buffer.find("client");
				size_t found_space = event_buffer.find(" ");
				if(found != std::string::npos)
				{
					m_groupInfo->ipAddr =  storeData(event_buffer.c_str(), "ip_addr");
					m_groupInfo->ipMask =  storeData(event_buffer.c_str(), "ip_mask");
					m_groupInfo->goIPAddr =  storeData(event_buffer.c_str(), "go_ip_addr");
					m_groupInfo->goDevAddr =  storeData(event_buffer.c_str(), "go_dev_addr");

					size_t found_client = event_buffer.find("client");
					m_groupInfo->interface =  event_buffer.substr(found_space, found_client-found_space);
					REMOVE_SPACES(m_groupInfo->interface);

					if(getenv("GET_PACKET_DUMP") != NULL)
					{
						std::string tcpdump;
						tcpdump.append("tcpdump -i ");
						tcpdump.append(m_groupInfo->interface);
						tcpdump.append(" -s 65535 -w /opt/dump.pcap &");
						MIRACASTLOG_VERBOSE("Dump command to execute - %s", tcpdump.c_str());
						system(tcpdump.c_str());
					}
					//STB is a client in the p2p group
					m_groupInfo->isGO = false;
					std::string localIP = startDHCPClient(m_groupInfo->interface);
					if(localIP.empty())
					{
						MIRACASTLOG_ERROR("Local IP address is not obtained");
						break;
					}
					else
					{
						ret = initiateTCP(m_groupInfo->goIPAddr);
					}

					if(ret == true)
					{
						rtsp_message_data.action = RTSP_ACTIVATE;
						m_rtsp_msg_handler_thread->send_message( &rtsp_message_data , RTSP_HANDLER_MSGQ_SIZE);
						ret = false;
					}
					else
					{
						MIRACASTLOG_FATAL("TCP connection Failed");
						continue;
					}
				}
				else
				{
					size_t found_go = event_buffer.find("GO");
					m_groupInfo->interface =  event_buffer.substr(found_space, found_go-found_space);
					//STB is the GO in the p2p group
					m_groupInfo->isGO = true;
				}
				m_groupInfo->SSID =  storeData(event_buffer.c_str(), "ssid");
				m_connectionStatus = true;
			}
			break;
			case SESSION_MGR_GO_GROUP_REMOVED:
			{
				MIRACASTLOG_INFO("SESSION_MGR_GO_GROUP_REMOVED Received\n");
				std::string reason = storeData(event_buffer.c_str(), "reason");
				RestartSession();
			}
			break;
			case SESSION_MGR_START_STREAMING:
			{
				MIRACASTLOG_INFO("SESSION_MGR_START_STREAMING Received\n");
				startStreaming();
			}
			break;
			case SESSION_MGR_PAUSE_STREAMING:
			{
				MIRACASTLOG_INFO("SESSION_MGR_PAUSE_STREAMING Received\n");
			}
			break;
			case SESSION_MGR_STOP_STREAMING:
			{
				MIRACASTLOG_INFO("SESSION_MGR_STOP_STREAMING Received\n");
			}
			break;
			case SESSION_MGR_RTSP_MSG_RECEIVED_PROPERLY:
			{
				MIRACASTLOG_INFO("SESSION_MGR_RTSP_MSG_RECEIVED_PROPERLY Received\n");
				startStreaming();
			}
			break;
			case SESSION_MGR_RTSP_MSG_TIMEDOUT:
			case SESSION_MGR_RTSP_INVALID_MESSAGE:
			case SESSION_MGR_RTSP_SEND_REQ_RESP_FAILED:
			{
				MIRACASTLOG_INFO("SESSION_MGR_RTSP_MSG_FAILED Received\n");
				RestartSession();
			}
			break;
		}
	}
}

void MiracastPrivate::RTSPMessageHandlerThread( void* args )
{
	char rtsp_message_socket[4096] = {0};
	RTSPHldrMsg message_data = {};
	SessionMgrMsg session_mgr_buffer = {0};
	RTSP_SEND_RESPONSE_CODE response_code = RTSP_RECV_TIMEDOUT;
	std::string socket_buffer;

	while( true ){
		MIRACASTLOG_INFO("[%s] Waiting for Event .....\n",__FUNCTION__);
		m_rtsp_msg_handler_thread->receive_message( &message_data , sizeof(message_data));

		MIRACASTLOG_INFO("[%s] Received Action[%#04X]\n",__FUNCTION__,message_data.action);

		if ( RTSP_SELF_ABORT == message_data.action ){
			MIRACASTLOG_INFO("RTSP_SELF_ABORT ACTION Received\n");
			break;
		}

		if ( RTSP_ACTIVATE != message_data.action ){
			continue;
		}
		message_data.action = M1_REQUEST_RECEIVED;

		memset( &rtsp_message_socket , 0x00 , sizeof(rtsp_message_socket));

		while( ReceiveBufferTimedOut( rtsp_message_socket , sizeof(rtsp_message_socket)))
		{
			socket_buffer.clear();
			socket_buffer = rtsp_message_socket;

			response_code = validate_rtsp_msg_response_back( socket_buffer , message_data.action );
		
			MIRACASTLOG_INFO("[%s] Validate RTSP Msg Action[%#04X] Response[%#04X]\n",__FUNCTION__,message_data.action,response_code);

			if (( RTSP_VALID_MSG_OR_SEND_REQ_RESPONSE_OK != response_code) || ( M7_REQUEST_ACK == message_data.action )){
				break;
			}
			memset( &rtsp_message_socket , 0x00 , sizeof(rtsp_message_socket));
			message_data.action = message_data.action + 1;
		}

		if (( RTSP_VALID_MSG_OR_SEND_REQ_RESPONSE_OK == response_code ) && ( M7_REQUEST_ACK == message_data.action ))
		{
			session_mgr_buffer.action = SESSION_MGR_RTSP_MSG_RECEIVED_PROPERLY;
		}
		else if ( RTSP_INVALID_MSG_RECEIVED == response_code ){
			session_mgr_buffer.action = SESSION_MGR_RTSP_INVALID_MESSAGE;
		}
		else if ( RTSP_SEND_REQ_RESPONSE_NOK == response_code ){
			session_mgr_buffer.action = SESSION_MGR_RTSP_SEND_REQ_RESP_FAILED;
		}
		else{
			session_mgr_buffer.action = SESSION_MGR_RTSP_MSG_TIMEDOUT;
		}
		m_session_manager_thread->send_message( &session_mgr_buffer , sizeof(session_mgr_buffer));
	}
}

void MiracastPrivate::ClientRequestHandlerThread( void* args )
{
	SessionMgrMsg session_mgr_msg_data = {0};
	ClientReqHldrMsg client_req_hldr_msg_data = {0};
	bool send_message = false;

	while( true )
	{
		send_message = true;
		memset( &session_mgr_msg_data , 0x00 , SESSION_MGR_MSGQ_SIZE );

		MIRACASTLOG_INFO("[%s] Waiting for Event .....\n",__FUNCTION__);
		m_client_req_handler_thread->receive_message( &client_req_hldr_msg_data , sizeof(client_req_hldr_msg_data));

		MIRACASTLOG_INFO("[%s] Received Action[%#04X]\n",__FUNCTION__,client_req_hldr_msg_data.action);

		switch (client_req_hldr_msg_data.action)
		{
			case CLIENT_REQ_HLDR_START_DISCOVER:
			{
				session_mgr_msg_data.action = SESSION_MGR_START_DISCOVERING;
				printf("App: Started Device discovery\n");
			}	
			break;
			case CLIENT_REQ_HLDR_STOP_DISCOVER:
			{
				session_mgr_msg_data.action = SESSION_MGR_STOP_DISCOVERING;
			}
			break;
			case CLIENT_REQ_HLDR_CONNECT_DEVICE_FROM_SESSION_MGR:
			{
				std::string device_name = client_req_hldr_msg_data.buffer_user_data;
				std::string MAC = client_req_hldr_msg_data.action_buffer;
				int input = 1;

				printf("GO Device[%s - %s] wants to connect:\n",device_name.c_str(),MAC.c_str());
			#if 0
				printf("Enter '1' to connect:\n");
				std::cin >> input;

				if ( 1 != input ){
					send_message = false;
				}
				else
			#endif
				{
					strcpy( session_mgr_msg_data.event_buffer , MAC.c_str());
					session_mgr_msg_data.action = SESSION_MGR_CONNECT_REQ_FROM_HANDLER;
				}
			}
			break;
			case CLIENT_REQ_HLDR_STOP_APPLICATION:
			{
				session_mgr_msg_data.action = SESSION_MGR_SELF_ABORT;
				printf("App:Exiting..!\n" );
			}
			break;
		}

		if ( true == send_message ){
			m_session_manager_thread->send_message( &session_mgr_msg_data , SESSION_MGR_MSGQ_SIZE );
			if ( CLIENT_REQ_HLDR_STOP_APPLICATION == client_req_hldr_msg_data.action ){
				break;
			}
		}
	}
}

void MiracastPrivate::SendMessageToClientReqHandler( size_t action )
{
	ClientReqHldrMsg client_message_data = {0};
	bool valid_mesage = true;

	switch ( action )
	{
		case Start_WiFi_Display:
		{
			client_message_data.action = CLIENT_REQ_HLDR_START_DISCOVER;
		}
		break;
		case Stop_WiFi_Display:
		{
			client_message_data.action = CLIENT_REQ_HLDR_STOP_DISCOVER;
		}
		break;
		case Stop_Miracast_Service:
		{
			client_message_data.action = CLIENT_REQ_HLDR_STOP_APPLICATION;
		}
		break;
		default:
		{
			valid_mesage = false;
		}
		break;
	}

	if ( true == valid_mesage ){
		m_client_req_handler_thread->send_message( &client_message_data , sizeof(client_message_data));
	}
}

void ClientRequestHandlerCallback( void* args )
{
	g_miracastPrivate->ClientRequestHandlerThread(NULL);
}

void SessionMgrThreadCallback( void* args )
{
	g_miracastPrivate->SessionManagerThread(NULL);
}

void RTSPMsgHandlerCallback( void* args )
{
	g_miracastPrivate->RTSPMessageHandlerThread(NULL);
}
