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

#include "ServiceMock.h"
#include "source/SystemInfo.h"
#include "source/WorkerPoolImplementation.h"

using namespace WPEFramework;

namespace {
const string webPrefix = _T("/Service/LocationSync");
const unsigned int waitRetries = 2000;
const unsigned int waitInterval = 100;
}

class LocationSyncTestFixture : public ::testing::Test {
protected:
    Core::ProxyType<WorkerPoolImplementation> workerPool;
    Core::Sink<SystemInfo> subSystem;
    Core::ProxyType<Plugin::LocationSync> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Context connection{};
    ServiceMock service;
    string response;

    LocationSyncTestFixture()
        : workerPool(Core::ProxyType<WorkerPoolImplementation>::Create(
            2, Core::Thread::DefaultStackSize(), 16))
        , plugin(Core::ProxyType<Plugin::LocationSync>::Create())
        , handler(*plugin)
    {
    }
    virtual ~LocationSyncTestFixture()
    {
    }

    virtual void SetUp()
    {
        Core::IWorkerPool::Assign(&(*workerPool));

        workerPool->Run();
    }

    virtual void TearDown()
    {
        plugin.Release();

        Core::IWorkerPool::Assign(nullptr);
        workerPool.Release();
    }
};

TEST_F(LocationSyncTestFixture, probeTest)
{
    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(::testing::Return("{\n"
                                    "  \"interval\":10,\n"
                                    "  \"retries\":20,\n"
                                    "  \"source\":\"http://jsonip.metrological.com/?maf=true\"\n"
                                    " }"));

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

    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("sync")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("location")));

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_INPROGRESS, handler.Invoke(connection, _T("sync"), _T("{}"), response));
    EXPECT_EQ(response,
        _T(""));

    Core::Event wait(false, true);
    for (unsigned int i = 0; ((subSystem.Get(PluginHost::ISubSystem::LOCATION) == nullptr) && (i < waitRetries)); i++) {
        wait.Lock(waitInterval);
    }

    EXPECT_TRUE(subSystem.Get(PluginHost::ISubSystem::LOCATION) != nullptr);
    EXPECT_TRUE(subSystem.Get(PluginHost::ISubSystem::INTERNET) != nullptr);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("location"), _T(""), response));
    EXPECT_TRUE(response.empty() == false);

    JsonObject params;

    EXPECT_TRUE(params.FromString(response));
    EXPECT_TRUE(params.HasLabel("city"));
    EXPECT_TRUE(params.HasLabel("country"));
    EXPECT_TRUE(params.HasLabel("region"));
    EXPECT_TRUE(params.HasLabel("timezone"));
    EXPECT_TRUE(params.HasLabel("publicip"));

    plugin->Deinitialize(&service);
}
