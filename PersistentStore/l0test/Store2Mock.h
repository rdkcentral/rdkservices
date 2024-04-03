#pragma once

#include <gmock/gmock.h>
#include <interfaces/IStore2.h>

class Store2Mock : public WPEFramework::Exchange::IStore2 {
public:
    ~Store2Mock() override = default;
    MOCK_METHOD(uint32_t, Register, (INotification*), (override));
    MOCK_METHOD(uint32_t, Unregister, (INotification*), (override));
    MOCK_METHOD(uint32_t, SetValue, (const ScopeType scope, const string& ns, const string& key, const string& value, const uint32_t ttl), (override));
    MOCK_METHOD(uint32_t, GetValue, (const ScopeType scope, const string& ns, const string& key, string& value, uint32_t& ttl), (override));
    MOCK_METHOD(uint32_t, DeleteKey, (const ScopeType scope, const string& ns, const string& key), (override));
    MOCK_METHOD(uint32_t, DeleteNamespace, (const ScopeType scope, const string& ns), (override));
    BEGIN_INTERFACE_MAP(Store2Mock)
    INTERFACE_ENTRY(IStore2)
    END_INTERFACE_MAP
};
