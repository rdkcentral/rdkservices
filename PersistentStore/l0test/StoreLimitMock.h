#pragma once

#include <gmock/gmock.h>
#include <interfaces/IStore2.h>

class StoreLimitMock : public WPEFramework::Exchange::IStoreLimit {
public:
    ~StoreLimitMock() override = default;
    MOCK_METHOD(uint32_t, GetNamespaceStorageLimit, (const ScopeType scope, const string& ns, uint32_t& size), (override));
    MOCK_METHOD(uint32_t, SetNamespaceStorageLimit, (const ScopeType scope, const string& ns, const uint32_t size), (override));
    BEGIN_INTERFACE_MAP(StoreLimitMock)
    INTERFACE_ENTRY(IStoreLimit)
    END_INTERFACE_MAP
};
