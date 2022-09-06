#include <gtest/gtest.h>

#include "Implementation/DeviceVideoCapabilities.h"

#include "HostMock.h"
#include "IarmBusMock.h"
#include "ManagerMock.h"
#include "VideoOutputPortConfigMock.h"
#include "VideoOutputPortMock.h"
#include "VideoOutputPortTypeMock.h"
#include "VideoResolutionMock.h"

#include "exception.hpp"

using namespace WPEFramework;

class DeviceVideoCapabilitiesTest : public ::testing::Test {
protected:
    IarmBusImplMock iarmBusImplMock;
    ManagerImplMock managerImplMock;
    Core::ProxyType<Plugin::DeviceVideoCapabilities> deviceVideoCapabilities;
    Exchange::IDeviceVideoCapabilities* interface;

    DeviceVideoCapabilitiesTest()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;
        device::Manager::getInstance().impl = &managerImplMock;

        EXPECT_CALL(iarmBusImplMock, IARM_Bus_IsConnected(::testing::_, ::testing::_))
            .Times(::testing::AnyNumber())
            .WillRepeatedly(::testing::Invoke(
                [](const char*, int* isRegistered) {
                    *isRegistered = 1;
                    return IARM_RESULT_SUCCESS;
                }));
        EXPECT_CALL(managerImplMock, Initialize())
            .Times(::testing::AnyNumber())
            .WillRepeatedly(::testing::Return());

        deviceVideoCapabilities = Core::ProxyType<Plugin::DeviceVideoCapabilities>::Create();

        interface = static_cast<Exchange::IDeviceVideoCapabilities*>(
            deviceVideoCapabilities->QueryInterface(Exchange::IDeviceVideoCapabilities::ID));
    }
    virtual ~DeviceVideoCapabilitiesTest()
    {
        interface->Release();

        IarmBus::getInstance().impl = nullptr;
        device::Manager::getInstance().impl = nullptr;
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

class DeviceVideoCapabilitiesDsTest : public DeviceVideoCapabilitiesTest {
protected:
    HostImplMock hostImplMock;
    VideoOutputPortConfigImplMock videoOutputPortConfigImplMock;

    DeviceVideoCapabilitiesDsTest()
        : DeviceVideoCapabilitiesTest()
    {
        device::Host::getInstance().impl = &hostImplMock;
        device::VideoOutputPortConfig::getInstance().impl = &videoOutputPortConfigImplMock;
    }
    virtual ~DeviceVideoCapabilitiesDsTest() override
    {
        device::Host::getInstance().impl = nullptr;
        device::VideoOutputPortConfig::getInstance().impl = nullptr;
    }
};

TEST_F(DeviceVideoCapabilitiesDsTest, SupportedVideoDisplays)
{
    VideoOutputPortMock videoOutputPortMock;
    device::VideoOutputPort videoOutputPort;
    videoOutputPort.impl = &videoOutputPortMock;
    RPC::IStringIterator* supportedVideoDisplays = nullptr;
    string videoPort(_T("HDMI0"));
    string element;

    EXPECT_CALL(videoOutputPortMock, getName())
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::ReturnRef(videoPort));
    EXPECT_CALL(hostImplMock, getVideoOutputPorts())
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Return(device::List<device::VideoOutputPort>({ videoOutputPort })));

    EXPECT_EQ(Core::ERROR_NONE, interface->SupportedVideoDisplays(supportedVideoDisplays));
    ASSERT_TRUE(supportedVideoDisplays != nullptr);
    ASSERT_TRUE(supportedVideoDisplays->Next(element));
    EXPECT_EQ(element, videoPort);

    supportedVideoDisplays->Release();
}

TEST_F(DeviceVideoCapabilitiesDsTest, HostEDID)
{
    std::vector<uint8_t> edidVec({ 't', 'e', 's', 't' });
    string edid;

    EXPECT_CALL(hostImplMock, getHostEDID(::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](std::vector<uint8_t>& edid) {
                edid = edidVec;
            }));

    EXPECT_EQ(Core::ERROR_NONE, interface->HostEDID(edid));
    EXPECT_EQ(edid, _T("dGVzdA=="));
}

TEST_F(DeviceVideoCapabilitiesDsTest, DefaultResolution_noParam)
{
    VideoOutputPortMock videoOutputPortMock;
    device::VideoOutputPort videoOutputPort;
    videoOutputPort.impl = &videoOutputPortMock;
    VideoResolutionMock videoResolutionMock;
    device::VideoResolution videoResolution;
    videoResolution.impl = &videoResolutionMock;
    string videoPort(_T("HDMI0"));
    string videoPortDefaultResolution(_T("1080p"));
    string defaultResolution;

    EXPECT_CALL(videoResolutionMock, getName())
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::ReturnRef(videoPortDefaultResolution));
    EXPECT_CALL(videoOutputPortMock, getDefaultResolution())
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::ReturnRef(videoResolution));
    EXPECT_CALL(hostImplMock, getDefaultVideoPortName())
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Return(videoPort));
    EXPECT_CALL(hostImplMock, getVideoOutputPort(::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::ReturnRef(videoOutputPort));

    EXPECT_EQ(Core::ERROR_NONE, interface->DefaultResolution(string(), defaultResolution));
    EXPECT_EQ(defaultResolution, videoPortDefaultResolution);
}

