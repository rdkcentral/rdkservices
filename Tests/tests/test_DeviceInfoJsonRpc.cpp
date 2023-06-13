#include <gtest/gtest.h>

#include "DeviceInfo.h"

#include "AudioOutputPortMock.h"
#include "HostMock.h"
#include "IarmBusMock.h"
#include "ManagerMock.h"
#include "ServiceMock.h"
#include "VideoOutputPortConfigMock.h"
#include "VideoOutputPortMock.h"
#include "VideoOutputPortTypeMock.h"
#include "VideoResolutionMock.h"

#include "SystemInfo.h"

#include <fstream>

using namespace WPEFramework;

using ::testing::NiceMock;

namespace {
const string webPrefix = _T("/Service/DeviceInfo");
}

class DeviceInfoJsonRpcTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::DeviceInfo> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    DeviceInfoJsonRpcTest()
        : plugin(Core::ProxyType<Plugin::DeviceInfo>::Create())
        , handler(*plugin)
        , connection(1, 0)
    {
    }
    virtual ~DeviceInfoJsonRpcTest() = default;
};

class DeviceInfoJsonRpcInitializedTest : public DeviceInfoJsonRpcTest {
protected:
    NiceMock<IarmBusImplMock> iarmBusImplMock;
    NiceMock<ManagerImplMock> managerImplMock;
    NiceMock<ServiceMock> service;
    Core::Sink<NiceMock<SystemInfo>> subSystem;

    DeviceInfoJsonRpcInitializedTest()
        : DeviceInfoJsonRpcTest()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;
        device::Manager::getInstance().impl = &managerImplMock;

        ON_CALL(service, ConfigLine())
            .WillByDefault(::testing::Return("{\"root\":{\"mode\":\"Off\"}}"));
        ON_CALL(service, WebPrefix())
            .WillByDefault(::testing::Return(webPrefix));
        ON_CALL(service, SubSystems())
            .WillByDefault(::testing::Invoke(
                [&]() {
                    PluginHost::ISubSystem* result = (&subSystem);
                    result->AddRef();
                    return result;
                }));

        EXPECT_EQ(string(""), plugin->Initialize(&service));
    }
    virtual ~DeviceInfoJsonRpcInitializedTest() override
    {
        plugin->Deinitialize(&service);

        IarmBus::getInstance().impl = nullptr;
        device::Manager::getInstance().impl = nullptr;
    }
};

class DeviceInfoJsonRpcInitializedDsTest : public DeviceInfoJsonRpcInitializedTest {
protected:
    NiceMock<HostImplMock> hostImplMock;

    DeviceInfoJsonRpcInitializedDsTest()
        : DeviceInfoJsonRpcInitializedTest()
    {
        device::Host::getInstance().impl = &hostImplMock;
    }
    virtual ~DeviceInfoJsonRpcInitializedDsTest() override
    {
        device::Host::getInstance().impl = nullptr;
    }
};

class DeviceInfoJsonRpcInitializedDsVideoOutputTest : public DeviceInfoJsonRpcInitializedDsTest {
protected:
    NiceMock<VideoOutputPortConfigImplMock> videoOutputPortConfigImplMock;

    DeviceInfoJsonRpcInitializedDsVideoOutputTest()
        : DeviceInfoJsonRpcInitializedDsTest()
    {
        device::VideoOutputPortConfig::getInstance().impl = &videoOutputPortConfigImplMock;
    }
    virtual ~DeviceInfoJsonRpcInitializedDsVideoOutputTest() override
    {
        device::VideoOutputPortConfig::getInstance().impl = nullptr;
    }
};

TEST_F(DeviceInfoJsonRpcTest, registeredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("socketinfo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("addresses")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("systeminfo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("firmwareversion")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("serialnumber")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("modelid")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("make")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("modelname")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("devicetype")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("distributorid")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("supportedaudioports")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("supportedvideodisplays")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("hostedid")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("defaultresolution")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("supportedresolutions")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("supportedhdcp")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("audiocapabilities")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("ms12capabilities")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("supportedms12audioprofiles")));
}

