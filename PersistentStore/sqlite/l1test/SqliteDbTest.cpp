#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../SqliteDb.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;

using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsNull;
using ::testing::IsTrue;
using ::testing::NotNull;
using ::testing::Test;

const std::string Path = "/tmp/sqlite/l1test/sqlitedbtest";
const uint32_t MaxSize = 100;
const uint32_t MaxValue = 3;

class AnSqliteDb : public Test {
protected:
    ISqliteDb* db;

    AnSqliteDb()
    {
        Core::File(Path).Destroy();
        db = Core::Service<SqliteDb>::Create<ISqliteDb>(Path, MaxSize, MaxValue);
    }
    virtual ~AnSqliteDb() override
    {
        db->Release();
    }
};

TEST_F(AnSqliteDb, IsNull)
{
    EXPECT_THAT((sqlite3*)(*db), IsNull());
}

TEST_F(AnSqliteDb, DoesNotCreateFile)
{
    EXPECT_THAT(Core::File(Path).Exists(), IsFalse());
}

class AnOpenSqliteDb : public AnSqliteDb {
protected:
    AnOpenSqliteDb()
        : AnSqliteDb()
    {
    }
    virtual ~AnOpenSqliteDb() override = default;
    virtual void SetUp() override
    {
        ASSERT_THAT(db->Open(), Eq(Core::ERROR_NONE));
    }
};

TEST_F(AnOpenSqliteDb, IsNotNull)
{
    EXPECT_THAT((sqlite3*)(*db), NotNull());
}

TEST_F(AnOpenSqliteDb, CreatesFile)
{
    EXPECT_THAT(Core::File(Path).Exists(), IsTrue());
}

TEST_F(AnOpenSqliteDb, CannotOpenTwice)
{
    EXPECT_THAT(db->Open(), Eq(Core::ERROR_ALREADY_CONNECTED));
}

TEST_F(AnOpenSqliteDb, OpensAfterClosed)
{
    db->Release();
    db = Core::Service<SqliteDb>::Create<ISqliteDb>(Path, MaxSize, MaxValue);
    EXPECT_THAT(db->Open(), Eq(Core::ERROR_NONE));
}
