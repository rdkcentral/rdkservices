#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Handle.h"
#include "../Store2Type.h"
#include "../StoreInspectorType.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;

using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::NotNull;
using ::testing::Test;

const std::string Path = "/tmp/sqlite/l1test/storeinspectortest";
const uint32_t MaxSize = 100;
const uint32_t MaxValue = 10;
const uint32_t Limit = 50;

class AStoreInspector : public Test {
protected:
    Exchange::IStoreInspector* inspector;
    ~AStoreInspector() override = default;
    void SetUp() override
    {
        Core::File(Path).Destroy();
        // File is destroyed

        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_PATH"), Path);
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_MAXSIZE"), std::to_string(MaxSize));
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_MAXVALUE"), std::to_string(MaxValue));
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_LIMIT"), std::to_string(Limit));
        inspector = Core::Service<Sqlite::StoreInspectorType<Sqlite::Handle>>::Create<Exchange::IStoreInspector>();
    }
    void TearDown() override
    {
        inspector->Release();
    }
};

TEST_F(AStoreInspector, GetsKeysForUnknownNamespaceWithoutError)
{
    RPC::IStringIterator* it;
    ASSERT_THAT(inspector->GetKeys(Exchange::IStore2::ScopeType::DEVICE, "x", it), Eq(Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    string element;
    EXPECT_THAT(it->Next(element), IsFalse());
    it->Release();
}

TEST_F(AStoreInspector, GetsNamespacesWhenEmptyWithoutError)
{
    RPC::IStringIterator* it;
    ASSERT_THAT(inspector->GetNamespaces(Exchange::IStore2::ScopeType::DEVICE, it), Eq(Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    string element;
    ASSERT_THAT(it->Next(element), IsFalse());
    it->Release();
}

TEST_F(AStoreInspector, GetsStorageSizesWhenEmptyWithoutError)
{
    Exchange::IStoreInspector::INamespaceSizeIterator* it;
    ASSERT_THAT(inspector->GetStorageSizes(Exchange::IStore2::ScopeType::DEVICE, it), Eq(Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    Exchange::IStoreInspector::NamespaceSize element;
    ASSERT_THAT(it->Next(element), IsFalse());
    it->Release();
}

class AStoreInspectorWithValues : public AStoreInspector {
protected:
    ~AStoreInspectorWithValues() override = default;
    void SetUp() override
    {
        AStoreInspector::SetUp();
        auto store = Core::Service<Sqlite::Store2Type<Sqlite::Handle>>::Create<Exchange::IStore2>();
        ASSERT_THAT(store->SetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key1", "value1", 0), Eq(Core::ERROR_NONE));
        ASSERT_THAT(store->SetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key2", "value2", 0), Eq(Core::ERROR_NONE));
        ASSERT_THAT(store->SetValue(Exchange::IStore2::ScopeType::DEVICE, "ns2", "key1", "value1", 0), Eq(Core::ERROR_NONE));
        store->Release();
    }
};

TEST_F(AStoreInspectorWithValues, GetsKeys)
{
    RPC::IStringIterator* it;
    ASSERT_THAT(inspector->GetKeys(Exchange::IStore2::ScopeType::DEVICE, "ns1", it), Eq(Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    string element;
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element, Eq("key1"));
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element, Eq("key2"));
    EXPECT_THAT(it->Next(element), IsFalse());
    it->Release();
}

TEST_F(AStoreInspectorWithValues, GetsNamespaces)
{
    RPC::IStringIterator* it;
    ASSERT_THAT(inspector->GetNamespaces(Exchange::IStore2::ScopeType::DEVICE, it), Eq(Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    string element;
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element, Eq("ns1"));
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element, Eq("ns2"));
    EXPECT_THAT(it->Next(element), IsFalse());
    it->Release();
}

TEST_F(AStoreInspectorWithValues, GetsStorageSizes)
{
    Exchange::IStoreInspector::INamespaceSizeIterator* it;
    ASSERT_THAT(inspector->GetStorageSizes(Exchange::IStore2::ScopeType::DEVICE, it), Eq(Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    Exchange::IStoreInspector::NamespaceSize element;
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element.ns, Eq("ns1"));
    EXPECT_THAT(element.size, Eq(20));
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element.ns, Eq("ns2"));
    EXPECT_THAT(element.size, Eq(10));
    EXPECT_THAT(it->Next(element), IsFalse());
    it->Release();
}
