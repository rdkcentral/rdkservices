#include <gtest/gtest.h>

#include "HdmiCecSink.h"

#include "FactoriesImplementation.h"


#include "IarmBusMock.h"
#include "ServiceMock.h"
#include "devicesettings.h"
#include "LibCCECMock.h"
#include "ConnectionMock.h"
#include "LogicalAddressMock.h"
#include "ActiveSourceMock.h"
#include "DeviceTypeMock.h"
#include "MessageEncoderMock.h"

///using namespace std;
using namespace WPEFramework;
using ::testing::NiceMock;

class HdmiCecSinkTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::HdmiCecSink> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;
    NiceMock<LibCCECImplMock> libCCECImplMock;
    NiceMock<IarmBusImplMock> iarmBusImplMock;
    NiceMock<MessageEncoderMock> messageEncoderMock;
    NiceMock<ConnectionImplMock> connectionImplMock;

    HdmiCecSinkTest()
        : plugin(Core::ProxyType<Plugin::HdmiCecSink>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;
        LibCCEC::getInstance().impl = &libCCECImplMock;
        Connection::getInstance().impl = &connectionImplMock;
        MessageEncoder::getInstance().impl = &messageEncoderMock;

        ON_CALL(messageEncoderMock, encode(::testing::_))
            .WillByDefault(::testing::ReturnRef(CECFrame::getInstance()));

		ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                if (strcmp(methodName, IARM_BUS_PWRMGR_API_GetPowerState) == 0) {
                    auto* param = static_cast<IARM_Bus_PWRMgr_GetPowerState_Param_t*>(arg);
                    param->curState  = IARM_BUS_PWRMGR_POWERSTATE_ON;
                }
                if (strcmp(methodName, IARM_BUS_DSMGR_API_dsHdmiInGetNumberOfInputs) == 0) {
                    auto* param = static_cast<dsHdmiInGetNumberOfInputsParam_t*>(arg);
                    param->result = dsERR_NONE;
                    param->numHdmiInputs = 1;
                }
                if (strcmp(methodName, IARM_BUS_CECMGR_API_isAvailable) == 0) {
                    //auto* param = static_cast<int*>(arg);
                    //param->curState = IARM_BUS_PWRMGR_POWERSTATE_ON;
                }

                return IARM_RESULT_SUCCESS;
            });
        EXPECT_EQ(string(""), plugin->Initialize(nullptr));
    }
    
	virtual ~HdmiCecSinkTest() override
    {
			plugin->Deinitialize(nullptr);
			IarmBus::getInstance().impl = nullptr;
			LibCCEC::getInstance().impl = nullptr;
			Connection::getInstance().impl = nullptr;
            MessageEncoder::getInstance().impl = nullptr;
    }
   
};
class HdmiCecSinkDsTest : public HdmiCecSinkTest {
protected:
    NiceMock<LibCCECImplMock> libCCECImplMock;
    
    HdmiCecSinkDsTest()
        : HdmiCecSinkTest()
    {
    }
    virtual ~HdmiCecSinkDsTest() override
    {
    }
};

class HdmiCecSinkInitializedTest : public HdmiCecSinkTest {
protected:
    IARM_EventHandler_t dsHdmiCecSinkEventHandler;
    IARM_EventHandler_t dsHdmiEventHandler;
 
    HdmiCecSinkInitializedTest()
        : HdmiCecSinkTest()
    {

        ON_CALL(iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                   if ((string(IARM_BUS_CECMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED)) {
                        EXPECT_TRUE(handler != nullptr);
                        dsHdmiCecSinkEventHandler = handler;
                    }
		   if ((string(IARM_BUS_CECMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_CECMGR_EVENT_STATUS_UPDATED)) {
                        EXPECT_TRUE(handler != nullptr);
                        dsHdmiCecSinkEventHandler = handler;
                    }
                   if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG)) {
                        EXPECT_TRUE(handler != nullptr);
                        dsHdmiEventHandler = handler;
                    }

                    return IARM_RESULT_SUCCESS;
                }));
    }
    virtual ~HdmiCecSinkInitializedTest() override
    {
		
    }
};
class HdmiCecSinkInitializedEventTest : public HdmiCecSinkInitializedTest {
protected:
    ServiceMock service;
    Core::JSONRPC::Message message;
    FactoriesImplementation factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    HdmiCecSinkInitializedEventTest()
        : HdmiCecSinkInitializedTest()
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
	NiceMock<LibCCECImplMock> libCCECImplMock;
    NiceMock<ConnectionImplMock> connectionImplMock;
    NiceMock<DeviceTypeMock> deviceTypeImplMock;
    NiceMock<LogicalAddressImplMock> logicalAddressImplMock;

