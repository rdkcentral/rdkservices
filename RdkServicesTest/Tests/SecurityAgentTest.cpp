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

#include "SecurityAgent.h"

#include "Source/WorkerPoolImplementation.h"
#include "Source/Config.h"
#include "Source/Service.h"

namespace RdkServicesTest {

TEST(SecurityAgentTest, test) {
    // assign worker pool

    auto _engine = WPEFramework::Core::ProxyType<WorkerPoolImplementation>::Create(2, WPEFramework::Core::Thread::DefaultStackSize(), 16);
    WPEFramework::Core::IWorkerPool::Assign(&(*_engine));
    _engine->Run();

    // create plugin

    auto securityAgent = WPEFramework::Core::ProxyType <WPEFramework::Plugin::SecurityAgent>::Create();

    WPEFramework::Core::JSONRPC::Handler& handler = *securityAgent;

    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Exists(_T("validate")));

    // init plugin

    WPEFramework::Core::File serverConf(string("thunder/install/etc/WPEFramework/config.json"), false);
    EXPECT_TRUE(serverConf.Open(true));

    WPEFramework::Core::File pluginConf(string("thunder/install/etc/WPEFramework/plugins/SecurityAgent.json"), false);
    EXPECT_TRUE(pluginConf.Open(true));

    WPEFramework::Core::OptionalType<WPEFramework::Core::JSON::Error> error;

    Config server(serverConf, error);
    EXPECT_FALSE(error.IsSet());

    WPEFramework::Plugin::Config plugin;
    plugin.IElement::FromFile(pluginConf, error);
    EXPECT_FALSE(error.IsSet());

    auto service = WPEFramework::Core::ProxyType <Service>::Create(server, plugin);

    EXPECT_EQ(string(""), securityAgent->Initialize(&(*service)));

    // invoke plugin

    string token, payload = "http://localhost";
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, securityAgent->CreateToken(static_cast<uint16_t>(payload.length()), reinterpret_cast<const uint8_t*>(payload.c_str()), token));
    EXPECT_EQ(0, token.rfind(_T("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.aHR0cDovL2xvY2FsaG9zdA."), 0));

    WPEFramework::Core::JSONRPC::Connection connection(1, 0);

    string response;
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Invoke(connection, _T("validate"), "{\"token\":\"" + token + "\"}", response));
    EXPECT_EQ(response, _T("{\"valid\":true}"));

    // invoke rpc-com

    string endPoint;
    EXPECT_TRUE(WPEFramework::Core::SystemInfo::GetEnvironment(_T("SECURITYAGENT_PATH"), endPoint));
    EXPECT_EQ(endPoint, _T("/tmp//token"));

    auto engine = WPEFramework::Core::ProxyType<WPEFramework::RPC::InvokeServerType<1, 0, 4>>::Create();
    auto client = WPEFramework::Core::ProxyType<WPEFramework::RPC::CommunicatorClient>::Create(WPEFramework::Core::NodeId(endPoint.c_str()), WPEFramework::Core::ProxyType<WPEFramework::Core::IIPCServer>(engine));

    EXPECT_TRUE(client.IsValid() == true);
    EXPECT_TRUE(client->IsOpen() == false);

    WPEFramework::PluginHost::IAuthenticate* securityAgentInterface = client->Open<WPEFramework::PluginHost::IAuthenticate>("SecurityAgent");
    EXPECT_TRUE(securityAgentInterface != nullptr);

    token.clear();
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, securityAgentInterface->CreateToken(static_cast<uint16_t>(payload.length()), reinterpret_cast<const uint8_t*>(payload.c_str()), token));
    EXPECT_EQ(0, token.rfind(_T("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.aHR0cDovL2xvY2FsaG9zdA."), 0));

    securityAgentInterface->Release();

    client.Release();
    engine.Release();

    // clean up

    securityAgent->Deinitialize(&(*service));
    securityAgent.Release();
    service.Release();

    WPEFramework::Core::IWorkerPool::Assign(nullptr);
    _engine.Release();
}

} // namespace RdkServicesTest
