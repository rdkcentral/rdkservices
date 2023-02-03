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

#include "FactoriesImplementation.h"
#include "IarmBusMock.h"
#include "ServiceMock.h"

using namespace WPEFramework;

using ::testing::Eq;

class TimerTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::Timer> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    TimerTest()
        : plugin(Core::ProxyType<Plugin::Timer>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
    }
    virtual ~TimerTest() = default;
};

class TimerInitializedTest : public TimerTest {
protected:
    IarmBusImplMock iarmBusImplMock;

    TimerInitializedTest()
        : TimerTest()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;

        EXPECT_EQ(string(""), plugin->Initialize(nullptr));
    }
    virtual ~TimerInitializedTest() override
    {
        plugin->Deinitialize(nullptr);

        IarmBus::getInstance().impl = nullptr;
    }
};

class TimerInitializedEventTest : public TimerInitializedTest {
protected:
    ServiceMock service;
    Core::JSONRPC::Message message;
    FactoriesImplementation factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    TimerInitializedEventTest()
        : TimerInitializedTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }
    virtual ~TimerInitializedEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();

        PluginHost::IFactories::Assign(nullptr);
    }
};

TEST_F(TimerTest, registeredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("startTimer")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("cancel")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("suspend")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("resume")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getTimerStatus")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getTimers")));
}

TEST_F(TimerTest, paramsMissing)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startTimer"), _T("{}"), response));
    EXPECT_THAT(response, Eq("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("cancel"), _T("{}"), response));
    EXPECT_THAT(response, Eq("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("suspend"), _T("{}"), response));
    EXPECT_THAT(response, Eq("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("resume"), _T("{}"), response));
    EXPECT_THAT(response, Eq("{\"success\":false}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTimerStatus"), _T("{}"), response));
    EXPECT_THAT(response, Eq("{\"success\":false}"));
}

TEST_F(TimerTest, jsonRpc)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startTimer"), _T("{\"interval\":10,\"repeatInterval\":15,\"remindBefore\":5}"), response));
    EXPECT_EQ(response, _T("{\"timerId\":0,\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTimerStatus"), _T("{\"timerId\":0}"), response));
    EXPECT_THAT(response, ::testing::MatchesRegex(_T("\\{"
                                                     "\"state\":\"RUNNING\","
                                                     "\"mode\":\"GENERIC\","
                                                     "\"timeRemaining\":\"[0-9]+.[0-9]+\","
                                                     "\"repeatInterval\":\"15.000\","
                                                     "\"remindBefore\":\"5.000\","
                                                     "\"success\":true"
                                                     "\\}")));
    //Get timer status - Negative test case - get status of a wrong timer
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTimerStatus"), _T("{\"timerId\":10}"), response));
    EXPECT_THAT(response, Eq("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTimers"), _T("{}"), response));
    EXPECT_THAT(response, ::testing::MatchesRegex(_T("\\{"
                                                     "\"timers\":\\["
                                                     "\\{"
                                                     "\"timerId\":0,"
                                                     "\"state\":\"RUNNING\","
                                                     "\"mode\":\"GENERIC\","
                                                     "\"timeRemaining\":\"[0-9]+.[0-9]+\","
                                                     "\"repeatInterval\":\"15.000\","
                                                     "\"remindBefore\":\"5.000\""
                                                     "\\}"
                                                     "\\],"
                                                     "\"success\":true"
                                                     "\\}")));
    //Suspend the timer
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("suspend"), _T("{\"timerId\":0}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    //Suspend the timer - Negative test case - Suspend an already suspended timer
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("suspend"), _T("{\"timerId\":0}"), response));
    EXPECT_THAT(response, Eq("{\"success\":false}"));
    //Suspend the timer - Negative test case - Suspend a wrong timer
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("suspend"), _T("{\"timerId\":10}"), response));
    EXPECT_THAT(response, Eq("{\"success\":false}"));

    //Resume the timer
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("resume"), _T("{\"timerId\":0}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    //Resume the timer - Negative test case - Resume an already resumed timer
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("resume"), _T("{\"timerId\":0}"), response));
    EXPECT_THAT(response, Eq("{\"success\":false}"));
    //Resume the timer - Negative test case - Resume a wrong timer
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("resume"), _T("{\"timerId\":10}"), response));
    EXPECT_THAT(response, Eq("{\"success\":false}"));

    //Cancel the timer
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("cancel"), _T("{\"timerId\":0}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    //Cancel the timer - Negative test case - Cancel an already cancelled timer
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("cancel"), _T("{\"timerId\":0}"), response));
    EXPECT_THAT(response, Eq("{\"success\":false}"));
    //Cancel the timer - Negative test case - Cancel a wrong timer
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("cancel"), _T("{\"timerId\":10}"), response));
    EXPECT_THAT(response, Eq("{\"success\":false}"));
}

/**
 * Segmentation fault without valgrind
 */
TEST_F(TimerInitializedEventTest, DISABLED_timerExpiry)
{
    Core::Event timerExpiryReminder(false, true);
    Core::Event timerExpired(false, true);

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

                timerExpiryReminder.SetEvent();

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

                timerExpired.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("timerExpiryReminder"), _T("org.rdk.Timer"), message);
    handler.Subscribe(0, _T("timerExpired"), _T("org.rdk.Timer"), message);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startTimer"), _T("{\"interval\":0.2,\"mode\":\"WAKE\",\"remindBefore\":0.1}"), response));
    EXPECT_EQ(response, _T("{\"timerId\":0,\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, timerExpiryReminder.Lock());
    EXPECT_EQ(Core::ERROR_NONE, timerExpired.Lock());

    handler.Unsubscribe(0, _T("timerExpiryReminder"), _T("org.rdk.Timer"), message);
    handler.Unsubscribe(0, _T("timerExpired"), _T("org.rdk.Timer"), message);
}
