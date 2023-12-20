#include <gtest/gtest.h>

#include "Implementation/DeviceInfo.h"

#include "IarmBusMock.h"

#include <fstream>

using namespace WPEFramework;

using ::testing::NiceMock;

class DeviceInfoTest : public ::testing::Test {
protected:
    IarmBusImplMock   *p_iarmBusImplMock = nullptr ;
    Core::ProxyType<Plugin::DeviceInfoImplementation> deviceInfoImplementation;
    Exchange::IDeviceInfo* interface;

    DeviceInfoTest()
    {
        p_iarmBusImplMock  = new NiceMock <IarmBusImplMock>;
        IarmBus::setImpl(p_iarmBusImplMock);

        deviceInfoImplementation = Core::ProxyType<Plugin::DeviceInfoImplementation>::Create();

        interface = static_cast<Exchange::IDeviceInfo*>(
            deviceInfoImplementation->QueryInterface(Exchange::IDeviceInfo::ID));
    }
    virtual ~DeviceInfoTest()
    {
        interface->Release();
        IarmBus::setImpl(nullptr);
        if (p_iarmBusImplMock != nullptr)
        {
            delete p_iarmBusImplMock;
            p_iarmBusImplMock = nullptr;
        }
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

TEST_F(DeviceInfoTest, Make)
{
    std::ofstream file("/etc/device.properties");
    file << "MFG_NAME=Pace";
    file.close();

    string make;
    EXPECT_EQ(Core::ERROR_NONE, interface->Make(make));
    EXPECT_EQ(make, _T("Pace"));
}

TEST_F(DeviceInfoTest, Model)
{
    std::ofstream file("/etc/device.properties");
    file << "FRIENDLY_ID=\"Pace Xi5\"";
    file.close();

    string model;
    EXPECT_EQ(Core::ERROR_NONE, interface->Model(model));
    EXPECT_EQ(model, _T("Pace Xi5"));
}

TEST_F(DeviceInfoTest, DeviceType)
{
    std::ofstream file("/etc/authService.conf");
    file << "deviceType=IpStb";
    file.close();

    string deviceType;
    EXPECT_EQ(Core::ERROR_NONE, interface->DeviceType(deviceType));
    EXPECT_EQ(deviceType, _T("IpStb"));
}
