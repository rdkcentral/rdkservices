#include <gtest/gtest.h>

#include "AVInput.h"

#include "FactoriesImplementation.h"

#include "HdmiInputMock.h"
#include "IarmBusMock.h"
#include "ServiceMock.h"

#include "dsMgr.h"

using namespace WPEFramework;

class AVInputTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::AVInput> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    AVInputTest()
        : plugin(Core::ProxyType<Plugin::AVInput>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
    }
    virtual ~AVInputTest() = default;
};

class AVInputDsTest : public AVInputTest {
protected:
    HdmiInputImplMock hdmiInputImplMock;

    AVInputDsTest()
        : AVInputTest()
    {
        device::HdmiInput::getInstance().impl = &hdmiInputImplMock;
    }
    virtual ~AVInputDsTest() override
    {
        device::HdmiInput::getInstance().impl = nullptr;
    }
};

class AVInputInitializedTest : public AVInputTest {
protected:
    IarmBusImplMock iarmBusImplMock;
    IARM_EventHandler_t dsHdmiEventHandler;

    AVInputInitializedTest()
        : AVInputTest()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;

        ON_CALL(iarmBusImplMock, IARM_Bus_IsConnected(::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [](const char*, int* isRegistered) {
                    *isRegistered = 1;
                    return IARM_RESULT_SUCCESS;
                }));
        EXPECT_CALL(iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .Times(::testing::AnyNumber())
            .WillRepeatedly(::testing::Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG)) {
                        EXPECT_TRUE(handler != nullptr);
                        dsHdmiEventHandler = handler;
                    }
                    return IARM_RESULT_SUCCESS;
                }));

        EXPECT_EQ(string(""), plugin->Initialize(nullptr));
    }
    virtual ~AVInputInitializedTest() override
    {
        ON_CALL(iarmBusImplMock, IARM_Bus_UnRegisterEventHandler(::testing::_, ::testing::_))
            .WillByDefault(::testing::Return(IARM_RESULT_SUCCESS));

        plugin->Deinitialize(nullptr);

        IarmBus::getInstance().impl = nullptr;
    }
};

class AVInputInitializedEventTest : public AVInputInitializedTest {
protected:
    ServiceMock service;
    Core::JSONRPC::Message message;
    FactoriesImplementation factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    AVInputInitializedEventTest()
        : AVInputInitializedTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }

    virtual ~AVInputInitializedEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();

        PluginHost::IFactories::Assign(nullptr);
    }
};

class AVInputInitializedEventDsTest : public AVInputInitializedEventTest {
protected:
    HdmiInputImplMock hdmiInputImplMock;

    AVInputInitializedEventDsTest()
        : AVInputInitializedEventTest()
    {
        device::HdmiInput::getInstance().impl = &hdmiInputImplMock;
    }

    virtual ~AVInputInitializedEventDsTest() override
    {
        device::HdmiInput::getInstance().impl = nullptr;
    }
};

TEST_F(AVInputTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("numberOfInputs")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("currentVideoMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("contentProtected")));
}

TEST_F(AVInputTest, contentProtected)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("contentProtected"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"isContentProtected\":true,\"success\":true}"));
}

TEST_F(AVInputDsTest, numberOfInputs)
{
    EXPECT_CALL(hdmiInputImplMock, getNumberOfInputs())
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Return(1));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("numberOfInputs"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"numberOfInputs\":1,\"success\":true}"));
}

TEST_F(AVInputDsTest, currentVideoMode)
{
    EXPECT_CALL(hdmiInputImplMock, getCurrentVideoMode())
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Return(string("unknown")));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("currentVideoMode"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"currentVideoMode\":\"unknown\",\"success\":true}"));
}

TEST_F(AVInputInitializedEventDsTest, onAVInputActive)
{
    ON_CALL(hdmiInputImplMock, getNumberOfInputs())
        .WillByDefault(::testing::Return(1));
    ON_CALL(hdmiInputImplMock, isPortConnected(::testing::_))
        .WillByDefault(::testing::Return(true));
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.AVInput.onAVInputActive\","
                                          "\"params\":{\"url\":\"avin:\\/\\/input0\"}"
                                          "}")));

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onAVInputActive"), _T("org.rdk.AVInput"), message);

    dsHdmiEventHandler(IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG, nullptr, 0);

    handler.Unsubscribe(0, _T("onAVInputActive"), _T("org.rdk.AVInput"), message);
}
