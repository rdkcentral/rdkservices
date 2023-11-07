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

#ifndef _MIRACAST_RSTP_MSG_H_
#define _MIRACAST_RTSP_MSG_H_

#include <MiracastCommon.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>

#define MAX_EPOLL_EVENTS 64
#define RTSP_REQUEST_RECV_TIMEOUT   ( 6 * ONE_SECOND_IN_MILLISEC )
#define RTSP_RESPONSE_RECV_TIMEOUT  ( 5 * ONE_SECOND_IN_MILLISEC )
#define SOCKET_DFLT_WAIT_TIMEOUT    ( 30 * ONE_SECOND_IN_MILLISEC )
#define RTSP_DFLT_KEEP_ALIVE_WAIT_TIMEOUT_SEC   ( 60 )
#define RTSP_KEEP_ALIVE_POLL_WAIT_TIMEOUT   ( ONE_SECOND_IN_MILLISEC )
#define RTSP_KEEP_ALIVE_WAIT_TIMEOUT_OFFSET_SEC   ( 20 )
#define RTSP_REQ_RESP_RECV_TIMEOUT_OFFSET_MSEC   ( 10 * ONE_SECOND_IN_MILLISEC )

typedef enum rtsp_status_e
{
    RTSP_MSG_FAILURE,
    RTSP_MSG_SUCCESS,
    RTSP_INVALID_MSG_RECEIVED,
    RTSP_MSG_TEARDOWN_REQUEST,
    RTSP_M1_M7_MSG_EXCHANGE_RECEIVED,
    RTSP_KEEP_ALIVE_MSG_RECEIVED,
    RTSP_TIMEDOUT,
    RTSP_METHOD_NOT_SUPPORTED
} RTSP_STATUS;

#if 0
#define RTSP_CRLF_STR "\r\n"
#define RTSP_DOUBLE_QUOTE_STR "\""
#define RTSP_SPACE_STR SPACE_CHAR
#define RTSP_SEMI_COLON_STR ";"

/* It will be used to parse the data from WFD Source */
#define RTSP_STD_REQUEST_STR "RTSP/1.0" RTSP_CRLF_STR
#define RTSP_REQ_OPTIONS "OPTIONS * " RTSP_STD_REQUEST_STR
#define RTSP_TRIGGER_METHOD_FIELD   "wfd_trigger_method: "

#define RTSP_STD_SEQUENCE_FIELD "CSeq: "
#define RTSP_STD_REQUIRE_FIELD "Require: "
#define RTSP_STD_SESSION_FIELD "Session: "

#define RTSP_REQ_PLAY_MODE "PLAY"
#define RTSP_REQ_PAUSE_MODE "PAUSE"
#define RTSP_REQ_TEARDOWN_MODE "TEARDOWN"

#define RTSP_STD_UNICAST_FIELD "unicast"

#define RTSP_WFD_CONTENT_PROTECT_FIELD "wfd_content_protection: "
#define RTSP_WFD_VIDEO_FMT_FIELD "wfd_video_formats: "
#define RTSP_WFD_AUDIO_FMT_FIELD "wfd_audio_codecs: "
#define RTSP_WFD_CLIENT_PORTS_FIELD "wfd_client_rtp_ports: "
#define RTSP_WFD_PRESENTATION_URL_FIELD "wfd_presentation_URL: "

#define RTSP_M16_REQUEST_MSG "GET_PARAMETER rtsp://localhost/wfd1.0 RTSP/1.0"
#define RTSP_SET_PARAMETER_FIELD "SET_PARAMETER"

#endif

/* Default values*/
#define RTSP_DFLT_CONTENT_PROTECTION "none"
#define RTSP_DFLT_VIDEO_FORMATS "00 00 03 10 0001ffff 1fffffff 00001fff 00 0000 0000 10 none none"
#define RTSP_DFLT_AUDIO_FORMATS "AAC 00000007 00"
#define RTSP_DFLT_TRANSPORT_PROFILE "RTP/AVP/UDP"
#define RTSP_DFLT_STREAMING_PORT "1990"
#define RTSP_STD_UNICAST_FIELD "unicast"
#define RTSP_DFLT_CLIENT_RTP_PORTS RTSP_DFLT_TRANSPORT_PROFILE RTSP_SEMI_COLON_STR RTSP_STD_UNICAST_FIELD RTSP_SPACE_STR RTSP_DFLT_STREAMING_PORT RTSP_SPACE_STR "0 mode=play"

