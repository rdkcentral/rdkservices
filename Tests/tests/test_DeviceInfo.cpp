#include <gtest/gtest.h>

#include "Implementation/DeviceInfo.h"

#include "IarmBusMock.h"

using namespace WPEFramework;

namespace {
const string iarmName = _T("Thunder_Plugins");
}

class DeviceInfoTest : public ::testing::Test {
protected:
    IarmBusImplMock iarmBusImplMock;
    Core::ProxyType<Plugin::DeviceInfoImplementation> deviceInfoImplementation;
    Exchange::IDeviceInfo* interface;

    DeviceInfoTest()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;
    }
    virtual ~DeviceInfoTest()
    {
        IarmBus::getInstance().impl = nullptr;
    }

    virtual void SetUp()
    {
        EXPECT_CALL(iarmBusImplMock, IARM_Bus_IsConnected(::testing::_, ::testing::_))
            .Times(1)
            .WillOnce(::testing::Invoke(
                [](const char* memberName, int* isRegistered) {
                    if (iarmName == string(memberName)) {
                        // Return 1 as not interested in all steps of IARM connection
                        *isRegistered = 1;
                        return IARM_RESULT_SUCCESS;
                    }
                    return IARM_RESULT_INVALID_PARAM;
                }));

        deviceInfoImplementation = Core::ProxyType<Plugin::DeviceInfoImplementation>::Create();

        interface = static_cast<Exchange::IDeviceInfo*>(
            deviceInfoImplementation->QueryInterface(Exchange::IDeviceInfo::ID));
        ASSERT_TRUE(interface != nullptr);
    }

    virtual void TearDown()
    {
        ASSERT_TRUE(interface != nullptr);
        interface->Release();
    }
};

TEST_F(DeviceInfoTest, Make)
{
    string make;

    EXPECT_EQ(Core::ERROR_NONE, interface->Make(make));
    EXPECT_EQ(make, _T("Pace"));
}

TEST_F(DeviceInfoTest, Model)
{
    string model;

    EXPECT_EQ(Core::ERROR_NONE, interface->Model(model));
    EXPECT_EQ(model, _T("Pace Xi5"));
}

TEST_F(DeviceInfoTest, DeviceType)
{
    string deviceType;

    EXPECT_EQ(Core::ERROR_NONE, interface->DeviceType(deviceType));
    EXPECT_EQ(deviceType, _T("IpStb"));
}
