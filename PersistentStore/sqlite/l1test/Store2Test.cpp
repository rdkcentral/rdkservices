#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Store2.h"
#include "Store2NotificationMock.h"
#include "WorkerPoolImplementation.h"

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
using ::WPEFramework::Exchange::IStoreInspector;
using ::WPEFramework::Exchange::IStoreLimit;
using ::WPEFramework::Plugin::Sqlite::Store2;
using ::WPEFramework::RPC::IStringIterator;

const auto kPath = "/tmp/persistentstore/sqlite/l1test/store2test";
const auto kPathCorrupt = "/tmp/persistentstore/sqlite/l1test/corrupt";
const auto kMaxSize = 100;
const auto kMaxValue = 5;
const auto kLimit = 50;
const auto kValue = "value";
const auto kKey = "key";
const auto kAppId = "app";
const auto kNoTtl = 0;

class AStore2 : public Test {
protected:
    WPEFramework::Core::ProxyType<WorkerPoolImplementation> workerPool;
    WPEFramework::Core::ProxyType<Store2> store2;
    AStore2()
        : workerPool(WPEFramework::Core::ProxyType<WorkerPoolImplementation>::Create(
              WPEFramework::Core::Thread::DefaultStackSize()))
        , store2(WPEFramework::Core::ProxyType<Store2>::Create(
              kPath, kMaxSize, kMaxValue, kLimit))
    {
        WPEFramework::Core::IWorkerPool::Assign(&(*workerPool));
    }
    ~AStore2() override
    {
        WPEFramework::Core::IWorkerPool::Assign(nullptr);
    }
};

TEST_F(AStore2, DoesNotSetValueWhenNamespaceEmpty)
{
    EXPECT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, "", kKey, kValue, kNoTtl),
        Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotSetValueWhenKeyEmpty)
{
    EXPECT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, "", kValue, kNoTtl),
        Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotSetValueWhenNamespaceTooLarge)
{
    EXPECT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, "this is too large",
                    kKey, kValue, kNoTtl),
        Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotSetValueWhenKeyTooLarge)
{
    EXPECT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, "this is too large",
                    kValue, kNoTtl),
        Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotSetValueWhenValueTooLarge)
{
    EXPECT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey,
                    "this is too large", kNoTtl),
        Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotGetValueWhenNamespaceDoesNotExist)
{
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(
                    IStore2::ScopeType::DEVICE, "none", kKey, value, ttl),
        Eq(WPEFramework::Core::ERROR_NOT_EXIST));
}

TEST_F(AStore2, DeletesKeyWhenNamespaceDoesNotExist)
{
    EXPECT_THAT(store2->DeleteKey(IStore2::ScopeType::DEVICE, "none", kKey),
        Eq(WPEFramework::Core::ERROR_NONE));
}

TEST_F(AStore2, DeletesNamespaceWhenNamespaceDoesNotExist)
{
    EXPECT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, "none"),
        Eq(WPEFramework::Core::ERROR_NONE));
}

TEST_F(AStore2, SetsValueWhenValueEmpty)
{
    ASSERT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, "", kNoTtl),
        Eq(WPEFramework::Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    ASSERT_THAT(store2->GetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, value, ttl),
        Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(""));
}

TEST_F(AStore2, GetsValueWhenTtl2Seconds)
{
    ASSERT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, 2 /*ttl*/),
        Eq(WPEFramework::Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    ASSERT_THAT(store2->GetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, value, ttl),
        Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(kValue));
    EXPECT_THAT(ttl, Le(2));
    EXPECT_THAT(ttl, Gt(0));
}

TEST_F(AStore2, DoesNotGetValueWhenTtl2SecondsExpired)
{
    ASSERT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, 2),
        Eq(WPEFramework::Core::ERROR_NONE));
    WPEFramework::Core::Event lock(false, true);
    lock.Lock(2 * WPEFramework::Core::Time::MilliSecondsPerSecond);
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, value, ttl),
        Eq(WPEFramework::Core::ERROR_UNKNOWN_KEY));
}

TEST_F(AStore2, SendsValueChangedEventWhenSetValue)
{
    IStore2::ScopeType eventScope;
    string eventNamespace;
    string eventKey;
    string eventValue;
    WPEFramework::Core::Event lock(false, true);
    WPEFramework::Core::Sink<NiceMock<Store2NotificationMock>> sink;
    EXPECT_CALL(sink, ValueChanged(_, _, _, _))
        .WillRepeatedly(Invoke(
            [&](const IStore2::ScopeType scope, const string& ns,
                const string& key, const string& value) {
                eventScope = scope;
                eventNamespace = ns;
                eventKey = key;
                eventValue = value;
                lock.SetEvent();
                return WPEFramework::Core::ERROR_NONE;
            }));
    store2->Register(&sink);
    EXPECT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl),
        Eq(WPEFramework::Core::ERROR_NONE));
    lock.Lock(2 * WPEFramework::Core::Time::MilliSecondsPerSecond);
    EXPECT_THAT(eventScope, Eq(IStore2::ScopeType::DEVICE));
    EXPECT_THAT(eventNamespace, Eq(kAppId));
    EXPECT_THAT(eventKey, Eq(kKey));
    EXPECT_THAT(eventValue, Eq(kValue));
    store2->Unregister(&sink);
}