TEST_F(DeviceInfoJsonRpcInitializedTest, systeminfo)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("systeminfo"), _T(""), response));
    EXPECT_THAT(response, ::testing::MatchesRegex("\\{"
                                                  "\"version\":\"#\","
                                                  "\"uptime\":[0-9]+,"
                                                  "\"totalram\":[0-9]+,"
                                                  "\"freeram\":[0-9]+,"
                                                  "\"totalswap\":[0-9]+,"
                                                  "\"freeswap\":[0-9]+,"
                                                  "\"devicename\":\".+\","
                                                  "\"cpuload\":\"[0-9]+\","
                                                  "\"cpuloadavg\":"
                                                  "\\{"
                                                  "\"avg1min\":[0-9]+,"
                                                  "\"avg5min\":[0-9]+,"
                                                  "\"avg15min\":[0-9]+"
                                                  "\\},"
                                                  "\"serialnumber\":\".+\","
                                                  "\"time\":\".+\""
                                                  "\\}"));
}

TEST_F(DeviceInfoJsonRpcInitializedTest, addresses)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("addresses"), _T(""), response));
    EXPECT_THAT(response, ::testing::MatchesRegex("\\["
                                                  "(\\{"
                                                  "\"name\":\"[^\"]+\","
                                                  "\"mac\":\"[^\"]+\""
                                                  "(,\"ip\":\\[(\"[^\"]+\",{0,1}){1,}\\]){0,1}"
                                                  "\\},{0,1}){0,}"
                                                  "\\]"));
}

TEST_F(DeviceInfoJsonRpcInitializedTest, socketinfo)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("socketinfo"), _T(""), response));
    EXPECT_THAT(response, ::testing::MatchesRegex("\\{\"runs\":[0-9]+\\}"));
}

TEST_F(DeviceInfoJsonRpcInitializedTest, firmwareversion)
{
    std::ofstream file("/version.txt");
    file << "imagename:PX051AEI_VBN_2203_sprint_20220331225312sdy_NG\nSDK_VERSION=17.3\nMEDIARITE=8.3.53\nYOCTO_VERSION=dunfell\n";
    file.close();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("firmwareversion"), _T(""), response));
    EXPECT_EQ(response, _T("{\"imagename\":\"PX051AEI_VBN_2203_sprint_20220331225312sdy_NG\",\"sdk\":\"17.3\",\"mediarite\":\"8.3.53\",\"yocto\":\"dunfell\"}"));
}

TEST_F(DeviceInfoJsonRpcInitializedTest, make)
{
    std::ofstream file("/etc/device.properties");
    file << "MFG_NAME=Pace\nFRIENDLY_ID=\"Pace Xi5\"\n";
    file.close();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("make"), _T(""), response));
    EXPECT_EQ(response, _T("{\"make\":\"pace\"}"));
}

TEST_F(DeviceInfoJsonRpcInitializedTest, modelname)
{
    std::ofstream file("/etc/device.properties");
    file << "MFG_NAME=Pace\nFRIENDLY_ID=\"Pace Xi5\"\n";
    file.close();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("modelname"), _T(""), response));
    EXPECT_EQ(response, _T("{\"model\":\"Pace Xi5\"}"));
}

TEST_F(DeviceInfoJsonRpcInitializedTest, devicetype)
{
    std::ofstream file("/etc/authService.conf");
    file << "deviceType=IpStb";
    file.close();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("devicetype"), _T(""), response));
    EXPECT_EQ(response, _T("{\"devicetype\":\"IpStb\"}"));
}

TEST_F(DeviceInfoJsonRpcInitializedDsTest, supportedaudioports)
{
    NiceMock<AudioOutputPortMock> audioOutputPortMock;
    device::AudioOutputPort audioOutputPort;
    audioOutputPort.impl = &audioOutputPortMock;
    string audioPort(_T("HDMI0"));

    ON_CALL(audioOutputPortMock, getName())
        .WillByDefault(::testing::ReturnRef(audioPort));
    ON_CALL(hostImplMock, getAudioOutputPorts())
        .WillByDefault(::testing::Return(device::List<device::AudioOutputPort>({ audioOutputPort })));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("supportedaudioports"), _T(""), response));
    EXPECT_EQ(response, _T("{\"supportedAudioPorts\":[\"HDMI0\"]}"));
}

