#include <gtest/gtest.h>
#include <future>
#include <thread>
#include "ServiceMock.h"
#include "MiracastPlayer.h"
#include "WrapsMock.h"

using ::testing::NiceMock;
using namespace WPEFramework;

namespace {

#define RTSP_SEND (1)
#define RTSP_RECV (2)

#define BUFFER_SIZE (2048)
#define PORT (7236)

typedef enum rtsp_srcmsg_reqresp
{
	RTSP_SEND_M1_REQUEST = 0x00,
	RTSP_RECV_M1_RESPONSE,
	RTSP_RECV_M2_REQUEST,
	RTSP_SEND_M2_RESPONSE,
	RTSP_SEND_M3_REQUEST,
	RTSP_RECV_M3_RESPONSE,
	RTSP_SEND_M4_REQUEST,
	RTSP_RECV_M4_RESPONSE,
	RTSP_SEND_M5_REQUEST,
	RTSP_RECV_M5_RESPONSE,
	RTSP_RECV_M6_REQUEST,
	RTSP_SEND_M6_RESPONSE,
	RTSP_RECV_M7_REQUEST,
	RTSP_SEND_M7_RESPONSE,
	RTSP_SEND_M16_REQUEST,
	RTSP_RECV_M16_RESPONSE,
	RTSP_SEND_TEARDOWN_REQUEST,
	RTSP_RECV_TEADOWN_RESPONSE,
	RTSP_RECV_TEARDOWN_REQUEST,
	RTSP_SEND_TEARDOWN_RESPONSE
}
RTSP_SRCMSG_REQ_RESP;

typedef struct rtsp_src_msg_handler_format
{
	int rtsp_sendorreceive;
	RTSP_SRCMSG_REQ_RESP rtsp_msg_type;
	const char* template_name;
}
RTSP_MSG_HANDLER_FORMAT;

RTSP_MSG_HANDLER_FORMAT default_rtsp_srcMsgbuffer[] =
{
	{ RTSP_SEND , RTSP_SEND_M1_REQUEST , "OPTIONS * RTSP/1.0\r\nCSeq: 1\r\nServer: AllShareCast/Galaxy/Android13\r\nRequire: org.wfa.wfd1.0\r\n"},
	{ RTSP_RECV , RTSP_RECV_M1_RESPONSE , "RTSP/1.0 200 OK\r\nPublic: \"org.wfa.wfd1.0, GET_PARAMETER, SET_PARAMETER\"\r\nCSeq: 1\r\n"},
	{ RTSP_RECV , RTSP_RECV_M2_REQUEST , "OPTIONS * RTSP/1.0\r\nRequire: org.wfa.wfd1.0\r\nCSeq: %s"},
	{ RTSP_SEND , RTSP_SEND_M2_RESPONSE , "RTSP/1.0 200 OK\r\nCSeq: %s\r\nPublic: org.wfa.wfd1.0, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER\r\n" },
	{ RTSP_SEND , RTSP_SEND_M3_REQUEST , "GET_PARAMETER rtsp://localhost/wfd1.0 RTSP/1.0\r\nCSeq: 2\r\nContent-Type: text/parameters\r\nContent-Length: 211\r\n\r\nwfd_video_formats\r\nwfd_audio_codecs\r\nwfd_uibc_capability\r\nwfd_client_rtp_ports\r\nwfd_content_protection\r\nwfd_sec_screensharing\r\nwfd_sec_portrait_display\r\nwfd_sec_rotation\r\nwfd_sec_hw_rotation\r\nwfd_sec_framerate\r\n" },
	{ RTSP_RECV , RTSP_RECV_M3_RESPONSE , "RTSP/1.0 200 OK\r\nContent-Length: 210\r\nContent-Type: text/parameters\r\nCSeq: 2\r\n\r\nwfd_content_protection: none\r\nwfd_video_formats: 00 00 03 10 0001ffff 1fffffff 00001fff 00 0000 0000 10 none none\r\nwfd_audio_codecs: AAC 00000007 00\r\nwfd_client_rtp_ports: RTP/AVP/UDP;unicast 1991 0 mode=play\r\n" },
	{ RTSP_SEND , RTSP_SEND_M4_REQUEST , "SET_PARAMETER rtsp://localhost/wfd1.0 RTSP/1.0\r\nCSeq: 3\r\nContent-Type: text/parameters\r\nContent-Length: 246\r\n\r\nwfd_video_formats: 00 00 02 04 00000080 00000000 00000000 00 0000 0000 00 none none\r\nwfd_audio_codecs: AAC 00000001 00\r\nwfd_presentation_URL: rtsp://192.168.49.1/wfd1.0/streamid=0 none\r\nwfd_client_rtp_ports: RTP/AVP/UDP;unicast 1990 0 mode=play\r\n" },
	{ RTSP_RECV , RTSP_RECV_M4_RESPONSE , "RTSP/1.0 200 OK\r\nCSeq: 3\r\n" },
	{ RTSP_SEND , RTSP_SEND_M5_REQUEST , "SET_PARAMETER rtsp://localhost/wfd1.0 RTSP/1.0\r\nCSeq: 4\r\nContent-Type: text/parameters\r\nContent-Length: 27\r\n\r\nwfd_trigger_method: SETUP\r\n" },
	{ RTSP_RECV , RTSP_RECV_M5_RESPONSE , "RTSP/1.0 200 OK\r\nCSeq: 4\r\n" },
	{ RTSP_RECV , RTSP_RECV_M6_REQUEST , "SETUP rtsp://192.168.49.1/wfd1.0/streamid=0 RTSP/1.0\r\nTransport: RTP/AVP/UDP;unicast;client_port=1990\r\nCSeq: %s\r\n"},
	{ RTSP_SEND , RTSP_SEND_M6_RESPONSE , "RTSP/1.0 200 OK\r\nCSeq: %s\r\nSession: 1804289383;timeout=30\r\nTransport: RTP/AVP/UDP;unicast;client_port=1991-1992;server_port=19000-19001\r\n" },
	{ RTSP_RECV , RTSP_RECV_M7_REQUEST , "RTSP/1.0 200 OK\r\nCSeq: %s\r\nSession: 1804289383;timeout=30\r\nRange: npt=now-\r\n" },
	{ RTSP_SEND , RTSP_SEND_M7_RESPONSE , "RTSP/1.0 200 OK\r\nCSeq: %s\r\nSession: 1804289383;timeout=30\r\nRange: npt=now-\r\n" },
	{ RTSP_SEND , RTSP_SEND_M16_REQUEST , "GET_PARAMETER rtsp://localhost/wfd1.0 RTSP/1.0\r\nCSeq: 5\r\nSession: 1804289383\r\n"},
	{ RTSP_RECV , RTSP_RECV_M16_RESPONSE , "RTSP/1.0 200 OK\r\nCSeq: 5\r\n" },
	{ RTSP_SEND , RTSP_SEND_TEARDOWN_REQUEST , "SET_PARAMETER rtsp://localhost/wfd1.0 RTSP/1.0\r\nCSeq: 6\r\nContent-Type: text/parameters\r\nContent-Length: 30\r\n\r\nwfd_trigger_method: TEARDOWN\r\n" },
	{ RTSP_RECV , RTSP_RECV_TEADOWN_RESPONSE , "RTSP/1.0 200 OK\r\nCSeq: 6\r\n" }
};

int default_rtsp_srcMsgSize = static_cast<int>(sizeof(default_rtsp_srcMsgbuffer) / sizeof(default_rtsp_srcMsgbuffer[0]));

const char* get_RequestResponseFormat(RTSP_MSG_HANDLER_FORMAT *pstRTSPSrcHldrFmt , size_t rtsp_msg_fmt_count , RTSP_SRCMSG_REQ_RESP format_type )
{
    int index = static_cast<RTSP_SRCMSG_REQ_RESP>(format_type) - static_cast<RTSP_SRCMSG_REQ_RESP>(RTSP_SEND_M1_REQUEST);
    if (index >= 0 && index < static_cast<int>(rtsp_msg_fmt_count))
    {
        return pstRTSPSrcHldrFmt[index].template_name;
    }
    return "";
}

std::string parse_received_parser_field_value(std::string rtsp_msg_buffer , const char* given_tag )
{
    std::string seq_str = "";
    std::stringstream ss(rtsp_msg_buffer);
    std::string prefix = "";
    std::string line;
    MIRACASTLOG_TRACE("Entering...");

    while (std::getline(ss, line))
    {
        if (line.find(given_tag) != std::string::npos)
        {
            prefix = given_tag;
            seq_str = line.substr(prefix.length());
            REMOVE_R(seq_str);
            REMOVE_N(seq_str);
            break;
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
    return seq_str;
}

void send_rtsp_msg( int sockfd , std::string msg_buffer )
{
    printf("[%d:%s] Entering...\n",__LINE__,__FUNCTION__);
    printf("[%d:%s] Send Msg[%lu][%s]...\n",__LINE__,__FUNCTION__,msg_buffer.size(),msg_buffer.c_str());
    send(sockfd, msg_buffer.c_str(), msg_buffer.size(), 0);
    printf("[%d:%s] Exiting...\n",__LINE__,__FUNCTION__);
}

bool recv_rtsp_msg( int sockfd , char* recv_msg_buffer , size_t recv_msg_buffer_size )
{
    bool return_value = false;
    printf("[%d:%s] Entering...\n",__LINE__,__FUNCTION__);
    return_value = read(sockfd, recv_msg_buffer , recv_msg_buffer_size );
    printf("[%d:%s] Recv Msg[%lu][%x][%s]...\n",__LINE__,__FUNCTION__,strlen(recv_msg_buffer),return_value,recv_msg_buffer);
    printf("[%d:%s] Exiting...\n",__LINE__,__FUNCTION__);
    return return_value;
}

void runRTSPSourceHandler(RTSP_MSG_HANDLER_FORMAT* custom_src_msg_buffer, size_t rtsp_msg_count , std::string & response_buffer )
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    int opt = 1;
    int addrlen = sizeof(server_addr);
    char buffer[BUFFER_SIZE] = {0};

    response_buffer = "FAIL";

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
	return;
    }

    // Forcefully attaching socket to the port 7236
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
	close(server_fd);
	return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket to localhost port 7236
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
	close(server_fd);
	return;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
	close(server_fd);
	return;
    }

