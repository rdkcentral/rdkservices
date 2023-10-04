/* If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2023 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
*/

#pragma once

typedef enum {
    WDMP_SUCCESS = 0, /**< Success. */
    WDMP_FAILURE, /**< General Failure */
    WDMP_ERR_TIMEOUT,
    WDMP_ERR_NOT_EXIST,
    WDMP_ERR_INVALID_PARAMETER_NAME,
    WDMP_ERR_INVALID_PARAMETER_TYPE,
    WDMP_ERR_INVALID_PARAMETER_VALUE,
    WDMP_ERR_NOT_WRITABLE,
    WDMP_ERR_SETATTRIBUTE_REJECTED,
    WDMP_ERR_REQUEST_REJECTED,
    WDMP_ERR_NAMESPACE_OVERLAP,
    WDMP_ERR_UNKNOWN_COMPONENT,
    WDMP_ERR_NAMESPACE_MISMATCH,
    WDMP_ERR_UNSUPPORTED_NAMESPACE,
    WDMP_ERR_DP_COMPONENT_VERSION_MISMATCH,
    WDMP_ERR_INVALID_PARAM,
    WDMP_ERR_UNSUPPORTED_DATATYPE,
    WDMP_STATUS_RESOURCES,
    WDMP_ERR_WIFI_BUSY,
    WDMP_ERR_INVALID_ATTRIBUTES,
    WDMP_ERR_WILDCARD_NOT_SUPPORTED,
    WDMP_ERR_SET_OF_CMC_OR_CID_NOT_SUPPORTED,
    WDMP_ERR_VALUE_IS_EMPTY,
    WDMP_ERR_VALUE_IS_NULL,
    WDMP_ERR_DATATYPE_IS_NULL,
    WDMP_ERR_CMC_TEST_FAILED,
    WDMP_ERR_NEW_CID_IS_MISSING,
    WDMP_ERR_CID_TEST_FAILED,
    WDMP_ERR_SETTING_CMC_OR_CID,
    WDMP_ERR_INVALID_INPUT_PARAMETER,
    WDMP_ERR_ATTRIBUTES_IS_NULL,
    WDMP_ERR_NOTIFY_IS_NULL,
    WDMP_ERR_INVALID_WIFI_INDEX,
    WDMP_ERR_INVALID_RADIO_INDEX,
    WDMP_ERR_ATOMIC_GET_SET_FAILED,
    WDMP_ERR_METHOD_NOT_SUPPORTED,
    WDMP_ERR_SESSION_IN_PROGRESS,
    WDMP_ERR_INTERNAL_ERROR,
    WDMP_ERR_DEFAULT_VALUE
} WDMP_STATUS;

typedef enum {
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

#define MAX_PARAM_LEN (2 * 1024)

typedef struct _RFC_Param_t {
    char name[MAX_PARAM_LEN];
    char value[MAX_PARAM_LEN];
    DATA_TYPE type;
} RFC_ParamData_t;
#if 0
class RfcApiImpl {
public:
    virtual ~RfcApiImpl() = default;

    virtual WDMP_STATUS getRFCParameter(char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) = 0;
    virtual WDMP_STATUS setRFCParameter(char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) = 0;
    virtual const char* getRFCErrorString(WDMP_STATUS code) = 0;
};

class RfcApi {
public:
    static RfcApi& getInstance()
    {
        static RfcApi instance;
        return instance;
    }

    RfcApiImpl* impl;

    static WDMP_STATUS getRFCParameter(char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData)
    {
        return getInstance().impl->getRFCParameter(pcCallerID, pcParameterName, pstParamData);
    }

    static WDMP_STATUS setRFCParameter(char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType)
    {
        return getInstance().impl->setRFCParameter(pcCallerID, pcParameterName, pcParameterValue, eDataType);
    }

    static const char* getRFCErrorString(WDMP_STATUS code)
    {
        return getInstance().impl->getRFCErrorString(code);
    }
};

constexpr auto getRFCParameter = &RfcApi::getRFCParameter;
constexpr auto setRFCParameter = &RfcApi::setRFCParameter;
constexpr auto getRFCErrorString = &RfcApi::getRFCErrorString;
#endif
WDMP_STATUS getRFCParameter(char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData);
WDMP_STATUS setRFCParameter(char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType);
const char* getRFCErrorString(WDMP_STATUS code);