TEST_F(DeviceVideoCapabilitiesDsTest, SupportedResolutions_noParam)
{
    VideoOutputPortMock videoOutputPortMock;
    device::VideoOutputPort videoOutputPort;
    videoOutputPort.impl = &videoOutputPortMock;
    VideoOutputPortTypeMock videoOutputPortTypeMock;
    device::VideoOutputPortType videoOutputPortType;
    videoOutputPortType.impl = &videoOutputPortTypeMock;
    VideoResolutionMock videoResolutionMock;
    device::VideoResolution videoResolution;
    videoResolution.impl = &videoResolutionMock;
    RPC::IStringIterator* supportedResolutions = nullptr;
    string videoPort(_T("HDMI0"));
    string videoPortSupportedResolution(_T("1080p"));
    string element;

    EXPECT_CALL(videoResolutionMock, getName())
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::ReturnRef(videoPortSupportedResolution));
    EXPECT_CALL(videoOutputPortTypeMock, getSupportedResolutions())
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Return(device::List<device::VideoResolution>({ videoResolution })));
    EXPECT_CALL(videoOutputPortTypeMock, getId())
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Return(0));
    EXPECT_CALL(videoOutputPortMock, getType())
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::ReturnRef(videoOutputPortType));
    EXPECT_CALL(hostImplMock, getDefaultVideoPortName())
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Return(videoPort));
    EXPECT_CALL(hostImplMock, getVideoOutputPort(::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::ReturnRef(videoOutputPort));
    EXPECT_CALL(videoOutputPortConfigImplMock, getPortType(::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::ReturnRef(videoOutputPortType));

    EXPECT_EQ(Core::ERROR_NONE, interface->SupportedResolutions(string(), supportedResolutions));
    ASSERT_TRUE(supportedResolutions != nullptr);
    ASSERT_TRUE(supportedResolutions->Next(element));
    EXPECT_EQ(element, videoPortSupportedResolution);

    supportedResolutions->Release();
}

TEST_F(DeviceVideoCapabilitiesDsTest, SupportedHdcp_noParam)
{
    VideoOutputPortMock videoOutputPortMock;
    device::VideoOutputPort videoOutputPort;
    videoOutputPort.impl = &videoOutputPortMock;
    string videoPort(_T("HDMI0"));
    auto supportedHDCPVersion = Exchange::IDeviceVideoCapabilities::CopyProtection::HDCP_UNAVAILABLE;

    EXPECT_CALL(videoOutputPortMock, getHDCPProtocol())
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Return(dsHDCP_VERSION_2X));
    EXPECT_CALL(hostImplMock, getDefaultVideoPortName())
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Return(videoPort));
    EXPECT_CALL(videoOutputPortConfigImplMock, getPort(::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::ReturnRef(videoOutputPort));

    EXPECT_EQ(Core::ERROR_NONE, interface->SupportedHdcp(string(), supportedHDCPVersion));
    EXPECT_EQ(supportedHDCPVersion, Exchange::IDeviceVideoCapabilities::CopyProtection::HDCP_22);
}

TEST_F(DeviceVideoCapabilitiesDsTest, SupportedVideoDisplays_exception)
{
    RPC::IStringIterator* supportedVideoDisplays = nullptr;

    EXPECT_CALL(hostImplMock, getVideoOutputPorts())
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&]() -> device::List<device::VideoOutputPort> {
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, interface->SupportedVideoDisplays(supportedVideoDisplays));
    EXPECT_EQ(supportedVideoDisplays, nullptr);
}

TEST_F(DeviceVideoCapabilitiesDsTest, HostEDID_exception)
{
    string edid;

    EXPECT_CALL(hostImplMock, getHostEDID(::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](std::vector<uint8_t>& edid) {
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, interface->HostEDID(edid));
    EXPECT_EQ(edid, string());
}

TEST_F(DeviceVideoCapabilitiesDsTest, DefaultResolution_HDMI0_exception)
{
    string defaultResolution;

    EXPECT_CALL(hostImplMock, getVideoOutputPort(::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const std::string& name) -> device::VideoOutputPort& {
                EXPECT_EQ(name, _T("HDMI0"));
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, interface->DefaultResolution(string(_T("HDMI0")), defaultResolution));
    EXPECT_EQ(defaultResolution, string());
}

TEST_F(DeviceVideoCapabilitiesDsTest, SupportedResolutions_HDMI0_exception)
{
    RPC::IStringIterator* supportedResolutions = nullptr;

    EXPECT_CALL(hostImplMock, getVideoOutputPort(::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const std::string& name) -> device::VideoOutputPort& {
                EXPECT_EQ(name, _T("HDMI0"));
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, interface->SupportedResolutions(string(_T("HDMI0")), supportedResolutions));
    EXPECT_EQ(supportedResolutions, nullptr);
}

TEST_F(DeviceVideoCapabilitiesDsTest, SupportedHdcp_HDMI0_exception)
{
    auto supportedHDCPVersion = Exchange::IDeviceVideoCapabilities::CopyProtection::HDCP_UNAVAILABLE;

    EXPECT_CALL(videoOutputPortConfigImplMock, getPort(::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const std::string& name) -> device::VideoOutputPort& {
                EXPECT_EQ(name, _T("HDMI0"));
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, interface->SupportedHdcp(string(_T("HDMI0")), supportedHDCPVersion));
    EXPECT_EQ(supportedHDCPVersion, Exchange::IDeviceVideoCapabilities::CopyProtection::HDCP_UNAVAILABLE);
}