#define RTSP_CRLF_STR "\r\n"
#define RTSP_DOUBLE_QUOTE_STR "\""
#define RTSP_SPACE_STR SPACE_CHAR
#define RTSP_SEMI_COLON_STR ";"

/* It will be used to parse the data from WFD Source */
//#define RTSP_STD_REQUEST_STR "RTSP/1.0" RTSP_CRLF_STR
//#define RTSP_REQ_OPTIONS "OPTIONS * " RTSP_STD_REQUEST_STR

//#define RTSP_STD_SEQUENCE_FIELD "CSeq: "
//#define RTSP_STD_REQUIRE_FIELD "Require: "
//#define RTSP_STD_SESSION_FIELD "Session: "

//#define RTSP_TRIGGER_METHOD_FIELD   "wfd_trigger_method: "

//#define RTSP_STD_UNICAST_FIELD "unicast"

//#define RTSP_WFD_CONTENT_PROTECT_FIELD "wfd_content_protection: "
//#define RTSP_WFD_VIDEO_FMT_FIELD "wfd_video_formats: "
//#define RTSP_WFD_AUDIO_FMT_FIELD "wfd_audio_codecs: "
//#define RTSP_WFD_CLIENT_PORTS_FIELD "wfd_client_rtp_ports: "
//#define RTSP_WFD_PRESENTATION_URL_FIELD "wfd_presentation_URL: "

//#define RTSP_M16_REQUEST_MSG "GET_PARAMETER rtsp://localhost/wfd1.0 RTSP/1.0"
//#define RTSP_SET_PARAMETER_FIELD "SET_PARAMETER"

class MiracastRTSPMsg;

typedef enum rtsp_parser_fields_e
{
    RTSP_PARSER_FIELD_START  = 0x00,

    RTSP_OPTIONS_REQ_FIELD,

    /* M2 Response validation Mark START */
    RTSP_M2_RESPONSE_VALIDATE_MARKER_START,
    RTSP_OPTIONS_QUERY_FIELD,
    RTSP_SETUP_FIELD,
    RTSP_TEARDOWN_FIELD,
    RTSP_PLAY_FIELD,
    RTSP_PAUSE_FIELD,
    RTSP_GET_PARAMETER_FIELD,
    RTSP_SET_PARAMETER_FIELD,
    RTSP_M2_RESPONSE_VALIDATE_MARKER_END,
    /* M2 Response validation Mark END */

    /* M3 Request validation Mark START */
    RTSP_M3_REQ_VALIDATE_MARKER_START,
    RTSP_WFD_HDCP_FIELD,
    RTSP_WFD_VIDEO_FMT_FIELD,
    RTSP_WFD_AUDIO_CODEC_FIELD,
    RTSP_WFD_CLI_RTP_PORTS_FIELD,
    RTSP_M3_REQ_VALIDATE_MARKER_END,
    /* M3 Request validation Mark END */

    RTSP_WFD_STREAMING_URL_FIELD,

    RTSP_SEQUENCE_FIELD,
    RTSP_REQUIRE_FIELD,
    RTSP_SESSION_FIELD,
    RTSP_PUBLIC_FIELD,
    RTSP_TRANSPORT_FIELD,
    RTSP_CONTENT_TEXT_FIELD,
    RTSP_TRIGGER_METHOD_FIELD,
    RTSP_UNICAST_FIELD,
    RTSP_VERSION_FIELD,

    RTSP_PARSER_FIELD_END
}
RTSP_PARSER_FIELDS;

typedef enum rtsp_message_format_sink2src_e
{
    RTSP_MSG_FMT_M1_RESPONSE = 0x00,
    RTSP_MSG_FMT_M2_REQUEST,
    RTSP_MSG_FMT_M3_RESPONSE,
    RTSP_MSG_FMT_M4_RESPONSE,
    RTSP_MSG_FMT_M5_RESPONSE,
    RTSP_MSG_FMT_M6_REQUEST,
    RTSP_MSG_FMT_M7_REQUEST,
    RTSP_MSG_FMT_M16_RESPONSE,
    RTSP_MSG_FMT_PAUSE_REQUEST,
    RTSP_MSG_FMT_PLAY_REQUEST,
    RTSP_MSG_FMT_TEARDOWN_REQUEST,
    RTSP_MSG_FMT_TRIGGER_METHODS_RESPONSE,
    RTSP_MSG_FMT_REPORT_ERROR,
    RTSP_MSG_FMT_INVALID
} RTSP_MSG_FMT_SINK2SRC;

