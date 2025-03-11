#include <gtest/gtest.h>
#include "HdmiCecSink.h"
#include "FactoriesImplementation.h"
#include "IarmBusMock.h"
#include "ServiceMock.h"
#include "devicesettings.h"
#include "HdmiCec.h"
#include "HdmiCecMock.h"
#include "WrapsMock.h"
#include "RfcApiMock.h"

using namespace WPEFramework;
using ::testing::NiceMock;

class HdmiCecSinkTest : public ::testing::Test {
protected:
    IarmBusImplMock         *p_iarmBusImplMock = nullptr ;
    ConnectionImplMock      *p_connectionImplMock = nullptr ;
    MessageEncoderMock      *p_messageEncoderMock = nullptr ;
    LibCCECImplMock         *p_libCCECImplMock = nullptr ;
    RfcApiImplMock   *p_rfcApiImplMock = nullptr ;
    WrapsImplMock  *p_wrapsImplMock   = nullptr ;
    Core::ProxyType<Plugin::HdmiCecSink> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    NiceMock<RfcApiImplMock> rfcApiImplMock;
    NiceMock<WrapsImplMock> wrapsImplMock;
    IARM_EventHandler_t pwrMgrModeChangeEventHandler;
    IARM_EventHandler_t dsHdmiEventHandler;
    IARM_EventHandler_t dsHdmiCecSinkEventHandler;
    string response;

    HdmiCecSinkTest()
        : plugin(Core::ProxyType<Plugin::HdmiCecSink>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
        p_iarmBusImplMock  = new NiceMock <IarmBusImplMock>;
        IarmBus::setImpl(p_iarmBusImplMock);

        p_libCCECImplMock  = new testing::NiceMock <LibCCECImplMock>;
        LibCCEC::setImpl(p_libCCECImplMock);

        p_messageEncoderMock  = new testing::NiceMock <MessageEncoderMock>;
        MessageEncoder::setImpl(p_messageEncoderMock);

        p_connectionImplMock  = new testing::NiceMock <ConnectionImplMock>;
        Connection::setImpl(p_connectionImplMock);

        p_rfcApiImplMock  = new testing::NiceMock <RfcApiImplMock>;
        RfcApi::setImpl(p_rfcApiImplMock);

        p_wrapsImplMock  = new testing::NiceMock <WrapsImplMock>;
        Wraps::setImpl(p_wrapsImplMock); /*Set up mock for fopen;
                                                      to use the mock implementation/the default behavior of the fopen function from Wraps class.*/

        ON_CALL(*p_connectionImplMock, poll(::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const LogicalAddress &from, const Throw_e &doThrow) {
                throw CECNoAckException();
                }));

        EXPECT_CALL(*p_libCCECImplMock, getPhysicalAddress(::testing::_))
            .WillRepeatedly(::testing::Invoke(
                [&](uint32_t *physAddress) {
                    *physAddress = (uint32_t)0x12345678;
                }));

        ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const DataBlock&>(::testing::_)))
            .WillByDefault(::testing::ReturnRef(CECFrame::getInstance()));
        ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
           .WillByDefault(::testing::ReturnRef(CECFrame::getInstance()));

        ON_CALL(*p_iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    if ((string(IARM_BUS_PWRMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_PWRMGR_EVENT_MODECHANGED)) {
                        EXPECT_TRUE(handler != nullptr);
                        pwrMgrModeChangeEventHandler = handler;
                    }
                    if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG)) {
                        EXPECT_TRUE(handler != nullptr);
                        dsHdmiEventHandler = handler;
                    }
                    if ((string(IARM_BUS_CECMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED)) {
                        EXPECT_TRUE(handler != nullptr);
                        dsHdmiCecSinkEventHandler = handler;
                    }
                    return IARM_RESULT_SUCCESS;
                }));

        ON_CALL(*p_connectionImplMock, open())
            .WillByDefault(::testing::Return());

        EXPECT_EQ(string(""), plugin->Initialize(nullptr));
    }

    virtual ~HdmiCecSinkTest() override
    {
        plugin->Deinitialize(nullptr);
        IarmBus::setImpl(nullptr);
        if (p_iarmBusImplMock != nullptr)
        {
            delete p_iarmBusImplMock;
            p_iarmBusImplMock = nullptr;
        }
        LibCCEC::setImpl(nullptr);
        if (p_libCCECImplMock != nullptr)
        {
            delete p_libCCECImplMock;
            p_libCCECImplMock = nullptr;
        }
        Connection::setImpl(nullptr);
        if (p_connectionImplMock != nullptr)
        {
            delete p_connectionImplMock;
            p_connectionImplMock = nullptr;
        }
        MessageEncoder::setImpl(nullptr);
        if (p_messageEncoderMock != nullptr)
        {
            delete p_messageEncoderMock;
            p_messageEncoderMock = nullptr;
        }

        RfcApi::setImpl(nullptr);
        if (p_rfcApiImplMock != nullptr)
        {
            delete p_rfcApiImplMock;
            p_rfcApiImplMock = nullptr;
        }

        Wraps::setImpl(nullptr);
        if (p_wrapsImplMock != nullptr)
        {
            delete p_wrapsImplMock;
            p_wrapsImplMock = nullptr;
        }

    }
};

