#ifndef RFCAPI_H_

#define RFCAPI_H_

#include <string.h>

#include <stdlib.h>

#define RFCVAR_FILE "/opt/secure/RFC/rfcVariable.ini"

#define TR181STORE_FILE "/opt/secure/RFC/tr181store.ini"
#define MAX_PARAM_LEN     (2*1024)

typedef enum
{
        WDMP_SUCCESS = 0,
        WDMP_ERR_DEFAULT_VALUE
}WDMP_STATUS;

typedef enum
{
    WDMP_STRING = 0,
    WDMP_INT,
    WDMP_UINT,
    WDMP_BOOLEAN,
    WDMP_DATETIME,
    WDMP_BASE64,
    WDMP_LONG,
    WDMP_ULONG,
    WDMP_FLOAT,
    WDMP_DOUBLE,
    WDMP_BYTE,
    WDMP_NONE,
    WDMP_BLOB
} DATA_TYPE;


WDMP_STATUS setRFCParameter(char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType);

const char* getRFCErrorString(WDMP_STATUS code);

typedef struct _RFC_Param_t {

   char name[MAX_PARAM_LEN];

   char value[MAX_PARAM_LEN];

   DATA_TYPE type;

} RFC_ParamData_t;

WDMP_STATUS getRFCParameter(char *pcCallerID, const char* pcParameterName, RFC_ParamData_t *pstParamData);

#endif
