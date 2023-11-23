#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../PersistentStore.h"

#include "FactoriesImplementation.h"
#include "ServiceMock.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;
using namespace WPEFramework::Exchange;

using ::testing::_;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::NiceMock;
using ::testing::NotNull;
using ::testing::Test;

const std::string Path = "/tmp/persistentstore/l1test/persistentstoretest";
const uint32_t MaxSize = 100;
const uint32_t MaxValue = 3;

class APersistentStore : public Test {
protected:
    PluginHost::IPlugin* plugin;
    PluginHost::ILocalDispatcher* dispatcher;
    PluginHost::IDispatcher::ICallback* callback;

    APersistentStore()
    {
        Core::File(Path).Destroy();
        plugin = Core::Service<PersistentStore>::Create<PluginHost::IPlugin>();
        callback = static_cast<PersistentStore*>(plugin);
    }
    virtual ~APersistentStore() override
    {
        plugin->Release();
    }
    virtual void SetUp() override
    {
        dispatcher = plugin->QueryInterface<PluginHost::ILocalDispatcher>();
        ASSERT_THAT(dispatcher, NotNull());
    }
    virtual void TearDown() override
    {
        if (dispatcher)
            dispatcher->Release();
    }
};

TEST_F(APersistentStore, DoesNotCreateFile)
{
    EXPECT_THAT(Core::File(Path).Exists(), IsFalse());
}

class AnInitializedPersistentStore : public APersistentStore {
protected:
    NiceMock<ServiceMock> service;
    Exchange::IStore* store;
    Exchange::IStore2* store2;
    Exchange::IStoreCache* storeCache;
    Exchange::IStoreInspector* storeInspector;

    AnInitializedPersistentStore()
        : APersistentStore()
    {
        JsonObject config;
        config["path"] = Path;
        config["maxsize"] = MaxSize;
        config["maxvalue"] = MaxValue;
        string configJsonStr;
        config.ToString(configJsonStr);

        ON_CALL(service, ConfigLine())
            .WillByDefault(
                ::testing::Return(configJsonStr));
    }
    virtual ~AnInitializedPersistentStore() override = default;
    virtual void SetUp() override
    {
        APersistentStore::SetUp();
        ASSERT_THAT(plugin->Initialize(&service), Eq(""));
        store = plugin->QueryInterface<Exchange::IStore>();
        ASSERT_THAT(store, NotNull());
        store2 = plugin->QueryInterface<Exchange::IStore2>();
        ASSERT_THAT(store2, NotNull());
        storeCache = plugin->QueryInterface<Exchange::IStoreCache>();
        ASSERT_THAT(storeCache, NotNull());
        storeInspector = plugin->QueryInterface<Exchange::IStoreInspector>();
        ASSERT_THAT(storeInspector, NotNull());
    }
    virtual void TearDown() override
    {
        if (store)
            store->Release();
        if (store2)
            store2->Release();
        if (storeCache)
            storeCache->Release();
        if (storeInspector)
            storeInspector->Release();
        plugin->Deinitialize(&service);
        APersistentStore::TearDown();
    }
};

TEST_F(AnInitializedPersistentStore, CreatesFile)
{
    EXPECT_THAT(Core::File(Path).Exists(), IsTrue());
}

TEST_F(AnInitializedPersistentStore, DoesNotGetValueInUnknownNamespace)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getValue", "{\"namespace\":\"x\",\"key\":\"x\"}", response), Eq(Core::ERROR_NOT_EXIST));
}

TEST_F(AnInitializedPersistentStore, DoesNotGetValueWhenMandatoryParamsAreMissing)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getValue", "{\"key\":\"x\"}", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getValue", "{\"namespace\":\"x\"}", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AnInitializedPersistentStore, DoesNotSetEmptyNamespace)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "setValue", "{\"namespace\":\"\",\"key\":\"x\",\"value\":\"x\"}", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AnInitializedPersistentStore, DoesNotSetEmptyKey)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "setValue", "{\"namespace\":\"x\",\"key\":\"\",\"value\":\"x\"}", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AnInitializedPersistentStore, DoesNotSetWhenMandatoryParamsAreMissing)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "setValue", "{\"key\":\"x\",\"value\":\"x\"}", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "setValue", "{\"namespace\":\"x\",\"value\":\"x\"}", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "setValue", "{\"namespace\":\"x\",\"key\":\"x\"}", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AnInitializedPersistentStore, DoesNotGetKeysWhenMandatoryParamsAreMissing)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getKeys", "", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AnInitializedPersistentStore, GetsKeysForUnknownNamespaceWithoutError)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getKeys", "{\"namespace\":\"ns1\"}", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"success\":true}"));
}

