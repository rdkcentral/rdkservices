#pragma once

#include <string>

typedef int IARM_EventId_t;

typedef enum _IARM_Result_t {
    IARM_RESULT_SUCCESS,
    IARM_RESULT_INVALID_PARAM,
    IARM_RESULT_INVALID_STATE,
    IARM_RESULT_IPCCORE_FAIL,
    IARM_RESULT_OOM,
} IARM_Result_t;

#define IARM_BUS_DAEMON_NAME "Daemon"

typedef IARM_Result_t (*IARM_BusCall_t)(void* arg);
typedef void (*IARM_EventHandler_t)(const char* owner, IARM_EventId_t eventId, void* data, size_t len);

class IarmBusImpl {
public:
    virtual ~IarmBusImpl() = default;

    virtual IARM_Result_t IARM_Bus_Init(const char* name) = 0;
    virtual IARM_Result_t IARM_Bus_Connect() = 0;
    virtual IARM_Result_t IARM_Bus_IsConnected(const char* memberName, int* isRegistered) = 0;
    virtual IARM_Result_t IARM_Bus_RegisterEventHandler(const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) = 0;
    virtual IARM_Result_t IARM_Bus_UnRegisterEventHandler(const char* ownerName, IARM_EventId_t eventId) = 0;
    virtual IARM_Result_t IARM_Bus_RemoveEventHandler(const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) = 0;
    virtual IARM_Result_t IARM_Bus_Call(const char* ownerName, const char* methodName, void* arg, size_t argLen) = 0;
    virtual IARM_Result_t IARM_Bus_RegisterCall(const char* methodName, IARM_BusCall_t handler) = 0;
    virtual IARM_Result_t IARM_Bus_Call_with_IPCTimeout(const char *ownerName,  const char *methodName, void *arg, size_t argLen, int timeout) = 0;
};

class IarmBus {
public:
    static IarmBus& getInstance()
    {
        static IarmBus instance;
        return instance;
    }

    IarmBusImpl* impl;

    static IARM_Result_t IARM_Bus_Init(const char* name)
    {
        return getInstance().impl->IARM_Bus_Init(name);
    }

    static IARM_Result_t IARM_Bus_Connect()
    {
        return getInstance().impl->IARM_Bus_Connect();
    }

    static IARM_Result_t IARM_Bus_IsConnected(const char* memberName, int* isRegistered)
    {
        return getInstance().impl->IARM_Bus_IsConnected(memberName, isRegistered);
    }

    static IARM_Result_t IARM_Bus_RegisterEventHandler(const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler)
    {
        return getInstance().impl->IARM_Bus_RegisterEventHandler(ownerName, eventId, handler);
    }

    static IARM_Result_t IARM_Bus_UnRegisterEventHandler(const char* ownerName, IARM_EventId_t eventId)
    {
        return getInstance().impl->IARM_Bus_UnRegisterEventHandler(ownerName, eventId);
    }

    static IARM_Result_t IARM_Bus_RemoveEventHandler(const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler)
    {
        return getInstance().impl->IARM_Bus_RemoveEventHandler(ownerName, eventId, handler);
    }

    static IARM_Result_t IARM_Bus_Call(const char* ownerName, const char* methodName, void* arg, size_t argLen)
    {
        return getInstance().impl->IARM_Bus_Call(ownerName, methodName, arg, argLen);
    }

    static IARM_Result_t IARM_Bus_RegisterCall(const char* methodName, IARM_BusCall_t handler)
    {
        return getInstance().impl->IARM_Bus_RegisterCall(methodName, handler);
    }

    static IARM_Result_t IARM_Bus_Call_with_IPCTimeout(const char *ownerName,  const char *methodName, void *arg, size_t argLen, int timeout)
    {
        return getInstance().impl->IARM_Bus_Call_with_IPCTimeout(ownerName, methodName, arg, argLen, timeout);
    }
};

constexpr auto IARM_Bus_Init = &IarmBus::IARM_Bus_Init;
constexpr auto IARM_Bus_Connect = &IarmBus::IARM_Bus_Connect;
constexpr auto IARM_Bus_IsConnected = &IarmBus::IARM_Bus_IsConnected;
constexpr auto IARM_Bus_RegisterEventHandler = &IarmBus::IARM_Bus_RegisterEventHandler;
constexpr auto IARM_Bus_UnRegisterEventHandler = &IarmBus::IARM_Bus_UnRegisterEventHandler;
constexpr auto IARM_Bus_RemoveEventHandler = &IarmBus::IARM_Bus_RemoveEventHandler;
constexpr auto IARM_Bus_Call = &IarmBus::IARM_Bus_Call;
constexpr auto IARM_Bus_RegisterCall = &IarmBus::IARM_Bus_RegisterCall;
constexpr auto IARM_Bus_Call_with_IPCTimeout = &IarmBus::IARM_Bus_Call_with_IPCTimeout;

#define IARM_BUS_COMMON_API_SysModeChange "SysModeChange"

typedef enum _IARM_Bus_Daemon_PowerState_t {
    IARM_BUS_PWRMGR_POWERSTATE_OFF,
    IARM_BUS_PWRMGR_POWERSTATE_STANDBY,
    IARM_BUS_PWRMGR_POWERSTATE_ON,
    IARM_BUS_PWRMGR_POWERSTATE_STANDBY_LIGHT_SLEEP,
    IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP
} IARM_Bus_PowerState_t;

typedef IARM_Bus_PowerState_t IARM_Bus_PWRMgr_PowerState_t;

typedef enum _IARM_Bus_Daemon_SysMode_t {
    IARM_BUS_SYS_MODE_NORMAL,
    IARM_BUS_SYS_MODE_EAS,
    IARM_BUS_SYS_MODE_WAREHOUSE
} IARM_Bus_Daemon_SysMode_t;

#define IARMBUS_AUDIOCAPTUREMGR_NAME "audiocapturemgr"
#define IARMBUS_AUDIOCAPTUREMGR_REQUEST_SAMPLE "requestSample"

#define IARMBUS_AUDIOCAPTUREMGR_OPEN "open"
#define IARMBUS_AUDIOCAPTUREMGR_CLOSE "close"
#define IARMBUS_AUDIOCAPTUREMGR_START "start"
#define IARMBUS_AUDIOCAPTUREMGR_STOP "stop"
#define IARMBUS_AUDIOCAPTUREMGR_GET_DEFAULT_AUDIO_PROPS "getDefaultAudioProperties"
#define IARMBUS_AUDIOCAPTUREMGR_GET_AUDIO_PROPS "getAudioProperties"
#define IARMBUS_AUDIOCAPTUREMGR_GET_OUTPUT_PROPS "getOutputProperties"
#define IARMBUS_AUDIOCAPTUREMGR_SET_AUDIO_PROPERTIES "setAudioProperties"
#define IARMBUS_AUDIOCAPTUREMGR_SET_OUTPUT_PROPERTIES "setOutputProperties"

#define AUDIOCAPTUREMGR_FILENAME_PREFIX "audio_sample"
#define AUDIOCAPTUREMGR_FILE_PATH "/opt/"

#define DATA_CAPTURE_IARM_EVENT_AUDIO_CLIP_READY 100

#define ACM_RESULT_GENERAL_FAILURE 0
#define ACM_RESULT_PRECAPTURE_NOT_SUPPORTED 1
#define ACM_RESULT_STREAM_UNAVAILABLE 2
#define ACM_RESULT_DURATION_OUT_OF_BOUNDS 3
#define BUFFERED_FILE_OUTPUT ""

enum Format {
    acmFormate16BitStereo,
    acmFormate16BitMonoLeft,
    acmFormate16BitMonoRight,
    acmFormate16BitMono,
    acmFormate24BitStereo,
    acmFormate24Bit5_1
};

