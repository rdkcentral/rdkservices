#include <gtest/gtest.h>

#include "FrameRate.h"

#include "HostMock.h"
#include "IarmBusMock.h"
#include "VideoDeviceMock.h"
#include "dsMgr.h"

using namespace WPEFramework;

namespace {
const string iarmName = _T("Thunder_Plugins");
}

IarmBusImpl* gIarmBusImpl = nullptr;

namespace device {
HostImpl* gHostImpl = nullptr;
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

    FrameRateTestFixture()
        : plugin(Core::ProxyType<Plugin::FrameRate>::Create())
        , handler(*(plugin))
        , handlerV2(*(plugin->GetHandler(2)))
        , connection(1, 0)
    {
    }
    virtual ~FrameRateTestFixture()
    {
    }

    virtual void SetUp()
    {
        gIarmBusImpl = &iarmBusImplMock;
        device::gHostImpl = &hostImplMock;
    }

    virtual void TearDown()
    {
        gIarmBusImpl = nullptr;
        device::gHostImpl = nullptr;
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
                device::List<std::reference_wrapper<device::VideoDevice>> result{ videoDeviceMock };
                return result;
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

    EXPECT_TRUE(handlerOnDisplayFrameRateChanging != nullptr);
    EXPECT_TRUE(handlerOnDisplayFrameRateChanged != nullptr);

    // events are logged, there's no way to mock
    handlerOnDisplayFrameRateChanging(
        IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_PRECHANGE, nullptr, 0);

    handlerOnDisplayFrameRateChanged(
        IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_POSTCHANGE, nullptr, 0);

    // Deinitialize

    plugin->Deinitialize(nullptr);
}
