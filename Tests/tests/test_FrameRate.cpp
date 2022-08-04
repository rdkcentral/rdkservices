#include <gtest/gtest.h>

#include "FrameRate.h"

#include "FactoriesImplementation.h"
#include "HostMock.h"
#include "IarmBusMock.h"
#include "ServiceMock.h"
#include "VideoDeviceMock.h"
#include "dsMgr.h"

using namespace WPEFramework;

namespace {
const string iarmName = _T("Thunder_Plugins");
}

class FrameRateTestFixture : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::FrameRate> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Handler& handlerV2;
    Core::JSONRPC::Connection connection;
    IarmBusImplMock iarmBusImplMock;
    HostImplMock hostImplMock;
    VideoDeviceMock videoDeviceMock;
    IARM_EventHandler_t handlerOnDisplayFrameRateChanging;
    IARM_EventHandler_t handlerOnDisplayFrameRateChanged;
    int frfmode;
    string framerate;
    string response;
    ServiceMock service;
    Core::JSONRPC::Message message;
    FactoriesImplementation factoriesImplementation;

    FrameRateTestFixture()
        : plugin(Core::ProxyType<Plugin::FrameRate>::Create())
        , handler(*(plugin))
        , handlerV2(*(plugin->GetHandler(2)))
        , connection(1, 0)
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);
    }
    virtual ~FrameRateTestFixture()
    {
        PluginHost::IFactories::Assign(nullptr);
    }

    virtual void SetUp()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;
        device::Host::getInstance().impl = &hostImplMock;
    }

    virtual void TearDown()
    {
        IarmBus::getInstance().impl = nullptr;
        device::Host::getInstance().impl = nullptr;
    }
};

TEST_F(FrameRateTestFixture, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setCollectionFrequency")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("startFpsCollection")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("stopFpsCollection")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("updateFps")));

    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("setCollectionFrequency")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("startFpsCollection")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("stopFpsCollection")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("updateFps")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("setFrmMode")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("getFrmMode")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("getDisplayFrameRate")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("setDisplayFrameRate")));
}

TEST_F(FrameRateTestFixture, Plugin)
{
    // IARM expectations

    // called by FrameRate::InitializeIARM, FrameRate::DeinitializeIARM
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_IsConnected(::testing::_, ::testing::_))
        .Times(3)
        .WillOnce(::testing::Invoke(
            [](const char* memberName, int* isRegistered) {
                if (iarmName == string(memberName)) {
                    *isRegistered = 0;
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            }))
        .WillOnce(::testing::Invoke(
            [](const char* memberName, int* isRegistered) {
                if (iarmName == string(memberName)) {
                    *isRegistered = 1;
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            }))
        .WillOnce(::testing::Invoke(
            [](const char* memberName, int* isRegistered) {
                if (iarmName == string(memberName)) {
                    *isRegistered = 1;
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            }));

    // called by FrameRate::InitializeIARM
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Init(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](const char* name) {
                if (iarmName == string(name)) {
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            }));

    // called by FrameRate::InitializeIARM
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Connect())
        .Times(1)
        .WillOnce(::testing::Return(IARM_RESULT_SUCCESS));

    // called by FrameRate::InitializeIARM
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
        .Times(2)
        .WillOnce(::testing::Invoke(
            [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_PRECHANGE)) {
                    handlerOnDisplayFrameRateChanging = handler;
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            }))
        .WillOnce(::testing::Invoke(
            [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_POSTCHANGE)) {
                    handlerOnDisplayFrameRateChanged = handler;
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            }));

    // called by FrameRate::DeinitializeIARM
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_UnRegisterEventHandler(::testing::_, ::testing::_))
        .Times(2)
        .WillOnce(::testing::Invoke(
            [&](const char* ownerName, IARM_EventId_t eventId) {
                if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_PRECHANGE)) {
                    handlerOnDisplayFrameRateChanging = nullptr;
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            }))
        .WillOnce(::testing::Invoke(
            [&](const char* ownerName, IARM_EventId_t eventId) {
                if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_POSTCHANGE)) {
                    handlerOnDisplayFrameRateChanged = nullptr;
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            }));

    // VideoDevice expectations

    EXPECT_CALL(videoDeviceMock, setFRFMode(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](int param) {
                frfmode = param;
                return 0;
            }));

    EXPECT_CALL(videoDeviceMock, getFRFMode(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](int* param) {
                *param = frfmode;
                return 0;
            }));

    EXPECT_CALL(videoDeviceMock, setDisplayframerate(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const char* param) {
                framerate = param;
                return 0;
            }));

    EXPECT_CALL(videoDeviceMock, getCurrentDisframerate(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](char* param) {
                ::memcpy(param, framerate.c_str(), framerate.length());
                return 0;
            }));

    // Host expectations

    // called by FrameRate::getDisplayFrameRate, FrameRate::setDisplayFrameRate, FrameRate::getFrmMode, FrameRate::setFrmMode
    EXPECT_CALL(hostImplMock, getVideoDevices())
        .Times(4)
        .WillRepeatedly(::testing::Invoke(
            [&]() {
                device::VideoDevice videoDevice;
                videoDevice.impl = &videoDeviceMock;
                return device::List<device::VideoDevice>({ videoDevice });
            }));

    // IShell expectations

    // called by FrameRate::frameRatePreChange, FrameRate::frameRatePostChange
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(2)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.FrameRate.onDisplayFrameRateChanging\","
                                          "\"params\":{}"
                                          "}")));

                return Core::ERROR_NONE;
            }))
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.FrameRate.onDisplayFrameRateChanged\","
                                          "\"params\":{}"
                                          "}")));

                return Core::ERROR_NONE;
            }));

    // Initialize

    EXPECT_EQ(string(""), plugin->Initialize(nullptr));

    // JSON-RPC methods

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setCollectionFrequency"), _T("{\"frequency\":1000}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startFpsCollection"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("stopFpsCollection"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("updateFps"), _T("{\"newFpsValue\":60}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("setFrmMode"), _T("{\"frmmode\":0}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("getFrmMode"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"auto-frm-mode\":0,\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("setDisplayFrameRate"), _T("{\"framerate\":\"3840x2160px48\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("getDisplayFrameRate"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"framerate\":\"3840x2160px48\",\"success\":true}"));

    // JSON-RPC events

    auto dispatcher = static_cast<PluginHost::IDispatcher*>(
        plugin->QueryInterface(PluginHost::IDispatcher::ID));
    EXPECT_TRUE(dispatcher != nullptr);

    dispatcher->Activate(&service);

    EXPECT_TRUE(handlerOnDisplayFrameRateChanging != nullptr);
    EXPECT_TRUE(handlerOnDisplayFrameRateChanged != nullptr);

    handler.Subscribe(0, _T("onDisplayFrameRateChanging"), _T("org.rdk.FrameRate"), message);
    handler.Subscribe(0, _T("onDisplayFrameRateChanged"), _T("org.rdk.FrameRate"), message);

    handlerOnDisplayFrameRateChanging(
        IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_PRECHANGE, nullptr, 0);

    handlerOnDisplayFrameRateChanged(
        IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_POSTCHANGE, nullptr, 0);

    handler.Unsubscribe(0, _T("onDisplayFrameRateChanging"), _T("org.rdk.FrameRate"), message);
    handler.Unsubscribe(0, _T("onDisplayFrameRateChanged"), _T("org.rdk.FrameRate"), message);

    dispatcher->Deactivate();

    dispatcher->Release();

    // Deinitialize

    plugin->Deinitialize(nullptr);
}
