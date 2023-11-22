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
#include <MiracastGstPlayer.h>

MiracastRTSPMsg *MiracastRTSPMsg::m_rtsp_msg_obj{nullptr};
static std::string empty_string = "";

void RTSPMsgHandlerCallback(void *args);

#ifdef ENABLE_HDCP2X_SUPPORT
void HDCPHandlerCallback(void *args);
#endif

#ifdef ENABLE_MIRACAST_PLAYER_TEST_NOTIFIER
void MiracastPlayerTestNotifierThreadCallback(void *args);
#endif

RTSP_MSG_FMT_TEMPLATE MiracastRTSPMsg::rtsp_msg_fmt_template[] = {
    {RTSP_MSG_FMT_M1_RESPONSE, "RTSP/1.0 200 OK\r\nPublic: \"%s, GET_PARAMETER, SET_PARAMETER\"\r\nCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_M2_REQUEST, "OPTIONS * RTSP/1.0\r\nRequire: %s\r\nCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_M3_RESPONSE, "RTSP/1.0 200 OK\r\nContent-Length: %s\r\nContent-Type: text/parameters\r\nCSeq: %s\r\n\r\n%s"},
    {RTSP_MSG_FMT_M4_RESPONSE, "%sCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_M5_RESPONSE, "%sCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_M6_REQUEST, "SETUP %s RTSP/1.0\r\nTransport: %s;%sclient_port=%s\r\nCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_M7_REQUEST, "PLAY %s RTSP/1.0\r\nSession: %s\r\nCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_M16_RESPONSE, "%sCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_PAUSE_REQUEST, "PAUSE %s RTSP/1.0\r\nSession: %s\r\nCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_PLAY_REQUEST, "PLAY %s RTSP/1.0\r\nSession: %s\r\nCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_TEARDOWN_REQUEST, "TEARDOWN %s RTSP/1.0\r\nSession: %s\r\nCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_TRIGGER_METHODS_RESPONSE, "%sCSeq: %s\r\n\r\n"},
    {RTSP_MSG_FMT_REPORT_ERROR, "%sCSeq: %s\r\n\r\n"}
};

RTSP_PARSER_TEMPLATE MiracastRTSPMsg::rtsp_msg_parser_fields[] = {
    {RTSP_PARSER_FIELD_START, ""},

    {RTSP_OPTIONS_REQ_FIELD, "OPTIONS * RTSP/1.0"},

    {RTSP_M2_RESPONSE_VALIDATE_MARKER_START, ""},
    {RTSP_OPTIONS_QUERY_FIELD, "org.wfa.wfd1.0"},
    {RTSP_SETUP_FIELD, "SETUP"},
    {RTSP_TEARDOWN_FIELD, "TEARDOWN"},
    {RTSP_PLAY_FIELD, "PLAY"},
    {RTSP_PAUSE_FIELD, "PAUSE"},
    {RTSP_GET_PARAMETER_FIELD, "GET_PARAMETER"},
    {RTSP_SET_PARAMETER_FIELD, "SET_PARAMETER"},
    {RTSP_M2_RESPONSE_VALIDATE_MARKER_END, ""},
    
    {RTSP_M3_REQ_VALIDATE_MARKER_START, ""},
    {RTSP_WFD_HDCP_FIELD, "wfd_content_protection" , &MiracastRTSPMsg::get_WFDContentProtection},
    {RTSP_WFD_VIDEO_FMT_FIELD, "wfd_video_formats" , &MiracastRTSPMsg::get_WFDVideoFormat },
    {RTSP_WFD_AUDIO_CODEC_FIELD, "wfd_audio_codecs" , &MiracastRTSPMsg::get_WFDAudioCodecs},
    {RTSP_WFD_CLI_RTP_PORTS_FIELD, "wfd_client_rtp_ports" , &MiracastRTSPMsg::get_WFDClientRTPPorts},
    {RTSP_M3_REQ_VALIDATE_MARKER_END, ""},

    {RTSP_WFD_STREAMING_URL_FIELD, "wfd_presentation_URL: "},

    {RTSP_SEQUENCE_FIELD, "CSeq: "},
    {RTSP_REQUIRE_FIELD, "Require: "},
    {RTSP_SESSION_FIELD, "Session: "},
    {RTSP_PUBLIC_FIELD, "Public: "},
    {RTSP_TRANSPORT_FIELD, "Transport: "},
    {RTSP_CONTENT_TEXT_FIELD, "Content-Type: text/parameters"},
    {RTSP_TRIGGER_METHOD_FIELD, "wfd_trigger_method: "},
    {RTSP_UNICAST_FIELD, "unicast"},
    {RTSP_VERSION_FIELD, "RTSP/1.0"},

    {RTSP_PARSER_FIELD_END, ""},
};

const int MiracastRTSPMsg::num_parse_fields = sizeof(rtsp_msg_parser_fields) / sizeof(rtsp_msg_parser_fields[0]);

RTSP_ERRORCODE_TEMPLATE MiracastRTSPMsg::rtsp_msg_error_codes[] = {
    {RTSP_ERRORCODE_CONTINUE, "RTSP/1.0 100 Continue\r\n"},

    {RTSP_ERRORCODE_OK, "RTSP/1.0 200 OK\r\n"},
    {RTSP_ERRORCODE_CREATED, "RTSP/1.0 201 Created\r\n"},

    {RTSP_ERRORCODE_LOW_STORAGE, "RTSP/1.0 250 Low on Storage Space\r\n"},

    {RTSP_ERRORCODE_MULTI_CHOICES, "RTSP/1.0 300 Multiple Choices\r\n"},
    {RTSP_ERRORCODE_MOVED_PERMANENTLY, "RTSP/1.0 301 Moved Permanently\r\n"},
    {RTSP_ERRORCODE_MOVED_TEMPORARILY, "RTSP/1.0 302 Moved Temporarily\r\n"},
    {RTSP_ERRORCODE_SEE_OTHER, "RTSP/1.0 303 See Other\r\n"},
    {RTSP_ERRORCODE_NOT_MODIFIED, "RTSP/1.0 304 Not Modified\r\n"},
    {RTSP_ERRORCODE_USE_PROXY, "RTSP/1.0 305 Use Proxy\r\n"},

    {RTSP_ERRORCODE_BAD_REQUEST, "RTSP/1.0 400 Bad Request\r\n"},
    {RTSP_ERRORCODE_UNAUTHORIZED, "RTSP/1.0 401 Unuthorized\r\n"},
    {RTSP_ERRORCODE_PAYMENT_REQUIRED, "RTSP/1.0 402 Payment Required\r\n"},
    {RTSP_ERRORCODE_FORBIDDEN, "RTSP/1.0 403 Forbidden\r\n"},
    {RTSP_ERRORCODE_NOT_FOUND, "RTSP/1.0 404 Not Found\r\n"},
    {RTSP_ERRORCODE_METHOD_NOT_ALLOWED, "RTSP/1.0 405 Method Not Allowed\r\n"},
    {RTSP_ERRORCODE_NOT_ACCEPTABLE, "RTSP/1.0 406 Not Acceptable\r\n"},
    {RTSP_ERRORCODE_PROXY_AUTHENDICATION_REQUIRED, "RTSP/1.0 407 Proxy Authendication Required\r\n"},
    {RTSP_ERRORCODE_REQUEST_TIMEDOUT, "RTSP/1.0 408 Request Time-Out\r\n"},
    {RTSP_ERRORCODE_GONE, "RTSP/1.0 410 Gone\r\n"},
    {RTSP_ERRORCODE_LENGTH_REQUIRED, "RTSP/1.0 411 Length Required\r\n"},
    {RTSP_ERRORCODE_PRECONDITION_FAILED, "RTSP/1.0 412 Precondition Failed\r\n"},
    {RTSP_ERRORCODE_REQUEST_ENTITY_TOO_LARGE, "RTSP/1.0 413 Request Entity Too Large\r\n"},
    {RTSP_ERRORCODE_REQUEST_URI_TOO_LARGE, "RTSP/1.0 413 Request-URI Too Large\r\n"},
    {RTSP_ERRORCODE_UNSUPPORTED_MEDIA_TYPE, "RTSP/1.0 414 Unsupported Media Type\r\n"},

    {RTSP_ERRORCODE_PARAMETER_NOT_UNDERSTOOD, "RTSP/1.0 451 Parameter Not Understood\r\n"},
    {RTSP_ERRORCODE_CONFERENCE_NOT_FOUND, "RTSP/1.0 452 Conference Not Found\r\n"},
    {RTSP_ERRORCODE_NOT_ENOUGH_BANDWIDTH, "RTSP/1.0 453 Not Enough Bandwidth\r\n"},
    {RTSP_ERRORCODE_SESSION_NOT_FOUND, "RTSP/1.0 454 Session Not Found\r\n"},
    {RTSP_ERRORCODE_METHOD_NOT_VALID, "RTSP/1.0 455 Method Not Valid in This State\r\n"},
    {RTSP_ERRORCODE_HEADER_FIELD_NOT_VALID, "RTSP/1.0 456 Header Field Not Valid\r\n"},
    {RTSP_ERRORCODE_INVALID_RANGE, "RTSP/1.0 457 Invalid Range\r\n"},
    {RTSP_ERRORCODE_RO_PARAMETER, "RTSP/1.0 458 Parameter Is Read-Only\r\n"},
    {RTSP_ERRORCODE_AGGREGATE_OPERATION_NOT_ALLOWED, "RTSP/1.0 459 Aggregate operation not allowed\r\n"},
    {RTSP_ERRORCODE_ONLY_AGGREGATE_OPERATION_ALLOWED, "RTSP/1.0 460 Only aggregate operation allowed\r\n"},
    {RTSP_ERRORCODE_UNSUPPORTED_TRANSPORT, "RTSP/1.0 461 Unsupported transport\r\n"},
    {RTSP_ERRORCODE_DEST_UNREACHABLE, "RTSP/1.0 462 Destination unreachable\r\n"},
    {RTSP_ERRORCODE_KEY_MGMNT_FAILURE, "RTSP/1.0 463 Key management Failure\r\n"},

    {RTSP_ERRORCODE_INTERNAL_SERVER_ERROR, "RTSP/1.0 500 Internal Server Error\r\n"},
    {RTSP_ERRORCODE_NOT_IMPLEMENTED, "RTSP/1.0 501 Not Implemented\r\n"},
    {RTSP_ERRORCODE_BAD_GATEWAY, "RTSP/1.0 502 Bad Gateway\r\n"},
    {RTSP_ERRORCODE_SERVICE_UNAVAILABLE, "RTSP/1.0 503 Service Unavailable\r\n"},
    {RTSP_ERRORCODE_GATEWAY_TIMEOUT, "RTSP/1.0 503 Gateway Time-out\r\n"},
    {RTSP_ERRORCODE_VERSION_NOT_SUPPORTED, "RTSP/1.0 504 RTSP Version not supported\r\n"},

    {RTSP_ERRORCODE_OPTION_NOT_SUPPORTED, "RTSP/1.0 551 Option not supported\r\n"}
};

