#pragma once

typedef enum
{
    WDMP_SUCCESS = 0,                    /**< Success. */
    WDMP_FAILURE,                        /**< General Failure */
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

class RfcApiImpl {
public:
    virtual ~RfcApiImpl() = default;

    virtual WDMP_STATUS setRFCParameter(char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) = 0;
};

class RfcApi {
public:
    static RfcApi& getInstance()
    {
        static RfcApi instance;
        return instance;
    }

    RfcApiImpl* impl;
    
    static WDMP_STATUS setRFCParameter(char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType)
    {
        return getInstance().impl->setRFCParameter(pcCallerID, pcParameterName, pcParameterValue, eDataType);
    }
};

constexpr auto setRFCParameter = &RfcApi::setRFCParameter;
