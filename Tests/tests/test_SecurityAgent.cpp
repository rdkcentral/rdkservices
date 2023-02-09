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

#include "SecurityAgent.h"

#include "ServiceMock.h"
#include "SystemInfo.h"
#include "WorkerPoolImplementation.h"

#include <fstream>

using namespace WPEFramework;

using ::testing::NiceMock;

namespace {
const string callSign = _T("SecurityAgent");
}

class SecurityAgentTest : public ::testing::Test {
protected:
    Core::ProxyType<WorkerPoolImplementation> workerPool;
    Core::ProxyType<Plugin::SecurityAgent> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;
    Core::Sink<NiceMock<SystemInfo>> subSystem;
    NiceMock<ServiceMock> service;

    SecurityAgentTest()
        : workerPool(Core::ProxyType<WorkerPoolImplementation>::Create(
            2, Core::Thread::DefaultStackSize(), 16))
        , plugin(Core::ProxyType<Plugin::SecurityAgent>::Create())
        , handler(*plugin)
        , connection(1, 0)
    {
        ON_CALL(service, ConfigLine())
            .WillByDefault(::testing::Return("{}"));
        ON_CALL(service, WebPrefix())
            .WillByDefault(::testing::Return(_T("/Service/SecurityAgent")));
        ON_CALL(service, SubSystems())
            .WillByDefault(::testing::Invoke(
                [&]() {
                    PluginHost::ISubSystem* result = (&subSystem);
                    result->AddRef();
                    return result;
                }));
        ON_CALL(service, PersistentPath())
            .WillByDefault(::testing::Return(_T("/tmp/")));
        ON_CALL(service, DataPath())
            .WillByDefault(::testing::Return(_T("/tmp/")));
        ON_CALL(service, VolatilePath())
            .WillByDefault(::testing::Return(_T("/tmp/")));
        ON_CALL(service, ProxyStubPath())
            .WillByDefault(::testing::Return(_T("install/usr/lib/wpeframework/proxystubs")));
        ON_CALL(service, Callsign())
            .WillByDefault(::testing::Return(callSign));

        Core::IWorkerPool::Assign(&(*workerPool));

        workerPool->Run();
    }
    virtual ~SecurityAgentTest()
    {
        plugin.Release();

        Core::IWorkerPool::Assign(nullptr);
        workerPool.Release();
    }
};

TEST_F(SecurityAgentTest, validate)
{
    const string payload = _T("{\"url\":\"http://localhost\"}");

    string token;

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("validate")));
    EXPECT_EQ(Core::ERROR_NONE, plugin->CreateToken(static_cast<uint16_t>(payload.length()), reinterpret_cast<const uint8_t*>(payload.c_str()), token));

    EXPECT_THAT(token, ::testing::MatchesRegex("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9\\.eyJ1cmwiOiJodHRwOi8vbG9jYWxob3N0In0\\..*"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("validate"), "{\"token\":\"" + token + "\"}", response));
    EXPECT_EQ(response, _T("{\"valid\":true}"));

    plugin->Deinitialize(&service);
}

TEST_F(SecurityAgentTest, rpcCom)
{
    const string payload = _T("{\"url\":\"http://localhost\"}");

    string token;
    string endPoint;

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_TRUE(Core::SystemInfo::GetEnvironment(_T("SECURITYAGENT_PATH"), endPoint));
    EXPECT_EQ(endPoint, _T("/tmp/token"));

    auto engine = Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create();
    auto client = Core::ProxyType<RPC::CommunicatorClient>::Create(
        Core::NodeId(endPoint.c_str()), Core::ProxyType<Core::IIPCServer>(engine));

    EXPECT_TRUE(client.IsValid() == true);
    EXPECT_TRUE(client->IsOpen() == false);

    auto interface = client->Open<PluginHost::IAuthenticate>(callSign.c_str());
    ASSERT_TRUE(interface != nullptr);

    EXPECT_EQ(Core::ERROR_NONE, interface->CreateToken(static_cast<uint16_t>(payload.length()), reinterpret_cast<const uint8_t*>(payload.c_str()), token));

    EXPECT_THAT(token, ::testing::MatchesRegex("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9\\.eyJ1cmwiOiJodHRwOi8vbG9jYWxob3N0In0\\..*"));

    interface->Release();

    //    /**
    //     * IUnknown Release() times out and
    //    returns without server response. The next action of the
    //    unit test is to destroy the server. When the server
    //    is being destroyed it also submits that response hence
    //    an ASSERT is hit. The problem with Release()
    //    happens due to the following commit in Thunder R2-v1.9
    //    e70fe4856c7cef952238decf9730e8b5283658e5 which
    //    introduces a lock for IUnknown Release() which
    //    blocks both RPC-COM client and server if they are
    //    in the same process.
    //     */
    //    sleep(1);
    // ^^^ should no longer be needed ??

    client.Release();
    engine.Release();

    plugin->Deinitialize(&service);
}