class HdmiCecSinkDsTest : public HdmiCecSinkTest {
protected:
    string response;

    HdmiCecSinkDsTest(): HdmiCecSinkTest()
    {
        ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
            .WillByDefault(
                [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                    if (strcmp(methodName, IARM_BUS_PWRMGR_API_GetPowerState) == 0) {
                        auto* param = static_cast<IARM_Bus_PWRMgr_GetPowerState_Param_t*>(arg);
                        param->curState  = IARM_BUS_PWRMGR_POWERSTATE_ON;
                    }
                    if (strcmp(methodName, IARM_BUS_DSMGR_API_dsHdmiInGetNumberOfInputs) == 0) {
                        auto* param = static_cast<dsHdmiInGetNumberOfInputsParam_t*>(arg);
                        param->result = dsERR_NONE;
                        param->numHdmiInputs = 3;
                    }
                    if (strcmp(methodName, IARM_BUS_DSMGR_API_dsHdmiInGetStatus) == 0) {
                        auto* param = static_cast<dsHdmiInGetStatusParam_t*>(arg);
                        param->result = dsERR_NONE;
                        param->status.isPortConnected[1] = 1;
                    }
                    if (strcmp(methodName, IARM_BUS_DSMGR_API_dsGetHDMIARCPortId) == 0) {
                        auto* param = static_cast<dsGetHDMIARCPortIdParam_t*>(arg);
                        param->portId = 1;
                    }
                    return IARM_RESULT_SUCCESS;
                });

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));
    }
    virtual ~HdmiCecSinkDsTest() override {
    }
};

class HdmiCecSinkInitializedEventTest : public HdmiCecSinkDsTest {
protected:
    NiceMock<ServiceMock> service;
    Core::JSONRPC::Message message;
    NiceMock<FactoriesImplementation> factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    HdmiCecSinkInitializedEventTest(): HdmiCecSinkDsTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);
        dispatcher = static_cast<PluginHost::IDispatcher*>(
        plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }
    virtual ~HdmiCecSinkInitializedEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();
        PluginHost::IFactories::Assign(nullptr);
    }
};

class HdmiCecSinkInitializedEventDsTest : public HdmiCecSinkInitializedEventTest {
protected:
    HdmiCecSinkInitializedEventDsTest(): HdmiCecSinkInitializedEventTest()
    {
    }
    virtual ~HdmiCecSinkInitializedEventDsTest() override
    {
    }
};

TEST_F(HdmiCecSinkTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setOSDName")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setVendorId")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getVendorId")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setActivePath")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setRoutingChange")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getDeviceList")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getActiveSource")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setActiveSource")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getActiveRoute")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setMenuLanguage")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("requestActiveSource")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setupARCRouting")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("requestShortAudioDescriptor")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("sendStandbyMessage")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("sendAudioDevicePowerOnMessage")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("sendKeyPressEvent")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("sendGetAudioStatusMessage")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getAudioDeviceConnectedStatus")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("requestAudioDevicePowerStatus")));
}

TEST_F(HdmiCecSinkDsTest, setOSDNameParamMissing)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setOSDName"), _T("{}"), response));
    EXPECT_EQ(response,  string(""));
}

TEST_F(HdmiCecSinkDsTest, getOSDName)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setOSDName"), _T("{\"name\":\"CECTEST\"}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getOSDName"), _T("{}"), response));
    EXPECT_EQ(response,  string("{\"name\":\"CECTEST\",\"success\":true}"));
}

TEST_F(HdmiCecSinkDsTest, setVendorIdParamMissing)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setVendorId"), _T("{}"), response));
    EXPECT_EQ(response,  string(""));
}

TEST_F(HdmiCecSinkDsTest, getVendorId)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setVendorId"), _T("{\"vendorid\":\"0x0019FF\"}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getVendorId"), _T("{}"), response));
    EXPECT_EQ(response,  string("{\"vendorid\":\"019ff\",\"success\":true}"));
}

TEST_F(HdmiCecSinkDsTest, setActivePathMissingParam)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setActivePath"), _T("{}"), response));
    EXPECT_EQ(response,  string(""));
}

