#pragma once

#define IARM_BUS_SYSMGR_NAME "SYSMgr"
#define IARM_BUS_SYSMGR_API_GetSystemStates     "GetSystemStates"

typedef enum _SYSMgr_EventId_t {
    IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE,
        IARM_BUS_SYSMGR_EVENT_XUPNP_DATA_REQUEST,               /*!< Xupnp data  request frm Receiver to UPNP*/
        IARM_BUS_SYSMGR_EVENT_XUPNP_DATA_UPDATE,    /*!< Xupnp data  frm  UPNP to  Receiver*/
        IARM_BUS_SYSMGR_EVENT_IMAGE_DNLD,   /*Image download status to Receiver   */
        IARM_BUS_SYSMGR_EVENT_CARD_FWDNLD,              /*!< CARD FW download event to Receiver - Valid on Cisco CA*/
        IARM_BUS_SYSMGR_EVENT_HDCP_PROFILE_UPDATE,      /*!< HDCP Profile Update */
        IARM_BUS_SYSMGR_EVENT_INTRUSION,                /*!< Intrusion Detection */
        IARM_BUS_SYSMGR_EVENT_EISS_FILTER_STATUS,       /*!< EISS Filter status event */
        IARM_BUS_SYSMGR_EVENT_EISS_APP_ID_UPDATE,       /*!< EISS App ID update */
        IARM_BUS_SYSMGR_EVENT_KEYCODE_LOGGING_CHANGED,  /*!< Key Code logging status update */
        IARM_BUS_SYSMGR_EVENT_USB_MOUNT_CHANGED,        /*!< Fires when USB mounts change */
        IARM_BUS_SYSMGR_EVENT_APP_RELEASE_FOCUS,        /*!< Application fires event to release focus*/
        IARM_BUS_SYSMGR_EVENT_MAX                               /*!< Max Event Id */
} IARM_Bus_SYSMgr_EventId_t;

typedef struct _IARM_BUS_SYSMGR_KEYCodeLoggingInfo_Param_t{
      int logStatus;                                      /*!< HDCP Profile Data */
}IARM_BUS_SYSMGR_KEYCodeLoggingInfo_Param_t;

/*
 * Declare RPC API names for Key Code Logging Status
 */
#define IARM_BUS_SYSMGR_API_GetKeyCodeLoggingPref "GetKeyCodeLoggingPref"
#define IARM_BUS_SYSMGR_API_SetKeyCodeLoggingPref "SetKeyCodeLoggingPref"

typedef enum _SYSMgr_SystemState_t {
  IARM_BUS_SYSMGR_SYSSTATE_CHANNELMAP, //0
  IARM_BUS_SYSMGR_SYSSTATE_DISCONNECTMGR,//1
  IARM_BUS_SYSMGR_SYSSTATE_TUNEREADY, //2
  IARM_BUS_SYSMGR_SYSSTATE_EXIT_OK,//3
  IARM_BUS_SYSMGR_SYSSTATE_CMAC,//4
  IARM_BUS_SYSMGR_SYSSTATE_MOTO_ENTITLEMENT,//5
  IARM_BUS_SYSMGR_SYSSTATE_MOTO_HRV_RX,//6
  IARM_BUS_SYSMGR_SYSSTATE_CARD_CISCO_STATUS,//7
  IARM_BUS_SYSMGR_SYSSTATE_VIDEO_PRESENTING,//8
  IARM_BUS_SYSMGR_SYSSTATE_HDMI_OUT,//9
  IARM_BUS_SYSMGR_SYSSTATE_HDCP_ENABLED,//10
  IARM_BUS_SYSMGR_SYSSTATE_HDMI_EDID_READ,//11
  IARM_BUS_SYSMGR_SYSSTATE_FIRMWARE_DWNLD,//12
  IARM_BUS_SYSMGR_SYSSTATE_TIME_SOURCE,//13
  IARM_BUS_SYSMGR_SYSSTATE_TIME_ZONE,//14
  IARM_BUS_SYSMGR_SYSSTATE_CA_SYSTEM,//15
  IARM_BUS_SYSMGR_SYSSTATE_ESTB_IP,//16
  IARM_BUS_SYSMGR_SYSSTATE_ECM_IP,//17
  IARM_BUS_SYSMGR_SYSSTATE_LAN_IP,//18
  IARM_BUS_SYSMGR_SYSSTATE_MOCA,//19
  IARM_BUS_SYSMGR_SYSSTATE_DOCSIS,//20
  IARM_BUS_SYSMGR_SYSSTATE_DSG_BROADCAST_CHANNEL,//21
  IARM_BUS_SYSMGR_SYSSTATE_DSG_CA_TUNNEL,//22
  IARM_BUS_SYSMGR_SYSSTATE_CABLE_CARD,//23
  IARM_BUS_SYSMGR_SYSSTATE_CABLE_CARD_DWNLD,//24
  IARM_BUS_SYSMGR_SYSSTATE_CVR_SUBSYSTEM,//25
  IARM_BUS_SYSMGR_SYSSTATE_DOWNLOAD,//26
  IARM_BUS_SYSMGR_SYSSTATE_VOD_AD,//27
  IARM_BUS_SYSMGR_SYSSTATE_DAC_INIT_TIMESTAMP,//28
  IARM_BUS_SYSMGR_SYSSTATE_CABLE_CARD_SERIAL_NO,//29
  IARM_BUS_SYSMGR_SYSSTATE_ECM_MAC,//30
  IARM_BUS_SYSMGR_SYSSTATE_DAC_ID,//31
  IARM_BUS_SYSMGR_SYSSTATE_PLANT_ID,//32
  IARM_BUS_SYSMGR_SYSSTATE_STB_SERIAL_NO, //33
  IARM_BUS_SYSMGR_SYSSTATE_BOOTUP, //34
  IARM_BUS_SYSMGR_SYSSTATE_GATEWAY_CONNECTION, //35
  IARM_BUS_SYSMGR_SYSSTATE_DST_OFFSET, //36
  IARM_BUS_SYSMGR_SYSSTATE_RF_CONNECTED, //37
  IARM_BUS_SYSMGR_SYSSTATE_PARTNERID_CHANGE, //38
  IARM_BUS_SYSMGR_SYSSTATE_IP_MODE, //39
  IARM_BUS_SYSMGR_SYSSTATE_LP_CONNECTION_RESET, //40
  IARM_BUS_SYSMGR_SYSSTATE_RWS_CONNECTION_RESET, //41
  IARM_BUS_SYSMGR_SYSSTATE_QAM_READY, //42
  IARM_BUS_SYSMGR_SYSSTATE_FIRMWARE_UPDATE_STATE //43, Added as part of RDK-19978, As the IARM
} IARM_Bus_SYSMgr_SystemState_t;