enum Sampling_frequency {
    acmFreqe48000,
    acmFreqe44100,
    acmFreqe32000,
    acmFreqe24000,
    acmFreqe16000
};

namespace audiocapturemgr {
using session_id_t = int;

struct audio_properties_ifce_t {
    Format format;
    Sampling_frequency sampling_frequency;
};
}

struct iarmbus_notification_payload_t {
    char dataLocator[64];
};

struct iarmbus_acm_arg_t {
    struct iarmbus_open_args {
        int source;
        std::string output_type;
    };

    struct iarmbus_request_payload_t {
        float duration;
        bool is_precapture;
    };

    struct output_t {
        unsigned int buffer_duration;
        unsigned int max_buffer_duration;
    };

    struct arg_output_props_t {
        output_t output;
    };

    struct Details {
        iarmbus_open_args arg_open;
        audiocapturemgr::audio_properties_ifce_t arg_audio_properties;
        iarmbus_request_payload_t arg_sample_request;
        arg_output_props_t arg_output_props;
    };

    int session_id;
    int result;
    Details details;
};

#define IARM_BUS_CECMGR_NAME "CECMgr"
#define IARM_BUS_CECMGR_MAX_DATA_LENGTH 62
#define IARM_BUS_CECMGR_API_Send "Send"

typedef struct _IARM_Bus_CECMgr_Send_Param_t {
    uint8_t length;
    uint8_t data[IARM_BUS_CECMGR_MAX_DATA_LENGTH + 1];
    uint8_t retVal;
} IARM_Bus_CECMgr_Send_Param_t;

#define IARM_BUS_DEEPSLEEPMGR_NAME "DEEPSLEEPMgr"

#define IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupReason "GetLastWakeupReason"
#define IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupKeyCode "GetLastWakeupKeycode"

typedef enum _DeepSleep_WakeupReason_t {
    DEEPSLEEP_WAKEUPREASON_IR = 0,
    DEEPSLEEP_WAKEUPREASON_RCU_BT,
    DEEPSLEEP_WAKEUPREASON_RCU_RF4CE,
    DEEPSLEEP_WAKEUPREASON_GPIO,
    DEEPSLEEP_WAKEUPREASON_LAN,
    DEEPSLEEP_WAKEUPREASON_WLAN,
    DEEPSLEEP_WAKEUPREASON_TIMER,
    DEEPSLEEP_WAKEUPREASON_FRONT_PANEL,
    DEEPSLEEP_WAKEUPREASON_WATCHDOG,
    DEEPSLEEP_WAKEUPREASON_SOFTWARE_RESET,
    DEEPSLEEP_WAKEUPREASON_THERMAL_RESET,
    DEEPSLEEP_WAKEUPREASON_WARM_RESET,
    DEEPSLEEP_WAKEUPREASON_COLDBOOT,
    DEEPSLEEP_WAKEUPREASON_STR_AUTH_FAILURE,
    DEEPSLEEP_WAKEUPREASON_CEC,
    DEEPSLEEP_WAKEUPREASON_PRESENCE,
    DEEPSLEEP_WAKEUPREASON_VOICE,
    DEEPSLEEP_WAKEUPREASON_UNKNOWN
} DeepSleep_WakeupReason_t;

typedef struct _IARM_Bus_DeepSleepMgr_WakeupKeyCode_Param_t {
    unsigned int keyCode;
} IARM_Bus_DeepSleepMgr_WakeupKeyCode_Param_t;

#define IARM_BUS_MFRLIB_NAME "MFRLib"
#define IARM_BUS_MFRLIB_API_SetBootLoaderPattern "mfrSetBootloaderPattern"
#define IARM_BUS_MFRLIB_API_GetSerializedData "mfrGetManufacturerData"

typedef enum _mfrSerializedType_t {
    mfrSERIALIZED_TYPE_MANUFACTURER = 0,
    mfrSERIALIZED_TYPE_MANUFACTUREROUI,
    mfrSERIALIZED_TYPE_MODELNAME,
    mfrSERIALIZED_TYPE_DESCRIPTION,
    mfrSERIALIZED_TYPE_PRODUCTCLASS,
    mfrSERIALIZED_TYPE_SERIALNUMBER,
    mfrSERIALIZED_TYPE_HARDWAREVERSION,
    mfrSERIALIZED_TYPE_SOFTWAREVERSION,
    mfrSERIALIZED_TYPE_PROVISIONINGCODE,
    mfrSERIALIZED_TYPE_FIRSTUSEDATE,
    mfrSERIALIZED_TYPE_DEVICEMAC,
    mfrSERIALIZED_TYPE_MOCAMAC,
    mfrSERIALIZED_TYPE_HDMIHDCP,
    mfrSERIALIZED_TYPE_PDRIVERSION,
    mfrSERIALIZED_TYPE_WIFIMAC,
    mfrSERIALIZED_TYPE_BLUETOOTHMAC,
    mfrSERIALIZED_TYPE_WPSPIN,
    mfrSERIALIZED_TYPE_MANUFACTURING_SERIALNUMBER,
    mfrSERIALIZED_TYPE_ETHERNETMAC,
    mfrSERIALIZED_TYPE_ESTBMAC,
    mfrSERIALIZED_TYPE_RF4CEMAC,
    mfrSERIALIZED_TYPE_PROVISIONED_MODELNAME,
    mfrSERIALIZED_TYPE_PMI,
    mfrSERIALIZED_TYPE_HWID,
    mfrSERIALIZED_TYPE_MODELNUMBER,
    /* boot data */
    mfrSERIALIZED_TYPE_SOC_ID,
    mfrSERIALIZED_TYPE_IMAGENAME,
    mfrSERIALIZED_TYPE_IMAGETYPE,
    mfrSERIALIZED_TYPE_BLVERSION,
    /* provisional data */
    mfrSERIALIZED_TYPE_REGION,
    /* other data */
    mfrSERIALIZED_TYPE_BDRIVERSION,
    /* led data */
    mfrSERIALIZED_TYPE_LED_WHITE_LEVEL,
    mfrSERIALIZED_TYPE_LED_PATTERN,
    mfrSERIALIZED_TYPE_MAX,
#ifdef PANEL_SERIALIZATION_TYPES
    mfrSERIALIZED_TYPE_COREBOARD_SERIALNUMBER=0x51,
    mfrSERIALIZED_TYPE_FACTORYBOOT,
    mfrSERIALIZED_TYPE_COUNTRYCODE,
    mfrSERIALIZED_TYPE_LANGUAGECODE,
    mfrSERIALIZED_TYPE_MANUFACTURERDATA,
    mfrSERIALIZED_TYPE_CPD_SIZE,
    mfrSERIALIZED_TYPE_PANEL_ID,
    mfrSERIALIZED_TYPE_PANEL_TYPE,
    mfrSERIALIZED_TYPE_PANEL_HDMI_WB_DATA_NORMAL,
    mfrSERIALIZED_TYPE_PANEL_HDMI_WB_DATA_COLD,
    mfrSERIALIZED_TYPE_PANEL_HDMI_WB_DATA_WARM,
    mfrSERIALIZED_TYPE_PANEL_HDMI_WB_DATA_USER,
    mfrSERIALIZED_TYPE_PANEL_TV_WB_DATA_NORMAL,
    mfrSERIALIZED_TYPE_PANEL_TV_WB_DATA_COLD,
    mfrSERIALIZED_TYPE_PANEL_TV_WB_DATA_WARM,
    mfrSERIALIZED_TYPE_PANEL_TV_WB_DATA_USER,
    mfrSERIALIZED_TYPE_PANEL_AV_WB_DATA_NORMAL,
    mfrSERIALIZED_TYPE_PANEL_AV_WB_DATA_COLD,
    mfrSERIALIZED_TYPE_PANEL_AV_WB_DATA_WARM,
    mfrSERIALIZED_TYPE_PANEL_AV_WB_DATA_USER,
    mfrSERIALIZED_TYPE_PANEL_DTB_VERSION,
    mfrSERIALIZED_TYPE_PANEL_DTB_DATA_SIZE,
    mfrSERIALIZED_TYPE_PANEL_DTB_DATA,
    /* panel data*/
    mfrSERIALIZED_TYPE_PANEL_DATA_FUNCTION_STATUS,
    mfrSERIALIZED_TYPE_PANEL_DATA_AGEING_TIME,
    mfrSERIALIZED_TYPE_PANEL_DATA_POWER_ON_TIME,
    mfrSERIALIZED_TYPE_PANEL_DATA_BACKLIGHT_TIME,
    mfrSERIALIZED_TYPE_PANEL_DATA_VALID,
    mfrSERIALIZED_TYPE_PANEL_DATA_TPV_APP_VERSION,
    mfrSERIALIZED_TYPE_PANEL_ALS_CALIBRATION_INDEX0,
    mfrSERIALIZED_TYPE_PANEL_ALS_CALIBRATION_INDEX1,
    mfrSERIALIZED_TYPE_MAX_PANEL,
#endif
} mfrSerializedType_t;

