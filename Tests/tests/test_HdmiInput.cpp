#include <gtest/gtest.h>

#include "HdmiInput.h"

#include "FactoriesImplementation.h"

#include "HdmiInputMock.h"
#include "IarmBusMock.h"
#include "ServiceMock.h"

#include "dsMgr.h"

using namespace WPEFramework;

using ::testing::NiceMock;

class HdmiInputTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::HdmiInput> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Handler& handlerV2;
    Core::JSONRPC::Connection connection;
    string response;

    HdmiInputTest()
        : plugin(Core::ProxyType<Plugin::HdmiInput>::Create())
        , handler(*(plugin))
        , handlerV2(*(plugin->GetHandler(2)))
        , connection(1, 0)
    {
    }
    virtual ~HdmiInputTest() = default;
};

class HdmiInputDsTest : public HdmiInputTest {
protected:
    NiceMock<HdmiInputImplMock> hdmiInputImplMock;

    HdmiInputDsTest()
        : HdmiInputTest()
    {
        device::HdmiInput::getInstance().impl = &hdmiInputImplMock;
    }
    virtual ~HdmiInputDsTest() override
    {
        device::HdmiInput::getInstance().impl = nullptr;
    }
};

class HdmiInputInitializedTest : public HdmiInputTest {
protected:
    NiceMock<IarmBusImplMock> iarmBusImplMock;
    IARM_EventHandler_t dsHdmiEventHandler;
    IARM_EventHandler_t dsHdmiStatusEventHandler;
    IARM_EventHandler_t dsHdmiSignalStatusEventHandler;
    IARM_EventHandler_t dsHdmiVideoModeEventHandler;
    IARM_EventHandler_t dsHdmiGameFeatureStatusEventHandler;

   // NiceMock<ServiceMock> service;
    ServiceMock service;
    Core::JSONRPC::Message message;
    FactoriesImplementation factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;
  
    HdmiInputInitializedTest()
        : HdmiInputTest()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;

        ON_CALL(iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG)) {
                        EXPECT_TRUE(handler != nullptr);
                        dsHdmiEventHandler = handler;
                    }
                   if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS)) {
                        EXPECT_TRUE(handler != nullptr);
                        dsHdmiStatusEventHandler = handler;
                   }
                    if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_HDMI_IN_SIGNAL_STATUS)) {
                        EXPECT_TRUE(handler != nullptr);
                        dsHdmiSignalStatusEventHandler = handler;
                    }
                    if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE)) {
                        EXPECT_TRUE(handler != nullptr);
                        dsHdmiVideoModeEventHandler = handler;
                    }
                    if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_HDMI_IN_ALLM_STATUS)) {
                        EXPECT_TRUE(handler != nullptr);
                        dsHdmiGameFeatureStatusEventHandler = handler;
                    }
                    return IARM_RESULT_SUCCESS;
                }));
#if 0
	PluginHost::IFactories::Assign(&factoriesImplementation);

#endif
        //dispatcher = static_cast<PluginHost::IDispatcher*>(
            //plugin->QueryInterface(PluginHost::IDispatcher::ID));
       //dispatcher->Activate(&service);
        EXPECT_EQ(string(""), plugin->Initialize(&service));
    }
    virtual ~HdmiInputInitializedTest() override
    {
        plugin->Deinitialize(&service);
        dispatcher->Deactivate();
        dispatcher->Release();

        PluginHost::IFactories::Assign(nullptr);

        IarmBus::getInstance().impl = nullptr;
    }
};


class HdmiInputInitializedEventTest : public HdmiInputInitializedTest {
protected:
   // NiceMock<ServiceMock> service;
    Core::JSONRPC::Message message;
    NiceMock<FactoriesImplementation> factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    HdmiInputInitializedEventTest()
        : HdmiInputInitializedTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
//       EXPECT_EQ(string(""), plugin->Initialize(&service));
    }

    virtual ~HdmiInputInitializedEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();

        PluginHost::IFactories::Assign(nullptr);
    }
};

