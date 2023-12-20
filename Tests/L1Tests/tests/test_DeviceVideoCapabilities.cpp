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

using ::testing::NiceMock;

class DeviceVideoCapabilitiesTest : public ::testing::Test {
protected:
    IarmBusImplMock   *p_iarmBusImplMock = nullptr ;
    ManagerImplMock   *p_managerImplMock = nullptr ;
    Core::ProxyType<Plugin::DeviceVideoCapabilities> deviceVideoCapabilities;
    Exchange::IDeviceVideoCapabilities* interface;

    DeviceVideoCapabilitiesTest()
    {
        p_iarmBusImplMock  = new NiceMock <IarmBusImplMock>;
        IarmBus::setImpl(p_iarmBusImplMock);

        p_managerImplMock  = new NiceMock <ManagerImplMock>;
        device::Manager::setImpl(p_managerImplMock);

        EXPECT_CALL(*p_managerImplMock, Initialize())
            .Times(::testing::AnyNumber())
            .WillRepeatedly(::testing::Return());

        deviceVideoCapabilities = Core::ProxyType<Plugin::DeviceVideoCapabilities>::Create();

        interface = static_cast<Exchange::IDeviceVideoCapabilities*>(
            deviceVideoCapabilities->QueryInterface(Exchange::IDeviceVideoCapabilities::ID));
    }
    virtual ~DeviceVideoCapabilitiesTest()
    {
        interface->Release();
        IarmBus::setImpl(nullptr);
        if (p_iarmBusImplMock != nullptr)
        {
            delete p_iarmBusImplMock;
            p_iarmBusImplMock = nullptr;
        }
        device::Manager::setImpl(nullptr);
        if (p_managerImplMock != nullptr)
        {
            delete p_managerImplMock;
            p_managerImplMock = nullptr;
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

class DeviceVideoCapabilitiesDsTest : public DeviceVideoCapabilitiesTest {
protected:
        HostImplMock      *p_hostImplMock = nullptr ;
        VideoOutputPortConfigImplMock      *p_videoOutputPortConfigImplMock = nullptr ;
        VideoOutputPortTypeMock      *p_videoOutputPortTypeMock = nullptr ;
        VideoOutputPortMock          *p_videoOutputPortMock = nullptr ;
        VideoResolutionMock          *p_videoResolutionMock = nullptr ;

    DeviceVideoCapabilitiesDsTest()
        : DeviceVideoCapabilitiesTest()
    {
        p_hostImplMock  = new NiceMock <HostImplMock>;
        device::Host::setImpl(p_hostImplMock);

        p_videoOutputPortConfigImplMock  = new NiceMock <VideoOutputPortConfigImplMock>;
        device::VideoOutputPortConfig::setImpl(p_videoOutputPortConfigImplMock);

        p_videoOutputPortTypeMock  = new NiceMock <VideoOutputPortTypeMock>;
        device::VideoOutputPortType::setImpl(p_videoOutputPortTypeMock);

        p_videoResolutionMock  = new NiceMock <VideoResolutionMock>;
        device::VideoResolution::setImpl(p_videoResolutionMock);

        p_videoOutputPortMock  = new NiceMock <VideoOutputPortMock>;
        device::VideoOutputPort::setImpl(p_videoOutputPortMock);
    }
    virtual ~DeviceVideoCapabilitiesDsTest() override
    {
        device::VideoResolution::setImpl(nullptr);
        if (p_videoResolutionMock != nullptr)
        {
            delete p_videoResolutionMock;
            p_videoResolutionMock = nullptr;
        }
        device::VideoOutputPortType::setImpl(nullptr);
        if (p_videoOutputPortTypeMock != nullptr)
        {
            delete p_videoOutputPortTypeMock;
            p_videoOutputPortTypeMock = nullptr;
        }
        device::VideoOutputPort::setImpl(nullptr);
        if (p_videoOutputPortMock != nullptr)
        {
            delete p_videoOutputPortMock;
            p_videoOutputPortMock = nullptr;
        }
        device::VideoOutputPortConfig::setImpl(nullptr);
        if (p_videoOutputPortConfigImplMock != nullptr)
        {
            delete p_videoOutputPortConfigImplMock;
            p_videoOutputPortConfigImplMock = nullptr;
        }
        device::Host::setImpl(nullptr);
        if (p_hostImplMock != nullptr)
        {
            delete p_hostImplMock;
            p_hostImplMock = nullptr;
        }
    }
};

TEST_F(DeviceVideoCapabilitiesDsTest, SupportedVideoDisplays)
{
    device::VideoOutputPort videoOutputPort;
    RPC::IStringIterator* supportedVideoDisplays = nullptr;
    string videoPort(_T("HDMI0"));
    string element;

    ON_CALL(*p_videoOutputPortMock, getName())
        .WillByDefault(::testing::ReturnRef(videoPort));
    ON_CALL(*p_hostImplMock, getVideoOutputPorts())
        .WillByDefault(::testing::Return(device::List<device::VideoOutputPort>({ videoOutputPort })));

    EXPECT_EQ(Core::ERROR_NONE, interface->SupportedVideoDisplays(supportedVideoDisplays));
    ASSERT_TRUE(supportedVideoDisplays != nullptr);
    ASSERT_TRUE(supportedVideoDisplays->Next(element));
    EXPECT_EQ(element, videoPort);

    supportedVideoDisplays->Release();
}

TEST_F(DeviceVideoCapabilitiesDsTest, HostEDID)
{
    string edid;

    ON_CALL(*p_hostImplMock, getHostEDID(::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](std::vector<uint8_t>& edid) {
                edid = { 't', 'e', 's', 't' };
            }));

    EXPECT_EQ(Core::ERROR_NONE, interface->HostEDID(edid));
    EXPECT_EQ(edid, _T("dGVzdA=="));
}

TEST_F(DeviceVideoCapabilitiesDsTest, DefaultResolution_noParam)
{
    NiceMock<VideoOutputPortMock> videoOutputPortMock;
    device::VideoOutputPort videoOutputPort;
    device::VideoResolution videoResolution;

    string videoPort(_T("HDMI0"));
    string videoPortDefaultResolution(_T("1080p"));
    string defaultResolution;

    ON_CALL(*p_videoResolutionMock, getName())
        .WillByDefault(::testing::ReturnRef(videoPortDefaultResolution));
    ON_CALL(*p_videoOutputPortMock, getDefaultResolution())
        .WillByDefault(::testing::ReturnRef(videoResolution));
    ON_CALL(*p_hostImplMock, getDefaultVideoPortName())
        .WillByDefault(::testing::Return(videoPort));
    ON_CALL(*p_hostImplMock, getVideoOutputPort(::testing::_))
        .WillByDefault(::testing::ReturnRef(videoOutputPort));

    EXPECT_EQ(Core::ERROR_NONE, interface->DefaultResolution(string(), defaultResolution));
    EXPECT_EQ(defaultResolution, videoPortDefaultResolution);
}

TEST_F(DeviceVideoCapabilitiesDsTest, SupportedResolutions_noParam)
{
    NiceMock<VideoOutputPortMock> videoOutputPortMock;
    device::VideoOutputPort videoOutputPort;
    device::VideoOutputPortType videoOutputPortType;
    device::VideoResolution videoResolution;
    RPC::IStringIterator* supportedResolutions = nullptr;
    string videoPort(_T("HDMI0"));
    string videoPortSupportedResolution(_T("1080p"));
    string element;

    ON_CALL(*p_videoResolutionMock, getName())
        .WillByDefault(::testing::ReturnRef(videoPortSupportedResolution));
    ON_CALL(*p_videoOutputPortTypeMock, getSupportedResolutions())
        .WillByDefault(::testing::Return(device::List<device::VideoResolution>({ videoResolution })));
    ON_CALL(*p_videoOutputPortTypeMock, getId())
        .WillByDefault(::testing::Return(0));
    ON_CALL(*p_videoOutputPortMock, getType())
        .WillByDefault(::testing::ReturnRef(videoOutputPortType));
    ON_CALL(*p_hostImplMock, getDefaultVideoPortName())
        .WillByDefault(::testing::Return(videoPort));
    ON_CALL(*p_hostImplMock, getVideoOutputPort(::testing::_))
        .WillByDefault(::testing::ReturnRef(videoOutputPort));
    ON_CALL(*p_videoOutputPortConfigImplMock, getPortType(::testing::_))
        .WillByDefault(::testing::ReturnRef(videoOutputPortType));

    EXPECT_EQ(Core::ERROR_NONE, interface->SupportedResolutions(string(), supportedResolutions));
    ASSERT_TRUE(supportedResolutions != nullptr);
    ASSERT_TRUE(supportedResolutions->Next(element));
    EXPECT_EQ(element, videoPortSupportedResolution);

    supportedResolutions->Release();
}

TEST_F(DeviceVideoCapabilitiesDsTest, SupportedHdcp_noParam)
{
    NiceMock<VideoOutputPortMock> videoOutputPortMock;
    device::VideoOutputPort videoOutputPort;
    string videoPort(_T("HDMI0"));
    auto supportedHDCPVersion = Exchange::IDeviceVideoCapabilities::CopyProtection::HDCP_UNAVAILABLE;

    ON_CALL(*p_videoOutputPortMock, getHDCPProtocol())
        .WillByDefault(::testing::Return(dsHDCP_VERSION_2X));
    ON_CALL(*p_hostImplMock, getDefaultVideoPortName())
        .WillByDefault(::testing::Return(videoPort));
    ON_CALL(*p_videoOutputPortConfigImplMock, getPort(::testing::_))
        .WillByDefault(::testing::ReturnRef(videoOutputPort));

    EXPECT_EQ(Core::ERROR_NONE, interface->SupportedHdcp(string(), supportedHDCPVersion));
    EXPECT_EQ(supportedHDCPVersion, Exchange::IDeviceVideoCapabilities::CopyProtection::HDCP_22);
}

TEST_F(DeviceVideoCapabilitiesDsTest, SupportedVideoDisplays_exception)
{
    RPC::IStringIterator* supportedVideoDisplays = nullptr;

    ON_CALL(*p_hostImplMock, getVideoOutputPorts())
        .WillByDefault(::testing::Invoke(
            [&]() -> device::List<device::VideoOutputPort> {
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, interface->SupportedVideoDisplays(supportedVideoDisplays));
    EXPECT_EQ(supportedVideoDisplays, nullptr);
}

TEST_F(DeviceVideoCapabilitiesDsTest, HostEDID_exception)
{
    string edid;

    ON_CALL(*p_hostImplMock, getHostEDID(::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](std::vector<uint8_t>& edid) {
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, interface->HostEDID(edid));
    EXPECT_EQ(edid, string());
}

TEST_F(DeviceVideoCapabilitiesDsTest, DefaultResolution_HDMI0_exception)
{
    string defaultResolution;

    ON_CALL(*p_hostImplMock, getVideoOutputPort(::testing::_))
        .WillByDefault(::testing::Invoke(
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

    ON_CALL(*p_hostImplMock, getVideoOutputPort(::testing::_))
        .WillByDefault(::testing::Invoke(
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

    ON_CALL(*p_videoOutputPortConfigImplMock, getPort(::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::VideoOutputPort& {
                EXPECT_EQ(name, _T("HDMI0"));
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, interface->SupportedHdcp(string(_T("HDMI0")), supportedHDCPVersion));
    EXPECT_EQ(supportedHDCPVersion, Exchange::IDeviceVideoCapabilities::CopyProtection::HDCP_UNAVAILABLE);
}
