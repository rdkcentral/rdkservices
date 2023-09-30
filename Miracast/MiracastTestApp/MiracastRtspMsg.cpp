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

#include <MiracastRtspMsg.h>

MiracastRTSPMsg *MiracastRTSPMsg::m_rtsp_msg_obj{nullptr};
static std::string empty_string = "";

void RTSPMsgHandlerCallback(void *args);

#ifdef ENABLE_HDCP2X_SUPPORT
void HDCPHandlerCallback(void *args);
#endif

RTSP_MSG_TEMPLATE_INFO MiracastRTSPMsg::rtsp_msg_template_info[] = {
    {RTSP_MSG_FMT_M1_RESPONSE, "RTSP/1.0 200 OK\r\nPublic: \"%s, GET_PARAMETER, SET_PARAMETER\"\r\nCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_M2_REQUEST, "OPTIONS * RTSP/1.0\r\nRequire: %s\r\nCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_M3_RESPONSE, "RTSP/1.0 200 OK\r\nContent-Length: %s\r\nContent-Type: text/parameters\r\nCSeq: %s\r\n\r\n%s"},
    {RTSP_MSG_FMT_M4_RESPONSE, "RTSP/1.0 200 OK\r\nCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_M5_RESPONSE, "RTSP/1.0 200 OK\r\nCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_M6_REQUEST, "SETUP %s RTSP/1.0\r\nTransport: %s;%sclient_port=%s\r\nCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_M7_REQUEST, "PLAY %s RTSP/1.0\r\nSession: %s\r\nCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_M16_RESPONSE, "RTSP/1.0 200 OK\r\nCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_PAUSE_REQUEST, "PAUSE %s RTSP/1.0\r\nSession: %s\r\nCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_PLAY_REQUEST, "PLAY %s RTSP/1.0\r\nSession: %s\r\nCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_TEARDOWN_REQUEST, "TEARDOWN %s RTSP/1.0\r\nSession: %s\r\nCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_TEARDOWN_RESPONSE, "RTSP/1.0 200 OK\r\nCSeq: %s\r\n\r\n"}};

