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
#include "source/SystemInfo.h"
#include "source/WorkerPoolImplementation.h"

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

class SecurityAgentTestFixture : public ::testing::Test {
protected:
    Core::ProxyType<WorkerPoolImplementation> workerPool;
    Core::Sink<SystemInfo> subSystem;
    Core::ProxyType<Plugin::SecurityAgent> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    ServiceMock service;
    string response;
    string token;
    string endPoint;

    SecurityAgentTestFixture()
        : workerPool(Core::ProxyType<WorkerPoolImplementation>::Create(
            2, Core::Thread::DefaultStackSize(), 16))
        , plugin(Core::ProxyType<Plugin::SecurityAgent>::Create())
        , handler(*plugin)
        , connection(1, 0)
    {
    }
    virtual ~SecurityAgentTestFixture()
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

TEST_F(SecurityAgentTestFixture, jsonRpc)
{
    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(::testing::Return("{}"));
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
    EXPECT_CALL(service, PersistentPath())
        .Times(1)
        .WillOnce(::testing::Return(persistentPath));
    EXPECT_CALL(service, DataPath())
        .Times(1)
        .WillOnce(::testing::Return(dataPath));
    EXPECT_CALL(service, VolatilePath())
        .Times(1)
        .WillOnce(::testing::Return(volatilePath));
    EXPECT_CALL(service, ProxyStubPath())
        .Times(1)
        .WillOnce(::testing::Return(proxyStubPath));
    EXPECT_CALL(service, Callsign())
        .Times(1)
        .WillOnce(::testing::Return(callSign));

    ON_CALL(service, Version())
        .WillByDefault(::testing::Return(string()));

    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("validate")));

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, plugin->CreateToken(static_cast<uint16_t>(payload.length()), reinterpret_cast<const uint8_t*>(payload.c_str()), token));

    EXPECT_EQ(0, token.rfind(tokenPrefix, 0));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("validate"), "{\"token\":\"" + token + "\"}", response));
    EXPECT_EQ(response,
        _T("{\"valid\":true}"));

    plugin->Deinitialize(&service);
}

TEST_F(SecurityAgentTestFixture, rpcCom)
{
    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(::testing::Return("{}"));
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
    EXPECT_CALL(service, PersistentPath())
        .Times(1)
        .WillOnce(::testing::Return(persistentPath));
    EXPECT_CALL(service, DataPath())
        .Times(1)
        .WillOnce(::testing::Return(dataPath));
    EXPECT_CALL(service, VolatilePath())
        .Times(1)
        .WillOnce(::testing::Return(volatilePath));
    EXPECT_CALL(service, ProxyStubPath())
        .Times(1)
        .WillOnce(::testing::Return(proxyStubPath));
    EXPECT_CALL(service, Callsign())
        .Times(1)
        .WillOnce(::testing::Return(callSign));

    ON_CALL(service, Version())
        .WillByDefault(::testing::Return(string()));

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_TRUE(Core::SystemInfo::GetEnvironment(
        envEndPoint.c_str(), endPoint));
    EXPECT_EQ(endPoint, expectedEndPoint);

    auto engine = Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create();
    auto client = Core::ProxyType<RPC::CommunicatorClient>::Create(
        Core::NodeId(endPoint.c_str()), Core::ProxyType<Core::IIPCServer>(engine));

    EXPECT_TRUE(client.IsValid() == true);
    EXPECT_TRUE(client->IsOpen() == false);

    auto interface = client->Open<PluginHost::IAuthenticate>(
        callSign.c_str());
    EXPECT_TRUE(interface != nullptr);

    token.clear();
    EXPECT_EQ(Core::ERROR_NONE, interface->CreateToken(static_cast<uint16_t>(payload.length()), reinterpret_cast<const uint8_t*>(payload.c_str()), token));

    EXPECT_EQ(0, token.rfind(tokenPrefix, 0));

    interface->Release();

    /**
     * IUnknown Release() times out and
    returns without server response. The next action of the
    unit test is to destroy the server. When the server
    is being destroyed it also submits that response hence
    an ASSERT is hit. The problem with Release()
    happens due to the following commit in Thunder R2-v1.9
    e70fe4856c7cef952238decf9730e8b5283658e5 which
    introduces a lock for IUnknown Release() which
    blocks both RPC-COM client and server if they are
    in the same process.
     */
    sleep(1);

    client.Release();
    engine.Release();

    plugin->Deinitialize(&service);
}