typedef enum _SYSMgr_FirmwareUpdateState_t
{
  IARM_BUS_SYSMGR_FIRMWARE_UPDATE_STATE_UNINITIALIZED = 0,
  IARM_BUS_SYSMGR_FIRMWARE_UPDATE_STATE_REQUESTING = 1,
  IARM_BUS_SYSMGR_FIRMWARE_UPDATE_STATE_DOWNLOADING = 2,
  IARM_BUS_SYSMGR_FIRMWARE_UPDATE_STATE_FAILED = 3,
  IARM_BUS_SYSMGR_FIRMWARE_UPDATE_STATE_DOWNLOAD_COMPLETE = 4,
  IARM_BUS_SYSMGR_FIRMWARE_UPDATE_STATE_VALIDATION_COMPLETE = 5,
  IARM_BUS_SYSMGR_FIRMWARE_UPDATE_STATE_PREPARING_TO_REBOOT = 6,
  IARM_BUS_SYSMGR_FIRMWARE_UPDATE_STATE_ONHOLD_FOR_OPTOUT = 7, /* On Hold for opt-out */
  IARM_BUS_SYSMGR_FIRMWARE_UPDATE_STATE_CRITICAL_REBOOT = 8
} IARM_Bus_SYSMGR_FirmwareUpdateState_t;

typedef struct _IARM_BUS_SYSMgr_EventData_t{
        union {
                struct _CARD_FWDNLD_DATA {
                        char eventType;
                        char status;
                } cardFWDNLD;
                struct _IMAGE_FWDNLD_DATA {
                        char status;
                } imageFWDNLD;
                struct _XUPNP_DATA {
                        unsigned long deviceInfoLength;
                } xupnpData;
                struct _HDCP_PROFILE {
                        unsigned int hdcpProfile;
                } hdcpProfileData;
                struct _SystemStates {
                        IARM_Bus_SYSMgr_SystemState_t stateId;
                        int state;
                        int error;
                        char payload[128];
                } systemStates;
                struct _EISS_STATUS {
                        int filterStatus;
                } eissEventData;
        struct _EISS_APP_ID {
                        unsigned char idList[4][6];
                        int count;
                } eissAppIDList;
                struct _KEY_CODE_LOG {
                        int logStatus;
                } keyCodeLogData;
                struct _USB_MOUNT {
                        int mounted;
                        char device[128];
                        char dir[256];
                } usbMountData;

        } data;
} IARM_Bus_SYSMgr_EventData_t;

typedef struct _propertyValue
{
  int state;
  int error;
  char payload[128];
} state_property;

typedef struct _IARM_Bus_SYSMgr_GetSystemStates_Param_t {
  state_property channel_map;
  state_property disconnect_mgr_state;
  state_property TuneReadyStatus;
  state_property exit_ok_key_sequence;
  state_property cmac;
  state_property card_moto_entitlements;
  state_property card_moto_hrv_rx;
  state_property dac_init_timestamp;
  state_property card_cisco_status;
  state_property video_presenting;
  state_property hdmi_out;
  state_property hdcp_enabled;
  state_property hdmi_edid_read;
  state_property firmware_download;
  state_property time_source;
  state_property time_zone_available;
  state_property ca_system;
  state_property estb_ip;
  state_property ecm_ip;
  state_property lan_ip;
  state_property moca;
  state_property docsis;
  state_property dsg_broadcast_tunnel;
  state_property dsg_ca_tunnel;
  state_property cable_card;
  state_property cable_card_download;
  state_property cvr_subsystem;
  state_property download;
  state_property vod_ad;
  state_property card_serial_no;
  state_property ecm_mac;
  state_property dac_id;
  state_property plant_id;
  state_property stb_serial_no;
  state_property bootup;
  state_property dst_offset;
  state_property rf_connected;
  state_property ip_mode;
  state_property qam_ready_status;
  state_property firmware_update_state;
} IARM_Bus_SYSMgr_GetSystemStates_Param_t;

