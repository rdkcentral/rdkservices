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

#include "DeviceDiagnostics.h"

#include "Source/WorkerPoolImplementation.h"
#include "Source/Config.h"
#include "Source/Service.h"

namespace RdkServicesTest {

TEST(DeviceDiagnosticsTest, test) {
    // assign worker pool
    auto _engine = WPEFramework::Core::ProxyType<WorkerPoolImplementation>::Create(2, WPEFramework::Core::Thread::DefaultStackSize(), 16);
    WPEFramework::Core::IWorkerPool::Assign(&(*_engine));
    _engine->Run();

    // create plugin
    auto deviceDiagnostic = WPEFramework::Core::ProxyType<WPEFramework::Plugin::DeviceDiagnostics>::Create();
    WPEFramework::Core::JSONRPC::Handler& handler = *deviceDiagnostic;

    // expect methods
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Exists(_T("getConfiguration")));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Exists(_T("getAVDecoderStatus")));

    // init server
    WPEFramework::Core::File serverConf(string("thunder/install/etc/WPEFramework/config.json"), false);
    EXPECT_TRUE(serverConf.Open(true));
    // init plugin
    WPEFramework::Core::File pluginConf(string("thunder/install/etc/WPEFramework/plugins/DeviceDiagnostics.json"), false);
    EXPECT_TRUE(pluginConf.Open(true));

    // expect server is runing
    WPEFramework::Core::OptionalType<WPEFramework::Core::JSON::Error> error;
    Config server(serverConf, error);
    EXPECT_FALSE(error.IsSet());

    // epxect plugin is runing
    WPEFramework::Plugin::Config plugin;
    plugin.IElement::FromFile(pluginConf, error);
    EXPECT_FALSE(error.IsSet());

    // init service
    auto service = WPEFramework::Core::ProxyType<Service>::Create(server, plugin);
    EXPECT_EQ(string(""), deviceDiagnostic->Initialize(&(*service)));

    // invoke plugin
    WPEFramework::Core::JSONRPC::Connection connection(1, 0);

    string response;
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Invoke(connection, _T("getConfiguration"), _T("{\"names\":[\"test\"]}"), response));
    // TODO pass here expected response
    EXPECT_EQ(response, _T("{\"paramList\":[\"Device.X_CISCO_COM_LED.RedPwm\":123],\"success\":true}"));

    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Invoke(connection, _T("getAVDecoderStatus"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"avDecoderStatus\":\"IDLE\",\"success\":true}"));

    // clean up
    deviceDiagnostic->Deinitialize(&(*service));
    deviceDiagnostic.Release();
    service.Release();

    WPEFramework::Core::IWorkerPool::Assign(nullptr);
    _engine.Release();
}

} // namespace RdkServicesTest