class HdmiInputInitializedEventDsTest : public HdmiInputInitializedEventTest {
protected:
    NiceMock<HdmiInputImplMock> hdmiInputImplMock;

    HdmiInputInitializedEventDsTest()
        : HdmiInputInitializedEventTest()
    {
        device::HdmiInput::getInstance().impl = &hdmiInputImplMock;
    }

    virtual ~HdmiInputInitializedEventDsTest() override
    {
        device::HdmiInput::getInstance().impl = nullptr;
    }
};



TEST_F(HdmiInputTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getHDMIInputDevices")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("writeEDID")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("readEDID")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("getRawHDMISPD")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("getHDMISPD")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("setEdidVersion")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("getEdidVersion")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("startHdmiInput")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("stopHdmiInput")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setVideoRectangle")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getSupportedGameFeatures")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getHdmiGameFeatureStatus")));

    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("getHDMIInputDevices")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("writeEDID")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("readEDID")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("startHdmiInput")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("stopHdmiInput")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("setVideoRectangle")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("getSupportedGameFeatures")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("getHdmiGameFeatureStatus")));
}

TEST_F(HdmiInputDsTest, getHDMIInputDevices)
{

    ON_CALL(hdmiInputImplMock, getNumberOfInputs())
        .WillByDefault(::testing::Return(1));
    ON_CALL(hdmiInputImplMock, isPortConnected(::testing::_))
        .WillByDefault(::testing::Return(true));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getHDMIInputDevices"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"devices\":[{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"connected\":\"true\"}],\"success\":true}"));
}


TEST_F(HdmiInputDsTest, writeEDIDEmpty)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("writeEDID"), _T("{\"message\": \"message\"}"), response));
    EXPECT_EQ(response, string(""));
}


TEST_F(HdmiInputDsTest, writeEDID)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("writeEDID"), _T("{\"deviceId\": 0, \"message\": \"message\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(HdmiInputDsTest, writeEDIDInvalid)
{
      ON_CALL(hdmiInputImplMock, getEDIDBytesInfo(::testing::_,::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](int iport, std::vector<uint8_t> &edidVec2) {
                edidVec2 = std::vector<uint8_t>({ 't', 'e', 's', 't' });
            }));        
   EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("readEDID"), _T("{\"deviceId\": \"b\"}"), response));
   EXPECT_EQ(response, string(""));
}

TEST_F(HdmiInputDsTest, readEDID)
{
    ON_CALL(hdmiInputImplMock, getEDIDBytesInfo(::testing::_,::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](int iport, std::vector<uint8_t> &edidVec2) {
                edidVec2 = std::vector<uint8_t>({ 't', 'e', 's', 't' });
            }));        
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("readEDID"), _T("{\"deviceId\": 0}"), response));
    EXPECT_EQ(response, string("{\"EDID\":\"dGVzdA==\",\"success\":true}"));
}

