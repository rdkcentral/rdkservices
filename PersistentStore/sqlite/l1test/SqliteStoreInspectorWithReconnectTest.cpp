#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../SqliteDb.h"
#include "../SqliteStoreInspector.h"
#include "../SqliteStoreInspectorWithReconnect.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;
using namespace WPEFramework::Exchange;

using ::testing::Eq;

const std::string Path = "/tmp/sqlite/l1test/sqlitestoreinspectorwithreconnecttest";
const uint32_t MaxSize = 100;
const uint32_t MaxValue = 3;

TEST(AnSqliteStoreInspectorWithReconnect, SetsLimitAfterFileDestroy)
{
    Core::File(Path).Destroy();

    auto db = Core::Service<SqliteDb>::Create<ISqliteDb>(Path, MaxSize, MaxValue);
    auto inspector = Core::Service<SqliteStoreInspectorWithReconnect<SqliteStoreInspector>>::Create<IStoreInspector>(db);

    ASSERT_THAT(db->Open(), Eq(Core::ERROR_NONE));

    Core::File(Path).Destroy();

    EXPECT_THAT(inspector->SetNamespaceLimit("ns1", 20, IStoreInspector::ScopeType::DEVICE), Eq(Core::ERROR_NONE));
    uint32_t value;
    EXPECT_THAT(inspector->GetNamespaceLimit("ns1", IStoreInspector::ScopeType::DEVICE, value), Eq(Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(20));

    inspector->Release();
    db->Release();
}
