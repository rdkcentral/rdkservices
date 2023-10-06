#include <gtest/gtest.h>

#include "HdcpProfile.h"

#include "FactoriesImplementation.h"
#include "HostMock.h"
#include "ManagerMock.h"
#include "VideoOutputPortConfigMock.h"
#include "VideoOutputPortMock.h"
#include "IarmBusMock.h"
#include "ServiceMock.h"
#include "dsMgr.h"
#include "dsDisplay.h"
#include "pwrMgr.h"
using namespace WPEFramework;

using ::testing::NiceMock;

class HDCPProfileTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::HdcpProfile> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;
    Core::JSONRPC::Message message;

    HDCPProfileTest()
        : plugin(Core::ProxyType<Plugin::HdcpProfile>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
    }
    virtual ~HDCPProfileTest() = default;

};

class HDCPProfileDsTest : public HDCPProfileTest {
protected:
    NiceMock<HostImplMock> hostImplMock;
    NiceMock<VideoOutputPortConfigImplMock> videoOutputPortConfigImplMock;

    HDCPProfileDsTest()
        : HDCPProfileTest()
    {
        device::Host::getInstance().impl = &hostImplMock;
        device::VideoOutputPortConfig::getInstance().impl = &videoOutputPortConfigImplMock;
    }
    virtual ~HDCPProfileDsTest() override
    {
        device::Host::getInstance().impl = nullptr;
        device::VideoOutputPortConfig::getInstance().impl = nullptr;
    }
};

class HDCPProfileEventTest : public HDCPProfileDsTest {
protected:
    NiceMock<ServiceMock> service;
    Core::JSONRPC::Message message;
    NiceMock<FactoriesImplementation> factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    HDCPProfileEventTest()
        : HDCPProfileDsTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }

    virtual ~HDCPProfileEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();

        PluginHost::IFactories::Assign(nullptr);
    }
};

class HDCPProfileEventIarmTest : public HDCPProfileEventTest {
protected:
    NiceMock<IarmBusImplMock> iarmBusImplMock;
    ManagerImplMock managerImplMock;
    IARM_EventHandler_t dsHdmiEventHandler;

    HDCPProfileEventIarmTest()
        : HDCPProfileEventTest()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;
        device::Manager::getInstance().impl = &managerImplMock;

        EXPECT_CALL(managerImplMock, Initialize())
            .Times(::testing::AnyNumber())
            .WillRepeatedly(::testing::Return());

        ON_CALL(iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG)) {
                        dsHdmiEventHandler = handler;
                    }
                    if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_HDCP_STATUS)) {
                         dsHdmiEventHandler = handler;
                    }
                    return IARM_RESULT_SUCCESS;
                }));

        EXPECT_EQ(string(""), plugin->Initialize(&service));
    }

    virtual ~HDCPProfileEventIarmTest() override
    {
        plugin->Deinitialize(&service);
        IarmBus::getInstance().impl = nullptr;
        device::Manager::getInstance().impl = nullptr;
    }
};

TEST_F(HDCPProfileTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getHDCPStatus")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getSettopHDCPSupport")));
}

