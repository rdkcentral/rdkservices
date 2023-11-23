#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../SqliteDb.h"
#include "../SqliteStore2.h"
#include "../SqliteStore2WithReconnect.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;
using namespace WPEFramework::Exchange;

using ::testing::Eq;

const std::string Path = "/tmp/sqlite/l1test/sqlitestore2withreconnecttest";
const uint32_t MaxSize = 100;
const uint32_t MaxValue = 3;

TEST(AnSqliteStore2WithReconnect, SetsValueAfterFileDestroy)
{
    Core::File(Path).Destroy();

    auto db = Core::Service<SqliteDb>::Create<ISqliteDb>(Path, MaxSize, MaxValue);
    auto store = Core::Service<SqliteStore2WithReconnect<SqliteStore2>>::Create<IStore2>(db);

    ASSERT_THAT(db->Open(), Eq(Core::ERROR_NONE));

    ASSERT_THAT(store->SetValue("ns1", "k1", "v1", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_NONE));

    Core::File(Path).Destroy();

    EXPECT_THAT(store->SetValue("ns1", "k2", "v2", IStore2::ScopeType::DEVICE, 0), Eq(Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    EXPECT_THAT(store->GetValue("ns1", "k1", IStore2::ScopeType::DEVICE, value, ttl), Eq(Core::ERROR_UNKNOWN_KEY));
    EXPECT_THAT(store->GetValue("ns1", "k2", IStore2::ScopeType::DEVICE, value, ttl), Eq(Core::ERROR_NONE));
    EXPECT_THAT(value, Eq("v2"));
    EXPECT_THAT(ttl, Eq(0));

    store->Release();
    db->Release();
}
