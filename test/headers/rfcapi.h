#include <string.h>
#include <stdlib.h>

#define RFCVAR_FILE "/opt/secure/RFC/rfcVariable.ini"
#define TR181STORE_FILE "/opt/secure/RFC/tr181store.ini"

#ifdef __cplusplus
extern "C"
{
#endif
#ifndef RDKC
#include <wdmp-c.h>
#endif

#define MAX_PARAM_LEN     (2*1024)

#ifdef RDKC
typedef enum
{
  SUCCESS=0,
  FAILURE,
  NONE,
  EMPTY
}DATATYPE;
#endif

#ifdef RDKC
typedef struct _RFC_Param_t {
   char name[MAX_PARAM_LEN];
   char value[MAX_PARAM_LEN];
   DATATYPE type;
} RFC_ParamData_t;
#else
typedef struct _RFC_Param_t {
   char name[MAX_PARAM_LEN];
   char value[MAX_PARAM_LEN];
   DATA_TYPE type;
} RFC_ParamData_t;
#endif
#ifdef RDKC
int getRFCParameter(const char* pcParameterName, RFC_ParamData_t *pstParamData);
#else
WDMP_STATUS getRFCParameter(char *pcCallerID, const char* pcParameterName, RFC_ParamData_t *pstParamData);
WDMP_STATUS setRFCParameter(char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType);
const char* getRFCErrorString(WDMP_STATUS code);
#endif
#ifdef __cplusplus
}
#endif

#endif