typedef enum _mfrBlPattern_t {
    mfrBL_PATTERN_NORMAL = 0,
    mfrBL_PATTERN_SILENT,
    mfrBL_PATTERN_SILENT_LED_ON
} mfrBlPattern_t;

typedef struct _IARM_Bus_MFRLib_SetBLPattern_Param_t {
    mfrBlPattern_t pattern;
} IARM_Bus_MFRLib_SetBLPattern_Param_t;

typedef struct _IARM_Bus_MFRLib_GetSerializedData_Param_t {
    mfrSerializedType_t type;
    char buffer[1280];
    int bufLen;
} IARM_Bus_MFRLib_GetSerializedData_Param_t;

#define PWRMGR_MAX_REBOOT_REASON_LENGTH 100

#define MAX_PWR_STATE_BEF_REBOOR_STR_LEN (32)
#define IARM_BUS_PWRMGR_API_GetPowerStateBeforeReboot "GetPowerStateBeforeReboot"
#define IARM_BUS_PWRMGR_API_SetStandbyVideoState "SetStandbyVideoState"
#define IARM_BUS_PWRMGR_API_GetStandbyVideoState "GetStandbyVideoState"
#define IARM_BUS_PWRMGR_API_SetNetworkStandbyMode "SetNetworkStandbyMode"
#define IARM_BUS_PWRMGR_API_GetNetworkStandbyMode "GetNetworkStandbyMode"
#define IARM_BUS_PWRMGR_API_Reboot "performReboot"
#define IARM_BUS_PWRMGR_API_WareHouseClear "WarehouseClear" /*!< */
#define IARM_BUS_PWRMGR_API_ColdFactoryReset "ColdFactoryReset" /*!< Reset the box to cold factory state*/
#define IARM_BUS_PWRMGR_API_FactoryReset "FactoryReset" /*!< Reset the box to factory state*/
#define IARM_BUS_PWRMGR_API_UserFactoryReset "UserFactoryReset" /*!< Reset the box to user factory state*/
#define IARM_BUS_PWRMGR_API_WareHouseReset  "WareHouseReset" /*!< Reset the box to warehouse state*/

typedef struct _IARM_Bus_PWRMgr_NetworkStandbyMode_Param_t {
    bool bStandbyMode;
} IARM_Bus_PWRMgr_NetworkStandbyMode_Param_t;

typedef struct _IARM_Bus_PWRMgr_GetPowerStateBeforeReboot_Param_t {
    char powerStateBeforeReboot[MAX_PWR_STATE_BEF_REBOOR_STR_LEN];
} IARM_Bus_PWRMgr_GetPowerStateBeforeReboot_Param_t;

typedef struct _IARM_Bus_PWRMgr_SetDeepSleepTimeOut_Param_t {
    unsigned int timeout;
} IARM_Bus_PWRMgr_SetDeepSleepTimeOut_Param_t;

typedef enum _IARM_Bus_PWRMgr_ThermalState_t {
    IARM_BUS_PWRMGR_TEMPERATURE_NORMAL = 0,
    IARM_BUS_PWRMGR_TEMPERATURE_HIGH,
    IARM_BUS_PWRMGR_TEMPERATURE_CRITICAL
} IARM_Bus_PWRMgr_ThermalState_t;

typedef enum _WakeupSrcType_t {
    WAKEUPSRC_VOICE = 0,
    WAKEUPSRC_PRESENCE_DETECTION,
    WAKEUPSRC_BLUETOOTH,
    WAKEUPSRC_WIFI,
    WAKEUPSRC_IR,
    WAKEUPSRC_POWER_KEY,
    WAKEUPSRC_TIMER,
    WAKEUPSRC_CEC,
    WAKEUPSRC_LAN
} WakeupSrcType_t;

typedef struct _PWRMgr_EventData_t {
    union {
        struct _MODE_DATA {
            IARM_Bus_PWRMgr_PowerState_t curState;
            IARM_Bus_PWRMgr_PowerState_t newState;
            uint32_t deep_sleep_timeout;
            bool nwStandbyMode;
        } state;
        struct _THERM_DATA {
            IARM_Bus_PWRMgr_ThermalState_t curLevel;
            IARM_Bus_PWRMgr_ThermalState_t newLevel;
            float curTemperature;
        } therm;
        bool bNetworkStandbyMode;
        int32_t reset_sequence_progress;
    } data;
} IARM_Bus_PWRMgr_EventData_t;

typedef struct _IARM_Bus_PWRMgr_GetThermalState_Param_t {
    IARM_Bus_PWRMgr_ThermalState_t curLevel;
    float curTemperature;
} IARM_Bus_PWRMgr_GetThermalState_Param_t;

typedef struct _IARM_Bus_PWRMgr_GetTempThresholds_Param_t {
    float tempHigh;
    float tempCritical;
} IARM_Bus_PWRMgr_GetTempThresholds_Param_t;

typedef struct _IARM_Bus_PWRMgr_SetTempThresholds_Param_t {
    float tempHigh;
    float tempCritical;
} IARM_Bus_PWRMgr_SetTempThresholds_Param_t;

typedef struct _IARM_Bus_PWRMgr_GetOvertempGraceInterval_Param_t {
    int graceInterval;
} IARM_Bus_PWRMgr_GetOvertempGraceInterval_Param_t;

typedef struct _IARM_Bus_PWRMgr_SetOvertempGraceInterval_Param_t {
    int graceInterval;
} IARM_Bus_PWRMgr_SetOvertempGraceInterval_Param_t;

typedef struct _IARM_Bus_CommonAPI_SysModeChange_Param_t {
    IARM_Bus_Daemon_SysMode_t oldMode;
    IARM_Bus_Daemon_SysMode_t newMode;
} IARM_Bus_CommonAPI_SysModeChange_Param_t;

typedef enum _PWRMgr_EventId_t {
    IARM_BUS_PWRMGR_EVENT_MODECHANGED = 0,
    IARM_BUS_PWRMGR_EVENT_DEEPSLEEP_TIMEOUT,
    IARM_BUS_PWRMGR_EVENT_RESET_SEQUENCE,
    IARM_BUS_PWRMGR_EVENT_REBOOTING,
    IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED,
    IARM_BUS_PWRMGR_EVENT_WAREHOUSEOPS_STATUSCHANGED,
    IARM_BUS_PWRMGR_EVENT_NETWORK_STANDBYMODECHANGED,
    IARM_BUS_PWRMGR_EVENT_MAX,
} IARM_Bus_PWRMgr_EventId_t;

