#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Handle.h"
#include "../Store2Type.h"
#include "../StoreLimitType.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;

using ::testing::Eq;
using ::testing::Test;

const std::string Path = "/tmp/sqlite/l1test/storelimittest";
const uint32_t MaxSize = 100;
const uint32_t MaxValue = 10;
const uint32_t Limit = 50;

class AStoreLimit : public Test {
protected:
    Exchange::IStoreLimit* limit;
    ~AStoreLimit() override = default;
    void SetUp() override
    {
        Core::File(Path).Destroy();
        // File is destroyed

        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_PATH"), Path);
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_MAXSIZE"), std::to_string(MaxSize));
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_MAXVALUE"), std::to_string(MaxValue));
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_LIMIT"), std::to_string(Limit));
        limit = Core::Service<Sqlite::StoreLimitType<Sqlite::Handle>>::Create<Exchange::IStoreLimit>();
    }
    void TearDown() override
    {
        limit->Release();
    }
};

TEST_F(AStoreLimit, SetsLimitForUnknownNamespaceWithoutError)
{
    EXPECT_THAT(limit->SetNamespaceStorageLimit(Exchange::IStore2::ScopeType::DEVICE, "x", 10), Eq(Core::ERROR_NONE));
}

TEST_F(AStoreLimit, DoesNotGetLimitForUnknownNamespace)
{
    uint32_t value;
    EXPECT_THAT(limit->GetNamespaceStorageLimit(Exchange::IStore2::ScopeType::DEVICE, "x", value), Eq(Core::ERROR_NOT_EXIST));
}

TEST_F(AStoreLimit, DoesNotSetLimitForTooLargeNamespaceName)
{
    EXPECT_THAT(limit->SetNamespaceStorageLimit(Exchange::IStore2::ScopeType::DEVICE, "this is too large", 10), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStoreLimit, DoesNotSetLimitWhenAtMaxSize)
{
    for (int i = 0; i < 27; i++) {
        EXPECT_THAT(limit->SetNamespaceStorageLimit(Exchange::IStore2::ScopeType::DEVICE, "ns" + std::to_string(i), 10), Eq(Core::ERROR_NONE));
    }
    EXPECT_THAT(limit->SetNamespaceStorageLimit(Exchange::IStore2::ScopeType::DEVICE, "ns", 10), Eq(Core::ERROR_NONE));
    // Size is 100

    EXPECT_THAT(limit->SetNamespaceStorageLimit(Exchange::IStore2::ScopeType::DEVICE, "x", 10), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AStoreLimit, GetsDefaultLimitForExistingNamespace)
{
    auto store2 = Core::Service<Sqlite::Store2Type<Sqlite::Handle>>::Create<Exchange::IStore2>();
    ASSERT_THAT(store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, "ns1", "key1", "value1", 0), Eq(Core::ERROR_NONE));
    store2->Release();
    // Namespace added

    uint32_t value;
    EXPECT_THAT(limit->GetNamespaceStorageLimit(Exchange::IStore2::ScopeType::DEVICE, "ns1", value), Eq(Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(Limit));
}

TEST_F(AStoreLimit, GetsLimitThatWasSet)
{
    ASSERT_THAT(limit->SetNamespaceStorageLimit(Exchange::IStore2::ScopeType::DEVICE, "ns1", 30), Eq(Core::ERROR_NONE));
    // Namespace limit set

    uint32_t value;
    EXPECT_THAT(limit->GetNamespaceStorageLimit(Exchange::IStore2::ScopeType::DEVICE, "ns1", value), Eq(Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(30));
}

TEST_F(AStoreLimit, UpdatesLimitThatWasSet)
{
    ASSERT_THAT(limit->SetNamespaceStorageLimit(Exchange::IStore2::ScopeType::DEVICE, "ns1", 30), Eq(Core::ERROR_NONE));
    // Namespace limit set

    ASSERT_THAT(limit->SetNamespaceStorageLimit(Exchange::IStore2::ScopeType::DEVICE, "ns1", 40), Eq(Core::ERROR_NONE));
    uint32_t value;
    EXPECT_THAT(limit->GetNamespaceStorageLimit(Exchange::IStore2::ScopeType::DEVICE, "ns1", value), Eq(Core::ERROR_NONE));
    EXPECT_THAT(value, Eq(40));
}