    // Accept an incoming connection
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen)) < 0) {
        perror("accept");
	close(server_fd);
	return;
    }

    response_buffer = "SUCCESS";

    printf("[%d:%s]socket accept done\n",__LINE__,__FUNCTION__);
    std::string receivedCSeqNum = "";
    size_t current_msg = 0;

    while ( current_msg < rtsp_msg_count )
    {
	    RTSP_SRCMSG_REQ_RESP rtsp_msg_type = custom_src_msg_buffer[current_msg].rtsp_msg_type;
	    int rtsp_sendorreceive = custom_src_msg_buffer[current_msg].rtsp_sendorreceive;
	    const char* rtsp_req_resp_format = get_RequestResponseFormat(custom_src_msg_buffer,rtsp_msg_count,rtsp_msg_type);
	    std::string msg_buffer = "";

	    memset( buffer , 0x00 , sizeof(buffer));

	    if ( RTSP_SEND == rtsp_sendorreceive )
	    {
		switch (rtsp_msg_type)
		{
			case RTSP_SEND_M1_REQUEST:
			case RTSP_SEND_M3_REQUEST:
			case RTSP_SEND_M4_REQUEST:
			case RTSP_SEND_M5_REQUEST:
			case RTSP_SEND_M16_REQUEST:
			case RTSP_SEND_TEARDOWN_REQUEST:
			{
				msg_buffer = rtsp_req_resp_format;
			}
			break;
			case RTSP_SEND_M2_RESPONSE:
			case RTSP_SEND_M6_RESPONSE:
			case RTSP_SEND_M7_RESPONSE:
			case RTSP_SEND_TEARDOWN_RESPONSE:
			{
				std::string temp_buffer = rtsp_req_resp_format;

				if (temp_buffer.find("%s") != std::string::npos)
				{
					sprintf( buffer , rtsp_req_resp_format , receivedCSeqNum.c_str());
					msg_buffer = buffer;
					printf("[%d:%s] Response sequence number replaced as [%s]\n",__LINE__,__FUNCTION__,receivedCSeqNum.c_str());
					receivedCSeqNum.clear();
				}
				else
				{
					msg_buffer = rtsp_req_resp_format;
				}
			}
			break;
			default:
			{

			}
			break;
		}
		if ( !msg_buffer.empty())
		{
			send_rtsp_msg( client_fd , msg_buffer );
		}
	    }
	    else
	    {
		bool status = recv_rtsp_msg( client_fd , buffer , sizeof(buffer));
		msg_buffer = buffer;
		if ((msg_buffer.find("TEARDOWN") == 0) || (false == status ))
		{
			printf("TEARDOWN initiated from Sink Device\n");
			response_buffer = "SUCCESS";
			break;
		}
		switch (rtsp_msg_type)
                {
                        case RTSP_RECV_M1_RESPONSE:
			case RTSP_RECV_M4_RESPONSE:
			case RTSP_RECV_M5_RESPONSE:
			case RTSP_RECV_M16_RESPONSE:
			case RTSP_RECV_TEADOWN_RESPONSE:
			{
                                msg_buffer = rtsp_req_resp_format;
				if ( 0 != strncmp( buffer , rtsp_req_resp_format , strlen(rtsp_req_resp_format)))
				{
					response_buffer = "FAIL";
					printf("[%d:%s] expected[%s] actual[%s]\n",__LINE__,__FUNCTION__,rtsp_req_resp_format,buffer);
				}
			}
			break;
			case RTSP_RECV_M3_RESPONSE:
			{
				std::string expected_sequence_number = parse_received_parser_field_value( msg_buffer , "CSeq: " ),
					actual_sequence_number = parse_received_parser_field_value( rtsp_req_resp_format , "CSeq: " );

				if ( 0 != expected_sequence_number.compare(actual_sequence_number))
				{
					response_buffer = "FAIL";
					printf("[%d:%s] expected[%s]actual[%s]\n",__LINE__,__FUNCTION__,expected_sequence_number,actual_sequence_number);
				}
			}
			break;
                        case RTSP_RECV_M2_REQUEST:
			case RTSP_RECV_M6_REQUEST:
			case RTSP_RECV_M7_REQUEST:
                        {
				receivedCSeqNum = parse_received_parser_field_value( msg_buffer , "CSeq: " );
                        }
                        break;
			default:
			{
				break;
			}
                }
	    }
	    ++current_msg;

	    if ( 0 == response_buffer.compare("FAIL"))
	    {
		    break;
	    }
    }
    close(client_fd);
    close(server_fd);

    if ( 0 == response_buffer.compare("FAIL"))
    {
	    printf("[%d:%s] It is FAILED\n",__LINE__,__FUNCTION__);
    }
    else
    {
	    printf("[%d:%s] It is SUCCESS\n",__LINE__,__FUNCTION__);
    }

}
}