TEST_F(HdmiCecSinkDsTest, setActivePath)
{
    EXPECT_CALL(*p_connectionImplMock, sendTo(::testing::_, ::testing::_, ::testing::_))
        .WillRepeatedly(::testing::Invoke(
            [&](const LogicalAddress &to, const CECFrame &frame, int timeout) {
               EXPECT_EQ(to.toInt(), LogicalAddress::BROADCAST);
               EXPECT_GT(timeout, 0);
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setActivePath"), _T("{\"activePath\":\"2.0.0.0\"}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(HdmiCecSinkDsTest, setRoutingChangeInvalidParam)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setRoutingChange"), _T("{\"oldPort\":\"HDMI0\"}"), response));
    EXPECT_EQ(response,  string(""));
}

TEST_F(HdmiCecSinkDsTest, setRoutingChange)
{
    std::this_thread::sleep_for(std::chrono::seconds(30));

    EXPECT_CALL(*p_connectionImplMock, sendTo(::testing::_, ::testing::_, ::testing::_))
        .WillRepeatedly(::testing::Invoke(
            [&](const LogicalAddress &to, const CECFrame &frame, int timeout) {
                EXPECT_EQ(to.toInt(), LogicalAddress::BROADCAST);
                EXPECT_GT(timeout, 0);
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setRoutingChange"), _T("{\"oldPort\":\"HDMI0\",\"newPort\":\"TV\"}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(HdmiCecSinkDsTest, setMenuLanguageInvalidParam)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setMenuLanguage"), _T("{\"language\":""}"), response));
    EXPECT_EQ(response,  string(""));
}

TEST_F(HdmiCecSinkDsTest, setMenuLanguage)
{
    EXPECT_CALL(*p_connectionImplMock, sendTo(::testing::_, ::testing::_, ::testing::_))
        .WillRepeatedly(::testing::Invoke(
            [&](const LogicalAddress &to, const CECFrame &frame, int timeout) {
                EXPECT_LE(to.toInt(), LogicalAddress::BROADCAST);
                EXPECT_GT(timeout, 0);
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setMenuLanguage"), _T("{\"language\":\"english\"}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(HdmiCecSinkDsTest, setupARCRoutingInvalidParam)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setupARCRouting"), _T("{}"), response));
    EXPECT_EQ(response,  string(""));
}

TEST_F(HdmiCecSinkDsTest, setupARCRouting)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setupARCRouting"), _T("{\"enabled\":\"true\"}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(HdmiCecSinkDsTest, sendKeyPressEventMissingParam)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": }"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(HdmiCecSinkDsTest, sendKeyPressEvent)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 65}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(HdmiCecSinkInitializedEventDsTest, onHdmiOutputHDCPStatusEvent)
{
    ASSERT_TRUE(dsHdmiEventHandler != nullptr);

    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_connect.port =dsHDMI_IN_PORT_1;
    eventData.data.hdmi_in_connect.isPortConnected = true;

    handler.Subscribe(0, _T("onDevicesChanged"), _T("client.events.onDevicesChanged"), message);
    dsHdmiEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG, &eventData , 0);
    handler.Unsubscribe(0, _T("onDevicesChanged"), _T("client.events.onDevicesChanged"), message);
}

TEST_F(HdmiCecSinkInitializedEventDsTest, powerModeChange)
{
    ASSERT_TRUE(pwrMgrModeChangeEventHandler != nullptr);

    IARM_Bus_PWRMgr_EventData_t eventData;
    eventData.data.state.newState =IARM_BUS_PWRMGR_POWERSTATE_ON;
    eventData.data.state.curState =IARM_BUS_PWRMGR_POWERSTATE_STANDBY;

    pwrMgrModeChangeEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, &eventData , 0);
}

TEST_F(HdmiCecSinkInitializedEventDsTest, HdmiCecEnableStatus)
{
    ASSERT_TRUE(dsHdmiCecSinkEventHandler != nullptr);
    IARM_Bus_CECMgr_Status_Updated_Param_t eventData;
    dsHdmiCecSinkEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED, &eventData , 0);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getEnabled"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"enabled\":true,\"success\":true}"));
}

TEST_F(HdmiCecSinkTest, DISABLED_getCecVersion)
{
    /*EXPECT_CALL(rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
                EXPECT_EQ(string(pcCallerID), string("HdmiCecSink"));
                EXPECT_EQ(string(pcParameterName), string("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.HdmiCecSink.CECVersion"));
                strncpy(pstParamData->value, "1.4", sizeof(pstParamData->value));
                return WDMP_SUCCESS;
            }));*/
    
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getCecVersion"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"CECVersion\":\"1.4\",\"success\":true}"));
}
