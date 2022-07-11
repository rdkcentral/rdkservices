#pragma once

#include <gmock/gmock.h>

#include "rfcapi.h"

class RfcApiImplMock : public RfcApiImpl {
public:
    virtual ~RfcApiImplMock() = default;

    MOCK_METHOD(WDMP_STATUS, setRFCParameter, (char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType), (override));
};