class MiracastPlayerTest : public ::testing::Test {
	protected:
		Core::ProxyType<Plugin::MiracastPlayer> plugin;
		Core::JSONRPC::Handler& handler;
		Core::JSONRPC::Connection connection;
		string response;
		ServiceMock service;
		NiceMock<WrapsImplMock> wrapsImplMock;

		MiracastPlayerTest()
			: plugin(Core::ProxyType<Plugin::MiracastPlayer>::Create())
			  , handler(*(plugin))
			  , connection(1, 0)
{
	Wraps::getInstance().impl = &wrapsImplMock;

	EXPECT_CALL(service, QueryInterfaceByCallsign(::testing::_, ::testing::_))
		.Times(::testing::AnyNumber())
		.WillRepeatedly(::testing::Invoke(
					[&](const uint32_t, const string& name) -> void* {
					return nullptr;
					}));
	EXPECT_EQ(string(""), plugin->Initialize(&service));
}
virtual ~MiracastPlayerTest() override
{
	plugin->Deinitialize(nullptr);
	WpaCtrl::getInstance().impl = nullptr;
}
};

TEST_F(MiracastPlayerTest, RegisteredMethods)
{
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("playRequest")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("stopRequest")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setPlayerState")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setVideoRectangle")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setLogging")));
}

