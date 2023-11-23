#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Handle.h"
#include "../Store2Type.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;

using ::testing::Eq;
using ::testing::Le;
using ::testing::Test;

const std::string Path = "/tmp/sqlite/l1test/store2test";
const uint32_t MaxSize = 100;
const uint32_t MaxValue = 10;
const uint32_t Limit = 50;

class AStore2 : public Test {
protected:
    Exchange::IStore2* store2;
    ~AStore2() override = default;
    void SetUp() override
    {
        Core::File(Path).Destroy();
        // File is destroyed

        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_PATH"), Path);
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_MAXSIZE"), std::to_string(MaxSize));
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_MAXVALUE"), std::to_string(MaxValue));
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_LIMIT"), std::to_string(Limit));
        store2 = Core::Service<Sqlite::Store2Type<Sqlite::Handle>>::Create<Exchange::IStore2>();
    }
    void TearDown() override
    {
        store2->Release();
    }
};

TEST_F(AStore2, DoesNotSetEmptyNamespace)
{
    EXPECT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "", "x", "x", 0), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotSetEmptyKey)
{
    EXPECT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "x", "", "x", 0), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotSetTooLargeNamespaceName)
{
    EXPECT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "this is too large", "x", "x", 0), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotSetTooLargeKey)
{
    EXPECT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "x", "this is too large", "x", 0), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotSetTooLargeValue)
{
    EXPECT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "x", "x", "this is too large", 0), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotGetValueInUnknownNamespace)
{
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(Exchange::IStore2::ScopeType::DEVICE, "x", "x", value, ttl), Eq(Core::ERROR_NOT_EXIST));
}

TEST_F(AStore2, DeletesKeyInUnknownNamespaceWithoutError)
{
    EXPECT_THAT(store2->DeleteKey(Exchange::IStore2::ScopeType::DEVICE, "x", "x"), Eq(Core::ERROR_NONE));
}

TEST_F(AStore2, DeletesUnknownNamespaceWithoutError)
{
    EXPECT_THAT(store2->DeleteNamespace(Exchange::IStore2::ScopeType::DEVICE, "x"), Eq(Core::ERROR_NONE));
}

TEST_F(AStore2, SetsEmptyValueWithoutError)
{
    EXPECT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key1", "", 0), Eq(Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key1", value, ttl), Eq(Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(""));
    EXPECT_THAT(ttl, Eq(0));
}

TEST_F(AStore2, GetsValueWithTtlThatWasSet)
{
    EXPECT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key1", "value1", 100), Eq(Core::ERROR_NONE));
    // Value with ttl set

    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key1", value, ttl), Eq(Core::ERROR_NONE));
    EXPECT_THAT(value, Eq("value1"));
    EXPECT_THAT(ttl, Le(100));
}

TEST_F(AStore2, DoesNotGetExpiredValueThatWasSet)
{
    EXPECT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key1", "value1", 1), Eq(Core::ERROR_NONE));
    // Value with ttl set
    Core::Event lock(false, true);
    lock.Lock(1000);
    // Value expired

    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key1", value, ttl), Eq(Core::ERROR_UNKNOWN_KEY));
}

class AStore2WithNotification : public AStore2 {
protected:
    class Store2Notification : public Exchange::IStore2::INotification {
    public:
        Core::Event _valueChanged;
        Exchange::IStore2::ScopeType _scope;
        string _ns;
        string _key;
        string _value;
        Store2Notification()
            : _valueChanged(false, true)
        {
        }
        void ValueChanged(const Exchange::IStore2::ScopeType scope, const string& ns, const string& key, const string& value) override
        {
            _valueChanged.SetEvent();
            _scope = scope;
            _ns = ns;
            _key = key;
            _value = value;
        }

        BEGIN_INTERFACE_MAP(Store2Notification)
        INTERFACE_ENTRY(Exchange::IStore2::INotification)
        END_INTERFACE_MAP
    };
    Core::Sink<Store2Notification> sink;
    ~AStore2WithNotification() override = default;
    void SetUp() override
    {
        AStore2::SetUp();
        store2->Register(&sink);
    }
    void TearDown() override
    {
        store2->Unregister(&sink);
        AStore2::TearDown();
    }
};