TEST_F(DeviceInfoJsonRpcInitializedDsTest, supportedvideodisplays)
{
    NiceMock<VideoOutputPortMock> videoOutputPortMock;
    device::VideoOutputPort videoOutputPort;
    videoOutputPort.impl = &videoOutputPortMock;
    string videoPort(_T("HDMI0"));

    ON_CALL(videoOutputPortMock, getName())
        .WillByDefault(::testing::ReturnRef(videoPort));
    ON_CALL(hostImplMock, getVideoOutputPorts())
        .WillByDefault(::testing::Return(device::List<device::VideoOutputPort>({ videoOutputPort })));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("supportedvideodisplays"), _T(""), response));
    EXPECT_EQ(response, _T("{\"supportedVideoDisplays\":[\"HDMI0\"]}"));
}

TEST_F(DeviceInfoJsonRpcInitializedDsTest, hostedid)
{
    ON_CALL(hostImplMock, getHostEDID(::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](std::vector<uint8_t>& edid) {
                edid = { 't', 'e', 's', 't' };
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("hostedid"), _T(""), response));
    EXPECT_EQ(response, _T("{\"EDID\":\"dGVzdA==\"}"));
}

TEST_F(DeviceInfoJsonRpcInitializedDsTest, defaultresolution)
{
    NiceMock<VideoOutputPortMock> videoOutputPortMock;
    device::VideoOutputPort videoOutputPort;
    videoOutputPort.impl = &videoOutputPortMock;
    NiceMock<VideoResolutionMock> videoResolutionMock;
    device::VideoResolution videoResolution;
    videoResolution.impl = &videoResolutionMock;
    string videoPort(_T("HDMI0"));
    string videoPortDefaultResolution(_T("1080p"));

    ON_CALL(videoResolutionMock, getName())
        .WillByDefault(::testing::ReturnRef(videoPortDefaultResolution));
    ON_CALL(videoOutputPortMock, getDefaultResolution())
        .WillByDefault(::testing::ReturnRef(videoResolution));
    ON_CALL(hostImplMock, getDefaultVideoPortName())
        .WillByDefault(::testing::Return(videoPort));
    ON_CALL(hostImplMock, getVideoOutputPort(::testing::_))
        .WillByDefault(::testing::ReturnRef(videoOutputPort));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("defaultresolution"), _T(""), response));
    EXPECT_EQ(response, _T("{\"defaultResolution\":\"1080p\"}"));
}

TEST_F(DeviceInfoJsonRpcInitializedDsVideoOutputTest, supportedresolutions)
{
    NiceMock<VideoOutputPortMock> videoOutputPortMock;
    device::VideoOutputPort videoOutputPort;
    videoOutputPort.impl = &videoOutputPortMock;
    NiceMock<VideoOutputPortTypeMock> videoOutputPortTypeMock;
    device::VideoOutputPortType videoOutputPortType;
    videoOutputPortType.impl = &videoOutputPortTypeMock;
    NiceMock<VideoResolutionMock> videoResolutionMock;
    device::VideoResolution videoResolution;
    videoResolution.impl = &videoResolutionMock;
    string videoPort(_T("HDMI0"));
    string videoPortSupportedResolution(_T("1080p"));

    ON_CALL(videoResolutionMock, getName())
        .WillByDefault(::testing::ReturnRef(videoPortSupportedResolution));
    ON_CALL(videoOutputPortTypeMock, getSupportedResolutions())
        .WillByDefault(::testing::Return(device::List<device::VideoResolution>({ videoResolution })));
    ON_CALL(videoOutputPortTypeMock, getId())
        .WillByDefault(::testing::Return(0));
    ON_CALL(videoOutputPortMock, getType())
        .WillByDefault(::testing::ReturnRef(videoOutputPortType));
    ON_CALL(hostImplMock, getDefaultVideoPortName())
        .WillByDefault(::testing::Return(videoPort));
    ON_CALL(hostImplMock, getVideoOutputPort(::testing::_))
        .WillByDefault(::testing::ReturnRef(videoOutputPort));
    ON_CALL(videoOutputPortConfigImplMock, getPortType(::testing::_))
        .WillByDefault(::testing::ReturnRef(videoOutputPortType));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("supportedresolutions"), _T(""), response));
    EXPECT_EQ(response, _T("{\"supportedResolutions\":[\"1080p\"]}"));
}

