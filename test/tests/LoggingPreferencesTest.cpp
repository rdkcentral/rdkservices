/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
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
 **/

#include "gtest/gtest.h"

#include "LoggingPreferences.h"

#include "FactoriesImplementation.h"
#include "IarmBusMock.h"
#include "ServiceMock.h"

using namespace WPEFramework;

namespace {
const string iarmName = _T("Thunder_Plugins");
}

class LoggingPreferencesTestFixture : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::LoggingPreferences> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;
    ServiceMock service;
    Core::JSONRPC::Message message;
    IarmBusImplMock iarmBusImplMock;
    FactoriesImplementation factoriesImplementation;

    LoggingPreferencesTestFixture()
        : plugin(Core::ProxyType<Plugin::LoggingPreferences>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);
    }
    virtual ~LoggingPreferencesTestFixture()
    {
        PluginHost::IFactories::Assign(nullptr);
    }

    virtual void SetUp()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;
    }

    virtual void TearDown()
    {
        IarmBus::getInstance().impl = nullptr;
    }
};

TEST_F(LoggingPreferencesTestFixture, registeredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("isKeystrokeMaskEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setKeystrokeMaskEnabled")));
}

TEST_F(LoggingPreferencesTestFixture, paramsMissing)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setKeystrokeMaskEnabled"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"success\":false}"));
}

TEST_F(LoggingPreferencesTestFixture, jsonRpc)
{
    /////////////// IARM expectations - Begin ////////////////////////////

    // called by InitializeIARM, DeinitializeIARM
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_IsConnected(::testing::_, ::testing::_))
        .Times(3)
        .WillOnce(::testing::Invoke(
            [](const char* memberName, int* isRegistered) {
                if (iarmName == string(memberName)) {
                    *isRegistered = 0;
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            }))
        .WillOnce(::testing::Invoke(
            [](const char* memberName, int* isRegistered) {
                if (iarmName == string(memberName)) {
                    *isRegistered = 1;
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            }))
        .WillOnce(::testing::Invoke(
            [](const char* memberName, int* isRegistered) {
                if (iarmName == string(memberName)) {
                    *isRegistered = 1;
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            }));

    // called by InitializeIARM
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Init(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](const char* name) {
                if (iarmName == string(name)) {
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            }));

    // called by InitializeIARM
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Connect())
        .Times(1)
        .WillOnce(::testing::Return(IARM_RESULT_SUCCESS));

    /////////////// IARM expectations - End //////////////////////////////

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(2)
        // called by onKeystrokeMaskEnabledChange
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.LoggingPreferences.onKeystrokeMaskEnabledChange\","
                                          "\"params\":{\"keystrokeMaskEnabled\":false}"
                                          "}")));
                return Core::ERROR_NONE;
            }))

        // called by onKeystrokeMaskEnabledChange
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.LoggingPreferences.onKeystrokeMaskEnabledChange\","
                                          "\"params\":{\"keystrokeMaskEnabled\":true}"
                                          "}")));

                return Core::ERROR_NONE;
            }));

    EXPECT_EQ(string(""), plugin->Initialize(nullptr));

    auto dispatcher = static_cast<PluginHost::IDispatcher*>(
        plugin->QueryInterface(PluginHost::IDispatcher::ID));
    EXPECT_TRUE(dispatcher != nullptr);

    dispatcher->Activate(&service);

    handler.Subscribe(0, _T("onKeystrokeMaskEnabledChange"), _T("org.rdk.LoggingPreferences"), message);

    //Default setting shold be false
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isKeystrokeMaskEnabled"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"keystrokeMaskEnabled\":false,\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setKeystrokeMaskEnabled"), _T("{\"keystrokeMaskEnabled\":true}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isKeystrokeMaskEnabled"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"keystrokeMaskEnabled\":true,\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setKeystrokeMaskEnabled"), _T("{\"keystrokeMaskEnabled\":false}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isKeystrokeMaskEnabled"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"keystrokeMaskEnabled\":false,\"success\":true}"));

    handler.Unsubscribe(0, _T("onKeystrokeMaskEnabledChange"), _T("org.rdk.LoggingPreferences"), message);

    dispatcher->Deactivate();
    dispatcher->Release();

    plugin->Deinitialize(nullptr);
}
