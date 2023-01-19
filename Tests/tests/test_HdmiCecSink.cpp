nclude <gtest/gtest.h>

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
using namespace WPEFramework;

class HdmiCecSinkTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::HdmiCecSink> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    HdmiCecSinkTest()
        : plugin(Core::ProxyType<Plugin::HdmiCecSink>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
    }
    virtual ~HdmiCecSinkTest() = default;
};
class HdmiCecSinkDsTest : public HdmiCecSinkTest {
protected:
    LibCCECImplMock libCCECImplMock;
    ConnectionImplMock connectionImplMock;
    HdmiCecSinkDsTest()
        : HdmiCecSinkTest()
    {
        LibCCEC::getInstance().impl = &libCCECImplMock;
    }
    virtual ~HdmiCecSinkDsTest() override
    {
        LibCCEC::getInstance().impl = nullptr;
    }
};

class HdmiCecSinkInitializedTest : public HdmiCecSinkTest {
protected:
    IarmBusImplMock iarmBusImplMock;
    IARM_EventHandler_t cecMgrEventHandler;
    IARM_EventHandler_t dsHdmiEventHandler;
 
    HdmiCecSinkInitializedTest()
        : HdmiCecSinkTest()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;

        ON_CALL(iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                   if ((string(IARM_BUS_CECMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED)) {
                        EXPECT_TRUE(handler != nullptr);
                        cecMgrEventHandler = handler;
                    }
		   if ((string(IARM_BUS_CECMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_CECMGR_EVENT_STATUS_UPDATED)) {
                        EXPECT_TRUE(handler != nullptr);
                        cecMgrEventHandler = handler;
                    }
                   if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG)) {
                        EXPECT_TRUE(handler != nullptr);
                        dsHdmiEventHandler = handler;
                    }

                    return IARM_RESULT_SUCCESS;
                }));

        EXPECT_EQ(string(""), plugin->Initialize(nullptr));
    }
    virtual ~HdmiCecSinkInitializedTest() override
    {  
        plugin->Deinitialize(nullptr);

        IarmBus::getInstance().impl = nullptr;
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
    LibCCECImplMock libCCECImplMock;
    ConnectionImplMock connectionImplMock;
    DeviceTypeImplMock deviceTypeImplMock;
    LogicalAddressImplMock logicalAddressImplMock;
     

    HdmiCecSinkInitializedEventDsTest()
        : HdmiCecSinkInitializedEventTest()
    {
        LibCCEC::getInstance().impl = &libCCECImplMock;
        plugin->getConnection()->impl = &connectionImplMock;	
	    DeviceType::getInstance(1).impl = &deviceTypeImplMock;
        LogicalAddress::getInstance(1).impl = &logicalAddressImplMock;
     

    }

    virtual ~HdmiCecSinkInitializedEventDsTest() override
    {
        
        LibCCEC::getInstance().impl = nullptr;
        plugin->getConnection()->impl = nullptr;
        DeviceType::getInstance(1).impl = nullptr;

    }
};


TEST_F(HdmiCecSinkTest, RegisteredMethods)
{

    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getCECAddresses")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("sendMessage")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getActiveSourceStatus")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getDeviceList")));
    
}

