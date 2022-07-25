#pragma once

#include <gmock/gmock.h>

#include "rfcapi.h"

class RfcApiImplMock : public RfcApiImpl {
public:
    virtual ~RfcApiImplMock() = default;

    MOCK_METHOD(WDMP_STATUS, getRFCParameter, (char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData), (override));
    MOCK_METHOD(WDMP_STATUS, setRFCParameter, (char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType), (override));
};
