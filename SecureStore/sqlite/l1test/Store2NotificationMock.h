#pragma once

#include <gmock/gmock.h>
#include <interfaces/IStore2.h>

class Store2NotificationMock : public WPEFramework::Exchange::IStore2::INotification {
public:
    ~Store2NotificationMock() override = default;
    MOCK_METHOD(void, ValueChanged, (const WPEFramework::Exchange::IStore2::ScopeType scope, const string& ns, const string& key, const string& value), (override));
    BEGIN_INTERFACE_MAP(Store2NotificationMock)
    INTERFACE_ENTRY(INotification)
    END_INTERFACE_MAP
};