TEST_F(HdmiCecSinkInitializedEventDsTest, setEnabled)
{

    //setting HdmiCecSink to enabled.
    ON_CALL(libCCECImplMock, getLogicalAddress(::testing::_))
        .WillByDefault(::testing::Return(1));
	ON_CALL(deviceTypeImplMock, toString())
        .WillByDefault(::testing::Return("Here"));
    ON_CALL(connectionImplMock, open())
        .WillByDefault(::testing::Return());
    ON_CALL(connectionImplMock, addFrameListener(::testing::_))
        .WillByDefault(::testing::Return());
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

    //Turning off HdmiCecSink. otherwise we get segementation faults as things memory early while threads are still running
    sleep(1);//short wait to allow setEnabled to reach thread loop, where it can exit safely with segementation faults
    ON_CALL(connectionImplMock, close())
        .WillByDefault(::testing::Return());
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": false}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}


TEST_F(HdmiCecSinkInitializedEventDsTest, getEnabledTrue)
{
    //setting HdmiCecSink to enabled.
    ON_CALL(libCCECImplMock, getLogicalAddress(::testing::_))
        .WillByDefault(::testing::Return(1));
	ON_CALL(deviceTypeImplMock, toString())
        .WillByDefault(::testing::Return("Here"));
    ON_CALL(connectionImplMock, open())
        .WillByDefault(::testing::Return());
    ON_CALL(connectionImplMock, addFrameListener(::testing::_))
        .WillByDefault(::testing::Return());
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

    //Getenabled just checks if CEC is on, which is a local variable.
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getEnabled"), _T(""), response));
    EXPECT_EQ(response, string("{\"enabled\":true,\"success\":true}"));



    //Turning off HdmiCecSink. otherwise we get segementation faults as things memory early while threads are still running
    sleep(1);//short wait to allow setEnabled to reach thread loop, where it can exit safely with segementation faults
    ON_CALL(connectionImplMock, close())
        .WillByDefault(::testing::Return());
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": false}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}


TEST_F(HdmiCecSinkInitializedEventDsTest, sendMessage)
{
    //setting HdmiCecSink to enabled.
    ON_CALL(libCCECImplMock, getLogicalAddress(::testing::_))
        .WillByDefault(::testing::Return(1));
	ON_CALL(deviceTypeImplMock, toString())
        .WillByDefault(::testing::Return("Here"));
    ON_CALL(connectionImplMock, open())
        .WillByDefault(::testing::Return());
    ON_CALL(connectionImplMock, addFrameListener(::testing::_))
        .WillByDefault(::testing::Return());
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendMessage"), _T("{\"message\": \"P4IwAA==\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));


    //Turning off HdmiCecSink. otherwise we get segementation faults as things memory early while threads are still running
    sleep(1);//short wait to allow setEnabled to reach thread loop, where it can exit safely with segementation faults
    ON_CALL(connectionImplMock, close())
        .WillByDefault(::testing::Return());
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": false}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

}


TEST_F(HdmiCecSinkInitializedEventDsTest, getActiveSourceStatusTrue)
{

    //setting HdmiCecSink to enabled.
    ON_CALL(libCCECImplMock, getLogicalAddress(::testing::_))
        .WillByDefault(::testing::Return(1));
	ON_CALL(deviceTypeImplMock, toString())
        .WillByDefault(::testing::Return("Here"));
    ON_CALL(connectionImplMock, open())
        .WillByDefault(::testing::Return());
    ON_CALL(connectionImplMock, addFrameListener(::testing::_))
        .WillByDefault(::testing::Return());
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

    //Specific message sets ActiveSource to true.
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendMessage"), _T("{\"message\": \"P4IwAA==\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getActiveSourceStatus"), _T(""), response));
    EXPECT_EQ(response, string("{\"status\":true,\"success\":true}"));


     //Turning off HdmiCecSink. otherwise we get segementation faults as things memory early while threads are still running
    sleep(1);//short wait to allow setEnabled to reach thread loop, where it can exit safely with segementation faults
    ON_CALL(connectionImplMock, close())
        .WillByDefault(::testing::Return());
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": false}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));


}
TEST_F(HdmiCecSinkInitializedEventDsTest, getActiveSourceStatusFalse)
{

    //setting HdmiCecSink to enabled.
    ON_CALL(libCCECImplMock, getLogicalAddress(::testing::_))
        .WillByDefault(::testing::Return(1));
	ON_CALL(deviceTypeImplMock, toString())
        .WillByDefault(::testing::Return("Here"));
    ON_CALL(connectionImplMock, open())
        .WillByDefault(::testing::Return());
    ON_CALL(connectionImplMock, addFrameListener(::testing::_))
        .WillByDefault(::testing::Return());
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getActiveSourceStatus"), _T(""), response));
    EXPECT_EQ(response, string("{\"status\":false,\"success\":true}"));


     //Turning off HdmiCecSink. otherwise we get segementation faults as things memory early while threads are still running
    sleep(1);//short wait to allow setEnabled to reach thread loop, where it can exit safely with segementation faults
    ON_CALL(connectionImplMock, close())
        .WillByDefault(::testing::Return());
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": false}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));


}


TEST_F(HdmiCecSinkInitializedEventDsTest, getCECAddress)
{
    //setting HdmiCecSink to enabled.
    ON_CALL(libCCECImplMock, getLogicalAddress(::testing::_))
        .WillByDefault(::testing::Return(1));
	ON_CALL(deviceTypeImplMock, toString())
        .WillByDefault(::testing::Return("Here"));
    ON_CALL(connectionImplMock, open())
        .WillByDefault(::testing::Return());
    ON_CALL(connectionImplMock, addFrameListener(::testing::_))
        .WillByDefault(::testing::Return());
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));



    ON_CALL(libCCECImplMock, getPhysicalAddress(::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](uint32_t *physAddress) {
                *physAddress = (uint32_t)12345;
            }));
    ASSERT_TRUE(cecMgrEventHandler != nullptr);
    IARM_Bus_CECMgr_Status_Updated_Param_t eventData;
    eventData.logicalAddress =1;
    
    handler.Subscribe(0, _T("onActiveSourceStatusUpdate"), _T("client.events.onActiveSourceStatusUpdate"), message);

	ON_CALL(deviceTypeImplMock, toString())
        .WillByDefault(::testing::Return("New"));
    cecMgrEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_STATUS_UPDATED, &eventData , 0);


    handler.Unsubscribe(0, _T("onActiveSourceStatusUpdate"), _T("client.events.onActiveSourceStatusUpdate"), message);


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getCECAddresses"), _T(""), response));
    EXPECT_EQ(response, string("{\"CECAddresses\":{\"physicalAddress\":12345,\"logicalAddress\":1,\"deviceType\":\"New\"},\"success\":true}"));




    //Turning off HdmiCecSink. otherwise we get segementation faults as things memory early while threads are still running
    sleep(1);//short wait to allow setEnabled to reach thread loop, where it can exit safely with segementation faults
    ON_CALL(connectionImplMock, close())
        .WillByDefault(::testing::Return());
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": false}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}



