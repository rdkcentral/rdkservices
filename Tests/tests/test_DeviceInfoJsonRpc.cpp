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

#include "source/SystemInfo.h"

using namespace WPEFramework;

namespace {
const string webPrefix = _T("/Service/DeviceInfo");
const string iarmName = _T("Thunder_Plugins");
}

class DeviceInfoJsonRpcTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::DeviceInfo> plugin;
    Core::JSONRPC::Handler& handler;

    DeviceInfoJsonRpcTest()
        : plugin(Core::ProxyType<Plugin::DeviceInfo>::Create())
        , handler(*plugin)
    {
    }
    virtual ~DeviceInfoJsonRpcTest()
    {
    }
};

class DeviceInfoJsonRpcInitializedTest : public DeviceInfoJsonRpcTest {
protected:
    IarmBusImplMock iarmBusImplMock;
    ManagerImplMock managerImplMock;
    ServiceMock service;
    Core::Sink<SystemInfo> subSystem;
    Core::JSONRPC::Connection connection;
    string response;

    DeviceInfoJsonRpcInitializedTest()
        : DeviceInfoJsonRpcTest()
        , connection(1, 0)
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;
        device::Manager::getInstance().impl = &managerImplMock;
    }
    virtual ~DeviceInfoJsonRpcInitializedTest()
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
        EXPECT_CALL(service, WebPrefix())
            .Times(1)
            .WillOnce(::testing::Return(webPrefix));
        ON_CALL(service, Version())
            .WillByDefault(::testing::Return(string()));
        EXPECT_CALL(service, SubSystems())
            .Times(1)
            .WillRepeatedly(::testing::Invoke(
                [&]() {
                    PluginHost::ISubSystem* result = (&subSystem);
                    result->AddRef();
                    return result;
                }));

        EXPECT_EQ(string(""), plugin->Initialize(&service));
    }

    virtual void TearDown()
    {
        plugin->Deinitialize(&service);
    }
};

class DeviceInfoJsonRpcDsTest : public DeviceInfoJsonRpcInitializedTest {
protected:
    HostImplMock hostImplMock;
    VideoOutputPortConfigImplMock videoOutputPortConfigImplMock;

    DeviceInfoJsonRpcDsTest()
        : DeviceInfoJsonRpcInitializedTest()
    {
        device::Host::getInstance().impl = &hostImplMock;
        device::VideoOutputPortConfig::getInstance().impl = &videoOutputPortConfigImplMock;
    }
    virtual ~DeviceInfoJsonRpcDsTest()
    {
        device::Host::getInstance().impl = nullptr;
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
    EXPECT_THAT(response, ::testing::MatchesRegex("\\[(\\{\"name\":\".+\",\"mac\":\".+\"(,\"ip\":\\[(\".+\"){1,}\\]){0,}\\}){0,}\\]"));
}

TEST_F(DeviceInfoJsonRpcInitializedTest, socketinfo)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("socketinfo"), _T(""), response));
    EXPECT_THAT(response, ::testing::MatchesRegex("\\{\"runs\":[0-9]+\\}"));
}

TEST_F(DeviceInfoJsonRpcInitializedTest, firmwareversion)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("firmwareversion"), _T(""), response));
    EXPECT_EQ(response, _T("{\"imagename\":\"PX051AEI_VBN_2203_sprint_20220331225312sdy_NG\",\"sdk\":\"17.3\",\"mediarite\":\"8.3.53\",\"yocto\":\"dunfell\"}"));
}

TEST_F(DeviceInfoJsonRpcInitializedTest, make)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("make"), _T(""), response));
    EXPECT_EQ(response, _T("{\"make\":\"pace\"}"));
}

TEST_F(DeviceInfoJsonRpcInitializedTest, modelname)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("modelname"), _T(""), response));
    EXPECT_EQ(response, _T("{\"model\":\"Pace Xi5\"}"));
}

TEST_F(DeviceInfoJsonRpcInitializedTest, devicetype)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("devicetype"), _T(""), response));
    EXPECT_EQ(response, _T("{\"devicetype\":\"IpStb\"}"));
}

