#pragma once

#include <gmock/gmock.h>

class floatingRtFunctionsMock : public floatingRtFunctionsImpl {
public:
    virtual ~floatingRtFunctionsMock() = default;

    MOCK_METHOD(rtError, rtRemoteLocateObject, (rtRemoteEnvironment *env, const char* str, rtObjectRef& obj, int x, remoteDisconnectCallback back, void *cbdata), (override));


};
