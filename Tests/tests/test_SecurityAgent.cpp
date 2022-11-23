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

using namespace WPEFramework;

namespace {
const string payload = _T("http://localhost");
const string tokenPrefix = _T("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.aHR0cDovL2xvY2FsaG9zdA.");
const string envEndPoint = _T("SECURITYAGENT_PATH");
const string expectedEndPoint = _T("/tmp/token");
const string callSign = _T("SecurityAgent");
const string webPrefix = _T("/Service/SecurityAgent");
const string persistentPath = _T("/tmp/");
const string dataPath = _T("/tmp/");
const string volatilePath = _T("/tmp/");
const string proxyStubPath = _T("install/usr/lib/wpeframework/proxystubs");
}

class SecurityAgentTest : public ::testing::Test {
protected:
    Core::ProxyType<WorkerPoolImplementation> workerPool;
    Core::ProxyType<Plugin::SecurityAgent> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    SecurityAgentTest()
        : workerPool(Core::ProxyType<WorkerPoolImplementation>::Create(
            2, Core::Thread::DefaultStackSize(), 16))
        , plugin(Core::ProxyType<Plugin::SecurityAgent>::Create())
        , handler(*plugin)
        , connection(1, 0)
    {
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

class SecurityAgentInitializedTest : public SecurityAgentTest {
protected:
    Core::Sink<SystemInfo> subSystem;
    ServiceMock service;

    SecurityAgentInitializedTest()
        : SecurityAgentTest()
    {
        ON_CALL(service, ConfigLine())
            .WillByDefault(::testing::Return("{}"));
        ON_CALL(service, WebPrefix())
            .WillByDefault(::testing::Return(webPrefix));
        ON_CALL(service, SubSystems())
            .WillByDefault(::testing::Invoke(
                [&]() {
                    PluginHost::ISubSystem* result = (&subSystem);
                    result->AddRef();
                    return result;
                }));
        ON_CALL(service, PersistentPath())
            .WillByDefault(::testing::Return(persistentPath));
        ON_CALL(service, DataPath())
            .WillByDefault(::testing::Return(dataPath));
        ON_CALL(service, VolatilePath())
            .WillByDefault(::testing::Return(volatilePath));
        ON_CALL(service, ProxyStubPath())
            .WillByDefault(::testing::Return(proxyStubPath));
        ON_CALL(service, Callsign())
            .WillByDefault(::testing::Return(callSign));

        EXPECT_EQ(string(""), plugin->Initialize(&service));
    }
    virtual ~SecurityAgentInitializedTest() override
    {
        plugin->Deinitialize(&service);
    }
};

TEST_F(SecurityAgentInitializedTest, validate)
{
    string token;

    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("validate")));
    EXPECT_EQ(Core::ERROR_NONE, plugin->CreateToken(static_cast<uint16_t>(payload.length()), reinterpret_cast<const uint8_t*>(payload.c_str()), token));

    EXPECT_EQ(0, token.rfind(tokenPrefix, 0));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("validate"), "{\"token\":\"" + token + "\"}", response));
    EXPECT_EQ(response, _T("{\"valid\":true}"));
}

TEST_F(SecurityAgentInitializedTest, rpcCom)
{
    string token;
    string endPoint;

    EXPECT_TRUE(Core::SystemInfo::GetEnvironment(envEndPoint.c_str(), endPoint));
    EXPECT_EQ(endPoint, expectedEndPoint);

    auto engine = Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create();
    auto client = Core::ProxyType<RPC::CommunicatorClient>::Create(
        Core::NodeId(endPoint.c_str()), Core::ProxyType<Core::IIPCServer>(engine));

    EXPECT_TRUE(client.IsValid() == true);
    EXPECT_TRUE(client->IsOpen() == false);

    auto interface = client->Open<PluginHost::IAuthenticate>(callSign.c_str());
    ASSERT_TRUE(interface != nullptr);

    EXPECT_EQ(Core::ERROR_NONE, interface->CreateToken(static_cast<uint16_t>(payload.length()), reinterpret_cast<const uint8_t*>(payload.c_str()), token));

    EXPECT_EQ(0, token.rfind(tokenPrefix, 0));

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
}
