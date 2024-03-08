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
#include "HdmiCecSource.h"

#include "FactoriesImplementation.h"


#include "IarmBusMock.h"
#include "ServiceMock.h"
#include "devicesettings.h"
#include "HdmiCecMock.h"
#include "DisplayMock.h"
#include "VideoOutputPortMock.h"
#include "HostMock.h"
#include "ManagerMock.h"



using namespace WPEFramework;

using namespace WPEFramework;

class HdmiCecSourceTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::HdmiCecSource> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    HdmiCecSourceTest()
        : plugin(Core::ProxyType<Plugin::HdmiCecSource>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
    }
    virtual ~HdmiCecSourceTest() = default;
};
class HdmiCecSourceDsTest : public HdmiCecSourceTest {
protected:
    LibCCECImplMock      *p_libCCECImplMock = nullptr ;

    HdmiCecSourceDsTest()
        : HdmiCecSourceTest()
    {
        p_libCCECImplMock  = new testing::NiceMock <LibCCECImplMock>;
        LibCCEC::setImpl(p_libCCECImplMock);
    }
    virtual ~HdmiCecSourceDsTest() override
    {
         LibCCEC::setImpl(nullptr);
         if (p_libCCECImplMock != nullptr)
         {
             delete p_libCCECImplMock;
             p_libCCECImplMock = nullptr;
         }
    }
};

class HdmiCecSourceInitializedTest : public HdmiCecSourceTest {
protected:
    IarmBusImplMock   *p_iarmBusImplMock = nullptr ;
    IARM_EventHandler_t cecMgrEventHandler;
    IARM_EventHandler_t dsHdmiEventHandler;
    IARM_EventHandler_t pwrMgrEventHandler;
    LibCCECImplMock      *p_libCCECImplMock = nullptr ;

    ManagerImplMock   *p_managerImplMock = nullptr ;
    HostImplMock      *p_hostImplMock = nullptr ;

    ConnectionImplMock      *p_connectionImplMock = nullptr ;
    VideoOutputPortMock      *p_videoOutputPortMock = nullptr ;

    MessageEncoderMock      *p_messageEncoderMock = nullptr ;
    DisplayMock      *p_displayMock = nullptr ;

