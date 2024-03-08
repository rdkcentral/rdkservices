
#include "gtest/gtest.h"
#include "FactoriesImplementation.h"
#include "Packager.h"
#include "PackagerImplementation.h"
#include "ServiceMock.h"
#include "COMLinkMock.h"
#include "IarmBusMock.h"
#include <fstream>
#include <iostream>


using namespace WPEFramework;
using ::testing::NiceMock;

namespace {
const string config = _T("Packager");
const string callSign = _T("Packager");
const string webPrefix = _T("/Service/Packager");
const string volatilePath = _T("/tmp/");
const string dataPath = _T("/tmp/");
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
	Core::ProxyType<Plugin::PackagerImplementation> PackagerImplementation;

    PackagerInitializedTest()
        : PackagerTest()
    {
		PackagerImplementation = Core::ProxyType<Plugin::PackagerImplementation>::Create();
        ON_CALL(service, ConfigLine())
            .WillByDefault(::testing::Return("{}"));
        ON_CALL(service, WebPrefix())
            .WillByDefault(::testing::Return(webPrefix));
        ON_CALL(service, VolatilePath())
            .WillByDefault(::testing::Return(volatilePath));
        ON_CALL(service, Callsign())
            .WillByDefault(::testing::Return(callSign));
		ON_CALL(service, DataPath())
                .WillByDefault(::testing::Return(dataPath));
        ON_CALL(service, COMLink())
            .WillByDefault(::testing::Return(&comLinkMock));
		ON_CALL(comLinkMock, Instantiate(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
	    .WillByDefault(::testing::Return(PackagerImplementation));

        PluginHost::IFactories::Assign(&factoriesImplementation);
        EXPECT_EQ(string(""), plugin->Initialize(&service));
		opkg_config->lists_dir = strdup("/tmp/test");
    }
    virtual ~PackagerInitializedTest() override
    {
        //plugin->Deinitialize(&service);
		free(opkg_config->lists_dir);
        PluginHost::IFactories::Assign(nullptr);
    }
};


TEST_F(PackagerInitializedTest, httpGetPutInstall)
{
    //HTTP_GET - Get status for all modules
    Web::Request request;
    request.Verb = Web::Request::HTTP_PUT;
    request.Path = webPrefix + _T("/Install");
    auto httpResponse = interface->Process(request);
    ASSERT_TRUE(httpResponse.IsValid());
}

TEST_F(PackagerInitializedTest, httpGetPutSynchornize)
{
    //HTTP_GET - Get status for all modules
    Web::Request request;
    request.Verb = Web::Request::HTTP_PUT;
    request.Path = webPrefix + _T("/SynchronizeRepository");
    auto httpResponse = interface->Process(request);
    ASSERT_TRUE(httpResponse.IsValid());
}

TEST_F(PackagerInitializedTest, InstallTest)
{
	EXPECT_EQ(Core::ERROR_NONE, PackagerImplementation->Install("Test", "1.0", "arm"));
    
}

TEST_F(PackagerInitializedTest, SynchronizeRepositoryTest)
{

	EXPECT_EQ(Core::ERROR_NONE, PackagerImplementation->SynchronizeRepository());
}