TEST_F(HdmiInputDsTest, getRawHDMISPD)
{
    ON_CALL(hdmiInputImplMock, getHDMISPDInfo(::testing::_,::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](int iport, std::vector<uint8_t>& edidVec2) {
                edidVec2 = { 't', 'e', 's', 't' };
            }));   
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("getRawHDMISPD"), _T("{\"portId\":0}"), response));
    EXPECT_EQ(response, string("{\"HDMISPD\":\"dGVzdA\",\"success\":true}"));
}
TEST_F(HdmiInputDsTest, getRawHDMISPDInvalid)
{
    ON_CALL(hdmiInputImplMock, getHDMISPDInfo(::testing::_,::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](int iport, std::vector<uint8_t>& edidVec2) {
                edidVec2 = { 't', 'e', 's', 't' };
            }));   
    EXPECT_EQ(Core::ERROR_GENERAL, handlerV2.Invoke(connection, _T("getRawHDMISPD"), _T("{\"portId\":\"b\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(HdmiInputDsTest, getHDMISPD)
{
    ON_CALL(hdmiInputImplMock, getHDMISPDInfo(::testing::_,::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](int iport, std::vector<uint8_t>& edidVec2) {
                edidVec2 = {'0','1','2','n', 'p', '1','2','3','4','5','6','7',0,'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',0,'q','r'};
            })); 
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("getHDMISPD"), _T("{\"portId\":0}"), response));
    EXPECT_EQ(response, string("{\"HDMISPD\":\"Packet Type:30,Version:49,Length:50,vendor name:1234567,product des:abcdefghijklmno,source info:71\",\"success\":true}"));
}
TEST_F(HdmiInputDsTest, getHDMISPDInvalid)
{
    ON_CALL(hdmiInputImplMock, getHDMISPDInfo(::testing::_,::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](int iport, std::vector<uint8_t>& edidVec2) {
                edidVec2 = {'0','1','2','n', 'p', '0'};
            })); 
    EXPECT_EQ(Core::ERROR_GENERAL, handlerV2.Invoke(connection, _T("getHDMISPD"), _T("{\"portId\":\"b\"}"), response));
    EXPECT_EQ(response, string(""));
}


TEST_F(HdmiInputDsTest, setEdidVersionInvalid)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handlerV2.Invoke(connection, _T("setEdidVersion"), _T("{\"portId\": \"b\", \"edidVersion\":\"HDMI1.4\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(HdmiInputDsTest, setEdidVersion14)
{
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("setEdidVersion"), _T("{\"portId\": \"0\", \"edidVersion\":\"HDMI1.4\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(HdmiInputDsTest, setEdidVersion20)
{
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("setEdidVersion"), _T("{\"portId\": \"0\", \"edidVersion\":\"HDMI2.0\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}
TEST_F(HdmiInputDsTest, setEdidVersionEmpty)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handlerV2.Invoke(connection, _T("setEdidVersion"), _T("{\"portId\": \"0\", \"edidVersion\":\"\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(HdmiInputDsTest, getEdidVersionInvalid)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handlerV2.Invoke(connection, _T("getEdidVersion"), _T("{\"portId\": \"b\", \"edidVersion\":\"HDMI1.4\"}"), response));
    EXPECT_EQ(response, string(""));
}
TEST_F(HdmiInputDsTest, getEdidVersionVer14)
{
    ON_CALL(hdmiInputImplMock, getEdidVersion(::testing::_,::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](int iPort, int *edidVersion) {
                *edidVersion = 0;
            })); 
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("getEdidVersion"), _T("{\"portId\": \"0\"}"), response));
    EXPECT_EQ(response, string("{\"edidVersion\":\"HDMI1.4\",\"success\":true}"));
}
TEST_F(HdmiInputDsTest, getEdidVersionVer20)
{
    ON_CALL(hdmiInputImplMock, getEdidVersion(::testing::_,::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](int iPort, int *edidVersion) {
                *edidVersion = 1;
            })); 
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("getEdidVersion"), _T("{\"portId\": \"0\"}"), response));
    EXPECT_EQ(response, string("{\"edidVersion\":\"HDMI2.0\",\"success\":true}"));
}

TEST_F(HdmiInputDsTest, startHdmiInputInvalid)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("startHdmiInput"), _T("{\"portId\": \"b\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(HdmiInputDsTest, startHdmiInput)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startHdmiInput"), _T("{\"portId\": \"0\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}")); 
}


TEST_F(HdmiInputDsTest, stopHdmiInput)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("stopHdmiInput"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":true}")); 
}

TEST_F(HdmiInputDsTest, setVideoRectangleInvalid)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setVideoRectangle"), _T("{\"x\": \"b\",\"y\": 0,\"w\": 1920,\"h\": 1080}"), response));
    EXPECT_EQ(response, string("")); 
}

TEST_F(HdmiInputDsTest, setVideoRectangle)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setVideoRectangle"), _T("{\"x\": 0,\"y\": 0,\"w\": 1920,\"h\": 1080}"), response));
    EXPECT_EQ(response, string("{\"success\":true}")); 
}