typedef enum rtsp_error_codes_e
{
    RTSP_ERRORCODE_CONTINUE = 0x00,
    RTSP_ERRORCODE_OK,
    RTSP_ERRORCODE_CREATED,
    RTSP_ERRORCODE_LOW_STORAGE,
    RTSP_ERRORCODE_MULTI_CHOICES,
    RTSP_ERRORCODE_MOVED_PERMANENTLY,
    RTSP_ERRORCODE_MOVED_TEMPORARILY,
    RTSP_ERRORCODE_SEE_OTHER,
    RTSP_ERRORCODE_NOT_MODIFIED,
    RTSP_ERRORCODE_USE_PROXY,
    RTSP_ERRORCODE_BAD_REQUEST,
    RTSP_ERRORCODE_UNAUTHORIZED,
    RTSP_ERRORCODE_PAYMENT_REQUIRED,
    RTSP_ERRORCODE_FORBIDDEN,
    RTSP_ERRORCODE_NOT_FOUND,
    RTSP_ERRORCODE_METHOD_NOT_ALLOWED,
    RTSP_ERRORCODE_NOT_ACCEPTABLE,
    RTSP_ERRORCODE_PROXY_AUTHENDICATION_REQUIRED,
    RTSP_ERRORCODE_REQUEST_TIMEDOUT,
    RTSP_ERRORCODE_GONE,
    RTSP_ERRORCODE_LENGTH_REQUIRED,
    RTSP_ERRORCODE_PRECONDITION_FAILED,
    RTSP_ERRORCODE_REQUEST_ENTITY_TOO_LARGE,
    RTSP_ERRORCODE_REQUEST_URI_TOO_LARGE,
    RTSP_ERRORCODE_UNSUPPORTED_MEDIA_TYPE,
    RTSP_ERRORCODE_PARAMETER_NOT_UNDERSTOOD,
    RTSP_ERRORCODE_CONFERENCE_NOT_FOUND,
    RTSP_ERRORCODE_NOT_ENOUGH_BANDWIDTH,
    RTSP_ERRORCODE_SESSION_NOT_FOUND,
    RTSP_ERRORCODE_METHOD_NOT_VALID,
    RTSP_ERRORCODE_HEADER_FIELD_NOT_VALID,
    RTSP_ERRORCODE_INVALID_RANGE,
    RTSP_ERRORCODE_RO_PARAMETER,
    RTSP_ERRORCODE_AGGREGATE_OPERATION_NOT_ALLOWED,
    RTSP_ERRORCODE_ONLY_AGGREGATE_OPERATION_ALLOWED,
    RTSP_ERRORCODE_UNSUPPORTED_TRANSPORT,
    RTSP_ERRORCODE_DEST_UNREACHABLE,
    RTSP_ERRORCODE_KEY_MGMNT_FAILURE,
    RTSP_ERRORCODE_INTERNAL_SERVER_ERROR,
    RTSP_ERRORCODE_NOT_IMPLEMENTED,
    RTSP_ERRORCODE_BAD_GATEWAY,
    RTSP_ERRORCODE_SERVICE_UNAVAILABLE,
    RTSP_ERRORCODE_GATEWAY_TIMEOUT,
    RTSP_ERRORCODE_VERSION_NOT_SUPPORTED,
    RTSP_ERRORCODE_OPTION_NOT_SUPPORTED,
    RTSP_ERRORCODE_INVALID
}RTSP_ERRORCODES;

typedef struct rtsp_msg_fmt_template_st
{
    RTSP_MSG_FMT_SINK2SRC rtsp_msg_fmt_e;
    const char *template_name;
} RTSP_MSG_FMT_TEMPLATE;

typedef struct rtsp_errorcode_template
{
    RTSP_ERRORCODES rtsp_errorcode_e;
    const char *string_fmt;
} RTSP_ERRORCODE_TEMPLATE;

