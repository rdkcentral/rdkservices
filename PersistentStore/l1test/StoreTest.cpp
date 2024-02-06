#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../PersistentStore.h"

#include "ServiceMock.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;

using ::testing::Eq;
using ::testing::NiceMock;
using ::testing::NotNull;
using ::testing::Test;

const std::string Path = "/tmp/persistentstore/l1test/storetest";
const uint32_t MaxSize = 100;
const uint32_t MaxValue = 10;
const uint32_t Limit = 50;

class AStore : public Test {
protected:
    NiceMock<ServiceMock> service;
    PluginHost::IPlugin* plugin;
    Exchange::IStore* store;
    ~AStore() override = default;
    void SetUp() override
    {
        Core::File(Path).Destroy();
        // File is destroyed

        plugin = Core::Service<PersistentStore>::Create<PluginHost::IPlugin>();
        JsonObject config;
        config["path"] = Path;
        config["maxsize"] = MaxSize;
        config["maxvalue"] = MaxValue;
        config["limit"] = Limit;
        string configJsonStr;
        config.ToString(configJsonStr);
        ON_CALL(service, ConfigLine())
            .WillByDefault(
                ::testing::Return(configJsonStr));
        ASSERT_THAT(plugin->Initialize(&service), Eq(""));
        store = plugin->QueryInterface<Exchange::IStore>();
        ASSERT_THAT(store, NotNull());
    }
    void TearDown() override
    {
        if (store)
            store->Release();
        plugin->Deinitialize(&service);
        plugin->Release();
    }
};

class AStoreWithNotification : public AStore {
protected:
    class StoreNotification : public Exchange::IStore::INotification {
    public:
        Core::Event _valueChanged;
        string _ns;
        string _key;
        string _value;
        StoreNotification()
            : _valueChanged(false, true)
        {
        }
        void ValueChanged(const string& ns, const string& key, const string& value) override
        {
            _valueChanged.SetEvent();
            _ns = ns;
            _key = key;
            _value = value;
        }
        void StorageExceeded() override
        {
        }

        BEGIN_INTERFACE_MAP(StoreNotification)
        INTERFACE_ENTRY(Exchange::IStore::INotification)
        END_INTERFACE_MAP
    };
    Core::Sink<StoreNotification> sink;
    ~AStoreWithNotification() override = default;
    void SetUp() override
    {
        AStore::SetUp();
        store->Register(&sink);
    }
    void TearDown() override
    {
        store->Unregister(&sink);
        AStore::TearDown();
    }
};

TEST_F(AStoreWithNotification, TriggersNotificationWhenValueIsSet)
{
    ASSERT_THAT(store->SetValue("ns1", "key1", "value1"), Eq(Core::ERROR_NONE));
    // Value is set

    EXPECT_THAT(sink._valueChanged.Lock(100), Eq(Core::ERROR_NONE));
    EXPECT_THAT(sink._ns, Eq("ns1"));
    EXPECT_THAT(sink._key, Eq("key1"));
    EXPECT_THAT(sink._value, Eq("value1"));
}

class AStoreWithValue : public AStore {
protected:
    ~AStoreWithValue() override = default;
    void SetUp() override
    {
        AStore::SetUp();
        ASSERT_THAT(store->SetValue("ns1", "key1", "value1"), Eq(Core::ERROR_NONE));
    }
};

TEST_F(AStoreWithValue, GetsValue)
{
    string value;
    EXPECT_THAT(store->GetValue("ns1", "key1", value), Eq(Core::ERROR_NONE));
    EXPECT_THAT(value, Eq("value1"));
}

TEST_F(AStoreWithValue, UpdatesValue)
{
    EXPECT_THAT(store->SetValue("ns1", "key1", "a"), Eq(Core::ERROR_NONE));
    string value;
    EXPECT_THAT(store->GetValue("ns1", "key1", value), Eq(Core::ERROR_NONE));
    EXPECT_THAT(value, Eq("a"));
}

TEST_F(AStoreWithValue, DoesNotGetDeletedValue)
{
    EXPECT_THAT(store->DeleteKey("ns1", "key1"), Eq(Core::ERROR_NONE));
    // Value is deleted

    string value;
    EXPECT_THAT(store->GetValue("ns1", "key1", value), Eq(Core::ERROR_UNKNOWN_KEY));
}

TEST_F(AStoreWithValue, DoesNotGetValueInDeletedNamespace)
{
    EXPECT_THAT(store->DeleteNamespace("ns1"), Eq(Core::ERROR_NONE));
    // Namespace is deleted

    string value;
    EXPECT_THAT(store->GetValue("ns1", "key1", value), Eq(Core::ERROR_NOT_EXIST));
}
