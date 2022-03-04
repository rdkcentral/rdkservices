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

#include "gtest/gtest.h"

#include "LocationSync.h"

#include "Source/WorkerPoolImplementation.h"
#include "Source/Config.h"
#include "Source/Service.h"

namespace RdkServicesTest {

TEST(LocationSyncTest, test) {
    // assign worker pool

    auto _engine = WPEFramework::Core::ProxyType<WorkerPoolImplementation>::Create(2, WPEFramework::Core::Thread::DefaultStackSize(), 16);
    WPEFramework::Core::IWorkerPool::Assign(&(*_engine));
    _engine->Run();

    // create plugin

    auto locationSync = WPEFramework::Core::ProxyType <WPEFramework::Plugin::LocationSync>::Create();

    WPEFramework::Core::JSONRPC::Handler& handler = *locationSync;

    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Exists(_T("sync")));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Exists(_T("location")));

    // init plugin

    WPEFramework::Core::File serverConf(string("thunder/install/etc/WPEFramework/config.json"), false);
    EXPECT_TRUE(serverConf.Open(true));

    WPEFramework::Core::File pluginConf(string("thunder/install/etc/WPEFramework/plugins/LocationSync.json"), false);
    EXPECT_TRUE(pluginConf.Open(true));

    WPEFramework::Core::OptionalType<WPEFramework::Core::JSON::Error> error;

    Config server(serverConf, error);
    EXPECT_FALSE(error.IsSet());

    WPEFramework::Plugin::Config plugin;
    plugin.IElement::FromFile(pluginConf, error);
    EXPECT_FALSE(error.IsSet());

    auto service = WPEFramework::Core::ProxyType <Service>::Create(server, plugin);

    EXPECT_EQ(string(""), locationSync->Initialize(&(*service)));

    // invoke plugin

    WPEFramework::Core::JSONRPC::Connection connection(1, 0);

    string response;
    EXPECT_EQ(WPEFramework::Core::ERROR_INPROGRESS, handler.Invoke(connection, _T("sync"), _T("{}"), response));
    EXPECT_EQ(response, _T(""));

    // probe in progress

    WPEFramework::PluginHost::ISubSystem* subSystem = service->SubSystems();

    WPEFramework::Core::Event wait(false, true);
    for (int i = 0; ((subSystem->Get(WPEFramework::PluginHost::ISubSystem::LOCATION) == nullptr) && (i < 1000)); i++) {
        wait.Lock(10);
    }

    EXPECT_TRUE(subSystem->Get(WPEFramework::PluginHost::ISubSystem::LOCATION) != nullptr);
    EXPECT_TRUE(subSystem->Get(WPEFramework::PluginHost::ISubSystem::INTERNET) != nullptr);

    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Invoke(connection, _T("location"), _T(""), response));
    EXPECT_TRUE(response.empty() == false);
//    EXPECT_EQ(response, _T("{\"city\":\"Odessa\",\"country\":\"UA\",\"region\":\"51\",\"timezone\":\"EET-2EEST,M3.5.0/3,M10.5.0/4\",\"publicip\":\"195.64.234.239\"}"));

    // clean up

    locationSync->Deinitialize(&(*service));
    locationSync.Release();
    service.Release();

    WPEFramework::Core::IWorkerPool::Assign(nullptr);
    _engine.Release();
}

} // namespace RdkServicesTest