MiracastRTSPMsg *MiracastRTSPMsg::getInstance(MiracastError &error_code , MiracastPlayerNotifier *player_notifier, MiracastThread *controller_thread_id)
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
    if ((nullptr != m_rtsp_msg_obj) && ( nullptr != player_notifier ))
    {
        m_rtsp_msg_obj->m_player_notify_handler = player_notifier;
    }
    MIRACASTLOG_TRACE("Exiting...");
    return m_rtsp_msg_obj;
}

void MiracastRTSPMsg::destroyInstance()
{
    RTSP_HLDR_MSGQ_STRUCT rtsp_hldr_msgq_data = { 0 };
    MIRACASTLOG_TRACE("Entering...");

    rtsp_hldr_msgq_data.state = RTSP_SELF_ABORT;
    m_rtsp_msg_obj->send_msgto_rtsp_msg_hdler_thread(rtsp_hldr_msgq_data);

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
    m_epollfd = -1;
    m_streaming_started = false;

    m_wfd_src_req_timeout = RTSP_REQUEST_RECV_TIMEOUT;
    m_wfd_src_res_timeout = RTSP_RESPONSE_RECV_TIMEOUT;
    m_wfd_src_session_timeout = RTSP_DFLT_KEEP_ALIVE_WAIT_TIMEOUT_SEC;

    m_current_sequence_number.clear();

    set_WFDEnableDisableUnicast(true);

    st_video_fmt.preferred_display_mode_supported = RTSP_PREFERED_DISPLAY_NOT_SUPPORTED;
    st_video_fmt.st_h264_codecs.profile = RTSP_PROFILE_BMP_CHP_SUPPORTED;
    st_video_fmt.st_h264_codecs.level = RTSP_H264_LEVEL_4_BITMAP;
    st_video_fmt.st_h264_codecs.cea_mask = static_cast<RTSP_CEA_RESOLUTIONS>(RTSP_CEA_RESOLUTION_720x480p60
                                            | RTSP_CEA_RESOLUTION_720x576p50
                                            | RTSP_CEA_RESOLUTION_1280x720p30
                                            | RTSP_CEA_RESOLUTION_1280x720p60
                                            | RTSP_CEA_RESOLUTION_1920x1080p30
                                            | RTSP_CEA_RESOLUTION_1920x1080p60);
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

#ifndef MIRACAST_CERT_BUILD
    m_wfd_src_req_timeout += RTSP_REQ_RESP_RECV_TIMEOUT_OFFSET_MSEC;
    m_wfd_src_res_timeout += RTSP_REQ_RESP_RECV_TIMEOUT_OFFSET_MSEC;
    MIRACASTLOG_TRACE("RequestResponse Timeouts have updated as [%u,%u]",m_wfd_src_req_timeout,m_wfd_src_res_timeout);
#endif

    MIRACASTLOG_TRACE("Exiting...");
}
MiracastRTSPMsg::~MiracastRTSPMsg()
{
    if ( nullptr != m_rtsp_msg_handler_thread )
    {
        delete m_rtsp_msg_handler_thread;
        m_rtsp_msg_handler_thread = nullptr;
    }

#ifdef ENABLE_MIRACAST_PLAYER_TEST_NOTIFIER
    destroy_TestNotifier();
#endif

    Release_SocketAndEpollDescriptor();
}