TEST_F(SecurityAgentTest, acl)
{
    const string payload = _T("{\"url\":\"http://example.com\"}");

    std::ofstream file("/tmp/acl.json");
    file
        << "{\n"
           "  \"assign\": [\n"
           "    {\n"
           "      \"url\": \"*://example.com\",\n"
           "      \"role\": \"example\"\n"
           "    }\n"
           "  ],\n"
           "  \"roles\": {\n"
           "    \"example\": {\n"
           "      \"default\": \"blocked\",\n"
           "      \"DeviceInfo\": {\n"
           "        \"default\": \"allowed\",\n"
           "        \"methods\": [ \"register\", \"unregister\" ]\n"
           "      }\n"
           "    }\n"
           "  }\n"
           "}";
    file.close();

    ON_CALL(service, ConfigLine())
        .WillByDefault(::testing::Return(_T("{\"acl\":\"acl.json\"}")));
    EXPECT_EQ(string(""), plugin->Initialize(&service));

    PluginHost::IAuthenticate* interface = static_cast<PluginHost::IAuthenticate*>(
        plugin->QueryInterface(PluginHost::IAuthenticate::ID));
    ASSERT_TRUE(interface != nullptr);

    string token;
    EXPECT_EQ(Core::ERROR_NONE, interface->CreateToken(static_cast<uint16_t>(payload.length()), reinterpret_cast<const uint8_t*>(payload.c_str()), token));
    PluginHost::ISecurity* security = interface->Officer(token);
    ASSERT_TRUE(security != nullptr);

    EXPECT_EQ(payload, security->Token());

    Core::JSONRPC::Message message;
    message.Designator = _T("Uknkown.method");
    EXPECT_FALSE(security->Allowed(message));
    message.Designator = _T("DeviceInfo.systeminfo");
    EXPECT_TRUE(security->Allowed(message));
    message.Designator = _T("DeviceInfo.register");
    EXPECT_FALSE(security->Allowed(message));

    security->Release();

    interface->Release();

    plugin->Deinitialize(&service);
}

TEST_F(SecurityAgentTest, aclForDACApps)
{
    const string payload = _T("{\"url\":\"example\",\"type\":\"dac\"}");

    Core::Directory dir(_T("/tmp/dac"));

    EXPECT_TRUE(dir.Destroy(false));
    ASSERT_TRUE(dir.CreatePath());

    ON_CALL(service, ConfigLine())
        .WillByDefault(::testing::Return(_T("{\"dac\":\"/tmp/dac\"}")));
    EXPECT_EQ(string(""), plugin->Initialize(&service));

    PluginHost::IAuthenticate* interface = static_cast<PluginHost::IAuthenticate*>(
        plugin->QueryInterface(PluginHost::IAuthenticate::ID));
    ASSERT_TRUE(interface != nullptr);

    string token;
    EXPECT_EQ(Core::ERROR_NONE, interface->CreateToken(static_cast<uint16_t>(payload.length()), reinterpret_cast<const uint8_t*>(payload.c_str()), token));
    PluginHost::ISecurity* security = interface->Officer(token);
    ASSERT_TRUE(security != nullptr);

    EXPECT_EQ(payload, security->Token());

    Core::JSONRPC::Message message;
    message.Designator = _T("DeviceInfo.systeminfo");
    EXPECT_FALSE(security->Allowed(message));

    std::ofstream file("/tmp/dac/example");
    file
        << "{\n"
           "  \"assign\": [\n"
           "    {\n"
           "      \"url\": \"example\",\n"
           "      \"role\": \"example\"\n"
           "    }\n"
           "  ],\n"
           "  \"roles\": {\n"
           "    \"example\": {\n"
           "      \"default\": \"blocked\",\n"
           "      \"DeviceInfo\": {\n"
           "        \"default\": \"allowed\"\n"
           "      }\n"
           "    }\n"
           "  }\n"
           "}";
    file.close();
    // Wait up to 1s
    for (int i = 0, retry = 20; ((i < retry) && (!security->Allowed(message))); i++) {
        SleepMs(50);
    }
    EXPECT_TRUE(security->Allowed(message));

    security->Release();

    interface->Release();

    plugin->Deinitialize(&service);
}
