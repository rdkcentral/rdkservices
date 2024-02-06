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

const std::string Path = "/tmp/persistentstore/l1test/jsonrpctest";
const uint32_t MaxSize = 100;
const uint32_t MaxValue = 10;
const uint32_t Limit = 50;

class AJsonRpc : public Test {
protected:
    NiceMock<ServiceMock> service;
    PluginHost::IPlugin* plugin;
    PluginHost::ILocalDispatcher* jsonRpc;
    ~AJsonRpc() override = default;
    void SetUp() override
    {
        Core::File(Path).Destroy();
        // File is destroyed

        plugin = Core::Service<PersistentStore>::Create<PluginHost::IPlugin>();
        jsonRpc = plugin->QueryInterface<PluginHost::ILocalDispatcher>();
        ASSERT_THAT(jsonRpc, NotNull());
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
        if (jsonRpc)
            jsonRpc->Release();
        plugin->Release();
    }
};

TEST_F(AJsonRpc, DoesNotGetValueInUnknownNamespace)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getValue", "{\"namespace\":\"x\",\"key\":\"x\"}", response), Eq(Core::ERROR_NOT_EXIST));
}

TEST_F(AJsonRpc, DoesNotGetValueWhenMandatoryParamsAreMissing)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getValue", "{\"key\":\"x\"}", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getValue", "{\"namespace\":\"x\"}", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AJsonRpc, DoesNotSetEmptyNamespace)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "setValue", "{\"namespace\":\"\",\"key\":\"x\",\"value\":\"x\"}", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AJsonRpc, DoesNotSetEmptyKey)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "setValue", "{\"namespace\":\"x\",\"key\":\"\",\"value\":\"x\"}", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AJsonRpc, DoesNotSetWhenMandatoryParamsAreMissing)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "setValue", "{\"key\":\"x\",\"value\":\"x\"}", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "setValue", "{\"namespace\":\"x\",\"value\":\"x\"}", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "setValue", "{\"namespace\":\"x\",\"key\":\"x\"}", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AJsonRpc, DoesNotGetKeysWhenMandatoryParamsAreMissing)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getKeys", "", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AJsonRpc, GetsKeysForUnknownNamespaceWithoutError)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getKeys", "{\"namespace\":\"x\"}", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"success\":true}"));
}

TEST_F(AJsonRpc, GetsNamespacesWhenEmptyWithoutError)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getNamespaces", "", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"success\":true}"));
}

TEST_F(AJsonRpc, GetsStorageSizesWhenEmptyWithoutError)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getStorageSizes", "", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{}"));
}

// Deprecated
TEST_F(AJsonRpc, GetsStorageSizeWhenEmptyWithoutError)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getStorageSize", "", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"namespaceSizes\":{},\"success\":true}"));
}

TEST_F(AJsonRpc, DoesNotGetLimitForUnknownNamespace)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getNamespaceStorageLimit", "{\"namespace\":\"x\"}", response), Eq(Core::ERROR_NOT_EXIST));
}

TEST_F(AJsonRpc, DoesNotGetLimitWhenMandatoryParamsAreMissing)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getNamespaceStorageLimit", "", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AJsonRpc, DoesNotSetLimitWhenMandatoryParamsAreMissing)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "setNamespaceStorageLimit", "", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AJsonRpc, DoesNotDeleteKeyWhenMandatoryParamsAreMissing)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "deleteKey", "{\"key\":\"x\"}", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "deleteKey", "{\"namespace\":\"x\"}", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AJsonRpc, DoesNotDeleteNamespaceWhenMandatoryParamsAreMissing)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "deleteNamespace", "", response), Eq(Core::ERROR_INVALID_INPUT_LENGTH));
}

TEST_F(AJsonRpc, FlushesCacheWithoutError)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "flushCache", "", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"success\":true}"));
}

TEST_F(AJsonRpc, GetsDefaultLimitForExistingNamespace)
{
    string response;
    ASSERT_THAT(jsonRpc->Invoke(0, 0, "", "setValue", "{\"namespace\":\"ns1\",\"key\":\"key1\",\"value\":\"value1\"}", response), Eq(Core::ERROR_NONE));
    ASSERT_THAT(response, Eq("{\"success\":true}"));
    // Namespace added

    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getNamespaceStorageLimit", "{\"namespace\":\"ns1\"}", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"storageLimit\":50}"));
}

