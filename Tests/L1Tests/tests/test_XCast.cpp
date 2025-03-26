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
#if 0
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
#include "RtArrayObjectMock.h"
#include "ServiceMock.h"
#include "DispatcherMock.h"

//Required, xdialCastObj cannot be moved into the .h, as we don't
//want header includes to have access to the object
#include "RtXcastConnector.cpp"

using namespace WPEFramework;

using ::testing::NiceMock;
using ::testing::Eq;

class XCastPreLoad : public ::testing::Test {
    protected:
    RfcApiImplMock   *p_rfcApiImplMock = nullptr ;

    XCastPreLoad()
    {
        p_rfcApiImplMock  = new NiceMock <RfcApiImplMock>;
        RfcApi::setImpl(p_rfcApiImplMock);

        ON_CALL(*p_rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
                EXPECT_EQ(string(pcCallerID), string("Xcast"));
                EXPECT_EQ(string(pcParameterName), string("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.XDial.Enable"));
                strncpy(pstParamData->value, "true", sizeof(pstParamData->value));
                pstParamData->type = WDMP_BOOLEAN;
                return WDMP_SUCCESS;
            }));
    }

    virtual ~XCastPreLoad() override
    {
        RfcApi::setImpl(nullptr);
        if (p_rfcApiImplMock != nullptr)
        {
            delete p_rfcApiImplMock;
            p_rfcApiImplMock = nullptr;
        }
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
    rtObjectBaseMock   *p_rtBaseMock = nullptr ;

    XCastDsTest()
        : XCastTest()
    {
        rtObjectBase::setImpl(p_rtBaseMock);
    }
    virtual ~XCastDsTest() override
    {
        rtObjectBase::setImpl(nullptr);
        if (p_rtBaseMock != nullptr)
        {
            delete p_rtBaseMock;
            p_rtBaseMock = nullptr;
        }
    }
};

class XCastInitializedTest : public XCastTest {
protected:
    IarmBusImplMock   *p_iarmBusImplMock = nullptr ;
    IARM_EventHandler_t dsHdmiEventHandler;
    IARM_EventHandler_t dsHdmiStatusEventHandler;
    IARM_EventHandler_t dsHdmiSignalStatusEventHandler;
    IARM_EventHandler_t dsHdmiVideoModeEventHandler;
    IARM_EventHandler_t dsHdmiGameFeatureStatusEventHandler;

    floatingRtFunctionsMock   *p_rtFloatingMock = nullptr ;
    rtObjectBaseMock   *p_rtBaseMock = nullptr ;
    rtObjectRefMock    *p_rtRefMock = nullptr ;
    rtArrayObjectMock    *p_rtArrayMock = nullptr ;

    NiceMock<ServiceMock> service;
    XCastInitializedTest()
        : XCastTest()
    {
        p_iarmBusImplMock  = new NiceMock <IarmBusImplMock>;
        IarmBus::setImpl(p_iarmBusImplMock);

        p_rtBaseMock  = new NiceMock <rtObjectBaseMock>;
        rtObjectBase::setImpl(p_rtBaseMock);

        p_rtRefMock  = new NiceMock <rtObjectRefMock>;
        rtObjectRef::setImpl(p_rtRefMock);

        p_rtFloatingMock  = new NiceMock <floatingRtFunctionsMock>;
        floatingRtFunctions::setImpl(p_rtFloatingMock);

        p_rtArrayMock  = new NiceMock <rtArrayObjectMock>;
        rtArrayObject::setImpl(p_rtArrayMock);

        EXPECT_CALL(service, QueryInterfaceByCallsign(::testing::_, ::testing::_))
            .Times(::testing::AnyNumber())
            .WillRepeatedly(::testing::Invoke(
                [&](const uint32_t, const string& name) -> void* {
                    return nullptr;
                }));

        ON_CALL(*p_rtFloatingMock, rtRemoteLocateObject(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](rtRemoteEnvironment *env, const char* str, rtObjectRef& obj, int x, remoteDisconnectCallback back, void *cbdata=NULL) {
                    rtIObject rtIObj;
                    obj = &rtIObj;
                    return RT_OK;
                }));