TEST_F(MiracastPlayerTest, playRequest)
{
	std::string rtsp_response = "";
	std::thread serverThread = std::thread([&]() { runRTSPSourceHandler( default_rtsp_srcMsgbuffer , default_rtsp_srcMsgSize , rtsp_response ); });
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("playRequest"), _T("{\"device_parameters\": {\"source_dev_ip\":\"127.0.0.1\",\"source_dev_mac\": \"A1:B2:C3:D4:E5:F6\",\"source_dev_name\":\"Sample-Android-Test-1\",\"sink_dev_ip\":\"192.168.59.1\"},\"video_rectangle\": {\"X\": 0,\"Y\" : 0,\"W\": 1920,\"H\": 1080}}"), response));
	EXPECT_EQ(response, string("{\"success\":true}"));
	serverThread.join();
	EXPECT_EQ(rtsp_response, string("SUCCESS"));
}

TEST_F(MiracastPlayerTest, stopRequest)
{
	std::string rtsp_response = "";
	std::thread serverThread = std::thread([&]() { runRTSPSourceHandler( default_rtsp_srcMsgbuffer , default_rtsp_srcMsgSize , rtsp_response ); });
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("playRequest"), _T("{\"device_parameters\": {\"source_dev_ip\":\"127.0.0.1\",\"source_dev_mac\": \"A1:B2:C3:D4:E5:F6\",\"source_dev_name\":\"Sample-Android-Test-1\",\"sink_dev_ip\":\"192.168.59.1\"},\"video_rectangle\": {\"X\": 0,\"Y\" : 0,\"W\": 1920,\"H\": 1080}}"), response));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("stopRequest"), _T("{\"reason_code\": 300}"), response));
	EXPECT_EQ(response, string("{\"success\":true}"));
	serverThread.join();
	EXPECT_EQ(rtsp_response, string("SUCCESS"));
}