TEST_F(AStore2, DoesNotGetValueWhenKeyDoesNotExist)
{
    ASSERT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl),
        Eq(WPEFramework::Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(
                    IStore2::ScopeType::DEVICE, kAppId, "none", value, ttl),
        Eq(WPEFramework::Core::ERROR_UNKNOWN_KEY));
}

TEST_F(AStore2, DeletesKeyWhenKeyDoesNotExist)
{
    ASSERT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl),
        Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->DeleteKey(
                    IStore2::ScopeType::DEVICE, kAppId, "none"),
        Eq(WPEFramework::Core::ERROR_NONE));
}

TEST_F(AStore2, DoesNotGetValueWhenDeletedKey)
{
    ASSERT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl),
        Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->DeleteKey(
                    IStore2::ScopeType::DEVICE, kAppId, kKey),
        Eq(WPEFramework::Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, value, ttl),
        Eq(WPEFramework::Core::ERROR_UNKNOWN_KEY));
}

TEST_F(AStore2, DoesNotGetValueWhenDeletedNamespace)
{
    ASSERT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl),
        Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, kAppId),
        Eq(WPEFramework::Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, value, ttl),
        Eq(WPEFramework::Core::ERROR_NOT_EXIST));
}

TEST(Store2, DoesNotSetValueWhenReachedMaxSize)
{
    auto workerPool = WPEFramework::Core::ProxyType<WorkerPoolImplementation>::Create(
        WPEFramework::Core::Thread::DefaultStackSize());
    auto store2 = WPEFramework::Core::ProxyType<Store2>::Create(
        kPath, 10 /*max size*/, kMaxValue, kLimit);
    WPEFramework::Core::IWorkerPool::Assign(&(*workerPool));
    ASSERT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, kAppId),
        Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl),
        Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
    WPEFramework::Core::IWorkerPool::Assign(nullptr);
}

TEST_F(AStore2, FlushesCache)
{
    EXPECT_THAT(store2->FlushCache(), Eq(WPEFramework::Core::ERROR_NONE));
}

TEST_F(AStore2, GetsKeysWhenNamespaceDoesNotExist)
{
    IStringIterator* it;
    ASSERT_THAT(store2->GetKeys(
                    IStoreInspector::ScopeType::DEVICE, "none", it),
        Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    string element;
    EXPECT_THAT(it->Next(element), IsFalse());
    it->Release();
}

TEST_F(AStore2, GetsKeys)
{
    ASSERT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl),
        Eq(WPEFramework::Core::ERROR_NONE));
    IStringIterator* it;
    ASSERT_THAT(store2->GetKeys(
                    IStoreInspector::ScopeType::DEVICE, kAppId, it),
        Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    string element;
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element, Eq(kKey));
    EXPECT_THAT(it->Next(element), IsFalse());
    it->Release();
}

TEST_F(AStore2, GetsNamespaces)
{
    ASSERT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl),
        Eq(WPEFramework::Core::ERROR_NONE));
    IStringIterator* it;
    ASSERT_THAT(store2->GetNamespaces(
                    IStoreInspector::ScopeType::DEVICE, it),
        Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    string element;
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element, Eq(kAppId));
    EXPECT_THAT(it->Next(element), IsFalse());
    it->Release();
}

TEST_F(AStore2, GetsStorageSizes)
{
    ASSERT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl),
        Eq(WPEFramework::Core::ERROR_NONE));
    IStoreInspector::INamespaceSizeIterator* it;
    ASSERT_THAT(store2->GetStorageSizes(
                    IStoreInspector::ScopeType::DEVICE, it),
        Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    IStoreInspector::NamespaceSize element;
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element.ns, Eq(kAppId));
    EXPECT_THAT(element.size, Eq(strlen(kKey) + strlen(kValue)));
    EXPECT_THAT(it->Next(element), IsFalse());
    it->Release();
}

TEST_F(AStore2, DoesNotGetNamespaceStorageLimitWhenNamespaceDoesNotExist)
{
    uint32_t value;
    EXPECT_THAT(store2->GetNamespaceStorageLimit(
                    IStoreLimit::ScopeType::DEVICE, "none", value),
        Eq(WPEFramework::Core::ERROR_NOT_EXIST));
}