    HdmiCecSourceInitializedTest()
        : HdmiCecSourceTest()
    {
        p_iarmBusImplMock  = new testing::NiceMock <IarmBusImplMock>;
        IarmBus::setImpl(p_iarmBusImplMock);

        p_hostImplMock  = new testing::NiceMock <HostImplMock>;
        device::Host::setImpl(p_hostImplMock);

        p_managerImplMock  = new testing::NiceMock <ManagerImplMock>;
        device::Manager::setImpl(p_managerImplMock);

        p_libCCECImplMock  = new testing::NiceMock <LibCCECImplMock>;
        LibCCEC::setImpl(p_libCCECImplMock);

        p_messageEncoderMock  = new testing::NiceMock <MessageEncoderMock>;
        MessageEncoder::setImpl(p_messageEncoderMock);

        p_connectionImplMock  = new testing::NiceMock <ConnectionImplMock>;
        Connection::setImpl(p_connectionImplMock);

        p_videoOutputPortMock  = new testing::NiceMock <VideoOutputPortMock>;
        device::VideoOutputPort::setImpl(p_videoOutputPortMock);

        p_displayMock  = new testing::NiceMock <DisplayMock>;
        device::Display::setImpl(p_displayMock);

        //OnCall required for intialize to run properly
        ON_CALL(*p_hostImplMock, getVideoOutputPort(::testing::_))
            .WillByDefault(::testing::ReturnRef(device::VideoOutputPort::getInstance()));

        ON_CALL(*p_videoOutputPortMock, getDisplay())
            .WillByDefault(::testing::ReturnRef(device::Display::getInstance()));

        ON_CALL(*p_videoOutputPortMock, isDisplayConnected())
            .WillByDefault(::testing::Return(true));

        ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const DataBlock&>(::testing::_)))
            .WillByDefault(::testing::ReturnRef(CECFrame::getInstance()));

        ON_CALL(*p_displayMock, getEDIDBytes(::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](std::vector<uint8_t> &edidVec2) {
                    edidVec2 = std::vector<uint8_t>({ 't', 'e', 's', 't' });
                }));

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
                if ((string(IARM_BUS_PWRMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_PWRMGR_EVENT_MODECHANGED)) {
                        EXPECT_TRUE(handler != nullptr);
                        pwrMgrEventHandler = handler;
                    }

                    return IARM_RESULT_SUCCESS;
                }));

        ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                if (strcmp(methodName, IARM_BUS_PWRMGR_API_GetPowerState) == 0) {
                    auto* param = static_cast<IARM_Bus_PWRMgr_GetPowerState_Param_t*>(arg);
                    param->curState = IARM_BUS_PWRMGR_POWERSTATE_ON;
                }
                return IARM_RESULT_SUCCESS;
            });


        EXPECT_EQ(string(""), plugin->Initialize(nullptr));
        //Set enabled needs to be
        ON_CALL(*p_libCCECImplMock, getLogicalAddress(::testing::_))
            .WillByDefault(::testing::Return(1));
        ON_CALL(*p_connectionImplMock, open())
            .WillByDefault(::testing::Return());
        ON_CALL(*p_connectionImplMock, addFrameListener(::testing::_))
            .WillByDefault(::testing::Return());
        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

    }
    virtual ~HdmiCecSourceInitializedTest() override
    {
            int lCounter = 0;
            while ((Plugin::HdmiCecSource::_instance->deviceList[0].m_isOSDNameUpdated) && (lCounter < (2*10))) { //sleep for 2sec.
                        usleep (100 * 1000); //sleep for 100 milli sec
                        lCounter ++;
                }
            EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": false}"), response));
            EXPECT_EQ(response, string("{\"success\":true}"));
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
            device::Display::setImpl(nullptr);
            if (p_displayMock != nullptr)
            {
                delete p_displayMock;
                p_displayMock = nullptr;
            }
            device::VideoOutputPort::setImpl(nullptr);
            if (p_videoOutputPortMock != nullptr)
            {
                delete p_videoOutputPortMock;
                p_videoOutputPortMock = nullptr;
            }
            device::Manager::setImpl(nullptr);
            if (p_managerImplMock != nullptr)
            {
                delete p_managerImplMock;
                p_managerImplMock = nullptr;
            }
            device::Host::setImpl(nullptr);
            if (p_hostImplMock != nullptr)
            {
                delete p_hostImplMock;
                p_hostImplMock = nullptr;
            }
    }
};
class HdmiCecSourceInitializedEventTest : public HdmiCecSourceInitializedTest {
protected:
    testing::NiceMock<ServiceMock> service;
    Core::JSONRPC::Message message;
    FactoriesImplementation factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    HdmiCecSourceInitializedEventTest()
        : HdmiCecSourceInitializedTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
        plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }

    virtual ~HdmiCecSourceInitializedEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();
        PluginHost::IFactories::Assign(nullptr);

    }
};

TEST_F(HdmiCecSourceTest, RegisteredMethods)
{

    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getActiveSourceStatus")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getDeviceList")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getOSDName")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getOTPEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getVendorId")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("performOTPAction")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("sendKeyPressEvent")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("sendStandbyMessage")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setOSDName")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setOTPEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setVendorId")));

}

TEST_F(HdmiCecSourceDsTest, getEnabledFalse)
{
    //Without setting cecEnable to true.
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getEnabled"), _T(""), response));
    EXPECT_EQ(response, string("{\"enabled\":false,\"success\":true}"));
}


TEST_F(HdmiCecSourceInitializedTest, getEnabledTrue)
{
    //Get enabled just checks if CEC is on, which is a global variable.
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getEnabled"), _T(""), response));
    EXPECT_EQ(response, string("{\"enabled\":true,\"success\":true}"));

}