TEST_F(DeviceInfoJsonRpcDsTest, supportedaudioports)
{
    AudioOutputPortMock audioOutputPortMock;
    device::AudioOutputPort audioOutputPort;
    audioOutputPort.impl = &audioOutputPortMock;
    string audioPort(_T("HDMI0"));

    EXPECT_CALL(audioOutputPortMock, getName())
        .Times(1)
        .WillOnce(::testing::ReturnRef(audioPort));
    EXPECT_CALL(hostImplMock, getAudioOutputPorts())
        .Times(1)
        .WillOnce(::testing::Return(device::List<device::AudioOutputPort>({ audioOutputPort })));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("supportedaudioports"), _T(""), response));
    EXPECT_EQ(response, _T("{\"supportedAudioPorts\":[\"HDMI0\"]}"));
}

TEST_F(DeviceInfoJsonRpcDsTest, supportedvideodisplays)
{
    VideoOutputPortMock videoOutputPortMock;
    device::VideoOutputPort videoOutputPort;
    videoOutputPort.impl = &videoOutputPortMock;
    string videoPort(_T("HDMI0"));

    EXPECT_CALL(videoOutputPortMock, getName())
        .Times(1)
        .WillOnce(::testing::ReturnRef(videoPort));
    EXPECT_CALL(hostImplMock, getVideoOutputPorts())
        .Times(1)
        .WillOnce(::testing::Return(device::List<device::VideoOutputPort>({ videoOutputPort })));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("supportedvideodisplays"), _T(""), response));
    EXPECT_EQ(response, _T("{\"supportedVideoDisplays\":[\"HDMI0\"]}"));
}

TEST_F(DeviceInfoJsonRpcDsTest, hostedid)
{
    std::vector<uint8_t> edidVec({ 't', 'e', 's', 't' });

    EXPECT_CALL(hostImplMock, getHostEDID(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](std::vector<uint8_t>& edid) {
                edid = edidVec;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("hostedid"), _T(""), response));
    EXPECT_EQ(response, _T("{\"EDID\":\"dGVzdA==\"}"));
}

TEST_F(DeviceInfoJsonRpcDsTest, defaultresolution)
{
    VideoOutputPortMock videoOutputPortMock;
    device::VideoOutputPort videoOutputPort;
    videoOutputPort.impl = &videoOutputPortMock;
    VideoResolutionMock videoResolutionMock;
    device::VideoResolution videoResolution;
    videoResolution.impl = &videoResolutionMock;
    string videoPort(_T("HDMI0"));
    string videoPortDefaultResolution(_T("1080p"));

    EXPECT_CALL(videoResolutionMock, getName())
        .Times(1)
        .WillOnce(::testing::ReturnRef(videoPortDefaultResolution));
    EXPECT_CALL(videoOutputPortMock, getDefaultResolution())
        .Times(1)
        .WillOnce(::testing::ReturnRef(videoResolution));
    EXPECT_CALL(hostImplMock, getDefaultVideoPortName())
        .Times(1)
        .WillOnce(::testing::Return(videoPort));
    EXPECT_CALL(hostImplMock, getVideoOutputPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::ReturnRef(videoOutputPort));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("defaultresolution"), _T(""), response));
    EXPECT_EQ(response, _T("{\"defaultResolution\":\"1080p\"}"));
}

TEST_F(DeviceInfoJsonRpcDsTest, supportedresolutions)
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
    string videoPort(_T("HDMI0"));
    string videoPortSupportedResolution(_T("1080p"));

    EXPECT_CALL(videoResolutionMock, getName())
        .Times(1)
        .WillOnce(::testing::ReturnRef(videoPortSupportedResolution));
    EXPECT_CALL(videoOutputPortTypeMock, getSupportedResolutions())
        .Times(1)
        .WillOnce(::testing::Return(device::List<device::VideoResolution>({ videoResolution })));
    EXPECT_CALL(videoOutputPortTypeMock, getId())
        .Times(1)
        .WillOnce(::testing::Return(0));
    EXPECT_CALL(videoOutputPortMock, getType())
        .Times(1)
        .WillOnce(::testing::ReturnRef(videoOutputPortType));
    EXPECT_CALL(hostImplMock, getDefaultVideoPortName())
        .Times(1)
        .WillOnce(::testing::Return(videoPort));
    EXPECT_CALL(hostImplMock, getVideoOutputPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::ReturnRef(videoOutputPort));
    EXPECT_CALL(videoOutputPortConfigImplMock, getPortType(::testing::_))
        .Times(1)
        .WillOnce(::testing::ReturnRef(videoOutputPortType));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("supportedresolutions"), _T(""), response));
    EXPECT_EQ(response, _T("{\"supportedResolutions\":[\"1080p\"]}"));
}