typedef struct rtsp_parser_template_st
{
    RTSP_PARSER_FIELDS rtsp_parser_field_e;
    const char *string_fmt;
    std::string (MiracastRTSPMsg::*parse_field_info_fn)(void);
} RTSP_PARSER_TEMPLATE;

typedef enum rtsp_native_timing_options_e
{
    RTSP_NATIVE_NO_RESOLUTION_SUPPORTED = 0x00,
    RTSP_NATIVE_CEA_RESOLUTION_SUPPORTED = 0x01,
    RTSP_NATIVE_VESA_RESOLUTION_SUPPORTED = 0x02,
    RTSP_NATIVE_HH_RESOLUTION_SUPPORTED = 0x04
}
RTSP_NATIVE_TIMING_OPTIONS;

typedef enum rtsp_display_options_e
{
    RTSP_PREFERED_DISPLAY_NOT_SUPPORTED = 0x00,
    RTSP_PREFERED_DISPLAY_SUPPORTED = 0x01
}
RTSP_DISPLAY_OPTIONS;

typedef enum rtsp_profile_bitmap_e
{
    RTSP_PROFILE_BMP_NOT_SUPPORTED  = 0x00,
    RTSP_PROFILE_BMP_CBP_SUPPORTED  = 0x01,
    RTSP_PROFILE_BMP_CHP_SUPPORTED  = 0x02,
    RTSP_PROFILE_BMP_BOTH_SUPPORTED = 0x03
}
RTSP_PROFILE_BITMAP;

typedef enum rtsp_h264_level_bitmap_e
{
    RTSP_H264_LEVEL_3p1_BITMAP  = 0x01,
    RTSP_H264_LEVEL_3p2_BITMAP  = 0x02,
    RTSP_H264_LEVEL_4_BITMAP    = 0x04,
    RTSP_H264_LEVEL_4p1_BITMAP  = 0x08,
    RTSP_H264_LEVEL_4p2_BITMAP  = 0x10
}
RTSP_H264_BITMAP_LEVEL;

typedef enum rtsp_cea_resolution_e
{
    RTSP_CEA_RESOLUTION_INVALID    = 0x00000000,
    RTSP_CEA_RESOLUTION_640x480p60 = 0x00000001,
    RTSP_CEA_RESOLUTION_720x480p60 = 0x00000002,
    RTSP_CEA_RESOLUTION_720x480i60 = 0x00000004,
    RTSP_CEA_RESOLUTION_720x576p50 = 0x00000008,
    RTSP_CEA_RESOLUTION_720x576i50 = 0x00000010,
    RTSP_CEA_RESOLUTION_1280x720p30 = 0x00000020,
    RTSP_CEA_RESOLUTION_1280x720p60 = 0x00000040,
    RTSP_CEA_RESOLUTION_1920x1080p30 = 0x00000080,
    RTSP_CEA_RESOLUTION_1920x1080p60 = 0x00000100,
    RTSP_CEA_RESOLUTION_1920x1080i60 = 0x00000200,
    RTSP_CEA_RESOLUTION_1280x720p25 = 0x00000400,
    RTSP_CEA_RESOLUTION_1280x720p50 = 0x00000800,
    RTSP_CEA_RESOLUTION_1920x1080p25 = 0x00001000,
    RTSP_CEA_RESOLUTION_1920x1080p50 = 0x00002000,
    RTSP_CEA_RESOLUTION_1920x1080i50 = 0x00004000,
    RTSP_CEA_RESOLUTION_1280x720p24 = 0x00008000,
    RTSP_CEA_RESOLUTION_1920x1080p24 = 0x00010000,
    RTSP_CEA_RESOLUTION_UNSUPPORTED_MASK = (~(RTSP_CEA_RESOLUTION_1920x1080p24|0x0000FFFF))
}
RTSP_CEA_RESOLUTIONS;