TEST_F(HdmiInputDsTest, getSupportedGameFeatures)
{
    ON_CALL(hdmiInputImplMock, getSupportedGameFeatures(::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](std::vector<std::string> &supportedFeatures) {
                supportedFeatures = {"ALLM"};
            })); 
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getSupportedGameFeatures"), _T("{\"supportedGameFeatures\": \"ALLM\"}"), response));
    EXPECT_EQ(response, string("{\"supportedGameFeatures\":[\"ALLM\"],\"success\":true}")); 
}


TEST_F(HdmiInputDsTest, getHdmiGameFeatureStatusInvalidPort)
{
    ON_CALL(hdmiInputImplMock, getHdmiALLMStatus(::testing::_,::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](int iport, bool *allm) {
                *allm = true;
            }));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getHdmiGameFeatureStatus"), _T("{\"portId\": \"b\",\"gameFeature\": \"ALLM\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(HdmiInputDsTest, getHdmiGameFeatureStatus)
{
    ON_CALL(hdmiInputImplMock, getHdmiALLMStatus(::testing::_,::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](int iport, bool *allm) {
                *allm = true;
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getHdmiGameFeatureStatus"), _T("{\"portId\": \"0\",\"gameFeature\": \"ALLM\"}"), response));
    EXPECT_EQ(response, string("{\"mode\":true,\"success\":true}")); 
}
TEST_F(HdmiInputDsTest, getHdmiGameFeatureStatusInvalidFeature)
{
    ON_CALL(hdmiInputImplMock, getHdmiALLMStatus(::testing::_,::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](int iport, bool *allm) {
                *allm = true;
            }));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getHdmiGameFeatureStatus"), _T("{\"portId\": \"0\",\"gameFeature\": \"Invalid\"}"), response));
    EXPECT_EQ(response, string("")); 
}

TEST_F(HdmiInputInitializedEventDsTest, onDevicesChanged)
{
   ASSERT_TRUE(dsHdmiEventHandler != nullptr);
    ON_CALL(hdmiInputImplMock, getNumberOfInputs())
        .WillByDefault(::testing::Return(1));
    ON_CALL(hdmiInputImplMock, isPortConnected(::testing::_))
        .WillByDefault(::testing::Return(true));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.onDevicesChanged.onDevicesChanged\",\"params\":{\"devices\":[{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"connected\":\"true\"}]}}")));

                return Core::ERROR_NONE;
            }));


    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_connect.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_connect.isPortConnected = true;	
    handler.Subscribe(0, _T("onDevicesChanged"), _T("client.events.onDevicesChanged"), message);

    dsHdmiEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG, &eventData , 0);

    handler.Unsubscribe(0, _T("onDevicesChanged"), _T("client.events.onDevicesChanged"), message); 
}