TEST_F(DeviceInfoJsonRpcDsTest, supportedhdcp)
{
    VideoOutputPortMock videoOutputPortMock;
    device::VideoOutputPort videoOutputPort;
    videoOutputPort.impl = &videoOutputPortMock;
    string videoPort(_T("HDMI0"));

    EXPECT_CALL(videoOutputPortMock, getHDCPProtocol())
        .Times(1)
        .WillOnce(::testing::Return(dsHDCP_VERSION_2X));
    EXPECT_CALL(hostImplMock, getDefaultVideoPortName())
        .Times(1)
        .WillOnce(::testing::Return(videoPort));
    EXPECT_CALL(videoOutputPortConfigImplMock, getPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::ReturnRef(videoOutputPort));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("supportedhdcp"), _T(""), response));
    EXPECT_EQ(response, _T("{\"supportedHDCPVersion\":\"2.2\"}"));
}

TEST_F(DeviceInfoJsonRpcDsTest, audiocapabilities)
{
    AudioOutputPortMock audioOutputPortMock;
    device::AudioOutputPort audioOutputPort;
    audioOutputPort.impl = &audioOutputPortMock;
    string audioPort(_T("HDMI0"));

    EXPECT_CALL(audioOutputPortMock, getAudioCapabilities(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](int* capabilities) {
                ASSERT_TRUE(capabilities != nullptr);
                EXPECT_EQ(*capabilities, dsAUDIOSUPPORT_NONE);
                *capabilities = dsAUDIOSUPPORT_ATMOS | dsAUDIOSUPPORT_DD | dsAUDIOSUPPORT_DDPLUS | dsAUDIOSUPPORT_DAD | dsAUDIOSUPPORT_DAPv2 | dsAUDIOSUPPORT_MS12;
            }));
    EXPECT_CALL(hostImplMock, getDefaultAudioPortName())
        .Times(1)
        .WillOnce(::testing::Return(audioPort));
    EXPECT_CALL(hostImplMock, getAudioOutputPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::ReturnRef(audioOutputPort));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("audiocapabilities"), _T(""), response));
    EXPECT_EQ(response, _T("{\"AudioCapabilities\":[\"ATMOS\",\"DOLBY DIGITAL\",\"DOLBY DIGITAL PLUS\",\"Dual Audio Decode\",\"DAPv2\",\"MS12\"]}"));
}

TEST_F(DeviceInfoJsonRpcDsTest, ms12capabilities)
{
    AudioOutputPortMock audioOutputPortMock;
    device::AudioOutputPort audioOutputPort;
    audioOutputPort.impl = &audioOutputPortMock;
    string audioPort(_T("HDMI0"));

    EXPECT_CALL(audioOutputPortMock, getMS12Capabilities(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](int* capabilities) {
                ASSERT_TRUE(capabilities != nullptr);
                EXPECT_EQ(*capabilities, dsMS12SUPPORT_NONE);
                *capabilities = dsMS12SUPPORT_DolbyVolume | dsMS12SUPPORT_InteligentEqualizer | dsMS12SUPPORT_DialogueEnhancer;
            }));
    EXPECT_CALL(hostImplMock, getDefaultAudioPortName())
        .Times(1)
        .WillOnce(::testing::Return(audioPort));
    EXPECT_CALL(hostImplMock, getAudioOutputPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::ReturnRef(audioOutputPort));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("ms12capabilities"), _T(""), response));
    EXPECT_EQ(response, _T("{\"MS12Capabilities\":[\"Dolby Volume\",\"Inteligent Equalizer\",\"Dialogue Enhancer\"]}"));
}

TEST_F(DeviceInfoJsonRpcDsTest, supportedms12audioprofiles)
{
    AudioOutputPortMock audioOutputPortMock;
    device::AudioOutputPort audioOutputPort;
    audioOutputPort.impl = &audioOutputPortMock;
    string audioPort(_T("HDMI0"));
    string audioPortMS12AudioProfile(_T("Movie"));

    EXPECT_CALL(audioOutputPortMock, getMS12AudioProfileList())
        .Times(1)
        .WillOnce(::testing::Return(std::vector<std::string>({ audioPortMS12AudioProfile })));
    EXPECT_CALL(hostImplMock, getDefaultAudioPortName())
        .Times(1)
        .WillOnce(::testing::Return(audioPort));
    EXPECT_CALL(hostImplMock, getAudioOutputPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::ReturnRef(audioOutputPort));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("supportedms12audioprofiles"), _T(""), response));
    EXPECT_EQ(response, _T("{\"supportedMS12AudioProfiles\":[\"Movie\"]}"));
}
