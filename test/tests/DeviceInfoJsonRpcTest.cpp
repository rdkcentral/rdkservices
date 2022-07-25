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

class DeviceInfoJsonRpcTestFixture : public ::testing::Test {
protected:
    IarmBusImplMock iarmBusImplMock;
    ManagerImplMock managerImplMock;
    HostImplMock hostImplMock;
    ServiceMock service;
    Core::Sink<SystemInfo> subSystem;
    VideoOutputPortConfigImplMock videoOutputPortConfigImplMock;

    Core::ProxyType<Plugin::DeviceInfo> plugin;
    Core::JSONRPC::Handler& handler;

    Core::JSONRPC::Connection connection;
    Core::JSONRPC::Message message;
    string response;

    DeviceInfoJsonRpcTestFixture()
        : plugin(Core::ProxyType<Plugin::DeviceInfo>::Create())
        , handler(*plugin)
        , connection(1, 0)
    {
    }
    virtual ~DeviceInfoJsonRpcTestFixture()
    {
    }

    virtual void SetUp()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;
        device::Manager::getInstance().impl = &managerImplMock;
        device::Host::getInstance().impl = &hostImplMock;
        device::VideoOutputPortConfig::getInstance().impl = &videoOutputPortConfigImplMock;
    }

    virtual void TearDown()
    {
        IarmBus::getInstance().impl = nullptr;
        device::Manager::getInstance().impl = nullptr;
        device::Host::getInstance().impl = nullptr;
        device::VideoOutputPortConfig::getInstance().impl = nullptr;
    }

    void Activate()
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

    void Deactivate()
    {
        plugin->Deinitialize(&service);
    }
};

TEST_F(DeviceInfoJsonRpcTestFixture, registeredMethods)
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

TEST_F(DeviceInfoJsonRpcTestFixture, activate_systeminfo_deactivate)
{
    Activate();

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

    Deactivate();
}

TEST_F(DeviceInfoJsonRpcTestFixture, activate_addresses_deactivate)
{
    Activate();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("addresses"), _T(""), response));
    EXPECT_THAT(response, ::testing::MatchesRegex("\\[(\\{\"name\":\".+\",\"mac\":\".+\",\"ip\":\\[(\".+\"){0,}\\]\\}){0,}\\]"));

    Deactivate();
}

TEST_F(DeviceInfoJsonRpcTestFixture, activate_socketinfo_deactivate)
{
    Activate();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("socketinfo"), _T(""), response));
    EXPECT_THAT(response, ::testing::MatchesRegex("\\{\"runs\":[0-9]+\\}"));

    Deactivate();
}

TEST_F(DeviceInfoJsonRpcTestFixture, activate_firmwareversion_deactivate)
{
    Activate();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("firmwareversion"), _T(""), response));
    EXPECT_EQ(response, _T("{\"imagename\":\"PX051AEI_VBN_2203_sprint_20220331225312sdy_NG\",\"sdk\":\"17.3\",\"mediarite\":\"8.3.53\",\"yocto\":\"dunfell\"}"));

    Deactivate();
}

TEST_F(DeviceInfoJsonRpcTestFixture, activate_make_deactivate)
{
    Activate();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("make"), _T(""), response));
    EXPECT_EQ(response, _T("{\"make\":\"pace\"}"));

    Deactivate();
}

TEST_F(DeviceInfoJsonRpcTestFixture, activate_modelname_deactivate)
{
    Activate();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("modelname"), _T(""), response));
    EXPECT_EQ(response, _T("{\"model\":\"Pace Xi5\"}"));

    Deactivate();
}

TEST_F(DeviceInfoJsonRpcTestFixture, activate_devicetype_deactivate)
{
    Activate();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("devicetype"), _T(""), response));
    EXPECT_EQ(response, _T("{\"devicetype\":\"IpStb\"}"));

    Deactivate();
}

TEST_F(DeviceInfoJsonRpcTestFixture, activate_supportedaudioports_deactivate)
{
    Activate();

    AudioOutputPortMock audioOutputPortMock;
    string audioPort(_T("HDMI0"));

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

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("supportedaudioports"), _T(""), response));
    EXPECT_EQ(response, _T("{\"supportedAudioPorts\":[\"HDMI0\"]}"));

    Deactivate();
}

TEST_F(DeviceInfoJsonRpcTestFixture, activate_supportedvideodisplays_deactivate)
{
    Activate();

    VideoOutputPortMock videoOutputPortMock;
    string videoPort(_T("HDMI0"));

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

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("supportedvideodisplays"), _T(""), response));
    EXPECT_EQ(response, _T("{\"supportedVideoDisplays\":[\"HDMI0\"]}"));

    Deactivate();
}

TEST_F(DeviceInfoJsonRpcTestFixture, activate_hostedid_deactivate)
{
    Activate();

    std::vector<uint8_t> edidVec({ 't', 'e', 's', 't' });

    EXPECT_CALL(hostImplMock, getHostEDID(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](std::vector<uint8_t>& edid) {
                edid = edidVec;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("hostedid"), _T(""), response));
    EXPECT_EQ(response, _T("{\"EDID\":\"dGVzdA==\"}"));

    Deactivate();
}

