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

#include <gtest/gtest.h>

#include "PersistentStoreMock.h"
#include "ServiceMock.h"

using namespace WPEFramework;

class PersistentStoreTestFixture : public ::testing::Test {
protected:
    Core::ProxyType<PersistentStoreMock> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    ServiceMock service;
    string response;

    PersistentStoreTestFixture()
        : plugin(Core::ProxyType<PersistentStoreMock>::Create())
        , handler(*plugin)
        , connection(1, 0)
    {
    }
    virtual ~PersistentStoreTestFixture()
    {
    }
};

TEST_F(PersistentStoreTestFixture, registeredMethods)
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

TEST_F(PersistentStoreTestFixture, paramsMissing)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setValue"), _T("{}"), response));
    EXPECT_EQ(response,
        _T("{\"error\":\"params missing\",\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getValue"), _T("{}"), response));
    EXPECT_EQ(response,
        _T("{\"error\":\"params missing\",\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getKeys"), _T("{}"), response));
    EXPECT_EQ(response,
        _T("{\"error\":\"params missing\",\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteKey"), _T("{}"), response));
    EXPECT_EQ(response,
        _T("{\"error\":\"params missing\",\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteNamespace"), _T("{}"), response));
    EXPECT_EQ(response,
        _T("{\"error\":\"params missing\",\"success\":false}"));
}

TEST_F(PersistentStoreTestFixture, paramsEmpty)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setValue"), _T("{\"namespace\":\"\",\"key\":\"\",\"value\":\"\"}"), response));
    EXPECT_EQ(response,
        _T("{\"error\":\"params empty\",\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getValue"), _T("{\"namespace\":\"\",\"key\":\"\"}"), response));
    EXPECT_EQ(response,
        _T("{\"error\":\"params empty\",\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getKeys"), _T("{\"namespace\":\"\"}"), response));
    EXPECT_EQ(response,
        _T("{\"error\":\"params empty\",\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteKey"), _T("{\"namespace\":\"\",\"key\":\"\"}"), response));
    EXPECT_EQ(response,
        _T("{\"error\":\"params empty\",\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteNamespace"), _T("{\"namespace\":\"\"}"), response));
    EXPECT_EQ(response,
        _T("{\"error\":\"params empty\",\"success\":false}"));
}

TEST_F(PersistentStoreTestFixture, notInitialized)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setValue"), _T("{\"namespace\":\"test\",\"key\":\"a\",\"value\":\"1\"}"), response));
    EXPECT_EQ(response,
        _T("{\"error\":\"params too long\",\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getValue"), _T("{\"namespace\":\"test\",\"key\":\"a\"}"), response));
    EXPECT_EQ(response,
        _T("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getNamespaces"), _T("{}"), response));
    EXPECT_EQ(response,
        _T("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStorageSize"), _T("{}"), response));
    EXPECT_EQ(response,
        _T("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getKeys"), _T("{\"namespace\":\"test\"}"), response));
    EXPECT_EQ(response,
        _T("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteKey"), _T("{\"namespace\":\"test\",\"key\":\"a\"}"), response));
    EXPECT_EQ(response,
        _T("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteNamespace"), _T("{\"namespace\":\"test\"}"), response));
    EXPECT_EQ(response,
        _T("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("flushCache"), _T("{}"), response));
    EXPECT_EQ(response,
        _T("{\"success\":false}"));
}

TEST_F(PersistentStoreTestFixture, jsonRpc)
{
    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(
            ::testing::Return("{"
                              "\"path\":\"/tmp/rdkservicestore\","
                              "\"key\":null,"
                              "\"maxsize\":20,"
                              "\"maxvalue\":10"
                              "}"));

    ON_CALL(*plugin, LegacyLocations)
        .WillByDefault(
            ::testing::Return(std::vector<string>()));

    EXPECT_CALL(*plugin, event_onValueChanged)
        .Times(1)
        .WillOnce(
            ::testing::Return());

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setValue"), _T("{\"namespace\":\"test\",\"key\":\"a\",\"value\":\"1\"}"), response));
    EXPECT_EQ(response,
        _T("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getValue"), _T("{\"namespace\":\"test\",\"key\":\"a\"}"), response));
    EXPECT_EQ(response,
        _T("{\"value\":\"1\",\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getNamespaces"), _T("{}"), response));
    EXPECT_EQ(response,
        _T("{\"namespaces\":[\"test\"],\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStorageSize"), _T("{}"), response));
    EXPECT_EQ(response,
        _T("{\"namespaceSizes\":{\"test\":2},\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getKeys"), _T("{\"namespace\":\"test\"}"), response));
    EXPECT_EQ(response,
        _T("{\"keys\":[\"a\"],\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteKey"), _T("{\"namespace\":\"test\",\"key\":\"a\"}"), response));
    EXPECT_EQ(response,
        _T("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteNamespace"), _T("{\"namespace\":\"test\"}"), response));
    EXPECT_EQ(response,
        _T("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("flushCache"), _T("{}"), response));
    EXPECT_EQ(response,
        _T("{\"success\":true}"));

    plugin->Deinitialize(&service);
}

TEST_F(PersistentStoreTestFixture, maxValue)
{
    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(
            ::testing::Return("{"
                              "\"path\":\"/tmp/rdkservicestore\","
                              "\"key\":null,"
                              "\"maxsize\":20,"
                              "\"maxvalue\":10"
                              "}"));

    ON_CALL(*plugin, LegacyLocations)
        .WillByDefault(
            ::testing::Return(std::vector<string>()));

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setValue"), _T("{\"namespace\":\"test\",\"key\":\"a\",\"value\":\"123456789123456789\"}"), response));
    EXPECT_EQ(response,
        _T("{\"error\":\"params too long\",\"success\":false}"));

    plugin->Deinitialize(&service);
}

