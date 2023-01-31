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

///using namespace std;
using namespace WPEFramework;
using ::testing::NiceMock;

class HdmiCecSinkTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::HdmiCecSink> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;
    NiceMock<IarmBusImplMock> iarmBusImplMock;

    HdmiCecSinkTest()
        : plugin(Core::ProxyType<Plugin::HdmiCecSink>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;
    std::cout<<"RemoteActionMapping getKeymap call - Success!\n";    
    std::cout << "Hello World!\n";
        #if 1
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
#endif
        std::cout << "calling initialise iarm\n";
        EXPECT_EQ(string(""), plugin->Initialize(nullptr));
        std::cout << "plugin->Initialize Done\n";
    }
    //virtual ~HdmiCecSinkTest() = default;
#if 1
    virtual ~HdmiCecSinkTest() override
    {
       // sleep(3);
    //EXPECT_EQ(string(""), plugin->Deinitialize(nullptr));    
    plugin->Deinitialize(nullptr);
        std::cout << "plugin->destructor \n";
       IarmBus::getInstance().impl = nullptr;
        std::cout << "plugin->destructor done\n";
    }
#endif   
};
class HdmiCecSinkDsTest : public HdmiCecSinkTest {
protected:
    LibCCECImplMock libCCECImplMock;
    
    HdmiCecSinkDsTest()
        : HdmiCecSinkTest()
    {
        std::cout << "plugin->destructor -1 \n";
        LibCCEC::getInstance().impl = &libCCECImplMock;
    }
    virtual ~HdmiCecSinkDsTest() override
    {
        LibCCEC::getInstance().impl = nullptr;
    }
};

class HdmiCecSinkInitializedTest : public HdmiCecSinkTest {
protected:
    IARM_EventHandler_t dsHdmiCecSinkEventHandler;
    IARM_EventHandler_t dsHdmiEventHandler;
 
    HdmiCecSinkInitializedTest()
        : HdmiCecSinkTest()
    {

        std::cout << "plugin->destructor -2 \n";
        #if 1
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
        #endif
    }
    virtual ~HdmiCecSinkInitializedTest() override
    {
		
	sleep(2);

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
        std::cout << "plugin->destructor -3\n";
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
    DeviceTypeMock deviceTypeImplMock;
    LogicalAddressImplMock logicalAddressImplMock;

    HdmiCecSinkInitializedEventDsTest()
        : HdmiCecSinkInitializedEventTest()
    {
        std::cout << "plugin->destructor -4\n";
	    LibCCEC::getInstance().impl = &libCCECImplMock;
        Connection::getInstance().impl = &connectionImplMock;	
	    DeviceType::getInstance().impl = &deviceTypeImplMock;
        LogicalAddress::getInstance().impl = &logicalAddressImplMock;

    }

    virtual ~HdmiCecSinkInitializedEventDsTest() override
    {
        LibCCEC::getInstance().impl = nullptr;

    }
};


TEST_F(HdmiCecSinkTest, RegisteredMethods)
{

    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getEnabled")));
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
#if 0
TEST_F(HdmiCecSinkInitializedEventDsTest, setEnabled)
{
std::cout << "plugin->destructor -8\n";
    //setting HdmiCec to enabled.
    ON_CALL(libCCECImplMock, getLogicalAddress(::testing::_))
        .WillByDefault(::testing::Return(1));
	ON_CALL(deviceTypeImplMock, toString())
        .WillByDefault(::testing::Return("New"));
    EXPECT_CALL(connectionImplMock, open())
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&]() {

            }));
    EXPECT_CALL(connectionImplMock, addFrameListener(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](FrameListener *listener) {
                //FrameListener only contains a destructor, and a void function. No values to check
            }));
    EXPECT_CALL(connectionImplMock, poll(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&]() {

            }));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

    //Turning off HdmiCec. otherwise we get segementation faults as things memory early while threads are still running
    sleep(5);//short wait to allow setEnabled to reach thread loop, where it can exit safely without segmentation faults

    EXPECT_CALL(connectionImplMock, close())
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&]() {

            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": false}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}
#endif
TEST_F(HdmiCecSinkDsTest, getEnabledFalse)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getEnabled"), _T(""), response));
    EXPECT_EQ(response, string("{\"enabled\":false,\"success\":true}"));
}