TEST_F(HDCPProfileDsTest, getHDCPStatus_isConnected_false)
{
    NiceMock<VideoOutputPortMock> videoOutputPortMock;
    device::VideoOutputPort videoOutputPort;
    videoOutputPort.impl = &videoOutputPortMock;


    string videoPort(_T("HDMI0"));

    ON_CALL(hostImplMock, getDefaultVideoPortName())
        .WillByDefault(::testing::Return(videoPort));
    ON_CALL(videoOutputPortConfigImplMock, getPort(::testing::_))
        .WillByDefault(::testing::ReturnRef(videoOutputPort));
    ON_CALL(videoOutputPortMock, isDisplayConnected())
        .WillByDefault(::testing::Return(false));
    ON_CALL(videoOutputPortMock, getHDCPProtocol())
        .WillByDefault(::testing::Return(dsHDCP_VERSION_2X));
    ON_CALL(videoOutputPortMock, getHDCPStatus())
        .WillByDefault(::testing::Return(dsHDCP_STATUS_UNPOWERED));
    ON_CALL(videoOutputPortMock, isContentProtected())
        .WillByDefault(::testing::Return(0));
    ON_CALL(videoOutputPortMock, getHDCPReceiverProtocol())
        .WillByDefault(::testing::Return(dsHDCP_VERSION_MAX));
    ON_CALL(videoOutputPortMock, getHDCPCurrentProtocol())
        .WillByDefault(::testing::Return(dsHDCP_VERSION_MAX));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getHDCPStatus"), _T(""), response));
    EXPECT_THAT(response, ::testing::MatchesRegex(_T("\\{"
                                                     "\"HDCPStatus\":"
                                                     "\\{"
                                                     "\"isConnected\":false,"
                                                     "\"isHDCPCompliant\":false,"
                                                     "\"isHDCPEnabled\":false,"
                                                     "\"hdcpReason\":0,"
                                                     "\"supportedHDCPVersion\":\"[1-2]+.[1-4]\","
                                                     "\"receiverHDCPVersion\":\"[1-2]+.[1-4]\","
                                                     "\"currentHDCPVersion\":\"[1-2]+.[1-4]\""
                                                     "\\},"
                                                     "\"success\":true"
                                                     "\\}")));
}

TEST_F(HDCPProfileDsTest, getHDCPStatus_isConnected_true)
{
    NiceMock<VideoOutputPortMock> videoOutputPortMock;
    device::VideoOutputPort videoOutputPort;
    videoOutputPort.impl = &videoOutputPortMock;


    string videoPort(_T("HDMI0"));

    ON_CALL(hostImplMock, getDefaultVideoPortName())
        .WillByDefault(::testing::Return(videoPort));
    ON_CALL(videoOutputPortConfigImplMock, getPort(::testing::_))
        .WillByDefault(::testing::ReturnRef(videoOutputPort));
    ON_CALL(videoOutputPortMock, isDisplayConnected())
        .WillByDefault(::testing::Return(true));
    ON_CALL(videoOutputPortMock, getHDCPProtocol())
        .WillByDefault(::testing::Return(dsHDCP_VERSION_2X));
    ON_CALL(videoOutputPortMock, getHDCPStatus())
        .WillByDefault(::testing::Return(dsHDCP_STATUS_AUTHENTICATED));
    ON_CALL(videoOutputPortMock, isContentProtected())
        .WillByDefault(::testing::Return(true));
    ON_CALL(videoOutputPortMock, getHDCPReceiverProtocol())
        .WillByDefault(::testing::Return(dsHDCP_VERSION_2X));
    ON_CALL(videoOutputPortMock, getHDCPCurrentProtocol())
        .WillByDefault(::testing::Return(dsHDCP_VERSION_2X));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getHDCPStatus"), _T(""), response));
    EXPECT_THAT(response, ::testing::MatchesRegex(_T("\\{"
                                                     "\"HDCPStatus\":"
                                                     "\\{"
                                                     "\"isConnected\":true,"
                                                     "\"isHDCPCompliant\":true,"
                                                     "\"isHDCPEnabled\":true,"
                                                     "\"hdcpReason\":2,"
                                                     "\"supportedHDCPVersion\":\"[1-2]+.[1-4]\","
                                                     "\"receiverHDCPVersion\":\"[1-2]+.[1-4]\","
                                                     "\"currentHDCPVersion\":\"[1-2]+.[1-4]\""
                                                     "\\},"
                                                     "\"success\":true"
                                                     "\\}")));
}

