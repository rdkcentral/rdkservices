#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Store2.h"
#include "Store2NotificationMock.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::Invoke;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::Le;
using ::testing::NiceMock;
using ::testing::NotNull;
using ::testing::Test;
using ::WPEFramework::Exchange::IStore2;
using ::WPEFramework::Exchange::IStoreCache;
using ::WPEFramework::Exchange::IStoreInspector;
using ::WPEFramework::Exchange::IStoreLimit;
using ::WPEFramework::Plugin::Sqlite::Store2;
using ::WPEFramework::RPC::IStringIterator;

const auto kPath = "/tmp/persistentstore/sqlite/l1test/store2test";
const auto kMaxSize = 100;
const auto kMaxValue = 10;
const auto kLimit = 50;
const auto kValue = "value_1";
const auto kKey = "key_1";
const auto kAppId = "app_id_1";
const auto kTtl = 2;
const auto kNoTtl = 0;
const auto kEmpty = "";
const auto kOversize = "this is too large";
const auto kUnknown = "unknown";
const auto kLimit20 = 20;
const auto kLimit30 = 30;
const auto kLimit40 = 40;

class AStore2 : public Test {
protected:
    WPEFramework::Core::ProxyType<Store2> store2;
    AStore2()
        : store2(WPEFramework::Core::ProxyType<Store2>::Create(kPath, kMaxSize, kMaxValue, kLimit))
    {
    }
};

TEST_F(AStore2, DoesNotSetValueWhenNamespaceEmpty)
{
    EXPECT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kEmpty, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotSetValueWhenKeyEmpty)
{
    EXPECT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kAppId, kEmpty, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotSetValueWhenNamespaceOversize)
{
    EXPECT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kOversize, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotSetValueWhenKeyOversize)
{
    EXPECT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kAppId, kOversize, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotSetValueWhenValueOversize)
{
    EXPECT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kAppId, kKey, kOversize, kNoTtl), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotGetValueWhenNamespaceUnknown)
{
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(IStore2::ScopeType::DEVICE, kUnknown, kKey, value, ttl), Eq(WPEFramework::Core::ERROR_NOT_EXIST));
}

TEST_F(AStore2, DeletesKeyWhenNamespaceUnknown)
{
    EXPECT_THAT(store2->DeleteKey(IStore2::ScopeType::DEVICE, kUnknown, kKey), Eq(WPEFramework::Core::ERROR_NONE));
}

TEST_F(AStore2, DeletesNamespaceWhenNamespaceUnknown)
{
    EXPECT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, kUnknown), Eq(WPEFramework::Core::ERROR_NONE));
}

TEST_F(AStore2, SetsValueWhenValueEmpty)
{
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kAppId, kKey, kEmpty, kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    ASSERT_THAT(store2->GetValue(IStore2::ScopeType::DEVICE, kAppId, kKey, value, ttl), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(kEmpty));
    EXPECT_THAT(ttl, Eq(kNoTtl));
}

TEST_F(AStore2, GetsValueWhenTtlNotExpired)
{
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kTtl), Eq(WPEFramework::Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    ASSERT_THAT(store2->GetValue(IStore2::ScopeType::DEVICE, kAppId, kKey, value, ttl), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(kValue));
    EXPECT_THAT(ttl, Le(kTtl));
    EXPECT_THAT(ttl, Gt(kNoTtl));
}

TEST_F(AStore2, DoesNotGetValueWhenTtlExpired)
{
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kTtl), Eq(WPEFramework::Core::ERROR_NONE));
    WPEFramework::Core::Event lock(false, true);
    lock.Lock(kTtl * WPEFramework::Core::Time::MilliSecondsPerSecond);
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(IStore2::ScopeType::DEVICE, kAppId, kKey, value, ttl), Eq(WPEFramework::Core::ERROR_UNKNOWN_KEY));
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
                        EXPECT_THAT(scope, Eq(IStore2::ScopeType::DEVICE));
                        EXPECT_THAT(ns, Eq(kAppId));
                        EXPECT_THAT(key, Eq(kKey));
                        EXPECT_THAT(value, Eq(kValue));
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    WPEFramework::Core::Sink<Store2Notification> sink;
    store2->Register(&sink);
    EXPECT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    store2->Unregister(&sink);
}

TEST_F(AStore2, DoesNotGetValueWhenKeyUnknown)
{
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(IStore2::ScopeType::DEVICE, kAppId, kUnknown, value, ttl), Eq(WPEFramework::Core::ERROR_UNKNOWN_KEY));
}

TEST_F(AStore2, DeletesKeyWhenKeyUnknown)
{
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->DeleteKey(IStore2::ScopeType::DEVICE, kAppId, kUnknown), Eq(WPEFramework::Core::ERROR_NONE));
}

TEST_F(AStore2, DeletesKey)
{
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->DeleteKey(IStore2::ScopeType::DEVICE, kAppId, kKey), Eq(WPEFramework::Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(IStore2::ScopeType::DEVICE, kAppId, kKey, value, ttl), Eq(WPEFramework::Core::ERROR_UNKNOWN_KEY));
}

TEST_F(AStore2, DeletesNamespace)
{
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, kAppId), Eq(WPEFramework::Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(IStore2::ScopeType::DEVICE, kAppId, kKey, value, ttl), Eq(WPEFramework::Core::ERROR_NOT_EXIST));
}