TEST_F(HdmiCecSourceInitializedTest, getActiveSourceStatusTrue)
{
    //SetsOTP to on.
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setOTPEnabled"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

    //Sets Activesource to true
        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("performOTPAction"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getActiveSourceStatus"), _T(""), response));
    EXPECT_EQ(response, string("{\"status\":true,\"success\":true}"));


}
TEST_F(HdmiCecSourceInitializedTest, getActiveSourceStatusFalse)
{
    //ActiveSource is a local variable, no mocked functions to check.
    //Active source is false by default.
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getActiveSourceStatus"), _T(""), response));
    EXPECT_EQ(response, string("{\"status\":false,\"success\":true}"));
}


TEST_F(HdmiCecSourceInitializedTest, getDeviceList)
{
    int iCounter = 0;
    //Checking to see if one of the values has been filled in (as the rest get filled in at the same time, and waiting if its not.
    while ((!Plugin::HdmiCecSource::_instance->deviceList[0].m_isOSDNameUpdated) && (iCounter < (2*10))) { //sleep for 2sec.
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

    Plugin::HdmiCecSourceProcessor proc(Connection::getInstance());

    proc.process(osdName, header); //calls the process that sets osdName for LogicalAddress = 1
    proc.process(vendorid, header); //calls the process that sets vendorID for LogicalAddress = 1

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceList"), _T(""), response));

    EXPECT_EQ(response, string(_T("{\"numberofdevices\":14,\"deviceList\":[{\"logicalAddress\":1,\"osdName\":\"TEST\",\"vendorID\":\"123\"},{\"logicalAddress\":2,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":3,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":4,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":5,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":6,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":7,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":8,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":9,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":10,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":11,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":12,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":13,\"osdName\":\"NA\",\"vendorID\":\"000\"},{\"logicalAddress\":14,\"osdName\":\"NA\",\"vendorID\":\"000\"}],\"success\":true}")));


}


TEST_F(HdmiCecSourceInitializedTest, getOTPEnabled)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setOTPEnabled"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getOTPEnabled"), _T("{}"), response));
        EXPECT_EQ(response, string("{\"enabled\":true,\"success\":true}"));

}



