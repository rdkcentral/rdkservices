#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../SqliteDb.h"
#include "../SqliteStore2.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;
using namespace WPEFramework::Exchange;

using ::testing::Eq;
using ::testing::Le;
using ::testing::Test;

const std::string Path = "/tmp/sqlite/l1test/sqlitestore2test";
const uint32_t MaxSize = 15;
const uint32_t MaxValue = 3;

class AnSqliteStore2 : public Test {
protected:
    ISqliteDb* db;
    IStore2* store;

    AnSqliteStore2()
    {
        Core::File(Path).Destroy();
        db = Core::Service<SqliteDb>::Create<ISqliteDb>(Path, MaxSize, MaxValue);
        store = Core::Service<SqliteStore2>::Create<IStore2>(db);
    }
    virtual ~AnSqliteStore2() override
    {
        db->Release();
        store->Release();
    }
    virtual void SetUp()
    {
        ASSERT_THAT(db->Open(), Eq(Core::ERROR_NONE));
    }
};

TEST_F(AnSqliteStore2, DoesNotSetEmptyNamespace)
{
    EXPECT_THAT(store->SetValue("", "k1", "v1", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AnSqliteStore2, DoesNotSetEmptyKey)
{
    EXPECT_THAT(store->SetValue("ns1", "", "v1", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AnSqliteStore2, DoesNotSetLargeNamespace)
{
    EXPECT_THAT(store->SetValue("ns111", "k1", "v1", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AnSqliteStore2, DoesNotSetLargeKey)
{
    EXPECT_THAT(store->SetValue("ns1", "k111", "v1", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AnSqliteStore2, DoesNotSetLargeValue)
{
    EXPECT_THAT(store->SetValue("ns1", "k1", "v111", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AnSqliteStore2, DoesNotGetValueWhenEmpty)
{
    string value;
    uint32_t ttl;
    EXPECT_THAT(store->GetValue("ns1", "k1", IStore2::ScopeType::DEVICE, value, ttl), Eq(Core::ERROR_NOT_EXIST));
}

TEST_F(AnSqliteStore2, DeletesUnknownKeyWithoutError)
{
    EXPECT_THAT(store->DeleteKey("ns1", "k1", IStore2::ScopeType::DEVICE), Eq(Core::ERROR_NONE));
}

TEST_F(AnSqliteStore2, DeletesUnknownNamespaceWithoutError)
{
    EXPECT_THAT(store->DeleteNamespace("ns1", IStore2::ScopeType::DEVICE), Eq(Core::ERROR_NONE));
}

TEST_F(AnSqliteStore2, CanSetEmptyValue)
{
    EXPECT_THAT(store->SetValue("ns1", "k1", "", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    EXPECT_THAT(store->GetValue("ns1", "k1", IStore2::ScopeType::DEVICE, value, ttl), Eq(Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(""));
    EXPECT_THAT(ttl, Eq(0));
}

TEST_F(AnSqliteStore2, GetsValueWithTtl)
{
    EXPECT_THAT(store->SetValue("ns1", "k1", "v1", IStore2::ScopeType::DEVICE, 100), Eq(Core::ERROR_NONE));

    string value;
    uint32_t ttl;
    EXPECT_THAT(store->GetValue("ns1", "k1", IStore2::ScopeType::DEVICE, value, ttl), Eq(Core::ERROR_NONE));
    EXPECT_THAT(value, Eq("v1"));
    EXPECT_THAT(ttl, Le(100));
}

TEST_F(AnSqliteStore2, DoesNotGetValueWhenTtlExpires)
{
    EXPECT_THAT(store->SetValue("ns1", "k1", "v1", IStore2::ScopeType::DEVICE, 1), Eq(Core::ERROR_NONE));
    Core::Event lock(false, true);
    lock.Lock(1000);

    string value;
    uint32_t ttl;
    EXPECT_THAT(store->GetValue("ns1", "k1", IStore2::ScopeType::DEVICE, value, ttl), Eq(Core::ERROR_UNKNOWN_KEY));
}

class StoreNotification : public IStore2::INotification {
public:
    StoreNotification()
        : valueChanged(false, true)
    {
    }
    ~StoreNotification() = default;
    virtual void ValueChanged(const string& ns, const string& key, const string& value, const IStore2::ScopeType scope) override
    {
        EXPECT_THAT(ns, Eq("ns1"));
        EXPECT_THAT(key, Eq("k1"));
        EXPECT_THAT(value, Eq("v1"));
        EXPECT_THAT(scope, Eq(IStore2::ScopeType::DEVICE));
        valueChanged.SetEvent();
    }

    BEGIN_INTERFACE_MAP(StoreNotification)
    INTERFACE_ENTRY(IStore2::INotification)
    END_INTERFACE_MAP

    Core::Event valueChanged;
};

TEST_F(AnSqliteStore2, TriggersNotificationWhenValueIsSet)
{
    Core::Sink<StoreNotification> sink;
    store->Register(&sink);

    EXPECT_THAT(store->SetValue("ns1", "k1", "v1", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_NONE));

    EXPECT_THAT(sink.valueChanged.Lock(100), Eq(Core::ERROR_NONE));

    store->Unregister(&sink);
}

class AnSqliteStore2WithValue : public AnSqliteStore2 {
protected:
    AnSqliteStore2WithValue()
        : AnSqliteStore2()
    {
    }
    virtual ~AnSqliteStore2WithValue() override = default;
    virtual void SetUp() override
    {
        AnSqliteStore2::SetUp();
        ASSERT_THAT(store->SetValue("ns1", "k1", "v1", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_NONE));
    }
};

TEST_F(AnSqliteStore2WithValue, DoesNotGetUnknownValue)
{
    string value;
    uint32_t ttl;
    EXPECT_THAT(store->GetValue("ns1", "k2", IStore2::ScopeType::DEVICE, value, ttl), Eq(Core::ERROR_UNKNOWN_KEY));
}

TEST_F(AnSqliteStore2WithValue, DeletesUnknownKeyWithoutError)
{
    EXPECT_THAT(store->DeleteKey("ns1", "k2", IStore2::ScopeType::DEVICE), Eq(Core::ERROR_NONE));
}

TEST_F(AnSqliteStore2WithValue, GetsValue)
{
    string value;
    uint32_t ttl;
    EXPECT_THAT(store->GetValue("ns1", "k1", IStore2::ScopeType::DEVICE, value, ttl), Eq(Core::ERROR_NONE));
    EXPECT_THAT(value, Eq("v1"));
    EXPECT_THAT(ttl, Eq(0));
}

TEST_F(AnSqliteStore2WithValue, UpdatesValue)
{
    EXPECT_THAT(store->SetValue("ns1", "k1", "a", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    EXPECT_THAT(store->GetValue("ns1", "k1", IStore2::ScopeType::DEVICE, value, ttl), Eq(Core::ERROR_NONE));
    EXPECT_THAT(value, Eq("a"));
    EXPECT_THAT(ttl, Eq(0));
}

TEST_F(AnSqliteStore2WithValue, DoesNotGetDeletedValue)
{
    EXPECT_THAT(store->DeleteKey("ns1", "k1", IStore2::ScopeType::DEVICE), Eq(Core::ERROR_NONE));

    string value;
    uint32_t ttl;
    EXPECT_THAT(store->GetValue("ns1", "k1", IStore2::ScopeType::DEVICE, value, ttl), Eq(Core::ERROR_UNKNOWN_KEY));
}

TEST_F(AnSqliteStore2WithValue, DoesNotGetValueInDeletedNamespace)
{
    EXPECT_THAT(store->DeleteNamespace("ns1", IStore2::ScopeType::DEVICE), Eq(Core::ERROR_NONE));

    string value;
    uint32_t ttl;
    EXPECT_THAT(store->GetValue("ns1", "k1", IStore2::ScopeType::DEVICE, value, ttl), Eq(Core::ERROR_NOT_EXIST));
}

TEST_F(AnSqliteStore2WithValue, DoesNotSetValueWhenReachesLimit)
{
    EXPECT_THAT(store->SetValue("ns1", "k2", "v2", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_NONE));
    EXPECT_THAT(store->SetValue("ns1", "k3", "v3", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_NONE));

    EXPECT_THAT(store->SetValue("ns1", "k4", "v4", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
    EXPECT_THAT(store->SetValue("ns2", "k5", "v5", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AnSqliteStore2WithValue, SetsValueAfterReachesLimitAndDeletesValues)
{
    EXPECT_THAT(store->SetValue("ns1", "k2", "v2", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_NONE));
    EXPECT_THAT(store->SetValue("ns1", "k3", "v3", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_NONE));
    EXPECT_THAT(store->SetValue("ns1", "k4", "v4", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
    EXPECT_THAT(store->SetValue("ns2", "k5", "v5", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
    EXPECT_THAT(store->DeleteKey("ns1", "k1", IStore2::ScopeType::DEVICE), Eq(Core::ERROR_NONE));
    EXPECT_THAT(store->DeleteKey("ns1", "k2", IStore2::ScopeType::DEVICE), Eq(Core::ERROR_NONE));

    EXPECT_THAT(store->SetValue("ns1", "k4", "v4", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_NONE));
}
