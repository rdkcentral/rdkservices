#pragma once

#include <gmock/gmock.h>

#include "devicesettings.h"

class LibCCECImplMock : public LibCCECImpl {
public:
    virtual ~LibCCECImplMock() = default;

    MOCK_METHOD(void, init, (const char *name), (const, override));
    MOCK_METHOD(void, init, (), (const, override));
    MOCK_METHOD(void, term, (), (const, override));
    MOCK_METHOD(void, getPhysicalAddress, (unsigned int *physicalAddress), (const, override));
    MOCK_METHOD(int, getLogicalAddress, (int devType), (const, override));
    MOCK_METHOD(int, addLogicalAddress, (const LogicalAddress &source), (const, override));

};