TEST_F(AnInitializedPersistentStore, GetsNamespaces)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getNamespaces", "", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"success\":true}"));
}

TEST_F(AnInitializedPersistentStore, GetsStorageSizes)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getStorageSizes", "", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{}"));
}

// Deprecated
TEST_F(AnInitializedPersistentStore, GetsStorageSize)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getStorageSize", "", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"namespaceSizes\":{},\"success\":true}"));
}

TEST_F(AnInitializedPersistentStore, DoesNotGetNamespaceStorageLimitForUnknownNamespace)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getNamespaceStorageLimit", "{\"namespace\":\"x\"}", response), Eq(Core::ERROR_NOT_EXIST));
}

TEST_F(AnInitializedPersistentStore, DoesNotGetStorageLimitWhenMandatoryParamsAreMissing)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getNamespaceStorageLimit", "", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AnInitializedPersistentStore, DoesNotSetStorageLimitWhenMandatoryParamsAreMissing)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "setNamespaceStorageLimit", "", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AnInitializedPersistentStore, DoesNotDeleteKeyWhenMandatoryParamsAreMissing)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "deleteKey", "{\"key\":\"k1\"}", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "deleteKey", "{\"namespace\":\"ns1\"}", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AnInitializedPersistentStore, DoesNotDeleteNamespaceWhenMandatoryParamsAreMissing)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "deleteNamespace", "", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AnInitializedPersistentStore, TriggersOnValueChangedWhenValueIsSet)
{
    NiceMock<FactoriesImplementation> factoriesImplementation;
    PluginHost::IFactories::Assign(&factoriesImplementation);

    dispatcher->Activate(&service);

    Core::Event onValueChanged(false, true);

    EXPECT_CALL(service, Submit(_, _))
        .Times(1)
        .WillOnce(Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_THAT(json->ToString(text), IsTrue());
                EXPECT_THAT(text, Eq("{"
                                     "\"jsonrpc\":\"2.0\","
                                     "\"method\":\"org.rdk.PersistentStore.onValueChanged\","
                                     "\"params\":{\"namespace\":\"ns1\",\"key\":\"k1\",\"value\":\"v1\",\"scope\":\"device\"}"
                                     "}"));

                onValueChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    callback->Subscribe(0, "onValueChanged", "org.rdk.PersistentStore");

    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "setValue", "{\"namespace\":\"ns1\",\"key\":\"k1\",\"value\":\"v1\"}", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"success\":true}"));

    EXPECT_THAT(onValueChanged.Lock(100), Eq(Core::ERROR_NONE));

    callback->Unsubscribe(0, "onValueChanged", "org.rdk.PersistentStore");

    dispatcher->Deactivate();

    PluginHost::IFactories::Assign(nullptr);
}

class AnInitializedPersistentStoreWithValues : public AnInitializedPersistentStore {
protected:
    AnInitializedPersistentStoreWithValues()
        : AnInitializedPersistentStore()
    {
    }
    virtual ~AnInitializedPersistentStoreWithValues() override = default;
    virtual void SetUp() override
    {
        AnInitializedPersistentStore::SetUp();
        string response;
        ASSERT_THAT(dispatcher->Invoke(0, 0, "", "setValue", "{\"namespace\":\"ns1\",\"key\":\"k1\",\"value\":\"v1\"}", response), Eq(Core::ERROR_NONE));
        ASSERT_THAT(response, Eq("{\"success\":true}"));
        ASSERT_THAT(dispatcher->Invoke(0, 0, "", "setValue", "{\"namespace\":\"ns1\",\"key\":\"k2\",\"value\":\"v2\"}", response), Eq(Core::ERROR_NONE));
        ASSERT_THAT(response, Eq("{\"success\":true}"));
        ASSERT_THAT(dispatcher->Invoke(0, 0, "", "setValue", "{\"namespace\":\"ns2\",\"key\":\"k3\",\"value\":\"v3\"}", response), Eq(Core::ERROR_NONE));
        ASSERT_THAT(response, Eq("{\"success\":true}"));
    }
};

