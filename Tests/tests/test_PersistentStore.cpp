/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "PersistentStore.h"

#include "FactoriesImplementation.h"
#include "ServiceMock.h"

namespace {
class PersistentStoreMock : public WPEFramework::Plugin::PersistentStore {
public:
    virtual ~PersistentStoreMock() = default;

    MOCK_METHOD(std::vector<string>, LegacyLocations, (), (const, override));
};
}

using namespace WPEFramework;

using ::testing::Eq;
using ::testing::NiceMock;

class PersistentStoreTest : public ::testing::Test {
protected:
    Core::ProxyType<PersistentStoreMock> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    PersistentStoreTest()
        : plugin(Core::ProxyType<PersistentStoreMock>::Create())
        , handler(*plugin)
        , connection(1, 0)
    {
    }
    virtual ~PersistentStoreTest() = default;
};

class PersistentStoreInitializedTest : public PersistentStoreTest {
protected:
    NiceMock<ServiceMock> service;

    PersistentStoreInitializedTest()
        : PersistentStoreTest()
    {
        ON_CALL(service, ConfigLine())
            .WillByDefault(
                ::testing::Return("{"
                                  "\"path\":\"/tmp/rdkservicestore\","
                                  "\"key\":null,"
                                  "\"maxsize\":20,"
                                  "\"maxvalue\":10"
                                  "}"));
        ON_CALL(*plugin, LegacyLocations)
            .WillByDefault(::testing::Return(std::vector<string>()));

        EXPECT_EQ(string(""), plugin->Initialize(&service));
    }
    virtual ~PersistentStoreInitializedTest() override
    {
        plugin->Deinitialize(&service);
    }
};

class PersistentStoreInitializedEventTest : public PersistentStoreInitializedTest {
protected:
    Core::JSONRPC::Message message;
    NiceMock<FactoriesImplementation> factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    PersistentStoreInitializedEventTest()
        : PersistentStoreInitializedTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }
    virtual ~PersistentStoreInitializedEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();

        PluginHost::IFactories::Assign(nullptr);
    }
};

TEST_F(PersistentStoreTest, registeredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setValue")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getValue")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("deleteKey")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("deleteNamespace")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getKeys")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getNamespaces")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getStorageSize")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("flushCache")));
}

TEST_F(PersistentStoreTest, paramsMissing)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setValue"), _T("{}"), response));
    EXPECT_THAT(response, Eq("{\"error\":\"params missing\",\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getValue"), _T("{}"), response));
    EXPECT_THAT(response, Eq("{\"error\":\"params missing\",\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getKeys"), _T("{}"), response));
    EXPECT_THAT(response, Eq("{\"error\":\"params missing\",\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteKey"), _T("{}"), response));
    EXPECT_THAT(response, Eq("{\"error\":\"params missing\",\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteNamespace"), _T("{}"), response));
    EXPECT_THAT(response, Eq("{\"error\":\"params missing\",\"success\":false}"));
}

TEST_F(PersistentStoreTest, paramsEmpty)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setValue"), _T("{\"namespace\":\"\",\"key\":\"\",\"value\":\"\"}"), response));
    EXPECT_THAT(response, Eq("{\"error\":\"params empty\",\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getValue"), _T("{\"namespace\":\"\",\"key\":\"\"}"), response));
    EXPECT_THAT(response, Eq("{\"error\":\"params empty\",\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getKeys"), _T("{\"namespace\":\"\"}"), response));
    EXPECT_THAT(response, Eq("{\"error\":\"params empty\",\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteKey"), _T("{\"namespace\":\"\",\"key\":\"\"}"), response));
    EXPECT_THAT(response, Eq("{\"error\":\"params empty\",\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteNamespace"), _T("{\"namespace\":\"\"}"), response));
    EXPECT_THAT(response, Eq("{\"error\":\"params empty\",\"success\":false}"));
}