typedef enum rtsp_vesa_resolution_e
{
    RTSP_VESA_RESOLUTION_INVALID    = 0x00000000,
    RTSP_VESA_RESOLUTION_800x600p30 = 0x00000001,
    RTSP_VESA_RESOLUTION_800x600p60 = 0x00000002,
    RTSP_VESA_RESOLUTION_1024x768p30 = 0x00000004,
    RTSP_VESA_RESOLUTION_1024x768p60 = 0x00000008,
    RTSP_VESA_RESOLUTION_1152x864p30 = 0x00000010,
    RTSP_VESA_RESOLUTION_1152x864p60 = 0x00000020,
    RTSP_VESA_RESOLUTION_1280x768p30 = 0x00000040,
    RTSP_VESA_RESOLUTION_1280x768p60 = 0x00000080,
    RTSP_VESA_RESOLUTION_1280x800p30 = 0x00000100,
    RTSP_VESA_RESOLUTION_1280x800p60 = 0x00000200,
    RTSP_VESA_RESOLUTION_1360x768p30 = 0x00000400,
    RTSP_VESA_RESOLUTION_1360x768p60 = 0x00000800,
    RTSP_VESA_RESOLUTION_1366x768p30 = 0x00001000,
    RTSP_VESA_RESOLUTION_1366x768p60 = 0x00002000,
    RTSP_VESA_RESOLUTION_1280x1024p30 = 0x00004000,
    RTSP_VESA_RESOLUTION_1280x1024p60 = 0x00008000,
    RTSP_VESA_RESOLUTION_1400x1050p30 = 0x00010000,
    RTSP_VESA_RESOLUTION_1400x1050p60 = 0x00020000,
    RTSP_VESA_RESOLUTION_1440x900p30 = 0x00040000,
    RTSP_VESA_RESOLUTION_1440x900p60 = 0x00080000,
    RTSP_VESA_RESOLUTION_1600x900p30 = 0x00100000,
    RTSP_VESA_RESOLUTION_1600x900p60 = 0x00200000,
    RTSP_VESA_RESOLUTION_1600x1200p30 = 0x00400000,
    RTSP_VESA_RESOLUTION_1600x1200p60 = 0x00800000,
    RTSP_VESA_RESOLUTION_1680x1024p30 = 0x01000000,
    RTSP_VESA_RESOLUTION_1680x1024p60 = 0x02000000,
    RTSP_VESA_RESOLUTION_1680x1050p30 = 0x04000000,
    RTSP_VESA_RESOLUTION_1680x1050p60 = 0x08000000,
    RTSP_VESA_RESOLUTION_1920x1200p60 = 0x10000000,
    RTSP_VESA_RESOLUTION_UNSUPPORTED_MASK = (~(RTSP_VESA_RESOLUTION_1920x1200p60|0x0FFFFFFF))
}
RTSP_VESA_RESOLUTIONS;

typedef enum rtsp_hh_resolution_e
{
    RTSP_HH_RESOLUTION_INVALID    = 0x00000000,
    RTSP_HH_RESOLUTION_800x480p60 = 0x00000001,
    RTSP_HH_RESOLUTION_854x480p30 = 0x00000002,
    RTSP_HH_RESOLUTION_854x480p60 = 0x00000004,
    RTSP_HH_RESOLUTION_864x480p30 = 0x00000008,
    RTSP_HH_RESOLUTION_864x480p60 = 0x00000010,
    RTSP_HH_RESOLUTION_600x360p30 = 0x00000020,
    RTSP_HH_RESOLUTION_600x360p60 = 0x00000040,
    RTSP_HH_RESOLUTION_960x540p30 = 0x00000080,
    RTSP_HH_RESOLUTION_960x540p60 = 0x00000100,
    RTSP_HH_RESOLUTION_848x480p30 = 0x00000200,
    RTSP_HH_RESOLUTION_848x480p60 = 0x00000400,
    RTSP_HH_RESOLUTION_UNSUPPORTED_MASK = (~(RTSP_HH_RESOLUTION_848x480p60|0x000000FF))
}
RTSP_HH_RESOLUTIONS;

typedef enum rtsp_audio_formats_e
{
    RTSP_LPCM_AUDIO_FORMAT  = 0x00000001,
    RTSP_AAC_AUDIO_FORMAT  = 0x00000002,
    RTSP_AC3_AUDIO_FORMAT  = 0x00000003,
    RTSP_UNSUPPORTED_AUDIO_FORMAT
}
RTSP_AUDIO_FORMATS;