        ON_CALL(*p_rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
                    EXPECT_EQ(string(pcCallerID), string("Xcast"));
                    EXPECT_EQ(string(pcParameterName), string("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.XDial.DynamicAppList"));
                    strncpy(pstParamData->value, "true", sizeof(pstParamData->value));
                    pstParamData->type = WDMP_BOOLEAN;
                    return WDMP_SUCCESS;
                }));


        ON_CALL(*p_iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    /*if ((string(IARM_BUS_PWRMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_PWRMGR_API_GetPowerState)) {
                        EXPECT_TRUE(handler != nullptr);
                        dsHdmiEventHandler = handler;
                    }*/
                    return IARM_RESULT_SUCCESS;
                }));
        ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto* param = static_cast<IARM_Bus_PWRMgr_GetPowerState_Param_t*>(arg);
                param->curState = IARM_BUS_PWRMGR_POWERSTATE_ON;
                return IARM_RESULT_SUCCESS;
            });


        ON_CALL(*p_rtRefMock, send(::testing::_, ::testing::_,::testing::_))
            .WillByDefault(
            [](const char* messageName, const char* method, rtFunctionCallback* callback) {
                delete callback;
                callback = nullptr;
                return RT_OK;
            }); 

        EXPECT_EQ(string(""), plugin->Initialize(&service));

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
        plugin->Deinitialize(&service);

        floatingRtFunctions::setImpl(nullptr);
        if (p_rtFloatingMock != nullptr)
        {
            delete p_rtFloatingMock;
            p_rtFloatingMock = nullptr;
        }

        IarmBus::setImpl(nullptr);
        if (p_iarmBusImplMock != nullptr)
        {
            delete p_iarmBusImplMock;
            p_iarmBusImplMock = nullptr;
        }

        rtObjectBase::setImpl(nullptr);
        if (p_rtBaseMock != nullptr)
        {
            delete p_rtBaseMock;
            p_rtBaseMock = nullptr;
        }

        rtObjectRef::setImpl(nullptr);
        if (p_rtRefMock != nullptr)
        {
            delete p_rtRefMock;
            p_rtRefMock = nullptr;
        }

        rtArrayObject::setImpl(nullptr);
        if (p_rtArrayMock != nullptr)
        {
            delete p_rtArrayMock;
            p_rtArrayMock = nullptr;
        }
    }
};


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

TEST_F(XCastDsTest, getsetManufacturerName)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setManufacturerName"), _T("{\"manufacturer\": \"manufacturerTest\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getManufacturerName"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"manufacturer\":\"manufacturerTest\",\"success\":true}"));
}

TEST_F(XCastDsTest, getsetModelName)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setModelName"), _T("{\"model\": \"modelTest\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getModelName"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"model\":\"modelTest\",\"success\":true}"));
}

