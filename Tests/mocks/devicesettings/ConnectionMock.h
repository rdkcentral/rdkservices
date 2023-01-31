#pragma once

#include <gmock/gmock.h>

#include "devicesettings.h"

class ConnectionImplMock : public ConnectionImpl {
public:
    virtual ~ConnectionImplMock() = default;
    
    MOCK_METHOD(void, open, (), (const, override));
    MOCK_METHOD(void, close, (), (const, override));
    MOCK_METHOD(void, addFrameListener, (FrameListener *listener), (const, override));
    MOCK_METHOD(void, removeFrameListener, (FrameListener *listener), (const, override));
    MOCK_METHOD(void, sendAsync, (const CECFrame &frame), (const, override));
    MOCK_METHOD(void, ping, (const LogicalAddress &from, const LogicalAddress &to, const Throw_e &doThrow), (const, override));
    MOCK_METHOD(void, sendToAsync, (const LogicalAddress &to, const CECFrame &frame), (const, override));
    MOCK_METHOD(void, sendTo, (const LogicalAddress &to, const CECFrame &frame), (const, override));
    MOCK_METHOD(void, sendTo, (const LogicalAddress &to, const CECFrame &frame, int timeout), (const, override));
    MOCK_METHOD(void, poll, (const LogicalAddress &from, const Throw_e &doThrow), (const, override));

};

