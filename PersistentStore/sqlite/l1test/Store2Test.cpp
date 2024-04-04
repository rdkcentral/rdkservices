#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Store2.h"
#include "Store2NotificationMock.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::Invoke;
using ::testing::Le;
using ::testing::NiceMock;
using ::testing::Test;
using ::WPEFramework::Exchange::IStore2;
using ::WPEFramework::Plugin::Sqlite::Store2;

const auto kPath = "/tmp/persistentstore/sqlite/l1test/store2test";
const auto kMaxSize = 100;
const auto kMaxValue = 10;
const auto kLimit = 50;
const auto kValue = "value_1";
const auto kKey = "key_1";
const auto kAppId = "app_id_1";
const auto kTtl = 2;
const auto kNoTtl = 0;
const auto kScope = IStore2::ScopeType::DEVICE;
const auto kEmpty = "";
const auto kOversize = "this is too large";
const auto kUnknown = "unknown";

class AStore2 : public Test {
protected:
    IStore2* store2;
    AStore2()
        : store2(WPEFramework::Core::Service<Store2>::Create<IStore2>(kPath, kMaxSize, kMaxValue, kLimit))
    {
    }
    ~AStore2() override
    {
        store2->Release();
    }
};

TEST_F(AStore2, DoesNotSetValueWhenNamespaceEmpty)
{
    EXPECT_THAT(store2->SetValue(kScope, kEmpty, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotSetValueWhenKeyEmpty)
{
    EXPECT_THAT(store2->SetValue(kScope, kAppId, kEmpty, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotSetValueWhenNamespaceOversize)
{
    EXPECT_THAT(store2->SetValue(kScope, kOversize, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotSetValueWhenKeyOversize)
{
    EXPECT_THAT(store2->SetValue(kScope, kAppId, kOversize, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotSetValueWhenValueOversize)
{
    EXPECT_THAT(store2->SetValue(kScope, kAppId, kKey, kOversize, kNoTtl), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotGetValueWhenNamespaceUnknown)
{
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(kScope, kUnknown, kKey, value, ttl), Eq(WPEFramework::Core::ERROR_NOT_EXIST));
}

TEST_F(AStore2, DeletesKeyWhenNamespaceUnknown)
{
    EXPECT_THAT(store2->DeleteKey(kScope, kUnknown, kKey), Eq(WPEFramework::Core::ERROR_NONE));
}

TEST_F(AStore2, DeletesNamespaceWhenNamespaceUnknown)
{
    EXPECT_THAT(store2->DeleteNamespace(kScope, kUnknown), Eq(WPEFramework::Core::ERROR_NONE));
}

TEST_F(AStore2, SetsValueWhenValueEmpty)
{
    ASSERT_THAT(store2->SetValue(kScope, kAppId, kKey, kEmpty, kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    ASSERT_THAT(store2->GetValue(kScope, kAppId, kKey, value, ttl), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(kEmpty));
    EXPECT_THAT(ttl, Eq(kNoTtl));
}

TEST_F(AStore2, GetsValueWhenTtlNotExpired)
{
    ASSERT_THAT(store2->SetValue(kScope, kAppId, kKey, kValue, kTtl), Eq(WPEFramework::Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    ASSERT_THAT(store2->GetValue(kScope, kAppId, kKey, value, ttl), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(kValue));
    EXPECT_THAT(ttl, Le(kTtl));
    EXPECT_THAT(ttl, Gt(kNoTtl));
}

TEST_F(AStore2, DoesNotGetValueWhenTtlExpired)
{
    ASSERT_THAT(store2->SetValue(kScope, kAppId, kKey, kValue, kTtl), Eq(WPEFramework::Core::ERROR_NONE));
    WPEFramework::Core::Event lock(false, true);
    lock.Lock(kTtl * WPEFramework::Core::Time::MilliSecondsPerSecond);
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(kScope, kAppId, kKey, value, ttl), Eq(WPEFramework::Core::ERROR_UNKNOWN_KEY));
}

TEST_F(AStore2, ValueChangedWhenSetValue)
{
    class Store2Notification : public NiceMock<Store2NotificationMock> {
    public:
        Store2Notification()
        {
            EXPECT_CALL(*this, ValueChanged(_, _, _, _))
                .WillRepeatedly(Invoke(
                    [](const IStore2::ScopeType scope, const string& ns, const string& key, const string& value) {
                        EXPECT_THAT(scope, Eq(kScope));
                        EXPECT_THAT(ns, Eq(kAppId));
                        EXPECT_THAT(key, Eq(kKey));
                        EXPECT_THAT(value, Eq(kValue));
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    WPEFramework::Core::Sink<Store2Notification> sink;
    store2->Register(&sink);
    EXPECT_THAT(store2->SetValue(kScope, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    store2->Unregister(&sink);
}

TEST_F(AStore2, DoesNotGetValueWhenKeyUnknown)
{
    ASSERT_THAT(store2->SetValue(kScope, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(kScope, kAppId, kUnknown, value, ttl), Eq(WPEFramework::Core::ERROR_UNKNOWN_KEY));
}

TEST_F(AStore2, DeletesKeyWhenKeyUnknown)
{
    ASSERT_THAT(store2->SetValue(kScope, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->DeleteKey(kScope, kAppId, kUnknown), Eq(WPEFramework::Core::ERROR_NONE));
}

TEST_F(AStore2, DeletesKey)
{
    ASSERT_THAT(store2->SetValue(kScope, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->DeleteKey(kScope, kAppId, kKey), Eq(WPEFramework::Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(kScope, kAppId, kKey, value, ttl), Eq(WPEFramework::Core::ERROR_UNKNOWN_KEY));
}

TEST_F(AStore2, DeletesNamespace)
{
    ASSERT_THAT(store2->SetValue(kScope, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->DeleteNamespace(kScope, kAppId), Eq(WPEFramework::Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(kScope, kAppId, kKey, value, ttl), Eq(WPEFramework::Core::ERROR_NOT_EXIST));
}

TEST_F(AStore2, DoesNotSetValueWhenReachedMaxSize)
{
    ASSERT_THAT(store2->DeleteNamespace(kScope, kAppId), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(kScope, "8InMXXU4hM", "YWKN74ODMf", "N0ed2C2h4n", kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(kScope, "XhrICnuerw", "jPKODBDk5K", "d3BarkA5xF", kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(kScope, "WNeBknDDI2", "GC96ZN6Fuq", "IBF2E1MLQh", kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->SetValue(kScope, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
    EXPECT_THAT(store2->DeleteNamespace(kScope, "8InMXXU4hM"), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->DeleteNamespace(kScope, "XhrICnuerw"), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->DeleteNamespace(kScope, "WNeBknDDI2"), Eq(WPEFramework::Core::ERROR_NONE));
}