void MiracastRTSPMsg::Release_SocketAndEpollDescriptor(void)
{
    MIRACASTLOG_TRACE("Entering...");
    if (-1 != m_tcpSockfd)
    {
        close(m_tcpSockfd);
        m_tcpSockfd = -1;
    }
    if ( -1 != m_epollfd )
    {
        close(m_epollfd);
        m_epollfd = -1;
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

    MIRACASTLOG_INFO("video format[%s]...\n",m_wfd_video_formats.c_str());
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

    MIRACASTLOG_INFO("audio format[%s]...\n",m_wfd_audio_codecs.c_str());
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

bool MiracastRTSPMsg::set_WFDRequestResponseTimeout( unsigned int request_timeout , unsigned int response_timeout )
{
    MIRACASTLOG_TRACE("Entering...");
    if (( 0 != request_timeout ) && ( 0 != response_timeout ))
    {
        m_wfd_src_req_timeout = request_timeout;
        m_wfd_src_res_timeout = response_timeout;
        MIRACASTLOG_INFO("Request[%u]Response[%u]\n",request_timeout,response_timeout);
    }
    MIRACASTLOG_TRACE("Exiting...");
    return true;
}

bool MiracastRTSPMsg::set_wait_timeout(unsigned int waittime_ms)
{
    MIRACASTLOG_TRACE("Entering...");
    m_current_wait_time_ms = waittime_ms;
    MIRACASTLOG_TRACE("Exiting...");
    return true;
}

unsigned int MiracastRTSPMsg::get_wait_timeout(void)
{
    MIRACASTLOG_TRACE("Entering and Exiting...");
    return m_current_wait_time_ms;
}

const char* MiracastRTSPMsg::get_RequestResponseFormat(RTSP_MSG_FMT_SINK2SRC format_type)
{
    MIRACASTLOG_TRACE("Entering...");
    int index = static_cast<RTSP_MSG_FMT_SINK2SRC>(format_type) - static_cast<RTSP_MSG_FMT_SINK2SRC>(RTSP_MSG_FMT_M1_RESPONSE);
    if (index >= 0 && index < static_cast<int>(sizeof(rtsp_msg_fmt_template) / sizeof(rtsp_msg_fmt_template[0])))
    {
        MIRACASTLOG_TRACE("Exiting...");
        return rtsp_msg_fmt_template[index].template_name;
    }
    MIRACASTLOG_TRACE("Exiting...");
    return "";
}

const char* MiracastRTSPMsg::get_errorcode_string(RTSP_ERRORCODES error_code)
{
    MIRACASTLOG_TRACE("Entering...");
    int index = static_cast<RTSP_ERRORCODES>(error_code) - static_cast<RTSP_ERRORCODES>(RTSP_ERRORCODE_CONTINUE);
    if (index >= 0 && index < static_cast<int>(sizeof(rtsp_msg_error_codes) / sizeof(rtsp_msg_error_codes[0])))
    {
        MIRACASTLOG_TRACE("Exiting...");
        return rtsp_msg_error_codes[index].string_fmt;
    }
    MIRACASTLOG_TRACE("Exiting...");
    return "";
}

const char* MiracastRTSPMsg::get_parser_field_by_index(RTSP_PARSER_FIELDS parse_field)
{
    MIRACASTLOG_TRACE("Entering...");

    if ( RTSP_PARSER_FIELD_START < parse_field && parse_field < num_parse_fields )
    {
        MIRACASTLOG_TRACE("Exiting with [%d][%s]...",parse_field,rtsp_msg_parser_fields[parse_field].string_fmt);
        return rtsp_msg_parser_fields[parse_field].string_fmt;
    }
    MIRACASTLOG_TRACE("Exiting...");
    return "";
}

std::string MiracastRTSPMsg::get_parser_field_value(RTSP_PARSER_FIELDS parse_field)
{
    MIRACASTLOG_TRACE("Entering [%#04X]...",parse_field);

    if ( RTSP_PARSER_FIELD_START < parse_field && parse_field < num_parse_fields )
    {
        MIRACASTLOG_TRACE("parse_field[%#04X][%x]",parse_field,rtsp_msg_parser_fields[parse_field].parse_field_info_fn);
        if ( nullptr != rtsp_msg_parser_fields[parse_field].parse_field_info_fn ){
            MIRACASTLOG_TRACE("Exiting...");
            return (this->*rtsp_msg_parser_fields[parse_field].parse_field_info_fn)();
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
    return "";
}

std::string MiracastRTSPMsg::get_parser_field_n_value_by_name(std::string request_field_name )
{
    RTSP_PARSER_FIELDS parse_field = static_cast<RTSP_PARSER_FIELDS>(RTSP_PARSER_FIELD_START+1);
    MIRACASTLOG_TRACE("Entering [%#04X]...",parse_field);

    while ( parse_field < RTSP_PARSER_FIELD_END )
    {
        if (0 == (request_field_name.compare(rtsp_msg_parser_fields[parse_field].string_fmt)))
        {
            MIRACASTLOG_TRACE("Inx[%d]request_field_name[%s]fmt[%s]",
                                parse_field,
                                request_field_name.c_str(),
                                rtsp_msg_parser_fields[parse_field].string_fmt);
            MIRACASTLOG_TRACE("Exiting...");
            return get_parser_field_value(parse_field);
        }
        parse_field = static_cast<RTSP_PARSER_FIELDS>(parse_field+1);
    }
    MIRACASTLOG_TRACE("Exiting...");
    return std::string("");
}

std::string MiracastRTSPMsg::parse_received_parser_field_value(std::string rtsp_msg_buffer , RTSP_PARSER_FIELDS parse_field )
{
    std::string seq_str = "";
    std::stringstream ss(rtsp_msg_buffer);
    std::string prefix = "";
    std::string line;
    MIRACASTLOG_TRACE("Entering...");
    const char  *given_tag = get_parser_field_by_index(parse_field);

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

std::string MiracastRTSPMsg::generate_request_response_msg(RTSP_MSG_FMT_SINK2SRC msg_fmt_needed, std::string received_session_no , std::string append_data1 , RTSP_ERRORCODES error_code )
{
    MIRACASTLOG_TRACE("Entering...");
    std::vector<const char *> sprintf_args;
    const char *template_str = get_RequestResponseFormat(msg_fmt_needed);
    std::string content_buffer = "";
    std::string unicast_supported = "";
    std::string content_buffer_len;
    std::string sequence_number = "";
    std::string URL = get_WFDPresentationURL();
    std::string TSProfile = get_WFDTransportProfile();
    std::string StreamingPort = get_WFDStreamingPortNumber();
    std::string WFDSessionNum = get_CurrentWFDSessionNumber();
    std::string resp_error_string = get_errorcode_string(error_code);

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
            content_buffer = append_data1;
            content_buffer_len = std::to_string(content_buffer.length());

            sprintf_args.push_back(content_buffer_len.c_str());
            sprintf_args.push_back(received_session_no.c_str());
            sprintf_args.push_back(content_buffer.c_str());

            MIRACASTLOG_TRACE("content_buffer - [%s]\n", content_buffer.c_str());
        }
        break;
        case RTSP_MSG_FMT_M4_RESPONSE:
        case RTSP_MSG_FMT_M5_RESPONSE:
        case RTSP_MSG_FMT_M16_RESPONSE:
        case RTSP_MSG_FMT_TRIGGER_METHODS_RESPONSE:
        case RTSP_MSG_FMT_REPORT_ERROR:
        {
            sprintf_args.push_back(resp_error_string.c_str());
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
            sequence_number = generate_RequestSequenceNumber();

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
            MIRACASTLOG_ERROR("!!! INVALID FMT REQUEST[%#04X] !!!\n",msg_fmt_needed);
        }
        break;
    }

    std::string result = "";

    if (0 != sprintf_args.size())
    {
        result = MiracastRTSPMsg::format_string(template_str, sprintf_args);
    }
    MIRACASTLOG_TRACE("!!! Formatted Buffer[%s] !!!\n",result.c_str());
    MIRACASTLOG_TRACE("Exiting...");
    return result;
}

std::string MiracastRTSPMsg::generate_RequestSequenceNumber(void)
{
    int next_number = std::stoi(m_current_sequence_number.empty() ? "1" : m_current_sequence_number) + 1;
    m_current_sequence_number = std::to_string(next_number);
    return m_current_sequence_number;
}

std::string MiracastRTSPMsg::get_RequestSequenceNumber(void)
{
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

void MiracastRTSPMsg::set_state( eMIRA_PLAYER_STATES state , bool send_notification , eM_PLAYER_REASON_CODE reason_code )
{
    MIRACASTLOG_INFO("Entering [%d]notify[%u]reason[%u]...",
                        state,send_notification,reason_code);
    m_current_state = state;

    if (( MIRACAST_PLAYER_STATE_STOPPED == state )||( MIRACAST_PLAYER_STATE_SELF_ABORT == state )){
        stop_streaming(state);
    }

    if (( true == send_notification ) && ( nullptr != m_player_notify_handler ))
    {
        m_player_notify_handler->onStateChange( m_connected_mac_addr, 
                                                m_connected_device_name, 
                                                state, 
                                                reason_code );
    }
    MIRACASTLOG_INFO("Exiting...");
}

eMIRA_PLAYER_STATES MiracastRTSPMsg::get_state(void)
{
    return m_current_state;
}

void MiracastRTSPMsg::store_srcsink_info( std::string client_name,
                                          std::string client_mac,
                                          std::string src_dev_ip,
                                          std::string sink_ip)
{
    MIRACASTLOG_TRACE("Entering...");
    m_connected_device_name = client_name;
    m_connected_mac_addr = client_mac;
    m_src_dev_ip = src_dev_ip;
    m_sink_ip = sink_ip;
    MIRACASTLOG_TRACE("Exiting...");
}

/*
 * Wait for data returned by the socket for specified time
 */
bool MiracastRTSPMsg::wait_data_timeout(int m_Sockfd, unsigned int ms)
{
    struct timeval timeout = {0};
    fd_set readFDSet;

    MIRACASTLOG_TRACE("Entering WaitTime[%d]...",ms);

    FD_ZERO(&readFDSet);
    FD_SET(m_Sockfd, &readFDSet);

    timeout.tv_sec = (ms / 1000);
    timeout.tv_usec = ((ms % 1000) * 1000);

    if (select(m_Sockfd + 1, &readFDSet, nullptr, nullptr, &timeout) > 0)
    {
        MIRACASTLOG_TRACE("Exiting ...");
        return FD_ISSET(m_Sockfd, &readFDSet);
    }
    MIRACASTLOG_TRACE("Exiting [TimedOut]...");
    return false;
}

RTSP_STATUS MiracastRTSPMsg::receive_buffer_timedOut(int socket_fd, void *buffer, size_t buffer_len , unsigned int wait_time_ms )
{
    int recv_return = -1;
    RTSP_STATUS status = RTSP_MSG_SUCCESS;

    MIRACASTLOG_TRACE("Entering WaitTime[%d]...",wait_time_ms);

    if (!wait_data_timeout(socket_fd, wait_time_ms))
    {
        MIRACASTLOG_TRACE("Exiting [TimedOut]...");
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
            MIRACASTLOG_ERROR("recv failed error [%s]\n", strerror(errno));
            status = RTSP_MSG_FAILURE;
        }
    }
    MIRACASTLOG_VERBOSE("received string(%d) - %s\n", recv_return, buffer);
    MIRACASTLOG_TRACE("Exiting [%d]...",status);
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

    unsigned char retry_count = 5;
    unsigned int current_waittime = SOCKET_DFLT_WAIT_TIMEOUT / retry_count;
    bool is_connected = false;

    while ( retry_count-- && ( false == is_connected ))
    {
        Release_SocketAndEpollDescriptor();

        m_tcpSockfd = socket(in_addr.ss_family, SOCK_STREAM | SOCK_CLOEXEC, 0);
        if (m_tcpSockfd < 0)
        {
            MIRACASTLOG_ERROR("TCP Socket creation error %s", strerror(errno));
            continue;
        }

        /*---Add socket to epoll---*/
        m_epollfd = epoll_create(1);
        struct epoll_event event;
        event.events = EPOLLIN | EPOLLOUT;
        event.data.fd = m_tcpSockfd;
        epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_tcpSockfd, &event);

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
                MIRACASTLOG_INFO("WaitingTime[%u] for Socket Connection",current_waittime);
                // connection in progress
                if (!wait_data_timeout(m_tcpSockfd, current_waittime ))
                {
                    // connection timed out or failed
                    MIRACASTLOG_ERROR("Socket Connection Timedout %s received(%d)...", strerror(errno), errno);
                }
                else
                {
                    // connection successful
                    // do something with the connected socket
                    MIRACASTLOG_INFO("Socket Connected Successfully ...\n");
                    ret = MIRACAST_OK;
                    is_connected = true;
                }
            }
        }
        else
        {
            MIRACASTLOG_INFO("Socket Connected Successfully ...\n");
            ret = MIRACAST_OK;
            is_connected = true;
        }
    }

    if (is_connected)
    {
        /*---Wait for socket connect to complete---*/
        num_ready = epoll_wait(m_epollfd, events, MAX_EPOLL_EVENTS, 1000 /*timeout*/);
        for (i = 0; i < num_ready; i++)
        {
            if (events[i].events & EPOLLOUT)
            {
                MIRACASTLOG_INFO("Socket(%d) %d connected (EPOLLOUT)", i, events[i].data.fd);
            }
        }

        num_ready = epoll_wait(m_epollfd, events, MAX_EPOLL_EVENTS, 1000 /*timeout*/);
        for (i = 0; i < num_ready; i++)
        {
            if (events[i].events & EPOLLOUT)
            {
                MIRACASTLOG_INFO("Socket %d got some data via EPOLLOUT", events[i].data.fd);
                break;
            }
        }
    }

    if ( MIRACAST_FAIL == ret )
    {
        Release_SocketAndEpollDescriptor();
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
        MIRACASTLOG_ERROR("Send Failed (%d)%s", errno, strerror(errno));
        return RTSP_MSG_FAILURE;
    }

    MIRACASTLOG_INFO("Sending the RTSP Msg [%d] Data[%s]\n", read_ret, rtsp_response_buffer.c_str());
    return RTSP_MSG_SUCCESS;
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_setparameter_request( std::string rtsp_msg_buffer )
{
    RTSP_STATUS status_code = RTSP_INVALID_MSG_RECEIVED;

    MIRACASTLOG_TRACE("Entering...");

    const char  *trigger_method_tag = get_parser_field_by_index(RTSP_TRIGGER_METHOD_FIELD);

    if (rtsp_msg_buffer.find(trigger_method_tag) != std::string::npos)
    {
        status_code = validate_rtsp_trigger_method_request(rtsp_msg_buffer);
    }
    else
    {
        // Processing M4 request and response back
        status_code = validate_rtsp_m4_response_back(rtsp_msg_buffer);
    }
    MIRACASTLOG_TRACE("Exiting...");

    return status_code;
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_getparameter_request( std::string rtsp_msg_buffer )
{
    RTSP_STATUS status_code = RTSP_MSG_FAILURE;
    MIRACASTLOG_TRACE("Entering...");
    const char *content_txt_tag = get_parser_field_by_index(RTSP_CONTENT_TEXT_FIELD);

    if (rtsp_msg_buffer.find(content_txt_tag) != std::string::npos)
    {
        status_code = validate_rtsp_m3_response_back(rtsp_msg_buffer);
    }
    else
    {
        // It looks get parameter without body. So consider it as keepalive M16
        std::string seq_str = "";
        std::stringstream ss(rtsp_msg_buffer);
        std::string prefix = "";
        std::string line;
        const char  *sequence_tag = get_parser_field_by_index(RTSP_SEQUENCE_FIELD);

        while (std::getline(ss, line))
        {
            if (line.find(sequence_tag) != std::string::npos)
            {
                prefix = sequence_tag;
                seq_str = line.substr(prefix.length());
                REMOVE_R(seq_str);
                REMOVE_N(seq_str);
                break;
            }
        }
        status_code = send_rtsp_reply_sink2src( RTSP_MSG_FMT_M16_RESPONSE , seq_str );
        if ( RTSP_MSG_SUCCESS == status_code )
        {
            // Overwriting the SUCCESS status as KEEP-ALIVE-MSG received to handle M16
            status_code = RTSP_KEEP_ALIVE_MSG_RECEIVED;
            MIRACASTLOG_INFO(" !!! RTSP_KEEP_ALIVE_MSG_RECEIVED OK !!!");
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
    return status_code;
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_options_request( std::string rtsp_msg_buffer )
{
    return validate_rtsp_m1_msg_m2_send_request(rtsp_msg_buffer);
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_generic_request_response( std::string rtsp_msg_buffer )
{
    RTSP_STATUS status_code = RTSP_INVALID_MSG_RECEIVED;

    MIRACASTLOG_TRACE("Entering...");

    std::string public_tag = get_parser_field_by_index(RTSP_PUBLIC_FIELD),
                transport_tag = get_parser_field_by_index(RTSP_TRANSPORT_FIELD),
                rtsp_version_tag = get_parser_field_by_index(RTSP_VERSION_FIELD);
    std::string received_seq_num = parse_received_parser_field_value( rtsp_msg_buffer , RTSP_SEQUENCE_FIELD);

    if (rtsp_msg_buffer.find(public_tag) != std::string::npos)
    {
        status_code = validate_rtsp_m2_request_ack(rtsp_msg_buffer);
    }
    else if (rtsp_msg_buffer.find(transport_tag) != std::string::npos){
        status_code = validate_rtsp_m6_ack_m7_send_request(rtsp_msg_buffer);
    }
    else if (rtsp_msg_buffer.find(get_errorcode_string(RTSP_ERRORCODE_OK)) != std::string::npos)
    {
        status_code = validate_rtsp_trigger_request_ack(rtsp_msg_buffer , received_seq_num );
    }
    else
    {
        if (rtsp_msg_buffer.find(rtsp_version_tag) != std::string::npos)
        {
            MIRACASTLOG_WARNING(" !!! Could be RTSP ERROR Reported !!!...",rtsp_msg_buffer.c_str());
            status_code = RTSP_MSG_SUCCESS;
        }
        else
        {
            MIRACASTLOG_ERROR("!!! [%s] has to be Handled properly CSeq[%s] !!!...",
                                rtsp_msg_buffer.c_str(),
                                received_seq_num.c_str());
            send_rtsp_reply_sink2src( RTSP_MSG_FMT_REPORT_ERROR , 
                                      received_seq_num, 
                                      RTSP_ERRORCODE_NOT_IMPLEMENTED );
            status_code = RTSP_METHOD_NOT_SUPPORTED;
        }
    }

    MIRACASTLOG_TRACE("Exiting...");
    return status_code;
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_m1_msg_m2_send_request(std::string rtsp_m1_msg_buffer)
{
    RTSP_STATUS status_code = RTSP_INVALID_MSG_RECEIVED;

    MIRACASTLOG_TRACE("Entering...");
    
    std::string m1_msg_resp_sink2src = "";
    MIRACASTLOG_INFO("M1 OPTIONS packet received");
    std::stringstream ss(rtsp_m1_msg_buffer);
    std::string prefix = "";
    std::string req_str;
    std::string seq_str;
    std::string line;
    const char *require_tag = get_parser_field_by_index(RTSP_REQUIRE_FIELD),
                *sequence_tag = get_parser_field_by_index(RTSP_SEQUENCE_FIELD);

    while (std::getline(ss, line))
    {
        if (line.find(require_tag) != std::string::npos)
        {
            prefix = require_tag;
            req_str = line.substr(prefix.length());
            REMOVE_R(req_str);
            REMOVE_N(req_str);
        }
        else if (line.find(sequence_tag) != std::string::npos)
        {
            prefix = sequence_tag;
            seq_str = line.substr(prefix.length());
            REMOVE_R(seq_str);
            REMOVE_N(seq_str);
        }
    }

    m1_msg_resp_sink2src = generate_request_response_msg(RTSP_MSG_FMT_M1_RESPONSE, seq_str, req_str);

    MIRACASTLOG_INFO("Sending the M1 response [%s]", m1_msg_resp_sink2src.c_str());

    status_code = send_rstp_msg(m_tcpSockfd, m1_msg_resp_sink2src);

    if (RTSP_MSG_SUCCESS == status_code)
    {
        std::string m2_msg_req_sink2src = "";
        MIRACASTLOG_INFO("M1 response sent\n");

        m2_msg_req_sink2src = generate_request_response_msg(RTSP_MSG_FMT_M2_REQUEST, empty_string, req_str);

        MIRACASTLOG_INFO("Sending the M2 request [%s]",m2_msg_req_sink2src.c_str());
        status_code = send_rstp_msg(m_tcpSockfd, m2_msg_req_sink2src);
        if (RTSP_MSG_SUCCESS == status_code)
        {
            MIRACASTLOG_INFO("M2 request sent\n");
            set_wait_timeout(m_wfd_src_res_timeout);
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
    MIRACASTLOG_TRACE("Exiting...");
    return (status_code);
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_m2_request_ack(std::string rtsp_m2_resp_ack_buffer)
{
    RTSP_STATUS status_code = RTSP_INVALID_MSG_RECEIVED;

    MIRACASTLOG_TRACE("Entering...");
    std::stringstream ss(rtsp_m2_resp_ack_buffer);
    std::string prefix = "";
    std::string public_str;
    std::string seq_str;
    std::string line;
    const char *public_tag = get_parser_field_by_index(RTSP_PUBLIC_FIELD),
                *sequence_tag = get_parser_field_by_index(RTSP_SEQUENCE_FIELD);
    bool    foundSequence = false;
    int processedBytes = 0,
        totalLen = rtsp_m2_resp_ack_buffer.length();

    while (std::getline(ss, line,'\n'))
    {
        processedBytes += line.length() + 1; // Add 1 for '\n'

        if (line.find(public_tag) != std::string::npos)
        {
            prefix = public_tag;
            public_str = line.substr(prefix.length());
            REMOVE_R(public_str);
            REMOVE_N(public_str);
            ss.get();
            processedBytes += 2; // Skip next '\r\n' line
            break;
        }
        else if (( false == foundSequence) &&(line.find(sequence_tag) != std::string::npos))
        {
            prefix = sequence_tag;
            seq_str = line.substr(prefix.length());
            REMOVE_R(seq_str);
            REMOVE_N(seq_str);
            foundSequence = true;
        }
    }

    if (0 == (get_RequestSequenceNumber().compare(seq_str)))
    {
        std::vector<std::string> requiredFields;

        for ( RTSP_PARSER_FIELDS parser_field = static_cast<RTSP_PARSER_FIELDS>(RTSP_M2_RESPONSE_VALIDATE_MARKER_START + 1);
              RTSP_M2_RESPONSE_VALIDATE_MARKER_END > parser_field; 
              parser_field = static_cast<RTSP_PARSER_FIELDS>(parser_field + 1) )
        {
            requiredFields.push_back(get_parser_field_by_index(parser_field));
            MIRACASTLOG_TRACE("[%s] stored as requiredFields for M2 response validation\n",get_parser_field_by_index(parser_field));
        }

        bool allRequiredFieldsPresent = true;

        for (const std::string& requiredField : requiredFields) {
            if (public_str.find(requiredField) == std::string::npos)
            {
                allRequiredFieldsPresent = false;
                MIRACASTLOG_ERROR("!!!! [%s] not present in the M2 Response[%s] !!!\n",
                                    requiredField.c_str(),
                                    public_str.c_str());
                break;
            }
        }

        if (allRequiredFieldsPresent){
            status_code = RTSP_MSG_SUCCESS;
            MIRACASTLOG_VERBOSE("[M2 ack OK]");
        }
    }
    else{
        MIRACASTLOG_ERROR("Invalid Sequence Number[%s] in M2 Response\n",seq_str.c_str());
    }

    if (  RTSP_MSG_SUCCESS != status_code )
    {
        send_rtsp_reply_sink2src( RTSP_MSG_FMT_REPORT_ERROR , seq_str, RTSP_ERRORCODE_BAD_REQUEST );
    }

    MIRACASTLOG_INFO("#### totalLen[%d] and processedBytes[%d] ####", totalLen,processedBytes);

    if ( 0 >= ( totalLen - processedBytes))
    {
        MIRACASTLOG_INFO("#### [M2-ack] only Received ####");
    }
    else
    {
        std::string m3_request_buffer = rtsp_m2_resp_ack_buffer.substr(processedBytes, totalLen - processedBytes);
        MIRACASTLOG_INFO("#### [M2-ack + M3 Req] Received M3[%s]  ####", m3_request_buffer.c_str());
        status_code = validate_rtsp_receive_buffer_handling(m3_request_buffer);
        MIRACASTLOG_INFO("#### Response[%#04X] ####", status_code);
    }
    set_wait_timeout(m_wfd_src_req_timeout);
    MIRACASTLOG_TRACE("Exiting...");
    return (status_code);
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_m3_response_back(std::string rtsp_m3_msg_buffer)
{
    RTSP_STATUS status_code = RTSP_INVALID_MSG_RECEIVED;
    MIRACASTLOG_TRACE("Entering...");
    MIRACASTLOG_INFO("M3 request received");

    std::string content_buffer = "";
    std::string m3_msg_resp_sink2src = "";
    std::string seq_str = "";
    std::stringstream ss(rtsp_m3_msg_buffer);
    std::string prefix = "";
    std::string line;
    std::string actual_parser_field = "",
                parser_field_value = "";
    const char  *sequence_tag = get_parser_field_by_index(RTSP_SEQUENCE_FIELD);

    while (std::getline(ss, line))
    {
        if (line.find(sequence_tag) != std::string::npos)
        {
            prefix = sequence_tag;
            seq_str = line.substr(prefix.length());
            REMOVE_R(seq_str);
            REMOVE_N(seq_str);
        }
        else if (line.find("wfd") != std::string::npos)
        {
            REMOVE_R(line);
            REMOVE_N(line);
            parser_field_value = get_parser_field_n_value_by_name(line);
            if (!parser_field_value.empty())
            {
                content_buffer.append(line);
                content_buffer.append(": ");
                content_buffer.append(parser_field_value);
                content_buffer.append(RTSP_CRLF_STR);
            }
            parser_field_value.clear();
            actual_parser_field.clear();
        }
    }

    m3_msg_resp_sink2src = generate_request_response_msg(RTSP_MSG_FMT_M3_RESPONSE, seq_str, content_buffer);

    MIRACASTLOG_VERBOSE("%s", m3_msg_resp_sink2src.c_str());

    status_code = send_rstp_msg(m_tcpSockfd, m3_msg_resp_sink2src);

    if (RTSP_MSG_SUCCESS == status_code)
    {
        MIRACASTLOG_INFO("Sending the M3 response");
    }
    else
    {
        MIRACASTLOG_ERROR("Sending the M3 response Failed");
    }
    MIRACASTLOG_TRACE("Exiting...");
    return (status_code);
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_m4_response_back(std::string rtsp_m4_msg_buffer)
{
    RTSP_STATUS status_code = RTSP_INVALID_MSG_RECEIVED;
    MIRACASTLOG_TRACE("Entering...");

    std::string m4_msg_resp_sink2src = "";
    std::string seq_str = "";
    std::string url = "";
    std::stringstream ss(rtsp_m4_msg_buffer);
    std::string prefix = "";
    std::string line;
    const char  *sequence_tag = get_parser_field_by_index(RTSP_SEQUENCE_FIELD),
                *url_tag = get_parser_field_by_index(RTSP_WFD_STREAMING_URL_FIELD);

    while (std::getline(ss, line))
    {
        if (line.find(sequence_tag) != std::string::npos)
        {
            prefix = sequence_tag;
            seq_str = line.substr(prefix.length());
            REMOVE_R(seq_str);
            REMOVE_N(seq_str);
        }
        else if (line.find(url_tag) != std::string::npos)
        {
            prefix = url_tag;
            std::size_t url_start_pos = line.find(prefix) + prefix.length();
            std::size_t url_end_pos = line.find(RTSP_SPACE_STR, url_start_pos);
            url = line.substr(url_start_pos, url_end_pos - url_start_pos);
            set_WFDPresentationURL(url);
        }
    }

    m4_msg_resp_sink2src = generate_request_response_msg( RTSP_MSG_FMT_M4_RESPONSE,seq_str,empty_string);

    MIRACASTLOG_INFO("Sending the M4 response");
    status_code = send_rstp_msg(m_tcpSockfd, m4_msg_resp_sink2src);
    if (RTSP_MSG_SUCCESS == status_code)
    {
        MIRACASTLOG_INFO("M4 response sent");
    }
    else
    {
        MIRACASTLOG_ERROR("Failed to sent M4 response");
    }
    MIRACASTLOG_TRACE("Exiting...");
    return (status_code);
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_m5_msg_m6_send_request(std::string rtsp_m5_msg_buffer)
{
    RTSP_STATUS status_code = RTSP_INVALID_MSG_RECEIVED;
    MIRACASTLOG_TRACE("Entering...");

    std::string m5_msg_resp_sink2src = "";
    std::string seq_str = "";
    std::stringstream ss(rtsp_m5_msg_buffer);
    std::string prefix = "";
    std::string line;
    const char  *sequence_tag = get_parser_field_by_index(RTSP_SEQUENCE_FIELD);

    while (std::getline(ss, line))
    {
        if (line.find(sequence_tag) != std::string::npos)
        {
            prefix = sequence_tag;
            seq_str = line.substr(prefix.length());
            REMOVE_R(seq_str);
            REMOVE_N(seq_str);
            break;
        }
    }

    m5_msg_resp_sink2src = generate_request_response_msg(RTSP_MSG_FMT_M5_RESPONSE, seq_str, empty_string);

    MIRACASTLOG_INFO("Sending the M5 response");
    status_code = send_rstp_msg(m_tcpSockfd, m5_msg_resp_sink2src);
    if (RTSP_MSG_SUCCESS == status_code)
    {
        MIRACASTLOG_INFO("M5 Response has sent");

        status_code = send_rtsp_reply_sink2src(RTSP_MSG_FMT_M6_REQUEST);
        set_wait_timeout(m_wfd_src_res_timeout);
    }
    else
    {
        MIRACASTLOG_ERROR("Failed to Send the M5 response");
    }
    MIRACASTLOG_TRACE("Exiting...");
    return (status_code);
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_m6_ack_m7_send_request(std::string rtsp_m6_ack_buffer)
{
    RTSP_STATUS status_code = RTSP_INVALID_MSG_RECEIVED;

    std::stringstream ss(rtsp_m6_ack_buffer);
    std::string line;
    std::string session_number = "",
                clientPortValue = "";
    int timeoutValue = -1;
    const char  *session_tag = get_parser_field_by_index(RTSP_SESSION_FIELD);

    MIRACASTLOG_TRACE("Entering...");

    while (std::getline(ss, line))
    {
        if (line.find(session_tag) != std::string::npos) {
            std::regex sessionRegex("Session: ([0-9a-fA-F]+)(?:;timeout=([0-9]+))?");
            std::smatch match;
            if (std::regex_search(line, match, sessionRegex) && match.size() == 3) {
                session_number = match[1];
                MIRACASTLOG_TRACE("Session Number[%s]\n",session_number.c_str());
                if (match.size() > 2 && match[2].matched)
                {
                    timeoutValue = std::stoi(match[2]);
                    MIRACASTLOG_INFO("timeoutValue[%d] in M6 ACK\n",timeoutValue);
                }
                else
                {
                    timeoutValue = RTSP_DFLT_KEEP_ALIVE_WAIT_TIMEOUT_SEC;
                    MIRACASTLOG_ERROR("Failed to obtain timeout from [%s] and configured the default value[%d]\n",
                                        line.c_str(),
                                        timeoutValue);
                }
            }
            else{
                MIRACASTLOG_ERROR("Failed to obtain Session and Timeout from [%s]\n",line.c_str());
            }
        }

        if (line.find("client_port=") != std::string::npos) {
            std::regex clientPortRegex("client_port=([0-9]+(?:-[0-9]+)?)");
            std::smatch match;
            if (std::regex_search(line, match, clientPortRegex) && match.size() > 1) {
                clientPortValue = match[1];
                MIRACASTLOG_TRACE("clientPortValue[%s]\n",clientPortValue.c_str());
            }
            else{
                MIRACASTLOG_ERROR("Failed to obtain client port from [%s]\n",line.c_str());
            }
        }
    }

    set_WFDSessionNumber(session_number);
    if ( -1 != timeoutValue )
    {
        m_wfd_src_session_timeout = timeoutValue;
    }

#ifndef MIRACAST_CERT_BUILD
    m_wfd_src_session_timeout += RTSP_KEEP_ALIVE_WAIT_TIMEOUT_OFFSET_SEC;
    MIRACASTLOG_INFO("SessionTimeout has updated as [%d]",m_wfd_src_session_timeout);
#endif

    if (!clientPortValue.empty())
    {
        status_code = send_rtsp_reply_sink2src(RTSP_MSG_FMT_M7_REQUEST);
        set_wait_timeout(m_wfd_src_res_timeout);
    }
    else
    {
        MIRACASTLOG_ERROR("clientPortValue looks empty.\n");
    }
    MIRACASTLOG_TRACE("Exiting...");
    return (status_code);
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_trigger_request_ack(std::string rtsp_trigger_req_ack_buffer , std::string received_seq_num )
{
    RTSP_STATUS status_code = RTSP_MSG_FAILURE;
    std::string reported_seq_num = get_RequestSequenceNumber();
    MIRACASTLOG_TRACE("Entering...");

    if (0 != (reported_seq_num.compare(received_seq_num)))
    {
        send_rtsp_reply_sink2src( RTSP_MSG_FMT_REPORT_ERROR , received_seq_num, RTSP_ERRORCODE_BAD_REQUEST );
        MIRACASTLOG_ERROR("Invalid Sequence Number received [%s] in trigger[%s]. Reported is [%s]",
                            received_seq_num.c_str(),
                            rtsp_trigger_req_ack_buffer.c_str(),
                            reported_seq_num.c_str());
    }
    else
    {
        if ( MIRACAST_PLAYER_STATE_INPROGRESS == get_state())
        {
            // It denotes response for M7 has received.
            status_code = RTSP_M1_M7_MSG_EXCHANGE_RECEIVED;
        }
        else
        {
            status_code = RTSP_MSG_SUCCESS;
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
    return status_code;
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_trigger_method_request(std::string rtsp_msg_buffer)
{
    RTSP_STATUS status_code = RTSP_MSG_FAILURE,
                sub_status_code = RTSP_MSG_FAILURE;
    const char  *setup_tag = get_parser_field_by_index(RTSP_SETUP_FIELD),
                *teardown_tag = get_parser_field_by_index(RTSP_TEARDOWN_FIELD),
                *play_tag = get_parser_field_by_index(RTSP_PLAY_FIELD),
                *pause_tag = get_parser_field_by_index(RTSP_PAUSE_FIELD);

    MIRACASTLOG_TRACE("Entering ...");
    
    if (rtsp_msg_buffer.find(setup_tag) != std::string::npos)
    {
        status_code = validate_rtsp_m5_msg_m6_send_request(rtsp_msg_buffer);
    }
    else
    {
        std::string received_seq_num = parse_received_parser_field_value(rtsp_msg_buffer , RTSP_SEQUENCE_FIELD);
        RTSP_ERRORCODES error_code = RTSP_ERRORCODE_OK;
        bool sink2src_resp_needed = true;

        if (rtsp_msg_buffer.find(teardown_tag) != std::string::npos)
        {
            sub_status_code = RTSP_MSG_TEARDOWN_REQUEST;
            MIRACASTLOG_INFO("TEARDOWN request from Source received\n");
        }
        else if (rtsp_msg_buffer.find(play_tag) != std::string::npos)
        {
            MIRACASTLOG_INFO("PLAY request from Source received\n");
            if ( MIRACAST_PLAYER_STATE_PLAYING == get_state())
            {
                error_code = RTSP_ERRORCODE_METHOD_NOT_VALID;
            }
        }
        else if (rtsp_msg_buffer.find(pause_tag) != std::string::npos)
        {
            MIRACASTLOG_INFO("PAUSE request from Source received\n");
            if ( MIRACAST_PLAYER_STATE_PAUSED == get_state()){
                error_code = RTSP_ERRORCODE_METHOD_NOT_VALID;
            }
        }
        else{
            sink2src_resp_needed = false;
        }

        if ( sink2src_resp_needed )
        {
            status_code = send_rtsp_reply_sink2src( RTSP_MSG_FMT_TRIGGER_METHODS_RESPONSE , 
                                                    received_seq_num , 
                                                    error_code );
            if ( RTSP_MSG_SUCCESS == status_code )
            {
                status_code = sub_status_code;
            }
        }
    }
    MIRACASTLOG_TRACE("Exiting ...");
    return status_code;
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_receive_buffer_handling(std::string rtsp_msg_buffer)
{
    MIRACASTLOG_TRACE("Entering...");
    const char  *options_tag = get_parser_field_by_index(RTSP_OPTIONS_REQ_FIELD),
                *get_parameter_tag = get_parser_field_by_index(RTSP_GET_PARAMETER_FIELD),
                *set_parameter_tag = get_parser_field_by_index(RTSP_SET_PARAMETER_FIELD);
    RTSP_STATUS status_code = RTSP_MSG_FAILURE;

    std::stringstream ss(rtsp_msg_buffer);
    std::string first_line = "";

    MIRACASTLOG_TRACE("Entering...");
    if (std::getline(ss, first_line))
    {
        if (first_line.find(options_tag) != std::string::npos)
        {
            status_code = validate_rtsp_options_request(rtsp_msg_buffer);
        }
        else if (first_line.find(get_parameter_tag) != std::string::npos)
        {
            status_code = validate_rtsp_getparameter_request(rtsp_msg_buffer);
        }
        else if (first_line.find(set_parameter_tag) != std::string::npos)
        {
            status_code = validate_rtsp_setparameter_request(rtsp_msg_buffer);
        }
        else
        {
            status_code = validate_rtsp_generic_request_response(rtsp_msg_buffer);
        }
    }
    MIRACASTLOG_TRACE("Exiting [%#04X]...",status_code);
    return status_code;
}

RTSP_STATUS MiracastRTSPMsg::send_rtsp_reply_sink2src( RTSP_MSG_FMT_SINK2SRC req_fmt , std::string received_seq_num , RTSP_ERRORCODES error_code )
{
    RTSP_STATUS status_code = RTSP_MSG_FAILURE;
    
    MIRACASTLOG_TRACE("Entering...");

    switch (req_fmt)
    {
        case RTSP_MSG_FMT_M6_REQUEST:
        case RTSP_MSG_FMT_M7_REQUEST:
        case RTSP_MSG_FMT_PAUSE_REQUEST:
        case RTSP_MSG_FMT_PLAY_REQUEST:
        case RTSP_MSG_FMT_TEARDOWN_REQUEST:
        case RTSP_MSG_FMT_M16_RESPONSE:
        case RTSP_MSG_FMT_REPORT_ERROR:
        case RTSP_MSG_FMT_TRIGGER_METHODS_RESPONSE:
        {
            std::string rtsp_request_buffer = "";
            rtsp_request_buffer = generate_request_response_msg(req_fmt, received_seq_num , empty_string , error_code );

            MIRACASTLOG_INFO("Sending the RTSP Msg for [%#04X] format\n",req_fmt);
            status_code = send_rstp_msg(m_tcpSockfd, rtsp_request_buffer);
            if (RTSP_MSG_SUCCESS == status_code)
            {
                MIRACASTLOG_VERBOSE("RTSP Msg has sent\n");
            }
            else
            {
                MIRACASTLOG_ERROR("Failed to Send the RTSP Msg\n");
            }
        }
        break;
        default:
        {
            MIRACASTLOG_ERROR("Invalid Request [%#04X] format\n",req_fmt);
        }
        break;
    }
    MIRACASTLOG_TRACE("Exiting...");

    return status_code;
}

RTSP_STATUS MiracastRTSPMsg::validate_rtsp_post_m1_m7_xchange(std::string rtsp_post_m1_m7_xchange_buffer)
{
    return validate_rtsp_receive_buffer_handling(rtsp_post_m1_m7_xchange_buffer);
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
        status_code = send_rtsp_reply_sink2src( request_mode );

        if ( RTSP_MSG_SUCCESS == status_code ){
            MiracastGstPlayer *miracastGstPlayerObj = MiracastGstPlayer::getInstance();

            if ( RTSP_MSG_FMT_PLAY_REQUEST == request_mode )
            {
                miracastGstPlayerObj->resume();
                MIRACASTLOG_INFO("GstPlayback resumed...");
            }
            else if ( RTSP_MSG_FMT_PAUSE_REQUEST == request_mode )
            {
                miracastGstPlayerObj->pause();
                MIRACASTLOG_INFO("GstPlayback paused...");
            }
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
    return status_code;
}

std::string MiracastRTSPMsg::get_localIp()
{
    MIRACASTLOG_TRACE("Entering...");
    
    MIRACASTLOG_TRACE("Exiting...");
    return m_sink_ip;
}

MiracastError MiracastRTSPMsg::start_streaming( VIDEO_RECT_STRUCT video_rect )
{
    MIRACASTLOG_TRACE("Entering...");
#if defined(PLATFORM_AMLOGIC)
    {
        char command[128] = {0};
        std::string default_error_proc_policy = "2151665463";
        std::ifstream decoder_error_proc_policy_file("/opt/aml_dec_error_proc_policy");

        if (decoder_error_proc_policy_file.is_open())
        {
            std::string new_error_proc_policy = "";
            std::getline(decoder_error_proc_policy_file, new_error_proc_policy);
            decoder_error_proc_policy_file.close();

            MIRACASTLOG_VERBOSE("decoder_error_proc_policy_file reading from file [/opt/aml_dec_error_proc_policy], new_error_proc_policy as [%s] ",
                                new_error_proc_policy.c_str());
            MIRACASTLOG_VERBOSE("Overwriting error_proc_policy default[%s] with new[%s]",
                                default_error_proc_policy.c_str(),
                                new_error_proc_policy.c_str());
            default_error_proc_policy = new_error_proc_policy;
        }

        if ( ! default_error_proc_policy.empty())
        {
            sprintf(command, "echo %s > /sys/module/amvdec_mh264/parameters/error_proc_policy",
                    default_error_proc_policy.c_str());

            MIRACASTLOG_INFO("command for applying error_proc_policy[%s]",command);
            if (0 == system(command))
            {
                MIRACASTLOG_INFO("error_proc_policy applied successfully");
            }
            else
            {
                MIRACASTLOG_ERROR("!!! Failed to apply error_proc_policy !!!");
            }
        }
    }
#endif
    std::string gstreamerPipeline = "";
    const char *mcastfile = "/opt/mcastgstpipline.txt";
    std::ifstream mcgstfile(mcastfile);

    if (mcgstfile.is_open())
    {
        std::getline(mcgstfile, gstreamerPipeline);
        MIRACASTLOG_VERBOSE("gstpipeline reading from file [%s], gstreamerPipeline as [ %s] ", mcastfile, gstreamerPipeline.c_str());
        mcgstfile.close();
        if (0 == system(gstreamerPipeline.c_str()))
            MIRACASTLOG_VERBOSE("Pipeline created successfully ");
        else
        {
            MIRACASTLOG_ERROR("Pipeline creation failure");
            return MIRACAST_FAIL;
        }
    }
    else
    {
        if (access("/opt/miracast_gst", F_OK) == 0)
        {
            gstreamerPipeline = "GST_DEBUG=3 gst-launch-1.0 -vvv playbin uri=udp://0.0.0.0:1990 video-sink=\"westerossink\"";
            MIRACASTLOG_VERBOSE("pipeline constructed is --> %s", gstreamerPipeline.c_str());
            if (0 == system(gstreamerPipeline.c_str()))
                MIRACASTLOG_VERBOSE("Pipeline created successfully ");
            else
            {
                MIRACASTLOG_ERROR("Pipeline creation failure");
                return MIRACAST_FAIL;
            }
        }
        else
        {
            MiracastGstPlayer *miracastGstPlayerObj = MiracastGstPlayer::getInstance();
            std::string port = get_WFDStreamingPortNumber();
            std::string local_ip = get_localIp();
            miracastGstPlayerObj->setVideoRectangle( video_rect );
            miracastGstPlayerObj->launch(local_ip, port,this);
        }
    }
    m_streaming_started = true;
    MIRACASTLOG_TRACE("Exiting...");
    return MIRACAST_OK;
}

MiracastError MiracastRTSPMsg::stop_streaming( eMIRA_PLAYER_STATES state )
{
    MIRACASTLOG_TRACE("Entering...");

    if ((MIRACAST_PLAYER_STATE_STOPPED == state)||
        (MIRACAST_PLAYER_STATE_SELF_ABORT == state))
    {
        if (m_streaming_started)
        {
            if (MIRACAST_PLAYER_STATE_SELF_ABORT == state)
            {
                MiracastGstPlayer::destroyInstance();
                MIRACASTLOG_INFO("MiracastGstPlayer instance destroyed...");
            }
            else
            {
                MiracastGstPlayer *miracastGstPlayerObj = MiracastGstPlayer::getInstance();
                miracastGstPlayerObj->stop();
                MIRACASTLOG_INFO("MiracastGstPlayer instance stopped...");
            }
            m_streaming_started = false;
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
    return MIRACAST_OK;
}

MiracastError MiracastRTSPMsg::updateVideoRectangle( VIDEO_RECT_STRUCT videorect )
{
    MIRACASTLOG_TRACE("Entering...");

    MiracastGstPlayer *miracastGstPlayerObj = MiracastGstPlayer::getInstance();
    miracastGstPlayerObj->setVideoRectangle( videorect , true );

    MIRACASTLOG_TRACE("Exiting...");
    return MIRACAST_OK;
}

void MiracastRTSPMsg::RTSPMessageHandler_Thread(void *args)
{
    char rtsp_message_socket[4096] = {0};
    RTSP_HLDR_MSGQ_STRUCT rtsp_message_data = {};
    VIDEO_RECT_STRUCT     video_rect_st = {0};
    RTSP_STATUS status_code = RTSP_TIMEDOUT;
    std::string rtsp_msg_buffer;
    std::string client_mac = "",
                client_name = "",
                go_ip_addr = "";
    bool    start_monitor_keep_alive_msg = false,
            rtsp_msg_hldr_running_state = true;

    MIRACASTLOG_TRACE("Entering...");

    while ((nullptr != m_rtsp_msg_handler_thread)&&(true == rtsp_msg_hldr_running_state))
    {
        set_state(MIRACAST_PLAYER_STATE_IDLE);
        MIRACASTLOG_TRACE("Waiting for Event .....\n");
        m_rtsp_msg_handler_thread->receive_message(&rtsp_message_data, sizeof(rtsp_message_data), THREAD_RECV_MSG_INDEFINITE_WAIT);

        MIRACASTLOG_INFO("Received Action[%#04X]\n", rtsp_message_data.state);

        if (RTSP_SELF_ABORT == rtsp_message_data.state)
        {
            MIRACASTLOG_INFO("RTSP_SELF_ABORT ACTION Received\n");
            rtsp_msg_hldr_running_state = false;
            set_state(MIRACAST_PLAYER_STATE_STOPPED);
            continue;
        }

        if ( RTSP_START_RECEIVE_MSGS == rtsp_message_data.state )
        {
            MIRACASTLOG_INFO("RTSP_START_RECEIVE_MSGS ACTION Received\n");
            store_srcsink_info( rtsp_message_data.source_dev_name , 
                                rtsp_message_data.source_dev_mac ,
                                rtsp_message_data.source_dev_ip,
                                rtsp_message_data.sink_dev_ip );

            MIRACASTLOG_INFO("dev_name[%s]dev_mac[%s]dev_ip[%s]sink_ip[%s]",
                                rtsp_message_data.source_dev_name,
                                rtsp_message_data.source_dev_mac,
                                rtsp_message_data.source_dev_ip,
                                rtsp_message_data.sink_dev_ip);
            MIRACASTLOG_INFO("VideoRect[%d,%d,%d,%d]",
                                rtsp_message_data.videorect.startX,
                                rtsp_message_data.videorect.startY,
                                rtsp_message_data.videorect.width,
                                rtsp_message_data.videorect.height);
            
            video_rect_st.startX = rtsp_message_data.videorect.startX;
            video_rect_st.startY = rtsp_message_data.videorect.startY;
            video_rect_st.width = rtsp_message_data.videorect.width;
            video_rect_st.height = rtsp_message_data.videorect.height;

            set_state( MIRACAST_PLAYER_STATE_INITIATED , true );

            if (MIRACAST_OK != initiate_TCP(rtsp_message_data.source_dev_ip))
            {
                set_state( MIRACAST_PLAYER_STATE_STOPPED , true , MIRACAST_PLAYER_REASON_CODE_RTSP_ERROR );
                continue;
            }
        }
        else
        {
            // NEED to define ERROR code
            set_state( MIRACAST_PLAYER_STATE_STOPPED , true , MIRACAST_PLAYER_REASON_CODE_INT_FAILURE );
            MIRACASTLOG_ERROR("[%#04X] action received and not yet handled\n", rtsp_message_data.state);
            continue;
        }
        set_state( MIRACAST_PLAYER_STATE_INPROGRESS , true );

        set_wait_timeout(m_wfd_src_req_timeout);

        memset(&rtsp_message_socket, 0x00, sizeof(rtsp_message_socket));

        while (( status_code = receive_buffer_timedOut( m_tcpSockfd, rtsp_message_socket, sizeof(rtsp_message_socket),get_wait_timeout())) &&
                ( status_code == RTSP_MSG_SUCCESS ))
        {
            rtsp_msg_buffer.clear();
            rtsp_msg_buffer = rtsp_message_socket;

            MIRACASTLOG_INFO("#### [M1-M7] RTSP SockMsg Received [%s] ####", rtsp_msg_buffer.c_str());
            status_code = validate_rtsp_receive_buffer_handling(rtsp_msg_buffer);
            MIRACASTLOG_INFO("#### [M1-M7] RTSP Response[%#04X] ####", status_code);

            if ((RTSP_MSG_SUCCESS != status_code) || 
                (RTSP_M1_M7_MSG_EXCHANGE_RECEIVED == status_code))
            {
                break;
            }
            memset(&rtsp_message_socket, 0x00, sizeof(rtsp_message_socket));

            if (true == m_rtsp_msg_handler_thread->receive_message(&rtsp_message_data, sizeof(rtsp_message_data), THREAD_RECV_MSG_WAIT_IMMEDIATE))
            {
                if (( RTSP_SELF_ABORT == rtsp_message_data.state ) ||
                    ( RTSP_TEARDOWN_FROM_SINK2SRC == rtsp_message_data.state ))
                {
                    MIRACASTLOG_WARNING("TEARDOWN Initiated during M1-M7 exchange[%#04X]\n", rtsp_message_data.state);
                    status_code = RTSP_MSG_TEARDOWN_REQUEST;
                    if ( RTSP_SELF_ABORT == rtsp_message_data.state )
                    {
                        rtsp_msg_hldr_running_state = false;
                    }
                    break;
                }
                else
                {
                    MIRACASTLOG_WARNING("Yet to Handle RTSP Msg Action[%#04X]\n", rtsp_message_data.state);
                }
            }
        }

        start_monitor_keep_alive_msg = false;

        if ((RTSP_MSG_SUCCESS == status_code) || (RTSP_M1_M7_MSG_EXCHANGE_RECEIVED == status_code ))
        {
            MIRACASTLOG_INFO("!!!! RTSP_M1_M7_MSG_EXCHANGE_RECEIVED[%#04X] !!!!", status_code);
            start_monitor_keep_alive_msg = true;
            start_streaming(video_rect_st);
            MIRACASTLOG_INFO("!!!! GstPlayer instance created, Waiting for first-frame !!!!");
            //set_state(MIRACAST_PLAYER_STATE_PLAYING , true );
        }
        else
        {
            eM_PLAYER_REASON_CODE reason = MIRACAST_PLAYER_REASON_CODE_RTSP_ERROR;

            if (RTSP_INVALID_MSG_RECEIVED == status_code)
            {
                MIRACASTLOG_ERROR("!!! INVALID RTSP MSG RECEIVED !!!\n");
            }
            else if (RTSP_MSG_FAILURE == status_code)
            {
                MIRACASTLOG_ERROR("!!! RTSP SENT/RECV FAILED !!!\n");
            }
            else if ( RTSP_MSG_TEARDOWN_REQUEST == status_code )
            {
                if ( MIRACAST_PLAYER_APP_REQ_TO_STOP_ON_EXIT == rtsp_message_data.stop_reason_code )
                {
                    reason = MIRACAST_PLAYER_REASON_CODE_APP_REQ_TO_STOP;
                    MIRACASTLOG_INFO("!!! APP REQUESTED TO STOP ON EXIT !!!\n");
                }
                else if ( MIRACAST_PLAYER_APP_REQ_TO_STOP_ON_NEW_CONNECTION == rtsp_message_data.stop_reason_code )
                {
                    reason = MIRACAST_PLAYER_REASON_CODE_NEW_SRC_DEV_CONNECT_REQ;
                    MIRACASTLOG_INFO("!!! APP REQUESTED TO STOP ON NEW CONNECTION !!!\n");
                }
            }
            else if ( RTSP_METHOD_NOT_SUPPORTED == status_code )
            {
                reason = MIRACAST_PLAYER_REASON_CODE_RTSP_METHOD_NOT_SUPPORTED;
                MIRACASTLOG_ERROR("!!! RTSP METHOD NOT SUPPORTED !!!\n");
            }
            else
            {
                reason = MIRACAST_PLAYER_REASON_CODE_RTSP_TIMEOUT;
                MIRACASTLOG_INFO("!!! RTSP RECV TIMEOUT !!!\n");
            }
            set_state(MIRACAST_PLAYER_STATE_STOPPED , true , reason );
        }

        RTSP_STATUS socket_state;
        struct timespec start_time, current_time;
        int elapsed_seconds = 0;

        //set_state(RTSP_STATE_M1_M7_EXCHANGE_DONE);
        clock_gettime(CLOCK_REALTIME, &start_time);

        while (true == start_monitor_keep_alive_msg)
        {
            clock_gettime(CLOCK_REALTIME, &current_time);

            elapsed_seconds = current_time.tv_sec - start_time.tv_sec;
            if (elapsed_seconds > m_wfd_src_session_timeout)
            {
                MIRACASTLOG_INFO("!!! RTSP M16 TIMEOUT[%d] Elapsed[%d]!!!",m_wfd_src_session_timeout,elapsed_seconds);
                set_state(MIRACAST_PLAYER_STATE_STOPPED , true , MIRACAST_PLAYER_REASON_CODE_RTSP_TIMEOUT );
                break;
            }

            memset(&rtsp_message_socket, 0x00, sizeof(rtsp_message_socket));
            socket_state = receive_buffer_timedOut( m_tcpSockfd,
                                                    rtsp_message_socket,
                                                    sizeof(rtsp_message_socket),
                                                    RTSP_KEEP_ALIVE_POLL_WAIT_TIMEOUT );
            if (RTSP_MSG_SUCCESS == socket_state)
            {
                rtsp_msg_buffer.clear();
                rtsp_msg_buffer = rtsp_message_socket;

                MIRACASTLOG_INFO("#### [POST_M1-M7] RTSP SockMsg Received [%s] ####", rtsp_msg_buffer.c_str());
                status_code = validate_rtsp_post_m1_m7_xchange(rtsp_msg_buffer);
                MIRACASTLOG_INFO("#### [POST_M1-M7] RTSP Response[%#04X] ####",status_code);

                if ( RTSP_KEEP_ALIVE_MSG_RECEIVED == status_code )
                {
                    // Refresh the Keep Alive Time
                    clock_gettime(CLOCK_REALTIME, &start_time);
                    MIRACASTLOG_INFO("!!! Refresh the Keep Alive Time !!!");
                }
                else if ((RTSP_MSG_TEARDOWN_REQUEST == status_code)||
                        ( RTSP_METHOD_NOT_SUPPORTED == status_code ))
                {
                    eM_PLAYER_REASON_CODE reason = MIRACAST_PLAYER_REASON_CODE_RTSP_METHOD_NOT_SUPPORTED;

                    if (RTSP_MSG_TEARDOWN_REQUEST == status_code)
                    {
                        reason = MIRACAST_PLAYER_REASON_CODE_SRC_DEV_REQ_TO_STOP;
                        MIRACASTLOG_INFO("!!! RTSP SRC DEV REQUESTED TO STOP !!!\n");
                    }
                    else
                    {
                        MIRACASTLOG_ERROR("!!! RTSP METHOD NOT SUPPORTED !!!\n");
                    }
                    set_state(MIRACAST_PLAYER_STATE_STOPPED , true , reason );
                    break;
                }
                else if ((RTSP_MSG_SUCCESS != status_code) &&
                        (RTSP_MSG_TEARDOWN_REQUEST != status_code))
                {
                    set_state(MIRACAST_PLAYER_STATE_STOPPED , true , MIRACAST_PLAYER_REASON_CODE_RTSP_ERROR );
                    MIRACASTLOG_ERROR("!!! RTSP SENT/RECV FAILED !!!\n");
                    break;
                }
            }
            else if (RTSP_MSG_FAILURE == socket_state)
            {
                set_state(MIRACAST_PLAYER_STATE_STOPPED , true , MIRACAST_PLAYER_REASON_CODE_RTSP_ERROR );
                MIRACASTLOG_ERROR("!!! RTSP SEND/RECV FAILED !!!\n");
                break;
            }

            MIRACASTLOG_TRACE("Waiting for Event .....\n");
            if (true == m_rtsp_msg_handler_thread->receive_message(&rtsp_message_data, sizeof(rtsp_message_data), 1))
            {
                MIRACASTLOG_INFO("Received Action[%#04X]\n", rtsp_message_data.state);
                switch (rtsp_message_data.state)
                {
                    case RTSP_RESTART:
                    {
                        MIRACASTLOG_INFO("[RTSP_RESTART] ACTION Received\n");
                        start_monitor_keep_alive_msg = false;
                    }
                    break;
                    case RTSP_SELF_ABORT:
                    case RTSP_PLAY_FROM_SINK2SRC:
                    case RTSP_PAUSE_FROM_SINK2SRC:
                    case RTSP_TEARDOWN_FROM_SINK2SRC:
                    {
                        if (RTSP_SELF_ABORT == rtsp_message_data.state){
                            MIRACASTLOG_INFO("[RTSP_SELF_ABORT] Received\n");
                            rtsp_msg_hldr_running_state = false;
                            set_state(MIRACAST_PLAYER_STATE_SELF_ABORT , true );
                        }
                        else
                        {
                            if ( RTSP_PLAY_FROM_SINK2SRC == rtsp_message_data.state )
                            {
                                set_state(MIRACAST_PLAYER_STATE_PLAYING );
                                MIRACASTLOG_INFO("[RTSP_PLAY] ACTION Received\n");
                            }
                            else if ( RTSP_PAUSE_FROM_SINK2SRC == rtsp_message_data.state )
                            {
                                set_state(MIRACAST_PLAYER_STATE_PAUSED );
                                MIRACASTLOG_INFO("[RTSP_PAUSE] ACTION Received\n");
                            }
                            else if ( RTSP_TEARDOWN_FROM_SINK2SRC == rtsp_message_data.state )
                            {
                                MIRACASTLOG_INFO("[RTSP_TEARDOWN] ACTION Received\n");
                            }
                        }
                        if ((RTSP_MSG_FAILURE == rtsp_sink2src_request_msg_handling(rtsp_message_data.state)) ||
                            (RTSP_TEARDOWN_FROM_SINK2SRC == rtsp_message_data.state))
                        {
                            eM_PLAYER_REASON_CODE reason_code = MIRACAST_PLAYER_REASON_CODE_SUCCESS;
                            start_monitor_keep_alive_msg = false;

                            if (RTSP_TEARDOWN_FROM_SINK2SRC == rtsp_message_data.state)
                            {
                                if ( MIRACAST_PLAYER_APP_REQ_TO_STOP_ON_EXIT == rtsp_message_data.stop_reason_code )
                                {
                                    reason_code = MIRACAST_PLAYER_REASON_CODE_APP_REQ_TO_STOP;
                                    MIRACASTLOG_INFO("!!! APP REQUESTED TO STOP ON EXIT!!!\n");
                                }
                                else if ( MIRACAST_PLAYER_APP_REQ_TO_STOP_ON_NEW_CONNECTION == rtsp_message_data.stop_reason_code )
                                {
                                    reason_code = MIRACAST_PLAYER_REASON_CODE_NEW_SRC_DEV_CONNECT_REQ;
                                    MIRACASTLOG_INFO("!!! APP REQUESTED TO STOP ON NEW CONNECTION !!!\n");
                                }
                                else
                                {
                                    MIRACASTLOG_ERROR("!!! INVALID STOP REASON RECEIVED [%#04X] !!!\n",rtsp_message_data.stop_reason_code);
                                }
                            }
                            else
                            {
                                MIRACASTLOG_ERROR("!!! INVALID ACTION RECEIVED [%#04X] !!!\n",rtsp_message_data.state);
                            }
                            set_state(MIRACAST_PLAYER_STATE_STOPPED , true , reason_code );
                        }
                    }
                    break;
                    case RTSP_UPDATE_VIDEO_RECT:
                    {
                        // Video Rectangle updation after successful streaming to be taken here
                        VIDEO_RECT_STRUCT videorect = {0};
                        videorect.startX = rtsp_message_data.videorect.startX;
                        videorect.startY = rtsp_message_data.videorect.startY;
                        videorect.width = rtsp_message_data.videorect.width;
                        videorect.height = rtsp_message_data.videorect.height;
                        MIRACASTLOG_INFO("!!! RTSP_UPDATE_VIDEO_RECT[%d,%d,%d,%d] !!!\n",
                                            videorect.startX,
                                            videorect.startY,
                                            videorect.width,
                                            videorect.height);
                        updateVideoRectangle(videorect);
                    }
                    break;
                    case RTSP_NOTIFY_GSTPLAYER_STATE:
                    {
                        MIRACASTLOG_INFO("!!! RTSP_NOTIFY_GSTPLAYER_STATE[%#08X] !!!\n",rtsp_message_data.gst_player_state);
                        if ( MIRACAST_GSTPLAYER_STATE_FIRST_VIDEO_FRAME_RECEIVED == rtsp_message_data.gst_player_state )
                        {
                            set_state(MIRACAST_PLAYER_STATE_PLAYING , true );
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

        Release_SocketAndEpollDescriptor();
    }
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastRTSPMsg::send_msgto_controller_thread(eCONTROLLER_FW_STATES state)
{
    MIRACASTLOG_TRACE("Entering...");
    if ( nullptr != m_controller_thread )
    {
        CONTROLLER_MSGQ_STRUCT controller_msgq_data = {0};
        controller_msgq_data.state = state;
        controller_msgq_data.msg_type = RTSP_MSG;
        m_controller_thread->send_message(&controller_msgq_data, CONTROLLER_MSGQ_SIZE);
    }
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastRTSPMsg::send_msgto_rtsp_msg_hdler_thread(RTSP_HLDR_MSGQ_STRUCT rtsp_hldr_msgq_data)
{
    MIRACASTLOG_TRACE("Entering...");
    if (nullptr != m_rtsp_msg_handler_thread)
    {
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
void MiracastRTSPMsg::DumpBuffer(char *buffer, int length)
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

void MiracastRTSPMsg::HDCPTCPServerHandlerThread(void *args)
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
    MiracastRTSPMsg *miracast_ctrler_obj = (MiracastRTSPMsg *)args;

    miracast_ctrler_obj->HDCPTCPServerHandlerThread(nullptr);
}
#endif /*ENABLE_HDCP2X_SUPPORT*/

#ifdef ENABLE_MIRACAST_PLAYER_TEST_NOTIFIER

MiracastError MiracastRTSPMsg::create_TestNotifier(void)
{
    MiracastError error_code = MIRACAST_OK;

    m_test_notifier_thread = nullptr;
    m_test_notifier_thread = new MiracastThread( MIRACAST_PLAYER_TEST_NOTIFIER_THREAD_NAME,
                                                MIRACAST_PLAYER_TEST_NOTIFIER_THREAD_STACK,
                                                MIRACAST_PLAYER_TEST_NOTIFIER_MSG_COUNT,
                                                MIRACAST_PLAYER_TEST_NOTIFIER_MSGQ_SIZE,
                                                reinterpret_cast<void (*)(void *)>(&MiracastPlayerTestNotifierThreadCallback),
                                                this);
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

void MiracastRTSPMsg::destroy_TestNotifier()
{
    MIRACAST_PLAYER_TEST_NOTIFIER_MSGQ_ST stMsgQ = {0};

    MIRACASTLOG_TRACE("Entering...");
    stMsgQ.state = MIRACAST_PLAYER_TEST_NOTIFIER_SHUTDOWN;
    m_rtsp_msg_obj->send_msgto_test_notifier_thread(stMsgQ);
    if (nullptr != m_test_notifier_thread)
    {
        delete m_test_notifier_thread;
        m_test_notifier_thread = nullptr;
    }
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastRTSPMsg::TestNotifier_Thread(void *args)
{
    MIRACAST_PLAYER_TEST_NOTIFIER_MSGQ_ST stMsgQ = {0};
    std::string device_name = "",
                mac_address = "",
                source_dev_ip = "",
                sink_dev_ip = "";

    MIRACASTLOG_TRACE("Entering...");

    while ( nullptr != m_test_notifier_thread )
    {
        memset( &stMsgQ , 0x00 , MIRACAST_PLAYER_TEST_NOTIFIER_MSGQ_SIZE );

        MIRACASTLOG_TRACE("!!! WAITING FOR NEW ACTION !!!\n");

        m_test_notifier_thread->receive_message(&stMsgQ, MIRACAST_PLAYER_TEST_NOTIFIER_MSGQ_SIZE , THREAD_RECV_MSG_INDEFINITE_WAIT);

        MIRACASTLOG_TRACE("!!! Received Action[%#08X] !!!\n", stMsgQ.state);

        device_name = stMsgQ.src_dev_name;
        mac_address = stMsgQ.src_dev_mac_addr;

        switch (stMsgQ.state)
        {
            case MIRACAST_PLAYER_TEST_NOTIFIER_STATE_CHANGED:
            {
                MIRACASTLOG_TRACE("[MIRACAST_PLAYER_TEST_NOTIFIER_STATE_CHANGED]...");
                m_player_notify_handler->onStateChange( mac_address ,
                                                        device_name ,
                                                        stMsgQ.player_state,
                                                        stMsgQ.reason_code );
            }
            break;
            case MIRACAST_PLAYER_TEST_NOTIFIER_SHUTDOWN:
            {
                MIRACASTLOG_TRACE("[MIRACAST_PLAYER_TEST_NOTIFIER_SHUTDOWN]...");
            }
            break;
            default:
            {
                MIRACASTLOG_ERROR("[UNKNOWN STATE]...");
            }
            break;
        }

        if (MIRACAST_PLAYER_TEST_NOTIFIER_SHUTDOWN == stMsgQ.state)
        {
            break;
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastPlayerTestNotifierThreadCallback(void *args)
{
    MiracastRTSPMsg *miracast_rtsp_obj = (MiracastRTSPMsg *)args;
    MIRACASTLOG_TRACE("Entering...");
    if ( nullptr != miracast_rtsp_obj )
    {
        miracast_rtsp_obj->TestNotifier_Thread(nullptr);
    }
    MIRACASTLOG_TRACE("Exiting...");
}

void MiracastRTSPMsg::send_msgto_test_notifier_thread(MIRACAST_PLAYER_TEST_NOTIFIER_MSGQ_ST stMsgQ)
{
    MIRACASTLOG_TRACE("Entering...");
    if (nullptr != m_test_notifier_thread)
    {
        m_test_notifier_thread->send_message(&stMsgQ, MIRACAST_PLAYER_TEST_NOTIFIER_MSGQ_SIZE);
    }
    MIRACASTLOG_TRACE("Exiting...");
}
#endif /* ENABLE_MIRACAST_PLAYER_TEST_NOTIFIER */
