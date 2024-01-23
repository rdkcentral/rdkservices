#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Handle.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;

using ::testing::Eq;
using ::testing::IsTrue;
using ::testing::Test;

const std::string Path = "/tmp/sqlite/l1test/handletest";
const uint32_t MaxSize = 100;
const uint32_t MaxValue = 10;
const uint32_t Limit = 50;

class AHandle : public Test {
protected:
    Core::ProxyType<Sqlite::Handle> handle;
    ~AHandle() override = default;
    void SetUp() override
    {
        Core::File(Path).Destroy();
        // File is destroyed

        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_PATH"), Path);
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_MAXSIZE"), std::to_string(MaxSize));
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_MAXVALUE"), std::to_string(MaxValue));
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_LIMIT"), std::to_string(Limit));
        handle = Core::ProxyType<Sqlite::Handle>::Create();
    }
};

TEST_F(AHandle, GetsFilename)
{
    EXPECT_THAT(sqlite3_db_filename((sqlite3*)(*handle), nullptr), Eq(Path));
}

TEST_F(AHandle, CreatesFile)
{
    EXPECT_THAT(Core::File(Path).Exists(), IsTrue());
}

TEST_F(AHandle, Reopens)
{
    EXPECT_THAT(handle->Open(), Eq(Core::ERROR_NONE));
}