TEST_F(AStore2, DoesNotSetValueWhenReachedMaxSize)
{
    ASSERT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, kAppId), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, "8InMXXU4hM", "YWKN74ODMf", "N0ed2C2h4n", kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, "XhrICnuerw", "jPKODBDk5K", "d3BarkA5xF", kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, "WNeBknDDI2", "GC96ZN6Fuq", "IBF2E1MLQh", kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
    EXPECT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, "8InMXXU4hM"), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, "XhrICnuerw"), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, "WNeBknDDI2"), Eq(WPEFramework::Core::ERROR_NONE));
}

TEST_F(AStore2, FlushesCache)
{
    EXPECT_THAT(store2->FlushCache(), Eq(WPEFramework::Core::ERROR_NONE));
}

TEST_F(AStore2, GetsKeysWhenNamespaceUnknown)
{
    IStringIterator* it;
    ASSERT_THAT(store2->GetKeys(IStoreInspector::ScopeType::DEVICE, kUnknown, it), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    string element;
    EXPECT_THAT(it->Next(element), IsFalse());
    it->Release();
}

TEST_F(AStore2, GetsKeys)
{
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    IStringIterator* it;
    ASSERT_THAT(store2->GetKeys(IStoreInspector::ScopeType::DEVICE, kAppId, it), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    string element;
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element, Eq(kKey));
    EXPECT_THAT(it->Next(element), IsFalse());
    it->Release();
}

TEST_F(AStore2, GetsNamespaces)
{
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    IStringIterator* it;
    ASSERT_THAT(store2->GetNamespaces(IStoreInspector::ScopeType::DEVICE, it), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    string element;
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element, Eq(kAppId));
    EXPECT_THAT(it->Next(element), IsFalse());
    it->Release();
}

TEST_F(AStore2, GetsStorageSizes)
{
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    IStoreInspector::INamespaceSizeIterator* it;
    ASSERT_THAT(store2->GetStorageSizes(IStoreInspector::ScopeType::DEVICE, it), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    IStoreInspector::NamespaceSize element;
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element.ns, Eq(kAppId));
    EXPECT_THAT(element.size, Eq(strlen(kKey) + strlen(kValue)));
    EXPECT_THAT(it->Next(element), IsFalse());
    it->Release();
}

TEST_F(AStore2, DoesNotGetNamespaceStorageLimitWhenNamespaceUnknown)
{
    uint32_t value;
    EXPECT_THAT(store2->GetNamespaceStorageLimit(IStoreLimit::ScopeType::DEVICE, kUnknown, value), Eq(WPEFramework::Core::ERROR_NOT_EXIST));
}

TEST_F(AStore2, DoesNotSetNamespaceStorageLimitWhenNamespaceEmpty)
{
    EXPECT_THAT(store2->SetNamespaceStorageLimit(IStoreLimit::ScopeType::DEVICE, kEmpty, kLimit20), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotSetNamespaceStorageLimitWhenNamespaceOversize)
{
    EXPECT_THAT(store2->SetNamespaceStorageLimit(IStoreLimit::ScopeType::DEVICE, kOversize, kLimit20), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, SetsNamespaceStorageLimit)
{
    ASSERT_THAT(store2->SetNamespaceStorageLimit(IStoreLimit::ScopeType::DEVICE, kAppId, kLimit20), Eq(WPEFramework::Core::ERROR_NONE));
    uint32_t value;
    ASSERT_THAT(store2->GetNamespaceStorageLimit(IStoreLimit::ScopeType::DEVICE, kAppId, value), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(kLimit20));
}

TEST_F(AStore2, SetsNamespaceStorageLimitWhenAlreadySet)
{
    ASSERT_THAT(store2->SetNamespaceStorageLimit(IStoreLimit::ScopeType::DEVICE, kAppId, kLimit30), Eq(WPEFramework::Core::ERROR_NONE));
    uint32_t value;
    ASSERT_THAT(store2->GetNamespaceStorageLimit(IStoreLimit::ScopeType::DEVICE, kAppId, value), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(kLimit30));
    ASSERT_THAT(store2->SetNamespaceStorageLimit(IStoreLimit::ScopeType::DEVICE, kAppId, kLimit40), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->GetNamespaceStorageLimit(IStoreLimit::ScopeType::DEVICE, kAppId, value), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(kLimit40));
}

TEST_F(AStore2, DoesNotSetNamespaceStorageLimitWhenReachedMaxSize)
{
    ASSERT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, kAppId), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, "8InMXXU4hM", "YWKN74ODMf", "N0ed2C2h4n", kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, "XhrICnuerw", "jPKODBDk5K", "d3BarkA5xF", kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, "WNeBknDDI2", "GC96ZN6Fuq", "IBF2E1MLQh", kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, "V92", "R1R", "rHk", kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->SetNamespaceStorageLimit(IStoreLimit::ScopeType::DEVICE, kAppId, kLimit), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
    EXPECT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, "8InMXXU4hM"), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, "XhrICnuerw"), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, "WNeBknDDI2"), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, "V92"), Eq(WPEFramework::Core::ERROR_NONE));
}

TEST_F(AStore2, EnforcesSetValueToFailWhenReachedDefaultLimit)
{
    ASSERT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, kAppId), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kAppId, "YWKN74ODMf", "N0ed2C2h4n", kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kAppId, "jPKODBDk5K", "d3BarkA5xF", kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, EnforcesSetValueToFailWhenReachedLimit)
{
    ASSERT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, kAppId), Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->SetNamespaceStorageLimit(IStoreLimit::ScopeType::DEVICE, kAppId, kLimit20), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kAppId, "YWKN74ODMf", "N0ed2C2h4n", kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}