typedef enum rtsp_lpcm_modes_e
{
    RTSP_LPCM_INVALID_MODE  = 0x00000000,
    RTSP_LPCM_CH2_44p1kHz  = 0x00000001,
    RTSP_LPCM_CH2_48kHz  = 0x00000002,
    RTSP_LPCM_UNSUPPORTED_MASK = (~(RTSP_LPCM_CH2_48kHz|0x00000001))
}
RTSP_LPCM_MODES;

typedef enum rtsp_aac_modes_e
{
    RTSP_AAC_INVALID_MODE   = 0x00000000,
    RTSP_AAC_CH2_48kHz      = 0x00000001,
    RTSP_AAC_CH4_48kHz      = 0x00000002,
    RTSP_AAC_CH6_48kHz      = 0x00000004,
    RTSP_AAC_CH8_48kHz      = 0x00000008,
    RTSP_AAC_UNSUPPORTED_MASK = (~(RTSP_AAC_CH8_48kHz|0x00000007))
}
RTSP_AAC_MODES;

typedef enum rtsp_ac3_modes_e
{
    RTSP_AC3_INVALID_MODE   = 0x00000000,
    RTSP_AC3_CH2_48kHz      = 0x00000001,
    RTSP_AC3_CH4_48kHz      = 0x00000002,
    RTSP_AC3_CH6_48kHz      = 0x00000004,
    RTSP_AC3_UNSUPPORTED_MASK = (~(RTSP_AC3_CH6_48kHz|0x00000003))
}
RTSP_AC3_MODES;

#pragma pack(1)
typedef struct rtsp_H264_codecs_st
{
    uint8_t profile;
    uint8_t level;
/*{{{   misc_params*/
    RTSP_CEA_RESOLUTIONS cea_mask;
    RTSP_VESA_RESOLUTIONS vesa_mask;
    RTSP_HH_RESOLUTIONS hh_mask;
    uint8_t latency;
    uint16_t min_slice;
    uint16_t slice_encode;
    bool video_frame_skip_support;
    uint8_t max_skip_intervals;
    bool video_frame_rate_change_support;
/*}}}   misc_params*/
    int32_t max_hres;
    int32_t max_vres;
}
RTSP_H264_CODEC_STRUCT;

#pragma pack(1)
typedef struct rtsp_wfd_video_format_st
{
    uint8_t native;
    uint8_t preferred_display_mode_supported;
    RTSP_H264_CODEC_STRUCT  st_h264_codecs;
}
RTSP_WFD_VIDEO_FMT_STRUCT;

#pragma pack(1)
typedef struct rtsp_wfd_audio_format_st
{
    RTSP_AUDIO_FORMATS audio_format;
    uint32_t    modes;
    uint8_t latency;
}
RTSP_WFD_AUDIO_FMT_STRUCT;

class MiracastRTSPMsg
{
public:
    static MiracastRTSPMsg *getInstance(MiracastError &error_code , MiracastPlayerNotifier *player_notifier = nullptr , MiracastThread *controller_thread_id = nullptr);
    static void destroyInstance();

    std::string get_WFDVideoFormat(void);
    std::string get_WFDAudioCodecs(void);
    std::string get_WFDClientRTPPorts(void);
    std::string get_WFDUIBCCapability(void);
    std::string get_WFDContentProtection(void);
    std::string get_WFDSecScreenSharing(void);
    std::string get_WFDPortraitDisplay(void);
    std::string get_WFDSecRotation(void);
    std::string get_WFDSecHWRotation(void);
    std::string get_WFDSecFrameRate(void);
    std::string get_WFDPresentationURL(void);
    std::string get_WFDTransportProfile(void);
    std::string get_WFDStreamingPortNumber(void);
    bool IsWFDUnicastSupported(void);
    std::string get_CurrentWFDSessionNumber(void);

    bool set_WFDVideoFormat(RTSP_WFD_VIDEO_FMT_STRUCT st_video_fmt);
    bool set_WFDAudioCodecs( RTSP_WFD_AUDIO_FMT_STRUCT st_audio_fmt );
    bool set_WFDClientRTPPorts(std::string client_rtp_ports);
    bool set_WFDUIBCCapability(std::string uibc_caps);
    bool set_WFDContentProtection(std::string content_protection);
    bool set_WFDSecScreenSharing(std::string screen_sharing);
    bool set_WFDPortraitDisplay(std::string portrait_display);
    bool set_WFDSecRotation(std::string rotation);
    bool set_WFDSecHWRotation(std::string hw_rotation);
    bool set_WFDSecFrameRate(std::string framerate);
    bool set_WFDPresentationURL(std::string URL);
    bool set_WFDTransportProfile(std::string profile);
    bool set_WFDStreamingPortNumber(std::string port_number);
    bool set_WFDEnableDisableUnicast(bool enable_disable_unicast);
    bool set_WFDSessionNumber(std::string session);

