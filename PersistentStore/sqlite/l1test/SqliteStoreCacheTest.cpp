#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../SqliteDb.h"
#include "../SqliteStoreCache.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;
using namespace WPEFramework::Exchange;

using ::testing::Eq;

const std::string Path = "/tmp/sqlite/l1test/sqlitestorecachetest";
const uint32_t MaxSize = 100;
const uint32_t MaxValue = 3;

TEST(AnSqliteStoreCache, FlushesCacheWithoutError)
{
    Core::File(Path).Destroy();

    auto db = Core::Service<SqliteDb>::Create<ISqliteDb>(Path, MaxSize, MaxValue);
    auto cache = Core::Service<SqliteStoreCache>::Create<IStoreCache>(db);

    ASSERT_THAT(db->Open(), Eq(Core::ERROR_NONE));

    EXPECT_THAT(cache->FlushCache(), Eq(Core::ERROR_NONE));

    cache->Release();
    db->Release();
}
