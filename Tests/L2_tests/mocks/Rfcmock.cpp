
#include "Rfc.h"


WDMP_STATUS getRFCParameter(char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData)
{
     printf("SUMA: inside mock getRFCparameter");
     pstParamData->type = WDMP_BOOLEAN;
     sprintf(pstParamData->value,"true");
     return WDMP_SUCCESS;
}
WDMP_STATUS setRFCParameter(char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType)
{
     return WDMP_SUCCESS;
}

const char* getRFCErrorString(WDMP_STATUS code)
{
   return "error";
}
