
/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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
#include "FactoriesImplementation.h"
#include "Packager.h"
#include "ServiceMock.h"
#include "COMLinkMock.h"


using namespace WPEFramework;
using ::testing::NiceMock;

namespace {
const string callSign = _T("Packager");
const string webPrefix = _T("/Service/Packager");
const string volatilePath = _T("/tmp/");
}

class PackagerTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::Packager> plugin;
    PluginHost::IWeb* interface;

    PackagerTest()
        : plugin(Core::ProxyType<Plugin::Packager>::Create())
    {
        interface = static_cast<PluginHost::IWeb*>(plugin->QueryInterface(PluginHost::IWeb::ID));
    }
    virtual ~PackagerTest()
    {
        interface->Release();
		plugin.Release();
    }

    virtual void SetUp()
    {
        ASSERT_TRUE(interface != nullptr);
    }

    virtual void TearDown()
    {
        ASSERT_TRUE(interface != nullptr);
    }
};

class PackagerInitializedTest : public PackagerTest {
protected:
    NiceMock<FactoriesImplementation> factoriesImplementation;
    NiceMock<ServiceMock> service;
    NiceMock<COMLinkMock> comLinkMock;

    PackagerInitializedTest()
        : PackagerTest()
    {
        ON_CALL(service, ConfigLine())
            .WillByDefault(::testing::Return("{}"));
        ON_CALL(service, WebPrefix())
            .WillByDefault(::testing::Return(webPrefix));
        ON_CALL(service, VolatilePath())
            .WillByDefault(::testing::Return(volatilePath));
        ON_CALL(service, Callsign())
            .WillByDefault(::testing::Return(callSign));
        ON_CALL(service, COMLink())
            .WillByDefault(::testing::Return(&comLinkMock));

        PluginHost::IFactories::Assign(&factoriesImplementation);
        EXPECT_EQ(string(""), plugin->Initialize(&service));
    }
    virtual ~PackagerInitializedTest() override
    {
        plugin->Deinitialize(&service);
        PluginHost::IFactories::Assign(nullptr);
    }
};

/*
class PackagerTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::Packager> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;
	NiceMock<ServiceMock> service;
	PluginHost::IWeb* interface;

    PackagerTest()
        : plugin(Core::ProxyType<Plugin::Packager>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
		interface = static_cast<PluginHost::IWeb*>(plugin->QueryInterface(PluginHost::IWeb::ID));
    }
    virtual ~PackagerTest() = default;
};

class PackagerInitializedTest : public PackagerTest {
protected:
    ServiceMock service;

    PackagerInitializedTest()
        : PackagerTest()
    {
       ON_CALL(service, WebPrefix())
            .WillByDefault(::testing::Return(_T("/Service/Packager")));
        EXPECT_EQ(string(""), plugin->Initialize(&service));
    }
    virtual ~PackagerInitializedTest() override
    {
        plugin->Deinitialize(&service);
    }
};

class PackagerImplementationTest : public PackagerInitializedTest {
protected:
      PackagerImplementationTest()
        : PackagerInitializedTest()
		{
		}
		EXPECT_EQ(string(""), plugin->Initialize(&service));
	virtual ~PackagerImplementationTest() override
    {
        plugin->Deinitialize(&service);
    }
};

TEST_F(PackagerImplementationTest, InstallTest) {
   
   EXPECT_EQ(1, 1);
   EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("{}"), _T("{}"), response));
   
}
*/

TEST_F(PackagerInitializedTest, httpGetPut)
{
    //HTTP_GET - Get status for all modules
    Web::Request request;
    request.Verb = Web::Request::HTTP_PUT;
    request.Path = webPrefix;
    auto httpResponse = interface->Process(request);
    ASSERT_TRUE(httpResponse.IsValid());
}