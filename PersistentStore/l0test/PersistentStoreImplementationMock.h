#pragma once

#include <gmock/gmock.h>
#include <interfaces/IStore.h>
#include <interfaces/IStore2.h>
#include <interfaces/IStoreCache.h>

class PersistentStoreImplementationMock
    : public WPEFramework::Exchange::IStore,
      public WPEFramework::Exchange::IStore2,
      public WPEFramework::Exchange::IStoreCache,
      public WPEFramework::Exchange::IStoreInspector,
      public WPEFramework::Exchange::IStoreLimit {
public:
    ~PersistentStoreImplementationMock() override = default;
    MOCK_METHOD(uint32_t, Register, (IStore::INotification * notification), (override));
    MOCK_METHOD(uint32_t, Unregister, (IStore::INotification * notification), (override));
    MOCK_METHOD(uint32_t, SetValue, (const string& ns, const string& key, const string& value), (override));
    MOCK_METHOD(uint32_t, GetValue, (const string& ns, const string& key, string& value), (override));
    MOCK_METHOD(uint32_t, DeleteKey, (const string& ns, const string& key), (override));
    MOCK_METHOD(uint32_t, DeleteNamespace, (const string& ns), (override));
    MOCK_METHOD(uint32_t, Register, (IStore2::INotification*), (override));
    MOCK_METHOD(uint32_t, Unregister, (IStore2::INotification*), (override));
    MOCK_METHOD(uint32_t, SetValue, (const IStore2::ScopeType scope, const string& ns, const string& key, const string& value, const uint32_t ttl), (override));
    MOCK_METHOD(uint32_t, GetValue, (const IStore2::ScopeType scope, const string& ns, const string& key, string& value, uint32_t& ttl), (override));
    MOCK_METHOD(uint32_t, DeleteKey, (const IStore2::ScopeType scope, const string& ns, const string& key), (override));
    MOCK_METHOD(uint32_t, DeleteNamespace, (const IStore2::ScopeType scope, const string& ns), (override));
    MOCK_METHOD(uint32_t, FlushCache, (), (override));
    MOCK_METHOD(uint32_t, GetKeys, (const IStoreInspector::ScopeType scope, const string& ns, IStringIterator*& keys), (override));
    MOCK_METHOD(uint32_t, GetNamespaces, (const IStoreInspector::ScopeType scope, IStringIterator*& namespaces), (override));
    MOCK_METHOD(uint32_t, GetStorageSizes, (const IStoreInspector::ScopeType scope, INamespaceSizeIterator*& storageList), (override));
    MOCK_METHOD(uint32_t, GetNamespaceStorageLimit, (const IStoreLimit::ScopeType scope, const string& ns, uint32_t& size), (override));
    MOCK_METHOD(uint32_t, SetNamespaceStorageLimit, (const IStoreLimit::ScopeType scope, const string& ns, const uint32_t size), (override));
    BEGIN_INTERFACE_MAP(PersistentStoreImplementationMock)
    INTERFACE_ENTRY(IStore)
    INTERFACE_ENTRY(IStore2)
    INTERFACE_ENTRY(IStoreCache)
    INTERFACE_ENTRY(IStoreInspector)
    INTERFACE_ENTRY(IStoreLimit)
    END_INTERFACE_MAP
};
