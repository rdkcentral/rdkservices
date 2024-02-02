#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Store2.h"
#include "../StoreLimit.h"

using ::testing::Eq;
using ::testing::Test;
using ::WPEFramework::Exchange::IStore2;
using ::WPEFramework::Exchange::IStoreLimit;
using ::WPEFramework::Plugin::Sqlite::Store2;
using ::WPEFramework::Plugin::Sqlite::StoreLimit;

const auto kPath = "/tmp/persistentstore/sqlite/l1test/storelimittest";
const auto kMaxSize = 100;
const auto kMaxValue = 10;
const auto kLimit = 50;
const auto kValue = "value_1";
const auto kKey = "key_1";
const auto kAppId = "app_id_1";
const auto kNoTtl = 0;
const auto kScope = IStoreLimit::ScopeType::DEVICE;
const auto kEmpty = "";
const auto kOversize = "this is too large";
const auto kUnknown = "unknown";
const auto kLimit20 = 20;
const auto kLimit30 = 30;
const auto kLimit40 = 40;

class AStoreLimit : public Test {
protected:
    IStoreLimit* limit;
    AStoreLimit()
        : limit(WPEFramework::Core::Service<StoreLimit>::Create<IStoreLimit>(kPath, kMaxSize, kMaxValue))
    {
    }
    ~AStoreLimit() override
    {
        limit->Release();
    }
};

TEST_F(AStoreLimit, DoesNotGetNamespaceStorageLimitWhenNamespaceUnknown)
{
    uint32_t value;
    EXPECT_THAT(limit->GetNamespaceStorageLimit(kScope, kUnknown, value), Eq(WPEFramework::Core::ERROR_NOT_EXIST));
}

TEST_F(AStoreLimit, DoesNotSetNamespaceStorageLimitWhenNamespaceEmpty)
{
    EXPECT_THAT(limit->SetNamespaceStorageLimit(kScope, kEmpty, kLimit20), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStoreLimit, DoesNotSetNamespaceStorageLimitWhenNamespaceOversize)
{
    EXPECT_THAT(limit->SetNamespaceStorageLimit(kScope, kOversize, kLimit20), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStoreLimit, SetsNamespaceStorageLimit)
{
    ASSERT_THAT(limit->SetNamespaceStorageLimit(kScope, kAppId, kLimit20), Eq(WPEFramework::Core::ERROR_NONE));
    uint32_t value;
    ASSERT_THAT(limit->GetNamespaceStorageLimit(kScope, kAppId, value), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(kLimit20));
}

TEST_F(AStoreLimit, SetsNamespaceStorageLimitWhenAlreadySet)
{
    ASSERT_THAT(limit->SetNamespaceStorageLimit(kScope, kAppId, kLimit30), Eq(WPEFramework::Core::ERROR_NONE));
    uint32_t value;
    ASSERT_THAT(limit->GetNamespaceStorageLimit(kScope, kAppId, value), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(kLimit30));
    ASSERT_THAT(limit->SetNamespaceStorageLimit(kScope, kAppId, kLimit40), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(limit->GetNamespaceStorageLimit(kScope, kAppId, value), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(kLimit40));
}

TEST_F(AStoreLimit, DoesNotSetNamespaceStorageLimitWhenReachedMaxSize)
{
    auto store2 = WPEFramework::Core::Service<Store2>::Create<IStore2>(kPath, kMaxSize, kMaxValue, kLimit);
    ASSERT_THAT(store2->DeleteNamespace(kScope, kAppId), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(kScope, "8InMXXU4hM", "YWKN74ODMf", "N0ed2C2h4n", kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(kScope, "XhrICnuerw", "jPKODBDk5K", "d3BarkA5xF", kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(kScope, "WNeBknDDI2", "GC96ZN6Fuq", "IBF2E1MLQh", kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(kScope, "V92", "R1R", "rHk", kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(limit->SetNamespaceStorageLimit(kScope, kAppId, kLimit), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
    EXPECT_THAT(store2->DeleteNamespace(kScope, "8InMXXU4hM"), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->DeleteNamespace(kScope, "XhrICnuerw"), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->DeleteNamespace(kScope, "WNeBknDDI2"), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->DeleteNamespace(kScope, "V92"), Eq(WPEFramework::Core::ERROR_NONE));
    store2->Release();
}

TEST_F(AStoreLimit, EnforcesSetValueToFailWhenReachedDefaultLimit)
{
    auto store2 = WPEFramework::Core::Service<Store2>::Create<IStore2>(kPath, kMaxSize, kMaxValue, kLimit);
    ASSERT_THAT(store2->DeleteNamespace(kScope, kAppId), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(kScope, kAppId, "YWKN74ODMf", "N0ed2C2h4n", kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(kScope, kAppId, "jPKODBDk5K", "d3BarkA5xF", kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(kScope, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
    store2->Release();
}

TEST_F(AStoreLimit, EnforcesSetValueToFailWhenReachedLimit)
{
    auto store2 = WPEFramework::Core::Service<Store2>::Create<IStore2>(kPath, kMaxSize, kMaxValue, kLimit);
    ASSERT_THAT(store2->DeleteNamespace(kScope, kAppId), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(limit->SetNamespaceStorageLimit(kScope, kAppId, kLimit20), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(kScope, kAppId, "YWKN74ODMf", "N0ed2C2h4n", kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(kScope, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
    store2->Release();
}
