/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "HdmiCec.h"

#include "FactoriesImplementation.h"


#include "IarmBusMock.h"
#include "ServiceMock.h"
#include "devicesettings.h"
#include "HdmiCecMock.h"
using namespace WPEFramework;
using ::testing::NiceMock;

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
    LibCCECImplMock         *p_libCCECImplMock = nullptr ;
    ConnectionImplMock      *p_connectionImplMock = nullptr ;
    HdmiCecDsTest()
        : HdmiCecTest()
    {
        p_libCCECImplMock  = new testing::NiceMock <LibCCECImplMock>;
        LibCCEC::setImpl(p_libCCECImplMock);

        p_connectionImplMock  = new testing::NiceMock <ConnectionImplMock>;
        Connection::setImpl(p_connectionImplMock);

    }
    virtual ~HdmiCecDsTest() override
    {
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
    }
};

class HdmiCecInitializedTest : public HdmiCecTest {
protected:
    IarmBusImplMock         *p_iarmBusImplMock = nullptr ;
    IARM_EventHandler_t cecMgrEventHandler;
    IARM_EventHandler_t dsHdmiEventHandler;
    LibCCECImplMock         *p_libCCECImplMock = nullptr ;
    ConnectionImplMock      *p_connectionImplMock = nullptr ;

    HdmiCecInitializedTest()
        : HdmiCecTest()
    {
        p_iarmBusImplMock  = new NiceMock <IarmBusImplMock>;
        IarmBus::setImpl(p_iarmBusImplMock);

        p_libCCECImplMock  = new testing::NiceMock <LibCCECImplMock>;
        LibCCEC::setImpl(p_libCCECImplMock);

        p_connectionImplMock  = new testing::NiceMock <ConnectionImplMock>;
        Connection::setImpl(p_connectionImplMock);

        ON_CALL(*p_iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
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

        //Setenable needs to run firzt, as it turns everything on, locally.
	    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));
    }
    virtual ~HdmiCecInitializedTest() override
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
        //Turning off HdmiCec. otherwise we get segementation faults as things memory early while threads are still running
        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": false}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

    }
};
class HdmiCecInitializedEventTest : public HdmiCecInitializedTest {
protected:
    testing::NiceMock<ServiceMock> service;
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

TEST_F(HdmiCecTest, RegisteredMethods)
{

    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getCECAddresses")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("sendMessage")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getActiveSourceStatus")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getDeviceList")));
    
}

TEST_F(HdmiCecDsTest, getEnabledFalse)
{
    //Without setting cecEnable to true.
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getEnabled"), _T(""), response));
    EXPECT_EQ(response, string("{\"enabled\":false,\"success\":true}"));
}


TEST_F(HdmiCecInitializedTest, getEnabledTrue)
{
    //Get enabled just checks if CEC is on, which is a global variable.
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getEnabled"), _T(""), response));
    EXPECT_EQ(response, string("{\"enabled\":true,\"success\":true}"));

}

TEST_F(HdmiCecInitializedTest, getActiveSourceStatusTrue)
{
    //ActiveSource is a local variable, no mocked functions to check.
    //Calling the sendMessage() function with the proper message sets ActiveSource to true.
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendMessage"), _T("{\"message\": \"P4IwAA==\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getActiveSourceStatus"), _T(""), response));
    EXPECT_EQ(response, string("{\"status\":true,\"success\":true}"));


}
TEST_F(HdmiCecInitializedTest, getActiveSourceStatusFalse)
{
    //ActiveSource is a local variable, no mocked functions to check.
    //Active source is false by default.
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getActiveSourceStatus"), _T(""), response));
    EXPECT_EQ(response, string("{\"status\":false,\"success\":true}"));
}

TEST_F(HdmiCecInitializedTest, getCECAddress)
{
    EXPECT_CALL(*p_libCCECImplMock, getPhysicalAddress(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](uint32_t *physAddress) {
                *physAddress = (uint32_t)12345;
            }));
    ASSERT_TRUE(cecMgrEventHandler != nullptr);
    IARM_Bus_CECMgr_Status_Updated_Param_t eventData;

    //Active Source Status update sets the address/logical address and what not to non-default values
    eventData.logicalAddress = 5;
    
    cecMgrEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_STATUS_UPDATED, &eventData , 0);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getCECAddresses"), _T(""), response));
    EXPECT_EQ(response, string(_T("{\"CECAddresses\":{\"physicalAddress\":12345,\"logicalAddress\":5,\"deviceType\":\"Audio System\"},\"success\":true}")));

}

TEST_F(HdmiCecInitializedEventTest, cecAddressesChanged)
{
    EXPECT_CALL(*p_libCCECImplMock, getPhysicalAddress(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](uint32_t *physAddress) {
                *physAddress = (uint32_t)1234;
            }));
    ASSERT_TRUE(dsHdmiEventHandler != nullptr);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.HdmiCec.cecAddressesChanged\",\"params\":{\"CECAddresses\":{\"physicalAddress\":1234}}}")));

                return Core::ERROR_NONE;
            }));
    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_hpd.event = 0;	
    handler.Subscribe(0, _T("cecAddressesChanged"), _T("org.rdk.HdmiCec"), message);
    dsHdmiEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG, &eventData , 0);
    handler.Unsubscribe(0, _T("cecAddressesChanged"), _T("org.rdk.HdmiCec"), message);
}

TEST_F(HdmiCecInitializedTest, getDeviceList)
{  
    int iCounter = 0;
    //Checking to see if one of the values has been filled in (as the rest get filled in at the same time, and waiting if its not.
    while ((!Plugin::HdmiCec::_instance->deviceList[0].m_isOSDNameUpdated) && (iCounter < (2*10))) { //sleep for 2sec.
		usleep (100 * 1000); //sleep for 100 milli sec
		iCounter ++;
	}

    const char* val = "TEST";
    OSDName name = OSDName(val);
    SetOSDName osdName = SetOSDName(name);

    Header header;
    header.from = LogicalAddress(1); //specifies with logicalAddress in the deviceList we're using

    VendorID vendor(1,2,3);
    DeviceVendorID vendorid(vendor);

    plugin->process(osdName, header); //calls the process that sets osdName for LogicalAddress = 1
    plugin->process(vendorid, header); //calls the process that sets vendorID for LogicalAddress = 1

    
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceList"), _T(""), response));

    EXPECT_EQ(response, string(_T("{\"numberofdevices\":14,\"deviceList\":[{\"logicalAddress\":1,\"osdName\":\"TEST\",\"vendorID\":\"123\"},{\"logicalAddress\":2,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":3,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":4,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":5,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":6,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":7,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":8,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":9,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":10,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":11,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":12,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":13,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":14,\"osdName\":\"NA\",\"vendorID\":\"000\"}],\"success\":true}")));


}
