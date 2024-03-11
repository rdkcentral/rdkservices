#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Store2.h"
#include "../StoreInspector.h"

using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::NotNull;
using ::testing::Test;
using ::WPEFramework::Exchange::IStore2;
using ::WPEFramework::Exchange::IStoreInspector;
using ::WPEFramework::Plugin::Sqlite::Store2;
using ::WPEFramework::Plugin::Sqlite::StoreInspector;
using ::WPEFramework::RPC::IStringIterator;

const auto kPath = "/tmp/persistentstore/sqlite/l1test/storeinspectortest";
const auto kMaxSize = 100;
const auto kMaxValue = 10;
const auto kLimit = 50;
const auto kValue = "value_1";
const auto kKey = "key_1";
const auto kAppId = "app_id_1";
const auto kNoTtl = 0;
const auto kScope = IStoreInspector::ScopeType::DEVICE;
const auto kUnknown = "unknown";

class AStoreInspector : public Test {
protected:
    IStoreInspector* inspector;
    AStoreInspector()
        : inspector(WPEFramework::Core::Service<StoreInspector>::Create<IStoreInspector>(kPath))
    {
    }
    ~AStoreInspector() override
    {
        inspector->Release();
    }
};

TEST_F(AStoreInspector, GetsKeysWhenNamespaceUnknown)
{
    IStringIterator* it;
    ASSERT_THAT(inspector->GetKeys(kScope, kUnknown, it), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    string element;
    EXPECT_THAT(it->Next(element), IsFalse());
    it->Release();
}

TEST_F(AStoreInspector, GetsKeys)
{
    auto store2 = WPEFramework::Core::Service<Store2>::Create<IStore2>(kPath, kMaxSize, kMaxValue, kLimit);
    ASSERT_THAT(store2->SetValue(kScope, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    store2->Release();
    IStringIterator* it;
    ASSERT_THAT(inspector->GetKeys(kScope, kAppId, it), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    string element;
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element, Eq(kKey));
    EXPECT_THAT(it->Next(element), IsFalse());
    it->Release();
}

TEST_F(AStoreInspector, GetsNamespaces)
{
    auto store2 = WPEFramework::Core::Service<Store2>::Create<IStore2>(kPath, kMaxSize, kMaxValue, kLimit);
    ASSERT_THAT(store2->SetValue(kScope, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    store2->Release();
    IStringIterator* it;
    ASSERT_THAT(inspector->GetNamespaces(kScope, it), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    string element;
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element, Eq(kAppId));
    EXPECT_THAT(it->Next(element), IsFalse());
    it->Release();
}

TEST_F(AStoreInspector, GetsStorageSizes)
{
    auto store2 = WPEFramework::Core::Service<Store2>::Create<IStore2>(kPath, kMaxSize, kMaxValue, kLimit);
    ASSERT_THAT(store2->SetValue(kScope, kAppId, kKey, kValue, kNoTtl), Eq(WPEFramework::Core::ERROR_NONE));
    store2->Release();
    IStoreInspector::INamespaceSizeIterator* it;
    ASSERT_THAT(inspector->GetStorageSizes(kScope, it), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    IStoreInspector::NamespaceSize element;
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element.ns, Eq(kAppId));
    EXPECT_THAT(element.size, Eq(strlen(kKey) + strlen(kValue)));
    EXPECT_THAT(it->Next(element), IsFalse());
    it->Release();
}
