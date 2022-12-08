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

#include "ActivityMonitor.h"

#include "FactoriesImplementation.h"
#include "ServiceMock.h"
#include "IarmBusMock.h"
#include <fstream>
#include <unistd.h>

using namespace WPEFramework;

namespace {
const string regFile = _T("/opt/waylandregistry.conf");
const uint8_t regFileData[] = "{\"waylandapps\":[{\"name\" : \"memcheck-amd64-\",\"binary\" : \"/usr/lib/x86_64-linux-gnu/valgrind/memcheck-amd64-%\"}]}";
}

class ActivityMonitorTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::ActivityMonitor> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    ActivityMonitorTest()
        : plugin(Core::ProxyType<Plugin::ActivityMonitor>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
        EXPECT_EQ(string(""), plugin->Initialize(nullptr));
    }
    virtual ~ActivityMonitorTest() override
    {
        IarmBus::getInstance().impl = nullptr;
        plugin->Deinitialize(nullptr);
    }
};

TEST_F(ActivityMonitorTest, registeredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("enableMonitoring")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("disableMonitoring")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getApplicationMemoryUsage")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getAllMemoryUsage")));
}

TEST_F(ActivityMonitorTest, disableMonitoring)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("disableMonitoring"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
}

TEST_F(ActivityMonitorTest, enableMonitoringEmptyConfig)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("enableMonitoring"), _T("{\"config\":[],\"memoryIntervalSeconds\":\"0\", \"cpuIntervalSeconds\":\"0\"}"), response));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("enableMonitoring"), _T("{\"config\":[{\"appPid\": 6763, \"memoryThresholdMB\": 10,\"cpuThresholdPercent\": 50,\"cpuThresholdSeconds\": 2}],\"memoryIntervalSeconds\":\"0\", \"cpuIntervalSeconds\":\"0\"}"), response));
}

class ActivityMonitorMemoryUsageTest : public ActivityMonitorTest
{
protected:
    IarmBusImplMock iarmBusImplMock;
    ActivityMonitorMemoryUsageTest() : ActivityMonitorTest()
    {
        //Add version info
        std::ofstream fileVer("/version.txt");
        fileVer << "imagename:PX051AEI_VBN_2203_sprint_20220331225312sdy_NG\nSDK_VERSION=17.3\nMEDIARITE=8.3.53\nYOCTO_VERSION=dunfell\n";
        fileVer.close();

        //Add Apps registry file
        Core::File fileApps(regFile);
        fileApps.Create();
        fileApps.Write(regFileData, sizeof(regFileData));
    }

    virtual ~ActivityMonitorMemoryUsageTest() override
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;
        // Clear file contents
        std::ofstream fileVer("/version.txt");
        fileVer.close();

        // Delete Apps registry file
        Core::File fileApps(regFile);
        fileApps.Destroy();
    }
};

TEST_F(ActivityMonitorMemoryUsageTest, getAllMemoryUsage)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getAllMemoryUsage"), _T(""), response));
    EXPECT_THAT(response, ::testing::MatchesRegex("\\{"
                                                  "\"freeMemoryMB\":[0-9]+,"
                                                  "\"applicationMemory\":"
                                                  "\\[(\\{"
                                                  "\"appPid\":[0-9]+,"
                                                  "\"appName\":\"memcheck-amd64-\","
                                                  "\"memoryMB\":[0-9]+"
                                                  "\\},{0,}){0,}\\],"
                                                  "\"success\":true"
                                                  "\\}"));

}

class ActivityMonitorEventTest : public ActivityMonitorMemoryUsageTest {
protected:
    ServiceMock service;
    Core::JSONRPC::Message message;
    FactoriesImplementation factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    ActivityMonitorEventTest()
        : ActivityMonitorMemoryUsageTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }
    virtual ~ActivityMonitorEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();

        PluginHost::IFactories::Assign(nullptr);
    }
};

TEST_F(ActivityMonitorEventTest, enableMonitoringWithConfig)
{
    Core::Event memoryThresholdEvent(false, true);
    Core::Event cpuThresholdEvent(false, true);
    handler.Subscribe(0, _T("onMemoryThreshold"), _T("org.rdk.ActivityMonitor"), message);
    handler.Subscribe(0, _T("onCPUThreshold"), _T("org.rdk.ActivityMonitor"), message);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(2)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                            "\"jsonrpc\":\"2.0\","
                                                            "\"method\":\"org.rdk.ActivityMonitor.onMemoryThreshold\","
                                                            "\"params\":"
                                                            "\\{"
                                                            "\"appPid\":[0-9]+,"
                                                            "\"threshold\":\"exceeded\","
                                                            "\"memoryMB\":[0-9]+"
                                                            "\\}"
                                                            "\\}")));

                memoryThresholdEvent.SetEvent();
                return Core::ERROR_NONE;
            }))
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                            "\"jsonrpc\":\"2.0\","
                                                            "\"method\":\"org.rdk.ActivityMonitor.onCPUThreshold\","
                                                            "\"params\":"
                                                            "\\{"
                                                            "\"appPid\":[0-9]+,"
                                                            "\"threshold\":\"exceeded\","
                                                            "\"cpuPercent\":[0-9]+"
                                                            "\\}"
                                                            "\\}")));

                cpuThresholdEvent.SetEvent();
                return Core::ERROR_NONE;
            }));

    //Monitor own PID
    unsigned int ownPID = static_cast<unsigned int>(getpid());
    
    string monitorString = "{\"config\":[{\"appPid\":" + std::to_string(ownPID) + ", \"memoryThresholdMB\": 1,\"cpuThresholdPercent\": 1,\"cpuThresholdSeconds\": 2}],\"memoryIntervalSeconds\":\"0.02\", \"cpuIntervalSeconds\":\"0.02\"}";
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("enableMonitoring"), monitorString, response));

    string AppPID = "{\"pid\":" + std::to_string(ownPID) + "}";
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getApplicationMemoryUsage"), AppPID, response));
    EXPECT_THAT(response, ::testing::MatchesRegex("\\{"
                                                  "\"applicationMemory\":"
                                                  "\\{"
                                                  "\"appPid\":[0-9]+,"
                                                  "\"appName\":\"memcheck-amd64-\","
                                                  "\"memoryMB\":[0-9]+"
                                                  "\\},"
                                                  "\"success\":true"
                                                  "\\}"));

    //Wait for events to occur
    EXPECT_EQ(Core::ERROR_NONE, memoryThresholdEvent.Lock());
    EXPECT_EQ(Core::ERROR_NONE, cpuThresholdEvent.Lock());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("disableMonitoring"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}