MiracastRTSPMsg *MiracastRTSPMsg::getInstance(MiracastError &error_code , MiracastThread *controller_thread_id)
{
    MIRACASTLOG_TRACE("Entering...");
    error_code = MIRACAST_OK;

    if (nullptr == m_rtsp_msg_obj)
    {
        m_rtsp_msg_obj = new MiracastRTSPMsg();
        if (nullptr != m_rtsp_msg_obj)
        {
            if ( MIRACAST_OK != m_rtsp_msg_obj->create_RTSPThread()){
                error_code = MIRACAST_RTSP_INIT_FAILED;
                delete m_rtsp_msg_obj;
                m_rtsp_msg_obj = nullptr;
            }
            else{
                m_rtsp_msg_obj->m_controller_thread = controller_thread_id;
            }
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
    return m_rtsp_msg_obj;
}

void MiracastRTSPMsg::destroyInstance()
{
    MIRACASTLOG_TRACE("Entering...");

    if (nullptr != m_rtsp_msg_obj)
    {
        delete m_rtsp_msg_obj;
        m_rtsp_msg_obj = nullptr;
    }
    MIRACASTLOG_TRACE("Exiting...");
}

MiracastRTSPMsg::MiracastRTSPMsg()
{
    RTSP_WFD_VIDEO_FMT_STRUCT st_video_fmt = {0};
    RTSP_WFD_AUDIO_FMT_STRUCT st_audio_fmt = { RTSP_UNSUPPORTED_AUDIO_FORMAT , 0 };
    std::string default_configuration;

    MIRACASTLOG_TRACE("Entering...");
    m_tcpSockfd = -1;
    m_current_sequence_number.clear();

    set_WFDEnableDisableUnicast(true);

    st_video_fmt.preferred_display_mode_supported = RTSP_PREFERED_DISPLAY_NOT_SUPPORTED;
    st_video_fmt.st_h264_codecs.profile = RTSP_PROFILE_BMP_CHP_SUPPORTED;
    st_video_fmt.st_h264_codecs.level = RTSP_H264_LEVEL_4_BITMAP;
    st_video_fmt.st_h264_codecs.cea_mask = RTSP_CEA_RESOLUTION_720x480p60
                                            | RTSP_CEA_RESOLUTION_720x576p50
                                            | RTSP_CEA_RESOLUTION_1280x720p30
                                            | RTSP_CEA_RESOLUTION_1280x720p60
                                            | RTSP_CEA_RESOLUTION_1920x1080p30
                                            | RTSP_CEA_RESOLUTION_1920x1080p60;
    st_video_fmt.st_h264_codecs.video_frame_rate_change_support = true;

    st_audio_fmt.audio_format = RTSP_AAC_AUDIO_FORMAT;
    st_audio_fmt.modes = RTSP_AAC_CH2_48kHz|RTSP_AAC_CH4_48kHz|RTSP_AAC_CH6_48kHz;

    set_WFDVideoFormat(st_video_fmt);
    set_WFDAudioCodecs(st_audio_fmt);

    default_configuration = RTSP_DFLT_CONTENT_PROTECTION;
    set_WFDContentProtection(default_configuration);

    default_configuration = RTSP_DFLT_TRANSPORT_PROFILE;
    set_WFDTransportProfile(default_configuration);

    default_configuration = RTSP_DFLT_STREAMING_PORT;
    set_WFDStreamingPortNumber(default_configuration);

    default_configuration = RTSP_DFLT_CLIENT_RTP_PORTS;
    set_WFDClientRTPPorts(default_configuration);

    MIRACASTLOG_TRACE("Exiting...");
}
MiracastRTSPMsg::~MiracastRTSPMsg()
{
    if ( nullptr != m_rtsp_msg_handler_thread ){
        delete m_rtsp_msg_handler_thread;
        m_rtsp_msg_handler_thread = nullptr;
    }

    MIRACASTLOG_TRACE("Entering...");
    if (-1 != m_tcpSockfd){
        close(m_tcpSockfd);
        m_tcpSockfd = -1;
    }
    MIRACASTLOG_TRACE("Exiting...");
}

MiracastError MiracastRTSPMsg::create_RTSPThread(void)
{
    MiracastError error_code = MIRACAST_FAIL;
    MIRACASTLOG_TRACE("Entering...");

    m_rtsp_msg_handler_thread = new MiracastThread( RTSP_HANDLER_THREAD_NAME,
                                                    RTSP_HANDLER_THREAD_STACK,
                                                    RTSP_HANDLER_MSG_COUNT,
                                                    RTSP_HANDLER_MSGQ_SIZE,
                                                    reinterpret_cast<void (*)(void *)>(&RTSPMsgHandlerCallback),
                                                    this);
    if (nullptr != m_rtsp_msg_handler_thread)
    {
        error_code = m_rtsp_msg_handler_thread->start();

        if ( MIRACAST_OK != error_code ){
            delete m_rtsp_msg_handler_thread;
            m_rtsp_msg_handler_thread = nullptr;
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
    return error_code;
}

std::string MiracastRTSPMsg::get_WFDVideoFormat(void)
{
    return m_wfd_video_formats;
}

std::string MiracastRTSPMsg::get_WFDAudioCodecs(void)
{
    return m_wfd_audio_codecs;
}

std::string MiracastRTSPMsg::get_WFDClientRTPPorts(void)
{
    return m_wfd_client_rtp_ports;
}

std::string MiracastRTSPMsg::get_WFDUIBCCapability(void)
{
    return m_wfd_uibc_capability;
}

std::string MiracastRTSPMsg::get_WFDContentProtection(void)
{
    return m_wfd_content_protection;
}

std::string MiracastRTSPMsg::get_WFDSecScreenSharing(void)
{
    return m_wfd_sec_screensharing;
}

std::string MiracastRTSPMsg::get_WFDPortraitDisplay(void)
{
    return m_wfd_sec_portrait_display;
}

std::string MiracastRTSPMsg::get_WFDSecRotation(void)
{
    return m_wfd_sec_rotation;
}

std::string MiracastRTSPMsg::get_WFDSecHWRotation(void)
{
    return m_wfd_sec_hw_rotation;
}

std::string MiracastRTSPMsg::get_WFDSecFrameRate(void)
{
    return m_wfd_sec_framerate;
}

std::string MiracastRTSPMsg::get_WFDPresentationURL(void)
{
    return m_wfd_presentation_URL;
}

std::string MiracastRTSPMsg::get_WFDTransportProfile(void)
{
    return m_wfd_transport_profile;
}

std::string MiracastRTSPMsg::get_WFDStreamingPortNumber(void)
{
    return m_wfd_streaming_port;
}

bool MiracastRTSPMsg::IsWFDUnicastSupported(void)
{
    return m_is_unicast;
}
std::string MiracastRTSPMsg::get_CurrentWFDSessionNumber(void)
{
    return m_wfd_session_number;
}

bool MiracastRTSPMsg::set_WFDVideoFormat(RTSP_WFD_VIDEO_FMT_STRUCT st_video_fmt)
{
    char video_format_buffer[256] = {0};
    uint8_t video_frame_control_support = 0x00;

    MIRACASTLOG_TRACE("Entering...");
    memset(&m_wfd_video_formats_st , 0x00 , sizeof(RTSP_WFD_VIDEO_FMT_STRUCT));
    m_wfd_video_formats.clear();

    if ((RTSP_CEA_RESOLUTION_UNSUPPORTED_MASK & st_video_fmt.st_h264_codecs.cea_mask)||
        (RTSP_VESA_RESOLUTION_UNSUPPORTED_MASK & st_video_fmt.st_h264_codecs.vesa_mask)||
        (RTSP_HH_RESOLUTION_UNSUPPORTED_MASK & st_video_fmt.st_h264_codecs.hh_mask))
    {
        MIRACASTLOG_ERROR("Invalid video format[%#08X][%#08X][%#08X]...\n",
                            st_video_fmt.st_h264_codecs.cea_mask,
                            st_video_fmt.st_h264_codecs.vesa_mask,
                            st_video_fmt.st_h264_codecs.hh_mask);
        MIRACASTLOG_TRACE("Exiting...");
        return false;
    }
    memcpy(&m_wfd_video_formats_st , &st_video_fmt , sizeof(RTSP_WFD_VIDEO_FMT_STRUCT));

    // Set the 0th bit to 1
    if (st_video_fmt.st_h264_codecs.video_frame_skip_support){
        video_frame_control_support |= 0x01; // 0th bit set
    }

    // Set the 4th bit to 1
    if (st_video_fmt.st_h264_codecs.video_frame_rate_change_support){
        video_frame_control_support |= 0x10; // 4th bit set
    }

    // Set the 1st to 3rd bits based on the value of skip_intervals
    video_frame_control_support |= ((0x07 & st_video_fmt.st_h264_codecs.max_skip_intervals) << 1); // 1:3 bits for intervals

    sprintf( video_format_buffer , 
                "%02x %02x %02x %02x %08x %08x %08x %02x %04x %04x %02x ",
                st_video_fmt.native,
                st_video_fmt.preferred_display_mode_supported,
                st_video_fmt.st_h264_codecs.profile,
                st_video_fmt.st_h264_codecs.level,
                st_video_fmt.st_h264_codecs.cea_mask,
                st_video_fmt.st_h264_codecs.vesa_mask,
                st_video_fmt.st_h264_codecs.hh_mask,
                st_video_fmt.st_h264_codecs.latency,
                st_video_fmt.st_h264_codecs.min_slice,
                st_video_fmt.st_h264_codecs.slice_encode,
                video_frame_control_support );
    m_wfd_video_formats = video_format_buffer;

    if (( -1 == st_video_fmt.st_h264_codecs.max_hres )||
        ( -1 == st_video_fmt.st_h264_codecs.max_vres )||
        ( 0 == st_video_fmt.preferred_display_mode_supported))
    {
        m_wfd_video_formats.append("none");
        m_wfd_video_formats.append(" ");
        m_wfd_video_formats.append("none");
    }
    else{
        memset( video_format_buffer , 0x00 , sizeof(video_format_buffer));
        sprintf( video_format_buffer , 
                    "%04x %04x",
                    st_video_fmt.st_h264_codecs.max_hres,
                    st_video_fmt.st_h264_codecs.max_vres );
        m_wfd_video_formats.append(video_format_buffer);
    }

    MIRACASTLOG_VERBOSE("video format[%s]...\n",m_wfd_video_formats.c_str());
    MIRACASTLOG_TRACE("Exiting...");
    return true;
}

bool MiracastRTSPMsg::set_WFDAudioCodecs( RTSP_WFD_AUDIO_FMT_STRUCT st_audio_fmt )
{
    char audio_format_buffer[256] = {0};
    std::string audio_format_str = "";

    MIRACASTLOG_TRACE("Entering...");

    memset(&m_wfd_audio_formats_st , 0x00 , sizeof(RTSP_WFD_AUDIO_FMT_STRUCT));
    m_wfd_audio_codecs.clear();

    if (((0 != st_audio_fmt.audio_format) &&
        (RTSP_UNSUPPORTED_AUDIO_FORMAT < st_audio_fmt.audio_format))||
        ((RTSP_LPCM_AUDIO_FORMAT == st_audio_fmt.audio_format) &&
        (RTSP_LPCM_UNSUPPORTED_MASK & st_audio_fmt.modes))||
        ((RTSP_AAC_AUDIO_FORMAT == st_audio_fmt.audio_format) &&
        (RTSP_AAC_UNSUPPORTED_MASK & st_audio_fmt.modes))||
        ((RTSP_AC3_AUDIO_FORMAT == st_audio_fmt.audio_format) &&
        (RTSP_AC3_UNSUPPORTED_MASK & st_audio_fmt.modes)))
    {
        MIRACASTLOG_ERROR("Invalid audio format/mode[%#08X/%#08X]...\n",st_audio_fmt.modes,st_audio_fmt.modes);
        MIRACASTLOG_TRACE("Exiting...");
        return false;
    }

    switch (st_audio_fmt.audio_format)
    {
        case RTSP_LPCM_AUDIO_FORMAT:
        {
            audio_format_str = "LPCM";
        }
        break;
        case RTSP_AAC_AUDIO_FORMAT:
        {
            audio_format_str = "AAC";
        }
        break;
        case RTSP_AC3_AUDIO_FORMAT:
        {
            audio_format_str = "AC3";
        }
        break;
        default:
        {
            MIRACASTLOG_ERROR("unknown audio format[%#08X]...\n",st_audio_fmt.audio_format);
            MIRACASTLOG_TRACE("Exiting...");
            return false;
        }
        break;
    }
    memcpy(&m_wfd_audio_formats_st , &st_audio_fmt , sizeof(RTSP_WFD_AUDIO_FMT_STRUCT));

    sprintf( audio_format_buffer , 
                "%s %08x %02x",
                audio_format_str.c_str(),
                st_audio_fmt.modes,
                st_audio_fmt.latency);
    m_wfd_audio_codecs = audio_format_buffer;

    MIRACASTLOG_VERBOSE("audio format[%s]...\n",m_wfd_audio_codecs.c_str());
    MIRACASTLOG_TRACE("Exiting...");
    return true;
}

bool MiracastRTSPMsg::set_WFDClientRTPPorts(std::string client_rtp_ports)
{
    m_wfd_client_rtp_ports = client_rtp_ports;
    return true;
}

bool MiracastRTSPMsg::set_WFDUIBCCapability(std::string uibc_caps)
{
    m_wfd_uibc_capability = uibc_caps;
    return true;
}

bool MiracastRTSPMsg::set_WFDContentProtection(std::string content_protection)
{
    m_wfd_content_protection = content_protection;
    return true;
}

bool MiracastRTSPMsg::set_WFDSecScreenSharing(std::string screen_sharing)
{
    m_wfd_sec_screensharing = screen_sharing;
    return true;
}

bool MiracastRTSPMsg::set_WFDPortraitDisplay(std::string portrait_display)
{
    m_wfd_sec_portrait_display = portrait_display;
    return true;
}

bool MiracastRTSPMsg::set_WFDSecRotation(std::string rotation)
{
    m_wfd_sec_rotation = rotation;
    return true;
}

bool MiracastRTSPMsg::set_WFDSecHWRotation(std::string hw_rotation)
{
    m_wfd_sec_hw_rotation = hw_rotation;
    return true;
}

bool MiracastRTSPMsg::set_WFDSecFrameRate(std::string framerate)
{
    m_wfd_sec_framerate = framerate;
    return true;
}

bool MiracastRTSPMsg::set_WFDPresentationURL(std::string URL)
{
    m_wfd_presentation_URL = URL;
    return true;
}

bool MiracastRTSPMsg::set_WFDTransportProfile(std::string profile)
{
    m_wfd_transport_profile = profile;
    return true;
}

bool MiracastRTSPMsg::set_WFDStreamingPortNumber(std::string port_number)
{
    m_wfd_streaming_port = port_number;
    return true;
}

bool MiracastRTSPMsg::set_WFDEnableDisableUnicast(bool enable_disable_unicast)
{
    m_is_unicast = enable_disable_unicast;
    return true;
}

bool MiracastRTSPMsg::set_WFDSessionNumber(std::string session)
{
    m_wfd_session_number = session;
    return true;
}

const char* MiracastRTSPMsg::get_RequestResponseFormat(RTSP_MSG_FMT_SINK2SRC format_type)
{
    int index = static_cast<RTSP_MSG_FMT_SINK2SRC>(format_type) - static_cast<RTSP_MSG_FMT_SINK2SRC>(RTSP_MSG_FMT_M1_RESPONSE);
    if (index >= 0 && index < static_cast<int>(sizeof(rtsp_msg_template_info) / sizeof(rtsp_msg_template_info[0])))
    {
        return rtsp_msg_template_info[index].template_name;
    }
    return "";
}

std::string MiracastRTSPMsg::generate_request_response_msg(RTSP_MSG_FMT_SINK2SRC msg_fmt_needed, std::string received_session_no, std::string append_data1)
{
    MIRACASTLOG_TRACE("Entering...");
    std::vector<const char *> sprintf_args;
    const char *template_str = get_RequestResponseFormat(msg_fmt_needed);
    std::string content_buffer = "";
    std::string unicast_supported = "";
    std::string content_buffer_len;
    std::string sequence_number = get_RequestSequenceNumber();
    std::string URL = get_WFDPresentationURL();
    std::string TSProfile = get_WFDTransportProfile();
    std::string StreamingPort = get_WFDStreamingPortNumber();
    std::string WFDSessionNum = get_CurrentWFDSessionNumber();

    // Determine the required buffer size using snprintf
    switch (msg_fmt_needed)
    {
    case RTSP_MSG_FMT_M1_RESPONSE:
    {
        sprintf_args.push_back(append_data1.c_str());
        sprintf_args.push_back(received_session_no.c_str());
    }
    break;
    case RTSP_MSG_FMT_M3_RESPONSE:
    {
        // prepare content buffer
        // Append content protection type
        content_buffer.append(RTSP_WFD_CONTENT_PROTECT_FIELD);
        content_buffer.append(get_WFDContentProtection());
        content_buffer.append(RTSP_CRLF_STR);
        // Append Video Formats
        content_buffer.append(RTSP_WFD_VIDEO_FMT_FIELD);
        content_buffer.append(get_WFDVideoFormat());
        content_buffer.append(RTSP_CRLF_STR);
        // Append Audio Formats
        content_buffer.append(RTSP_WFD_AUDIO_FMT_FIELD);
        content_buffer.append(get_WFDAudioCodecs());
        content_buffer.append(RTSP_CRLF_STR);
        // Append Client RTP Client port configuration
        content_buffer.append(RTSP_WFD_CLIENT_PORTS_FIELD);
        content_buffer.append(get_WFDClientRTPPorts());
        content_buffer.append(RTSP_CRLF_STR);

        content_buffer_len = std::to_string(content_buffer.length());

        sprintf_args.push_back(content_buffer_len.c_str());
        sprintf_args.push_back(received_session_no.c_str());
        sprintf_args.push_back(content_buffer.c_str());

        MIRACASTLOG_INFO("content_buffer - [%s]\n", content_buffer.c_str());
    }
    break;
    case RTSP_MSG_FMT_M4_RESPONSE:
    case RTSP_MSG_FMT_M5_RESPONSE:
    case RTSP_MSG_FMT_M16_RESPONSE:
    case RTSP_MSG_FMT_TEARDOWN_RESPONSE:
    {
        sprintf_args.push_back(received_session_no.c_str());
    }
    break;
    case RTSP_MSG_FMT_M2_REQUEST:
    case RTSP_MSG_FMT_M6_REQUEST:
    case RTSP_MSG_FMT_M7_REQUEST:
    case RTSP_MSG_FMT_PAUSE_REQUEST:
    case RTSP_MSG_FMT_PLAY_REQUEST:
    case RTSP_MSG_FMT_TEARDOWN_REQUEST:
    {
        if (RTSP_MSG_FMT_M2_REQUEST == msg_fmt_needed)
        {
            sprintf_args.push_back(append_data1.c_str());
        }
        else
        {
            sprintf_args.push_back(URL.c_str());

            if (RTSP_MSG_FMT_M6_REQUEST == msg_fmt_needed)
            {
                sprintf_args.push_back(TSProfile.c_str());
                if (true == IsWFDUnicastSupported())
                {
                    unicast_supported.append(RTSP_STD_UNICAST_FIELD);
                    unicast_supported.append(RTSP_SEMI_COLON_STR);
                    sprintf_args.push_back(unicast_supported.c_str());
                }
                sprintf_args.push_back(StreamingPort.c_str());
            }
            else
            {
                sprintf_args.push_back(WFDSessionNum.c_str());
            }
        }
        sprintf_args.push_back(sequence_number.c_str());
    }
    break;
    default:
    {
        MIRACASTLOG_ERROR("INVALID FMT REQUEST\n");
    }
    break;
    }

    std::string result = "";

    if (0 != sprintf_args.size())
    {
        result = MiracastRTSPMsg::format_string(template_str, sprintf_args);
    }
    MIRACASTLOG_TRACE("Exiting...");
    return result;
}

std::string MiracastRTSPMsg::get_RequestSequenceNumber(void)
{
    int next_number = std::stoi(m_current_sequence_number.empty() ? "0" : m_current_sequence_number) + 1;
    m_current_sequence_number = std::to_string(next_number);
    return m_current_sequence_number;
}

void MiracastRTSPMsg::set_WFDSourceMACAddress(std::string MAC_Addr)
{
    m_connected_mac_addr = MAC_Addr;
}

void MiracastRTSPMsg::set_WFDSourceName(std::string device_name)
{
    m_connected_device_name = device_name;
}

std::string MiracastRTSPMsg::get_WFDSourceName(void)
{
    return m_connected_device_name;
}

std::string MiracastRTSPMsg::get_WFDSourceMACAddress(void)
{
    return m_connected_mac_addr;
}

void MiracastRTSPMsg::reset_WFDSourceMACAddress(void)
{
    m_connected_mac_addr.clear();
}

void MiracastRTSPMsg::reset_WFDSourceName(void)
{
    m_connected_device_name.clear();
}

/*
 * Wait for data returned by the socket for specified time
 */
bool MiracastRTSPMsg::wait_data_timeout(int m_Sockfd, unsigned int ms)
{
    struct timeval timeout = {0};
    fd_set readFDSet;

    FD_ZERO(&readFDSet);
    FD_SET(m_Sockfd, &readFDSet);

    timeout.tv_sec = (ms / 1000);
    timeout.tv_usec = ((ms % 1000) * 1000);

    if (select(m_Sockfd + 1, &readFDSet, nullptr, nullptr, &timeout) > 0)
    {
        return FD_ISSET(m_Sockfd, &readFDSet);
    }
    return false;
}

RTSP_STATUS MiracastRTSPMsg::receive_buffer_timedOut(int socket_fd, void *buffer, size_t buffer_len , unsigned int wait_time_ms )
{
    int recv_return = -1;
    RTSP_STATUS status = RTSP_MSG_SUCCESS;

    if (!wait_data_timeout(socket_fd, wait_time_ms))
    {
        return RTSP_TIMEDOUT;
    }
    else
    {
        recv_return = recv(socket_fd, buffer, buffer_len, 0);
    }

    if (recv_return <= 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            MIRACASTLOG_ERROR("error: recv timed out\n");
            status = RTSP_TIMEDOUT;
        }
        else
        {
            MIRACASTLOG_ERROR("error: recv failed, or connection closed\n");
            status = RTSP_MSG_FAILURE;
        }
    }
    MIRACASTLOG_INFO("received string(%d) - %s\n", recv_return, buffer);
    return status;
}

MiracastError MiracastRTSPMsg::initiate_TCP(std::string goIP)
{
    MIRACASTLOG_TRACE("Entering...");
    MiracastError ret = MIRACAST_FAIL;
    int r, i, num_ready = 0;
    size_t addr_size = 0;
    struct epoll_event events[MAX_EPOLL_EVENTS];
    struct sockaddr_in addr = {0};
    struct sockaddr_storage str_addr = {0};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(7236);

    if (!goIP.empty()){
        r = inet_pton(AF_INET, goIP.c_str(), &addr.sin_addr);
        if (r != 1)
        {
            MIRACASTLOG_ERROR("inet_issue");
            return MIRACAST_FAIL;
        }
    }
    else{
        addr.sin_addr.s_addr = INADDR_ANY;
    }

    memcpy(&str_addr, &addr, sizeof(addr));
    addr_size = sizeof(addr);

    struct sockaddr_storage in_addr = str_addr;

    if (-1 != m_tcpSockfd){
        close(m_tcpSockfd);
        m_tcpSockfd = -1;
    }

    m_tcpSockfd = socket(in_addr.ss_family, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (m_tcpSockfd < 0)
    {
        MIRACASTLOG_ERROR("TCP Socket creation error %s", strerror(errno));
        return MIRACAST_FAIL;
    }

    /*---Add socket to epoll---*/
    int epfd = epoll_create(1);
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLOUT;
    event.data.fd = m_tcpSockfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, m_tcpSockfd, &event);

    fcntl(m_tcpSockfd, F_SETFL, O_NONBLOCK);
    MIRACASTLOG_INFO("NON_BLOCKING Socket Enabled...\n");

    r = connect(m_tcpSockfd, (struct sockaddr *)&in_addr, addr_size);
    if (r < 0)
    {
        if (errno != EINPROGRESS)
        {
            MIRACASTLOG_INFO("Event %s received(%d)", strerror(errno), errno);
        }
        else
        {
            // connection in progress
            if (!wait_data_timeout(m_tcpSockfd, SOCKET_DFLT_WAIT_TIMEOUT))
            {
                // connection timed out or failed
                MIRACASTLOG_ERROR("Socket Connection Timedout ...\n");
            }
            else
            {
                // connection successful
                // do something with the connected socket
                MIRACASTLOG_INFO("Socket Connected Successfully ...\n");
                ret = MIRACAST_OK;
            }
        }
    }
    else
    {
        MIRACASTLOG_INFO("Socket Connected Successfully ...\n");
        ret = MIRACAST_OK;
    }

    /*---Wait for socket connect to complete---*/
    num_ready = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, 1000 /*timeout*/);
    for (i = 0; i < num_ready; i++)
    {
        if (events[i].events & EPOLLOUT)
        {
            MIRACASTLOG_INFO("Socket(%d) %d connected (EPOLLOUT)", i, events[i].data.fd);
        }
    }

    num_ready = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, 1000 /*timeout*/);
    for (i = 0; i < num_ready; i++)
    {
        if (events[i].events & EPOLLOUT)
        {
            MIRACASTLOG_INFO("Socket %d got some data via EPOLLOUT", events[i].data.fd);
            break;
        }
    }

    if ( MIRACAST_FAIL == ret ){
        close(m_tcpSockfd);
        m_tcpSockfd = -1;
    }

    MIRACASTLOG_TRACE("Exiting...");
    return ret;
}

RTSP_STATUS MiracastRTSPMsg::send_rstp_msg(int socket_fd, std::string rtsp_response_buffer)
{
    int read_ret = 0;
    read_ret = send(socket_fd, rtsp_response_buffer.c_str(), rtsp_response_buffer.length(), 0);

    if (0 > read_ret)
    {
        MIRACASTLOG_INFO("Send Failed (%d)%s", errno, strerror(errno));
        return RTSP_MSG_FAILURE;
    }

    MIRACASTLOG_INFO("Sending the RTSP response %d Data[%s]\n", read_ret, rtsp_response_buffer.c_str());
    return RTSP_MSG_SUCCESS;
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_m1_msg_m2_send_request(std::string rtsp_m1_msg_buffer)
{
    RTSP_STATUS status_code = RTSP_INVALID_MSG_RECEIVED;
    size_t found = rtsp_m1_msg_buffer.find(RTSP_REQ_OPTIONS);
    MIRACASTLOG_TRACE("Entering...");
    MIRACASTLOG_INFO("M1 request received");
    if (found != std::string::npos)
    {
        std::string m1_msg_resp_sink2src = "";
        MIRACASTLOG_INFO("M1 OPTIONS packet received");
        std::stringstream ss(rtsp_m1_msg_buffer);
        std::string prefix = "";
        std::string req_str;
        std::string seq_str;
        std::string line;

        while (std::getline(ss, line))
        {
            if (line.find(RTSP_STD_REQUIRE_FIELD) != std::string::npos)
            {
                prefix = RTSP_STD_REQUIRE_FIELD;
                req_str = line.substr(prefix.length());
                REMOVE_R(req_str);
                REMOVE_N(req_str);
            }
            else if (line.find(RTSP_STD_SEQUENCE_FIELD) != std::string::npos)
            {
                prefix = RTSP_STD_SEQUENCE_FIELD;
                seq_str = line.substr(prefix.length());
                REMOVE_R(seq_str);
                REMOVE_N(seq_str);
            }
        }

        m1_msg_resp_sink2src = generate_request_response_msg(RTSP_MSG_FMT_M1_RESPONSE, seq_str, req_str);

        MIRACASTLOG_INFO("Sending the M1 response \n-%s", m1_msg_resp_sink2src.c_str());

        status_code = send_rstp_msg(m_tcpSockfd, m1_msg_resp_sink2src);

        if (RTSP_MSG_SUCCESS == status_code)
        {
            std::string m2_msg_req_sink2src = "";
            MIRACASTLOG_INFO("M1 response sent\n");

            m2_msg_req_sink2src = generate_request_response_msg(RTSP_MSG_FMT_M2_REQUEST, empty_string, req_str);

            MIRACASTLOG_INFO("%s", m2_msg_req_sink2src.c_str());
            MIRACASTLOG_INFO("Sending the M2 request \n");
            status_code = send_rstp_msg(m_tcpSockfd, m2_msg_req_sink2src);
            if (RTSP_MSG_SUCCESS == status_code)
            {
                MIRACASTLOG_INFO("M2 request sent\n");
            }
            else
            {
                MIRACASTLOG_ERROR("M2 request failed\n");
            }
        }
        else
        {
            MIRACASTLOG_ERROR("M1 Response failed\n");
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
    return (status_code);
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_m2_request_ack(std::string rtsp_m1_response_ack_buffer)
{
    // Yet to check and validate the response
    return RTSP_MSG_SUCCESS;
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_m3_response_back(std::string rtsp_m3_msg_buffer)
{
    RTSP_STATUS status_code = RTSP_INVALID_MSG_RECEIVED;
    MIRACASTLOG_TRACE("Entering...");
    MIRACASTLOG_INFO("M3 request received");

    if (rtsp_m3_msg_buffer.find("wfd_video_formats") != std::string::npos)
    {
        std::string m3_msg_resp_sink2src = "";
        std::string seq_str = "";
        std::stringstream ss(rtsp_m3_msg_buffer);
        std::string prefix = "";
        std::string line;

        while (std::getline(ss, line))
        {
            if (line.find(RTSP_STD_SEQUENCE_FIELD) != std::string::npos)
            {
                prefix = RTSP_STD_SEQUENCE_FIELD;
                seq_str = line.substr(prefix.length());
                REMOVE_R(seq_str);
                REMOVE_N(seq_str);
                break;
            }
        }

        std::string content_buffer;

        m3_msg_resp_sink2src = generate_request_response_msg(RTSP_MSG_FMT_M3_RESPONSE, seq_str, empty_string);

        MIRACASTLOG_INFO("%s", m3_msg_resp_sink2src.c_str());

        status_code = send_rstp_msg(m_tcpSockfd, m3_msg_resp_sink2src);

        if (RTSP_MSG_SUCCESS == status_code)
        {
            MIRACASTLOG_INFO("Sending the M3 response \n");
        }
        else
        {
            MIRACASTLOG_ERROR("Sending the M3 response Failed\n");
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
    return (status_code);
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_m4_response_back(std::string rtsp_m4_msg_buffer)
{
    RTSP_STATUS status_code = RTSP_INVALID_MSG_RECEIVED;
    MIRACASTLOG_TRACE("Entering...");
    if (rtsp_m4_msg_buffer.find("SET_PARAMETER") != std::string::npos)
    {
        std::string m4_msg_resp_sink2src = "";
        std::string seq_str = "";
        std::string url = "";
        std::stringstream ss(rtsp_m4_msg_buffer);
        std::string prefix = "";
        std::string line;

        while (std::getline(ss, line))
        {
            if (line.find(RTSP_STD_SEQUENCE_FIELD) != std::string::npos)
            {
                prefix = RTSP_STD_SEQUENCE_FIELD;
                seq_str = line.substr(prefix.length());
                REMOVE_R(seq_str);
                REMOVE_N(seq_str);
            }
            else if (line.find(RTSP_WFD_PRESENTATION_URL_FIELD) != std::string::npos)
            {
                prefix = RTSP_WFD_PRESENTATION_URL_FIELD;
                std::size_t url_start_pos = line.find(prefix) + prefix.length();
                std::size_t url_end_pos = line.find(RTSP_SPACE_STR, url_start_pos);
                url = line.substr(url_start_pos, url_end_pos - url_start_pos);
                set_WFDPresentationURL(url);
            }
        }

        m4_msg_resp_sink2src = generate_request_response_msg(RTSP_MSG_FMT_M4_RESPONSE, seq_str, empty_string);

        MIRACASTLOG_INFO("Sending the M4 response \n");
        status_code = send_rstp_msg(m_tcpSockfd, m4_msg_resp_sink2src);
        if (RTSP_MSG_SUCCESS == status_code)
        {
            MIRACASTLOG_INFO("M4 response sent\n");
        }
        else
        {
            MIRACASTLOG_INFO("Failed to sent M4 response\n");
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
    return (status_code);
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_m5_msg_m6_send_request(std::string rtsp_m5_msg_buffer)
{
    RTSP_STATUS status_code = RTSP_INVALID_MSG_RECEIVED;
    MIRACASTLOG_TRACE("Entering...");
    if (rtsp_m5_msg_buffer.find("wfd_trigger_method: SETUP") != std::string::npos)
    {
        std::string m5_msg_resp_sink2src = "";
        std::string seq_str = "";
        std::stringstream ss(rtsp_m5_msg_buffer);
        std::string prefix = "";
        std::string line;

        while (std::getline(ss, line))
        {
            if (line.find(RTSP_STD_SEQUENCE_FIELD) != std::string::npos)
            {
                prefix = RTSP_STD_SEQUENCE_FIELD;
                seq_str = line.substr(prefix.length());
                REMOVE_R(seq_str);
                REMOVE_N(seq_str);
                break;
            }
        }

        m5_msg_resp_sink2src = generate_request_response_msg(RTSP_MSG_FMT_M5_RESPONSE, seq_str, empty_string);

        MIRACASTLOG_INFO("Sending the M5 response \n");
        status_code = send_rstp_msg(m_tcpSockfd, m5_msg_resp_sink2src);
        if (RTSP_MSG_SUCCESS == status_code)
        {
            std::string m6_msg_req_sink2src = "";
            MIRACASTLOG_INFO("M5 Response has sent\n");

            m6_msg_req_sink2src = generate_request_response_msg(RTSP_MSG_FMT_M6_REQUEST, seq_str, empty_string);

            MIRACASTLOG_INFO("Sending the M6 Request\n");
            status_code = send_rstp_msg(m_tcpSockfd, m6_msg_req_sink2src);
            if (RTSP_MSG_SUCCESS == status_code)
            {
                MIRACASTLOG_INFO("M6 Request has sent\n");
            }
            else
            {
                MIRACASTLOG_ERROR("Failed to Send the M6 Request\n");
            }
        }
        else
        {
            MIRACASTLOG_ERROR("Failed to Send the M5 response\n");
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
    return (status_code);
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_m6_ack_m7_send_request(std::string rtsp_m6_ack_buffer)
{
    RTSP_STATUS status_code = RTSP_INVALID_MSG_RECEIVED;
    MIRACASTLOG_TRACE("Entering...");
    if (!rtsp_m6_ack_buffer.empty())
    {
        size_t pos_ses = rtsp_m6_ack_buffer.find(RTSP_STD_SESSION_FIELD);
        std::string session = rtsp_m6_ack_buffer.substr(pos_ses + strlen(RTSP_STD_SESSION_FIELD));
        pos_ses = session.find(";");
        std::string session_number = session.substr(0, pos_ses);

        set_WFDSessionNumber(session_number);

        if (rtsp_m6_ack_buffer.find("client_port") != std::string::npos)
        {
            std::string m7_msg_req_sink2src = "";

            m7_msg_req_sink2src = generate_request_response_msg(RTSP_MSG_FMT_M7_REQUEST, empty_string, empty_string);

            MIRACASTLOG_INFO("Sending the M7 Request\n");
            status_code = send_rstp_msg(m_tcpSockfd, m7_msg_req_sink2src);
            if (RTSP_MSG_SUCCESS == status_code)
            {
                MIRACASTLOG_INFO("M7 Request has sent\n");
            }
            else
            {
                MIRACASTLOG_ERROR("Failed to Send the M7 Request\n");
            }
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
    return (status_code);
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_m7_request_ack(std::string rtsp_m7_ack_buffer)
{
    MIRACASTLOG_TRACE("Entering and Exiting...");
    // Yet to check and validate the M7 acknowledgement
    return RTSP_MSG_SUCCESS;
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_post_m1_m7_xchange(std::string rtsp_post_m1_m7_xchange_buffer)
{
    MIRACASTLOG_TRACE("Entering...");
    RTSP_STATUS status_code = RTSP_INVALID_MSG_RECEIVED;
    RTSP_STATUS sub_status_code = RTSP_MSG_SUCCESS;
    std::string rtsp_resp_sink2src = "";
    std::string seq_str = "";
    std::stringstream ss(rtsp_post_m1_m7_xchange_buffer);
    std::string prefix = "";
    std::string line;

    while (std::getline(ss, line))
    {
        if (line.find(RTSP_STD_SEQUENCE_FIELD) != std::string::npos)
        {
            prefix = RTSP_STD_SEQUENCE_FIELD;
            seq_str = line.substr(prefix.length());
            REMOVE_R(seq_str);
            REMOVE_N(seq_str);
            break;
        }
    }

    if (rtsp_post_m1_m7_xchange_buffer.find(RTSP_M16_REQUEST_MSG) != std::string::npos)
    {
        rtsp_resp_sink2src = generate_request_response_msg(RTSP_MSG_FMT_M16_RESPONSE, seq_str, empty_string);
    }
    else if (rtsp_post_m1_m7_xchange_buffer.find(RTSP_REQ_TEARDOWN_MODE) != std::string::npos)
    {
        MIRACASTLOG_INFO("TEARDOWN request from Source received\n");
        rtsp_resp_sink2src = generate_request_response_msg(RTSP_MSG_FMT_TEARDOWN_RESPONSE, seq_str, empty_string);
        sub_status_code = RTSP_MSG_TEARDOWN_REQUEST;
    }

    if (!(rtsp_resp_sink2src.empty()))
    {
        MIRACASTLOG_INFO("Sending the Response \n");
        status_code = send_rstp_msg(m_tcpSockfd, rtsp_resp_sink2src);
        if (RTSP_MSG_SUCCESS == status_code)
        {
            status_code = sub_status_code;
            MIRACASTLOG_INFO("Response sent\n");
        }
        else
        {
            MIRACASTLOG_INFO("Failed to sent Response\n");
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
    return status_code;
}

RTSP_STATUS MiracastRTSPMsg::rtsp_sink2src_request_msg_handling(eCONTROLLER_FW_STATES action_id)
{
    MIRACASTLOG_TRACE("Entering...");
    RTSP_MSG_FMT_SINK2SRC request_mode = RTSP_MSG_FMT_INVALID;
    RTSP_STATUS status_code = RTSP_MSG_SUCCESS;

    switch (action_id)
    {
        case RTSP_TEARDOWN_FROM_SINK2SRC:
        {
            request_mode = RTSP_MSG_FMT_TEARDOWN_REQUEST;
        }
        break;
        case RTSP_PLAY_FROM_SINK2SRC:
        {
            request_mode = RTSP_MSG_FMT_PLAY_REQUEST;
        }
        break;
        case RTSP_PAUSE_FROM_SINK2SRC:
        {
            request_mode = RTSP_MSG_FMT_PAUSE_REQUEST;
        }
        break;
        default:
        {
            //
        }
        break;
    }

    if (RTSP_MSG_FMT_INVALID != request_mode)
    {
        std::string rtsp_resp_sink2src;
        rtsp_resp_sink2src = generate_request_response_msg(RTSP_MSG_FMT_TEARDOWN_RESPONSE, empty_string, empty_string);

        if (!(rtsp_resp_sink2src.empty()))
        {
            MIRACASTLOG_INFO("Sending the [PLAY/PAUSE/TEARDOWN] REQUEST \n");
            status_code = send_rstp_msg(m_tcpSockfd, rtsp_resp_sink2src);
            if (RTSP_MSG_SUCCESS == status_code)
            {
                MIRACASTLOG_INFO("[PLAY/PAUSE/TEARDOWN] sent\n");
            }
            else
            {
                MIRACASTLOG_ERROR("Failed to sent PLAY/PAUSE\n");
            }
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
    return status_code;
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_msg_response_back(std::string rtsp_msg_buffer, eCONTROLLER_FW_STATES action_id)
{
    RTSP_STATUS status_code = RTSP_INVALID_MSG_RECEIVED;
    MIRACASTLOG_TRACE("Entering...");
    switch (action_id)
    {
        case RTSP_M1_REQUEST_RECEIVED:
        {
            status_code = validate_rtsp_m1_msg_m2_send_request(rtsp_msg_buffer);
        }
        break;
        case RTSP_M2_REQUEST_ACK:
        {
            status_code = validate_rtsp_m2_request_ack(rtsp_msg_buffer);
        }
        break;
        case RTSP_M3_REQUEST_RECEIVED:
        {
            status_code = validate_rtsp_m3_response_back(rtsp_msg_buffer);
        }
        break;
        case RTSP_M4_REQUEST_RECEIVED:
        {
            status_code = validate_rtsp_m4_response_back(rtsp_msg_buffer);
        }
        break;
        case RTSP_M5_REQUEST_RECEIVED:
        {
            status_code = validate_rtsp_m5_msg_m6_send_request(rtsp_msg_buffer);
        }
        break;
        case RTSP_M6_REQUEST_ACK:
        {
            status_code = validate_rtsp_m6_ack_m7_send_request(rtsp_msg_buffer);
        }
        break;
        case RTSP_M7_REQUEST_ACK:
        {
            status_code = validate_rtsp_m7_request_ack(rtsp_msg_buffer);
        }
        break;
        case RTSP_MSG_POST_M1_M7_XCHANGE:
        {
            status_code = validate_rtsp_post_m1_m7_xchange(rtsp_msg_buffer);
        }
        break;
        default:
        {
            //
        }
        break;
    }
    MIRACASTLOG_INFO("Validating RTSP Msg => ACTION[%#04X] Resp[%#04X]\n", action_id, status_code);
    MIRACASTLOG_TRACE("Exiting...");
    return status_code;
}

void MiracastRTSPMsg::RTSPMessageHandler_Thread(void *args)
{
    char rtsp_message_socket[4096] = {0};
    RTSP_HLDR_MSGQ_STRUCT rtsp_message_data = {};
    eCONTROLLER_FW_STATES controller_state = CONTROLLER_INVALID_STATE;
    RTSP_STATUS status_code = RTSP_TIMEDOUT;
    std::string socket_buffer;
    bool    start_monitor_keep_alive_msg = false,
            rtsp_msg_hldr_running_state = true,
            restart_discovery_needed = true;

    MIRACASTLOG_TRACE("Entering...");

    while ((nullptr != m_rtsp_msg_handler_thread)&&(true == rtsp_msg_hldr_running_state))
    {
        MIRACASTLOG_TRACE("!!! Waiting for Event !!!\n");
        m_rtsp_msg_handler_thread->receive_message(&rtsp_message_data, sizeof(rtsp_message_data), THREAD_RECV_MSG_INDEFINITE_WAIT);

        MIRACASTLOG_TRACE("!!! Received Action[%#04X] !!!\n", rtsp_message_data.state);

        if (RTSP_SELF_ABORT == rtsp_message_data.state)
        {
            MIRACASTLOG_TRACE("RTSP_SELF_ABORT ACTION Received\n");
            rtsp_msg_hldr_running_state = false;
            continue;
        }

        if (RTSP_START_RECEIVE_MSGS != rtsp_message_data.state)
        {
            continue;
        }
        rtsp_message_data.state = RTSP_M1_REQUEST_RECEIVED;

        memset(&rtsp_message_socket, 0x00, sizeof(rtsp_message_socket));

        while (RTSP_MSG_SUCCESS == receive_buffer_timedOut(m_tcpSockfd, rtsp_message_socket, sizeof(rtsp_message_socket)))
        {
            socket_buffer.clear();
            socket_buffer = rtsp_message_socket;

            status_code = validate_rtsp_msg_response_back(socket_buffer, rtsp_message_data.state);

            MIRACASTLOG_TRACE("!!! Validate RTSP Msg Action[%#04X] Response[%#04X] !!!\n", rtsp_message_data.state, status_code);

            if ((RTSP_MSG_SUCCESS != status_code) || (RTSP_M7_REQUEST_ACK == rtsp_message_data.state))
            {
                break;
            }
            memset(&rtsp_message_socket, 0x00, sizeof(rtsp_message_socket));
            rtsp_message_data.state = static_cast<eCONTROLLER_FW_STATES>(rtsp_message_data.state + 1);
        }

        start_monitor_keep_alive_msg = false;
        restart_discovery_needed = true;

        if ((RTSP_MSG_SUCCESS == status_code) && (RTSP_M7_REQUEST_ACK == rtsp_message_data.state))
        {
            controller_state = CONTROLLER_RTSP_MSG_RECEIVED_PROPERLY;
            start_monitor_keep_alive_msg = true;
        }
        else if (RTSP_INVALID_MSG_RECEIVED == status_code)
        {
            controller_state = CONTROLLER_RTSP_INVALID_MESSAGE;
        }
        else if (RTSP_MSG_FAILURE == status_code)
        {
            controller_state = CONTROLLER_RTSP_SEND_REQ_RESP_FAILED;
        }
        else
        {
            controller_state = CONTROLLER_RTSP_MSG_TIMEDOUT;
        }
        MIRACASTLOG_TRACE("Msg to Controller Action[%#04X]\n", controller_state);
        send_msgto_controller_thread(controller_state);

        if (CONTROLLER_RTSP_MSG_RECEIVED_PROPERLY != controller_state)
        {
            continue;
        }

        RTSP_STATUS socket_state;

        while (true == start_monitor_keep_alive_msg)
        {
            memset(&rtsp_message_socket, 0x00, sizeof(rtsp_message_socket));
            socket_state = receive_buffer_timedOut( m_tcpSockfd,
                                                    rtsp_message_socket,
                                                    sizeof(rtsp_message_socket),
                                                    RTSP_KEEP_ALIVE_WAIT_TIMEOUT );
            if (RTSP_MSG_SUCCESS == socket_state)
            {
                socket_buffer.clear();
                socket_buffer = rtsp_message_socket;
                MIRACASTLOG_TRACE("\n #### RTSP Message [%s] #### \n", socket_buffer.c_str());

                status_code = validate_rtsp_msg_response_back(socket_buffer, RTSP_MSG_POST_M1_M7_XCHANGE);

                MIRACASTLOG_TRACE("!!! Validate RTSP Msg Action[%#04X] Response[%#04X] !!!\n", rtsp_message_data.state, status_code);
                if ((RTSP_MSG_TEARDOWN_REQUEST == status_code) ||
                    ((RTSP_MSG_SUCCESS != status_code) &&
                     (RTSP_MSG_TEARDOWN_REQUEST != status_code)))
                {
                    MIRACASTLOG_TRACE("Msg to Controller Action[%#04X]\n", CONTROLLER_RTSP_TEARDOWN_REQ_RECEIVED);
                    send_msgto_controller_thread(CONTROLLER_RTSP_TEARDOWN_REQ_RECEIVED);
                    restart_discovery_needed = false;
                    break;
                }
            }
            else if (RTSP_MSG_FAILURE == socket_state)
            {
                controller_state = CONTROLLER_RTSP_SEND_REQ_RESP_FAILED;
                break;
            }

            MIRACASTLOG_TRACE("!!! Waiting for Event !!!\n");
            if (true == m_rtsp_msg_handler_thread->receive_message(&rtsp_message_data, sizeof(rtsp_message_data), 1))
            {
                MIRACASTLOG_TRACE("!!! Received Action[%#04X] !!!\n", rtsp_message_data.state);
                switch (rtsp_message_data.state)
                {
                    case RTSP_RESTART:
                    {
                        MIRACASTLOG_TRACE("[RTSP_RESTART] ACTION Received\n");
                        start_monitor_keep_alive_msg = false;
                    }
                    break;
                    case RTSP_SELF_ABORT:
                    case RTSP_PLAY_FROM_SINK2SRC:
                    case RTSP_PAUSE_FROM_SINK2SRC:
                    case RTSP_TEARDOWN_FROM_SINK2SRC:
                    {
                        if (RTSP_SELF_ABORT == rtsp_message_data.state){
                            rtsp_message_data.state = RTSP_TEARDOWN_FROM_SINK2SRC;
                            MIRACASTLOG_TRACE("[RTSP_SELF_ABORT] Received\n");
                            rtsp_msg_hldr_running_state = false;
                        }
                        else{
                            MIRACASTLOG_TRACE("[RTSP_PLAY/RTSP_PAUSE/RTSP_TEARDOWN] ACTION Received\n");
                        }
                        if ((RTSP_MSG_FAILURE == rtsp_sink2src_request_msg_handling(rtsp_message_data.state) ||
                            (RTSP_TEARDOWN_FROM_SINK2SRC == rtsp_message_data.state)))
                        {
                            start_monitor_keep_alive_msg = false;
                        }
                    }
                    break;
                    default:
                    {
                        //
                    }
                    break;
                }
            }
        }

        MIRACASTLOG_TRACE("!!! Received Action[%#04X] !!!\n", rtsp_message_data.state);
        if ( true == restart_discovery_needed )
        {
            MIRACASTLOG_TRACE("Msg to Controller Action[%#04X]\n", CONTROLLER_RTSP_RESTART_DISCOVERING);
            send_msgto_controller_thread(CONTROLLER_RTSP_RESTART_DISCOVERING);
        }
        if ( -1 != m_tcpSockfd )
        {
            close(m_tcpSockfd);
            m_tcpSockfd = -1;
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastRTSPMsg::send_msgto_controller_thread(eCONTROLLER_FW_STATES state)
{
    CONTROLLER_MSGQ_STRUCT controller_msgq_data = {0};
    MIRACASTLOG_TRACE("Entering...");
    controller_msgq_data.state = state;
    controller_msgq_data.msg_type = RTSP_MSG;
    m_controller_thread->send_message(&controller_msgq_data, CONTROLLER_MSGQ_SIZE);
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastRTSPMsg::send_msgto_rtsp_msg_hdler_thread(eCONTROLLER_FW_STATES state)
{
    RTSP_HLDR_MSGQ_STRUCT rtsp_hldr_msgq_data = {RTSP_INVALID_ACTION,0};
    MIRACASTLOG_TRACE("Entering...");
    if (nullptr != m_rtsp_msg_handler_thread){
        rtsp_hldr_msgq_data.state = state;
        m_rtsp_msg_handler_thread->send_message(&rtsp_hldr_msgq_data, RTSP_HANDLER_MSGQ_SIZE);
    }
    MIRACASTLOG_TRACE("Exiting...");
}

void RTSPMsgHandlerCallback(void *args)
{
    MiracastRTSPMsg *rtsp_msg_obj = (MiracastRTSPMsg *)args;
    MIRACASTLOG_TRACE("Entering...");
    rtsp_msg_obj->RTSPMessageHandler_Thread(nullptr);
    MIRACASTLOG_TRACE("Exiting...");
}

#ifdef ENABLE_HDCP2X_SUPPORT
void MiracastController::DumpBuffer(char *buffer, int length)
{
    // Loop through the buffer, printing each byte in hex format
    std::string hex_string;
    for (int i = 0; i < length; i++)
    {
        char hex_byte[3];
        snprintf(hex_byte, sizeof(hex_byte), "%02X", (unsigned char)buffer[i]);
        hex_string += "0x";
        hex_string += hex_byte;
        hex_string += " ";
    }
    MIRACASTLOG_INFO("\n######### DUMP BUFFER[%u] #########\n%s\n###############################\n", length, hex_string.c_str());
}

void MiracastController::HDCPTCPServerHandlerThread(void *args)
{
    char buff[HDCP2X_SOCKET_BUF_MAX];
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        MIRACASTLOG_ERROR("socket creation failed...\n");
    }
    else
        MIRACASTLOG_INFO("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(HDCP2X_PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        MIRACASTLOG_ERROR("socket bind failed...\n");
    }
    else
        MIRACASTLOG_INFO("Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0)
    {
        MIRACASTLOG_ERROR("Listen failed...\n");
    }
    else
        MIRACASTLOG_INFO("Server listening..\n");
    len = sizeof(cli);

    // Accept the data packet from client and verification
    connfd = accept(sockfd, (SA *)&cli, &len);
    if (connfd < 0)
    {
        MIRACASTLOG_ERROR("server accept failed...\n");
    }
    else
        MIRACASTLOG_INFO("server accept the client...\n");

    while (true)
    {
        bzero(buff, HDCP2X_SOCKET_BUF_MAX);

        // read the message from client and copy it in buffer
        int n = read(connfd, buff, sizeof(buff));

        if (0 < n)
        {
            DumpBuffer(buff, n);
        }

        bzero(buff, HDCP2X_SOCKET_BUF_MAX);
    }
}

void HDCPHandlerCallback(void *args)
{
    MiracastController *miracast_ctrler_obj = (MiracastController *)args;

    miracast_ctrler_obj->HDCPTCPServerHandlerThread(nullptr);
}
#endif /*ENABLE_HDCP2X_SUPPORT*/