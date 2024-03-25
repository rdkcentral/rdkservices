#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../PersistentStore.h"
#include "ServiceMock.h"
#include "Store2Mock.h"
#include "StoreCacheMock.h"
#include "StoreInspectorMock.h"
#include "StoreLimitMock.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::NiceMock;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::Test;
using ::WPEFramework::Core::PublishedServiceType;
using ::WPEFramework::Exchange::IStore;
using ::WPEFramework::Exchange::IStoreInspector;
using ::WPEFramework::JsonData::PersistentStore::DeleteKeyParamsInfo;
using ::WPEFramework::JsonData::PersistentStore::DeleteNamespaceParamsInfo;
using ::WPEFramework::JsonData::PersistentStore::GetKeysResultData;
using ::WPEFramework::JsonData::PersistentStore::GetNamespacesResultData;
using ::WPEFramework::JsonData::PersistentStore::GetNamespaceStorageLimitResultData;
using ::WPEFramework::JsonData::PersistentStore::GetStorageSizesResultData;
using ::WPEFramework::JsonData::PersistentStore::GetValueResultData;
using ::WPEFramework::JsonData::PersistentStore::SetNamespaceStorageLimitParamsData;
using ::WPEFramework::JsonData::PersistentStore::SetValueParamsData;
using ::WPEFramework::Plugin::PersistentStore;
using ::WPEFramework::PluginHost::ILocalDispatcher;
using ::WPEFramework::PluginHost::IPlugin;
using ::WPEFramework::RPC::IStringIterator;
using ::WPEFramework::RPC::IteratorType;
using ::WPEFramework::RPC::StringIterator;

const auto kValue = "value_1";
const auto kKey = "key_1";
const auto kAppId = "app_id_1";
const auto kTtl = 100;
const std::vector<string> kKeys{ "key_1", "key_2" };
const std::vector<string> kAppIds{ "app_id_1", "app_id_2" };
const std::vector<IStoreInspector::NamespaceSize> kSizes{ { "app_id_1", 10 }, { "app_id_2", 20 } };
const auto kSize = 1000;

class APersistentStore : public Test {
protected:
    NiceMock<ServiceMock>* service;
    IPlugin* plugin;
    APersistentStore()
        : service(WPEFramework::Core::Service<NiceMock<ServiceMock>>::Create<NiceMock<ServiceMock>>())
        , plugin(WPEFramework::Core::Service<PersistentStore>::Create<IPlugin>())
    {
    }
    ~APersistentStore() override
    {
        plugin->Release();
        service->Release();
    }
};

