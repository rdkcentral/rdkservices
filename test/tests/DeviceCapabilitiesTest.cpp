#include <gtest/gtest.h>

#include "DeviceCapabilities.h"

#include "AudioOutputPortMock.h"
#include "HostMock.h"
#include "IarmBusMock.h"
#include "ManagerMock.h"
#include "VideoOutputPortConfigMock.h"
#include "VideoOutputPortMock.h"
#include "VideoOutputPortTypeMock.h"
#include "VideoResolutionMock.h"

#include "exception.hpp"

using namespace WPEFramework;

namespace {
const string iarmName = _T("Thunder_Plugins");
}

class DeviceCapabilitiesTestFixture : public ::testing::Test {
protected:
    IarmBusImplMock iarmBusImplMock;
    ManagerImplMock managerImplMock;
    HostImplMock hostImplMock;
    VideoOutputPortConfigImplMock videoOutputPortConfigImplMock;

    Core::ProxyType<Plugin::DeviceCapabilities> deviceCapabilities;
    Exchange::IDeviceCapabilities* interface;

    DeviceCapabilitiesTestFixture()
    {
    }
    virtual ~DeviceCapabilitiesTestFixture()
    {
    }

    virtual void SetUp()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;
        device::Manager::getInstance().impl = &managerImplMock;
        device::Host::getInstance().impl = &hostImplMock;
        device::VideoOutputPortConfig::getInstance().impl = &videoOutputPortConfigImplMock;

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
        EXPECT_CALL(managerImplMock, Initialize())
            .Times(1)
            .WillOnce(::testing::Return());

        deviceCapabilities = Core::ProxyType<Plugin::DeviceCapabilities>::Create();
        interface = static_cast<Exchange::IDeviceCapabilities*>(
            deviceCapabilities->QueryInterface(Exchange::IDeviceCapabilities::ID));
        EXPECT_TRUE(interface != nullptr);
    }

    virtual void TearDown()
    {
        IarmBus::getInstance().impl = nullptr;
        device::Manager::getInstance().impl = nullptr;
        device::Host::getInstance().impl = nullptr;
        device::VideoOutputPortConfig::getInstance().impl = nullptr;

        interface->Release();
    }
};

TEST_F(DeviceCapabilitiesTestFixture, Make)
{
    string make;

    EXPECT_EQ(Core::ERROR_NONE, interface->Make(make));
    EXPECT_EQ(make, _T("Pace"));
}

TEST_F(DeviceCapabilitiesTestFixture, Model)
{
    string model;

    EXPECT_EQ(Core::ERROR_NONE, interface->Model(model));
    EXPECT_EQ(model, _T("Pace Xi5"));
}

TEST_F(DeviceCapabilitiesTestFixture, DeviceType)
{
    string deviceType;

    EXPECT_EQ(Core::ERROR_NONE, interface->DeviceType(deviceType));
    EXPECT_EQ(deviceType, _T("IpStb"));
}

TEST_F(DeviceCapabilitiesTestFixture, SupportedAudioPorts)
{
    AudioOutputPortMock audioOutputPortMock;
    RPC::IStringIterator* supportedAudioPorts = nullptr;
    string audioPort(_T("HDMI0"));
    string element;

    EXPECT_CALL(audioOutputPortMock, getName())
        .Times(1)
        .WillOnce(::testing::ReturnRef(audioPort));
    EXPECT_CALL(hostImplMock, getAudioOutputPorts())
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&]() {
                device::List<std::reference_wrapper<device::AudioOutputPort>> result{ audioOutputPortMock };
                return result;
            }));

    EXPECT_EQ(Core::ERROR_NONE, interface->SupportedAudioPorts(supportedAudioPorts));
    EXPECT_TRUE(supportedAudioPorts->Next(element));
    EXPECT_EQ(element, audioPort);

    supportedAudioPorts->Release();
}

TEST_F(DeviceCapabilitiesTestFixture, SupportedVideoDisplays)
{
    VideoOutputPortMock videoOutputPortMock;
    RPC::IStringIterator* supportedVideoDisplays = nullptr;
    string videoPort(_T("HDMI0"));
    string element;

    EXPECT_CALL(videoOutputPortMock, getName())
        .Times(1)
        .WillOnce(::testing::ReturnRef(videoPort));
    EXPECT_CALL(hostImplMock, getVideoOutputPorts())
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&]() {
                device::List<std::reference_wrapper<device::VideoOutputPort>> result{ videoOutputPortMock };
                return result;
            }));

    EXPECT_EQ(Core::ERROR_NONE, interface->SupportedVideoDisplays(supportedVideoDisplays));
    EXPECT_TRUE(supportedVideoDisplays->Next(element));
    EXPECT_EQ(element, videoPort);

    supportedVideoDisplays->Release();
}

