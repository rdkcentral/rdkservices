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

using ::testing::NiceMock;

class LoggingPreferencesTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::LoggingPreferences> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    LoggingPreferencesTest()
        : plugin(Core::ProxyType<Plugin::LoggingPreferences>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
    }
    virtual ~LoggingPreferencesTest() = default;
};

class LoggingPreferencesInitializedTest : public LoggingPreferencesTest {
protected:
    IarmBusImplMock         *p_iarmBusImplMock = nullptr ;

    LoggingPreferencesInitializedTest()
        : LoggingPreferencesTest()
    {
        p_iarmBusImplMock  = new NiceMock <IarmBusImplMock>;
        IarmBus::setImpl(p_iarmBusImplMock);


        EXPECT_EQ(string(""), plugin->Initialize(nullptr));
    }
    virtual ~LoggingPreferencesInitializedTest() override
    {
        plugin->Deinitialize(nullptr);

        IarmBus::setImpl(nullptr);
        if (p_iarmBusImplMock != nullptr)
        {
            delete p_iarmBusImplMock;
            p_iarmBusImplMock = nullptr;
        }

    }
};

class LoggingPreferencesInitializedEventTest : public LoggingPreferencesInitializedTest {
protected:
    NiceMock<ServiceMock> service;
    Core::JSONRPC::Message message;
    NiceMock<FactoriesImplementation> factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    LoggingPreferencesInitializedEventTest()
        : LoggingPreferencesInitializedTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }
    virtual ~LoggingPreferencesInitializedEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();

        PluginHost::IFactories::Assign(nullptr);
    }
};

TEST_F(LoggingPreferencesTest, registeredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("isKeystrokeMaskEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setKeystrokeMaskEnabled")));
}

TEST_F(LoggingPreferencesTest, paramsMissing)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setKeystrokeMaskEnabled"), _T("{}"), response));
}

TEST_F(LoggingPreferencesInitializedTest, getKeystrokeMask)
{
    ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                if (strcmp(methodName, IARM_BUS_SYSMGR_API_GetKeyCodeLoggingPref) == 0) {
                    auto* param = static_cast<IARM_BUS_SYSMGR_KEYCodeLoggingInfo_Param_t*>(arg);
                    param->logStatus = 1; //Setting 1 returns response as false
                }
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isKeystrokeMaskEnabled"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"keystrokeMaskEnabled\":false,\"success\":true}"));
}

TEST_F(LoggingPreferencesInitializedEventTest, enableKeystrokeMask)
{
    Core::Event onKeystrokeMaskEnabledChange(false, true);

    ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                if (strcmp(methodName, IARM_BUS_SYSMGR_API_GetKeyCodeLoggingPref) == 0) {
                    auto* param = static_cast<IARM_BUS_SYSMGR_KEYCodeLoggingInfo_Param_t*>(arg);
                    param->logStatus = 1;
                }
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

                onKeystrokeMaskEnabledChange.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onKeystrokeMaskEnabledChange"), _T("org.rdk.LoggingPreferences"), message);

    //Simulating the case for setting the same value again
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setKeystrokeMaskEnabled"), _T("{\"keystrokeMaskEnabled\":false}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setKeystrokeMaskEnabled"), _T("{\"keystrokeMaskEnabled\":true}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, onKeystrokeMaskEnabledChange.Lock());

    handler.Unsubscribe(0, _T("onKeystrokeMaskEnabledChange"), _T("org.rdk.LoggingPreferences"), message);
}

TEST_F(LoggingPreferencesInitializedEventTest, disbleKeystrokeMask)
{
    Core::Event onKeystrokeMaskEnabledChange(false, true);

    ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                if (strcmp(methodName, IARM_BUS_SYSMGR_API_GetKeyCodeLoggingPref) == 0) {
                    auto* param = static_cast<IARM_BUS_SYSMGR_KEYCodeLoggingInfo_Param_t*>(arg);
                    param->logStatus = 0;
                }
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

                onKeystrokeMaskEnabledChange.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onKeystrokeMaskEnabledChange"), _T("org.rdk.LoggingPreferences"), message);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setKeystrokeMaskEnabled"), _T("{\"keystrokeMaskEnabled\":false}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, onKeystrokeMaskEnabledChange.Lock());

    handler.Unsubscribe(0, _T("onKeystrokeMaskEnabledChange"), _T("org.rdk.LoggingPreferences"), message);
}

TEST_F(LoggingPreferencesInitializedTest, errorCases)
{
    ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                if (strcmp(methodName, IARM_BUS_SYSMGR_API_GetKeyCodeLoggingPref) == 0) {
                    return IARM_RESULT_IPCCORE_FAIL;
                } else if (strcmp(methodName, IARM_BUS_SYSMGR_API_GetKeyCodeLoggingPref) == 0) {
                    auto* param = static_cast<IARM_BUS_SYSMGR_KEYCodeLoggingInfo_Param_t*>(arg);
                    param->logStatus = 0;
                }
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("isKeystrokeMaskEnabled"), _T("{}"), response));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setKeystrokeMaskEnabled"), _T("{\"keystrokeMaskEnabled\":false}"), response));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setKeystrokeMaskEnabled"), _T("{\"keystrokeMaskEnabled\":false}"), response));
}
