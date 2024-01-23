#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Handle.h"
#include "../StoreCacheType.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;

using ::testing::Eq;
using ::testing::Test;

const std::string Path = "/tmp/sqlite/l1test/storecachetest";
const uint32_t MaxSize = 100;
const uint32_t MaxValue = 10;
const uint32_t Limit = 50;

class AStoreCache : public Test {
protected:
    Exchange::IStoreCache* cache;
    ~AStoreCache() override = default;
    void SetUp() override
    {
        Core::File(Path).Destroy();
        // File is destroyed

        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_PATH"), Path);
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_MAXSIZE"), std::to_string(MaxSize));
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_MAXVALUE"), std::to_string(MaxValue));
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_LIMIT"), std::to_string(Limit));
        cache = Core::Service<Sqlite::StoreCacheType<Sqlite::Handle>>::Create<Exchange::IStoreCache>();
    }
    void TearDown() override
    {
        cache->Release();
    }
};

TEST_F(AStoreCache, FlushesCacheWithoutError)
{
    EXPECT_THAT(cache->FlushCache(), Eq(Core::ERROR_NONE));
}