TEST_F(AnInitializedPersistentStoreWithValues, GetsValues)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getValue", "{\"namespace\":\"ns1\",\"key\":\"k1\"}", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"value\":\"v1\",\"success\":true}"));
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getValue", "{\"namespace\":\"ns1\",\"key\":\"k2\"}", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"value\":\"v2\",\"success\":true}"));
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getValue", "{\"namespace\":\"ns2\",\"key\":\"k3\"}", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"value\":\"v3\",\"success\":true}"));
}

TEST_F(AnInitializedPersistentStoreWithValues, DoesNotGetUnknownKey)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getValue", "{\"namespace\":\"ns1\",\"key\":\"x\"}", response), Eq(Core::ERROR_UNKNOWN_KEY));
}

TEST_F(AnInitializedPersistentStoreWithValues, GetsKeys)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getKeys", "{\"namespace\":\"ns1\"}", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"keys\":[\"k1\",\"k2\"],\"success\":true}"));
}

TEST_F(AnInitializedPersistentStoreWithValues, GetsNamespaces)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getNamespaces", "", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"namespaces\":[\"ns1\",\"ns2\"],\"success\":true}"));
}

TEST_F(AnInitializedPersistentStoreWithValues, GetsStorageSizes)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getStorageSizes", "", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"storageList\":[{\"namespace\":\"ns1\",\"size\":8},{\"namespace\":\"ns2\",\"size\":4}]}"));
}

// Deprecated
TEST_F(AnInitializedPersistentStoreWithValues, GetsStorageSize)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getStorageSize", "", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"namespaceSizes\":{\"ns1\":8,\"ns2\":4},\"success\":true}"));
}

TEST_F(AnInitializedPersistentStoreWithValues, DoesNotGetDeletedKey)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "deleteKey", "{\"namespace\":\"ns1\",\"key\":\"k1\"}", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"success\":true}"));
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getValue", "{\"namespace\":\"ns1\",\"key\":\"k1\"}", response), Eq(Core::ERROR_UNKNOWN_KEY));
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getValue", "{\"namespace\":\"ns1\",\"key\":\"k2\"}", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"value\":\"v2\",\"success\":true}"));
}

TEST_F(AnInitializedPersistentStoreWithValues, DoesNotGetKeyInDeletedNamespace)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "deleteNamespace", "{\"namespace\":\"ns1\"}", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"success\":true}"));
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getValue", "{\"namespace\":\"ns1\",\"key\":\"k1\"}", response), Eq(Core::ERROR_NOT_EXIST));
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getValue", "{\"namespace\":\"ns1\",\"key\":\"k2\"}", response), Eq(Core::ERROR_NOT_EXIST));
}

TEST_F(AnInitializedPersistentStoreWithValues, FlushesCacheWithoutError)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "flushCache", "", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"success\":true}"));
}

class AnInitializedPersistentStoreWithValuesAndLimit : public AnInitializedPersistentStoreWithValues {
protected:
    AnInitializedPersistentStoreWithValuesAndLimit()
        : AnInitializedPersistentStoreWithValues()
    {
    }
    virtual ~AnInitializedPersistentStoreWithValuesAndLimit() override = default;
    virtual void SetUp() override
    {
        AnInitializedPersistentStoreWithValues::SetUp();
        string response;
        EXPECT_THAT(dispatcher->Invoke(0, 0, "", "setNamespaceStorageLimit", "{\"namespace\":\"ns1\",\"storageLimit\":\"10\"}", response), Eq(Core::ERROR_NONE));
    }
};

TEST_F(AnInitializedPersistentStoreWithValuesAndLimit, GetsNamespaceStorageLimit)
{
    string response;
    EXPECT_THAT(dispatcher->Invoke(0, 0, "", "getNamespaceStorageLimit", "{\"namespace\":\"ns1\"}", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"storageLimit\":10}"));
}