TEST_F(DeviceCapabilitiesTestFixture, HostEDID)
{
    std::vector<uint8_t> edidVec({ 't', 'e', 's', 't' });
    string edid;

    EXPECT_CALL(hostImplMock, getHostEDID(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](std::vector<uint8_t>& edid) {
                edid = edidVec;
            }));

    EXPECT_EQ(Core::ERROR_NONE, interface->HostEDID(edid));
    EXPECT_EQ(edid, _T("dGVzdA=="));
}

TEST_F(DeviceCapabilitiesTestFixture, DefaultResolution_noParam)
{
    VideoOutputPortMock videoOutputPortMock;
    VideoResolutionMock videoResolutionMock;
    string videoPort(_T("HDMI0"));
    string videoPortDefaultResolution(_T("1080p"));
    string defaultResolution;

    EXPECT_CALL(videoResolutionMock, getName())
        .Times(1)
        .WillOnce(::testing::ReturnRef(videoPortDefaultResolution));
    EXPECT_CALL(videoOutputPortMock, getDefaultResolution())
        .Times(1)
        .WillOnce(::testing::ReturnRef(videoResolutionMock));
    EXPECT_CALL(hostImplMock, getDefaultVideoPortName())
        .Times(1)
        .WillOnce(::testing::Return(videoPort));
    EXPECT_CALL(hostImplMock, getVideoOutputPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::ReturnRef(videoOutputPortMock));

    EXPECT_EQ(Core::ERROR_NONE, interface->DefaultResolution(string(), defaultResolution));
    EXPECT_EQ(defaultResolution, videoPortDefaultResolution);
}

TEST_F(DeviceCapabilitiesTestFixture, SupportedResolutions_noParam)
{
    VideoOutputPortMock videoOutputPortMock;
    VideoOutputPortTypeMock videoOutputPortTypeMock;
    VideoResolutionMock videoResolutionMock;
    RPC::IStringIterator* supportedResolutions = nullptr;
    string videoPort(_T("HDMI0"));
    string videoPortSupportedResolution(_T("1080p"));
    string element;

    EXPECT_CALL(videoResolutionMock, getName())
        .Times(1)
        .WillOnce(::testing::ReturnRef(videoPortSupportedResolution));
    EXPECT_CALL(videoOutputPortTypeMock, getSupportedResolutions())
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&]() {
                device::List<std::reference_wrapper<device::VideoResolution>> result{ videoResolutionMock };
                return result;
            }));
    EXPECT_CALL(videoOutputPortTypeMock, getId())
        .Times(1)
        .WillOnce(::testing::Return(0));
    EXPECT_CALL(videoOutputPortMock, getType())
        .Times(1)
        .WillOnce(::testing::ReturnRef(videoOutputPortTypeMock));
    EXPECT_CALL(hostImplMock, getDefaultVideoPortName())
        .Times(1)
        .WillOnce(::testing::Return(videoPort));
    EXPECT_CALL(hostImplMock, getVideoOutputPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::ReturnRef(videoOutputPortMock));
    EXPECT_CALL(videoOutputPortConfigImplMock, getPortType(::testing::_))
        .Times(1)
        .WillOnce(::testing::ReturnRef(videoOutputPortTypeMock));

    EXPECT_EQ(Core::ERROR_NONE, interface->SupportedResolutions(string(), supportedResolutions));
    EXPECT_TRUE(supportedResolutions->Next(element));
    EXPECT_EQ(element, videoPortSupportedResolution);

    supportedResolutions->Release();
}

TEST_F(DeviceCapabilitiesTestFixture, SupportedHdcp_noParam)
{
    VideoOutputPortMock videoOutputPortMock;
    string videoPort(_T("HDMI0"));
    Exchange::IDeviceCapabilities::CopyProtection supportedHDCPVersion;

    EXPECT_CALL(videoOutputPortMock, getHDCPProtocol())
        .Times(1)
        .WillOnce(::testing::Return(dsHDCP_VERSION_2X));
    EXPECT_CALL(hostImplMock, getDefaultVideoPortName())
        .Times(1)
        .WillOnce(::testing::Return(videoPort));
    EXPECT_CALL(videoOutputPortConfigImplMock, getPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::ReturnRef(videoOutputPortMock));

    EXPECT_EQ(Core::ERROR_NONE, interface->SupportedHdcp(string(), supportedHDCPVersion));
    EXPECT_EQ(supportedHDCPVersion, Exchange::IDeviceCapabilities::CopyProtection::HDCP_22);
}

