/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2020 RDK Management
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "LocationSync.h"

#include "FactoriesImplementation.h"
#include "ServiceMock.h"
#include "SystemInfo.h"
#include "WorkerPoolImplementation.h"

#include <future>

using namespace WPEFramework;

using ::testing::NiceMock;

namespace {
const string webPrefix = _T("/Service/LocationSync");
}

namespace {
class SystemInfoMock : public SystemInfo {
public:
    virtual ~SystemInfoMock() = default;
    MOCK_METHOD(void, Set, (const subsystem type, WPEFramework::Core::IUnknown* information), (override));
};
}

class LocationSyncTest : public ::testing::Test {
protected:
    Core::ProxyType<WorkerPoolImplementation> workerPool;
    NiceMock<FactoriesImplementation> factoriesImplementation;
    NiceMock<ServiceMock> service;
    Core::Sink<NiceMock<SystemInfoMock>> subSystem;
    Core::ProxyType<Plugin::LocationSync> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    Core::JSONRPC::Message message;
    string response;
    PluginHost::IDispatcher* dispatcher;

    LocationSyncTest()
        : workerPool(Core::ProxyType<WorkerPoolImplementation>::Create(
            5, Core::Thread::DefaultStackSize(), 16))
        , plugin(Core::ProxyType<Plugin::LocationSync>::Create())
        , handler(*plugin)
        , connection(1, 0)
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);
        Core::IWorkerPool::Assign(&(*workerPool));
        workerPool->Run();

        ON_CALL(service, ConfigLine())
            .WillByDefault(::testing::Return("{\n"
                                             "\"interval\":10,\n"
                                             "\"retries\":20,\n"
                                             "\"source\":\"http://jsonip.metrological.com/?maf=true\"\n"
                                             "}"));
        ON_CALL(service, WebPrefix())
            .WillByDefault(::testing::Return(webPrefix));
        ON_CALL(service, SubSystems())
            .WillByDefault(::testing::Invoke(
                [&]() {
                    PluginHost::ISubSystem* result = (&subSystem);
                    result->AddRef();
                    return result;
                }));
        ON_CALL(subSystem, Set(::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const PluginHost::ISubSystem::subsystem type, WPEFramework::Core::IUnknown* information) {
                    subSystem.SystemInfo::Set(type, information);
                }));

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }
    virtual ~LocationSyncTest()
    {
        dispatcher->Deactivate();
        dispatcher->Release();

        plugin.Release();

        Core::IWorkerPool::Assign(nullptr);
        workerPool.Release();
        PluginHost::IFactories::Assign(nullptr);
    }
};

TEST_F(LocationSyncTest, activate_locationchange_location_deactivate)
{
    Core::Event locationchange(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
                                          "\"method\":\"LocationSync.locationchange\","
                                          "\"params\":\"\""
                                          "}")));

                locationchange.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("locationchange"), _T("LocationSync"), message);

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, locationchange.Lock(100000)); // 100s

    EXPECT_TRUE(subSystem.Get(PluginHost::ISubSystem::LOCATION) != nullptr);
    EXPECT_TRUE(subSystem.Get(PluginHost::ISubSystem::INTERNET) != nullptr);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("location"), _T(""), response));
    EXPECT_THAT(response, ::testing::MatchesRegex("\\{"
                                                  "\"city\":\".*\","
                                                  "\"country\":\".*\","
                                                  "\"region\":\".*\","
                                                  "\"timezone\":\".+\","
                                                  "\"publicip\":\".+\""
                                                  "\\}"));

    handler.Unsubscribe(0, _T("locationchange"), _T("LocationSync"), message);

    plugin->Deinitialize(&service);
}

TEST_F(LocationSyncTest, activate_sync_deactivate)
{
    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_INPROGRESS, handler.Invoke(connection, _T("sync"), _T("{}"), response));

    plugin->Deinitialize(&service);
}

TEST_F(LocationSyncTest, activateWithIncorrectUrl)
{
    ON_CALL(service, ConfigLine())
        .WillByDefault(::testing::Return("{\n"
                                         "\"interval\":10,\n"
                                         "\"retries\":20,\n"
                                         "\"source\":\"http://0.0.0.0\"\n"
                                         "}"));

    EXPECT_EQ(string("URL for retrieving location is incorrect !!!"), plugin->Initialize(&service));
}

TEST_F(LocationSyncTest, activateWithUnreachableHost_location_deactivate)
{
    Core::Event location(false, true);
    Core::Event internet(false, true);

    ON_CALL(service, ConfigLine())
        .WillByDefault(::testing::Return("{\n"
                                         "\"interval\":1,\n"
                                         "\"retries\":1,\n"
                                         "\"source\":\"http://jsonip.metrological.com:1234/?maf=true\"\n"
                                         "}"));
    EXPECT_CALL(subSystem, Set(::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const PluginHost::ISubSystem::subsystem type, WPEFramework::Core::IUnknown* information) {
                subSystem.SystemInfo::Set(type, information);
                if (type == PluginHost::ISubSystem::LOCATION) {
                    location.SetEvent();
                } else if (type == PluginHost::ISubSystem::INTERNET) {
                    internet.SetEvent();
                }
            }));

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, location.Lock(100000)); // 100s
    EXPECT_EQ(Core::ERROR_NONE, internet.Lock(100000)); // 100s

    EXPECT_TRUE(subSystem.Get(PluginHost::ISubSystem::LOCATION) != nullptr);
    EXPECT_TRUE(subSystem.Get(PluginHost::ISubSystem::INTERNET) != nullptr);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("location"), _T(""), response));
    EXPECT_THAT(response, ::testing::MatchesRegex("\\{"
                                                  "\"city\":\"\","
                                                  "\"country\":\"\","
                                                  "\"region\":\"\","
                                                  "\"timezone\":\"\","
                                                  "\"publicip\":\"\""
                                                  "\\}"));

    plugin->Deinitialize(&service);
}
