#include <gtest/gtest.h>

#include "DeviceIdentification.h"

#include "ServiceMock.h"
#include "source/SystemInfo.h"

using namespace WPEFramework;

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

class DeviceIdentificationTestFixture : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::DeviceIdentification> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    Core::Sink<SystemInfo> subSystem;
    ServiceMock service;
    string response;

    DeviceIdentificationTestFixture()
        : plugin(Core::ProxyType<Plugin::DeviceIdentification>::Create())
        , handler(*plugin)
        , connection(1, 0)
    {
    }
    virtual ~DeviceIdentificationTestFixture()
    {
    }
};

void PrepareExpectCallsForMockMethods(ServiceMock& service, Core::Sink<SystemInfo>& subSystem)
{
    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(::testing::Return("{\n"
                                    "   \"root\":{\n"
                                    "      \"outofprocess\":false\n"
                                    "   }\n"
                                    "}"));

    EXPECT_CALL(service, Locator())
        .Times(1)
        .WillOnce(::testing::Return(string()));

    EXPECT_CALL(service, SubSystems())
        .Times(2)
        .WillRepeatedly(::testing::Invoke(
            [&]() {
                PluginHost::ISubSystem* result = (&subSystem);

                result->AddRef();

                return result;
            }));

    ON_CALL(service, COMLink())
        .WillByDefault(::testing::Return(&service));

    return;
}

TEST_F(DeviceIdentificationTestFixture, RegisteredMethods)
{

    PrepareExpectCallsForMockMethods(service, subSystem);

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("deviceidentification")));

    plugin->Deinitialize(&service);
}

TEST_F(DeviceIdentificationTestFixture, Property)
{
    PrepareExpectCallsForMockMethods(service, subSystem);

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_TRUE(subSystem.Get(PluginHost::ISubSystem::IDENTIFIER) != nullptr);

    EXPECT_EQ(Core::ERROR_NONE,
        handler.Invoke(connection, _T("deviceidentification"), _T(""), response));
    EXPECT_TRUE(response.empty() == false);

    JsonObject params;

    EXPECT_TRUE(params.FromString(response));
    EXPECT_TRUE(params.HasLabel(_T("firmwareversion")));
    EXPECT_TRUE(params.HasLabel(_T("chipset")));
    EXPECT_TRUE(params.HasLabel(_T("deviceid")));

    EXPECT_EQ(testFirmwareVersion, params[_T("firmwareversion")].Value());
    EXPECT_EQ(testChipset, params[_T("chipset")].Value());
    EXPECT_EQ(deviceId, params[_T("deviceid")].Value());

    plugin->Deinitialize(&service);
}
