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

    WPEFramework::Core::ProxyType<WorkerPoolImplementation> _engine;
    _engine = WPEFramework::Core::ProxyType<WorkerPoolImplementation>::Create(2, WPEFramework::Core::Thread::DefaultStackSize(), 16);
    WPEFramework::Core::IWorkerPool::Assign(&(*_engine));

    // create plugin

    WPEFramework::Core::ProxyType <WPEFramework::Plugin::LocationSync> locationSync;
    locationSync = WPEFramework::Core::ProxyType <WPEFramework::Plugin::LocationSync>::Create();

    WPEFramework::Core::JSONRPC::Handler& handler = *locationSync;

    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Exists(_T("sync")));

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

    WPEFramework::Core::ProxyType <Service> service;
    service = WPEFramework::Core::ProxyType <Service>::Create(server, plugin);
    EXPECT_EQ(string(""), locationSync->Initialize(&(*service)));

    // invoke plugin

    WPEFramework::Core::JSONRPC::Connection connection(1, 0);

    string response;
    EXPECT_EQ(WPEFramework::Core::ERROR_INPROGRESS, handler.Invoke(connection, _T("sync"), _T("{}"), response));
    EXPECT_EQ(response, _T(""));
}

} // namespace RdkServicesTest
