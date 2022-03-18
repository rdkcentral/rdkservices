#include <gtest/gtest.h>

#include "DeviceIdentification.h"

#include "DeviceImplementation.h"
#include "ServiceMock.h"
#include "Source/SystemInfo.h"

namespace WPEFramework {
namespace Plugin {
    SERVICE_REGISTRATION(DeviceImplementation, 1, 0);
}
}

using namespace WPEFramework;

class DeviceIdentificationTestFixture : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::DeviceIdentification> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    Core::Sink<SystemInfo> subSystem;
    Exchange::IDeviceProperties* device;
    PluginHost::ISubSystem::IIdentifier* identifier;
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

TEST_F(DeviceIdentificationTestFixture, registeredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("deviceidentification")));
}

TEST_F(DeviceIdentificationTestFixture, deviceidentification)
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

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    device = const_cast<Exchange::IDeviceProperties*>(
        dynamic_cast<const Exchange::IDeviceProperties*>(
            subSystem.Get(PluginHost::ISubSystem::IDENTIFIER)));

    EXPECT_TRUE(device != nullptr);
    EXPECT_EQ(string("testChipset"), device->Chipset());
    EXPECT_EQ(string("testFirmwareVersion"), device->FirmwareVersion());

    identifier = device->QueryInterface<PluginHost::ISubSystem::IIdentifier>();

    EXPECT_TRUE(identifier != nullptr);
    uint8_t buf[64];
    buf[0] = identifier->Identifier(sizeof(buf) - 1, &(buf[1]));
    EXPECT_EQ(string("testIdentity"), string((const char*)&(buf[1]), buf[0]));

    identifier->Release();

    EXPECT_EQ(Core::ERROR_NONE,
        handler.Invoke(connection, _T("deviceidentification"), _T(""), response));
    EXPECT_TRUE(response.empty() == false);

    JsonObject params;

    EXPECT_TRUE(params.FromString(response));
    EXPECT_TRUE(params.HasLabel(_T("firmwareversion")));
    EXPECT_TRUE(params.HasLabel(_T("chipset")));
    EXPECT_TRUE(params.HasLabel(_T("deviceid")));

    EXPECT_EQ(string("testFirmwareVersion"), params[_T("firmwareversion")].Value());
    EXPECT_EQ(string("testChipset"), params[_T("chipset")].Value());
    EXPECT_EQ(string("WPEdGVzdElkZW50aXR5"), params[_T("deviceid")].Value());

    plugin->Deinitialize(&service);
}
