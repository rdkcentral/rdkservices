/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
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
#ifndef __BTR_MGR_H__
#define __BTR_MGR_H__
/**
 * @file btmgr.h
 *
 * @defgroup BTR_MGR Bluetooth Manager
 *
 * Bluetooth Manager (An RDK component) interfaces with BlueZ through the D-Bus API,
 * so there is no direct linking of the BlueZ library with Bluetooth Manager.
 * Bluetooth manager provides an interface to port any Bluetooth stack on RDK
 * The Bluetooth manager daemon manages Bluetooth services in RDK.
 * It uses IARM Bus to facilitate communication between the application and Bluetooth driver
 * through Bluetooth Manager component.
 * @ingroup  Bluetooth
 *
 * @defgroup BTR_MGR_API Bluetooth Manager Data Types and API(s)
 * This file provides the data types and API(s) used by the bluetooth manager.
 * @ingroup  BTR_MGR
 *
 */
#ifdef __cplusplus
extern "C"
{
#endif
/**
 * @addtogroup BTR_MGR_API
 * @{
 */
#define BTRMGR_MAX_STR_LEN                  256
#define BTRMGR_NAME_LEN_MAX                 64
#define BTRMGR_STR_LEN                      32
#define BTRMGR_DEVICE_COUNT_MAX             32
#define BTRMGR_DISCOVERED_DEVICE_COUNT_MAX  128
#define BTRMGR_LE_DEVICE_COUNT_MAX          BTRMGR_DEVICE_COUNT_MAX / 3
#define BTRMGR_ADAPTER_COUNT_MAX            16
#define BTRMGR_MAX_DEVICE_PROFILE           32
#define BTRMGR_LE_FLAG_LIST_SIZE            10
#define BTRMGR_MEDIA_ELEMENT_COUNT_MAX      64
#define BTRMGR_DEVICE_MAC_LEN               6
#define BTRMGR_MAX_DEV_OP_DATA_LEN          BTRMGR_MAX_STR_LEN * 3
#define BTRMGR_SERVICE_DATA_LEN_MAX         32
#define BTRMGR_UUID_STR_LEN_MAX             64
#define BTRMGR_SERIAL_NUM_LEN               12
#define BTRMGR_DEVICE_INFORMATION_UUID      "0x180a"
#define BTRMGR_RDKDIAGNOSTICS_UUID          "0xFDB9"
#define BTRMGR_COLUMBO_UUID                 "64d9f574-7756-4ebc-9ebe-ed5f7f2871ab"
#define BTRMGR_XBOX_ELITE_PRODUCT_ID         0x0B05
#define BTRMGR_XBOX_ELITE_VENDOR_ID          0x045E
#define BTRMGR_SONY_PS_VENDOR_ID             0x054c
#define BTRMGR_SONY_PS_PRODUCT_ID_1          0x05c4
#define BTRMGR_SONY_PS_PRODUCT_ID_2          0x09cc
#define BTRMGR_XBOX_GAMESIR_PRODUCT_ID       0x0402
#define BTRMGR_XBOX_GAMESIR_VENDOR_ID        0x1949
#define BTRMGR_NINTENDO_GAMESIR_PRODUCT_ID   0x2009
#define BTRMGR_NINTENDO_GAMESIR_VENDOR_ID    0x057E
#define BTRMGR_HID_GAMEPAD_LE_APPEARANCE     0x3C4
#define BTRMGR_XBOX_ELITE_DEFAULT_FIRMWARE   0x0407
#define BTRMGR_SYSTEM_ID_UUID               "0x2a23"
#define BTRMGR_MODEL_NUMBER_UUID            "0x2a24"
#define BTRMGR_SERIAL_NUMBER_UUID           "0x2a25"
#define BTRMGR_FIRMWARE_REVISION_UUID       "0x2a26"
#define BTRMGR_HARDWARE_REVISION_UUID       "0x2a27"
#define BTRMGR_SOFTWARE_REVISION_UUID       "0x2a28"
#define BTRMGR_MANUFACTURER_NAME_UUID       "0x2a29"
#define BTRMGR_DEVICE_STATUS_UUID               "1f113f2c-cc01-4f03-9c5c-4b273ed631bb"
#define BTRMGR_FWDOWNLOAD_STATUS_UUID           "915f96a6-3788-4271-a7ea-6820e98896b8"
#define BTRMGR_WEBPA_STATUS_UUID                "9d5d3aae-51e3-4767-a055-59febd71de9d"
#define BTRMGR_WIFIRADIO1_STATUS_UUID           "59a99d5a-3d2f-4265-af13-316c7c76b1f0"
#define BTRMGR_WIFIRADIO2_STATUS_UUID           "9d6cf473-4fa6-4868-bf2b-c310f38df0c8"
#define BTRMGR_RF_STATUS_UUID                   "91b9497e-634c-408a-9f77-8375b1461b8b"
#define BTRMGR_COLUMBO_START                    "7c3fea2e-c082-4e17-b78b-1e69ca3889b9"
#define BTRMGR_COLUMBO_STOP                     "5a7e479b-9fac-4d73-b5b0-906669946720"
#define BTRMGR_COLUMBO_STATUS                   "26f05ee1-cefa-460d-8985-98c0dc078d6c"
#define BTRMGR_COLUMBO_REPORT                   "c1e62616-b4de-4f72-86ca-9d9469041b6d"
#define BTRMGR_DEVICE_MAC                       "device_mac"
#define BTRMGR_WIFI_CONNECT_DUMMY_UUID          "4ffab12b-e545-1baf-1dc6-bd3fd749716a"
#define BTRMGR_WIFI_SSID_DUMMY_UUID             "22d68435-f7af-1156-b2e2-c7d17211b026"
#define BTRMGR_WIFI_PWD_DUMMY_UUID              "6bfebfe7-294d-2f4e-a4b3-b04f2a66f2f0"
#define BTRMGR_WIFI_SEC_MODE_DUMMY_UUID         "112f9c72-82c1-93a3-d4fc-3dba3441b2c8"
#define BTRMGR_LEONBRDG_SERVICE_UUID_SETUP      "8DF5AD72-9BBC-4167-BCD9-E8EB9E4D671B"
#define BTRMGR_LEONBRDG_UUID_QR_CODE            "12984C43-3B43-4952-A387-715DCF9795C6"
#define BTRMGR_LEONBRDG_UUID_PROVISION_STATUS   "79DEFBC1-EB45-448D-9F2A-1ECC3A47A242"
#define BTRMGR_LEONBRDG_UUID_PUBLIC_KEY         "CB9FEE4D-C6ED-48C1-AB46-C3F2DA38EEDD"
#define BTRMGR_LEONBRDG_UUID_WIFI_CONFIG        "B87A896B-4052-4CAB-A7E7-A71594D9C353"
#define BTRMGR_LEONBRDG_UUID_SSID_LIST          "AAF92F88-7F35-48F1-9C3E-1FE5C3978B7A"
#define BTRMGR_DEBUG_DIRECTORY "/tmp/btrMgr_DebugArtifacts"
typedef unsigned long long int BTRMgrDeviceHandle;
typedef unsigned long long int BTRMgrMediaElementHandle;
/**
 * @brief Represents the status of the operation.
 */
typedef enum _BTRMGR_Result_t {
    BTRMGR_RESULT_SUCCESS = 0,
    BTRMGR_RESULT_GENERIC_FAILURE = -1,
    BTRMGR_RESULT_INVALID_INPUT = -2,
    BTRMGR_RESULT_INIT_FAILED = -3
} BTRMGR_Result_t;
/**
 * @brief Represents the event status.
 */
typedef enum _BTRMGR_Events_t {
    BTRMGR_EVENT_DEVICE_OUT_OF_RANGE = 100,
    BTRMGR_EVENT_DEVICE_DISCOVERY_UPDATE,
    BTRMGR_EVENT_DEVICE_DISCOVERY_COMPLETE,
    BTRMGR_EVENT_DEVICE_PAIRING_COMPLETE,
    BTRMGR_EVENT_DEVICE_UNPAIRING_COMPLETE,
    BTRMGR_EVENT_DEVICE_CONNECTION_COMPLETE,
    BTRMGR_EVENT_DEVICE_DISCONNECT_COMPLETE,
    BTRMGR_EVENT_DEVICE_PAIRING_FAILED,
    BTRMGR_EVENT_DEVICE_UNPAIRING_FAILED,
    BTRMGR_EVENT_DEVICE_CONNECTION_FAILED,
    BTRMGR_EVENT_DEVICE_DISCONNECT_FAILED,
    BTRMGR_EVENT_RECEIVED_EXTERNAL_PAIR_REQUEST,
    BTRMGR_EVENT_RECEIVED_EXTERNAL_CONNECT_REQUEST,
    BTRMGR_EVENT_RECEIVED_EXTERNAL_PLAYBACK_REQUEST,
    BTRMGR_EVENT_DEVICE_FOUND,
    BTRMGR_EVENT_MEDIA_TRACK_STARTED,
    BTRMGR_EVENT_MEDIA_TRACK_PLAYING,
    BTRMGR_EVENT_MEDIA_TRACK_PAUSED,
    BTRMGR_EVENT_MEDIA_TRACK_STOPPED,
    BTRMGR_EVENT_MEDIA_TRACK_POSITION,
    BTRMGR_EVENT_MEDIA_TRACK_CHANGED,
    BTRMGR_EVENT_MEDIA_PLAYBACK_ENDED,
    BTRMGR_EVENT_DEVICE_DISCOVERY_STARTED,
    BTRMGR_EVENT_DEVICE_OP_READY,
    BTRMGR_EVENT_DEVICE_OP_INFORMATION,
    BTRMGR_EVENT_MEDIA_PLAYER_NAME,
    BTRMGR_EVENT_MEDIA_PLAYER_VOLUME,
    BTRMGR_EVENT_MEDIA_PLAYER_DELAY,
    BTRMGR_EVENT_MEDIA_PLAYER_EQUALIZER_OFF,
    BTRMGR_EVENT_MEDIA_PLAYER_EQUALIZER_ON,
    BTRMGR_EVENT_MEDIA_PLAYER_SHUFFLE_OFF,
    BTRMGR_EVENT_MEDIA_PLAYER_SHUFFLE_ALLTRACKS,
    BTRMGR_EVENT_MEDIA_PLAYER_SHUFFLE_GROUP,
    BTRMGR_EVENT_MEDIA_PLAYER_REPEAT_OFF,
    BTRMGR_EVENT_MEDIA_PLAYER_REPEAT_SINGLETRACK,
    BTRMGR_EVENT_MEDIA_PLAYER_REPEAT_ALLTRACKS,
    BTRMGR_EVENT_MEDIA_PLAYER_REPEAT_GROUP,
    BTRMGR_EVENT_MEDIA_ALBUM_INFO,
    BTRMGR_EVENT_MEDIA_ARTIST_INFO,
    BTRMGR_EVENT_MEDIA_GENRE_INFO,
    BTRMGR_EVENT_MEDIA_COMPILATION_INFO,
    BTRMGR_EVENT_MEDIA_PLAYLIST_INFO,
    BTRMGR_EVENT_MEDIA_TRACKLIST_INFO,
    BTRMGR_EVENT_MEDIA_TRACK_INFO,
    BTRMGR_EVENT_MEDIA_PLAYER_MUTE,
    BTRMGR_EVENT_MEDIA_PLAYER_UNMUTE,
    BTRMGR_EVENT_DEVICE_MEDIA_STATUS,
    BTRMGR_EVENT_BATTERY_INFO,
    BTRMGR_EVENT_MAX
} BTRMGR_Events_t;
/**
 * @brief Represents the bluetooth device types.
 */
typedef enum _BTRMGR_DeviceType_t {
    BTRMGR_DEVICE_TYPE_UNKNOWN,
    BTRMGR_DEVICE_TYPE_WEARABLE_HEADSET,
    BTRMGR_DEVICE_TYPE_HANDSFREE,
    BTRMGR_DEVICE_TYPE_RESERVED,
    BTRMGR_DEVICE_TYPE_MICROPHONE,
    BTRMGR_DEVICE_TYPE_LOUDSPEAKER,
    BTRMGR_DEVICE_TYPE_HEADPHONES,
    BTRMGR_DEVICE_TYPE_PORTABLE_AUDIO,
    BTRMGR_DEVICE_TYPE_CAR_AUDIO,
    BTRMGR_DEVICE_TYPE_STB,
    BTRMGR_DEVICE_TYPE_HIFI_AUDIO_DEVICE,
    BTRMGR_DEVICE_TYPE_VCR,
    BTRMGR_DEVICE_TYPE_VIDEO_CAMERA,
    BTRMGR_DEVICE_TYPE_CAMCODER,
    BTRMGR_DEVICE_TYPE_VIDEO_MONITOR,
    BTRMGR_DEVICE_TYPE_TV,
    BTRMGR_DEVICE_TYPE_VIDEO_CONFERENCE,
    BTRMGR_DEVICE_TYPE_SMARTPHONE,
    BTRMGR_DEVICE_TYPE_TABLET,
    // LE
    BTRMGR_DEVICE_TYPE_TILE,
    BTRMGR_DEVICE_TYPE_XBB,
    BTRMGR_DEVICE_TYPE_HID,
    BTRMGR_DEVICE_TYPE_HID_GAMEPAD,
    BTRMGR_DEVICE_TYPE_END
} BTRMGR_DeviceType_t;
/**
 * @brief Represents the stream output types.
 */
typedef enum _BTRMGR_StreamOut_Type_t {
    BTRMGR_STREAM_PRIMARY = 0,
    BTRMGR_STREAM_AUXILIARY
} BTRMGR_StreamOut_Type_t;
/**
 * @brief Represents the operation type for bluetooth device.
 */
typedef enum _BTRMGR_DeviceOperationType_t {
    BTRMGR_DEVICE_OP_TYPE_AUDIO_OUTPUT  = 1 << 0,
    BTRMGR_DEVICE_OP_TYPE_AUDIO_INPUT   = 1 << 1,
    BTRMGR_DEVICE_OP_TYPE_LE            = 1 << 2,
    BTRMGR_DEVICE_OP_TYPE_HID           = 1 << 3,
    BTRMGR_DEVICE_OP_TYPE_UNKNOWN       = 1 << 4,
    BTRMGR_DEVICE_OP_TYPE_AUDIO_AND_HID = (1 << 0) | (1 << 3) //combined type for scanning
} BTRMGR_DeviceOperationType_t;
/**
 * @brief Represents the bluetooth power states.
 */
typedef enum _BTRMGR_DevicePower_t {
    BTRMGR_DEVICE_POWER_ACTIVE = 1,
    BTRMGR_DEVICE_POWER_LOW,
    BTRMGR_DEVICE_POWER_STANDBY
} BTRMGR_DevicePower_t;
/**
 * @brief Represents the bluetooth signal strength
 */
typedef enum _BTRMGR_RSSIValue_type_t {
    BTRMGR_RSSI_NONE = 0,      //!< No signal (0 bar)
    BTRMGR_RSSI_POOR,          //!< Poor (1 bar)
    BTRMGR_RSSI_FAIR,          //!< Fair (2 bars)
    BTRMGR_RSSI_GOOD,          //!< Good (3 bars)
    BTRMGR_RSSI_EXCELLENT      //!< Excellent (4 bars)
} BTRMGR_RSSIValue_t;
/**
 * @brief Represents the bluetooth Discovery Status
 */
typedef enum _BTRMGR_DiscoveryStatus_t {
    BTRMGR_DISCOVERY_STATUS_OFF,
    BTRMGR_DISCOVERY_STATUS_IN_PROGRESS,
} BTRMGR_DiscoveryStatus_t;
/**
 * @brief Represents the commands to control the media files.
 */
typedef enum _BTRMGR_MediaControlCommand_t {
    BTRMGR_MEDIA_CTRL_PLAY,
    BTRMGR_MEDIA_CTRL_PAUSE,
    BTRMGR_MEDIA_CTRL_STOP,
    BTRMGR_MEDIA_CTRL_NEXT,
    BTRMGR_MEDIA_CTRL_PREVIOUS,
    BTRMGR_MEDIA_CTRL_FASTFORWARD,
    BTRMGR_MEDIA_CTRL_REWIND,
    BTRMGR_MEDIA_CTRL_VOLUMEUP,
    BTRMGR_MEDIA_CTRL_VOLUMEDOWN,
    BTRMGR_MEDIA_CTRL_EQUALIZER_OFF,
    BTRMGR_MEDIA_CTRL_EQUALIZER_ON,
    BTRMGR_MEDIA_CTRL_SHUFFLE_OFF,
    BTRMGR_MEDIA_CTRL_SHUFFLE_ALLTRACKS,
    BTRMGR_MEDIA_CTRL_SHUFFLE_GROUP,
    BTRMGR_MEDIA_CTRL_REPEAT_OFF,
    BTRMGR_MEDIA_CTRL_REPEAT_SINGLETRACK,
    BTRMGR_MEDIA_CTRL_REPEAT_ALLTRACKS,
    BTRMGR_MEDIA_CTRL_REPEAT_GROUP,
    BTRMGR_MEDIA_CTRL_SCAN_OFF,
    BTRMGR_MEDIA_CTRL_SCAN_ALLTRACKS,
    BTRMGR_MEDIA_CTRL_SCAN_GROUP,
    BTRMGR_MEDIA_CTRL_MUTE,
    BTRMGR_MEDIA_CTRL_UNMUTE,
    BTRMGR_MEDIA_CTRL_UNKNOWN
} BTRMGR_MediaControlCommand_t;
/**
 * @brief Represents LE properties.
 */
typedef enum _BTRMGR_LeProperty_t {  // looking for a better enum name
    BTRMGR_LE_PROP_UUID,
    BTRMGR_LE_PROP_PRIMARY,
    BTRMGR_LE_PROP_DEVICE,
    BTRMGR_LE_PROP_SERVICE,
    BTRMGR_LE_PROP_VALUE,
    BTRMGR_LE_PROP_NOTIFY,
    BTRMGR_LE_PROP_FLAGS,
    BTRMGR_LE_PROP_CHAR,
    BTRMGR_LE_PROP_DESC
} BTRMGR_LeProperty_t;
/**
 * @brief Represents the Low energy operations.
 */
typedef enum _BTRMGR_LeOp_t {
    BTRMGR_LE_OP_READY,
    BTRMGR_LE_OP_READ_VALUE,
    BTRMGR_LE_OP_WRITE_VALUE,
    BTRMGR_LE_OP_START_NOTIFY,
    BTRMGR_LE_OP_STOP_NOTIFY,
    BTRMGR_LE_OP_UNKNOWN
} BTRMGR_LeOp_t;
/**
 * @brief Represents Gatt Characteristic Flags.
 */
typedef enum _BTRMGR_GattCharFlags_t {
    BTRMGR_GATT_CHAR_FLAG_READ                         = 1 << 0,
    BTRMGR_GATT_CHAR_FLAG_WRITE                        = 1 << 1,
    BTRMGR_GATT_CHAR_FLAG_ENCRYPT_READ                 = 1 << 2,
    BTRMGR_GATT_CHAR_FLAG_ENCRYPT_WRITE                = 1 << 3,
    BTRMGR_GATT_CHAR_FLAG_ENCRYPT_AUTHENTICATED_READ   = 1 << 4,
    BTRMGR_GATT_CHAR_FLAG_ENCRYPT_AUTHENTICATED_WRITE  = 1 << 5,
    BTRMGR_GATT_CHAR_FLAG_SECURE_READ                  = 1 << 6,
    BTRMGR_GATT_CHAR_FLAG_SECURE_WRITE                 = 1 << 7,
    BTRMGR_GATT_CHAR_FLAG_NOTIFY                       = 1 << 8,
    BTRMGR_GATT_CHAR_FLAG_INDICATE                     = 1 << 9,
    BTRMGR_GATT_CHAR_FLAG_BROADCAST                    = 1 << 10,
    BTRMGR_GATT_CHAR_FLAG_WRITE_WITHOUT_RESPONSE       = 1 << 11,
    BTRMGR_GATT_CHAR_FLAG_AUTHENTICATED_SIGNED_WRITES  = 1 << 12,
    BTRMGR_GATT_CHAR_FLAG_RELIABLE_WRITE               = 1 << 13,
    BTRMGR_GATT_CHAR_FLAG_WRITABLE_AUXILIARIES         = 1 << 14
} BTRMGR_GattCharFlags_t;
typedef enum _BTRMGR_ScanFilter_t {
    BTRMGR_DISCOVERY_FILTER_UUID,
    BTRMGR_DISCOVERY_FILTER_RSSI,
    BTRMGR_DISCOVERY_FILTER_PATH_LOSS,
    BTRMGR_DISCOVERY_FILTER_SCAN_TYPE
} BTRMGR_ScanFilter_t;
/**
 * @brief Represents Media Element Types.
 */
typedef enum _BTRMGR_MediaElementType_t {
    BTRMGR_MEDIA_ELEMENT_TYPE_UNKNOWN,
    BTRMGR_MEDIA_ELEMENT_TYPE_ALBUM,
    BTRMGR_MEDIA_ELEMENT_TYPE_ARTIST,
    BTRMGR_MEDIA_ELEMENT_TYPE_GENRE,
    BTRMGR_MEDIA_ELEMENT_TYPE_COMPILATIONS,
    BTRMGR_MEDIA_ELEMENT_TYPE_PLAYLIST,
    BTRMGR_MEDIA_ELEMENT_TYPE_TRACKLIST,
    BTRMGR_MEDIA_ELEMENT_TYPE_TRACK
} BTRMGR_MediaElementType_t;
typedef enum _eBTRMGRDevMediaType_t {
    BTRMGR_DEV_MEDIA_TYPE_PCM,
    BTRMGR_DEV_MEDIA_TYPE_SBC,
    BTRMGR_DEV_MEDIA_TYPE_MPEG,
    BTRMGR_DEV_MEDIA_TYPE_AAC,
    BTRMGR_DEV_MEDIA_TYPE_Unknown
} eBTRMGRDevMediaType_t;
typedef enum _eBTRMGRDevMediaAChan_t {
    BTRMGR_DEV_MEDIA_CHANNEL_MONO,
    BTRMGR_DEV_MEDIA_CHANNEL_DUAL,
    BTRMGR_DEV_MEDIA_CHANNEL_STEREO,
    BTRMGR_DEV_MEDIA_CHANNEL_JOINT_STEREO,
    BTRMGR_DEV_MEDIA_CHANNEL_5_1,
    BTRMGR_DEV_MEDIA_CHANNEL_7_1,
    BTRMGR_DEV_MEDIA_CHANNEL_UNKNOWN
} eBTRMGRDevMediaAChan_t;
/**
 * @brief Represents the media track info.
 */
typedef struct _BTRMGR_MediaTrackInfo_t {
    char            pcAlbum[BTRMGR_MAX_STR_LEN];
    char            pcGenre[BTRMGR_MAX_STR_LEN];
    char            pcTitle[BTRMGR_MAX_STR_LEN];
    char            pcArtist[BTRMGR_MAX_STR_LEN];
    unsigned int    ui32TrackNumber;
    unsigned int    ui32Duration;
    unsigned int    ui32NumberOfTracks;
} BTRMGR_MediaTrackInfo_t;
/**
 * @brief Represents the media position info.
 */
typedef struct _BTRMGR_MediaPositionInfo_t {
    unsigned int          m_mediaDuration;
    unsigned int          m_mediaPosition;
} BTRMGR_MediaPositionInfo_t;
typedef struct _BTRMGR_LeUUID_t {
    unsigned short  flags;
    char            m_uuid[BTRMGR_NAME_LEN_MAX];
} BTRMGR_LeUUID_t;
/**
 * @brief Represents the supported service of the device.
 */
typedef struct _BTRMGR_DeviceService_t {
    unsigned short  m_uuid;
    char            m_profile[BTRMGR_NAME_LEN_MAX];
} BTRMGR_DeviceService_t;
/**
 * @brief Represents device services list.
 */
typedef struct _BTRMGR_DeviceServiceList_t {
    unsigned short          m_numOfService;
    union { /* have introduced BTRMGR_LeUUID_t inorder that the usage of BTRMGR_DeviceService_t shouldn't be confused
               if BTRMGR_DeviceService_t  alone is sufficient, then lets change in the next commit */
        BTRMGR_DeviceService_t  m_profileInfo[BTRMGR_MAX_DEVICE_PROFILE];
        BTRMGR_LeUUID_t         m_uuidInfo[BTRMGR_MAX_DEVICE_PROFILE];
    };
} BTRMGR_DeviceServiceList_t;
typedef struct _BTRMGR_DeviceAdServiceData_t {
    char            m_UUIDs[BTRMGR_UUID_STR_LEN_MAX];;
    unsigned char   m_ServiceData[BTRMGR_SERVICE_DATA_LEN_MAX];
    unsigned int    m_len;
} BTRMGR_DeviceAdServiceData_t;
/**
 * @brief Represents the property of the device.
 */
typedef struct _BTRMGR_DevicesProperty_t {
    BTRMgrDeviceHandle          m_deviceHandle;
    BTRMGR_DeviceType_t         m_deviceType;
    char                        m_name [BTRMGR_NAME_LEN_MAX];
    char                        m_deviceAddress [BTRMGR_NAME_LEN_MAX];
    int                         m_rssi;
    BTRMGR_RSSIValue_t          m_signalLevel;
    unsigned short              m_vendorID;
    unsigned char               m_isPaired;
    unsigned char               m_isConnected; /* This must be used only when m_isPaired is TRUE */
    unsigned char               m_isLowEnergyDevice;
    unsigned char               m_batteryLevel;
    BTRMGR_DeviceServiceList_t  m_serviceInfo;
    BTRMGR_DeviceAdServiceData_t m_adServiceData[BTRMGR_MAX_DEVICE_PROFILE];
    char                        m_modalias[BTRMGR_NAME_LEN_MAX/2];
    char                        m_firmwareRevision[BTRMGR_NAME_LEN_MAX/2];
} BTRMGR_DevicesProperty_t;
/**
 * @brief Represents the details of device connected.
 */
typedef struct _BTRMGR_ConnectedDevice_t {
    BTRMgrDeviceHandle          m_deviceHandle;
    BTRMGR_DeviceType_t         m_deviceType;
    char                        m_name [BTRMGR_NAME_LEN_MAX];
    char                        m_deviceAddress [BTRMGR_NAME_LEN_MAX];
    BTRMGR_DeviceServiceList_t  m_serviceInfo;
    unsigned short              m_vendorID;
    unsigned char               m_isLowEnergyDevice;
    unsigned char               m_isConnected; /* This must be used only when m_isPaired is TRUE */
    BTRMGR_DevicePower_t        m_powerStatus;
    unsigned int                m_ui32DevClassBtSpec;
    unsigned short              m_ui16DevAppearanceBleSpec;
} BTRMGR_ConnectedDevice_t;
/**
 * @brief Represents the paired devices information.
 */
typedef struct _BTRMGR_PairedDevices_t {
    BTRMgrDeviceHandle          m_deviceHandle;
    BTRMGR_DeviceType_t         m_deviceType;
    char                        m_name [BTRMGR_NAME_LEN_MAX];
    char                        m_deviceAddress [BTRMGR_NAME_LEN_MAX];
    BTRMGR_DeviceServiceList_t  m_serviceInfo;
    unsigned short              m_vendorID;
    unsigned char               m_isLowEnergyDevice;
    unsigned char               m_isConnected; /* This must be used only when m_isPaired is TRUE */
    unsigned char               m_isLastConnectedDevice;
    unsigned int                m_ui32DevClassBtSpec;
    unsigned short              m_ui16DevAppearanceBleSpec;
} BTRMGR_PairedDevices_t;
/**
 * @brief Represents the discovered device's details.
 */
typedef struct _BTRMGR_DiscoveredDevices_t {
    BTRMgrDeviceHandle  m_deviceHandle;
    BTRMGR_DeviceType_t m_deviceType;
    char                m_name [BTRMGR_NAME_LEN_MAX];
    char                m_deviceAddress [BTRMGR_NAME_LEN_MAX];
    unsigned short      m_vendorID;
    unsigned char       m_isPairedDevice;
    unsigned char       m_isConnected; /* This must be used only when m_isPaired is TRUE */
    unsigned char       m_isLowEnergyDevice;
    int                 m_rssi;
    BTRMGR_RSSIValue_t  m_signalLevel;
    unsigned char       m_isDiscovered;
    unsigned char       m_isLastConnectedDevice;
    unsigned int        m_ui32DevClassBtSpec;
    unsigned short      m_ui16DevAppearanceBleSpec;
} BTRMGR_DiscoveredDevices_t;
/**
 * @brief Represents the connected devices list.
 */
typedef struct _BTRMGR_ConnectedDevicesList_t {
    unsigned short              m_numOfDevices;
    BTRMGR_ConnectedDevice_t    m_deviceProperty[BTRMGR_DEVICE_COUNT_MAX];
} BTRMGR_ConnectedDevicesList_t;
/**
 * @brief Represents the list of paired devices.
 */
typedef struct _BTRMGR_PairedDevicesList_t {
    unsigned short          m_numOfDevices;
    BTRMGR_PairedDevices_t  m_deviceProperty[BTRMGR_DEVICE_COUNT_MAX];
} BTRMGR_PairedDevicesList_t;
/**
 * @brief Represents the list of scanned devices.
 */
typedef struct _BTRMGR_DiscoveredDevicesList_t {
    unsigned short              m_numOfDevices;
    BTRMGR_DiscoveredDevices_t  m_deviceProperty[BTRMGR_DISCOVERED_DEVICE_COUNT_MAX];
} BTRMGR_DiscoveredDevicesList_t;
/**
 * @brief Represents the details of external devices connected.
 */
typedef struct _BTRMGR_ExternalDevice_t {
    BTRMgrDeviceHandle          m_deviceHandle;
    BTRMGR_DeviceType_t         m_deviceType;
    char                        m_name [BTRMGR_NAME_LEN_MAX];
    char                        m_deviceAddress [BTRMGR_NAME_LEN_MAX];
    BTRMGR_DeviceServiceList_t  m_serviceInfo;
    unsigned short              m_vendorID;
    unsigned char               m_isLowEnergyDevice;
    unsigned int                m_externalDevicePIN;
    unsigned char               m_requestConfirmation;
} BTRMGR_ExternalDevice_t;
/**
 * @brief Represents Media Element details.
 */
typedef struct _BTRMGR_MediaElementInfo_t {
    BTRMgrMediaElementHandle    m_mediaElementHdl;
    unsigned char               m_IsPlayable;
    char                        m_mediaElementName[BTRMGR_MAX_STR_LEN];
    BTRMGR_MediaTrackInfo_t     m_mediaTrackInfo;
} BTRMGR_MediaElementInfo_t;
/**
  * @brief Represents Media Element List.
 */
typedef struct _BTRMGR_MediaElementListInfo_t {
    unsigned short              m_numberOfElements;
    BTRMGR_MediaElementInfo_t   m_mediaElementInfo[BTRMGR_MEDIA_ELEMENT_COUNT_MAX];
} BTRMGR_MediaElementListInfo_t;
/**
  * @brief Represents Media Element List.
 */
typedef struct _BTRMGR_MediaDeviceStatus_t {
    unsigned char                   m_ui8mediaDevVolume;
    unsigned char                   m_ui8mediaDevMute;
    BTRMGR_MediaControlCommand_t    m_enmediaCtrlCmd;
} BTRMGR_MediaDeviceStatus_t;
/**
 * @brief Represents the media info.
 */
typedef struct _BTRMGR_MediaInfo_t {
    BTRMgrDeviceHandle     m_deviceHandle;
    BTRMGR_DeviceType_t    m_deviceType;
    char                   m_name [BTRMGR_NAME_LEN_MAX];
    union {
        BTRMGR_MediaTrackInfo_t         m_mediaTrackInfo;
        BTRMGR_MediaPositionInfo_t      m_mediaPositionInfo;
        BTRMGR_MediaElementListInfo_t   m_mediaAlbumListInfo;
        BTRMGR_MediaElementListInfo_t   m_mediaArtistListInfo;
        BTRMGR_MediaElementListInfo_t   m_mediaGenreListInfo;
        BTRMGR_MediaElementListInfo_t   m_mediaCompilationInfo;
        BTRMGR_MediaElementListInfo_t   m_mediaPlayListInfo;
        BTRMGR_MediaElementListInfo_t   m_mediaTrackListInfo;
        char                            m_mediaPlayerName[BTRMGR_MAX_STR_LEN];
        unsigned char                   m_mediaPlayerVolume;
        unsigned short                  m_mediaPlayerDelay;
        BTRMGR_MediaDeviceStatus_t      m_mediaDevStatus;
    };
} BTRMGR_MediaInfo_t;
/**
 * @brief Represents the configuration for a PCM stream.
 */
typedef struct _BTRMGR_MediaPCMInfo_t {
    eBTRMGRDevMediaAChan_t  m_channelMode;
    unsigned int            m_freq;
    unsigned int            m_format;
} BTRMGR_MediaPCMInfo_t;
/**
 * @brief Represents the configuration for a SBC stream.
 */
typedef struct _BTRMGR_MediaSBCInfo_t {
    eBTRMGRDevMediaAChan_t  m_channelMode;
    unsigned int            m_freq;
    unsigned char           m_allocMethod;
    unsigned char           m_subbands;
    unsigned char           m_blockLength;
    unsigned char           m_minBitpool;
    unsigned char           m_maxBitpool;
    unsigned short          m_frameLen;
    unsigned short          m_bitrate;
} BTRMGR_MediaSBCInfo_t;
/**
 * @brief Represents the configuration for a Mpeg stream.
 */
typedef struct _BTRMGR_MediaMPEGInfo_t {
    eBTRMGRDevMediaAChan_t  m_channelMode;
    unsigned int            m_freq;
    unsigned char           m_crc;
    unsigned char           m_layer;
    unsigned char           m_mpf;
    unsigned char           m_rfa;
    unsigned short          m_frameLen;
    unsigned short          m_bitrate;
} BTRMGR_MediaMPEGInfo_t;
/**
 * @brief Represents the codec configuration for a stream.
 */
typedef struct _BTRMGR_MediaStreamInfo_t {
    eBTRMGRDevMediaType_t m_codec;
    union{
        BTRMGR_MediaSBCInfo_t    m_sbcInfo;
        BTRMGR_MediaMPEGInfo_t   m_mpegInfo;
        BTRMGR_MediaPCMInfo_t    m_pcmInfo;
    };
} BTRMGR_MediaStreamInfo_t;
/**
 * @brief Represents the battery info.
 */
typedef struct _BTRMGR_BatteryInfo_t {
        BTRMGR_DeviceType_t    m_deviceType;
        char                   m_name [BTRMGR_NAME_LEN_MAX];
        char                   m_uuid [BTRMGR_UUID_STR_LEN_MAX];
        BTRMgrDeviceHandle     m_deviceHandle;
        union {
            char               m_notifyData[BTRMGR_MAX_DEV_OP_DATA_LEN];
        };
} BTRMGR_BatteryInfo_t;
/**
    * @brief Represents the notification data
 */
typedef struct _BTRMGR_DeviceOpInfo_t {
    BTRMgrDeviceHandle              m_deviceHandle;
    BTRMGR_DeviceType_t             m_deviceType;
    BTRMGR_DeviceOperationType_t    m_deviceOpType;
    char                            m_deviceAddress[BTRMGR_NAME_LEN_MAX];
    char                            m_name[BTRMGR_NAME_LEN_MAX];
    char                            m_uuid[BTRMGR_MAX_STR_LEN];
    BTRMGR_LeOp_t                   m_leOpType;
    union {
        char                        m_readData[BTRMGR_MAX_DEV_OP_DATA_LEN];
        char                        m_writeData[BTRMGR_MAX_DEV_OP_DATA_LEN];
        char                        m_notifyData[BTRMGR_MAX_DEV_OP_DATA_LEN];
    };
} BTRMGR_DeviceOpInfo_t;
/**
 * @brief Represents the event message info.
 */
typedef struct _BTRMGR_EventMessage_t {
    unsigned char   m_adapterIndex;
    BTRMGR_Events_t m_eventType;
    union {
        BTRMGR_DiscoveredDevices_t  m_discoveredDevice;
        BTRMGR_ExternalDevice_t     m_externalDevice;
        BTRMGR_PairedDevices_t      m_pairedDevice;
        BTRMGR_MediaInfo_t          m_mediaInfo;
        BTRMGR_DeviceOpInfo_t       m_deviceOpInfo;
	BTRMGR_BatteryInfo_t        m_batteryInfo;
    };
} BTRMGR_EventMessage_t;
/**
 * @brief Represents the event response.
 */
typedef struct _BTRMGR_EventResponse_t {
    BTRMGR_Events_t     m_eventType;
    BTRMgrDeviceHandle  m_deviceHandle;
    char                m_writeData[BTRMGR_MAX_DEV_OP_DATA_LEN];
    union {
        unsigned char   m_eventResp;
    };
} BTRMGR_EventResponse_t;
typedef struct _BTRMGR_UUID_t {
    char**  m_uuid;
    short   m_uuidCount;
} BTRMGR_UUID_t;
typedef struct _BTRMGR_DiscoveryFilterHandle_t {
    
    BTRMGR_UUID_t               m_btuuid;
    int                         m_rssi;
    int                         m_pathloss;
    //BTRMGR_DeviceScanType_t     m_scanType;
} BTRMGR_DiscoveryFilterHandle_t;
/**
 * @brief Structure for the custom advertisement payload
 */
typedef struct _BTRMGR_LeCustomAdvertisement_t {
    unsigned char len_flags;
    unsigned char type_flags;
    unsigned char val_flags;
    unsigned char len_comcastflags;
    unsigned char type_comcastflags;
    unsigned char deviceInfo_UUID_LO;
    unsigned char deviceInfo_UUID_HI;
    unsigned char rdk_diag_UUID_LO;
    unsigned char rdk_diag_UUID_HI;
    unsigned char len_manuf;
    unsigned char type_manuf;
    /* First two bytes must contain the manufacturer ID (little-endian order) */
    unsigned char company_LO;
    unsigned char company_HI;
    unsigned short device_model;
    unsigned char serial_number[BTRMGR_SERIAL_NUM_LEN];
    unsigned char device_mac[BTRMGR_DEVICE_MAC_LEN];
} BTRMGR_LeCustomAdvertisement_t;
/* Fptr Callbacks types */
typedef BTRMGR_Result_t (*BTRMGR_EventCallback)(BTRMGR_EventMessage_t astEventMessage);
/* Interfaces */
/**
 * @brief  This API initializes the bluetooth manager.
 *
 * This API performs the following operations:
 *
 * - Initializes the bluetooth core layer.
 * - Initialize the Paired Device List for Default adapter.
 * - Register for callback to get the status of connected Devices.
 * - Register for callback to get the Discovered Devices.
 * - Register for callback to process incoming pairing requests.
 * - Register for callback to process incoming connection requests.
 * - Register for callback to process incoming media events.
 * - Activates the default agent.
 * - Initializes the persistant interface and saves all bluetooth profiles to the database.
 *
 * @return Returns the status of the operation.
 * @retval BTRMGR_RESULT_SUCCESS on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_Init(void);
/**
 * @brief  This API invokes the deinit function of bluetooth core and persistant interface module.
 *
 * @return Returns the status of the operation.
 * @retval BTRMGR_RESULT_SUCCESS on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_DeInit(void);
/**
 * @brief This API registers all the IARM call backs for BTRMGR and Third party Clients.
 * 
 * @return Returns the status of the operation.
 * @retval BTRMGR_RESULT_SUCCESS on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_RegisterForCallbacks(const char* apcProcessName);
/**
 * @brief This API Unregisters all the IARM call backs for BTRMGR and Third party Clients.
 *
 * @return Returns the status of the operation.
 * @retval BTRMGR_RESULT_SUCCESS on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_UnRegisterFromCallbacks(const char* apcProcessName);
/**
 * @brief  This API returns the number of bluetooth adapters available.
 *
 * @param[out] pNumOfAdapters    Indicates the number of adapters available.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_GetNumberOfAdapters(unsigned char *pNumOfAdapters);
/**
 * @brief  This API is designed to reset the bluetooth adapter.
 *
 * As of now, HAL implementation is not available for this API.
 *
 * @param[in] aui8AdapterIdx     Index of bluetooth adapter.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_ResetAdapter(unsigned char aui8AdapterIdx);
/**
 * @brief  This API is used to set the new name to the bluetooth adapter
 *
 * @param[in] aui8AdapterIdx     Index of bluetooth adapter.
 * @param[in] pNameOfAdapter     The name to set.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_SetAdapterName(unsigned char aui8AdapterIdx, const char* pNameOfAdapter);
/**
 * @brief  This API fetches the bluetooth adapter name.
 *
 * @param[in]  aui8AdapterIdx     Index of bluetooth adapter.
 * @param[out] pNameOfAdapter     Bluetooth adapter name.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_GetAdapterName(unsigned char aui8AdapterIdx, char* pNameOfAdapter);
/**
 * @brief  This API sets the bluetooth adapter power to ON/OFF.
 *
 * @param[in] aui8AdapterIdx     Index of bluetooth adapter.
 * @param[in] power_status        Value to set the  power. 0 to OFF & 1 to ON.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_SetAdapterPowerStatus(unsigned char aui8AdapterIdx, unsigned char power_status);
/**
 * @brief  This API fetches the power status, either 0 or 1.
 *
 * @param[in]  aui8AdapterIdx  Index of bluetooth adapter.
 * @param[out] pPowerStatus    Indicates the power status.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_GetAdapterPowerStatus(unsigned char aui8AdapterIdx, unsigned char *pPowerStatus);
/**
 * @brief  This API is to make the adapter discoverable until the given timeout.
 *
 * @param[in]  aui8AdapterIdx  Index of bluetooth adapter.
 * @param[in]  discoverable    Value to turn on or off the discovery.
 * @param[in]  timeout         Timeout to turn on discovery.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_SetAdapterDiscoverable(unsigned char aui8AdapterIdx, unsigned char discoverable, int timeout);
/**
 * @brief  This API checks the adapter is discoverable or not.
 *
 * @param[in]   aui8AdapterIdx  Index of bluetooth adapter.
 * @param[out]  pDiscoverable   Indicates discoverable or not.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_IsAdapterDiscoverable(unsigned char aui8AdapterIdx, unsigned char *pDiscoverable);
/**
 * @brief  This API initiates the scanning process.
 *
 * @param[in]   aui8AdapterIdx  Index of bluetooth adapter.
 * @param[in]   aenBTRMgrDevOpT Device operation type.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_StartDeviceDiscovery(unsigned char aui8AdapterIdx, BTRMGR_DeviceOperationType_t aenBTRMgrDevOpT);
/**
 * @brief  This API terminates the scanning process.
 *
 * @param[in]   aui8AdapterIdx  Index of bluetooth adapter.
 * @param[in]   aenBTRMgrDevOpT Device operation type.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_StopDeviceDiscovery(unsigned char aui8AdapterIdx, BTRMGR_DeviceOperationType_t aenBTRMgrDevOpT);
/**
 * @brief  This API gives the discovery status.
 *
 * @param[in]   aui8AdapterIdx  Index of bluetooth adapter.
 * @param[out]  aeDiscoveryStatus Device discovery status.
 * @param[out]  aenBTRMgrDevOpT  Device operation type.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_GetDiscoveryStatus (unsigned char aui8AdapterIdx, BTRMGR_DiscoveryStatus_t *aeDiscoveryStatus, BTRMGR_DeviceOperationType_t *aenBTRMgrDevOpT);
/**
 * @brief  This API fetches the list of devices scanned.
 *
 * @param[in]   aui8AdapterIdx        Index of bluetooth adapter.
 * @param[out]  pDiscoveredDevices    Structure which holds the details of device scanned.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_GetDiscoveredDevices(unsigned char aui8AdapterIdx, BTRMGR_DiscoveredDevicesList_t *pDiscoveredDevices);
/**
 * @brief  This API is used to pair the device that you wish to pair.
 *
 * @param[in] aui8AdapterIdx  Index of bluetooth adapter.
 * @param[in] ahBTRMgrDevHdl  Indicates the device handle.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_PairDevice(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl);
/**
 * @brief  This API is used to remove the pairing information of the device selected.
 *
 * @param[in]   aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]   ahBTRMgrDevHdl       Device handle.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_UnpairDevice(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl);
/**
 * @brief  This API returns the list of devices paired.
 *
 * @param[in] aui8AdapterIdx  Index of bluetooth adapter.
 * @param[in] pPairedDevices  Structure which holds the paired devices information.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_GetPairedDevices(unsigned char aui8AdapterIdx, BTRMGR_PairedDevicesList_t *pPairedDevices);
/**
 * @brief  This API connects the device as audio sink/headset/audio src based on the device type specified.
 *
 * @param[in] aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in] ahBTRMgrDevHdl        Indicates device handle.
 * @param[in] connectAs            Device operation type.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_ConnectToDevice(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_DeviceOperationType_t connectAs);
/**
 * @brief  This API terminates the current connection.
 *
 * @param[in] aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in] ahBTRMgrDevHdl        Indicates device handle.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_DisconnectFromDevice(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl);
/**
 * @brief  This API returns the list of devices connected.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[out] pConnectedDevices    List of connected devices.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_GetConnectedDevices(unsigned char aui8AdapterIdx, BTRMGR_ConnectedDevicesList_t *pConnectedDevices);
/**
 * @brief  This API returns the device information that includes the device name, mac address, RSSI value, battery level etc.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl       Indicates device handle.
 * @param[out] pDeviceProperty      Device property information.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_GetDeviceProperties(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_DevicesProperty_t *pDeviceProperty);
/**
 * @brief  This API returns the device battery level
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl       Indicates device handle.
 * @param[out] pDeviceBatteryLevel  Holds battery level, is 0 if battery level can't be found
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_GetDeviceBatteryLevel (unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl,unsigned char * pDeviceBatteryLevel);
/**
 * @brief  This API initates the streaming from the device with default operation type.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  aenBTRMgrDevConT     Device opeartion type.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_StartAudioStreamingOut_StartUp(unsigned char aui8AdapterIdx, BTRMGR_DeviceOperationType_t aenBTRMgrDevConT);
/**
 * @brief  This API initates the streaming from the device with the selected operation type.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl       Indicates device Handle.
 * @param[in]  connectAs            Device operation type.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_StartAudioStreamingOut(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_DeviceOperationType_t connectAs);
/**
 * @brief  This API terminates the streaming from the device.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl       Indicates device Handle.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_StopAudioStreamingOut(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl);
/**
 * @brief  This API returns the stream out status.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[out] pStreamingStatus     Streaming status.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
//TODO: Return deviceHandle if we are streaming out
BTRMGR_Result_t BTRMGR_IsAudioStreamingOut(unsigned char aui8AdapterIdx, unsigned char *pStreamingStatus);
/**
 * @brief  This API is to set the audio type as primary or secondary.
 *
 * Secondary audio support is not implemented yet. Always primary audio is played for now.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  type                 Streaming type primary/secondary
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_SetAudioStreamingOutType(unsigned char aui8AdapterIdx, BTRMGR_StreamOut_Type_t type);
/**
 * @brief  This API starts the audio streaming.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl       Device handle.
 * @param[in]  connectAs            Device opeartion type.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_StartAudioStreamingIn(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_DeviceOperationType_t connectAs);
/**
 * @brief  This API termines the audio streaming.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl       Device handle.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_StopAudioStreamingIn(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl);
/**
 * @brief  This API returns the audio streaming status.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[out] pStreamingStatus     Streaming status.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
//TODO: Return  deviceHandle if we are streaming in
BTRMGR_Result_t BTRMGR_IsAudioStreamingIn(unsigned char aui8AdapterIdx, unsigned char *pStreamingStatus);
/**
 * @brief  This API handles the events received.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  apstBTRMgrEvtRsp     Structure which holds the event response.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_SetEventResponse(unsigned char aui8AdapterIdx, BTRMGR_EventResponse_t* apstBTRMgrEvtRsp);
/**
 * @brief  This API is used to perform the media control operations.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl       Device handle.
 * @param[in]  mediaCtrlCmd         Indicates the play, pause, resume etc.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_MediaControl(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_MediaControlCommand_t mediaCtrlCmd);
/**
 * @brief  This API is used to fetch the data path and configuration of a streaming out device.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl       Device handle.
 * @param[out]  pi32dataPath        Device data path.
 * @param[out]  pi32Readmtu         Read mtu size.
 * @param[out]  pi32Writemtu        Write mtu size.
 * @param[out]  pi32Delay           Device delay needed.
 * @param[out]  pstBtrMgrDevStreamInfo Codec information.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_GetDataPathAndConfigurationForStreamOut( unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, int* pi32dataPath, int* pi32Readmtu, int* pi32Writemtu, unsigned int* pi32Delay, BTRMGR_MediaStreamInfo_t* pstBtrMgrDevStreamInfo);
/**
 * @brief  This API is used to release the data path of a streaming out device.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl       Device handle.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_ReleaseDataPathForStreamOut( unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl);
/**
 * @brief  This API is used to start streaming audio to a bluetooth device from a socket described by pcAudioInputFilePath.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl       Device handle.
 * @param[in]  pstMediaAudioOutInfo The audio information of the stream output.
 * @param[in]  pstMediaAudioInInfo  The audio information of the stream input.
 * @param[in]  i32OutFd             The output file descriptor of the bluetooth device.
 * @param[in]  outMTUSize           The output MTU size.
 * @param[in]  outDevDelay          The output device delay.
 * @param[in]  pcAudioInputFilePath The path to the socket from which audio will be consumed.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_StartSendingAudioFromFile(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_MediaStreamInfo_t* pstMediaAudioOutInfo, BTRMGR_MediaStreamInfo_t* pstMediaAudioInInfo, int i32OutFd, int outMTUSize,  unsigned int outDevDelay, char * pcAudioInputFilePath);
/**
 * @brief  This API is used to stop streaming to a bluetooth device when the stream was started with BTRMGR_StartSendingAudioFromFile.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_StopSendingAudioFromFile (void);
/**
 * @brief  This API is used to fetch the media volume and mute data.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl       Device handle.
 * @param[in]  deviceOpType         device operation type audio out or in etc.
 * @param[out]  pui8Volume          Media volume value.
 * @param[out]  pui8Mute            Media mute set or not.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_GetDeviceVolumeMute(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_DeviceOperationType_t deviceOpType, unsigned char *pui8Volume, unsigned char *pui8Mute);
/**
 * @brief  This API is used to set the media volume and mute data.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl       Device handle.
 * @param[in]  deviceOpType         device operation type audio out or in etc.
 * @param[out]  ui8Volume          Media volume value.
 * @param[out]  ui8Mute            Media mute set or not.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_SetDeviceVolumeMute(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_DeviceOperationType_t deviceOpType, unsigned char ui8Volume, unsigned char ui8Mute);
/**
 * @brief  This API is used to get the delay on the gstreamer pipeline.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl       Device handle.
 * @param[in]  deviceOpType         device operation type audio out or in etc.
 * @param[out]  pui16Delay          Media delay value.
 * @param[out]  pui16MsInBuffer     Media mute set or not.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_GetDeviceDelay (unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl,BTRMGR_DeviceOperationType_t deviceOpType,unsigned int* pui16Delay,unsigned int* pui16MsInBuffer);
/**
 * @brief  This API is used to set the delay on the gstreamer pipeline.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl       Device handle.
 * @param[in]  deviceOpType         device operation type audio out or in etc.
 * @param[out]  ui16Delay          Media volume delay.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_SetDeviceDelay ( unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl,BTRMGR_DeviceOperationType_t deviceOpType,unsigned int ui16Delay);
/**
 * @brief  This API fetches the media track info like title, genre, duration, number of tracks, current track number.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl       Device handle.
 * @param[out]  mediaTrackInfo       Track info like title, genre, duration etc.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_GetMediaTrackInfo(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_MediaTrackInfo_t *mediaTrackInfo);
/**
 * @brief  This API fetches the media track info like title, genre, duration, number of tracks, current track number.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl       Device handle.
 * @param[in]  ahBTRMgrMedElementHdl     Media Element handle .
 * @param[out]  mediaTrackInfo       Track info like title, genre, duration etc.
 *
 * @return Returns the status of the operation.
 * @retval eBTRMgrSuccess on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_GetMediaElementTrackInfo(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMgrMediaElementHandle ahBTRMgrMedElementHdl, BTRMGR_MediaTrackInfo_t *mediaTrackInfo);
/**
 * @brief  This API fetches the current position and total duration of the media.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl       Device handle.
 * @param[out] mediaPositionInfo    Media position info.
 *
 * @return Returns the status of the operation.
 * @retval BTRMGR_RESULT_SUCCESS on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_GetMediaCurrentPosition(unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMGR_MediaPositionInfo_t*  mediaPositionInfo);
/**
 * @brief  This API sets the mentioned media element list active/in_scope for further operations on it.
 *
 * @param[in]  aui8AdapterIdx            Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl            Device handle.
 * @param[in]  ahBTRMgrMedElementHdl     Media Element handle
 * @param[in]  aui16MediaElementStartIdx Starting index of the list.
 * @param[in]  aui16MediaElementEndIdx   Ending index of the list
 * @param[in]  aMediaElementType         Media Element Type.
 *
 * @return Returns the status of the operation.
 * @retval BTRMGR_RESULT_SUCCESS on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_SetMediaElementActive (unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMgrMediaElementHandle ahBTRMgrMedElementHdl, 
                                                                                                            BTRMGR_MediaElementType_t aMediaElementType);
/**
 * @brief  This API gets the media element list.
 * @param[in]  aui8AdapterIdx            Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl            Device handle.
 * @param[in]  ahBTRMgrMedElementHdl     Media Element handle
 * @param[in]  aui16MediaElementStartIdx Starting index of the list.
 * @param[in]  aui16MediaElementEndIdx   Ending index of the list
 * @param[in]  aMediaElementType         Media Element Type.
 * @param[out] aMediaElementListInfo     Media Element List.
 *
 * @return Returns the status of the operation.
 * @retval BTRMGR_RESULT_SUCCESS on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_GetMediaElementList (unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMgrMediaElementHandle ahBTRMgrMedElementHdl, unsigned short aui16MediaElementStartIdx,
                unsigned short aui16MediaElementEndIdx, unsigned char abMediaElementListDepth, BTRMGR_MediaElementType_t aMediaElementType, BTRMGR_MediaElementListInfo_t* aMediaElementListInfo);
/**
 * @brief  This API performs operation based on the element type selected.
 * @param[in]  aui8AdapterIdx            Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl            Device handle.
 * @param[in]  ahBTRMgrMedElementHdl     Media Element handle
 * @param[in]  aMediaElementType         Media Element Type.
 *
 * @return Returns the status of the operation.
 * @retval BTRMGR_RESULT_SUCCESS on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_SelectMediaElement (unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, BTRMgrMediaElementHandle ahBTRMgrMedElementHdl, BTRMGR_MediaElementType_t aMediaElementType);
/**
 * @brief  This API fetches the Device name of the media.
 *
 * @param[in]  type     Device type.
 *
 * @return Returns the device name.
 */
const char* BTRMGR_GetDeviceTypeAsString(BTRMGR_DeviceType_t type);
/**
 * @brief  This API Enable/Disables Audio-In on the specified bluetooth adapter.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  aui8State            0/1- Enable or Disable AudioIn service.
 *
 * @return Returns the status of the operation.
 * @retval BTRMGR_RESULT_SUCCESS on success.
 */
BTRMGR_Result_t BTRMGR_SetAudioInServiceState (unsigned char aui8AdapterIdx, unsigned char aui8State);
/**
 * @brief  This API Enable/Disables Hid GamePad on the specified bluetooth adapter.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  aui8State            0/1- Enable or Disable AudioIn service.
 *
 * @return Returns the status of the operation.
 * @retval BTRMGR_RESULT_SUCCESS on success.
 */
BTRMGR_Result_t BTRMGR_SetHidGamePadServiceState (unsigned char aui8AdapterIdx, unsigned char aui8State);
/**
 * @brief  This API Enable/Disables Debug Mode for the btrMgrBus process.
 *
 * @param[in]  aui8State            0/1- Enable or Disable Debug Mode.
 *
 * @return Returns the status of the operation.
 * @retval BTRMGR_RESULT_SUCCESS on success.
 */
BTRMGR_Result_t BTRMGR_SetBtmgrDebugModeState (unsigned char aui8State);
/**
 * @brief  This API Enable/Disables Broadcast feature.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  aui8State            0/1- Enable or Disable AudioIn service.
 *
 * @return Returns the status of the operation.
 * @retval BTRMGR_RESULT_SUCCESS on success.
 */
BTRMGR_Result_t BTRMGR_SetBroadcastState (unsigned char aui8AdapterIdx, unsigned char aui8State);
/**
 * @brief  This API Gets Beacon Detection status on the specified bluetooth adapter.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[out] isLimited            Current Beacon Detection Status.
 *
 * @return Returns the status of the operation.
 * @retval BTRMGR_RESULT_SUCCESS on success.
 */
BTRMGR_Result_t BTRMGR_GetLimitBeaconDetection(unsigned char aui8AdapterIdx, unsigned char *isLimited);
/**
 * @brief  This API Sets Beacon Detection status on the specified bluetooth adapter.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]   isLimited            Current Beacon Detection Status.
 *
 * @return Returns the status of the operation.
 * @retval BTRMGR_RESULT_SUCCESS on success.
 */
BTRMGR_Result_t BTRMGR_SetLimitBeaconDetection(unsigned char aui8AdapterIdx, unsigned char isLimited);
BTRMGR_Result_t BTRMGR_GetLeProperty (unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, const char* apBtrPropUuid, BTRMGR_LeProperty_t aenLeProperty, void* vpPropValue);
/**
 * @brief  This API fetches the characteristic uuid of Le device.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl       Device handle.
 * @param[in]  apBtrServiceUuid     service UUID.
 * @param[out] apBtrCharUuidList    uuid list.
 *
 * @return Returns the status of the operation.
 * @retval BTRMGR_RESULT_SUCCESS on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_GetLeCharacteristicUUID (unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, const char* apBtrServiceUuid, char* apBtrCharUuidList);
/**
 * @brief  This API performs LE operations on the specified bluetooth adapter.
 *
 * @param[in]  aui8AdapterIdx       Index of bluetooth adapter.
 * @param[in]  ahBTRMgrDevHdl       Device handle.
 * @param[in]  aBtrLeUuid           LE device uuid.
 * @param[in]  aLeOpType            LE device operation type.
 * @param[out] rOpResult            LE device operation result.
 *
 * @return Returns the status of the operation.
 * @retval BTRMGR_RESULT_SUCCESS on success, appropriate error code otherwise.
 */
BTRMGR_Result_t BTRMGR_PerformLeOp (unsigned char aui8AdapterIdx, BTRMgrDeviceHandle ahBTRMgrDevHdl, const char* aBtrLeUuid, BTRMGR_LeOp_t aLeOpType, char* aLeOpArg, char* rOpResult);
BTRMGR_Result_t BTRMGR_LE_StartAdvertisement(unsigned char aui8AdapterIdx, BTRMGR_LeCustomAdvertisement_t *pstBTMGR_LeCustomAdvt);
BTRMGR_Result_t BTRMGR_LE_StopAdvertisement(unsigned char aui8AdapterIdx);
BTRMGR_Result_t BTRMGR_LE_GetPropertyValue(unsigned char aui8AdapterIdx, char* lUUID, char *aValue, BTRMGR_LeProperty_t aElement);
BTRMGR_Result_t BTRMGR_LE_SetServiceUUIDs(unsigned char aui8AdapterIdx, char *aUUID);
BTRMGR_Result_t BTRMGR_LE_SetServiceInfo(unsigned char aui8AdapterIdx, char *aUUID, unsigned char aServiceType);
BTRMGR_Result_t BTRMGR_LE_SetGattInfo(unsigned char aui8AdapterIdx, char *aParentUUID, char *aCharUUID, unsigned short aFlags, char *aValue, BTRMGR_LeProperty_t aElement);
BTRMGR_Result_t BTRMGR_LE_SetGattPropertyValue(unsigned char aui8AdapterIdx, char* aUUID, char *aValue, BTRMGR_LeProperty_t aElement);
BTRMGR_Result_t BTRMGR_SysDiagInfo(unsigned char aui8AdapterIdx, char *apDiagElement, char *apValue, BTRMGR_LeOp_t aOpType);
BTRMGR_Result_t BTRMGR_ConnectToWifi(unsigned char aui8AdapterIdx, char *apSSID, char *apPassword, int aSecMode);
BTRMGR_Result_t BTRMGR_StartLEDeviceActivation(void);
BTRMGR_Result_t BTRMGR_SetBatteryOpsState(unsigned char aui8AdapterIdx, unsigned char aui8State);
BTRMGR_Result_t BTRMGR_LEDeviceActivation(void);
BTRMGR_Result_t BTRMGR_SetLTEServiceState(unsigned char aui8AdapterIdx, unsigned char aui8State);
// Outgoing callbacks Registration Interfaces
BTRMGR_Result_t BTRMGR_RegisterEventCallback(BTRMGR_EventCallback afpcBBTRMgrEventOut);
/** @} */
#ifdef __cplusplus
}
#endif
#endif /* __BTR_MGR_H__ */
