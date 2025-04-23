#include <gtest/gtest.h>

#include "DeviceIdentification.h"

#include "COMLinkMock.h"
#include "ServiceMock.h"
#include "SystemInfo.h"

using namespace WPEFramework;

using ::testing::NiceMock;

namespace {
const string testArchitecture = _T("testArchitecture");
const string testChipset = _T("testChipset");
const string testFirmwareVersion = _T("testFirmwareVersion");
const string testId = _T("testIdentity");
const string deviceId = _T("WPEdGVzdElkZW50aXR5");
}

namespace WPEFramework {
namespace Plugin {
    class DeviceImplementation : public PluginHost::ISubSystem::IIdentifier {
    public:
        virtual ~DeviceImplementation() = default;

        string Architecture() const override { return testArchitecture; }
        string Chipset() const override { return testChipset; }
        string FirmwareVersion() const override { return testFirmwareVersion; }
        uint8_t Identifier(const uint8_t length, uint8_t buffer[]) const override
        {
            ::memcpy(buffer, testId.c_str(), testId.length());

            return testId.length();
        }

        BEGIN_INTERFACE_MAP(DeviceImplementation)
        INTERFACE_ENTRY(PluginHost::ISubSystem::IIdentifier)
        END_INTERFACE_MAP
    };

    SERVICE_REGISTRATION(DeviceImplementation, 1, 0);
}
}

class DeviceIdentificationTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::DeviceIdentification> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    DeviceIdentificationTest()
        : plugin(Core::ProxyType<Plugin::DeviceIdentification>::Create())
        , handler(*plugin)
        , connection(1, 0)
    {
    }
    virtual ~DeviceIdentificationTest()
    {
    }
};

class DeviceIdentificationInitializedTest : public DeviceIdentificationTest {
protected:
    Core::Sink<NiceMock<SystemInfo>> subSystem;
    NiceMock<ServiceMock> service;
    NiceMock<COMLinkMock> comLinkMock;

    DeviceIdentificationInitializedTest()
        : DeviceIdentificationTest()
    {
        ON_CALL(service, ConfigLine())
            .WillByDefault(::testing::Return("{\n"
                                             "   \"root\":{\n"
                                             "      \"outofprocess\":false\n"
                                             "   }\n"
                                             "}"));
        ON_CALL(service, SubSystems())
            .WillByDefault(::testing::Invoke(
                [&]() {
                    PluginHost::ISubSystem* result = (&subSystem);
                    result->AddRef();
                    return result;
                }));
        ON_CALL(service, COMLink())
            .WillByDefault(::testing::Return(&comLinkMock));

        EXPECT_EQ(string(""), plugin->Initialize(&service));
    }
    virtual ~DeviceIdentificationInitializedTest() override
    {
        plugin->Deinitialize(&service);
    }
};

TEST_F(DeviceIdentificationInitializedTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("deviceidentification")));
}

TEST_F(DeviceIdentificationInitializedTest, deviceidentification)
{
    EXPECT_TRUE(subSystem.Get(PluginHost::ISubSystem::IDENTIFIER) != nullptr);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deviceidentification"), _T(""), response));
    EXPECT_EQ(response, _T("{\"firmwareversion\":\"testFirmwareVersion\",\"chipset\":\"testChipset\",\"deviceid\":\"WPEdGVzdElkZW50aXR5\"}"));
}