TEST_F(DeviceInfoJsonRpcTestFixture, activate_defaultresolution_deactivate)
{
    Activate();

    VideoOutputPortMock videoOutputPortMock;
    VideoResolutionMock videoResolutionMock;
    string videoPort(_T("HDMI0"));
    string videoPortDefaultResolution(_T("1080p"));

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

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("defaultresolution"), _T(""), response));
    EXPECT_EQ(response, _T("{\"defaultResolution\":\"1080p\"}"));

    Deactivate();
}

TEST_F(DeviceInfoJsonRpcTestFixture, activate_supportedresolutions_deactivate)
{
    Activate();

    VideoOutputPortMock videoOutputPortMock;
    VideoOutputPortTypeMock videoOutputPortTypeMock;
    VideoResolutionMock videoResolutionMock;
    string videoPort(_T("HDMI0"));
    string videoPortSupportedResolution(_T("1080p"));

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

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("supportedresolutions"), _T(""), response));
    EXPECT_EQ(response, _T("{\"supportedResolutions\":[\"1080p\"]}"));

    Deactivate();
}

TEST_F(DeviceInfoJsonRpcTestFixture, activate_supportedhdcp_deactivate)
{
    Activate();

    VideoOutputPortMock videoOutputPortMock;
    string videoPort(_T("HDMI0"));

    EXPECT_CALL(videoOutputPortMock, getHDCPProtocol())
        .Times(1)
        .WillOnce(::testing::Return(dsHDCP_VERSION_2X));
    EXPECT_CALL(hostImplMock, getDefaultVideoPortName())
        .Times(1)
        .WillOnce(::testing::Return(videoPort));
    EXPECT_CALL(videoOutputPortConfigImplMock, getPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::ReturnRef(videoOutputPortMock));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("supportedhdcp"), _T(""), response));
    EXPECT_EQ(response, _T("{\"supportedHDCPVersion\":\"2.2\"}"));

    Deactivate();
}

TEST_F(DeviceInfoJsonRpcTestFixture, activate_audiocapabilities_deactivate)
{
    Activate();

    AudioOutputPortMock audioOutputPortMock;
    string audioPort(_T("HDMI0"));

    EXPECT_CALL(audioOutputPortMock, getAudioCapabilities(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](int* capabilities) {
                EXPECT_TRUE(capabilities != nullptr);
                EXPECT_EQ(*capabilities, dsAUDIOSUPPORT_NONE);
                *capabilities = dsAUDIOSUPPORT_ATMOS | dsAUDIOSUPPORT_DD | dsAUDIOSUPPORT_DDPLUS | dsAUDIOSUPPORT_DAD | dsAUDIOSUPPORT_DAPv2 | dsAUDIOSUPPORT_MS12;
            }));
    EXPECT_CALL(hostImplMock, getDefaultAudioPortName())
        .Times(1)
        .WillOnce(::testing::Return(audioPort));
    EXPECT_CALL(hostImplMock, getAudioOutputPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::ReturnRef(audioOutputPortMock));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("audiocapabilities"), _T(""), response));
    EXPECT_EQ(response, _T("{\"AudioCapabilities\":[\"ATMOS\",\"DOLBY DIGITAL\",\"DOLBY DIGITAL PLUS\",\"Dual Audio Decode\",\"DAPv2\",\"MS12\"]}"));

    Deactivate();
}

TEST_F(DeviceInfoJsonRpcTestFixture, activate_ms12capabilities_deactivate)
{
    Activate();

    AudioOutputPortMock audioOutputPortMock;
    string audioPort(_T("HDMI0"));

    EXPECT_CALL(audioOutputPortMock, getMS12Capabilities(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](int* capabilities) {
                EXPECT_TRUE(capabilities != nullptr);
                EXPECT_EQ(*capabilities, dsMS12SUPPORT_NONE);
                *capabilities = dsMS12SUPPORT_DolbyVolume | dsMS12SUPPORT_InteligentEqualizer | dsMS12SUPPORT_DialogueEnhancer;
            }));
    EXPECT_CALL(hostImplMock, getDefaultAudioPortName())
        .Times(1)
        .WillOnce(::testing::Return(audioPort));
    EXPECT_CALL(hostImplMock, getAudioOutputPort(::testing::_))
        .Times(1)
        .WillOnce(::testing::ReturnRef(audioOutputPortMock));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("ms12capabilities"), _T(""), response));
    EXPECT_EQ(response, _T("{\"MS12Capabilities\":[\"Dolby Volume\",\"Inteligent Equalizer\",\"Dialogue Enhancer\"]}"));

    Deactivate();
}

TEST_F(DeviceInfoJsonRpcTestFixture, activate_supportedms12audioprofiles_deactivate)
{
    Activate();

    AudioOutputPortMock audioOutputPortMock;
    string audioPort(_T("HDMI0"));
    string audioPortMS12AudioProfile(_T("Movie"));

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

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("supportedms12audioprofiles"), _T(""), response));
    EXPECT_EQ(response, _T("{\"supportedMS12AudioProfiles\":[\"Movie\"]}"));

    Deactivate();
}