typedef struct _IARM_Bus_PWRMgr_RebootParam_t {
    char reboot_reason_custom[PWRMGR_MAX_REBOOT_REASON_LENGTH];
    char reboot_reason_other[PWRMGR_MAX_REBOOT_REASON_LENGTH];
    char requestor[PWRMGR_MAX_REBOOT_REASON_LENGTH];
} IARM_Bus_PWRMgr_RebootParam_t;

#define IARM_BUS_PWRMGR_NAME "PWRMgr"
#define IARM_BUS_PWRMGR_API_SetPowerState "SetPowerState"
#define IARM_BUS_PWRMGR_API_GetPowerState "GetPowerState"

typedef struct _IARM_Bus_PWRMgr_SetPowerState_Param_t {
    IARM_Bus_PWRMgr_PowerState_t newState;
    int keyCode;
} IARM_Bus_PWRMgr_SetPowerState_Param_t;

typedef struct _IARM_Bus_PWRMgr_GetPowerState_Param_t {
    IARM_Bus_PWRMgr_PowerState_t curState;
} IARM_Bus_PWRMgr_GetPowerState_Param_t;

#define IARM_BUS_PWRMGR_API_GetThermalState "GetThermalState"
#define IARM_BUS_PWRMGR_API_GetTemperatureThresholds "GetTemperatureThresholds"
#define IARM_BUS_PWRMGR_API_SetTemperatureThresholds "SetTemperatureThresholds"
#define IARM_BUS_PWRMGR_API_GetOvertempGraceInterval "GetOvertempGraceInterval"
#define IARM_BUS_PWRMGR_API_SetOvertempGraceInterval "SetOvertempGraceInterval"

typedef struct _IARM_Bus_PWRMgr_SetWakeupSrcConfig_Param_t {
    WakeupSrcType_t srcType;
    bool config;
} IARM_Bus_PWRMgr_SetWakeupSrcConfig_Param_t;

/**
 *  @brief Structure which holds the ware house reset time.
 */
typedef struct _IARM_Bus_PWRMgr_WareHouseReset_Param_t {
        bool suppressReboot; /*!< STB should not be rebooted */
} IARM_Bus_PWRMgr_WareHouseReset_Param_t;

/**
 *  @brief Enumerator which represents the  possible warehouse ops
 */
typedef enum _IARM_Bus_PWRMgr_WareHouseOpsStatus_t{
    IARM_BUS_PWRMGR_WAREHOUSE_COMPLETED = 0,    /* warehouse operation completed sucessfully */
    IARM_BUS_PWRMGR_WAREHOUSE_INPROGRESS,          /* warehouse operation in progress*/
    IARM_BUS_PWRMGR_WAREHOUSE_FAILED,          /* warehouse operation failed.*/
} IARM_Bus_PWRMgr_WareHouseOpsStatus_t;

/**
 *  @brief Enumerator which represents the  possible warehouse ops
 */
typedef enum _IARM_Bus_PWRMgr_WareHouseOps_t{
    IARM_BUS_PWRMGR_WAREHOUSE_RESET = 0,    /* warehouse reset */
    IARM_BUS_PWRMGR_WAREHOUSE_CLEAR,          /* warehouse clear*/
} IARM_Bus_PWRMgr_WareHouseOps_t;

/**
 *  @brief Structure which holds warehouse opn status value.
 */
typedef struct _IARM_BUS_PWRMgr_WareHouseOpn_EventData_t {
        IARM_Bus_PWRMgr_WareHouseOps_t wareHouseOpn;        /*!< WareHouse operation*/
        IARM_Bus_PWRMgr_WareHouseOpsStatus_t status;        /*!< WareHouse operation status*/
} IARM_BUS_PWRMgr_WareHouseOpn_EventData_t;

#define IARM_BUS_PWRMGR_API_SetWakeupSrcConfig "setWakeupSrcConfig"

#define IARM_BUS_SYSMGR_NAME "SYSMgr"
#define IARM_BUS_SYSMGR_API_GetSystemStates "GetSystemStates"

typedef enum _SYSMgr_EventId_t {
    IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE,
    IARM_BUS_SYSMGR_EVENT_XUPNP_DATA_REQUEST, /*!< Xupnp data  request frm Receiver to UPNP*/
    IARM_BUS_SYSMGR_EVENT_XUPNP_DATA_UPDATE, /*!< Xupnp data  frm  UPNP to  Receiver*/
    IARM_BUS_SYSMGR_EVENT_IMAGE_DNLD, /*Image download status to Receiver   */
    IARM_BUS_SYSMGR_EVENT_CARD_FWDNLD, /*!< CARD FW download event to Receiver - Valid on Cisco CA*/
    IARM_BUS_SYSMGR_EVENT_HDCP_PROFILE_UPDATE, /*!< HDCP Profile Update */
    IARM_BUS_SYSMGR_EVENT_INTRUSION, /*!< Intrusion Detection */
    IARM_BUS_SYSMGR_EVENT_EISS_FILTER_STATUS, /*!< EISS Filter status event */
    IARM_BUS_SYSMGR_EVENT_EISS_APP_ID_UPDATE, /*!< EISS App ID update */
    IARM_BUS_SYSMGR_EVENT_KEYCODE_LOGGING_CHANGED, /*!< Key Code logging status update */
    IARM_BUS_SYSMGR_EVENT_USB_MOUNT_CHANGED, /*!< Fires when USB mounts change */
    IARM_BUS_SYSMGR_EVENT_APP_RELEASE_FOCUS, /*!< Application fires event to release focus*/
    IARM_BUS_SYSMGR_EVENT_MAX /*!< Max Event Id */
} IARM_Bus_SYSMgr_EventId_t;

typedef struct _IARM_BUS_SYSMGR_KEYCodeLoggingInfo_Param_t {
    int logStatus; /*!< HDCP Profile Data */
} IARM_BUS_SYSMGR_KEYCodeLoggingInfo_Param_t;

#define IARM_BUS_SYSMGR_API_GetKeyCodeLoggingPref "GetKeyCodeLoggingPref"
#define IARM_BUS_SYSMGR_API_SetKeyCodeLoggingPref "SetKeyCodeLoggingPref"