TEST_F(HDCPProfileDsTest, getSettopHDCPSupport_Hdcp_v1x)
{
    NiceMock<VideoOutputPortMock> videoOutputPortMock;
    device::VideoOutputPort videoOutputPort;
    videoOutputPort.impl = &videoOutputPortMock;


    string videoPort(_T("HDMI0"));

    ON_CALL(hostImplMock, getDefaultVideoPortName())
        .WillByDefault(::testing::Return(videoPort));
    ON_CALL(videoOutputPortConfigImplMock, getPort(::testing::_))
        .WillByDefault(::testing::ReturnRef(videoOutputPort));
    ON_CALL(videoOutputPortMock, getHDCPProtocol())
        .WillByDefault(::testing::Return(dsHDCP_VERSION_1X));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getSettopHDCPSupport"), _T(""), response));
    EXPECT_THAT(response, ::testing::MatchesRegex(_T("\\{"
                                                     "\"supportedHDCPVersion\":\"[1-2]+.[1-4]\","
                                                     "\"isHDCPSupported\":true,"
                                                     "\"success\":true"
                                                     "\\}")));
}

TEST_F(HDCPProfileDsTest, getSettopHDCPSupport_Hdcp_v2x)
{
    NiceMock<VideoOutputPortMock> videoOutputPortMock;
    device::VideoOutputPort videoOutputPort;
    videoOutputPort.impl = &videoOutputPortMock;


    string videoPort(_T("HDMI0"));

    ON_CALL(hostImplMock, getDefaultVideoPortName())
        .WillByDefault(::testing::Return(videoPort));
    ON_CALL(videoOutputPortConfigImplMock, getPort(::testing::_))
        .WillByDefault(::testing::ReturnRef(videoOutputPort));
    ON_CALL(videoOutputPortMock, getHDCPProtocol())
        .WillByDefault(::testing::Return(dsHDCP_VERSION_2X));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getSettopHDCPSupport"), _T(""), response));
    EXPECT_THAT(response, ::testing::MatchesRegex(_T("\\{"
                                                     "\"supportedHDCPVersion\":\"[1-2]+.[1-4]\","
                                                     "\"isHDCPSupported\":true,"
                                                     "\"success\":true"
                                                     "\\}")));
}

TEST_F(HDCPProfileEventIarmTest, onDisplayConnectionChanged)
{
    ASSERT_TRUE(dsHdmiEventHandler != nullptr);

    Core::Event onDisplayConnectionChanged(false, true);

    NiceMock<VideoOutputPortMock> videoOutputPortMock;
    device::VideoOutputPort videoOutputPort;
    videoOutputPort.impl = &videoOutputPortMock;


    string videoPort(_T("HDMI0"));

    ON_CALL(hostImplMock, getDefaultVideoPortName())
        .WillByDefault(::testing::Return(videoPort));
    ON_CALL(videoOutputPortConfigImplMock, getPort(::testing::_))
        .WillByDefault(::testing::ReturnRef(videoOutputPort));
    ON_CALL(videoOutputPortMock, isDisplayConnected())
        .WillByDefault(::testing::Return(true));
    ON_CALL(videoOutputPortMock, getHDCPProtocol())
        .WillByDefault(::testing::Return(dsHDCP_VERSION_2X));
    ON_CALL(videoOutputPortMock, getHDCPStatus())
        .WillByDefault(::testing::Return(dsHDCP_STATUS_AUTHENTICATED));
    ON_CALL(videoOutputPortMock, isContentProtected())
        .WillByDefault(::testing::Return(true));
    ON_CALL(videoOutputPortMock, getHDCPReceiverProtocol())
        .WillByDefault(::testing::Return(dsHDCP_VERSION_2X));
    ON_CALL(videoOutputPortMock, getHDCPCurrentProtocol())
        .WillByDefault(::testing::Return(dsHDCP_VERSION_2X));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));

                //EXPECT_EQ(text, string(_T("")));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                "\"jsonrpc\":\"2.0\","
                "\"method\":\"client.events.onDisplayConnectionChanged\","
                "\"params\":"
                "\\{\"HDCPStatus\":"
                "\\{"
                "\"isConnected\":true,"
                "\"isHDCPCompliant\":true,"
                "\"isHDCPEnabled\":true,"
                "\"hdcpReason\":2,"
                "\"supportedHDCPVersion\":\"2.2\","
                "\"receiverHDCPVersion\":\"2.2\","
                "\"currentHDCPVersion\":\"2.2\""
                "\\}"
                "\\}"
                "\\}")));

                onDisplayConnectionChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onDisplayConnectionChanged"), _T("client.events"), message);

    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_hpd.event = dsDISPLAY_EVENT_CONNECTED;
    dsHdmiEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG, &eventData, 0);

    EXPECT_EQ(Core::ERROR_NONE, onDisplayConnectionChanged.Lock());

    handler.Unsubscribe(0, _T("onDisplayConnectionChanged"), _T("client.events"), message);
}