TEST_F(DeviceCapabilitiesTestFixture, AudioCapabilities_noParam)
{
    AudioOutputPortMock audioOutputPortMock;
    Exchange::IDeviceCapabilities::IAudioCapabilityIterator* audioCapabilities = nullptr;
    string audioPort(_T("HDMI0"));
    Exchange::IDeviceCapabilities::AudioCapability element;

    EXPECT_CALL(audioOutputPortMock, getAudioCapabilities(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](int* capabilities) {
                EXPECT_TRUE(capabilities != nullptr);
                EXPECT_EQ(*capabilities, dsAUDIOSUPPORT_NONE);
                *capabilities = dsAUDIOSUPPORT_ATMOS | dsAUDIOSUPPORT_DDPLUS;
            }));
    EXPECT_CALL(hostImplMock, getDefaultAudioPortName())
        .Times(1)
        .WillOnce(::testing::Return(audioPort));
    EXPECT_CALL(hostImplMock, getAudioOutputPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::ReturnRef(audioOutputPortMock));

    EXPECT_EQ(Core::ERROR_NONE, interface->AudioCapabilities(string(), audioCapabilities));
    EXPECT_TRUE(audioCapabilities->Next(element));
    EXPECT_EQ(element, Exchange::IDeviceCapabilities::AudioCapability::ATMOS);
    EXPECT_TRUE(audioCapabilities->Next(element));
    EXPECT_EQ(element, Exchange::IDeviceCapabilities::AudioCapability::DDPLUS);

    audioCapabilities->Release();
}

TEST_F(DeviceCapabilitiesTestFixture, MS12Capabilities_noParam)
{
    AudioOutputPortMock audioOutputPortMock;
    Exchange::IDeviceCapabilities::IMS12CapabilityIterator* ms12Capabilities = nullptr;
    string audioPort(_T("HDMI0"));
    Exchange::IDeviceCapabilities::MS12Capability element;

    EXPECT_CALL(audioOutputPortMock, getMS12Capabilities(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](int* capabilities) {
                EXPECT_TRUE(capabilities != nullptr);
                EXPECT_EQ(*capabilities, dsMS12SUPPORT_NONE);
                *capabilities = dsMS12SUPPORT_DolbyVolume | dsMS12SUPPORT_InteligentEqualizer;
            }));
    EXPECT_CALL(hostImplMock, getDefaultAudioPortName())
        .Times(1)
        .WillOnce(::testing::Return(audioPort));
    EXPECT_CALL(hostImplMock, getAudioOutputPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::ReturnRef(audioOutputPortMock));

    EXPECT_EQ(Core::ERROR_NONE, interface->MS12Capabilities(string(), ms12Capabilities));
    EXPECT_TRUE(ms12Capabilities->Next(element));
    EXPECT_EQ(element, Exchange::IDeviceCapabilities::MS12Capability::DOLBYVOLUME);
    EXPECT_TRUE(ms12Capabilities->Next(element));
    EXPECT_EQ(element, Exchange::IDeviceCapabilities::MS12Capability::INTELIGENTEQUALIZER);

    ms12Capabilities->Release();
}

TEST_F(DeviceCapabilitiesTestFixture, SupportedMS12AudioProfiles_noParam)
{
    AudioOutputPortMock audioOutputPortMock;
    RPC::IStringIterator* supportedMS12AudioProfiles = nullptr;
    string audioPort(_T("HDMI0"));
    string audioPortMS12AudioProfile(_T("Movie"));
    string element;

    EXPECT_CALL(audioOutputPortMock, getMS12AudioProfileList())
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&]() {
                std::vector<std::string> result{ audioPortMS12AudioProfile };
                return result;
            }));
    EXPECT_CALL(hostImplMock, getDefaultAudioPortName())
        .Times(1)
        .WillOnce(::testing::Return(audioPort));
    EXPECT_CALL(hostImplMock, getAudioOutputPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::ReturnRef(audioOutputPortMock));

    EXPECT_EQ(Core::ERROR_NONE, interface->SupportedMS12AudioProfiles(string(), supportedMS12AudioProfiles));
    EXPECT_TRUE(supportedMS12AudioProfiles->Next(element));
    EXPECT_EQ(element, audioPortMS12AudioProfile);

    supportedMS12AudioProfiles->Release();
}