TEST_F(MiracastPlayerTest, InvalidstopRequest)
{
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("playRequest"), _T("{\"device_parameters\": {\"source_dev_ip\":\"127.0.0.1\",\"source_dev_mac\": \"A1:B2:C3:D4:E5:F6\",\"source_dev_name\":\"Sample-Android-Test-1\",\"sink_dev_ip\":\"192.168.59.1\"},\"video_rectangle\": {\"X\": 0,\"Y\" : 0,\"W\": 1920,\"H\": 1080}}"), response));
	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("stopRequest"), _T("{\"reason_code\": 303}"), response));
}

TEST_F(MiracastPlayerTest, setVideoRectangle)
{
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("playRequest"), _T("{\"device_parameters\": {\"source_dev_ip\":\"127.0.0.1\",\"source_dev_mac\": \"A1:B2:C3:D4:E5:F6\",\"source_dev_name\":\"Sample-Android-Test-1\",\"sink_dev_ip\":\"192.168.59.1\"}}"), response));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setVideoRectangle"), _T("{\"X\": 0,\"Y\": 0,\"W\": 1280,\"H\": 720}"), response));
	EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(MiracastPlayerTest, setPlayerState)
{
	std::string rtsp_response = "";
	std::thread serverThread = std::thread([&]() { runRTSPSourceHandler( default_rtsp_srcMsgbuffer , default_rtsp_srcMsgSize , rtsp_response ); });
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("playRequest"), _T("{\"device_parameters\": {\"source_dev_ip\":\"127.0.0.1\",\"source_dev_mac\": \"A1:B2:C3:D4:E5:F6\",\"source_dev_name\":\"Sample-Android-Test-1\",\"sink_dev_ip\":\"192.168.59.1\"},\"video_rectangle\": {\"X\": 0,\"Y\" : 0,\"W\": 1920,\"H\": 1080}}"), response));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setPlayerState"), _T("{\"state\": \"PAUSE\"}"), response));
	EXPECT_EQ(response, string("{\"success\":true}"));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setPlayerState"), _T("{\"state\": \"PLAY\"}"), response));
	EXPECT_EQ(response, string("{\"success\":true}"));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setPlayerState"), _T("{\"state\": \"STOP\"}"), response));
	EXPECT_EQ(response, string("{\"success\":true}"));
	serverThread.join();
	EXPECT_EQ(rtsp_response, string("SUCCESS"));
}

