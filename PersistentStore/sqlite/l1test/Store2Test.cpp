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
using ::WPEFramework::Exchange::IStoreInspector;
using ::WPEFramework::Exchange::IStoreLimit;
using ::WPEFramework::Plugin::Sqlite::Store2;
using ::WPEFramework::RPC::IStringIterator;

const auto kPath = "/tmp/persistentstore/sqlite/l1test/store2test";
const auto kMaxSize = 100;
const auto kMaxValue = 5;
const auto kLimit = 50;
const auto kNewLimit = 10;
const auto kValue = "value";
const auto kKey = "key";
const auto kAppId = "app";
const auto kTtl = 2;
const auto kNoTtl = 0;

class AStore2 : public Test {
protected:
    WPEFramework::Core::ProxyType<Store2> store2;

    void SetUp() override
    {
        // Create a WorkerPool with multiple threads
        _dispatcher = new WorkerPoolDispatcher();
        _workerPool = new WPEFramework::Core::WorkerPool(
            1, // threadCount
            WPEFramework::Core::Thread::DefaultStackSize(),
            2, // queueSize
            _dispatcher,
            nullptr);

        WPEFramework::Core::IWorkerPool::Assign(_workerPool);
        _workerPool->Run();

        store2 = WPEFramework::Core::ProxyType<Store2>::Create(
            kPath, kMaxSize, kMaxValue, kLimit);
    }
    void TearDown() override
    {
        if (_workerPool != nullptr) {
            _workerPool->Stop();
            WPEFramework::Core::IWorkerPool::Assign(nullptr);
            delete _workerPool;
            _workerPool = nullptr;
        }

        if (_dispatcher != nullptr) {
            delete _dispatcher;
            _dispatcher = nullptr;
        }
    }

    // Simple dispatcher for testing
    class WorkerPoolDispatcher
        : public WPEFramework::Core::ThreadPool::IDispatcher {
    public:
        WorkerPoolDispatcher() = default;
        ~WorkerPoolDispatcher() override = default;

        void Initialize() override {}
        void Deinitialize() override {}
        void Dispatch(WPEFramework::Core::IDispatch* job) override
        {
            job->Dispatch();
        }
    };

    WorkerPoolDispatcher* _dispatcher;
    WPEFramework::Core::WorkerPool* _workerPool;
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

TEST_F(AStore2, GetsValueWithTtl)
{
    ASSERT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kTtl),
        Eq(WPEFramework::Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    ASSERT_THAT(store2->GetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, value, ttl),
        Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(kValue));
    EXPECT_THAT(ttl, Le(kTtl));
    EXPECT_THAT(ttl, Gt(0));
}

TEST_F(AStore2, DoesNotGetValueWhenTtlExpired)
{
    ASSERT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kTtl),
        Eq(WPEFramework::Core::ERROR_NONE));
    WPEFramework::Core::Event lock(false, true);
    lock.Lock(kTtl * WPEFramework::Core::Time::MilliSecondsPerSecond);
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
    lock.Lock();
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

TEST_F(AStore2, DoesNotSetValueWhenReachedMaxSize)
{
    store2 = WPEFramework::Core::ProxyType<Store2>::Create(
        kPath, kNewLimit, kMaxValue, kLimit);
    ASSERT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, kAppId),
        Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl),
        Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
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
                    IStoreLimit::ScopeType::DEVICE, "", kNewLimit),
        Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotSetNamespaceStorageLimitWhenNamespaceTooLarge)
{
    EXPECT_THAT(store2->SetNamespaceStorageLimit(
                    IStoreLimit::ScopeType::DEVICE, "this is too large", kNewLimit),
        Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, SetsNamespaceStorageLimit)
{
    ASSERT_THAT(store2->SetNamespaceStorageLimit(
                    IStoreLimit::ScopeType::DEVICE, kAppId, kNewLimit),
        Eq(WPEFramework::Core::ERROR_NONE));
    uint32_t value;
    ASSERT_THAT(store2->GetNamespaceStorageLimit(
                    IStoreLimit::ScopeType::DEVICE, kAppId, value),
        Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(kNewLimit));
    // restore:
    EXPECT_THAT(store2->SetNamespaceStorageLimit(
                    IStoreLimit::ScopeType::DEVICE, kAppId, kLimit),
        Eq(WPEFramework::Core::ERROR_NONE));
}

TEST_F(AStore2, DoesNotSetValueWhenReachedDefaultLimit)
{
    store2 = WPEFramework::Core::ProxyType<Store2>::Create(
        kPath, kMaxSize, kMaxValue, kMaxValue);
    ASSERT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, kAppId),
        Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl),
        Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStore2, DoesNotSetValueWhenReachedLimit)
{
    ASSERT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, kAppId),
        Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetNamespaceStorageLimit(
                    IStoreLimit::ScopeType::DEVICE, kAppId, kMaxValue),
        Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, kValue, kNoTtl),
        Eq(WPEFramework::Core::ERROR_INVALID_INPUT_LENGTH));
    // restore:
    EXPECT_THAT(store2->SetNamespaceStorageLimit(
                    IStoreLimit::ScopeType::DEVICE, kAppId, kLimit),
        Eq(WPEFramework::Core::ERROR_NONE));
}

TEST_F(AStore2, SetsValueWhenDoesNotReachLimit)
{
    ASSERT_THAT(store2->DeleteNamespace(IStore2::ScopeType::DEVICE, kAppId),
        Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(store2->SetNamespaceStorageLimit(
                    IStoreLimit::ScopeType::DEVICE, kAppId, kMaxValue),
        Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(store2->SetValue(
                    IStore2::ScopeType::DEVICE, kAppId, kKey, "", kNoTtl),
        Eq(WPEFramework::Core::ERROR_NONE));
    // restore:
    EXPECT_THAT(store2->SetNamespaceStorageLimit(
                    IStoreLimit::ScopeType::DEVICE, kAppId, kLimit),
        Eq(WPEFramework::Core::ERROR_NONE));
}