    HdmiCecSinkInitializedEventDsTest()
        : HdmiCecSinkInitializedEventTest()
    {
	    DeviceType::getInstance().impl = &deviceTypeImplMock;

    }

    virtual ~HdmiCecSinkInitializedEventDsTest() override
    {
        DeviceType::getInstance().impl = nullptr;
    }
};


TEST_F(HdmiCecSinkTest, RegisteredMethods)
{

    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setEnabledFalse")));
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

TEST_F(HdmiCecSinkInitializedEventDsTest, setEnabled)
{
    //setting HdmiCec to enabled.
    ON_CALL(libCCECImplMock, getLogicalAddress(::testing::_))
        .WillByDefault(::testing::Return(1));
	ON_CALL(deviceTypeImplMock, toString())
        .WillByDefault(::testing::Return("New"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

}

TEST_F(HdmiCecSinkInitializedEventDsTest, getEnableTrue)
{
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getEnabled"), _T("{}"), response));
	EXPECT_EQ(response, string("{\"enabled\":true,\"success\":true}"));
}

TEST_F(HdmiCecSinkInitializedEventDsTest, setEnableFalse)
{
    //setting HdmiCec to enabled.
    ON_CALL(libCCECImplMock, getLogicalAddress(::testing::_))
        .WillByDefault(::testing::Return(1));
    ON_CALL(deviceTypeImplMock, toString())
        .WillByDefault(::testing::Return("New"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": false}"), response));
	EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(HdmiCecSinkInitializedEventDsTest, getEnabledFalse)
{

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getEnabled"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"enabled\":false,\"success\":true}"));
}

TEST_F(HdmiCecSinkDsTest, setOSDName)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setOSDName"), _T("{\"name\":\"CECTEST\"}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));

}

TEST_F(HdmiCecSinkDsTest, getOSDName)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getOSDName"), _T("{}"), response));
    EXPECT_EQ(response,  string("{\"name\":\"CECTEST\",\"success\":true}"));

}

TEST_F(HdmiCecSinkDsTest, setVendorId)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setVendorId"), _T("{\"vendorid\":\"0x0019FB\"}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));

}

TEST_F(HdmiCecSinkDsTest, getVendorId)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getVendorId"), _T("{}"), response));
    EXPECT_EQ(response,  string("{\"vendorid\":\"1\",\"success\":true}"));

}

TEST_F(HdmiCecSinkDsTest, setActivePath)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setActivePath"), _T("{\"activePath\":\"2.0.0.0\"}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));

}

TEST_F(HdmiCecSinkDsTest, setRoutingChange)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setRoutingChange"), _T("{\"oldPort\":\"TV\",\"newPort\":\"HDMI1\"}"), response));
    EXPECT_EQ(response,  string(""));

}

TEST_F(HdmiCecSinkDsTest, getDeviceList)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceList"), _T(""), response));
    EXPECT_EQ(response,  string("{\"numberofdevices\":0,\"deviceList\":[],\"success\":true}"));

}

TEST_F(HdmiCecSinkDsTest, getActiveSource)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getActiveSource"), _T(""), response));
    EXPECT_EQ(response,  string("{\"available\":false,\"success\":true}"));
}

TEST_F(HdmiCecSinkDsTest, setActiveSource)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setActiveSource"), _T(""), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(HdmiCecSinkDsTest, setMenuLanguage)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setMenuLanguage"), _T("{\"language\":\"english\"}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(HdmiCecSinkDsTest, requestActiveSource)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("requestActiveSource"), _T(""), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(HdmiCecSinkDsTest, requestShortAudioDescriptor)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("requestShortAudioDescriptor"), _T(""), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(HdmiCecSinkDsTest, setupARCRouting)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setupARCRouting"), _T(""), response));
    EXPECT_EQ(response,  string(""));
}

TEST_F(HdmiCecSinkDsTest, sendStandbyMessage)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendStandbyMessage"), _T(""), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}
TEST_F(HdmiCecSinkDsTest, sendAudioDevicePowerOnMessage)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendAudioDevicePowerOnMessage"), _T(""), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}
TEST_F(HdmiCecSinkDsTest, sendKeyPressEvent)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\":\"0x0019FB\", \"keyCode\":\"0x0001\"}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(HdmiCecSinkTest, sendGetAudioStatusMessage)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendGetAudioStatusMessage"), _T(""), response));
    EXPECT_EQ(response,  string(""));
}

TEST_F(HdmiCecSinkDsTest, getAudioDeviceConnectedStatus)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getAudioDeviceConnectedStatus"), _T(""), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(HdmiCecSinkDsTest, requestAudioDevicePowerStatus)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("requestAudioDevicePowerStatus"), _T(""), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}
