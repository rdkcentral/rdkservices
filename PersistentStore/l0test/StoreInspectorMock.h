#pragma once

#include <gmock/gmock.h>
#include <interfaces/IStore2.h>

class StoreInspectorMock : public WPEFramework::Exchange::IStoreInspector {
public:
    ~StoreInspectorMock() override = default;
    MOCK_METHOD(uint32_t, GetKeys, (const ScopeType scope, const string& ns, IStringIterator*& keys), (override));
    MOCK_METHOD(uint32_t, GetNamespaces, (const ScopeType scope, IStringIterator*& namespaces), (override));
    MOCK_METHOD(uint32_t, GetStorageSizes, (const ScopeType scope, INamespaceSizeIterator*& storageList), (override));
    BEGIN_INTERFACE_MAP(StoreInspectorMock)
    INTERFACE_ENTRY(IStoreInspector)
    END_INTERFACE_MAP
};