    void set_WFDSourceMACAddress(std::string MAC_Addr);
    void set_WFDSourceName(std::string device_name);
    std::string get_WFDSourceName(void);
    std::string get_WFDSourceMACAddress(void);
    void reset_WFDSourceMACAddress(void);
    void reset_WFDSourceName(void);

    bool set_WFDRequestResponseTimeout( unsigned int request_timeout , unsigned int response_timeout );

    eMIRA_PLAYER_STATES get_state(void);

    void send_msgto_rtsp_msg_hdler_thread(RTSP_HLDR_MSGQ_STRUCT rtsp_hldr_msgq_data);
    MiracastError initiate_TCP(std::string goIP);
    MiracastError start_streaming( VIDEO_RECT_STRUCT video_rect );
    MiracastError stop_streaming( eMIRA_PLAYER_STATES state );
    void RTSPMessageHandler_Thread(void *args);
#ifdef ENABLE_MIRACAST_PLAYER_TEST_NOTIFIER
    MiracastError create_TestNotifier(void);
    void destroy_TestNotifier();
    void TestNotifier_Thread(void *args);
    void send_msgto_test_notifier_thread(MIRACAST_PLAYER_TEST_NOTIFIER_MSGQ_ST stMsgQ);
    MiracastThread  *m_test_notifier_thread;
#endif /* ENABLE_MIRACAST_PLAYER_TEST_NOTIFIER */

    static std::string format_string(const char *fmt, const std::vector<const char *> &args)
    {
        std::string result = fmt;
        size_t arg_index = 0;
        size_t arg_count = args.size();
        while (arg_index < arg_count)
        {
            size_t found = result.find("%s");
            if (found != std::string::npos)
            {
                result.replace(found, 2, args[arg_index]);
            }
            ++arg_index;
        }
        return result;
    };

private:
    static MiracastRTSPMsg *m_rtsp_msg_obj;
    MiracastRTSPMsg();
    virtual ~MiracastRTSPMsg();
    MiracastRTSPMsg &operator=(const MiracastRTSPMsg &) = delete;
    MiracastRTSPMsg(const MiracastRTSPMsg &) = delete;

    int m_tcpSockfd;
    int m_epollfd;
    unsigned int m_wfd_src_req_timeout;
    unsigned int m_wfd_src_res_timeout;
    unsigned int m_current_wait_time_ms;
    int m_wfd_src_session_timeout;
    eMIRA_PLAYER_STATES m_current_state;

    bool m_streaming_started;

    std::string m_connected_mac_addr;
    std::string m_connected_device_name;
    std::string m_wfd_video_formats;
    std::string m_wfd_audio_codecs;
    std::string m_wfd_client_rtp_ports;
    std::string m_wfd_uibc_capability;
    std::string m_wfd_content_protection;
    std::string m_wfd_sec_screensharing;
    std::string m_wfd_sec_portrait_display;
    std::string m_wfd_sec_rotation;
    std::string m_wfd_sec_hw_rotation;
    std::string m_wfd_sec_framerate;
    std::string m_wfd_presentation_URL;
    std::string m_wfd_transport_profile;
    std::string m_wfd_streaming_port;
    bool m_is_unicast;
    std::string m_wfd_session_number;
    std::string m_current_sequence_number;
    std::string m_src_dev_ip;
    std::string m_sink_ip;
    RTSP_WFD_VIDEO_FMT_STRUCT   m_wfd_video_formats_st;
    RTSP_WFD_AUDIO_FMT_STRUCT   m_wfd_audio_formats_st;

    static RTSP_MSG_FMT_TEMPLATE rtsp_msg_fmt_template[];
    static RTSP_ERRORCODE_TEMPLATE rtsp_msg_error_codes[];
    static RTSP_PARSER_TEMPLATE rtsp_msg_parser_fields[];
    static const int num_parse_fields;

