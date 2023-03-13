#include <gtest/gtest.h>

#include "FactoriesImplementation.h"

#include "XCast.h"
//#include "RtXcastConnector.h"
#include "RfcApiMock.h"
#include "IarmBusMock.h"
#include "ServiceMock.h"

#include "dsMgr.h"
#include "devicesettings.h"
#include "RtObjectBaseMock.h"
#include "FloatingRtFunctionsMock.h"
#include "RtObjectRefMock.h"

//Required, xdialCastObj cannot be moved into the .h, as we don't 
//want header includes to have access to the object
#include "RtXcastConnector.cpp"

using namespace WPEFramework;

using ::testing::NiceMock;
using ::testing::Eq;

class XCastPreLoad : public ::testing::Test {
    protected:
    NiceMock<RfcApiImplMock> rfcApiImplMock;

    XCastPreLoad() 
    {
        RfcApi::getInstance().impl = &rfcApiImplMock;
        ON_CALL(rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
                EXPECT_EQ(string(pcCallerID), string("Xcast"));
                EXPECT_EQ(string(pcParameterName), string("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.XDial.Enable"));
                strncpy(pstParamData->value, "true", sizeof(pstParamData->value));
                pstParamData->type = WDMP_BOOLEAN;
                return WDMP_SUCCESS;
            }));
    }

};

class XCastTest : public XCastPreLoad {
protected:
    Core::ProxyType<Plugin::XCast> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    XCastTest()
        : XCastPreLoad()
        , plugin(Core::ProxyType<Plugin::XCast>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
        

    }
    virtual ~XCastTest() = default;
};

class XCastDsTest : public XCastTest {
protected:
    NiceMock<rtObjectBaseMock> rtBaseMock;

    XCastDsTest()
        : XCastTest()
    {
        rtObjectBase::getInstance().impl = &rtBaseMock;
    }
    virtual ~XCastDsTest() override
    {
        rtObjectBase::getInstance().impl = nullptr;
    }
};

class XCastInitializedTest : public XCastTest {
protected:
    NiceMock<IarmBusImplMock> iarmBusImplMock;
    IARM_EventHandler_t dsHdmiEventHandler;
    IARM_EventHandler_t dsHdmiStatusEventHandler;
    IARM_EventHandler_t dsHdmiSignalStatusEventHandler;
    IARM_EventHandler_t dsHdmiVideoModeEventHandler;
    IARM_EventHandler_t dsHdmiGameFeatureStatusEventHandler;

    NiceMock<floatingRtFunctionsMock> rtFloatingMock;
        NiceMock<rtObjectBaseMock> rtBaseMock;
        NiceMock<rtObjectRefMock> rtRefMock;


    XCastInitializedTest()
        : XCastTest()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;
        rtObjectBase::getInstance().impl = &rtBaseMock;
        rtObjectRef::getInstance().impl = &rtRefMock;
        floatingRtFunctions::getInstance().impl = &rtFloatingMock;
        RfcApi::getInstance().impl = &rfcApiImplMock;
        
        ON_CALL(rtFloatingMock, rtRemoteLocateObject(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](rtRemoteEnvironment *env, const char* str, rtObjectRef& obj, int x, remoteDisconnectCallback back, void *cbdata=NULL) {
                    obj = new rtIObject();
                    return RT_OK;
                })); 
        
        ON_CALL(rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
                    EXPECT_EQ(string(pcCallerID), string("Xcast"));
                    EXPECT_EQ(string(pcParameterName), string("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.XDial.DynamicAppList"));
                    strncpy(pstParamData->value, "true", sizeof(pstParamData->value));
                    pstParamData->type = WDMP_BOOLEAN;
                    return WDMP_SUCCESS;
                }));
        

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

        EXPECT_EQ(string(""), plugin->Initialize(nullptr));

        //Wait until threads populate xdialCastObj.
        int iCounter = 0;
        while ((xdialCastObj == NULL) && (iCounter < (5*10))) { //sleep for 2sec.
	    	usleep (100 * 1000); //sleep for 100 milli sec
	    	iCounter ++;
	    }

        //required to enable some backend processes for certain functions.
        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enabled\": true}"), response));
        EXPECT_EQ(response, string("{\"success\":true}"));
    }
    virtual ~XCastInitializedTest() override
    {
        plugin->Deinitialize(nullptr);
        RfcApi::getInstance().impl = nullptr;
        floatingRtFunctions::getInstance().impl = nullptr;
        rtObjectBase::getInstance().impl = nullptr;
        IarmBus::getInstance().impl = nullptr;
        rtObjectRef::getInstance().impl = nullptr;
    }
};