TEST_F(XCastInitializedTest, onApplicationStateChanged)
{
    EXPECT_CALL(*p_rtBaseMock, set(::testing::_, ::testing::Matcher<const char*>(::testing::_)))
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
    ON_CALL(*p_rtRefMock, send(::testing::_, ::testing::Matcher<rtObjectRef&>(::testing::_)))
            .WillByDefault(::testing::Invoke(
                [](const char* messageName, rtObjectRef& ref) {
                    delete ref.mRef;
                    return RT_OK;
                }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("onApplicationStateChanged"), _T("{\"applicationName\": \"NetflixApp\", \"state\":\"running\", \"applicationId\": \"1234\", \"error\": \"\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

}

TEST_F(XCastInitializedTest, getProtocolVersionInitialized)
{
    ON_CALL(*p_rtBaseMock, sendReturns(::testing::_, ::testing::_))
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

TEST_F(XCastInitializedTest, unRegisterAllApplications)
{

    EXPECT_CALL(*p_rtArrayMock, pushBack(::testing::Matcher<const char*>(::testing::_)))
            .WillOnce(::testing::Invoke(
                [](const char* value) {
                    std::string strValue = value;
                    EXPECT_EQ("Youtube", strValue);
                }))
            .WillOnce(::testing::Invoke(
                [](const char* value) {
                    std::string strValue = value;
                    EXPECT_EQ("myYouTube", strValue);
                }))
            .WillOnce(::testing::Invoke(
                [](const char* value) {
                    std::string strValue = value;
                    EXPECT_EQ("youtube.com", strValue);
                }))
            .WillOnce(::testing::Invoke(
                [](const char* value) {
                    std::string strValue = value;
                    EXPECT_EQ("Netflix", strValue);
                }))
            .WillOnce(::testing::Invoke(
                [](const char* value) {
                    std::string strValue = value;
                    EXPECT_EQ("myNetflix", strValue);
                }))
            .WillOnce(::testing::Invoke(
                [](const char* value) {
                    std::string strValue = value;
                    EXPECT_EQ("netflix.com", strValue);
                }))
            //First six are from the registerApplications call
            .WillOnce(::testing::Invoke(
                [](const char* value) {
                    std::string strValue = value;
                    EXPECT_EQ("Netflix", strValue);
                }))
            .WillOnce(::testing::Invoke(
                [](const char* value) {
                    std::string strValue = value;
                    EXPECT_EQ("myNetflix", strValue);
                }))
            .WillOnce(::testing::Invoke(
                [](const char* value) {
                    std::string strValue = value;
                    EXPECT_EQ("netflix.com", strValue);
                }));
            //Last three are the ones remaining afte youtube has been unregistered.
    EXPECT_CALL(*p_rtBaseMock, set(::testing::_, ::testing::Matcher<bool>(::testing::_)))
            .WillOnce(::testing::Invoke(
                [](const char* name, bool value) {
                    bool testBool = true;
                    EXPECT_EQ(testBool, value);

                    return RT_OK;
                }))
            .WillOnce(::testing::Invoke(
                [](const char* name, bool value) {
                    bool testBool = true;
                    EXPECT_EQ(testBool, value);

                    return RT_OK;
                }))
                //First two are form register Applications call.
                .WillOnce(::testing::Invoke(
                [](const char* name, bool value) {
                    bool testBool = true;
                    EXPECT_EQ(testBool, value);

                    return RT_OK;
                }));
                //Last is from the remaining once Youtube has been unregistered.
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("registerApplications"), _T("{\"applications\": [{\"names\": [\"Youtube\"], \"prefixes\": [\"myYouTube\"], \"cors\": [\"youtube.com\"], \"properties\": {\"allowStop\": true}},{\"names\": [\"Netflix\"], \"prefixes\": [\"myNetflix\"], \"cors\": [\"netflix.com\"], \"properties\": {\"allowStop\": true}, \"launchParameters\": {\"query\":\"testquery\", \"payload\":\"testpayload\"}}]}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("unregisterApplications"), _T("{\"applications\": [\"Youtube\"]}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(XCastInitializedTest, registerApplications)
{

    EXPECT_CALL(*p_rtArrayMock, pushBack(::testing::Matcher<const char*>(::testing::_)))
            .WillOnce(::testing::Invoke(
                [](const char* value) {
                    std::string strValue = value;
                    EXPECT_EQ("Youtube", strValue);
                }))
            .WillOnce(::testing::Invoke(
                [](const char* value) {
                    std::string strValue = value;
                    EXPECT_EQ("myYouTube", strValue);
                }))
            .WillOnce(::testing::Invoke(
                [](const char* value) {
                    std::string strValue = value;
                    EXPECT_EQ("youtube.com", strValue);
                }))
            .WillOnce(::testing::Invoke(
                [](const char* value) {
                    std::string strValue = value;
                    EXPECT_EQ("Netflix", strValue);
                }))
            .WillOnce(::testing::Invoke(
                [](const char* value) {
                    std::string strValue = value;
                    EXPECT_EQ("myNetflix", strValue);
                }))
            .WillOnce(::testing::Invoke(
                [](const char* value) {
                    std::string strValue = value;
                    EXPECT_EQ("netflix.com", strValue);
                }));
            //Last three are the ones remaining afte youtube has been unregistered.
    EXPECT_CALL(*p_rtBaseMock, set(::testing::_, ::testing::Matcher<bool>(::testing::_)))
            .WillOnce(::testing::Invoke(
                [](const char* name, bool value) {
                    bool testBool = true;
                    EXPECT_EQ(testBool, value);

                    return RT_OK;
                }))
            .WillOnce(::testing::Invoke(
                [](const char* name, bool value) {
                    bool testBool = true;
                    EXPECT_EQ(testBool, value);

                    return RT_OK;
                }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("registerApplications"), _T("{\"applications\": [{\"names\": [\"Youtube\"], \"prefixes\": [\"myYouTube\"], \"cors\": [\"youtube.com\"], \"properties\": {\"allowStop\": true}},{\"names\": [\"Netflix\"], \"prefixes\": [\"myNetflix\"], \"cors\": [\"netflix.com\"], \"properties\": {\"allowStop\": true}, \"launchParameters\": {\"query\":\"testquery\", \"payload\":\"testpayload\"}}]}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("unregisterApplications"), _T("{\"applications\": []}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}


TEST_F(XCastInitializedEventTest, onApplicationHideRequest)
{
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.onApplicationHideRequest\",\"params\":{\"applicationName\":\"NetflixApp\",\"applicationId\":\"1234\"}}")));
                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onApplicationHideRequest"), _T("client.events"), message);
    plugin->onXcastApplicationHideRequest("Netflix", "1234");
    handler.Unsubscribe(0, _T("onApplicationHideRequest"), _T("client.events"), message);
}
TEST_F(XCastInitializedEventTest, onApplicationStateRequest)
{
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.onApplicationStateRequest\",\"params\":{\"applicationName\":\"NetflixApp\",\"applicationId\":\"1234\"}}")));
                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onApplicationStateRequest"), _T("client.events"), message);
    plugin->onXcastApplicationStateRequest("Netflix", "1234");
    handler.Unsubscribe(0, _T("onApplicationStateRequest"), _T("client.events"), message);
}
TEST_F(XCastInitializedEventTest, onApplicationLaunchRequest)
{
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.onApplicationLaunchRequest\",\"params\":{\"applicationName\":\"Netflix\",\"parameters\":{\"url\":\"1234\"}}}")));
                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onApplicationLaunchRequest"), _T("client.events"), message);
    plugin->onXcastApplicationLaunchRequest("Netflix", "1234");
    handler.Unsubscribe(0, _T("onApplicationLaunchRequest"), _T("client.events"), message);
}
TEST_F(XCastInitializedEventTest, onApplicationResumeRequest)
{
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.onApplicationResumeRequest\",\"params\":{\"applicationName\":\"NetflixApp\",\"applicationId\":\"1234\"}}")));
                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onApplicationResumeRequest"), _T("client.events"), message);
    plugin->onXcastApplicationResumeRequest("Netflix", "1234");
    handler.Unsubscribe(0, _T("onApplicationResumeRequest"), _T("client.events"), message);
}
TEST_F(XCastInitializedEventTest, onApplicationStopRequest)
{
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"client.events.onApplicationStopRequest\",\"params\":{\"applicationName\":\"Netflix\",\"applicationId\":\"1234\"}}")));
                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onApplicationStopRequest"), _T("client.events"), message);
    plugin->onXcastApplicationStopRequest("Netflix", "1234");
    handler.Unsubscribe(0, _T("onApplicationStopRequest"), _T("client.events"), message);
}
#endif
