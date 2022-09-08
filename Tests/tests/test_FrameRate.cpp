#include <gtest/gtest.h>

#include "FrameRate.h"

#include "FactoriesImplementation.h"
#include "HostMock.h"
#include "IarmBusMock.h"
#include "ServiceMock.h"
#include "VideoDeviceMock.h"
#include "dsMgr.h"

using namespace WPEFramework;

class FrameRateTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::FrameRate> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Handler& handlerV2;
    Core::JSONRPC::Connection connection;
    string response;

    FrameRateTest()
        : plugin(Core::ProxyType<Plugin::FrameRate>::Create())
        , handler(*(plugin))
        , handlerV2(*(plugin->GetHandler(2)))
        , connection(1, 0)
    {
    }
    virtual ~FrameRateTest() = default;
};

class FrameRateInitializedTest : public FrameRateTest {
protected:
    IarmBusImplMock iarmBusImplMock;
    IARM_EventHandler_t handlerOnDisplayFrameRateChanging;
    IARM_EventHandler_t handlerOnDisplayFrameRateChanged;

    FrameRateInitializedTest()
        : FrameRateTest()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;

        ON_CALL(iarmBusImplMock, IARM_Bus_IsConnected(::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [](const char*, int* isRegistered) {
                    *isRegistered = 1;
                    return IARM_RESULT_SUCCESS;
                }));
        ON_CALL(iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_PRECHANGE)) {
                        handlerOnDisplayFrameRateChanging = handler;
                    }
                    if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_POSTCHANGE)) {
                        handlerOnDisplayFrameRateChanged = handler;
                    }
                    return IARM_RESULT_SUCCESS;
                }));

        EXPECT_EQ(string(""), plugin->Initialize(nullptr));
    }
    virtual ~FrameRateInitializedTest() override
    {
        ON_CALL(iarmBusImplMock, IARM_Bus_UnRegisterEventHandler(::testing::_, ::testing::_))
            .WillByDefault(::testing::Return(IARM_RESULT_SUCCESS));

        plugin->Deinitialize(nullptr);

        IarmBus::getInstance().impl = nullptr;
    }
};

class FrameRateInitializedEventTest : public FrameRateInitializedTest {
protected:
    ServiceMock service;
    Core::JSONRPC::Message message;
    FactoriesImplementation factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    FrameRateInitializedEventTest()
        : FrameRateInitializedTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }
    virtual ~FrameRateInitializedEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();

        PluginHost::IFactories::Assign(nullptr);
    }
};

class FrameRateDsTest : public FrameRateTest {
protected:
    HostImplMock hostImplMock;
    VideoDeviceMock videoDeviceMock;
    device::VideoDevice videoDevice;

    FrameRateDsTest()
        : FrameRateTest()
    {
        device::Host::getInstance().impl = &hostImplMock;
        videoDevice.impl = &videoDeviceMock;

        ON_CALL(hostImplMock, getVideoDevices())
            .WillByDefault(::testing::Return(device::List<device::VideoDevice>({ videoDevice })));
    }
    virtual ~FrameRateDsTest() override
    {
        device::Host::getInstance().impl = nullptr;
    }
};

TEST_F(FrameRateTest, RegisteredMethods)
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

TEST_F(FrameRateTest, setCollectionFrequency_startFpsCollection_stopFpsCollection_updateFps)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setCollectionFrequency"), _T("{\"frequency\":1000}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startFpsCollection"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("stopFpsCollection"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("updateFps"), _T("{\"newFpsValue\":60}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(FrameRateDsTest, setFrmMode)
{
    EXPECT_CALL(videoDeviceMock, setFRFMode(::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](int param) {
                EXPECT_EQ(param, 0);
                return 0;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("setFrmMode"), _T("{\"frmmode\":0}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(FrameRateDsTest, getFrmMode)
{
    EXPECT_CALL(videoDeviceMock, getFRFMode(::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](int* param) {
                *param = 0;
                return 0;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("getFrmMode"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"auto-frm-mode\":0,\"success\":true}"));
}

TEST_F(FrameRateDsTest, setDisplayFrameRate)
{
    EXPECT_CALL(videoDeviceMock, setDisplayframerate(::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const char* param) {
                EXPECT_EQ(param, string("3840x2160px48"));
                return 0;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("setDisplayFrameRate"), _T("{\"framerate\":\"3840x2160px48\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(FrameRateDsTest, getDisplayFrameRate)
{
    EXPECT_CALL(videoDeviceMock, getCurrentDisframerate(::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](char* param) {
                string framerate("3840x2160px48");
                ::memcpy(param, framerate.c_str(), framerate.length());
                return 0;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("getDisplayFrameRate"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"framerate\":\"3840x2160px48\",\"success\":true}"));
}

TEST_F(FrameRateInitializedEventTest, onDisplayFrameRateChanging)
{
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
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
            }));

    ASSERT_TRUE(handlerOnDisplayFrameRateChanging != nullptr);
    handler.Subscribe(0, _T("onDisplayFrameRateChanging"), _T("org.rdk.FrameRate"), message);
    handlerOnDisplayFrameRateChanging(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_PRECHANGE, nullptr, 0);
    handler.Unsubscribe(0, _T("onDisplayFrameRateChanging"), _T("org.rdk.FrameRate"), message);
}

TEST_F(FrameRateInitializedEventTest, onDisplayFrameRateChanged)
{
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
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

    ASSERT_TRUE(handlerOnDisplayFrameRateChanged != nullptr);
    handler.Subscribe(0, _T("onDisplayFrameRateChanged"), _T("org.rdk.FrameRate"), message);
    handlerOnDisplayFrameRateChanged(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_POSTCHANGE, nullptr, 0);
    handler.Unsubscribe(0, _T("onDisplayFrameRateChanged"), _T("org.rdk.FrameRate"), message);
}
