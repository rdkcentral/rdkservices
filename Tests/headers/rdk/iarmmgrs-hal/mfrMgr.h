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
    mfrSERIALIZED_TYPE_MAX,
} mfrSerializedType_t;

typedef enum _mfrBlPattern_t
{
    mfrBL_PATTERN_NORMAL = 0,
    mfrBL_PATTERN_SILENT,
    mfrBL_PATTERN_SILENT_LED_ON
} mfrBlPattern_t;

typedef struct _IARM_Bus_MFRLib_SetBLPattern_Param_t{
    mfrBlPattern_t pattern;
} IARM_Bus_MFRLib_SetBLPattern_Param_t;

typedef struct _IARM_Bus_MFRLib_GetSerializedData_Param_t {
    mfrSerializedType_t type;
    char buffer[1280];
    int bufLen;
} IARM_Bus_MFRLib_GetSerializedData_Param_t;
