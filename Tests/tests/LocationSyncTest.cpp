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

#include <gtest/gtest.h>

#include "LocationSync.h"

#include "FactoriesImplementation.h"
#include "ServiceMock.h"
#include "source/SystemInfo.h"
#include "source/WorkerPoolImplementation.h"

#include <future>

using namespace WPEFramework;

namespace {
const string webPrefix = _T("/Service/LocationSync");
}

class LocationSyncTestFixture : public ::testing::Test {
protected:
    Core::ProxyType<WorkerPoolImplementation> workerPool;
    FactoriesImplementation factoriesImplementation;

    ServiceMock service;
    Core::Sink<SystemInfo> subSystem;

    Core::ProxyType<Plugin::LocationSync> plugin;
    Core::JSONRPC::Handler& handler;

    Core::JSONRPC::Connection connection;
    Core::JSONRPC::Message message;
    string response;

    Core::Event locationchange;

    LocationSyncTestFixture()
        : workerPool(Core::ProxyType<WorkerPoolImplementation>::Create(
            5, Core::Thread::DefaultStackSize(), 16))
        , plugin(Core::ProxyType<Plugin::LocationSync>::Create())
        , handler(*plugin)
        , connection(1, 0)
        , locationchange(false, true)
    {
    }
    virtual ~LocationSyncTestFixture()
    {
    }

    virtual void SetUp()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);
        Core::IWorkerPool::Assign(&(*workerPool));
        workerPool->Run();
    }

    virtual void TearDown()
    {
        plugin.Release();

        Core::IWorkerPool::Assign(nullptr);
        workerPool.Release();
        PluginHost::IFactories::Assign(nullptr);
    }
};

TEST_F(LocationSyncTestFixture, registeredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("sync")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("location")));
}

TEST_F(LocationSyncTestFixture, activate_locationchange_location_deactivate)
{
    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(::testing::Return("{\n"
                                    "\"interval\":10,\n"
                                    "\"retries\":20,\n"
                                    "\"source\":\"http://jsonip.metrological.com/?maf=true\"\n"
                                    "}"));
    EXPECT_CALL(service, WebPrefix())
        .Times(1)
        .WillOnce(::testing::Return(webPrefix));
    EXPECT_CALL(service, SubSystems())
        .Times(2)
        .WillRepeatedly(::testing::Invoke(
            [&]() {
                PluginHost::ISubSystem* result = (&subSystem);
                result->AddRef();
                return result;
            }));
    ON_CALL(service, Version())
        .WillByDefault(::testing::Return(string()));
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

    auto dispatcher = static_cast<PluginHost::IDispatcher*>(
        plugin->QueryInterface(PluginHost::IDispatcher::ID));
    EXPECT_TRUE(dispatcher != nullptr);
    dispatcher->Activate(&service);
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
    dispatcher->Deactivate();
    dispatcher->Release();
    plugin->Deinitialize(&service);
}

TEST_F(LocationSyncTestFixture, activate_sync_deactivate)
{
    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(::testing::Return("{\n"
                                    "\"interval\":10,\n"
                                    "\"retries\":20,\n"
                                    "\"source\":\"http://jsonip.metrological.com/?maf=true\"\n"
                                    "}"));
    EXPECT_CALL(service, WebPrefix())
        .Times(1)
        .WillOnce(::testing::Return(webPrefix));
    ON_CALL(service, SubSystems())
        .WillByDefault(::testing::Invoke(
            [&]() {
                PluginHost::ISubSystem* result = (&subSystem);
                result->AddRef();
                return result;
            }));
    ON_CALL(service, Version())
        .WillByDefault(::testing::Return(string()));

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_INPROGRESS, handler.Invoke(connection, _T("sync"), _T("{}"), response));

    plugin->Deinitialize(&service);
}

TEST_F(LocationSyncTestFixture, activateWithIncorrectUrl)
{
    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(::testing::Return("{\n"
                                    "\"interval\":10,\n"
                                    "\"retries\":20,\n"
                                    "\"source\":\"http://0.0.0.0\"\n"
                                    "}"));
    ON_CALL(service, Version())
        .WillByDefault(::testing::Return(string()));

    EXPECT_EQ(string("URL for retrieving location is incorrect !!!"), plugin->Initialize(&service));
}

TEST_F(LocationSyncTestFixture, activateWithUnreachableHost_locationchange_location_deactivate)
{
    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(::testing::Return("{\n"
                                    "\"interval\":1,\n"
                                    "\"retries\":1,\n"
                                    "\"source\":\"http://jsonip.metrological.com:1234/?maf=true\"\n"
                                    "}"));
    EXPECT_CALL(service, WebPrefix())
        .Times(1)
        .WillOnce(::testing::Return(webPrefix));
    EXPECT_CALL(service, SubSystems())
        .Times(2)
        .WillOnce(::testing::Invoke(
            [&]() {
                PluginHost::ISubSystem* result = (&subSystem);
                result->AddRef();

                locationchange.SetEvent();

                return result;
            }))
        .WillOnce(::testing::Invoke(
            [&]() {
                PluginHost::ISubSystem* result = (&subSystem);
                result->AddRef();
                return result;
            }));
    ON_CALL(service, Version())
        .WillByDefault(::testing::Return(string()));

    auto dispatcher = static_cast<PluginHost::IDispatcher*>(
        plugin->QueryInterface(PluginHost::IDispatcher::ID));
    EXPECT_TRUE(dispatcher != nullptr);
    dispatcher->Activate(&service);
    handler.Subscribe(0, _T("locationchange"), _T("LocationSync"), message);

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, locationchange.Lock(100000)); // 100s

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

    handler.Unsubscribe(0, _T("locationchange"), _T("LocationSync"), message);
    dispatcher->Deactivate();
    dispatcher->Release();
    plugin->Deinitialize(&service);
}

TEST_F(LocationSyncTestFixture, activate_locationchange_sync_locationchange_sync_locationchange_deactivate)
{
    int iteration = 0;
    int numIterations = 2;

    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(::testing::Return("{\n"
                                    "\"interval\":10,\n"
                                    "\"retries\":20,\n"
                                    "\"source\":\"http://jsonip.metrological.com/?maf=true\"\n"
                                    "}"));
    EXPECT_CALL(service, WebPrefix())
        .Times(1)
        .WillOnce(::testing::Return(webPrefix));
    EXPECT_CALL(service, SubSystems())
        .Times(numIterations + 1)
        .WillRepeatedly(::testing::Invoke(
            [&]() {
                PluginHost::ISubSystem* result = (&subSystem);
                result->AddRef();
                return result;
            }));
    ON_CALL(service, Version())
        .WillByDefault(::testing::Return(string()));
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(numIterations + 1)
        .WillRepeatedly(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>&) {
                if (iteration++ < numIterations) {
                    std::thread t([&]() {
                        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sync"), _T("{}"), response));
                    });
                    t.detach();
                } else {
                    locationchange.SetEvent();
                }

                return Core::ERROR_NONE;
            }));

    auto dispatcher = static_cast<PluginHost::IDispatcher*>(
        plugin->QueryInterface(PluginHost::IDispatcher::ID));
    EXPECT_TRUE(dispatcher != nullptr);
    dispatcher->Activate(&service);
    handler.Subscribe(0, _T("locationchange"), _T("LocationSync"), message);

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, locationchange.Lock(100000)); // 100s

    handler.Unsubscribe(0, _T("locationchange"), _T("LocationSync"), message);
    dispatcher->Deactivate();
    dispatcher->Release();
    plugin->Deinitialize(&service);
}