TEST_F(AStore2, DoesNotSetNamespaceStorageLimitWhenNamespaceEmpty)
{
    EXPECT_THAT(store2->SetNamespaceStorageLimit(
                    IStoreLimit::ScopeType::DEVICE, "", 10 /*limit*/),
        Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotSetNamespaceStorageLimitWhenNamespaceTooLarge)
{
    EXPECT_THAT(store2->SetNamespaceStorageLimit(
                    IStoreLimit::ScopeType::DEVICE, "this is too large", 10 /*limit*/),
        Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, SetsNamespaceStorageLimit)
{
    ASSERT_THAT(store2->SetNamespaceStorageLimit(
                    IStoreLimit::ScopeType::DEVICE, kAppId, 10 /*limit*/),
        Eq(WPEFramework::Core::ERROR_NONE));
    uint32_t value;
    ASSERT_THAT(store2->GetNamespaceStorageLimit(
                    IStoreLimit::ScopeType::DEVICE, kAppId, value),
        Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(10));
}

TEST_F(AStore2, UpdatesNamespaceStorageLimit)
{
    ASSERT_THAT(store2->SetNamespaceStorageLimit(
                    IStoreLimit::ScopeType::DEVICE, kAppId, 10 /*limit*/),
        Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetNamespaceStorageLimit(
                    IStoreLimit::ScopeType::DEVICE, kAppId, 20 /*limit*/),
        Eq(WPEFramework::Core::ERROR_NONE));
    uint32_t value;
    ASSERT_THAT(store2->GetNamespaceStorageLimit(
                    IStoreLimit::ScopeType::DEVICE, kAppId, value),
        Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(20));
}

TEST(Store2, DoesNotSetValueWhenReachedDefaultLimit)
{
    auto workerPool = WPEFramework::Core::ProxyType<WorkerPoolImplementation>::Create(
        WPEFramework::Core::Thread::DefaultStackSize());
    auto store2 = WPEFramework::Core::ProxyType<Store2>::Create(
        kPath, kMaxSize, kMaxValue, 5 /*limit*/);
    WPEFramework::Core::IWorkerPool::Assign(&(*workerPool));
    ASSERT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, kAppId),
        Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl),
        Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
    WPEFramework::Core::IWorkerPool::Assign(nullptr);
}

TEST_F(AStore2, DoesNotSetValueWhenReachedLimit)
{
    ASSERT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, kAppId),
        Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetNamespaceStorageLimit(
                    IStoreLimit::ScopeType::DEVICE, kAppId, 5 /*limit*/),
        Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl),
        Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, SetsValueWhenDoesNotReachLimit)
{
    ASSERT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, kAppId),
        Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetNamespaceStorageLimit(
                    IStoreLimit::ScopeType::DEVICE, kAppId, 5 /*limit*/),
        Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, "", kNoTtl),
        Eq(WPEFramework::Core::ERROR_NONE));
}

TEST(Store2, SetsValueWhenFileIsNotDatabase)
{
    {
        WPEFramework::Core::File file(kPathCorrupt);
        file.Destroy();
        WPEFramework::Core::Directory(file.PathName().c_str()).CreatePath();
        ASSERT_THAT(file.Create(), IsTrue());
        uint8_t buffer[1024];
        ASSERT_THAT(file.Write(buffer, 1024), Eq(1024));
    }
    auto workerPool = WPEFramework::Core::ProxyType<WorkerPoolImplementation>::Create(
        WPEFramework::Core::Thread::DefaultStackSize());
    auto store2 = WPEFramework::Core::ProxyType<Store2>::Create(
        kPathCorrupt, kMaxSize, kMaxValue, kLimit);
    WPEFramework::Core::IWorkerPool::Assign(&(*workerPool));
    EXPECT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl),
        Eq(WPEFramework::Core::ERROR_NONE));
    WPEFramework::Core::IWorkerPool::Assign(nullptr);
}

TEST(Store2, SetsValueWhenFileCorrupt)
{
    {
        WPEFramework::Core::ProxyType<Store2>::Create(
            kPathCorrupt, kMaxSize, kMaxValue, kLimit);
    }
    {
        WPEFramework::Core::File file(kPathCorrupt);
        ASSERT_THAT(file.Open(false /*readOnly*/), IsTrue());
        ASSERT_THAT(file.Position(false /*relative*/, 8192), IsTrue());
        uint8_t buffer[1024];
        ASSERT_THAT(file.Write(buffer, 1024), Eq(1024));
    }
    auto workerPool = WPEFramework::Core::ProxyType<WorkerPoolImplementation>::Create(
        WPEFramework::Core::Thread::DefaultStackSize());
    auto store2 = WPEFramework::Core::ProxyType<Store2>::Create(
        kPathCorrupt, kMaxSize, kMaxValue, kLimit);
    WPEFramework::Core::IWorkerPool::Assign(&(*workerPool));
    EXPECT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl),
        Eq(WPEFramework::Core::ERROR_NONE));
    WPEFramework::Core::IWorkerPool::Assign(nullptr);
}
