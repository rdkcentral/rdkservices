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
#include <gmock/gmock.h>

#include "Warehouse.h"

#include "FactoriesImplementation.h"
#include "IarmBusMock.h"
#include "ServiceMock.h"

using namespace WPEFramework;

class WarehouseTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::Warehouse> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    WarehouseTest()
        : plugin(Core::ProxyType<Plugin::Warehouse>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
    }

    virtual ~WarehouseTest()
    {
        plugin.Release();
    }
};

class WarehouseInitializedTest : public WarehouseTest {
protected:
    IarmBusImplMock iarmBusImplMock;

    WarehouseInitializedTest()
             : WarehouseTest()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;

        EXPECT_EQ(string(""), plugin->Initialize(nullptr));
    }
    virtual ~WarehouseInitializedTest() override
    {
        plugin->Deinitialize(nullptr);
        IarmBus::getInstance().impl = nullptr;
    }
};

class WarehouseEventTest : public WarehouseInitializedTest {
protected:
    ServiceMock service;
    Core::JSONRPC::Message message;
    FactoriesImplementation factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    WarehouseEventTest()
        : WarehouseInitializedTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }
    virtual ~WarehouseEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();

        PluginHost::IFactories::Assign(nullptr);
    }
};

TEST_F(WarehouseTest, registeredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("resetDevice")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getDeviceInfo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setFrontPanelState")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("internalReset")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("lightReset")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("isClean")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("executeHardwareTest")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getHardwareTestResults")));
}

TEST_F(WarehouseEventTest, resetDevice)
{
    Core::Event resetDone(false, true);
    unsigned int userFactoryResetCnt = 0;

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(6)
        // called by WareHouseResetIARM
        .WillRepeatedly(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                //Check for error response for the second call - Negative test case
                if(userFactoryResetCnt == 2)
                {
                    userFactoryResetCnt = 0;
                    EXPECT_EQ(text, string(_T("{"
                                            "\"jsonrpc\":\"2.0\","
                                            "\"method\":\"org.rdk.Warehouse.resetDone\","
                                            "\"params\":{\"success\":false,"
                                            "\"error\":\"Reset failed\"}"
                                            "}")));
                }
                else
                {
                    EXPECT_EQ(text, string(_T("{"
                                            "\"jsonrpc\":\"2.0\","
                                            "\"method\":\"org.rdk.Warehouse.resetDone\","
                                            "\"params\":{\"success\":true}"
                                            "}")));
                }
                resetDone.SetEvent();
                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("resetDone"), _T("org.rdk.Warehouse"), message);

    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [&](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                if (string(methodName) == string(_T(IARM_BUS_PWRMGR_API_UserFactoryReset)))
                {
                    //Throw error for the second call - Negative test case
                    if(++userFactoryResetCnt == 2)
                    {
                        return IARM_RESULT_INVALID_PARAM;
                    }
                }
                return IARM_RESULT_SUCCESS;
            });

    //reset: suppress reboot: true, type: COLD
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("resetDevice"), _T("{\"suppressReboot\":true,\"resetType\":\"COLD\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, resetDone.Lock());

    //reset: suppress reboot: true, type: FACTORY
    resetDone.ResetEvent();
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("resetDevice"), _T("{\"suppressReboot\":true,\"resetType\":\"FACTORY\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, resetDone.Lock());

    //reset: suppress reboot: true, type: USERFACTORY
    resetDone.ResetEvent();
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("resetDevice"), _T("{\"suppressReboot\":true,\"resetType\":\"USERFACTORY\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, resetDone.Lock());

    //reset: suppress reboot: true, type: USERFACTORY
    //Same as above test case, but return error from EXPECT_CALL above
    resetDone.ResetEvent();
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("resetDevice"), _T("{\"suppressReboot\":true,\"resetType\":\"USERFACTORY\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, resetDone.Lock());

    //reset: suppress reboot: false, type: WAREHOUSE_CLEAR
    resetDone.ResetEvent();
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("resetDevice"), _T("{\"suppressReboot\":false,\"resetType\":\"WAREHOUSE_CLEAR\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, resetDone.Lock());

    //reset: suppress reboot: false
    resetDone.ResetEvent();
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("resetDevice"), _T("{\"suppressReboot\":false}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, resetDone.Lock());

    handler.Unsubscribe(0, _T("resetDone"), _T("org.rdk.Warehouse"), message);
}
