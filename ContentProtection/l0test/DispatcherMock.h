#pragma once

#include "../Module.h"
#include <gmock/gmock.h>

class DispatcherMock : public WPEFramework::PluginHost::IDispatcher {
public:
    ~DispatcherMock() override = default;
    MOCK_METHOD(uint32_t, Invoke, (const uint32_t channelid, const uint32_t id, const string& token, const string& method, const string& parameters, string& response), (override));
    MOCK_METHOD(WPEFramework::Core::hresult, Subscribe, (ICallback * callback, const string& event, const string& designator), (override));
    MOCK_METHOD(WPEFramework::Core::hresult, Unsubscribe, (ICallback * callback, const string& event, const string& designator), (override));
    MOCK_METHOD(WPEFramework::Core::hresult, Attach, (WPEFramework::PluginHost::IShell::IConnectionServer::INotification * &sink, WPEFramework::PluginHost::IShell* service), (override));
    MOCK_METHOD(WPEFramework::Core::hresult, Detach, (WPEFramework::PluginHost::IShell::IConnectionServer::INotification * &sink), (override));
    MOCK_METHOD(void, Dropped, (const ICallback* callback), (override));
    BEGIN_INTERFACE_MAP(DispatcherMock)
    INTERFACE_ENTRY(IDispatcher)
    END_INTERFACE_MAP
};