#if 0
TEST_F(HdmiCecSinkInitializedEventDsTest, getEnabledTrue)
{
    //setting HdmiCec to enabled.
    ON_CALL(libCCECImplMock, getLogicalAddress(::testing::_))
        .WillByDefault(::testing::Return(1));
	ON_CALL(deviceTypeImplMock, toString())
        .WillByDefault(::testing::Return("New"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));

    //Get enabled just checks if CEC is on, which is a global variable.
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getEnabled"), _T(""), response));
    EXPECT_EQ(response, string("{\"enabled\":true,\"success\":true}"));



    //Turning off HdmiCec. otherwise we get segementation faults as things memory early while threads are still running
    sleep(5);//short wait to allow setEnabled to reach thread loop, where it can exit safely without segmentation faults
    ON_CALL(connectionImplMock, close())
        .WillByDefault(::testing::Return());
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": false}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(HdmiCecSinkInitializedEventDsTest, onDevicesChanged)
{
 //setting HdmiCec to enabled.
    ON_CALL(libCCECImplMock, getLogicalAddress(::testing::_))
        .WillByDefault(::testing::Return(1));
	ON_CALL(deviceTypeImplMock, toString())
        .WillByDefault(::testing::Return("New"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));


    ASSERT_TRUE(dsHdmiEventHandler != nullptr);


    IARM_Bus_DSMgr_EventData_t eventData;
    eventData.data.hdmi_in_connect.port =dsHDMI_IN_PORT_0;
    eventData.data.hdmi_in_connect.isPortConnected = true;

    handler.Subscribe(0, _T("onDeviceAdded"), _T("client.events.onDeviceAdded"), message);

    dsHdmiEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG, &eventData , 0);

    handler.Unsubscribe(0, _T("onDeviceAdded"), _T("client.events.onDeviceAdded"), message);

    //Turning off HdmiCec. otherwise we get segementation faults as things memory early while threads are still running
    sleep(1);//short wait to allow setEnabled to reach thread loop, where it can exit safely without segmentation faults
    ON_CALL(connectionImplMock, close())
        .WillByDefault(::testing::Return());
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": false}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));


}

TEST_F(HdmiCecSinkInitializedEventDsTest, getDeviceList)
{

     //setting HdmiCec to enabled.
    ON_CALL(libCCECImplMock, getLogicalAddress(::testing::_))
        .WillByDefault(::testing::Return(1));
	ON_CALL(deviceTypeImplMock, toString())
        .WillByDefault(::testing::Return("New"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));
    sleep(1); //Allow the thread that populates deviceList to actually populate before we run getDeviceList.

    //Calling the device list, which is a defualt list of the hdmiCec class. Kist grabs the deviceList.
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceList"), _T(""), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T(".*[({\"logicalAddress\":[0-9]*,\"osdName\":\"[a-zA-Z0-9 ]*\",\"vendorID\":\"[a-zA-Z0-9 ]*\"})*.*")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T(".*\"numberofdevices\":[0-9]*,\"deviceList\":.*")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T(".*\"success\":true.*")));


    //Turning off HdmiCec. otherwise we get segementation faults as things memory early while threads are still running
    sleep(1);//short wait to allow setEnabled to reach thread loop, where it can exit safely without segmentation faults
    ON_CALL(connectionImplMock, close())
        .WillByDefault(::testing::Return());
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": false}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

}

#endif
TEST_F(HdmiCecSinkDsTest, setOSDName)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setOSDName"), _T("{\"name\":\"CECTEST\"}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));

}

TEST_F(HdmiCecSinkDsTest, setOSDNameInvalid)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setOSDName"), _T("{\"name\":""}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));

}


TEST_F(HdmiCecSinkDsTest, getOSDName)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getOSDName"), _T("{}"), response));
    EXPECT_EQ(response,  string(""));

}


TEST_F(HdmiCecSinkDsTest, setVendorId)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setVendorId"), _T("{\"vendorid\":\"0x0019FB\"}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));

}

TEST_F(HdmiCecSinkDsTest, setVendorIdInvalid)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setVendorId"), _T("{\"vendorid\":""}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));

}

TEST_F(HdmiCecSinkDsTest, getVendorId)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getVendorId"), _T("{}"), response));
    EXPECT_EQ(response,  string(""));

}

TEST_F(HdmiCecSinkDsTest, setActivePath)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setActivePath"), _T("{\"activePath\":\"path1\"}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));

}

TEST_F(HdmiCecSinkDsTest, setActivePathInvalid)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setActivePath"), _T("{\"activePath\":""}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));

}

TEST_F(HdmiCecSinkDsTest, setRoutingChange)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setRoutingChange"), _T("{\"oldPort\":[{\"HDMI\":\"100\", \"TV\":\"150\"}], \"newPort\":[{\"HDMI\":\"100\", \"TV\":\"150\"}}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));

}

TEST_F(HdmiCecSinkDsTest, getDeviceList)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceList"), _T(""), response));
    EXPECT_EQ(response,  string(""));

}

TEST_F(HdmiCecSinkDsTest, getActiveSource)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getActiveSource"), _T(""), response));
    EXPECT_EQ(response,  string(""));
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

TEST_F(HdmiCecSinkDsTest, setMenuLanguageInvalid)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setMenuLanguage"), _T(""), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(HdmiCecSinkDsTest, requestActiveSource)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("requestActiveSource"), _T(""), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(HdmiCecSinkDsTest, setupARCRouting)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setupARCRouting"), _T(""), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(HdmiCecSinkDsTest, requestShortAudioDescriptor)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("requestShortAudioDescriptor"), _T(""), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
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

TEST_F(HdmiCecSinkDsTest, sendGetAudioStatusMessage)
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
