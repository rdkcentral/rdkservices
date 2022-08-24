#include <gtest/gtest.h>

#include "Implementation/DeviceAudioCapabilities.h"

#include "AudioOutputPortMock.h"
#include "HostMock.h"
#include "IarmBusMock.h"
#include "ManagerMock.h"

#include "exception.hpp"

using namespace WPEFramework;

namespace {
const string iarmName = _T("Thunder_Plugins");
}

class DeviceAudioCapabilitiesTest : public ::testing::Test {
protected:
    IarmBusImplMock iarmBusImplMock;
    ManagerImplMock managerImplMock;
    Core::ProxyType<Plugin::DeviceAudioCapabilities> deviceAudioCapabilities;
    Exchange::IDeviceAudioCapabilities* interface;

    DeviceAudioCapabilitiesTest()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;
        device::Manager::getInstance().impl = &managerImplMock;
    }
    virtual ~DeviceAudioCapabilitiesTest()
    {
        IarmBus::getInstance().impl = nullptr;
        device::Manager::getInstance().impl = nullptr;
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
        EXPECT_CALL(managerImplMock, Initialize())
            .Times(1)
            .WillOnce(::testing::Return());

        deviceAudioCapabilities = Core::ProxyType<Plugin::DeviceAudioCapabilities>::Create();

        interface = static_cast<Exchange::IDeviceAudioCapabilities*>(
            deviceAudioCapabilities->QueryInterface(Exchange::IDeviceAudioCapabilities::ID));
        ASSERT_TRUE(interface != nullptr);
    }

    virtual void TearDown()
    {
        ASSERT_TRUE(interface != nullptr);
        interface->Release();
    }
};

class DeviceAudioCapabilitiesDsTest : public DeviceAudioCapabilitiesTest {
protected:
    HostImplMock hostImplMock;

    DeviceAudioCapabilitiesDsTest()
        : DeviceAudioCapabilitiesTest()
    {
        device::Host::getInstance().impl = &hostImplMock;
    }
    virtual ~DeviceAudioCapabilitiesDsTest()
    {
        device::Host::getInstance().impl = nullptr;
    }
};

TEST_F(DeviceAudioCapabilitiesDsTest, SupportedAudioPorts)
{
    AudioOutputPortMock audioOutputPortMock;
    device::AudioOutputPort audioOutputPort;
    audioOutputPort.impl = &audioOutputPortMock;
    RPC::IStringIterator* supportedAudioPorts = nullptr;
    string audioPort(_T("HDMI0"));
    string element;

    EXPECT_CALL(audioOutputPortMock, getName())
        .Times(1)
        .WillOnce(::testing::ReturnRef(audioPort));
    EXPECT_CALL(hostImplMock, getAudioOutputPorts())
        .Times(1)
        .WillOnce(::testing::Return(device::List<device::AudioOutputPort>({ audioOutputPort })));

    EXPECT_EQ(Core::ERROR_NONE, interface->SupportedAudioPorts(supportedAudioPorts));
    ASSERT_TRUE(supportedAudioPorts != nullptr);
    ASSERT_TRUE(supportedAudioPorts->Next(element));
    EXPECT_EQ(element, audioPort);

    supportedAudioPorts->Release();
}

TEST_F(DeviceAudioCapabilitiesDsTest, AudioCapabilities_noParam)
{
    AudioOutputPortMock audioOutputPortMock;
    device::AudioOutputPort audioOutputPort;
    audioOutputPort.impl = &audioOutputPortMock;
    Exchange::IDeviceAudioCapabilities::IAudioCapabilityIterator* audioCapabilities = nullptr;
    string audioPort(_T("HDMI0"));
    Exchange::IDeviceAudioCapabilities::AudioCapability element;

    EXPECT_CALL(audioOutputPortMock, getAudioCapabilities(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](int* capabilities) {
                ASSERT_TRUE(capabilities != nullptr);
                EXPECT_EQ(*capabilities, dsAUDIOSUPPORT_NONE);
                *capabilities = dsAUDIOSUPPORT_ATMOS | dsAUDIOSUPPORT_DDPLUS;
            }));
    EXPECT_CALL(hostImplMock, getDefaultAudioPortName())
        .Times(1)
        .WillOnce(::testing::Return(audioPort));
    EXPECT_CALL(hostImplMock, getAudioOutputPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::ReturnRef(audioOutputPort));

    EXPECT_EQ(Core::ERROR_NONE, interface->AudioCapabilities(string(), audioCapabilities));
    ASSERT_TRUE(audioCapabilities != nullptr);
    ASSERT_TRUE(audioCapabilities->Next(element));
    EXPECT_EQ(element, Exchange::IDeviceAudioCapabilities::AudioCapability::ATMOS);
    ASSERT_TRUE(audioCapabilities->Next(element));
    EXPECT_EQ(element, Exchange::IDeviceAudioCapabilities::AudioCapability::DDPLUS);

    audioCapabilities->Release();
}