TEST_F(HdmiCecSinkInitializedEventDsTest, onDevicesChanged)
{
 //setting HdmiCecSink to enabled.
    ON_CALL(libCCECImplMock, getLogicalAddress(::testing::_))
        .WillByDefault(::testing::Return(1));
	ON_CALL(deviceTypeImplMock, toString())
        .WillByDefault(::testing::Return("Here"));
    ON_CALL(connectionImplMock, open())
        .WillByDefault(::testing::Return());
    ON_CALL(connectionImplMock, addFrameListener(::testing::_))
        .WillByDefault(::testing::Return());
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));


    ASSERT_TRUE(dsHdmiEventHandler != nullptr);


    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_connect.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_connect.isPortConnected = true;

    handler.Subscribe(0, _T("onDeviceAdded"), _T("client.events.onDeviceAdded"), message);

    dsHdmiEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG, &eventData , 0);

    handler.Unsubscribe(0, _T("onDeviceAdded"), _T("client.events.onDeviceAdded"), message);

    //Turning off HdmiCecSink. otherwise we get segementation faults as things memory early while threads are still running
    sleep(1);//short wait to allow setEnabled to reach thread loop, where it can exit safely with segementation faults
    ON_CALL(connectionImplMock, close())
        .WillByDefault(::testing::Return());
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": false}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));


}

TEST_F(HdmiCecSinkInitializedEventDsTest, getDeviceList)
{

     //setting HdmiCecSink to enabled.
    ON_CALL(libCCECImplMock, getLogicalAddress(::testing::_))
        .WillByDefault(::testing::Return(1));
	ON_CALL(deviceTypeImplMock, toString())
        .WillByDefault(::testing::Return("Here"));
    ON_CALL(connectionImplMock, open())
        .WillByDefault(::testing::Return());
    ON_CALL(connectionImplMock, addFrameListener(::testing::_))
        .WillByDefault(::testing::Return());
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

    //Calling the device list, which is a defualt list of the HdmiCecSink class, with no easy way to add to it, hence why it's blank.
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceList"), _T(""), response));
    EXPECT_EQ(response, string("{\"numberofdevices\":0,\"deviceList\":[],\"success\":true}"));


    //Turning off HdmiCecSink. otherwise we get segementation faults as things memory early while threads are still running
    sleep(1);//short wait to allow setEnabled to reach thread loop, where it can exit safely with segementation faults
    ON_CALL(connectionImplMock, close())
        .WillByDefault(::testing::Return());
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": false}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

}

