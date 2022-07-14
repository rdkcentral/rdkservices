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
#include "sysMgr.h"

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

    void initService()
    {
        EXPECT_CALL(iarmBusImplMock, IARM_Bus_IsConnected)
            .Times(testing::AtLeast(2))
            .WillRepeatedly([](const char* memberName, int* isRegistered) {
                if (iarmName == string(memberName)) {
                    *isRegistered = 0;
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            });

        EXPECT_CALL(iarmBusImplMock, IARM_Bus_Init)
            .WillOnce([](const char* name) {
                if (iarmName == string(name)) {
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            });

        EXPECT_CALL(iarmBusImplMock, IARM_Bus_Connect)
            .WillOnce(testing::Return(IARM_RESULT_SUCCESS));

        EXPECT_EQ(string(""), plugin->Initialize(nullptr));
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

TEST_F(LoggingPreferencesTestFixture, getKeystrokeMask)
{
    initService();
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_TRUE(strcmp(methodName, IARM_BUS_SYSMGR_API_GetKeyCodeLoggingPref) == 0);
                auto* param = static_cast<IARM_BUS_SYSMGR_KEYCodeLoggingInfo_Param_t*>(arg);
                param->logStatus = 1;   //Setting 1 returns response as false
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isKeystrokeMaskEnabled"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"keystrokeMaskEnabled\":false,\"success\":true}"));

    plugin->Deinitialize(nullptr);
}

TEST_F(LoggingPreferencesTestFixture, enableKeystrokeMask)
{
    initService();
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_TRUE(strcmp(methodName, IARM_BUS_SYSMGR_API_GetKeyCodeLoggingPref) == 0);
                auto* param = static_cast<IARM_BUS_SYSMGR_KEYCodeLoggingInfo_Param_t*>(arg);
                param->logStatus = 1;
                return IARM_RESULT_SUCCESS;
            })
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_TRUE(strcmp(methodName, IARM_BUS_SYSMGR_API_GetKeyCodeLoggingPref) == 0);
                auto* param = static_cast<IARM_BUS_SYSMGR_KEYCodeLoggingInfo_Param_t*>(arg);
                param->logStatus = 1;
                return IARM_RESULT_SUCCESS;
            })
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_TRUE(strcmp(methodName, IARM_BUS_SYSMGR_API_SetKeyCodeLoggingPref) == 0);
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
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

    auto dispatcher = static_cast<PluginHost::IDispatcher*>(
        plugin->QueryInterface(PluginHost::IDispatcher::ID));
    EXPECT_TRUE(dispatcher != nullptr);

    dispatcher->Activate(&service);

    handler.Subscribe(0, _T("onKeystrokeMaskEnabledChange"), _T("org.rdk.LoggingPreferences"), message);

    //Simulating the case for setting the same value again
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setKeystrokeMaskEnabled"), _T("{\"keystrokeMaskEnabled\":false}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setKeystrokeMaskEnabled"), _T("{\"keystrokeMaskEnabled\":true}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));

    handler.Unsubscribe(0, _T("onKeystrokeMaskEnabledChange"), _T("org.rdk.LoggingPreferences"), message);

    dispatcher->Deactivate();
    dispatcher->Release();

    plugin->Deinitialize(nullptr);
}

TEST_F(LoggingPreferencesTestFixture, disbleKeystrokeMask)
{
    initService();
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_TRUE(strcmp(methodName, IARM_BUS_SYSMGR_API_GetKeyCodeLoggingPref) == 0);
                auto* param = static_cast<IARM_BUS_SYSMGR_KEYCodeLoggingInfo_Param_t*>(arg);
                param->logStatus = 0;
                return IARM_RESULT_SUCCESS;
            })

        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_TRUE(strcmp(methodName, IARM_BUS_SYSMGR_API_SetKeyCodeLoggingPref) == 0);
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
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
            }));

    auto dispatcher = static_cast<PluginHost::IDispatcher*>(
        plugin->QueryInterface(PluginHost::IDispatcher::ID));
    EXPECT_TRUE(dispatcher != nullptr);

    dispatcher->Activate(&service);

    handler.Subscribe(0, _T("onKeystrokeMaskEnabledChange"), _T("org.rdk.LoggingPreferences"), message);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setKeystrokeMaskEnabled"), _T("{\"keystrokeMaskEnabled\":false}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));

    handler.Unsubscribe(0, _T("onKeystrokeMaskEnabledChange"), _T("org.rdk.LoggingPreferences"), message);

    dispatcher->Deactivate();
    dispatcher->Release();

    plugin->Deinitialize(nullptr);
}

TEST_F(LoggingPreferencesTestFixture, errorCases)
{
    initService();
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_TRUE(strcmp(methodName, IARM_BUS_SYSMGR_API_GetKeyCodeLoggingPref) == 0);
                return IARM_RESULT_IPCCORE_FAIL;
            })
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_TRUE(strcmp(methodName, IARM_BUS_SYSMGR_API_GetKeyCodeLoggingPref) == 0);
                return IARM_RESULT_IPCCORE_FAIL;
            })
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_TRUE(strcmp(methodName, IARM_BUS_SYSMGR_API_GetKeyCodeLoggingPref) == 0);
                auto* param = static_cast<IARM_BUS_SYSMGR_KEYCodeLoggingInfo_Param_t*>(arg);
                param->logStatus = 0;
                return IARM_RESULT_SUCCESS;
            })
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_TRUE(strcmp(methodName, IARM_BUS_SYSMGR_API_SetKeyCodeLoggingPref) == 0);
                return IARM_RESULT_IPCCORE_FAIL;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isKeystrokeMaskEnabled"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setKeystrokeMaskEnabled"), _T("{\"keystrokeMaskEnabled\":false}"), response));
    EXPECT_EQ(response, _T("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setKeystrokeMaskEnabled"), _T("{\"keystrokeMaskEnabled\":false}"), response));
    EXPECT_EQ(response, _T("{\"success\":false}"));

    plugin->Deinitialize(nullptr);
}