    MiracastThread *m_rtsp_msg_handler_thread;
    MiracastThread *m_controller_thread;
    MiracastPlayerNotifier *m_player_notify_handler;

    void set_state( eMIRA_PLAYER_STATES state , bool send_notification = false , eM_PLAYER_REASON_CODE reason_code = MIRACAST_PLAYER_REASON_CODE_SUCCESS );
    void store_srcsink_info( std::string client_name, std::string client_mac, std::string src_dev_ip, std::string sink_ip);

    void send_msgto_controller_thread(eCONTROLLER_FW_STATES state);
    MiracastError create_RTSPThread(void);
    void Release_SocketAndEpollDescriptor(void);

    RTSP_STATUS validate_rtsp_m1_msg_m2_send_request(std::string rtsp_m1_msg_buffer);
    RTSP_STATUS validate_rtsp_m2_request_ack(std::string rtsp_m1_response_ack_buffer);
    RTSP_STATUS validate_rtsp_m3_response_back(std::string rtsp_m3_msg_buffer);
    RTSP_STATUS validate_rtsp_m4_response_back(std::string rtsp_m4_msg_buffer);
    RTSP_STATUS validate_rtsp_m5_msg_m6_send_request(std::string rtsp_m5_msg_buffer);
    RTSP_STATUS validate_rtsp_m6_ack_m7_send_request(std::string rtsp_m6_ack_buffer);
    RTSP_STATUS validate_rtsp_trigger_request_ack(std::string rtsp_trigger_req_ack_buffer , std::string received_seq_num );
    RTSP_STATUS validate_rtsp_post_m1_m7_xchange(std::string rtsp_post_m1_m7_xchange_buffer);
    RTSP_STATUS rtsp_sink2src_request_msg_handling(eCONTROLLER_FW_STATES state);

    RTSP_STATUS validate_rtsp_receive_buffer_handling(std::string rtsp_msg_buffer);
    RTSP_STATUS validate_rtsp_generic_request_response( std::string rtsp_msg_buffer );
    RTSP_STATUS validate_rtsp_options_request( std::string rtsp_msg_buffer );
    RTSP_STATUS validate_rtsp_getparameter_request( std::string rtsp_msg_buffer );
    RTSP_STATUS validate_rtsp_setparameter_request( std::string rtsp_msg_buffer );
    RTSP_STATUS validate_rtsp_trigger_method_request(std::string rtsp_msg_buffer);
    RTSP_STATUS send_rtsp_reply_sink2src( RTSP_MSG_FMT_SINK2SRC req_fmt , std::string received_seq_num = "" , RTSP_ERRORCODES error_code = RTSP_ERRORCODE_OK );

    const char *get_RequestResponseFormat(RTSP_MSG_FMT_SINK2SRC format_type);
    const char* get_errorcode_string(RTSP_ERRORCODES error_code);
    const char* get_parser_field_by_index(RTSP_PARSER_FIELDS parse_field);
    std::string get_parser_field_value(RTSP_PARSER_FIELDS parse_field);
    std::string get_parser_field_n_value_by_name(std::string request_field_name );
    std::string parse_received_parser_field_value(std::string rtsp_msg_buffer , RTSP_PARSER_FIELDS parse_field );

    std::string generate_request_response_msg(RTSP_MSG_FMT_SINK2SRC msg_fmt_needed, std::string received_session_no , std::string append_data1 , RTSP_ERRORCODES error_code = RTSP_ERRORCODE_OK );
    std::string get_RequestSequenceNumber(void);
    std::string generate_RequestSequenceNumber(void);

    bool set_wait_timeout(unsigned int waittime_ms);
    unsigned int get_wait_timeout(void);

    RTSP_STATUS receive_buffer_timedOut(int sockfd, void *buffer, size_t buffer_len , unsigned int wait_time_ms = RTSP_REQUEST_RECV_TIMEOUT );
    bool wait_data_timeout(int m_Sockfd, unsigned int ms);
    RTSP_STATUS send_rstp_msg(int sockfd, std::string rtsp_response_buffer);
    std::string get_localIp();
    MiracastError updateVideoRectangle( VIDEO_RECT_STRUCT videorect );
};

#endif