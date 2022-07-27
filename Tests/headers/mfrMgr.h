#define IARM_BUS_MFRLIB_NAME "MFRLib"
#define IARM_BUS_MFRLIB_API_SetBootLoaderPattern "mfrSetBootloaderPattern"

typedef enum _mfrBlPattern_t
{
    mfrBL_PATTERN_NORMAL = 0,
    mfrBL_PATTERN_SILENT,
    mfrBL_PATTERN_SILENT_LED_ON
} mfrBlPattern_t;

typedef struct _IARM_Bus_MFRLib_SetBLPattern_Param_t{
    mfrBlPattern_t pattern;
} IARM_Bus_MFRLib_SetBLPattern_Param_t;
