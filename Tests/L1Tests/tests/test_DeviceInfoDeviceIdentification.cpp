#include <gtest/gtest.h>

#include "DeviceInfo.h"
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
    class DeviceImplementation : public Exchange::IDeviceIdentification, public PluginHost::ISubSystem::IIdentifier {
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
		
	   Core::hresult Identification(DeviceInfo& info) const override
		{
			info.deviceID = deviceId;
			info.firmwareVersion = FirmwareVersion();
			info.chipset = Chipset();

			return Core::ERROR_NONE;
		}


        BEGIN_INTERFACE_MAP(DeviceImplementation)
	INTERFACE_ENTRY(Exchange::IDeviceIdentification)
        INTERFACE_ENTRY(PluginHost::ISubSystem::IIdentifier)		
        END_INTERFACE_MAP
    };

    SERVICE_REGISTRATION(DeviceImplementation, 1, 0);
}
}

class DeviceInfoDeviceIdentificationTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::DeviceImplementation> deviceInfoDeviceIdentification;
    Exchange::IDeviceIdentification* interface;

    DeviceInfoDeviceIdentificationTest()
        : deviceInfoDeviceIdentification(Core::ProxyType<Plugin::DeviceImplementation>::Create())
    {
        interface = static_cast<Exchange::IDeviceIdentification*>(
            deviceInfoDeviceIdentification->QueryInterface(Exchange::IDeviceIdentification::ID));
    }
    virtual ~DeviceInfoDeviceIdentificationTest()
    {
        interface->Release();
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



TEST_F(DeviceInfoDeviceIdentificationTest, Identification)
{
    Exchange::IDeviceIdentification::DeviceInfo info;
    EXPECT_EQ(Core::ERROR_NONE, interface->Identification(info));
    EXPECT_EQ(info.chipset, _T("testChipset"));
    EXPECT_EQ(info.deviceID, _T("WPEdGVzdElkZW50aXR5"));
    EXPECT_EQ(info.firmwareVersion, _T("testFirmwareVersion"));
}


