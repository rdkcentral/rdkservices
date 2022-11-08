#pragma once

#include <gmock/gmock.h>

class RfcApiMock {
public:
    static RfcApiMock* instance{ nullptr };
    RfcApiMock()
    {
        instance = this;
    }
    virtual ~RfcApiMock()
    {
        instance = nullptr;
    }

    MOCK_METHOD(WDMP_STATUS, getRFCParameter, (char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData));
    MOCK_METHOD(WDMP_STATUS, setRFCParameter, (char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType));
    MOCK_METHOD(const char*, getRFCErrorString, (WDMP_STATUS code));

    static WDMP_STATUS _getRFCParameter(char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData)
    {
        return instance->getRFCParameter(pcCallerID, pcParameterName, pstParamData);
    }

    static WDMP_STATUS _setRFCParameter(char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType)
    {
        return instance->setRFCParameter(pcCallerID, pcParameterName, pcParameterValue, eDataType);
    }

    static const char* _getRFCErrorString(WDMP_STATUS code)
    {
        return instance->getRFCErrorString(code);
    }
};

constexpr auto getRFCParameter = &RfcApiMock::_getRFCParameter;
constexpr auto setRFCParameter = &RfcApiMock::_setRFCParameter;
constexpr auto getRFCErrorString = &RfcApiMock::_getRFCErrorString;