TEST_F(DeviceInfoJsonRpcInitializedDsVideoOutputTest, supportedhdcp)
{
    NiceMock<VideoOutputPortMock> videoOutputPortMock;
    device::VideoOutputPort videoOutputPort;
    videoOutputPort.impl = &videoOutputPortMock;
    string videoPort(_T("HDMI0"));

    ON_CALL(videoOutputPortMock, getHDCPProtocol())
        .WillByDefault(::testing::Return(dsHDCP_VERSION_2X));
    ON_CALL(hostImplMock, getDefaultVideoPortName())
        .WillByDefault(::testing::Return(videoPort));
    ON_CALL(videoOutputPortConfigImplMock, getPort(::testing::_))
        .WillByDefault(::testing::ReturnRef(videoOutputPort));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("supportedhdcp"), _T(""), response));
    EXPECT_EQ(response, _T("{\"supportedHDCPVersion\":\"2.2\"}"));
}

TEST_F(DeviceInfoJsonRpcInitializedDsTest, audiocapabilities)
{
    NiceMock<AudioOutputPortMock> audioOutputPortMock;
    device::AudioOutputPort audioOutputPort;
    audioOutputPort.impl = &audioOutputPortMock;
    string audioPort(_T("HDMI0"));

    ON_CALL(audioOutputPortMock, getAudioCapabilities(::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](int* capabilities) {
                ASSERT_TRUE(capabilities != nullptr);
                EXPECT_EQ(*capabilities, dsAUDIOSUPPORT_NONE);
                *capabilities = dsAUDIOSUPPORT_ATMOS | dsAUDIOSUPPORT_DD | dsAUDIOSUPPORT_DDPLUS | dsAUDIOSUPPORT_DAD | dsAUDIOSUPPORT_DAPv2 | dsAUDIOSUPPORT_MS12;
            }));
    ON_CALL(hostImplMock, getDefaultAudioPortName())
        .WillByDefault(::testing::Return(audioPort));
    ON_CALL(hostImplMock, getAudioOutputPort(::testing::_))
        .WillByDefault(::testing::ReturnRef(audioOutputPort));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("audiocapabilities"), _T(""), response));
    EXPECT_EQ(response, _T("{\"AudioCapabilities\":[\"ATMOS\",\"DOLBY DIGITAL\",\"DOLBY DIGITAL PLUS\",\"Dual Audio Decode\",\"DAPv2\",\"MS12\"]}"));
}

TEST_F(DeviceInfoJsonRpcInitializedDsTest, ms12capabilities)
{
    NiceMock<AudioOutputPortMock> audioOutputPortMock;
    device::AudioOutputPort audioOutputPort;
    audioOutputPort.impl = &audioOutputPortMock;
    string audioPort(_T("HDMI0"));

    ON_CALL(audioOutputPortMock, getMS12Capabilities(::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](int* capabilities) {
                ASSERT_TRUE(capabilities != nullptr);
                EXPECT_EQ(*capabilities, dsMS12SUPPORT_NONE);
                *capabilities = dsMS12SUPPORT_DolbyVolume | dsMS12SUPPORT_InteligentEqualizer | dsMS12SUPPORT_DialogueEnhancer;
            }));
    ON_CALL(hostImplMock, getDefaultAudioPortName())
        .WillByDefault(::testing::Return(audioPort));
    ON_CALL(hostImplMock, getAudioOutputPort(::testing::_))
        .WillByDefault(::testing::ReturnRef(audioOutputPort));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("ms12capabilities"), _T(""), response));
    EXPECT_EQ(response, _T("{\"MS12Capabilities\":[\"Dolby Volume\",\"Inteligent Equalizer\",\"Dialogue Enhancer\"]}"));
}

TEST_F(DeviceInfoJsonRpcInitializedDsTest, supportedms12audioprofiles)
{
    NiceMock<AudioOutputPortMock> audioOutputPortMock;
    device::AudioOutputPort audioOutputPort;
    audioOutputPort.impl = &audioOutputPortMock;
    string audioPort(_T("HDMI0"));
    string audioPortMS12AudioProfile(_T("Movie"));

    ON_CALL(audioOutputPortMock, getMS12AudioProfileList())
        .WillByDefault(::testing::Return(std::vector<std::string>({ audioPortMS12AudioProfile })));
    ON_CALL(hostImplMock, getDefaultAudioPortName())
        .WillByDefault(::testing::Return(audioPort));
    ON_CALL(hostImplMock, getAudioOutputPort(::testing::_))
        .WillByDefault(::testing::ReturnRef(audioOutputPort));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("supportedms12audioprofiles"), _T(""), response));
    EXPECT_EQ(response, _T("{\"supportedMS12AudioProfiles\":[\"Movie\"]}"));
}