TEST_F(DeviceCapabilitiesTestFixture, SupportedAudioPorts_exception)
{
    RPC::IStringIterator* supportedAudioPorts = nullptr;

    EXPECT_CALL(hostImplMock, getAudioOutputPorts())
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&]() -> device::List<std::reference_wrapper<device::AudioOutputPort>> {
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, interface->SupportedAudioPorts(supportedAudioPorts));
    EXPECT_EQ(supportedAudioPorts, nullptr);
}

TEST_F(DeviceCapabilitiesTestFixture, SupportedVideoDisplays_exception)
{
    RPC::IStringIterator* supportedVideoDisplays = nullptr;

    EXPECT_CALL(hostImplMock, getVideoOutputPorts())
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&]() -> device::List<std::reference_wrapper<device::VideoOutputPort>> {
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, interface->SupportedVideoDisplays(supportedVideoDisplays));
    EXPECT_EQ(supportedVideoDisplays, nullptr);
}

TEST_F(DeviceCapabilitiesTestFixture, HostEDID_exception)
{
    string edid;

    EXPECT_CALL(hostImplMock, getHostEDID(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](std::vector<uint8_t>& edid) {
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, interface->HostEDID(edid));
    EXPECT_EQ(edid, string());
}

TEST_F(DeviceCapabilitiesTestFixture, DefaultResolution_HDMI0_exception)
{
    string defaultResolution;

    EXPECT_CALL(hostImplMock, getVideoOutputPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const std::string& name) -> device::VideoOutputPort& {
                EXPECT_EQ(name, _T("HDMI0"));
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, interface->DefaultResolution(string(_T("HDMI0")), defaultResolution));
    EXPECT_EQ(defaultResolution, string());
}

TEST_F(DeviceCapabilitiesTestFixture, SupportedResolutions_HDMI0_exception)
{
    RPC::IStringIterator* supportedResolutions = nullptr;

    EXPECT_CALL(hostImplMock, getVideoOutputPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const std::string& name) -> device::VideoOutputPort& {
                EXPECT_EQ(name, _T("HDMI0"));
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, interface->SupportedResolutions(string(_T("HDMI0")), supportedResolutions));
    EXPECT_EQ(supportedResolutions, nullptr);
}

TEST_F(DeviceCapabilitiesTestFixture, SupportedHdcp_HDMI0_exception)
{
    Exchange::IDeviceCapabilities::CopyProtection supportedHDCPVersion;

    EXPECT_CALL(videoOutputPortConfigImplMock, getPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const std::string& name) -> device::VideoOutputPort& {
                EXPECT_EQ(name, _T("HDMI0"));
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, interface->SupportedHdcp(string(_T("HDMI0")), supportedHDCPVersion));
    EXPECT_EQ(supportedHDCPVersion, Exchange::IDeviceCapabilities::CopyProtection::HDCP_UNAVAILABLE);
}

TEST_F(DeviceCapabilitiesTestFixture, AudioCapabilities_HDMI0_exception)
{
    Exchange::IDeviceCapabilities::IAudioCapabilityIterator* audioCapabilities = nullptr;

    EXPECT_CALL(hostImplMock, getAudioOutputPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const std::string& name) -> device::AudioOutputPort& {
                EXPECT_EQ(name, _T("HDMI0"));
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, interface->AudioCapabilities(string(_T("HDMI0")), audioCapabilities));
    EXPECT_EQ(audioCapabilities, nullptr);
}

TEST_F(DeviceCapabilitiesTestFixture, MS12Capabilities_HDMI0_exception)
{
    Exchange::IDeviceCapabilities::IMS12CapabilityIterator* ms12Capabilities = nullptr;

    EXPECT_CALL(hostImplMock, getAudioOutputPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const std::string& name) -> device::AudioOutputPort& {
                EXPECT_EQ(name, _T("HDMI0"));
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, interface->MS12Capabilities(string(_T("HDMI0")), ms12Capabilities));
    EXPECT_EQ(ms12Capabilities, nullptr);
}

TEST_F(DeviceCapabilitiesTestFixture, SupportedMS12AudioProfiles_HDMI0_exception)
{
    RPC::IStringIterator* supportedMS12AudioProfiles = nullptr;

    EXPECT_CALL(hostImplMock, getAudioOutputPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const std::string& name) -> device::AudioOutputPort& {
                EXPECT_EQ(name, _T("HDMI0"));
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, interface->SupportedMS12AudioProfiles(string(_T("HDMI0")), supportedMS12AudioProfiles));
    EXPECT_EQ(supportedMS12AudioProfiles, nullptr);
}