TEST_F(HdmiInputInitializedEventDsTest, onInputStatusChangeOn)
{
   ASSERT_TRUE(dsHdmiStatusEventHandler != nullptr);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.onInputStatusChanged.onInputStatusChanged\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"status\":\"started\"}}")));
                return Core::ERROR_NONE;
            }));
    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_status.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_status.isPresented = true;	
    handler.Subscribe(0, _T("onInputStatusChanged"), _T("client.events.onInputStatusChanged"), message);
    dsHdmiStatusEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS, &eventData , 0);
    handler.Unsubscribe(0, _T("onInputStatusChanged"), _T("client.events.onInputStatusChanged"), message); 
}
TEST_F(HdmiInputInitializedEventDsTest, onInputStatusChangeOff)
{
   ASSERT_TRUE(dsHdmiStatusEventHandler != nullptr);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.onInputStatusChanged.onInputStatusChanged\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"status\":\"stopped\"}}")));
                return Core::ERROR_NONE;
            }));
    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_status.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_status.isPresented = false;	
    handler.Subscribe(0, _T("onInputStatusChanged"), _T("client.events.onInputStatusChanged"), message);
    dsHdmiStatusEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS, &eventData , 0);
    handler.Unsubscribe(0, _T("onInputStatusChanged"), _T("client.events.onInputStatusChanged"), message); 
}
TEST_F(HdmiInputInitializedEventDsTest, onSignalChangedStable)
{
   ASSERT_TRUE(dsHdmiSignalStatusEventHandler != nullptr);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.onSignalChanged.onSignalChanged\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"signalStatus\":\"stableSignal\"}}")));
                return Core::ERROR_NONE;
            }));
    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_sig_status.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_sig_status.status = dsHDMI_IN_SIGNAL_STATUS_STABLE;	
    handler.Subscribe(0, _T("onSignalChanged"), _T("client.events.onSignalChanged"), message);
    dsHdmiSignalStatusEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_SIGNAL_STATUS, &eventData , 0);
    handler.Unsubscribe(0, _T("onSignalChanged"), _T("client.events.onSignalChanged"), message); 
}
TEST_F(HdmiInputInitializedEventDsTest, onSignalChangedNoSignal)
{
   ASSERT_TRUE(dsHdmiSignalStatusEventHandler != nullptr);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.onSignalChanged.onSignalChanged\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"signalStatus\":\"noSignal\"}}")));
                return Core::ERROR_NONE;
            }));
    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_sig_status.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_sig_status.status = dsHDMI_IN_SIGNAL_STATUS_NOSIGNAL;	
    handler.Subscribe(0, _T("onSignalChanged"), _T("client.events.onSignalChanged"), message);
    dsHdmiSignalStatusEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_SIGNAL_STATUS, &eventData , 0);
    handler.Unsubscribe(0, _T("onSignalChanged"), _T("client.events.onSignalChanged"), message); 
}
TEST_F(HdmiInputInitializedEventDsTest, onSignalChangedUnstable)
{
   ASSERT_TRUE(dsHdmiSignalStatusEventHandler != nullptr);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.onSignalChanged.onSignalChanged\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"signalStatus\":\"unstableSignal\"}}")));
                return Core::ERROR_NONE;
            }));
    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_sig_status.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_sig_status.status = dsHDMI_IN_SIGNAL_STATUS_UNSTABLE;	
    handler.Subscribe(0, _T("onSignalChanged"), _T("client.events.onSignalChanged"), message);
    dsHdmiSignalStatusEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_SIGNAL_STATUS, &eventData , 0);
    handler.Unsubscribe(0, _T("onSignalChanged"), _T("client.events.onSignalChanged"), message); 
}
TEST_F(HdmiInputInitializedEventDsTest, onSignalChangedNotSupported)
{
   ASSERT_TRUE(dsHdmiSignalStatusEventHandler != nullptr);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.onSignalChanged.onSignalChanged\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"signalStatus\":\"notSupportedSignal\"}}")));
                return Core::ERROR_NONE;
            }));
    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_sig_status.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_sig_status.status = dsHDMI_IN_SIGNAL_STATUS_NOTSUPPORTED;	
    handler.Subscribe(0, _T("onSignalChanged"), _T("client.events.onSignalChanged"), message);
    dsHdmiSignalStatusEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_SIGNAL_STATUS, &eventData , 0);
    handler.Unsubscribe(0, _T("onSignalChanged"), _T("client.events.onSignalChanged"), message); 
}
TEST_F(HdmiInputInitializedEventDsTest, onSignalChangedDefault)
{
   ASSERT_TRUE(dsHdmiSignalStatusEventHandler != nullptr);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.onSignalChanged.onSignalChanged\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"signalStatus\":\"none\"}}")));
                return Core::ERROR_NONE;
            }));
    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_sig_status.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_sig_status.status = dsHDMI_IN_SIGNAL_STATUS_MAX;	
    handler.Subscribe(0, _T("onSignalChanged"), _T("client.events.onSignalChanged"), message);
    dsHdmiSignalStatusEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_SIGNAL_STATUS, &eventData , 0);
    handler.Unsubscribe(0, _T("onSignalChanged"), _T("client.events.onSignalChanged"), message); 
}

