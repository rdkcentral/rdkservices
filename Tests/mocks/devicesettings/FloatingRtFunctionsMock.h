#pragma once

#include <gmock/gmock.h>

class floatingRtFunctionsMock : public floatingRtFunctionsImpl {
public:
    virtual ~floatingRtFunctionsMock() = default;

    MOCK_METHOD(rtError, rtRemoteLocateObject, (rtRemoteEnvironment *env, const char* str, rtObjectRef& obj, int x, remoteDisconnectCallback back, void *cbdata), (override));
    MOCK_METHOD(rtRemoteEnvironment*, rtEnvironmentGetGlobal, (), (override));
    MOCK_METHOD(rtError,rtRemoteShutdown, (rtRemoteEnvironment *env), (override));
    MOCK_METHOD(rtError,rtRemoteInit, (rtRemoteEnvironment *env), (override));
    MOCK_METHOD(rtError,rtRemoteProcessSingleItem, (), (override));
    MOCK_METHOD(char*,  rtStrError, (rtError err), (override));


};