TEST_F(HdmiCecSourceInitializedTest, sendStandbyMessage)
{

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendStandbyMessage"), _T("{}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(HdmiCecSourceInitializedTest, setOSDName)
{

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setOSDName"), _T("{\"name\": \"Sky Tv\"}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getOSDName"), _T("{}"), response));
        EXPECT_EQ(response, string("{\"name\":\"Sky Tv\",\"success\":true}"));



}
TEST_F(HdmiCecSourceInitializedTest, setVendorId)
{

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setVendorId"), _T("{\"vendorid\": \"0x0019FB\"}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getVendorId"), _T("{}"), response));
        EXPECT_EQ(response, string("{\"vendorid\":\"019fb\",\"success\":true}"));


}
TEST_F(HdmiCecSourceInitializedTest, setOTPEnabled)
{

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setOTPEnabled"), _T("{\"enabled\": false}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

}

 

TEST_F(HdmiCecSourceInitializedTest, sendKeyPressEventUp)
{
    ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
            [](const UserControlPressed& m) -> CECFrame&  {
                EXPECT_EQ(m.uiCommand.toInt(),UICommand::UI_COMMAND_VOLUME_UP );
                return CECFrame::getInstance();
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 65}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));
}
TEST_F(HdmiCecSourceInitializedTest, sendKeyPressEvent2)
{
    ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
            [](const UserControlPressed& m) -> CECFrame&  {
                EXPECT_EQ(m.uiCommand.toInt(),UICommand::UI_COMMAND_VOLUME_DOWN );
                return CECFrame::getInstance();
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 66}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

}
TEST_F(HdmiCecSourceInitializedTest, sendKeyPressEvent3)
{
    ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
            [](const UserControlPressed& m) -> CECFrame&  {
                EXPECT_EQ(m.uiCommand.toInt(),UICommand::UI_COMMAND_MUTE );
                return CECFrame::getInstance();
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 67}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));
}
TEST_F(HdmiCecSourceInitializedTest, sendKeyPressEvent4)
{
    ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
            [](const UserControlPressed& m) -> CECFrame&  {
                EXPECT_EQ(m.uiCommand.toInt(),UICommand::UI_COMMAND_UP );
                return CECFrame::getInstance();
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 1}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));
}
TEST_F(HdmiCecSourceInitializedTest, sendKeyPressEvent5)
{
    ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
            [](const UserControlPressed& m) -> CECFrame&  {
                EXPECT_EQ(m.uiCommand.toInt(),UICommand::UI_COMMAND_DOWN );
                return CECFrame::getInstance();
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 2}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

}
TEST_F(HdmiCecSourceInitializedTest, sendKeyPressEvent6)
{
    ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
            [](const UserControlPressed& m) -> CECFrame&  {
                EXPECT_EQ(m.uiCommand.toInt(),UICommand::UI_COMMAND_LEFT );
                return CECFrame::getInstance();
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 3}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));
}
TEST_F(HdmiCecSourceInitializedTest, sendKeyPressEvent7)
{
    ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
            [](const UserControlPressed& m) -> CECFrame&  {
                EXPECT_EQ(m.uiCommand.toInt(),UICommand::UI_COMMAND_RIGHT );
                return CECFrame::getInstance();
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 4}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

}
TEST_F(HdmiCecSourceInitializedTest, sendKeyPressEvent8)
{
    ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
            [](const UserControlPressed& m) -> CECFrame&  {
                EXPECT_EQ(m.uiCommand.toInt(),UICommand::UI_COMMAND_SELECT );
                return CECFrame::getInstance();
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 0}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));
}
TEST_F(HdmiCecSourceInitializedTest, sendKeyPressEvent9)
{
    ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
            [](const UserControlPressed& m) -> CECFrame&  {
                EXPECT_EQ(m.uiCommand.toInt(),UICommand::UI_COMMAND_HOME );
                return CECFrame::getInstance();
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 9}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

}
TEST_F(HdmiCecSourceInitializedTest, sendKeyPressEvent10)
{
    ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
            [](const UserControlPressed& m) -> CECFrame&  {
                EXPECT_EQ(m.uiCommand.toInt(),UICommand::UI_COMMAND_BACK );
                return CECFrame::getInstance();
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 13}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

}
TEST_F(HdmiCecSourceInitializedTest, sendKeyPressEvent11)
{
    ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
            [](const UserControlPressed& m) -> CECFrame&  {
                EXPECT_EQ(m.uiCommand.toInt(),UICommand::UI_COMMAND_NUM_0 );
                return CECFrame::getInstance();
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 32}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

}
TEST_F(HdmiCecSourceInitializedTest, sendKeyPressEvent12)
{
    ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
            [](const UserControlPressed& m) -> CECFrame&  {
                EXPECT_EQ(m.uiCommand.toInt(),UICommand::UI_COMMAND_NUM_1 );
                return CECFrame::getInstance();
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 33}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

}
TEST_F(HdmiCecSourceInitializedTest, sendKeyPressEvent13)
{
    ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
            [](const UserControlPressed& m) -> CECFrame&  {
                EXPECT_EQ(m.uiCommand.toInt(),UICommand::UI_COMMAND_NUM_2 );
                return CECFrame::getInstance();
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 34}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

}
TEST_F(HdmiCecSourceInitializedTest, sendKeyPressEvent14)
{
    ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
            [](const UserControlPressed& m) -> CECFrame&  {
                EXPECT_EQ(m.uiCommand.toInt(),UICommand::UI_COMMAND_NUM_3 );
                return CECFrame::getInstance();
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 35}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

}
TEST_F(HdmiCecSourceInitializedTest, sendKeyPressEvent15)
{
    ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
            [](const UserControlPressed& m) -> CECFrame&  {
                EXPECT_EQ(m.uiCommand.toInt(),UICommand::UI_COMMAND_NUM_4 );
                return CECFrame::getInstance();
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 36}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

}
TEST_F(HdmiCecSourceInitializedTest, sendKeyPressEvent16)
{
    ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
            [](const UserControlPressed& m) -> CECFrame&  {
                EXPECT_EQ(m.uiCommand.toInt(),UICommand::UI_COMMAND_NUM_5 );
                return CECFrame::getInstance();
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 37}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

}
TEST_F(HdmiCecSourceInitializedTest, sendKeyPressEvent17)
{
    ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
            [](const UserControlPressed& m) -> CECFrame&  {
                EXPECT_EQ(m.uiCommand.toInt(),UICommand::UI_COMMAND_NUM_6 );
                return CECFrame::getInstance();
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 38}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

}
TEST_F(HdmiCecSourceInitializedTest, sendKeyPressEvent18)
{
    ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
            [](const UserControlPressed& m) -> CECFrame&  {
                EXPECT_EQ(m.uiCommand.toInt(),UICommand::UI_COMMAND_NUM_7 );
                return CECFrame::getInstance();
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 39}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

}
TEST_F(HdmiCecSourceInitializedTest, sendKeyPressEvent19)
{
    ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
            [](const UserControlPressed& m) -> CECFrame&  {
                EXPECT_EQ(m.uiCommand.toInt(),UICommand::UI_COMMAND_NUM_8 );
                return CECFrame::getInstance();
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 40}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

}
TEST_F(HdmiCecSourceInitializedTest, sendKeyPressEvent20)
{
    ON_CALL(*p_messageEncoderMock, encode(::testing::Matcher<const UserControlPressed&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
            [](const UserControlPressed& m) -> CECFrame&  {
                EXPECT_EQ(m.uiCommand.toInt(),UICommand::UI_COMMAND_NUM_9 );
                return CECFrame::getInstance();
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendKeyPressEvent"), _T("{\"logicalAddress\": 0, \"keyCode\": 41}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

}

