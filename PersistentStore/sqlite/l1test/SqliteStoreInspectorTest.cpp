#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../SqliteDb.h"
#include "../SqliteStore2.h"
#include "../SqliteStoreInspector.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;
using namespace WPEFramework::Exchange;

using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::NotNull;
using ::testing::Test;

const std::string Path = "/tmp/sqlite/l1test/sqlitestoreinspectortest";
const uint32_t MaxSize = 30;
const uint32_t MaxValue = 3;

class AnSqliteStoreInspector : public Test {
protected:
    ISqliteDb* db;
    IStore2* store;
    IStoreInspector* inspector;

    AnSqliteStoreInspector()
    {
        Core::File(Path).Destroy();
        db = Core::Service<SqliteDb>::Create<ISqliteDb>(Path, MaxSize, MaxValue);
        store = Core::Service<SqliteStore2>::Create<IStore2>(db);
        inspector = Core::Service<SqliteStoreInspector>::Create<IStoreInspector>(db);
    }
    virtual ~AnSqliteStoreInspector() override
    {
        db->Release();
        store->Release();
        inspector->Release();
    }
    virtual void SetUp()
    {
        ASSERT_THAT(db->Open(), Eq(Core::ERROR_NONE));
    }
};

TEST_F(AnSqliteStoreInspector, GetsKeysForUnknownNamespaceWithoutError)
{
    RPC::IStringIterator* it;
    EXPECT_THAT(inspector->GetKeys("x", IStoreInspector::ScopeType::DEVICE, it), Eq(Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    string element;
    EXPECT_THAT(it->Next(element), IsFalse());
    it->Release();
}

TEST_F(AnSqliteStoreInspector, SetsLimitForUnknownNamespaceWithoutError)
{
    EXPECT_THAT(inspector->SetNamespaceLimit("x", 10, IStoreInspector::ScopeType::DEVICE), Eq(Core::ERROR_NONE));
}

TEST_F(AnSqliteStoreInspector, GetsNamespacesWhenEmpty)
{
    RPC::IStringIterator* it;
    EXPECT_THAT(inspector->GetNamespaces(IStoreInspector::ScopeType::DEVICE, it), Eq(Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    string element;
    ASSERT_THAT(it->Next(element), IsFalse());
    it->Release();
}

TEST_F(AnSqliteStoreInspector, GetsNamespaceSizesWhenEmpty)
{
    Exchange::IStoreInspector::INamespaceSizeIterator* it;
    EXPECT_THAT(inspector->GetNamespaceSizes(IStoreInspector::ScopeType::DEVICE, it), Eq(Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    Exchange::IStoreInspector::NamespaceSize element;
    ASSERT_THAT(it->Next(element), IsFalse());
    it->Release();
}

TEST_F(AnSqliteStoreInspector, DoesNotGetLimitForUnknownNamespace)
{
    uint32_t value;
    EXPECT_THAT(inspector->GetNamespaceLimit("x", IStoreInspector::ScopeType::DEVICE, value), Eq(Core::ERROR_NOT_EXIST));
}

TEST_F(AnSqliteStoreInspector, DoesNotSetLimitForLargeNamespace)
{
    EXPECT_THAT(inspector->SetNamespaceLimit("ns111", 10, IStoreInspector::ScopeType::DEVICE), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AnSqliteStoreInspector, DoesNotSetLimitWhenReachesLimit)
{
    EXPECT_THAT(inspector->SetNamespaceLimit("ns1", 10, IStoreInspector::ScopeType::DEVICE), Eq(Core::ERROR_NONE));
    EXPECT_THAT(inspector->SetNamespaceLimit("ns2", 10, IStoreInspector::ScopeType::DEVICE), Eq(Core::ERROR_NONE));
    EXPECT_THAT(inspector->SetNamespaceLimit("ns3", 10, IStoreInspector::ScopeType::DEVICE), Eq(Core::ERROR_NONE));
    EXPECT_THAT(inspector->SetNamespaceLimit("ns4", 10, IStoreInspector::ScopeType::DEVICE), Eq(Core::ERROR_NONE));
    EXPECT_THAT(inspector->SetNamespaceLimit("ns5", 10, IStoreInspector::ScopeType::DEVICE), Eq(Core::ERROR_NONE));
    EXPECT_THAT(inspector->SetNamespaceLimit("ns6", 10, IStoreInspector::ScopeType::DEVICE), Eq(Core::ERROR_NONE));
    EXPECT_THAT(inspector->SetNamespaceLimit("ns7", 10, IStoreInspector::ScopeType::DEVICE), Eq(Core::ERROR_NONE));
    EXPECT_THAT(inspector->SetNamespaceLimit("ns8", 10, IStoreInspector::ScopeType::DEVICE), Eq(Core::ERROR_NONE));
    EXPECT_THAT(inspector->SetNamespaceLimit("ns9", 10, IStoreInspector::ScopeType::DEVICE), Eq(Core::ERROR_NONE));
    EXPECT_THAT(inspector->SetNamespaceLimit("abc", 10, IStoreInspector::ScopeType::DEVICE), Eq(Core::ERROR_NONE));

    EXPECT_THAT(inspector->SetNamespaceLimit("def", 10, IStoreInspector::ScopeType::DEVICE), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

class AnSqliteStoreInspectorWithValues : public AnSqliteStoreInspector {
protected:
    AnSqliteStoreInspectorWithValues()
        : AnSqliteStoreInspector()
    {
    }
    virtual ~AnSqliteStoreInspectorWithValues() override = default;
    virtual void SetUp()
    {
        AnSqliteStoreInspector::SetUp();
        ASSERT_THAT(store->SetValue("ns1", "k1", "v1", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_NONE));
        ASSERT_THAT(store->SetValue("ns1", "k2", "v2", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_NONE));
        ASSERT_THAT(store->SetValue("ns2", "k1", "v1", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_NONE));
    }
};

TEST_F(AnSqliteStoreInspectorWithValues, GetsKeys)
{
    RPC::IStringIterator* it;
    EXPECT_THAT(inspector->GetKeys("ns1", IStoreInspector::ScopeType::DEVICE, it), Eq(Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    string element;
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element, Eq("k1"));
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element, Eq("k2"));
    EXPECT_THAT(it->Next(element), IsFalse());
    it->Release();
}

TEST_F(AnSqliteStoreInspectorWithValues, GetsNamespaces)
{
    RPC::IStringIterator* it;
    EXPECT_THAT(inspector->GetNamespaces(IStoreInspector::ScopeType::DEVICE, it), Eq(Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    string element;
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element, Eq("ns1"));
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element, Eq("ns2"));
    EXPECT_THAT(it->Next(element), IsFalse());
    it->Release();
}

TEST_F(AnSqliteStoreInspectorWithValues, GetsNamespaceSizes)
{
    Exchange::IStoreInspector::INamespaceSizeIterator* it;
    EXPECT_THAT(inspector->GetNamespaceSizes(IStoreInspector::ScopeType::DEVICE, it), Eq(Core::ERROR_NONE));
    ASSERT_THAT(it, NotNull());
    Exchange::IStoreInspector::NamespaceSize element;
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element.ns, Eq("ns1"));
    EXPECT_THAT(element.size, Eq(8));
    ASSERT_THAT(it->Next(element), IsTrue());
    EXPECT_THAT(element.ns, Eq("ns2"));
    EXPECT_THAT(element.size, Eq(4));
    EXPECT_THAT(it->Next(element), IsFalse());
    it->Release();
}

class AnSqliteStoreInspectorWithValuesAndLimit : public AnSqliteStoreInspectorWithValues {
protected:
    AnSqliteStoreInspectorWithValuesAndLimit()
        : AnSqliteStoreInspectorWithValues()
    {
    }
    virtual ~AnSqliteStoreInspectorWithValuesAndLimit() override = default;
    virtual void SetUp() override
    {
        AnSqliteStoreInspectorWithValues::SetUp();
        ASSERT_THAT(inspector->SetNamespaceLimit("ns1", 10, IStoreInspector::ScopeType::DEVICE), Eq(Core::ERROR_NONE));
    }
};

TEST_F(AnSqliteStoreInspectorWithValuesAndLimit, GetsLimit)
{
    uint32_t value;
    EXPECT_THAT(inspector->GetNamespaceLimit("ns1", IStoreInspector::ScopeType::DEVICE, value), Eq(Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(10));
}

TEST_F(AnSqliteStoreInspectorWithValuesAndLimit, UpdatesLimit)
{
    ASSERT_THAT(inspector->SetNamespaceLimit("ns1", 20, IStoreInspector::ScopeType::DEVICE), Eq(Core::ERROR_NONE));
    uint32_t value;
    EXPECT_THAT(inspector->GetNamespaceLimit("ns1", IStoreInspector::ScopeType::DEVICE, value), Eq(Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(20));
}

TEST_F(AnSqliteStoreInspectorWithValuesAndLimit, DoesNotSetValueWhenReachesLimit)
{
    EXPECT_THAT(store->SetValue("ns1", "a", "b", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_NONE));

    EXPECT_THAT(store->SetValue("ns1", "c", "d", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AnSqliteStoreInspectorWithValuesAndLimit, SetsValueAfterReachesAndUpdatesLimit)
{
    EXPECT_THAT(store->SetValue("ns1", "a", "b", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_NONE));
    EXPECT_THAT(store->SetValue("ns1", "c", "d", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
    ASSERT_THAT(inspector->SetNamespaceLimit("ns1", 20, IStoreInspector::ScopeType::DEVICE), Eq(Core::ERROR_NONE));

    EXPECT_THAT(store->SetValue("ns1", "c", "d", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_NONE));
}
