#include <gtest/gtest.h>

#include "HdmiCec.h"

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

class HdmiCecTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::HdmiCec> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    HdmiCecTest()
        : plugin(Core::ProxyType<Plugin::HdmiCec>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
    }
    virtual ~HdmiCecTest() = default;
};
class HdmiCecDsTest : public HdmiCecTest {
protected:
    LibCCECImplMock libCCECImplMock;
    ConnectionImplMock connectionImplMock;
    LogicalAddressImplMock logicalAddressMock;
    DeviceTypeMock deviceTypeMock;
    HdmiCecDsTest()
        : HdmiCecTest()
    {
        LibCCEC::getInstance().impl = &libCCECImplMock;
	    Connection::getInstance().impl =  &connectionImplMock;
        LogicalAddress::getInstance().impl = &logicalAddressMock;
        DeviceType::getInstance().impl = &deviceTypeMock;
    }
    virtual ~HdmiCecDsTest() override
    {
        LibCCEC::getInstance().impl = nullptr;
        Connection::getInstance().impl =  nullptr;
        LogicalAddress::getInstance().impl = nullptr;
        DeviceType::getInstance().impl = nullptr;
    }
};

class HdmiCecInitializedTest : public HdmiCecTest {
protected:
    IarmBusImplMock iarmBusImplMock;
    ConnectionImplMock connectionImplMock;
    LogicalAddressImplMock logicalAddressMock;
    DeviceTypeMock deviceTypeMock;

    IARM_EventHandler_t dsHdmiCecEventHandler;
    IARM_EventHandler_t dsHdmiEventHandler;
 
    HdmiCecInitializedTest()
        : HdmiCecTest()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;
        Connection::getInstance().impl = &connectionImplMock;
        LogicalAddress::getInstance().impl = &logicalAddressMock;
        DeviceType::getInstance().impl = &deviceTypeMock;


        ON_CALL(iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                   if ((string(IARM_BUS_CECMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED)) {
                        EXPECT_TRUE(handler != nullptr);
                        dsHdmiCecEventHandler = handler;
                    }
		   if ((string(IARM_BUS_CECMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_CECMGR_EVENT_STATUS_UPDATED)) {
                        EXPECT_TRUE(handler != nullptr);
                        dsHdmiCecEventHandler = handler;
                    }
                   if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG)) {
                        EXPECT_TRUE(handler != nullptr);
                        dsHdmiEventHandler = handler;
                    }

                    return IARM_RESULT_SUCCESS;
                }));

        EXPECT_EQ(string(""), plugin->Initialize(nullptr));
    }
    virtual ~HdmiCecInitializedTest() override
    {
	sleep(2);
        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": false}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));
		
	sleep(2);
        Connection::getInstance().impl = nullptr;
        LogicalAddress::getInstance().impl = nullptr;
        DeviceType::getInstance().impl = nullptr;


        plugin->Deinitialize(nullptr);

        IarmBus::getInstance().impl = nullptr;
    }
};
class HdmiCecInitializedEventTest : public HdmiCecInitializedTest {
protected:
    ServiceMock service;
    Core::JSONRPC::Message message;
    FactoriesImplementation factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    HdmiCecInitializedEventTest()
        : HdmiCecInitializedTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }

    virtual ~HdmiCecInitializedEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();
        PluginHost::IFactories::Assign(nullptr);
    }
};
class HdmiCecInitializedEventDsTest : public HdmiCecInitializedEventTest {
protected:
    LibCCECImplMock libCCECImplMock;
        
     
    HdmiCecInitializedEventDsTest()
        : HdmiCecInitializedEventTest()
    {
        LibCCEC::getInstance().impl = &libCCECImplMock;
	

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));


    }

    virtual ~HdmiCecInitializedEventDsTest() override
    {
        LibCCEC::getInstance().impl = nullptr;

    }
};


TEST_F(HdmiCecTest, RegisteredMethods)
{

    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getCECAddresses")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("sendMessage")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getActiveSourceStatus")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getDeviceList")));
    
}

TEST_F(HdmiCecInitializedEventDsTest, getEnabled)
{

    Header head;
    const Header& head2 = head;

    ReportPhysicalAddress address;

    const ReportPhysicalAddress& address2 = address;

    ReportPowerStatus status;

    const ReportPowerStatus& status2 = status;


    Plugin::HdmiCec::_instance->process(address2, head2);
    Plugin::HdmiCec::_instance->process(status2, head2);



    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getEnabled"), _T(""), response));
    EXPECT_EQ(response, string("{\"enabled\":true,\"success\":true}"));


}
TEST_F(HdmiCecDsTest, getActiveSourceStatus)
{
   ActiveSourceImplMock activeSourceImplMock;

     ActiveSource source;
     source.impl = &activeSourceImplMock;
    const ActiveSource& source2 = source;
    Header head;
    const Header& head2 = head;

    ImageViewOn image;

    const ImageViewOn& image2 = image;

    Plugin::HdmiCec::_instance->process(source2, head2);
    Plugin::HdmiCec::_instance->process(image2, head2);


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getActiveSourceStatus"), _T(""), response));
    EXPECT_EQ(response, string("{\"status\":false,\"success\":true}"));


}
TEST_F(HdmiCecDsTest, getCECAddress)
{

    Header head;
    const Header& head2 = head;

    TextViewOn text;

    const TextViewOn& text2 = text;

    Plugin::HdmiCec::_instance->process(text2, head2);



    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getCECAddresses"), _T(""), response));
    EXPECT_EQ(response, string("{\"CECAddresses\":{\"physicalAddress\":252645135,\"logicalAddress\":255,\"deviceType\":\"None\"},\"success\":true}"));


}

TEST_F(HdmiCecInitializedEventDsTest, sendMessage)
{
    Header head;
    const Header& head2 = head;

    CECVersion version;

    const CECVersion& version2 = version;

    Plugin::HdmiCec::_instance->process(version2, head2);


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendMessage"), _T("{\"message\": \"123456789\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));


}

TEST_F(HdmiCecInitializedEventDsTest, hdmiCecActiveStatusUpdate)
{
   ASSERT_TRUE(dsHdmiCecEventHandler != nullptr);
   IARM_Bus_CECMgr_Status_Updated_Param_t eventData;
    eventData.logicalAddress =1;
    
    handler.Subscribe(0, _T("onActiveSourceStatusUpdate"), _T("client.events.onActiveSourceStatusUpdate"), message);


    dsHdmiCecEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_STATUS_UPDATED, &eventData , 0);


    handler.Unsubscribe(0, _T("onActiveSourceStatusUpdate"), _T("client.events.onActiveSourceStatusUpdate"), message);
}

TEST_F(HdmiCecInitializedEventDsTest, onDevicesChanged)
{
    ASSERT_TRUE(dsHdmiEventHandler != nullptr);


    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_connect.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_connect.isPortConnected = true;

    handler.Subscribe(0, _T("onDeviceAdded"), _T("client.events.onDeviceAdded"), message);

    dsHdmiEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG, &eventData , 0);

    handler.Unsubscribe(0, _T("onDeviceAdded"), _T("client.events.onDeviceAdded"), message);


}

TEST_F(HdmiCecInitializedEventDsTest, getDeviceList)
{

    //Calling the device list, which is a defualt list of the hdmiCec class, with no easy way to add to it, hence why it's blank.
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceList"), _T(""), response));
    EXPECT_EQ(response, string("{\"numberofdevices\":0,\"deviceList\":[],\"success\":true}"));

}