typedef enum _SYSMgr_SystemState_t {
    IARM_BUS_SYSMGR_SYSSTATE_CHANNELMAP, //0
    IARM_BUS_SYSMGR_SYSSTATE_DISCONNECTMGR, //1
    IARM_BUS_SYSMGR_SYSSTATE_TUNEREADY, //2
    IARM_BUS_SYSMGR_SYSSTATE_EXIT_OK, //3
    IARM_BUS_SYSMGR_SYSSTATE_CMAC, //4
    IARM_BUS_SYSMGR_SYSSTATE_MOTO_ENTITLEMENT, //5
    IARM_BUS_SYSMGR_SYSSTATE_MOTO_HRV_RX, //6
    IARM_BUS_SYSMGR_SYSSTATE_CARD_CISCO_STATUS, //7
    IARM_BUS_SYSMGR_SYSSTATE_VIDEO_PRESENTING, //8
    IARM_BUS_SYSMGR_SYSSTATE_HDMI_OUT, //9
    IARM_BUS_SYSMGR_SYSSTATE_HDCP_ENABLED, //10
    IARM_BUS_SYSMGR_SYSSTATE_HDMI_EDID_READ, //11
    IARM_BUS_SYSMGR_SYSSTATE_FIRMWARE_DWNLD, //12
    IARM_BUS_SYSMGR_SYSSTATE_TIME_SOURCE, //13
    IARM_BUS_SYSMGR_SYSSTATE_TIME_ZONE, //14
    IARM_BUS_SYSMGR_SYSSTATE_CA_SYSTEM, //15
    IARM_BUS_SYSMGR_SYSSTATE_ESTB_IP, //16
    IARM_BUS_SYSMGR_SYSSTATE_ECM_IP, //17
    IARM_BUS_SYSMGR_SYSSTATE_LAN_IP, //18
    IARM_BUS_SYSMGR_SYSSTATE_MOCA, //19
    IARM_BUS_SYSMGR_SYSSTATE_DOCSIS, //20
    IARM_BUS_SYSMGR_SYSSTATE_DSG_BROADCAST_CHANNEL, //21
    IARM_BUS_SYSMGR_SYSSTATE_DSG_CA_TUNNEL, //22
    IARM_BUS_SYSMGR_SYSSTATE_CABLE_CARD, //23
    IARM_BUS_SYSMGR_SYSSTATE_CABLE_CARD_DWNLD, //24
    IARM_BUS_SYSMGR_SYSSTATE_CVR_SUBSYSTEM, //25
    IARM_BUS_SYSMGR_SYSSTATE_DOWNLOAD, //26
    IARM_BUS_SYSMGR_SYSSTATE_VOD_AD, //27
    IARM_BUS_SYSMGR_SYSSTATE_DAC_INIT_TIMESTAMP, //28
    IARM_BUS_SYSMGR_SYSSTATE_CABLE_CARD_SERIAL_NO, //29
    IARM_BUS_SYSMGR_SYSSTATE_ECM_MAC, //30
    IARM_BUS_SYSMGR_SYSSTATE_DAC_ID, //31
    IARM_BUS_SYSMGR_SYSSTATE_PLANT_ID, //32
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
    IARM_BUS_SYSMGR_SYSSTATE_FIRMWARE_UPDATE_STATE, //43, Added as part of RDK-19978, As the IARM
    IARM_BUS_SYSMGR_SYSSTATE_USB_DETECTED, //44
    IARM_BUS_SYSMGR_SYSSTATE_LOG_UPLOAD, //45
} IARM_Bus_SYSMgr_SystemState_t;