TEST_F(AJsonRpc, GetsLimitThatWasSet)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "setNamespaceStorageLimit", "{\"namespace\":\"ns1\",\"storageLimit\":\"10\"}", response), Eq(Core::ERROR_NONE));
    // Namespace limit set

    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getNamespaceStorageLimit", "{\"namespace\":\"ns1\"}", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"storageLimit\":10}"));
}

class AJsonRpcWithValues : public AJsonRpc {
protected:
    ~AJsonRpcWithValues() override = default;
    void SetUp() override
    {
        AJsonRpc::SetUp();
        string response;
        ASSERT_THAT(jsonRpc->Invoke(0, 0, "", "setValue", "{\"namespace\":\"ns1\",\"key\":\"key1\",\"value\":\"value1\"}", response), Eq(Core::ERROR_NONE));
        ASSERT_THAT(response, Eq("{\"success\":true}"));
        ASSERT_THAT(jsonRpc->Invoke(0, 0, "", "setValue", "{\"namespace\":\"ns1\",\"key\":\"key2\",\"value\":\"value2\"}", response), Eq(Core::ERROR_NONE));
        ASSERT_THAT(response, Eq("{\"success\":true}"));
        ASSERT_THAT(jsonRpc->Invoke(0, 0, "", "setValue", "{\"namespace\":\"ns2\",\"key\":\"key1\",\"value\":\"value1\"}", response), Eq(Core::ERROR_NONE));
        ASSERT_THAT(response, Eq("{\"success\":true}"));
    }
};

TEST_F(AJsonRpcWithValues, GetsValues)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getValue", "{\"namespace\":\"ns1\",\"key\":\"key1\"}", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"value\":\"value1\",\"success\":true}"));
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getValue", "{\"namespace\":\"ns1\",\"key\":\"key2\"}", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"value\":\"value2\",\"success\":true}"));
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getValue", "{\"namespace\":\"ns2\",\"key\":\"key1\"}", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"value\":\"value1\",\"success\":true}"));
}

TEST_F(AJsonRpcWithValues, DoesNotGetUnknownValueInExistingNamespace)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getValue", "{\"namespace\":\"ns1\",\"key\":\"x\"}", response), Eq(Core::ERROR_UNKNOWN_KEY));
}

TEST_F(AJsonRpcWithValues, GetsKeys)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getKeys", "{\"namespace\":\"ns1\"}", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"keys\":[\"key1\",\"key2\"],\"success\":true}"));
}

TEST_F(AJsonRpcWithValues, GetsNamespaces)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getNamespaces", "", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"namespaces\":[\"ns1\",\"ns2\"],\"success\":true}"));
}

TEST_F(AJsonRpcWithValues, GetsStorageSizes)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getStorageSizes", "", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"storageList\":[{\"namespace\":\"ns1\",\"size\":20},{\"namespace\":\"ns2\",\"size\":10}]}"));
}

// Deprecated
TEST_F(AJsonRpcWithValues, GetsStorageSize)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getStorageSize", "", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"namespaceSizes\":{\"ns1\":20,\"ns2\":10},\"success\":true}"));
}

TEST_F(AJsonRpcWithValues, DoesNotGetDeletedValue)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "deleteKey", "{\"namespace\":\"ns1\",\"key\":\"key1\"}", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"success\":true}"));
    // Value is deleted

    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getValue", "{\"namespace\":\"ns1\",\"key\":\"key1\"}", response), Eq(Core::ERROR_UNKNOWN_KEY));
}

TEST_F(AJsonRpcWithValues, DoesNotGetValueInDeletedNamespace)
{
    string response;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "deleteNamespace", "{\"namespace\":\"ns1\"}", response), Eq(Core::ERROR_NONE));
    EXPECT_THAT(response, Eq("{\"success\":true}"));
    // Namespace is deleted

    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "getValue", "{\"namespace\":\"ns1\",\"key\":\"key1\"}", response), Eq(Core::ERROR_NOT_EXIST));
}