TEST_F(HdmiInputInitializedEventDsTest, videoStreamInfoUpdate1)
{
   ASSERT_TRUE(dsHdmiVideoModeEventHandler != nullptr);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.videoStreamInfoUpdate.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":1920,\"height\":1080,\"progressive\":false,\"frameRateN\":60000,\"frameRateD\":1001}}")));
                return Core::ERROR_NONE;
            }));
    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_video_mode.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_video_mode.resolution.pixelResolution = dsVIDEO_PIXELRES_1920x1080;	
    eventData.data.hdmi_in_video_mode.resolution.interlaced = true;	
    eventData.data.hdmi_in_video_mode.resolution.frameRate = dsVIDEO_FRAMERATE_59dot94;	
    handler.Subscribe(0, _T("videoStreamInfoUpdate"), _T("client.events.videoStreamInfoUpdate"), message);
    dsHdmiVideoModeEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE, &eventData , 0);
    handler.Unsubscribe(0, _T("videoStreamInfoUpdate"), _T("client.events.videoStreamInfoUpdate"), message); 
}
TEST_F(HdmiInputInitializedEventDsTest, videoStreamInfoUpdate2)
{
   ASSERT_TRUE(dsHdmiVideoModeEventHandler != nullptr);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.videoStreamInfoUpdate.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":720,\"height\":480,\"progressive\":false,\"frameRateN\":24000,\"frameRateD\":1000}}")));
                return Core::ERROR_NONE;
            }));
    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_video_mode.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_video_mode.resolution.pixelResolution = dsVIDEO_PIXELRES_720x480;	
    eventData.data.hdmi_in_video_mode.resolution.interlaced = true;	
    eventData.data.hdmi_in_video_mode.resolution.frameRate = dsVIDEO_FRAMERATE_24;	
    handler.Subscribe(0, _T("videoStreamInfoUpdate"), _T("client.events.videoStreamInfoUpdate"), message);
    dsHdmiVideoModeEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE, &eventData , 0);
    handler.Unsubscribe(0, _T("videoStreamInfoUpdate"), _T("client.events.videoStreamInfoUpdate"), message); 
}
TEST_F(HdmiInputInitializedEventDsTest, videoStreamInfoUpdate3)
{
   ASSERT_TRUE(dsHdmiVideoModeEventHandler != nullptr);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.videoStreamInfoUpdate.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":720,\"height\":576,\"progressive\":false,\"frameRateN\":25000,\"frameRateD\":1000}}")));
                return Core::ERROR_NONE;
            }));
    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_video_mode.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_video_mode.resolution.pixelResolution = dsVIDEO_PIXELRES_720x576;	
    eventData.data.hdmi_in_video_mode.resolution.interlaced = true;	
    eventData.data.hdmi_in_video_mode.resolution.frameRate = dsVIDEO_FRAMERATE_25;	
    handler.Subscribe(0, _T("videoStreamInfoUpdate"), _T("client.events.videoStreamInfoUpdate"), message);
    dsHdmiVideoModeEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE, &eventData , 0);
    handler.Unsubscribe(0, _T("videoStreamInfoUpdate"), _T("client.events.videoStreamInfoUpdate"), message); 
}
TEST_F(HdmiInputInitializedEventDsTest, videoStreamInfoUpdate4)
{
   ASSERT_TRUE(dsHdmiVideoModeEventHandler != nullptr);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.videoStreamInfoUpdate.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":3840,\"height\":2160,\"progressive\":false,\"frameRateN\":30000,\"frameRateD\":1000}}")));
                return Core::ERROR_NONE;
            }));
    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_video_mode.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_video_mode.resolution.pixelResolution = dsVIDEO_PIXELRES_3840x2160;	
    eventData.data.hdmi_in_video_mode.resolution.interlaced = true;	
    eventData.data.hdmi_in_video_mode.resolution.frameRate = dsVIDEO_FRAMERATE_30;	
    handler.Subscribe(0, _T("videoStreamInfoUpdate"), _T("client.events.videoStreamInfoUpdate"), message);
    dsHdmiVideoModeEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE, &eventData , 0);
    handler.Unsubscribe(0, _T("videoStreamInfoUpdate"), _T("client.events.videoStreamInfoUpdate"), message); 
}
TEST_F(HdmiInputInitializedEventDsTest, videoStreamInfoUpdate5)
{
   ASSERT_TRUE(dsHdmiVideoModeEventHandler != nullptr);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.videoStreamInfoUpdate.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":4096,\"height\":2160,\"progressive\":false,\"frameRateN\":50000,\"frameRateD\":1000}}")));
                return Core::ERROR_NONE;
            }));
    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_video_mode.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_video_mode.resolution.pixelResolution = dsVIDEO_PIXELRES_4096x2160;	
    eventData.data.hdmi_in_video_mode.resolution.interlaced = true;	
    eventData.data.hdmi_in_video_mode.resolution.frameRate = dsVIDEO_FRAMERATE_50;	
    handler.Subscribe(0, _T("videoStreamInfoUpdate"), _T("client.events.videoStreamInfoUpdate"), message);
    dsHdmiVideoModeEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE, &eventData , 0);
    handler.Unsubscribe(0, _T("videoStreamInfoUpdate"), _T("client.events.videoStreamInfoUpdate"), message); 
}
TEST_F(HdmiInputInitializedEventDsTest, videoStreamInfoUpdate6)
{
   ASSERT_TRUE(dsHdmiVideoModeEventHandler != nullptr);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.videoStreamInfoUpdate.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":4096,\"height\":2160,\"progressive\":false,\"frameRateN\":60000,\"frameRateD\":1000}}")));
                return Core::ERROR_NONE;
            }));
    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_video_mode.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_video_mode.resolution.pixelResolution = dsVIDEO_PIXELRES_4096x2160;	
    eventData.data.hdmi_in_video_mode.resolution.interlaced = true;	
    eventData.data.hdmi_in_video_mode.resolution.frameRate = dsVIDEO_FRAMERATE_60;	
    handler.Subscribe(0, _T("videoStreamInfoUpdate"), _T("client.events.videoStreamInfoUpdate"), message);
    dsHdmiVideoModeEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE, &eventData , 0);
    handler.Unsubscribe(0, _T("videoStreamInfoUpdate"), _T("client.events.videoStreamInfoUpdate"), message); 
}
TEST_F(HdmiInputInitializedEventDsTest, videoStreamInfoUpdate7)
{
   ASSERT_TRUE(dsHdmiVideoModeEventHandler != nullptr);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.videoStreamInfoUpdate.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":4096,\"height\":2160,\"progressive\":false,\"frameRateN\":24000,\"frameRateD\":1001}}")));
                return Core::ERROR_NONE;
            }));
    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_video_mode.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_video_mode.resolution.pixelResolution = dsVIDEO_PIXELRES_4096x2160;	
    eventData.data.hdmi_in_video_mode.resolution.interlaced = true;	
    eventData.data.hdmi_in_video_mode.resolution.frameRate = dsVIDEO_FRAMERATE_23dot98;	
    handler.Subscribe(0, _T("videoStreamInfoUpdate"), _T("client.events.videoStreamInfoUpdate"), message);
    dsHdmiVideoModeEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE, &eventData , 0);
    handler.Unsubscribe(0, _T("videoStreamInfoUpdate"), _T("client.events.videoStreamInfoUpdate"), message); 
}
TEST_F(HdmiInputInitializedEventDsTest, videoStreamInfoUpdate8)
{
   ASSERT_TRUE(dsHdmiVideoModeEventHandler != nullptr);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.videoStreamInfoUpdate.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":4096,\"height\":2160,\"progressive\":false,\"frameRateN\":30000,\"frameRateD\":1001}}")));
                return Core::ERROR_NONE;
            }));
    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_video_mode.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_video_mode.resolution.pixelResolution = dsVIDEO_PIXELRES_4096x2160;	
    eventData.data.hdmi_in_video_mode.resolution.interlaced = true;	
    eventData.data.hdmi_in_video_mode.resolution.frameRate = dsVIDEO_FRAMERATE_29dot97;	
    handler.Subscribe(0, _T("videoStreamInfoUpdate"), _T("client.events.videoStreamInfoUpdate"), message);
    dsHdmiVideoModeEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE, &eventData , 0);
    handler.Unsubscribe(0, _T("videoStreamInfoUpdate"), _T("client.events.videoStreamInfoUpdate"), message); 
}
TEST_F(HdmiInputInitializedEventDsTest, videoStreamInfoUpdate9)
{
   ASSERT_TRUE(dsHdmiVideoModeEventHandler != nullptr);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.videoStreamInfoUpdate.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":1280,\"height\":720,\"progressive\":false,\"frameRateN\":30000,\"frameRateD\":1001}}")));
                return Core::ERROR_NONE;
            }));
    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_video_mode.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_video_mode.resolution.pixelResolution = dsVIDEO_PIXELRES_1280x720;	
    eventData.data.hdmi_in_video_mode.resolution.interlaced = true;	
    eventData.data.hdmi_in_video_mode.resolution.frameRate = dsVIDEO_FRAMERATE_29dot97;	
    handler.Subscribe(0, _T("videoStreamInfoUpdate"), _T("client.events.videoStreamInfoUpdate"), message);
    dsHdmiVideoModeEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE, &eventData , 0);
    handler.Unsubscribe(0, _T("videoStreamInfoUpdate"), _T("client.events.videoStreamInfoUpdate"), message); 
}
TEST_F(HdmiInputInitializedEventDsTest, videoStreamInfoUpdateDefault)
{
   ASSERT_TRUE(dsHdmiVideoModeEventHandler != nullptr);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.videoStreamInfoUpdate.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":1920,\"height\":1080,\"progressive\":false,\"frameRateN\":60000,\"frameRateD\":1000}}")));
                return Core::ERROR_NONE;
            }));
    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_video_mode.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_video_mode.resolution.pixelResolution = dsVIDEO_PIXELRES_MAX;
    eventData.data.hdmi_in_video_mode.resolution.interlaced = true;	
    eventData.data.hdmi_in_video_mode.resolution.frameRate= dsVIDEO_FRAMERATE_MAX;
    handler.Subscribe(0, _T("videoStreamInfoUpdate"), _T("client.events.videoStreamInfoUpdate"), message);
    dsHdmiVideoModeEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE, &eventData , 0);
    handler.Unsubscribe(0, _T("videoStreamInfoUpdate"), _T("client.events.videoStreamInfoUpdate"), message); 
}
TEST_F(HdmiInputInitializedEventDsTest, hdmiGameFeatureStatusUpdate)
{
   ASSERT_TRUE(dsHdmiGameFeatureStatusEventHandler != nullptr);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.hdmiGameFeatureStatusUpdate.hdmiGameFeatureStatusUpdate\",\"params\":{\"id\":0,\"gameFeature\":\"ALLM\",\"mode\":true}}")));

                return Core::ERROR_NONE;
            }));


    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_allm_mode.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_allm_mode.allm_mode = true;	
    handler.Subscribe(0, _T("hdmiGameFeatureStatusUpdate"), _T("client.events.hdmiGameFeatureStatusUpdate"), message);

    dsHdmiGameFeatureStatusEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_ALLM_STATUS, &eventData , 0);

    handler.Unsubscribe(0, _T("hdmiGameFeatureStatusUpdate"), _T("client.events.hdmiGameFeatureStatusUpdate"), message); 
}
