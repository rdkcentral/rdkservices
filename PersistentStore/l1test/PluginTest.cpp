#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../PersistentStore.h"

#include "ServiceMock.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;

using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::NiceMock;
using ::testing::NotNull;
using ::testing::Test;

const std::string Path = "/tmp/persistentstore/l1test/plugintest";
const std::string Path2 = "/tmp/persistentstore/l1test/plugintest2";
const uint32_t MaxSize = 100;
const uint32_t MaxValue = 10;
const uint32_t Limit = 50;

class APlugin : public Test {
protected:
    NiceMock<ServiceMock> service;
    PluginHost::IPlugin* plugin;
    ~APlugin() override = default;
    void SetUp() override
    {
        Core::File(Path).Destroy();
        // File is destroyed

        plugin = Core::Service<PersistentStore>::Create<PluginHost::IPlugin>();
        JsonObject config;
        config["path"] = Path;
        config["maxsize"] = MaxSize;
        config["maxvalue"] = MaxValue;
        config["limit"] = Limit;
        string configJsonStr;
        config.ToString(configJsonStr);
        ON_CALL(service, ConfigLine())
            .WillByDefault(
                ::testing::Return(configJsonStr));
        ASSERT_THAT(plugin->Initialize(&service), Eq(""));
    }
    void TearDown() override
    {
        plugin->Deinitialize(&service);
        plugin->Release();
    }
};

TEST_F(APlugin, CreatesFile)
{
    EXPECT_THAT(Core::File(Path).Exists(), IsTrue());
}

template <typename ACTUALPLUGINTEST>
class APluginWithValue : public ACTUALPLUGINTEST {
protected:
    ~APluginWithValue() override = default;
    void SetUp() override
    {
        ACTUALPLUGINTEST::SetUp();
        auto jsonRpc = this->plugin->template QueryInterface<PluginHost::ILocalDispatcher>();
        ASSERT_THAT(jsonRpc, NotNull());
        string response;
        ASSERT_THAT(jsonRpc->Invoke(0, 0, "", "setValue", "{\"namespace\":\"ns1\",\"key\":\"key1\",\"value\":\"value1\"}", response), Eq(Core::ERROR_NONE));
        ASSERT_THAT(response, Eq("{\"success\":true}"));
        if (jsonRpc)
            jsonRpc->Release();
    }
};

template <typename ACTUALPLUGINTEST>
class APluginReopenedAtPath2 : public ACTUALPLUGINTEST {
protected:
    ~APluginReopenedAtPath2() override = default;
    void SetUp() override
    {
        ACTUALPLUGINTEST::SetUp();
        Core::File(Path2).Destroy();
        // File is destroyed

        this->plugin->Deinitialize(&this->service);
        JsonObject config;
        config["path"] = Path2;
        config["legacypath"] = Path;
        config["maxsize"] = MaxSize;
        config["maxvalue"] = MaxValue;
        config["limit"] = Limit;
        string configJsonStr;
        config.ToString(configJsonStr);
        ON_CALL(this->service, ConfigLine())
            .WillByDefault(
                ::testing::Return(configJsonStr));
        ASSERT_THAT(this->plugin->Initialize(&this->service), Eq(""));
        // Reopened at Path2
    }
};

class APluginWithValueReopenedAtPath2 : public APluginReopenedAtPath2<APluginWithValue<APlugin>> {
};

TEST_F(APluginWithValueReopenedAtPath2, MovesFile)
{
    EXPECT_THAT(Core::File(Path2).Exists(), IsTrue());
    EXPECT_THAT(Core::File(Path).Exists(), IsFalse());
}

TEST_F(APluginWithValueReopenedAtPath2, GetsValue)
{
    string response;
    auto jsonRpc = plugin->QueryInterface<PluginHost::ILocalDispatcher>();
    ASSERT_THAT(jsonRpc, NotNull());
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getValue", "{\"namespace\":\"ns1\",\"key\":\"key1\"}", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"value\":\"value1\",\"success\":true}"));
    if (jsonRpc)
        jsonRpc->Release();
}
