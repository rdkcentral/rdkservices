#pragma once

#include "rfcapi.h"

namespace Utils {
inline bool getRFCConfig(char* paramName, RFC_ParamData_t& paramOutput)
{
    WDMP_STATUS wdmpStatus = getRFCParameter(nullptr, paramName, &paramOutput);
    if (wdmpStatus == WDMP_SUCCESS || wdmpStatus == WDMP_ERR_DEFAULT_VALUE) {
        return true;
    }
    return false;
}
}