TEST_F(AStore2WithNotification, TriggersNotificationWhenValueIsSet)
{
    ASSERT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key1", "value1", 0), Eq(Core::ERROR_NONE));
    // Value is set

    EXPECT_THAT(sink._valueChanged.Lock(100), Eq(Core::ERROR_NONE));
    EXPECT_THAT(sink._scope, Eq(Exchange::IStore2::ScopeType::DEVICE));
    EXPECT_THAT(sink._ns, Eq("ns1"));
    EXPECT_THAT(sink._key, Eq("key1"));
    EXPECT_THAT(sink._value, Eq("value1"));
}

class AStore2WithValue : public AStore2 {
protected:
    ~AStore2WithValue() override = default;
    void SetUp() override
    {
        AStore2::SetUp();
        ASSERT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key1", "value1", 0), Eq(Core::ERROR_NONE));
    }
};

TEST_F(AStore2WithValue, DoesNotGetUnknownValueInExistingNamespace)
{
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "x", value, ttl), Eq(Core::ERROR_UNKNOWN_KEY));
}

TEST_F(AStore2WithValue, DeletesUnknownKeyInExistingNamespaceWithoutError)
{
    EXPECT_THAT(store2->DeleteKey(Exchange::IStore2::ScopeType::DEVICE, "ns1", "x"), Eq(Core::ERROR_NONE));
}

TEST_F(AStore2WithValue, GetsValue)
{
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key1", value, ttl), Eq(Core::ERROR_NONE));
    EXPECT_THAT(value, Eq("value1"));
    EXPECT_THAT(ttl, Eq(0));
}

TEST_F(AStore2WithValue, UpdatesValue)
{
    EXPECT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key1", "value2", 0), Eq(Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key1", value, ttl), Eq(Core::ERROR_NONE));
    EXPECT_THAT(value, Eq("value2"));
    EXPECT_THAT(ttl, Eq(0));
}

TEST_F(AStore2WithValue, DoesNotGetDeletedValue)
{
    EXPECT_THAT(store2->DeleteKey(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key1"), Eq(Core::ERROR_NONE));
    // Value is deleted

    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key1", value, ttl), Eq(Core::ERROR_UNKNOWN_KEY));
}

TEST_F(AStore2WithValue, DoesNotGetValueInDeletedNamespace)
{
    EXPECT_THAT(store2->DeleteNamespace(Exchange::IStore2::ScopeType::DEVICE, "ns1"), Eq(Core::ERROR_NONE));
    // Namespace is deleted

    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key1", value, ttl), Eq(Core::ERROR_NOT_EXIST));
}

class AStore2AtMaxSize : public AStore2 {
protected:
    ~AStore2AtMaxSize() override = default;
    void SetUp() override
    {
        AStore2::SetUp();
        for (int i = 0; i < 7; i++) {
            ASSERT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "ns" + std::to_string(i), "key1", "value1", 0), Eq(Core::ERROR_NONE));
        }
        ASSERT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "ns7", "key1", "va", 0), Eq(Core::ERROR_NONE));
        // Size is 100
    }
};

TEST_F(AStore2AtMaxSize, DoesNotSetValue)
{
    EXPECT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "x", "x", "x", 0), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2AtMaxSize, SetsValueAfterDeletesValue)
{
    EXPECT_THAT(store2->DeleteKey(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key1"), Eq(Core::ERROR_NONE));
    // Value deleted, size is 90

    EXPECT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key2", "value2", 0), Eq(Core::ERROR_NONE));
}

class AStore2AtLimit : public AStore2 {
protected:
    ~AStore2AtLimit() override = default;
    void SetUp() override
    {
        AStore2::SetUp();
        for (int i = 0; i < 5; i++) {
            ASSERT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key" + std::to_string(i), "value" + std::to_string(i), 0), Eq(Core::ERROR_NONE));
        }
        // Namespace size is 50
    }
};

TEST_F(AStore2AtLimit, DoesNotSetValue)
{
    EXPECT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key5", "value5", 0), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2AtLimit, SetsValueInAnotherNamespaceWithoutError)
{
    EXPECT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "ns2", "key1", "value1", 0), Eq(Core::ERROR_NONE));
}

TEST_F(AStore2AtLimit, SetsValueAfterDeletesValue)
{
    EXPECT_THAT(store2->DeleteKey(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key1"), Eq(Core::ERROR_NONE));
    // Value deleted, namespace size is 40

    EXPECT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key5", "value5", 0), Eq(Core::ERROR_NONE));
}