TEST_F(PersistentStoreTest, notInitialized)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setValue"), _T("{\"namespace\":\"test\",\"key\":\"a\",\"value\":\"1\"}"), response));
    EXPECT_THAT(response, Eq("{\"error\":\"params too long\",\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getValue"), _T("{\"namespace\":\"test\",\"key\":\"a\"}"), response));
    EXPECT_THAT(response, Eq("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getNamespaces"), _T("{}"), response));
    EXPECT_THAT(response, Eq("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStorageSize"), _T("{}"), response));
    EXPECT_THAT(response, Eq("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getKeys"), _T("{\"namespace\":\"test\"}"), response));
    EXPECT_THAT(response, Eq("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteKey"), _T("{\"namespace\":\"test\",\"key\":\"a\"}"), response));
    EXPECT_THAT(response, Eq("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteNamespace"), _T("{\"namespace\":\"test\"}"), response));
    EXPECT_THAT(response, Eq("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("flushCache"), _T("{}"), response));
    EXPECT_THAT(response, Eq("{\"success\":false}"));
}

TEST_F(PersistentStoreInitializedEventTest, jsonRpc)
{
    Core::Event onValueChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.PersistentStore.onValueChanged\","
                                          "\"params\":{\"namespace\":\"test\",\"key\":\"a\",\"value\":\"1\"}"
                                          "}")));

                onValueChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onValueChanged"), _T("org.rdk.PersistentStore"), message);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setValue"), _T("{\"namespace\":\"test\",\"key\":\"a\",\"value\":\"1\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, onValueChanged.Lock());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getValue"), _T("{\"namespace\":\"test\",\"key\":\"a\"}"), response));
    EXPECT_EQ(response, _T("{\"value\":\"1\",\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getNamespaces"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"namespaces\":[\"test\"],\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStorageSize"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"namespaceSizes\":{\"test\":2},\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getKeys"), _T("{\"namespace\":\"test\"}"), response));
    EXPECT_EQ(response, _T("{\"keys\":[\"a\"],\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteKey"), _T("{\"namespace\":\"test\",\"key\":\"a\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteNamespace"), _T("{\"namespace\":\"test\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("flushCache"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));

    handler.Unsubscribe(0, _T("onValueChanged"), _T("org.rdk.PersistentStore"), message);
}

TEST_F(PersistentStoreInitializedTest, maxValue)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setValue"), _T("{\"namespace\":\"test\",\"key\":\"a\",\"value\":\"123456789123456789\"}"), response));
    EXPECT_THAT(response, Eq("{\"error\":\"params too long\",\"success\":false}"));
}

TEST_F(PersistentStoreInitializedEventTest, onStorageExceeded)
{
    Core::Event onStorageExceeded(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.PersistentStore.onStorageExceeded\","
                                          "\"params\":{}"
                                          "}")));

                onStorageExceeded.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onStorageExceeded"), _T("org.rdk.PersistentStore"), message);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setValue"), _T("{\"namespace\":\"test\",\"key\":\"a\",\"value\":\"123456789\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setValue"), _T("{\"namespace\":\"test\",\"key\":\"b\",\"value\":\"12345678\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setValue"), _T("{\"namespace\":\"test\",\"key\":\"c\",\"value\":\"1\"}"), response));
    EXPECT_THAT(response, Eq("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, onStorageExceeded.Lock());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteNamespace"), _T("{\"namespace\":\"test\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));

    handler.Unsubscribe(0, _T("onStorageExceeded"), _T("org.rdk.PersistentStore"), message);
}

TEST_F(PersistentStoreTest, legacyLocation)
{
    ServiceMock service;

    EXPECT_CALL(service, ConfigLine())
        .Times(2)
        .WillOnce(
            ::testing::Return("{"
                              "\"path\":\"/tmp/path/to/legacy/location/store\","
                              "\"key\":null,"
                              "\"maxsize\":20,"
                              "\"maxvalue\":10"
                              "}"))
        .WillOnce(
            ::testing::Return("{"
                              "\"path\":\"/tmp/rdkservicestore\","
                              "\"key\":null,"
                              "\"maxsize\":20,"
                              "\"maxvalue\":10"
                              "}"));
    EXPECT_CALL(*plugin, LegacyLocations)
        .Times(2)
        .WillOnce(::testing::Return(std::vector<string>()))
        .WillOnce(::testing::Return(std::vector<string>{ "/tmp/path/to/legacy/location/store" }));

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setValue"), _T("{\"namespace\":\"test\",\"key\":\"d\",\"value\":\"abc\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));

    plugin->Deinitialize(&service);

    if (Core::File(string("/tmp/rdkservicestore")).Exists()) {
        EXPECT_TRUE(Core::File(string("/tmp/rdkservicestore")).Destroy());
        EXPECT_FALSE(Core::File(string("/tmp/rdkservicestore")).Exists());
    }

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getValue"), _T("{\"namespace\":\"test\",\"key\":\"d\"}"), response));
    EXPECT_EQ(response, _T("{\"value\":\"abc\",\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteNamespace"), _T("{\"namespace\":\"test\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));

    plugin->Deinitialize(&service);
}