typedef enum _SYSMgr_FirmwareUpdateState_t {
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

typedef enum _SYSMgr_LogUpload_t
{
  IARM_BUS_SYSMGR_LOG_UPLOAD_SUCCESS = 0,
  IARM_BUS_SYSMGR_LOG_UPLOAD_FAILED = 1,
  IARM_BUS_SYSMGR_LOG_UPLOAD_ABORTED = 2,
} IARM_Bus_SYSMGR_SYSMgr_LogUpload_t;

typedef struct _IARM_BUS_SYSMgr_EventData_t {
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

typedef struct _propertyValue {
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

#define IARM_BUS_DSMGR_NAME "DSMgr"

/*! Events published from DS Mananger */
typedef enum _DSMgr_EventId_t {
    IARM_BUS_DSMGR_EVENT_RES_PRECHANGE = 0, /*!< Resolution Pre Change Event  */
    IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE, /*!< Resolution Post Change Event  */
    IARM_BUS_DSMGR_EVENT_ZOOM_SETTINGS, /*!< Zoom Settings Events */
    IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG, /*!< HDMI HPD DETECT Change Event  */
    IARM_BUS_DSMGR_EVENT_AUDIO_MODE, /*!< HDMI HPD DETECT Change Event  */
    IARM_BUS_DSMGR_EVENT_HDCP_STATUS, /*!< HDMI HDCP status */
    IARM_BUS_DSMGR_EVENT_RX_SENSE, /*!< HDMI Rx Sense status */
    IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG, /*!< HDMI IN HPD change event */
    IARM_BUS_DSMGR_EVENT_HDMI_IN_SIGNAL_STATUS, /*!< HDMI IN signal status change event */
    IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS, /*!< HDMI IN status change event */
    IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE, /*!< HDMI IN video mode update event */
    IARM_BUS_DSMGR_EVENT_HDMI_IN_ALLM_STATUS, /*!< HDMI IN ALLM mode update event */
    IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_HOTPLUG, /*!< COMPOSITE IN HPD change event */
    IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_SIGNAL_STATUS, /*!< COMPOSITE IN signal status change event */
    IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_STATUS, /*!< COMPOSITE IN status change event */
    IARM_BUS_DSMGR_EVENT_TIME_FORMAT_CHANGE, /*!< Clock Time Format Change Event*/
    IARM_BUS_DSMGR_EVENT_AUDIO_LEVEL_CHANGED, /*!< Audio Level Change Event*/
    IARM_BUS_DSMGR_EVENT_AUDIO_OUT_HOTPLUG, /*!< AUDIO OUT HPD change event */
    IARM_BUS_DSMGR_EVENT_AUDIO_FORMAT_UPDATE, /*!< Audio Format change event */
    IARM_BUS_DSMGR_EVENT_AUDIO_PRIMARY_LANGUAGE_CHANGED, /*!< Audio Primary Language change event */
    IARM_BUS_DSMGR_EVENT_AUDIO_SECONDARY_LANGUAGE_CHANGED, /*!< Audio Secondary Language change event */
    IARM_BUS_DSMGR_EVENT_AUDIO_FADER_CONTROL_CHANGED, /*!< Audio Fader Control change event */
    IARM_BUS_DSMGR_EVENT_AUDIO_ASSOCIATED_AUDIO_MIXING_CHANGED, /*!< Audio Associated Audio Mixing change event */
    IARM_BUS_DSMGR_EVENT_VIDEO_FORMAT_UPDATE, /*!< Video Format change event */
    IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_PRECHANGE, /*!< Frame rate pre change */
    IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_POSTCHANGE, /*!< Frame rate post change */
    IARM_BUS_DSMGR_EVENT_AUDIO_PORT_STATE, /*!< Audio Port Init State */
    IARM_BUS_DSMGR_EVENT_SLEEP_MODE_CHANGED, /*!< Sleep Mode Change Event*/
    IARM_BUS_DSMGR_EVENT_HDMI_IN_AVI_CONTENT_TYPE, /*<HDMI IN content type event */
    IARM_BUS_DSMGR_EVENT_MAX, /*!< Max Event  */
} IARM_Bus_DSMgr_EventId_t;


/* ############################# wifiSrvMgrIarmIf.h ################################# */

#define IARM_BUS_NM_SRV_MGR_NAME "NET_SRV_MGR"

#define SSID_SIZE 	33
#define BSSID_BUFF 	20
#define PASSPHRASE_BUFF 385
#define MAX_SSIDLIST_BUF (48*1024)
/*IARM Interface for wifiManager_2 */
#define IARM_BUS_WIFI_MGR_API_getAvailableSSIDs             "getAvailableSSIDs"             /**< Retrieve array of strings representing SSIDs */
#define IARM_BUS_WIFI_MGR_API_getAvailableSSIDsWithName     "getAvailableSSIDsWithName"     /**< Retrieve array of strings representing SSIDs info for a specific SSID and band */
#define IARM_BUS_WIFI_MGR_API_getAvailableSSIDsAsync        "getAvailableSSIDsAsync"        /**< Retrieve array of strings representing SSIDs */
#define IARM_BUS_WIFI_MGR_API_getAvailableSSIDsAsyncIncr    "getAvailableSSIDsAsyncIncr"    /**< Retrieve array of strings representing SSIDs in an incremental way */
#define IARM_BUS_WIFI_MGR_API_stopProgressiveWifiScanning   "stopProgressiveWifiScanning"   /**< Stop any in-progress wifi progressive scanning thread */
#define IARM_BUS_WIFI_MGR_API_getCurrentState               "getCurrentState"               /**< Retrieve current state */
#define IARM_BUS_WIFI_MGR_API_getConnectedSSID              "getConnectedSSID"              /**< Return properties of the currently connected SSID */
#define IARM_BUS_WIFI_MGR_API_cancelWPSPairing              "cancelWPSPairing"              /**< Cancel in-progress WPS */
#define IARM_BUS_WIFI_MGR_API_getPairedSSID                 "getPairedSSID"                 /**< Return paired SSID as a string */
#define IARM_BUS_WIFI_MGR_API_setEnabled                    "setEnabled"                    /**< Enable wifi adapter on the box */
#define IARM_BUS_WIFI_MGR_API_connect                       "connect"                       /**< Connect with given or saved SSID and passphrase */
#define IARM_BUS_WIFI_MGR_API_initiateWPSPairing            "initiateWPSPairing"            /**< Initiate connection via WPS */
#define IARM_BUS_WIFI_MGR_API_initiateWPSPairing2           "initiateWPSPairing2"           /**< Initiate connection via WPS via either Push Button or PIN */
#define IARM_BUS_WIFI_MGR_API_saveSSID                      "saveSSID"                      /**< Save SSID and passphrase */
#define IARM_BUS_WIFI_MGR_API_clearSSID                     "clearSSID"                     /**< Clear given SSID */
#define IARM_BUS_WIFI_MGR_API_disconnectSSID                "disconnectSSID"                /**< Disconnect from current SSID */
#define IARM_BUS_WIFI_MGR_API_getPairedSSID                 "getPairedSSID"                 /**< Get paired SSID */
#define IARM_BUS_WIFI_MGR_API_isPaired                      "isPaired"                      /**< Retrieve the paired status */
#define IARM_BUS_WIFI_MGR_API_getLNFState                   "getLNFState"                   /**< Retrieve the LNF state */
#define IARM_BUS_WIFI_MGR_API_isStopLNFWhileDisconnected    "isStopLNFWhileDisconnected"    /**< Check if LNF is stopped */
#define IARM_BUS_WIFI_MGR_API_getConnectionType             "getConnectionType"             /**< Get connection type the current state */
#define IARM_BUS_WIFI_MGR_API_getSwitchToPrivateResults     "getSwitchToPrivateResults"     /**< Get all switch to private results */
#define IARM_BUS_WIFI_MGR_API_isAutoSwitchToPrivateEnabled  "isAutoSwitchToPrivateEnabled"  /**< Inform whether switch to private is enabled */
#define IARM_BUS_WIFI_MGR_API_getPairedSSIDInfo             "getPairedSSIDInfo"             /**< Get last paired ssid info */


/*! Event states associated with WiFi connection  */
typedef enum _WiFiStatusCode_t {
    WIFI_UNINSTALLED,        /**< The device was in an installed state, and was uninstalled */
    WIFI_DISABLED,           /**< The device is installed (or was just installed) and has not yet been enabled */
    WIFI_DISCONNECTED,       /**< The device is not connected to a network */
    WIFI_PAIRING,            /**< The device is not connected to a network, but not yet connecting to a network */
    WIFI_CONNECTING,         /**< The device is attempting to connect to a network */
    WIFI_CONNECTED,          /**< The device is successfully connected to a network */
    WIFI_FAILED              /**< The device has encountered an unrecoverable error with the wifi adapter */
} WiFiStatusCode_t;

/*! Error code: A recoverable, unexpected error occurred,
 * as defined by one of the following values */
typedef enum _WiFiErrorCode_t {
    WIFI_SSID_CHANGED,              /**< The SSID of the network changed */
    WIFI_CONNECTION_LOST,           /**< The connection to the network was lost */
    WIFI_CONNECTION_FAILED,         /**< The connection failed for an unknown reason */
    WIFI_CONNECTION_INTERRUPTED,    /**< The connection was interrupted */
    WIFI_INVALID_CREDENTIALS,       /**< The connection failed due to invalid credentials */
    WIFI_NO_SSID,                   /**< The SSID does not exist */
    WIFI_UNKNOWN,                   /**< Any other error */
    WIFI_AUTH_FAILED                /**< The connection failed due to auth failure */
} WiFiErrorCode_t;

/*! Supported values are NONE - 0, WPA - 1, WEP - 2*/
typedef enum _SsidSecurity
{
    NET_WIFI_SECURITY_NONE = 0,
    NET_WIFI_SECURITY_WEP_64,
    NET_WIFI_SECURITY_WEP_128,
    NET_WIFI_SECURITY_WPA_PSK_TKIP,
    NET_WIFI_SECURITY_WPA_PSK_AES,
    NET_WIFI_SECURITY_WPA2_PSK_TKIP,
    NET_WIFI_SECURITY_WPA2_PSK_AES,
    NET_WIFI_SECURITY_WPA_ENTERPRISE_TKIP,
    NET_WIFI_SECURITY_WPA_ENTERPRISE_AES,
    NET_WIFI_SECURITY_WPA2_ENTERPRISE_TKIP,
    NET_WIFI_SECURITY_WPA2_ENTERPRISE_AES,
    NET_WIFI_SECURITY_WPA_WPA2_PSK,
    NET_WIFI_SECURITY_WPA_WPA2_ENTERPRISE,
    NET_WIFI_SECURITY_WPA3_PSK_AES,
    NET_WIFI_SECURITY_WPA3_SAE,
    NET_WIFI_SECURITY_NOT_SUPPORTED = 99,
} SsidSecurity;

typedef struct _WiFiConnection
{
    char ssid[SSID_SIZE];
    char passphrase[PASSPHRASE_BUFF];
    SsidSecurity security_mode;
} WiFiConnection;

typedef struct _WiFiConnectedSSIDInfo
{
    char ssid[SSID_SIZE];     /**< The name of connected SSID. */
    char bssid[BSSID_BUFF];   /**< The the Basic Service Set ID (mac address). */
    int securityMode;         /**< Current WiFi Security Mode used for connection. */
    int  frequency;                    /**< The Frequency wt which the client is connected to. */
    float rate;               /**< The Physical data rate in Mbps */
    float noise;              /**< The average noise strength in dBm. */
    float signalStrength;     /**< The RSSI value in dBm. */

} WiFiConnectedSSIDInfo_t;

typedef struct _WiFiPairedSSIDInfo
{
    char ssid[SSID_SIZE];      /**< The name of connected SSID. */
    char bssid[BSSID_BUFF];    /**< The the Basic Service Set ID (mac address). */
} WiFiPairedSSIDInfo_t;

/*! Get/Set Data associated with WiFi Service Manager */
typedef struct _IARM_Bus_WiFiSrvMgr_SsidList_Param_t {
    bool status;
} IARM_Bus_WiFiSrvMgr_SsidList_Param_t;

typedef struct _IARM_Bus_WiFiSrvMgr_Param_t {
    union {
        WiFiStatusCode_t wifiStatus;
        WiFiConnection connect;
        WiFiConnectedSSIDInfo_t getConnectedSSID;
        WiFiPairedSSIDInfo_t getPairedSSIDInfo;
        struct getPairedSSID {
            char ssid[SSID_SIZE];
        } getPairedSSID;
        bool isPaired;
    } data;
    bool status;
} IARM_Bus_WiFiSrvMgr_Param_t;

typedef struct _IARM_Bus_WiFiSrvMgr_WPS_Parameters_t
{
    bool pbc;
    char pin[9];
    bool status;
} IARM_Bus_WiFiSrvMgr_WPS_Parameters_t;

/*! Event Data associated with WiFi Service Manager */
typedef struct _IARM_BUS_WiFiSrvMgr_EventData_t {
    union {
        struct _WIFI_STATECHANGE_DATA {
            WiFiStatusCode_t state;
        } wifiStateChange;
        struct _WIFI_ERROR {
            WiFiErrorCode_t code;
        } wifiError;
        struct _WIFI_SSID_LIST {
            char ssid_list[MAX_SSIDLIST_BUF];
            bool more_data;
        } wifiSSIDList;
    } data;
} IARM_BUS_WiFiSrvMgr_EventData_t;

/*! Events published from WiFi Service Manager */
typedef enum _IARM_Bus_NMgr_WiFi_EventId_t {
    IARM_BUS_WIFI_MGR_EVENT_onWIFIStateChanged = 1,
    IARM_BUS_WIFI_MGR_EVENT_onError,
    IARM_BUS_WIFI_MGR_EVENT_onSSIDsChanged,
    IARM_BUS_WIFI_MGR_EVENT_onAvailableSSIDs,
    IARM_BUS_WIFI_MGR_EVENT_onAvailableSSIDsIncr,
    IARM_BUS_WIFI_MGR_EVENT_MAX,           		/*!< Maximum event id*/
} IARM_Bus_NMgr_WiFi_EventId_t;

/* ############################## Network Manager ####################### */
#define IARM_BUS_NM_SRV_MGR_NAME "NET_SRV_MGR"
#define INTERFACE_SIZE 10
#define INTERFACE_LIST 50
#define MAX_IP_ADDRESS_LEN 46
#define MAX_IP_FAMILY_SIZE 10
#define MAX_HOST_NAME_LEN 128
#define IARM_BUS_NETSRVMGR_API_getActiveInterface "getActiveInterface"
#define IARM_BUS_NETSRVMGR_API_getNetworkInterfaces "getNetworkInterfaces"
#define IARM_BUS_NETSRVMGR_API_getInterfaceList "getInterfaceList"
#define IARM_BUS_NETSRVMGR_API_getDefaultInterface "getDefaultInterface"
#define IARM_BUS_NETSRVMGR_API_setDefaultInterface "setDefaultInterface"
#define IARM_BUS_NETSRVMGR_API_isInterfaceEnabled "isInterfaceEnabled"
#define IARM_BUS_NETSRVMGR_API_setInterfaceEnabled "setInterfaceEnabled"
#define IARM_BUS_NETSRVMGR_API_getSTBip "getSTBip"
#define IARM_BUS_NETSRVMGR_API_setIPSettings "setIPSettings"
#define IARM_BUS_NETSRVMGR_API_getIPSettings "getIPSettings"
#define IARM_BUS_NETSRVMGR_API_getSTBip_family "getSTBip_family"
#define IARM_BUS_NETSRVMGR_API_isConnectedToInternet "isConnectedToInternet"
#define IARM_BUS_NETSRVMGR_API_setConnectivityTestEndpoints "setConnectivityTestEndpoints"
#define IARM_BUS_NETSRVMGR_API_getInternetConnectionState "getInternetConnectionState"
#define IARM_BUS_NETSRVMGR_API_monitorConnectivity "monitorConnectivity"
#define IARM_BUS_NETSRVMGR_API_stopConnectivityMonitoring "stopConnectivityMonitoring"
#define IARM_BUS_NETSRVMGR_API_isAvailable "isAvailable"
#define IARM_BUS_NETSRVMGR_API_getPublicIP "getPublicIP"

// TODO: remove this
#define registerMethod(...) for (uint8_t i = 1; GetHandler(i); i++) GetHandler(i)->Register<JsonObject, JsonObject>(__VA_ARGS__)

/* Netsrvmgr Based Macros & Structures */
typedef struct _IARM_BUS_NetSrvMgr_Iface_EventData_t {
   union {
        char activeIface[INTERFACE_SIZE];
        char allNetworkInterfaces[INTERFACE_LIST];
        char setInterface[INTERFACE_SIZE];
	char activeIfaceIpaddr[MAX_IP_ADDRESS_LEN];
	};
   char interfaceCount;
   bool isInterfaceEnabled;
   bool persist;
   char ipfamily[MAX_IP_FAMILY_SIZE];
} IARM_BUS_NetSrvMgr_Iface_EventData_t;

#define IARM_BUS_SYSMGR_API_RunScript "RunScript"
/*! Parameter for RunScript call*/
typedef struct _IARM_Bus_SYSMgr_RunScript_t{
    char script_path [256];   //[in]  Null terminated path name of the script.
    int  return_value;        //[out] Returns the ret value of system.
} IARM_Bus_SYSMgr_RunScript_t;

typedef enum _CECMgr_EventId_t {
    IARM_BUS_CECMGR_EVENT_SEND,
    IARM_BUS_CECMGR_EVENT_RECV,
    IARM_BUS_CECMGR_EVENT_ENABLE,
    IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED,
    IARM_BUS_CECMGR_EVENT_MAX,
    IARM_BUS_CECMGR_EVENT_STATUS_UPDATED
} IARM_Bus_CECMgr_EventId_t;
typedef struct _IARM_Bus_CECMgr_Status_Updated_Param_t
{
    int logicalAddress;
}IARM_Bus_CECMgr_Status_Updated_Param_t;
#define IARM_BUS_CECMGR_API_isAvailable "isAvailable"
#define IARM_BUS_DSMGR_API_dsHdmiInGetNumberOfInputs    "dsHdmiInGetNumberOfInputs"
#define IARM_BUS_DSMGR_API_dsHdmiInGetStatus            "dsHdmiInGetStatus"
#define IARM_BUS_DSMGR_API_dsGetHDMIARCPortId  "dsGetHDMIARCPortId"
/* irMgr */
#define IARM_BUS_IRMGR_NAME 						"IRMgr" /*!< IR Manager IARM -bus name */

/*! Events published from IR Mananger */
typedef enum _IRMgr_EventId_t {
    IARM_BUS_IRMGR_EVENT_IRKEY,    /*!< Key Event  */
    IARM_BUS_IRMGR_EVENT_CONTROL,
    IARM_BUS_IRMGR_EVENT_MAX,      /*!< Maximum event id*/
} IARM_Bus_IRMgr_EventId_t;

/*! Possible IR Manager Key Source */
typedef enum _IRMgr_KeySrc_t {
	IARM_BUS_IRMGR_KEYSRC_FP,	/*!< Key Source FP */
	IARM_BUS_IRMGR_KEYSRC_IR,	/*!< Key Source IR */
	IARM_BUS_IRMGR_KEYSRC_RF,	/*!< Key Source RF */
} IARM_Bus_IRMgr_KeySrc_t;

/*! Key Event Data */
typedef struct _IRMgr_EventData_t {
    union {
        struct _IRKEY_DATA{
            /* Declare Event Data structure for IRMGR_EVENT_DUMMY0 */
            int keyType;                    /*!< Key type (UP/DOWN/REPEAT) */
            int keyCode;                    /*!< Key code */
            int keyTag;                     /*!< key tag (identifies which remote) */
            int keyOwner;                   /*!< key owner (normal or pairing op) */
            int isFP;                       /*!< Key Source -- Remote/FP - TBD - Need to be removed after changing MPEOS*/
            IARM_Bus_IRMgr_KeySrc_t keySrc; /*!< Key Source -- FP,IR,Remote */
            unsigned int keySourceId;
        } irkey, fpkey;
    } data;
}IARM_Bus_IRMgr_EventData_t;

/*
 * Declare RPC API names and their arguments
 */
#define IARM_BUS_IRMGR_API_SetRepeatInterval 		"SetRepeatInterval" /*!< Set the repeat key event interval*/

/*! Declare Argument Data structure for SetRepeatInterval */
typedef struct _IARM_Bus_IRMgr_SetRepeatInterval_Param_t {
    unsigned int timeout;           /*!< Repeat key interval timeout*/
} IARM_Bus_IRMgr_SetRepeatInterval_Param_t;

#define IARM_BUS_IRMGR_API_GetRepeatInterval 		"GetRepeatInterval" /*!< Retrives the current repeat interval*/

/*! Declare Argument Data structure for GetRepeatInterval */
typedef struct _IARM_Bus_IRMgr_GetRepeatInterval_Param_t {
	unsigned int timeout;          /*!< Repeat key interval timeout value retrived*/
} IARM_Bus_IRMgr_GetRepeatInterval_Param_t;

/* comcastIrKeyCodes */
#define DELAY_REPEAT_MASK       0x00000001
#define LANGUAGE_MASK           0x00000010
#define MISSED_KEY_TIMEOUT_MASK 0x00000100
#define REPEAT_KEY_ENABLED_MASK 0x00001000
#define REPEAT_FREQUENCY_MASK   0x00010000
#define REPORT_MODIFIERS_MASK   0x00100000
#define NUM_OF_DEVICES_MASK     0x01000000


#define KET_KEYDOWN 		0x00008000UL
#define KET_KEYUP   		0x00008100UL
#define KET_KEYREPEAT		0x00008200UL

//Numeric keys (common in Remote and Key Board)
//Combination of Key + device type (HID_DEVICES) is unique.
#define KED_DIGIT0		    0x00000030UL
#define KED_DIGIT1		    0x00000031UL
#define KED_DIGIT2		    0x00000032UL
#define KED_DIGIT3		    0x00000033UL
#define KED_DIGIT4		    0x00000034UL
#define KED_DIGIT5		    0x00000035UL
#define KED_DIGIT6		    0x00000036UL
#define KED_DIGIT7		    0x00000037UL
#define KED_DIGIT8		    0x00000038UL
#define KED_DIGIT9		    0x00000039UL
#define KED_PERIOD		    0x00000040UL

#define KED_DISCRETE_POWER_ON       0x00000050UL
#define KED_DISCRETE_POWER_STANDBY  0x00000051UL

#define KED_SEARCH          0x000000CFUL
#define KED_SETUP           0x00000052UL

#define KED_CLOSED_CAPTIONING       0x00000060UL
#define KED_LANGUAGE                0x00000061UL
#define KED_VOICE_GUIDANCE          0x00000062UL
#define KED_HEARTBEAT               0x00000063UL
#define KED_PUSH_TO_TALK            0x00000064UL
#define KED_DESCRIPTIVE_AUDIO       0x00000065UL
#define KED_VOLUME_OPTIMIZE         0x00000066UL
#define KED_XR2V3                   0x00000067UL
#define KED_XR5V2                   0x00000068UL
#define KED_XR11V1                  0x00000069UL
#define KED_XR11V2                  0x0000006AUL
#define KED_XR13                    0x0000006BUL
#define KED_XR11_NOTIFY             0x0000006CUL

#define KED_XR15V1_NOTIFY           0x00000070UL
#define KED_XR15V1_SELECT           0x00000071UL
#define KED_XR15V1_PUSH_TO_TALK     0x00000072UL

#define KED_SCREEN_BIND_NOTIFY      0x00000073UL

#define KED_XR16V1_NOTIFY           0x00000074UL
#define KED_XR16V1_SELECT           0x00000075UL
#define KED_XR16V1_PUSH_TO_TALK     0x00000076UL

#define KED_RF_POWER        0x0000007FUL
#define KED_POWER           0x00000080UL
#define KED_FP_POWER        KED_POWER
#define KED_ARROWUP         0x00000081UL
#define KED_ARROWDOWN       0x00000082UL
#define KED_ARROWLEFT       0x00000083UL
#define KED_ARROWRIGHT      0x00000084UL
#define KED_SELECT          0x00000085UL
#define KED_ENTER           0x00000086UL
#define KED_EXIT            0x00000087UL
#define KED_CHANNELUP       0x00000088UL
#define KED_CHANNELDOWN     0x00000089UL
#define KED_VOLUMEUP        0x0000008AUL
#define KED_VOLUMEDOWN      0x0000008BUL
#define KED_MUTE            0x0000008CUL
#define KED_GUIDE           0x0000008DUL
#define KED_VIEWINGGUIDE    KED_GUIDE
#define KED_INFO            0x0000008EUL
#define KED_SETTINGS        0x0000008FUL
#define KED_PAGEUP          0x00000090UL
#define KED_PAGEDOWN        0x00000091UL
#define KED_KEYA            0x00000092UL
#define KED_KEYB            0x00000093UL
#define KED_KEYC            0x00000094UL
#define KED_KEYD            0x0000009FUL
#define KED_KEY_RED_CIRCLE      KED_KEYC //Host2.1 Sec 7.2
#define KED_KEY_GREEN_DIAMOND   KED_KEYD //Host2.1 Sec 7.2
#define KED_KEY_BLUE_SQUARE     KED_KEYB //Host2.1 Sec 7.2
#define KED_KEY_YELLOW_TRIANGLE KED_KEYA //Host2.1 Sec 7.2
#define KED_LAST            0x00000095UL
#define KED_FAVORITE        0x00000096UL
#define KED_REWIND          0x00000097UL
#define KED_FASTFORWARD     0x00000098UL
#define KED_PLAY            0x00000099UL
#define KED_STOP            0x0000009AUL
#define KED_PAUSE           0x0000009BUL
#define KED_RECORD          0x0000009CUL
#define KED_BYPASS          0x0000009DUL
#define KED_TVVCR           0x0000009EUL

#define KED_REPLAY               0x000000A0UL
#define KED_HELP                 0x000000A1UL
#define KED_RECALL_FAVORITE_0    0x000000A2UL
#define KED_CLEAR                0x000000A3UL
#define KED_DELETE               0x000000A4UL
#define KED_START                0x000000A5UL
#define KED_POUND                0x000000A6UL
#define KED_FRONTPANEL1          0x000000A7UL
#define KED_FRONTPANEL2          0x000000A8UL
#define KED_OK                   0x000000A9UL
#define KED_STAR                 0x000000AAUL
#define KED_PROGRAM              0x000000ABUL

#define KED_TVPOWER                0x000000C1UL	// Alt remote
#define KED_PREVIOUS               0x000000C3UL	// Alt remote
#define KED_NEXT                   0x000000C4UL	// Alt remote
#define KED_MENU                   0x000000C0UL	// Alt remote
#define KED_INPUTKEY               0x000000D0UL	//
#define KED_LIVE                   0x000000D1UL	//
#define KED_MYDVR                  0x000000D2UL	//
#define KED_ONDEMAND               0x000000D3UL	//
#define KED_STB_MENU               0x000000D4UL    //
#define KED_AUDIO                  0x000000D5UL    //
#define KED_FACTORY                0x000000D6UL    //
#define KED_RFENABLE               0x000000D7UL    //
#define KED_LIST                   0x000000D8UL
#define KED_RF_PAIR_GHOST          0x000000EFUL	// Ghost code to implement auto pairing in RF remotes
#define KED_WPS                    0x000000F0UL    // Key to initiate WiFi WPS pairing
#define KED_DEEPSLEEP_WAKEUP       0x000000F1UL    // Key to initiate deepsleep wakeup
#define KED_NEW_BATTERIES_INSERTED 0x000000F2UL // Signals a battery set was replaced
#define KED_GRACEFUL_SHUTDOWN      0x000000F3UL // Signals an external power supply device is shutting down
#define KED_UNDEFINEDKEY           0x000000FEUL	// Use for keys not defined here.  Pass raw code as well.


#define KED_BACK            0x100000FEUL
#define KED_DISPLAY_SWAP    0x300000FEUL
#define KED_PINP_MOVE       0x400000FEUL
#define KED_PINP_TOGGLE     0x500000FEUL
#define KED_PINP_CHDOWN     0x600000FEUL
#define KED_PINP_CHUP       0x700000FEUL
#define KED_DMC_ACTIVATE    0x800000FEUL
#define KED_DMC_DEACTIVATE  0x900000FEUL
#define KED_DMC_QUERY       0xA00000FEUL
#define KED_OTR_START       0xB00000FEUL
#define KED_OTR_STOP        0xC00000FEUL
