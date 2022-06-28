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
#include "Timer.h"
#include "ServiceMock.h"
#include "IarmBusMock.h"
#include "FactoriesImplementation.h"

using namespace WPEFramework;

namespace {
const string iarmName = _T("Thunder_Plugins");
}

class TimerTestFixture : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::Timer> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;
    ServiceMock service;
    Core::JSONRPC::Message message;
    IarmBusImplMock iarmBusImplMock;
    FactoriesImplementation factoriesImplementation;

    TimerTestFixture()
        : plugin(Core::ProxyType<Plugin::Timer>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);
    }
    virtual ~TimerTestFixture()
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

TEST_F(TimerTestFixture, registeredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("startTimer")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("cancel")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("suspend")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("resume")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getTimerStatus")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getTimers")));
}

TEST_F(TimerTestFixture, paramsMissing)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startTimer"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("cancel"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("suspend"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("resume"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTimerStatus"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"success\":false}"));
}

TEST_F(TimerTestFixture, jsonRpc)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startTimer"), 
                _T("{\"interval\":10, \"repeatInterval\":15, \"remindBefore\":5}"), response));

    //Extract timer id from response
    JsonObject params;
    EXPECT_TRUE(params.FromString(response));
    EXPECT_TRUE(params.HasLabel(_T("timerId")));
    string sTimerID = params["timerId"].String();
    std::string sTimerStrID = "{\"timerId\":" + sTimerID + "}";

    //Compare response from startTimer call
    std::string sRespCompare = "{\"timerId\":" + sTimerID + ",\"success\":true}";
    EXPECT_EQ(response, sRespCompare.c_str());

    //get timer status
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTimerStatus"), sTimerStrID.c_str(), response));
    EXPECT_TRUE(params.FromString(response));

    //match the response as per https://rdkcentral.github.io/rdkservices/#/api/TimerPlugin?id=gettimerstatus
    EXPECT_TRUE(params.HasLabel("state"));
    EXPECT_TRUE(params.HasLabel("mode"));
    EXPECT_TRUE(params.HasLabel("timeRemaining"));
    EXPECT_TRUE(params.HasLabel("repeatInterval"));
    EXPECT_TRUE(params.HasLabel("remindBefore"));
    EXPECT_TRUE(params.HasLabel("success"));
    EXPECT_EQ(_T("RUNNING"), params[_T("state")].Value());
    EXPECT_EQ(_T("GENERIC"), params[_T("mode")].Value());
    EXPECT_GT(std::stoi(params["timeRemaining"].String()), 0);
    EXPECT_GT(std::stoi(params["repeatInterval"].String()), 0);
    EXPECT_GT(std::stoi(params["remindBefore"].String()), 0);

    //get all timers
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTimers"), _T("{}"), response));
    EXPECT_TRUE(params.FromString(response));
    EXPECT_TRUE(params.HasLabel("timers"));

    //match the response as per https://rdkcentral.github.io/rdkservices/#/api/TimerPlugin?id=gettimers
    JsonArray timersArray = params["timers"].Array();
    EXPECT_GT(timersArray.Length(), 0);
    JsonObject timerStatus = timersArray[0].String();

    EXPECT_TRUE(timerStatus.HasLabel("timerId"));
    EXPECT_TRUE(timerStatus.HasLabel("state"));
    EXPECT_TRUE(timerStatus.HasLabel("mode"));
    EXPECT_TRUE(timerStatus.HasLabel("timeRemaining"));
    EXPECT_TRUE(timerStatus.HasLabel("repeatInterval"));
    EXPECT_TRUE(timerStatus.HasLabel("remindBefore"));
    EXPECT_EQ(std::stoi(timerStatus["timerId"].String()), 0);
    EXPECT_EQ(_T("RUNNING"), timerStatus[_T("state")].Value());
    EXPECT_EQ(_T("GENERIC"), timerStatus[_T("mode")].Value());
    EXPECT_GT(std::stoi(timerStatus["timeRemaining"].String()), 0);
    EXPECT_GT(std::stoi(timerStatus["repeatInterval"].String()), 0);
    EXPECT_GT(std::stoi(timerStatus["remindBefore"].String()), 0);

    //Suspend the timer
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("suspend"), sTimerStrID.c_str(), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));

    //Resume the timer
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("resume"), sTimerStrID.c_str(), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));

    //Cancel the timer
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("cancel"), sTimerStrID.c_str(), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
}

TEST_F(TimerTestFixture, timerExpiry)
{
    /////////////// IARM expectations - Begin ////////////////////////////

    // called by Timer::InitializeIARM, Timer::DeinitializeIARM
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

    // called by Timer::InitializeIARM
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Init(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](const char* name) {
                if (iarmName == string(name)) {
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            }));

    // called by Timer::InitializeIARM
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Connect())
        .Times(1)
        .WillOnce(::testing::Return(IARM_RESULT_SUCCESS));

    /////////////// IARM expectations - End //////////////////////////////

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(2)
        // called by Timer::sendTimerExpiryReminder
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.Timer.timerExpiryReminder\","
                                          "\"params\":{\"timerId\":0,"
                                           "\"mode\":\"WAKE\","
                                           "\"timeRemaining\":0}"
                                          "}")));

                return Core::ERROR_NONE;
            }))
    
        // called by Timer::sendTimerExpired
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.Timer.timerExpired\","
                                          "\"params\":{\"timerId\":0,"
                                           "\"mode\":\"WAKE\","
                                           "\"status\":0}"
                                          "}")));

                return Core::ERROR_NONE;
            }));

    // Initialize
    EXPECT_EQ(string(""), plugin->Initialize(nullptr));

    //Create a timer of 0.2 sec with a reminder of 0.1 sec
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startTimer"), 
                        _T("{\"interval\":0.2, \"mode\":\"WAKE\", \"remindBefore\": 0.1}"), response));
    EXPECT_THAT(response, testing::HasSubstr("timerId"));
    EXPECT_THAT(response, testing::HasSubstr("\"success\":true"));

    //Extract timer id from response
    JsonObject params;
    EXPECT_TRUE(params.FromString(response));
    EXPECT_TRUE(params.HasLabel(_T("timerId")));

    // JSON-RPC events
    auto dispatcher = static_cast<PluginHost::IDispatcher*>(
        plugin->QueryInterface(PluginHost::IDispatcher::ID));
    EXPECT_TRUE(dispatcher != nullptr);

    dispatcher->Activate(&service);

    handler.Subscribe(0, _T("timerExpiryReminder"), _T("org.rdk.Timer"), message);
    handler.Subscribe(0, _T("timerExpired"), _T("org.rdk.Timer"), message);

    //Wait for timer expiry & reminder calls
    Core::Event wait(false, true);
    wait.Lock(200);

    handler.Unsubscribe(0, _T("timerExpiryReminder"), _T("org.rdk.Timer"), message);
    handler.Unsubscribe(0, _T("timerExpired"), _T("org.rdk.Timer"), message);

    dispatcher->Deactivate();
    dispatcher->Release();

    // Deinitialize
    plugin->Deinitialize(nullptr);
}
