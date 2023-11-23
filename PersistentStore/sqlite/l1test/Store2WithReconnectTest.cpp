#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Handle.h"
#include "../Store2Type.h"
#include "../Store2WithReconnectType.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;

using ::testing::Eq;
using ::testing::Test;

const std::string Path = "/tmp/sqlite/l1test/store2withreconnecttest";
const uint32_t MaxSize = 100;
const uint32_t MaxValue = 10;
const uint32_t Limit = 50;

class AStore2WithReconnect : public Test {
protected:
    Exchange::IStore2* store2;
    ~AStore2WithReconnect() override = default;
    void SetUp() override
    {
        Core::File(Path).Destroy();
        // File is destroyed

        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_PATH"), Path);
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_MAXSIZE"), std::to_string(MaxSize));
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_MAXVALUE"), std::to_string(MaxValue));
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_LIMIT"), std::to_string(Limit));
        store2 = Core::Service<Sqlite::Store2WithReconnectType<Sqlite::Store2Type<Sqlite::Handle>>>::Create<Exchange::IStore2>();
    }
    void TearDown() override
    {
        store2->Release();
    }
};

TEST_F(AStore2WithReconnect, SetsValueAfterFileDestroy)
{
    Core::File(Path).Destroy();
    // File is destroyed

    EXPECT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key1", "value1", 0), Eq(Core::ERROR_NONE));
    string value;
    uint32_t ttl;
    EXPECT_THAT(store2->GetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key1", value, ttl), Eq(Core::ERROR_NONE));
    EXPECT_THAT(value, Eq("value1"));
    EXPECT_THAT(ttl, Eq(0));
}