/*
class XCastInitializedEventTest : public XCastInitializedTest {
protected:
    NiceMock<ServiceMock> service;
    Core::JSONRPC::Message message;
    NiceMock<FactoriesImplementation> factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    XCastInitializedEventTest()
        : XCastInitializedTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }

    virtual ~XCastInitializedEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();

        PluginHost::IFactories::Assign(nullptr);
    }
};

class XCastInitializedEventDsTest : public XCastInitializedEventTest {
protected:
    NiceMock<XCastImplMock> XCastImplMock;

    XCastInitializedEventDsTest()
        : XCastInitializedEventTest()
    {
        device::XCast::getInstance().impl = &XCastImplMock;
    }

    virtual ~XCastInitializedEventDsTest() override
    {
        device::XCast::getInstance().impl = nullptr;
    }
};

*/

TEST_F(XCastTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getApiVersionNumber")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("onApplicationStateChanged")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getStandbyBehavior")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setStandbyBehavior")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getFriendlyName")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setFriendlyName")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getProtocolVersion")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("unregisterApplications")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getProtocolVersion")));
}


TEST_F(XCastDsTest, getApiVersionNumber)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getApiVersionNumber"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"version\":1,\"success\":true}"));
}
TEST_F(XCastDsTest, getProtocolVersion)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getProtocolVersion"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"version\":\"2.1\",\"success\":true}"));
}

TEST_F(XCastDsTest, getsetFriendlyName)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setFriendlyName"), _T("{\"friendlyname\": \"friendlyTest\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFriendlyName"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"friendlyname\":\"friendlyTest\",\"success\":true}"));
}

TEST_F(XCastDsTest, getsetStandbyBehavoir)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setStandbyBehavior"), _T("{\"standbybehavior\": \"active\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStandbyBehavior"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"standbybehavior\":\"active\",\"success\":true}"));
}

TEST_F(XCastInitializedTest, onApplicationStateChanged)
{
    EXPECT_CALL(rtBaseMock, set(::testing::_, ::testing::Matcher<const char*>(::testing::_)))
            .WillOnce(::testing::Invoke(
                [](const char* name, const char* value) {
                    std::string cnv = value;
                    EXPECT_EQ("Netflix", cnv);
                    return RT_OK;
                }))
            .WillOnce(::testing::Invoke(
                [](const char* name, const char* value) {
                    std::string cnv = value;
                    EXPECT_EQ("1234", cnv);
                    return RT_OK;
                }))
            .WillOnce(::testing::Invoke(
                [](const char* name, const char* value) {
                    std::string cnv = value;
                    EXPECT_EQ("running", cnv);
                    return RT_OK;
                }))
            .WillOnce(::testing::Invoke(
                [](const char* name, const char* value) {
                    std::string cnv = value;
                    EXPECT_EQ("", cnv);
                    return RT_OK;
                }))
            .WillRepeatedly(::testing::Invoke(
                [](const char* name, const char* value) {
                    return RT_OK;
                })); 
    ON_CALL(rtRefMock, send(::testing::_, ::testing::_))
            .WillByDefault(::testing::Return(RT_OK)); 
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("onApplicationStateChanged"), _T("{\"applicationName\": \"NetflixApp\", \"state\":\"running\", \"applicationId\": \"1234\", \"error\": \"\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    
}

TEST_F(XCastInitializedTest, getProtocolVersionInitialized)
{
    ON_CALL(rtBaseMock, sendReturns(::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* messageName, rtString& result) {

                    std::string tmp = "test";
                    result = tmp.c_str();

                    return RT_OK;
                })); 
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getProtocolVersion"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"version\":\"test\",\"success\":true}"));
}

TEST_F(XCastInitializedTest, unregisterApplications)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("unregisterApplications"), _T("{\"applications\": [\"NetflixApp\", \"NetFlixApp2\"]}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(XCastInitializedTest, registerApplications)
{

    const rtValue val;
    EXPECT_CALL(rtBaseMock, set(::testing::_, ::testing::Matcher<const rtValue&>(::testing::_)))
            .WillOnce(::testing::Invoke(
                [](const char* name, const rtValue& value) {
                    std::string strName = name;
                    std::string strValue =value.mValue.objectValue->mElements[0];
                    EXPECT_EQ("Names", strName);
                    EXPECT_EQ("Youtube", strValue);

                    return RT_OK;
                }))
            .WillRepeatedly(::testing::Invoke(
                [](const char* name, const rtValue& value) {
                    return RT_OK;
                })); 
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("registerApplications"), _T("{\"applications\": [{\"names\": [\"Youtube\"], \"prefixes\": [\"myYouTube\"], \"cors\": [\"youtube.com\"], \"properties\": {\"allowstop\": true}}]}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}