TEST_F(PersistentStoreTestFixture, onValueChanged)
{
    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(
            ::testing::Return("{"
                              "\"path\":\"/tmp/rdkservicestore\","
                              "\"key\":null,"
                              "\"maxsize\":20,"
                              "\"maxvalue\":10"
                              "}"));

    ON_CALL(*plugin, LegacyLocations)
        .WillByDefault(
            ::testing::Return(std::vector<string>()));

    EXPECT_CALL(*plugin, event_onValueChanged("test", "a", "12345"))
        .Times(1)
        .WillOnce(
            ::testing::Return());

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setValue"), _T("{\"namespace\":\"test\",\"key\":\"a\",\"value\":\"12345\"}"), response));
    EXPECT_EQ(response,
        _T("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteNamespace"), _T("{\"namespace\":\"test\"}"), response));
    EXPECT_EQ(response,
        _T("{\"success\":true}"));

    plugin->Deinitialize(&service);
}

TEST_F(PersistentStoreTestFixture, onStorageExceeded)
{
    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(
            ::testing::Return("{"
                              "\"path\":\"/tmp/rdkservicestore\","
                              "\"key\":null,"
                              "\"maxsize\":20,"
                              "\"maxvalue\":20"
                              "}"));

    ON_CALL(*plugin, LegacyLocations)
        .WillByDefault(
            ::testing::Return(std::vector<string>()));

    EXPECT_CALL(*plugin, event_onValueChanged("test", "a", "123456789123456789"))
        .Times(1)
        .WillOnce(
            ::testing::Return());
    EXPECT_CALL(*plugin, event_onStorageExceeded())
        .Times(1)
        .WillOnce(
            ::testing::Return());

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setValue"), _T("{\"namespace\":\"test\",\"key\":\"a\",\"value\":\"123456789123456789\"}"), response));
    EXPECT_EQ(response,
        _T("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setValue"), _T("{\"namespace\":\"test\",\"key\":\"b\",\"value\":\"1\"}"), response));
    EXPECT_EQ(response,
        _T("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getValue"), _T("{\"namespace\":\"test\",\"key\":\"a\"}"), response));
    EXPECT_EQ(response,
        _T("{\"value\":\"123456789123456789\",\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteNamespace"), _T("{\"namespace\":\"test\"}"), response));
    EXPECT_EQ(response,
        _T("{\"success\":true}"));

    plugin->Deinitialize(&service);
}

TEST_F(PersistentStoreTestFixture, setupLegacyLocation)
{
    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(
            ::testing::Return("{"
                              "\"path\":\"/tmp/path/to/legacy/location/store\","
                              "\"key\":null,"
                              "\"maxsize\":20,"
                              "\"maxvalue\":10"
                              "}"));

    ON_CALL(*plugin, LegacyLocations)
        .WillByDefault(
            ::testing::Return(std::vector<string>()));

    EXPECT_CALL(*plugin, event_onValueChanged("test", "d", "abc"))
        .Times(1)
        .WillOnce(
            ::testing::Return());

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setValue"), _T("{\"namespace\":\"test\",\"key\":\"d\",\"value\":\"abc\"}"), response));
    EXPECT_EQ(response,
        _T("{\"success\":true}"));

    plugin->Deinitialize(&service);
}

TEST_F(PersistentStoreTestFixture, useLegacyLocation)
{
    if (Core::File(string("/tmp/rdkservicestore")).Exists()) {
        EXPECT_TRUE(Core::File(string("/tmp/rdkservicestore")).Destroy());
        EXPECT_FALSE(Core::File(string("/tmp/rdkservicestore")).Exists());
    }

    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(
            ::testing::Return("{"
                              "\"path\":\"/tmp/rdkservicestore\","
                              "\"key\":null,"
                              "\"maxsize\":20,"
                              "\"maxvalue\":10"
                              "}"));

    EXPECT_CALL(*plugin, LegacyLocations)
        .Times(1)
        .WillOnce(
            ::testing::Return(std::vector<string>{
                "/tmp/path/to/legacy/location/store" }));

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getValue"), _T("{\"namespace\":\"test\",\"key\":\"d\"}"), response));
    EXPECT_EQ(response,
        _T("{\"value\":\"abc\",\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deleteNamespace"), _T("{\"namespace\":\"test\"}"), response));
    EXPECT_EQ(response,
        _T("{\"success\":true}"));

    plugin->Deinitialize(&service);
}