TEST_F(DeviceAudioCapabilitiesDsTest, MS12Capabilities_noParam)
{
    AudioOutputPortMock audioOutputPortMock;
    device::AudioOutputPort audioOutputPort;
    audioOutputPort.impl = &audioOutputPortMock;
    Exchange::IDeviceAudioCapabilities::IMS12CapabilityIterator* ms12Capabilities = nullptr;
    string audioPort(_T("HDMI0"));
    Exchange::IDeviceAudioCapabilities::MS12Capability element;

    EXPECT_CALL(audioOutputPortMock, getMS12Capabilities(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](int* capabilities) {
                ASSERT_TRUE(capabilities != nullptr);
                EXPECT_EQ(*capabilities, dsMS12SUPPORT_NONE);
                *capabilities = dsMS12SUPPORT_DolbyVolume | dsMS12SUPPORT_InteligentEqualizer;
            }));
    EXPECT_CALL(hostImplMock, getDefaultAudioPortName())
        .Times(1)
        .WillOnce(::testing::Return(audioPort));
    EXPECT_CALL(hostImplMock, getAudioOutputPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::ReturnRef(audioOutputPort));

    EXPECT_EQ(Core::ERROR_NONE, interface->MS12Capabilities(string(), ms12Capabilities));
    ASSERT_TRUE(ms12Capabilities != nullptr);
    ASSERT_TRUE(ms12Capabilities->Next(element));
    EXPECT_EQ(element, Exchange::IDeviceAudioCapabilities::MS12Capability::DOLBYVOLUME);
    ASSERT_TRUE(ms12Capabilities->Next(element));
    EXPECT_EQ(element, Exchange::IDeviceAudioCapabilities::MS12Capability::INTELIGENTEQUALIZER);

    ms12Capabilities->Release();
}

TEST_F(DeviceAudioCapabilitiesDsTest, SupportedMS12AudioProfiles_noParam)
{
    AudioOutputPortMock audioOutputPortMock;
    device::AudioOutputPort audioOutputPort;
    audioOutputPort.impl = &audioOutputPortMock;
    RPC::IStringIterator* supportedMS12AudioProfiles = nullptr;
    string audioPort(_T("HDMI0"));
    string audioPortMS12AudioProfile(_T("Movie"));
    string element;

    EXPECT_CALL(audioOutputPortMock, getMS12AudioProfileList())
        .Times(1)
        .WillOnce(::testing::Return(std::vector<std::string>({ audioPortMS12AudioProfile })));
    EXPECT_CALL(hostImplMock, getDefaultAudioPortName())
        .Times(1)
        .WillOnce(::testing::Return(audioPort));
    EXPECT_CALL(hostImplMock, getAudioOutputPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::ReturnRef(audioOutputPort));

    EXPECT_EQ(Core::ERROR_NONE, interface->SupportedMS12AudioProfiles(string(), supportedMS12AudioProfiles));
    ASSERT_TRUE(supportedMS12AudioProfiles != nullptr);
    ASSERT_TRUE(supportedMS12AudioProfiles->Next(element));
    EXPECT_EQ(element, audioPortMS12AudioProfile);

    supportedMS12AudioProfiles->Release();
}

TEST_F(DeviceAudioCapabilitiesDsTest, SupportedAudioPorts_exception)
{
    RPC::IStringIterator* supportedAudioPorts = nullptr;

    EXPECT_CALL(hostImplMock, getAudioOutputPorts())
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&]() -> device::List<device::AudioOutputPort> {
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, interface->SupportedAudioPorts(supportedAudioPorts));
    EXPECT_EQ(supportedAudioPorts, nullptr);
}

TEST_F(DeviceAudioCapabilitiesDsTest, AudioCapabilities_HDMI0_exception)
{
    Exchange::IDeviceAudioCapabilities::IAudioCapabilityIterator* audioCapabilities = nullptr;

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

TEST_F(DeviceAudioCapabilitiesDsTest, MS12Capabilities_HDMI0_exception)
{
    Exchange::IDeviceAudioCapabilities::IMS12CapabilityIterator* ms12Capabilities = nullptr;

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

TEST_F(DeviceAudioCapabilitiesDsTest, SupportedMS12AudioProfiles_HDMI0_exception)
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