TEST_F(APersistentStore, GetsValueInDeviceScopeViaJsonRpc)
{
    class SqliteStore2 : public NiceMock<Store2Mock> {
    public:
        SqliteStore2()
        {
            EXPECT_CALL(*this, GetValue(_, _, _, _, _))
                .WillRepeatedly(Invoke(
                    [](const ScopeType scope, const string& ns, const string& key, string& value, uint32_t& ttl) {
                        EXPECT_THAT(scope, Eq(ScopeType::DEVICE));
                        EXPECT_THAT(ns, Eq(kAppId));
                        EXPECT_THAT(key, Eq(kKey));
                        value = kValue;
                        ttl = kTtl;
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<SqliteStore2> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto jsonRpc = plugin->QueryInterface<ILocalDispatcher>();
    ASSERT_THAT(jsonRpc, NotNull());
    DeleteKeyParamsInfo params;
    params.Namespace = kAppId;
    params.Key = kKey;
    string paramsJsonStr;
    params.ToString(paramsJsonStr);
    string resultJsonStr;
    ASSERT_THAT(jsonRpc->Invoke(0, 0, "", "getValue", paramsJsonStr, resultJsonStr), Eq(WPEFramework::Core::ERROR_NONE));
    GetValueResultData result;
    result.FromString(resultJsonStr);
    EXPECT_THAT(result.Value.Value(), Eq(kValue));
    EXPECT_THAT(result.Ttl.Value(), Eq(kTtl));
    jsonRpc->Release();
    plugin->Deinitialize(service);
}

TEST_F(APersistentStore, GetsValueInAccountScopeViaJsonRpc)
{
    class GrpcStore2 : public NiceMock<Store2Mock> {
    public:
        GrpcStore2()
        {
            EXPECT_CALL(*this, GetValue(_, _, _, _, _))
                .WillRepeatedly(Invoke(
                    [](const ScopeType scope, const string& ns, const string& key, string& value, uint32_t& ttl) {
                        EXPECT_THAT(scope, Eq(ScopeType::ACCOUNT));
                        EXPECT_THAT(ns, Eq(kAppId));
                        EXPECT_THAT(key, Eq(kKey));
                        value = kValue;
                        ttl = kTtl;
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<GrpcStore2> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto jsonRpc = plugin->QueryInterface<ILocalDispatcher>();
    ASSERT_THAT(jsonRpc, NotNull());
    DeleteKeyParamsInfo params;
    params.Scope = WPEFramework::JsonData::PersistentStore::ScopeType::ACCOUNT;
    params.Namespace = kAppId;
    params.Key = kKey;
    string paramsJsonStr;
    params.ToString(paramsJsonStr);
    string resultJsonStr;
    ASSERT_THAT(jsonRpc->Invoke(0, 0, "", "getValue", paramsJsonStr, resultJsonStr), Eq(WPEFramework::Core::ERROR_NONE));
    GetValueResultData result;
    result.FromString(resultJsonStr);
    EXPECT_THAT(result.Value.Value(), Eq(kValue));
    EXPECT_THAT(result.Ttl.Value(), Eq(kTtl));
    jsonRpc->Release();
    plugin->Deinitialize(service);
}

TEST_F(APersistentStore, SetsValueInDeviceScopeViaJsonRpc)
{
    class SqliteStore2 : public NiceMock<Store2Mock> {
    public:
        SqliteStore2()
        {
            EXPECT_CALL(*this, SetValue(_, _, _, _, _))
                .WillRepeatedly(Invoke(
                    [](const ScopeType scope, const string& ns, const string& key, const string& value, const uint32_t ttl) {
                        EXPECT_THAT(scope, Eq(ScopeType::DEVICE));
                        EXPECT_THAT(ns, Eq(kAppId));
                        EXPECT_THAT(key, Eq(kKey));
                        EXPECT_THAT(value, Eq(kValue));
                        EXPECT_THAT(ttl, Eq(kTtl));
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<SqliteStore2> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto jsonRpc = plugin->QueryInterface<ILocalDispatcher>();
    ASSERT_THAT(jsonRpc, NotNull());
    SetValueParamsData params;
    params.Namespace = kAppId;
    params.Key = kKey;
    params.Value = kValue;
    params.Ttl = kTtl;
    string paramsJsonStr;
    params.ToString(paramsJsonStr);
    string resultJsonStr;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "setValue", paramsJsonStr, resultJsonStr), Eq(WPEFramework::Core::ERROR_NONE));
    jsonRpc->Release();
    plugin->Deinitialize(service);
}

TEST_F(APersistentStore, SetsValueInAccountScopeViaJsonRpc)
{
    class GrpcStore2 : public NiceMock<Store2Mock> {
    public:
        GrpcStore2()
        {
            EXPECT_CALL(*this, SetValue(_, _, _, _, _))
                .WillRepeatedly(Invoke(
                    [](const ScopeType scope, const string& ns, const string& key, const string& value, const uint32_t ttl) {
                        EXPECT_THAT(scope, Eq(ScopeType::ACCOUNT));
                        EXPECT_THAT(ns, Eq(kAppId));
                        EXPECT_THAT(key, Eq(kKey));
                        EXPECT_THAT(value, Eq(kValue));
                        EXPECT_THAT(ttl, Eq(kTtl));
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<GrpcStore2> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto jsonRpc = plugin->QueryInterface<ILocalDispatcher>();
    ASSERT_THAT(jsonRpc, NotNull());
    SetValueParamsData params;
    params.Scope = WPEFramework::JsonData::PersistentStore::ScopeType::ACCOUNT;
    params.Namespace = kAppId;
    params.Key = kKey;
    params.Value = kValue;
    params.Ttl = kTtl;
    string paramsJsonStr;
    params.ToString(paramsJsonStr);
    string resultJsonStr;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "setValue", paramsJsonStr, resultJsonStr), Eq(WPEFramework::Core::ERROR_NONE));
    jsonRpc->Release();
    plugin->Deinitialize(service);
}

TEST_F(APersistentStore, DeletesKeyInDeviceScopeViaJsonRpc)
{
    class SqliteStore2 : public NiceMock<Store2Mock> {
    public:
        SqliteStore2()
        {
            EXPECT_CALL(*this, DeleteKey(_, _, _))
                .WillRepeatedly(Invoke(
                    [](const ScopeType scope, const string& ns, const string& key) {
                        EXPECT_THAT(scope, Eq(ScopeType::DEVICE));
                        EXPECT_THAT(ns, Eq(kAppId));
                        EXPECT_THAT(key, Eq(kKey));
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<SqliteStore2> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto jsonRpc = plugin->QueryInterface<ILocalDispatcher>();
    ASSERT_THAT(jsonRpc, NotNull());
    DeleteKeyParamsInfo params;
    params.Namespace = kAppId;
    params.Key = kKey;
    string paramsJsonStr;
    params.ToString(paramsJsonStr);
    string resultJsonStr;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "deleteKey", paramsJsonStr, resultJsonStr), Eq(WPEFramework::Core::ERROR_NONE));
    jsonRpc->Release();
    plugin->Deinitialize(service);
}

TEST_F(APersistentStore, DeletesKeyInAccountScopeViaJsonRpc)
{
    class GrpcStore2 : public NiceMock<Store2Mock> {
    public:
        GrpcStore2()
        {
            EXPECT_CALL(*this, DeleteKey(_, _, _))
                .WillRepeatedly(Invoke(
                    [](const ScopeType scope, const string& ns, const string& key) {
                        EXPECT_THAT(scope, Eq(ScopeType::ACCOUNT));
                        EXPECT_THAT(ns, Eq(kAppId));
                        EXPECT_THAT(key, Eq(kKey));
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<GrpcStore2> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto jsonRpc = plugin->QueryInterface<ILocalDispatcher>();
    ASSERT_THAT(jsonRpc, NotNull());
    DeleteKeyParamsInfo params;
    params.Scope = WPEFramework::JsonData::PersistentStore::ScopeType::ACCOUNT;
    params.Namespace = kAppId;
    params.Key = kKey;
    string paramsJsonStr;
    params.ToString(paramsJsonStr);
    string resultJsonStr;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "deleteKey", paramsJsonStr, resultJsonStr), Eq(WPEFramework::Core::ERROR_NONE));
    jsonRpc->Release();
    plugin->Deinitialize(service);
}

TEST_F(APersistentStore, DeletesNamespaceInDeviceScopeViaJsonRpc)
{
    class SqliteStore2 : public NiceMock<Store2Mock> {
    public:
        SqliteStore2()
        {
            EXPECT_CALL(*this, DeleteNamespace(_, _))
                .WillRepeatedly(Invoke(
                    [](const ScopeType scope, const string& ns) {
                        EXPECT_THAT(scope, Eq(ScopeType::DEVICE));
                        EXPECT_THAT(ns, Eq(kAppId));
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<SqliteStore2> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto jsonRpc = plugin->QueryInterface<ILocalDispatcher>();
    ASSERT_THAT(jsonRpc, NotNull());
    DeleteNamespaceParamsInfo params;
    params.Namespace = kAppId;
    string paramsJsonStr;
    params.ToString(paramsJsonStr);
    string resultJsonStr;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "deleteNamespace", paramsJsonStr, resultJsonStr), Eq(WPEFramework::Core::ERROR_NONE));
    jsonRpc->Release();
    plugin->Deinitialize(service);
}

TEST_F(APersistentStore, DeletesNamespaceInAccountScopeViaJsonRpc)
{
    class GrpcStore2 : public NiceMock<Store2Mock> {
    public:
        GrpcStore2()
        {
            EXPECT_CALL(*this, DeleteNamespace(_, _))
                .WillRepeatedly(Invoke(
                    [](const ScopeType scope, const string& ns) {
                        EXPECT_THAT(scope, Eq(ScopeType::ACCOUNT));
                        EXPECT_THAT(ns, Eq(kAppId));
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<GrpcStore2> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto jsonRpc = plugin->QueryInterface<ILocalDispatcher>();
    ASSERT_THAT(jsonRpc, NotNull());
    DeleteNamespaceParamsInfo params;
    params.Scope = WPEFramework::JsonData::PersistentStore::ScopeType::ACCOUNT;
    params.Namespace = kAppId;
    string paramsJsonStr;
    params.ToString(paramsJsonStr);
    string resultJsonStr;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "deleteNamespace", paramsJsonStr, resultJsonStr), Eq(WPEFramework::Core::ERROR_NONE));
    jsonRpc->Release();
    plugin->Deinitialize(service);
}

TEST_F(APersistentStore, FlushesCacheViaJsonRpc)
{
    class SqliteStoreCache : public NiceMock<StoreCacheMock> {
    public:
        SqliteStoreCache()
        {
            EXPECT_CALL(*this, FlushCache())
                .WillRepeatedly(Return(WPEFramework::Core::ERROR_NONE));
        }
    };
    PublishedServiceType<SqliteStoreCache> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto jsonRpc = plugin->QueryInterface<ILocalDispatcher>();
    ASSERT_THAT(jsonRpc, NotNull());
    string resultJsonStr;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "flushCache", "", resultJsonStr), Eq(WPEFramework::Core::ERROR_NONE));
    jsonRpc->Release();
    plugin->Deinitialize(service);
}

TEST_F(APersistentStore, GetsKeysInDeviceScopeViaJsonRpc)
{
    class SqliteStoreInspector : public NiceMock<StoreInspectorMock> {
    public:
        SqliteStoreInspector()
        {
            EXPECT_CALL(*this, GetKeys(_, _, _))
                .WillRepeatedly(Invoke(
                    [](const ScopeType scope, const string& ns, IStringIterator*& keys) {
                        EXPECT_THAT(scope, Eq(ScopeType::DEVICE));
                        EXPECT_THAT(ns, Eq(kAppId));
                        keys = (WPEFramework::Core::Service<StringIterator>::Create<IStringIterator>(kKeys));
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<SqliteStoreInspector> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto jsonRpc = plugin->QueryInterface<ILocalDispatcher>();
    ASSERT_THAT(jsonRpc, NotNull());
    DeleteNamespaceParamsInfo params;
    params.Namespace = kAppId;
    string paramsJsonStr;
    params.ToString(paramsJsonStr);
    string resultJsonStr;
    ASSERT_THAT(jsonRpc->Invoke(0, 0, "", "getKeys", paramsJsonStr, resultJsonStr), Eq(WPEFramework::Core::ERROR_NONE));
    GetKeysResultData result;
    result.FromString(resultJsonStr);
    auto index(result.Keys.Elements());
    ASSERT_THAT(index.Next(), IsTrue());
    EXPECT_THAT(index.Current().Value(), Eq(kKeys.at(0)));
    ASSERT_THAT(index.Next(), IsTrue());
    EXPECT_THAT(index.Current().Value(), Eq(kKeys.at(1)));
    EXPECT_THAT(index.Next(), IsFalse());
    jsonRpc->Release();
    plugin->Deinitialize(service);
}

TEST_F(APersistentStore, GetsNamespacesInDeviceScopeViaJsonRpc)
{
    class SqliteStoreInspector : public NiceMock<StoreInspectorMock> {
    public:
        SqliteStoreInspector()
        {
            EXPECT_CALL(*this, GetNamespaces(_, _))
                .WillRepeatedly(Invoke(
                    [](const ScopeType scope, IStringIterator*& namespaces) {
                        EXPECT_THAT(scope, Eq(ScopeType::DEVICE));
                        namespaces = (WPEFramework::Core::Service<StringIterator>::Create<IStringIterator>(kAppIds));
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<SqliteStoreInspector> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto jsonRpc = plugin->QueryInterface<ILocalDispatcher>();
    ASSERT_THAT(jsonRpc, NotNull());
    string resultJsonStr;
    ASSERT_THAT(jsonRpc->Invoke(0, 0, "", "getNamespaces", "", resultJsonStr), Eq(WPEFramework::Core::ERROR_NONE));
    GetNamespacesResultData result;
    result.FromString(resultJsonStr);
    auto index(result.Namespaces.Elements());
    ASSERT_THAT(index.Next(), IsTrue());
    EXPECT_THAT(index.Current().Value(), Eq(kAppIds.at(0)));
    ASSERT_THAT(index.Next(), IsTrue());
    EXPECT_THAT(index.Current().Value(), Eq(kAppIds.at(1)));
    EXPECT_THAT(index.Next(), IsFalse());
    jsonRpc->Release();
    plugin->Deinitialize(service);
}

TEST_F(APersistentStore, GetsStorageSizesInDeviceScopeViaJsonRpc)
{
    class SqliteStoreInspector : public NiceMock<StoreInspectorMock> {
    public:
        SqliteStoreInspector()
        {
            EXPECT_CALL(*this, GetStorageSizes(_, _))
                .WillRepeatedly(Invoke(
                    [](const ScopeType scope, INamespaceSizeIterator*& storageList) {
                        EXPECT_THAT(scope, Eq(ScopeType::DEVICE));
                        storageList = (WPEFramework::Core::Service<IteratorType<INamespaceSizeIterator>>::Create<INamespaceSizeIterator>(kSizes));
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<SqliteStoreInspector> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto jsonRpc = plugin->QueryInterface<ILocalDispatcher>();
    ASSERT_THAT(jsonRpc, NotNull());
    string resultJsonStr;
    ASSERT_THAT(jsonRpc->Invoke(0, 0, "", "getStorageSizes", "", resultJsonStr), Eq(WPEFramework::Core::ERROR_NONE));
    GetStorageSizesResultData result;
    result.FromString(resultJsonStr);
    auto index(result.StorageList.Elements());
    ASSERT_THAT(index.Next(), IsTrue());
    EXPECT_THAT(index.Current().Namespace.Value(), Eq(kSizes.at(0).ns));
    EXPECT_THAT(index.Current().Size.Value(), Eq(kSizes.at(0).size));
    ASSERT_THAT(index.Next(), IsTrue());
    EXPECT_THAT(index.Current().Namespace.Value(), Eq(kSizes.at(1).ns));
    EXPECT_THAT(index.Current().Size.Value(), Eq(kSizes.at(1).size));
    EXPECT_THAT(index.Next(), IsFalse());
    jsonRpc->Release();
    plugin->Deinitialize(service);
}

TEST_F(APersistentStore, GetsNamespaceStorageLimitInDeviceScopeViaJsonRpc)
{
    class SqliteStoreLimit : public NiceMock<StoreLimitMock> {
    public:
        SqliteStoreLimit()
        {
            EXPECT_CALL(*this, GetNamespaceStorageLimit(_, _, _))
                .WillRepeatedly(Invoke(
                    [](const ScopeType scope, const string& ns, uint32_t& size) {
                        EXPECT_THAT(scope, Eq(ScopeType::DEVICE));
                        EXPECT_THAT(ns, Eq(kAppId));
                        size = kSize;
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<SqliteStoreLimit> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto jsonRpc = plugin->QueryInterface<ILocalDispatcher>();
    ASSERT_THAT(jsonRpc, NotNull());
    DeleteNamespaceParamsInfo params;
    params.Namespace = kAppId;
    string paramsJsonStr;
    params.ToString(paramsJsonStr);
    string resultJsonStr;
    ASSERT_THAT(jsonRpc->Invoke(0, 0, "", "getNamespaceStorageLimit", paramsJsonStr, resultJsonStr), Eq(WPEFramework::Core::ERROR_NONE));
    GetNamespaceStorageLimitResultData result;
    result.FromString(resultJsonStr);
    EXPECT_THAT(result.StorageLimit.Value(), Eq(kSize));
    jsonRpc->Release();
    plugin->Deinitialize(service);
}

TEST_F(APersistentStore, SetsNamespaceStorageLimitInDeviceScopeViaJsonRpc)
{
    class SqliteStoreLimit : public NiceMock<StoreLimitMock> {
    public:
        SqliteStoreLimit()
        {
            EXPECT_CALL(*this, SetNamespaceStorageLimit(_, _, _))
                .WillRepeatedly(Invoke(
                    [](const ScopeType scope, const string& ns, const uint32_t size) {
                        EXPECT_THAT(scope, Eq(ScopeType::DEVICE));
                        EXPECT_THAT(ns, Eq(kAppId));
                        EXPECT_THAT(size, Eq(kSize));
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<SqliteStoreLimit> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto jsonRpc = plugin->QueryInterface<ILocalDispatcher>();
    ASSERT_THAT(jsonRpc, NotNull());
    SetNamespaceStorageLimitParamsData params;
    params.Namespace = kAppId;
    params.StorageLimit = kSize;
    string paramsJsonStr;
    params.ToString(paramsJsonStr);
    string resultJsonStr;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "setNamespaceStorageLimit", paramsJsonStr, resultJsonStr), Eq(WPEFramework::Core::ERROR_NONE));
    jsonRpc->Release();
    plugin->Deinitialize(service);
}

TEST_F(APersistentStore, GetsValueInDeviceScopeViaIStore)
{
    class SqliteStore2 : public NiceMock<Store2Mock> {
    public:
        SqliteStore2()
        {
            EXPECT_CALL(*this, GetValue(_, _, _, _, _))
                .WillRepeatedly(Invoke(
                    [](const ScopeType scope, const string& ns, const string& key, string& value, uint32_t& ttl) {
                        EXPECT_THAT(scope, Eq(ScopeType::DEVICE));
                        EXPECT_THAT(ns, Eq(kAppId));
                        EXPECT_THAT(key, Eq(kKey));
                        value = kValue;
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<SqliteStore2> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto store = plugin->QueryInterface<IStore>();
    ASSERT_THAT(store, NotNull());
    string value;
    ASSERT_THAT(store->GetValue(kAppId, kKey, value), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(kValue));
    store->Release();
    plugin->Deinitialize(service);
}

TEST_F(APersistentStore, SetsValueInDeviceScopeViaIStore)
{
    class SqliteStore2 : public NiceMock<Store2Mock> {
    public:
        SqliteStore2()
        {
            EXPECT_CALL(*this, SetValue(_, _, _, _, _))
                .WillRepeatedly(Invoke(
                    [](const ScopeType scope, const string& ns, const string& key, const string& value, const uint32_t ttl) {
                        EXPECT_THAT(scope, Eq(ScopeType::DEVICE));
                        EXPECT_THAT(ns, Eq(kAppId));
                        EXPECT_THAT(key, Eq(kKey));
                        EXPECT_THAT(value, Eq(kValue));
                        EXPECT_THAT(ttl, Eq(0));
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<SqliteStore2> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto store = plugin->QueryInterface<IStore>();
    ASSERT_THAT(store, NotNull());
    EXPECT_THAT(store->SetValue(kAppId, kKey, kValue), Eq(WPEFramework::Core::ERROR_NONE));
    store->Release();
    plugin->Deinitialize(service);
}

TEST_F(APersistentStore, DeletesKeyInDeviceScopeViaIStore)
{
    class SqliteStore2 : public NiceMock<Store2Mock> {
    public:
        SqliteStore2()
        {
            EXPECT_CALL(*this, DeleteKey(_, _, _))
                .WillRepeatedly(Invoke(
                    [](const ScopeType scope, const string& ns, const string& key) {
                        EXPECT_THAT(scope, Eq(ScopeType::DEVICE));
                        EXPECT_THAT(ns, Eq(kAppId));
                        EXPECT_THAT(key, Eq(kKey));
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<SqliteStore2> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto store = plugin->QueryInterface<IStore>();
    ASSERT_THAT(store, NotNull());
    EXPECT_THAT(store->DeleteKey(kAppId, kKey), Eq(WPEFramework::Core::ERROR_NONE));
    store->Release();
    plugin->Deinitialize(service);
}

TEST_F(APersistentStore, DeletesNamespaceInDeviceScopeViaIStore)
{
    class SqliteStore2 : public NiceMock<Store2Mock> {
    public:
        SqliteStore2()
        {
            EXPECT_CALL(*this, DeleteNamespace(_, _))
                .WillRepeatedly(Invoke(
                    [](const ScopeType scope, const string& ns) {
                        EXPECT_THAT(scope, Eq(ScopeType::DEVICE));
                        EXPECT_THAT(ns, Eq(kAppId));
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<SqliteStore2> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto store = plugin->QueryInterface<IStore>();
    ASSERT_THAT(store, NotNull());
    EXPECT_THAT(store->DeleteNamespace(kAppId), Eq(WPEFramework::Core::ERROR_NONE));
    store->Release();
    plugin->Deinitialize(service);
}