TEST_F(HDCPProfileEventIarmTest, onHdmiOutputHDCPStatusEvent)
{
    ASSERT_TRUE(dsHdmiEventHandler != nullptr);

    Core::Event onDisplayConnectionChanged(false, true);

    NiceMock<VideoOutputPortMock> videoOutputPortMock;
    device::VideoOutputPort videoOutputPort;
    videoOutputPort.impl = &videoOutputPortMock;

    string videoPort(_T("HDMI0"));

    ON_CALL(hostImplMock, getDefaultVideoPortName())
        .WillByDefault(::testing::Return(videoPort));
    ON_CALL(videoOutputPortConfigImplMock, getPort(::testing::_))
        .WillByDefault(::testing::ReturnRef(videoOutputPort));
    ON_CALL(videoOutputPortMock, isDisplayConnected())
        .WillByDefault(::testing::Return(true));
    ON_CALL(videoOutputPortMock, getHDCPProtocol())
        .WillByDefault(::testing::Return(dsHDCP_VERSION_2X));
    ON_CALL(videoOutputPortMock, getHDCPStatus())
        .WillByDefault(::testing::Return(dsHDCP_STATUS_AUTHENTICATED));
    ON_CALL(videoOutputPortMock, isContentProtected())
        .WillByDefault(::testing::Return(true));
    ON_CALL(videoOutputPortMock, getHDCPReceiverProtocol())
        .WillByDefault(::testing::Return(dsHDCP_VERSION_2X));
    ON_CALL(videoOutputPortMock, getHDCPCurrentProtocol())
        .WillByDefault(::testing::Return(dsHDCP_VERSION_2X));
    
      EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));

                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                "\"jsonrpc\":\"2.0\","
                "\"method\":\"client.events.onDisplayConnectionChanged\","
                "\"params\":"
                "\\{\"HDCPStatus\":"
                "\\{"
                "\"isConnected\":true,"
                "\"isHDCPCompliant\":true,"
                "\"isHDCPEnabled\":true,"
                "\"hdcpReason\":2,"
                "\"supportedHDCPVersion\":\"2.2\","
                "\"receiverHDCPVersion\":\"2.2\","
                "\"currentHDCPVersion\":\"2.2\""
                "\\}"
                "\\}"
                "\\}"))); 

                onDisplayConnectionChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

      ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                if (strcmp(methodName, IARM_BUS_PWRMGR_API_GetPowerState) == 0) {
                    auto* param = static_cast<IARM_Bus_PWRMgr_GetPowerState_Param_t*>(arg);
                    param->curState = IARM_BUS_PWRMGR_POWERSTATE_ON; 
                }
                return IARM_RESULT_SUCCESS;
            });

    handler.Subscribe(0, _T("onDisplayConnectionChanged"), _T("client.events"), message);

    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_hdcp.hdcpStatus = dsDISPLAY_HDCPPROTOCOL_CHANGE;
    dsHdmiEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDCP_STATUS, &eventData, 0);

    EXPECT_EQ(Core::ERROR_NONE, onDisplayConnectionChanged.Lock());

    handler.Unsubscribe(0, _T("onDisplayConnectionChanged"), _T("client.events"), message);
}
