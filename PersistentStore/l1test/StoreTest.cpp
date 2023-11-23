#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../PersistentStore.h"

#include "ServiceMock.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;
using namespace WPEFramework::Exchange;

using ::testing::Eq;
using ::testing::NiceMock;
using ::testing::NotNull;
using ::testing::Test;

const std::string Path = "/tmp/persistentstore/l1test/storetest";
const uint32_t MaxSize = 100;
const uint32_t MaxValue = 3;

class AStore : public Test {
protected:
    PluginHost::IPlugin* plugin;
    NiceMock<ServiceMock> service;
    Exchange::IStore* store;

    AStore()
    {
        Core::File(Path).Destroy();

        plugin = Core::Service<PersistentStore>::Create<PluginHost::IPlugin>();

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
    virtual ~AStore() override
    {
        plugin->Release();
    }
    virtual void SetUp() override
    {
        ASSERT_THAT(plugin->Initialize(&service), Eq(""));
        store = plugin->QueryInterface<Exchange::IStore>();
        ASSERT_THAT(store, NotNull());
    }
    virtual void TearDown() override
    {
        if (store)
            store->Release();
        plugin->Deinitialize(&service);
    }
};

class StoreNotification : public IStore::INotification {
public:
    StoreNotification()
        : valueChanged(false, true)
    {
    }
    ~StoreNotification() = default;
    virtual void ValueChanged(const string& ns, const string& key, const string& value) override
    {
        EXPECT_THAT(ns, Eq("ns1"));
        EXPECT_THAT(key, Eq("k1"));
        EXPECT_THAT(value, Eq("v1"));
        valueChanged.SetEvent();
    }
    virtual void StorageExceeded() override
    {
    }

    BEGIN_INTERFACE_MAP(StoreNotification)
    INTERFACE_ENTRY(IStore::INotification)
    END_INTERFACE_MAP

    Core::Event valueChanged;
};

TEST_F(AStore, TriggersNotificationWhenValueIsSet)
{
    Core::Sink<StoreNotification> sink;
    store->Register(&sink);
    EXPECT_THAT(store->SetValue("ns1", "k1", "v1"), Eq(Core::ERROR_NONE));
    EXPECT_THAT(sink.valueChanged.Lock(100), Eq(Core::ERROR_NONE));
    store->Unregister(&sink);
}

class AStoreWithValue : public AStore {
protected:
    AStoreWithValue()
        : AStore()
    {
    }
    virtual ~AStoreWithValue() override = default;
    virtual void SetUp() override
    {
        AStore::SetUp();
        ASSERT_THAT(store->SetValue("ns1", "k1", "v1"), Eq(Core::ERROR_NONE));
    }
};

TEST_F(AStoreWithValue, GetsValue)
{
    string value;
    EXPECT_THAT(store->GetValue("ns1", "k1", value), Eq(Core::ERROR_NONE));
    EXPECT_THAT(value, Eq("v1"));
}

TEST_F(AStoreWithValue, UpdatesValue)
{
    EXPECT_THAT(store->SetValue("ns1", "k1", "a"), Eq(Core::ERROR_NONE));
    string value;
    EXPECT_THAT(store->GetValue("ns1", "k1", value), Eq(Core::ERROR_NONE));
    EXPECT_THAT(value, Eq("a"));
}

TEST_F(AStoreWithValue, DoesNotGetDeletedValue)
{
    EXPECT_THAT(store->DeleteKey("ns1", "k1"), Eq(Core::ERROR_NONE));
    string value;
    EXPECT_THAT(store->GetValue("ns1", "k1", value), Eq(Core::ERROR_UNKNOWN_KEY));
}

TEST_F(AStoreWithValue, DoesNotGetValueInDeletedNamespace)
{
    EXPECT_THAT(store->DeleteNamespace("ns1"), Eq(Core::ERROR_NONE));
    string value;
    EXPECT_THAT(store->GetValue("ns1", "k1", value), Eq(Core::ERROR_NOT_EXIST));
}
