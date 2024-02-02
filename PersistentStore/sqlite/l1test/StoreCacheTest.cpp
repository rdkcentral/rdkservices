#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../StoreCache.h"

using ::testing::Eq;
using ::testing::Test;
using ::WPEFramework::Exchange::IStoreCache;
using ::WPEFramework::Plugin::Sqlite::StoreCache;

const auto kPath = "/tmp/persistentstore/sqlite/l1test/storecachetest";

class AStoreCache : public Test {
protected:
    IStoreCache* cache;
    AStoreCache()
        : cache(WPEFramework::Core::Service<StoreCache>::Create<IStoreCache>(kPath))
    {
    }
    ~AStoreCache() override
    {
        cache->Release();
    }
};

TEST_F(AStoreCache, FlushesCache)
{
    EXPECT_THAT(cache->FlushCache(), Eq(WPEFramework::Core::ERROR_NONE));
}
