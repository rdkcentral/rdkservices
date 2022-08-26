#include <gtest/gtest.h>

#include "AVInput.h"

#include "FactoriesImplementation.h"
#include "HdmiInputMock.h"
#include "IarmBusMock.h"
#include "ServiceMock.h"
#include "dsMgr.h"

using namespace WPEFramework;

namespace {
const string iarmName = _T("Thunder_Plugins");
}

class AVInputTestFixture : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::AVInput> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    IarmBusImplMock iarmBusImplMock;
    HdmiInputImplMock hdmiInputImplMock;
    IARM_EventHandler_t dsHdmiEventHandler;
    string response;
    ServiceMock service;
    Core::JSONRPC::Message message;
    FactoriesImplementation factoriesImplementation;

    AVInputTestFixture()
        : plugin(Core::ProxyType<Plugin::AVInput>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);
    }
    virtual ~AVInputTestFixture()
    {
        PluginHost::IFactories::Assign(nullptr);
    }

    virtual void SetUp()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;
        device::HdmiInput::getInstance().impl = &hdmiInputImplMock;
    }

    virtual void TearDown()
    {
        IarmBus::getInstance().impl = nullptr;
        device::HdmiInput::getInstance().impl = nullptr;
    }
};

TEST_F(AVInputTestFixture, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("numberOfInputs")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("currentVideoMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("contentProtected")));
}

TEST_F(AVInputTestFixture, Plugin)
{
    // IARM expectations

    // called by AVInput::InitializeIARM, AVInput::DeinitializeIARM
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

    // called by AVInput::InitializeIARM
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Init(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](const char* name) {
                if (iarmName == string(name)) {
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            }));

    // called by AVInput::InitializeIARM
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Connect())
        .Times(1)
        .WillOnce(::testing::Return(IARM_RESULT_SUCCESS));

    // called by AVInput::InitializeIARM
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG)) {
                    dsHdmiEventHandler = handler;
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            }));

    // called by AVInput::DeinitializeIARM
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_UnRegisterEventHandler(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const char* ownerName, IARM_EventId_t eventId) {
                if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG)) {
                    dsHdmiEventHandler = nullptr;
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            }));

    // HdmiInput expectations

    // called by AVInput::numberOfInputs, dsHdmiEventHandler
    EXPECT_CALL(hdmiInputImplMock, getNumberOfInputs())
        .Times(2)
        .WillRepeatedly(::testing::Return(1));

    // called by dsHdmiEventHandler
    EXPECT_CALL(hdmiInputImplMock, isPortConnected(::testing::_))
        .Times(1)
        .WillRepeatedly(::testing::Return(true));

    // called by AVInput::currentVideoMode
    EXPECT_CALL(hdmiInputImplMock, getCurrentVideoMode())
        .Times(1)
        .WillRepeatedly(::testing::Return(string("unknown")));

    // IShell expectations

    // called by AVInput::event_onAVInputActive
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

    // Initialize

    EXPECT_EQ(string(""), plugin->Initialize(nullptr));

    // JSON-RPC methods

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("numberOfInputs"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"numberOfInputs\":1,\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("currentVideoMode"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"currentVideoMode\":\"unknown\",\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("contentProtected"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"isContentProtected\":true,\"success\":true}"));

    // JSON-RPC events

    auto dispatcher = static_cast<PluginHost::IDispatcher*>(
        plugin->QueryInterface(PluginHost::IDispatcher::ID));
    EXPECT_TRUE(dispatcher != nullptr);

    dispatcher->Activate(&service);

    EXPECT_TRUE(dsHdmiEventHandler != nullptr);

    handler.Subscribe(0, _T("onAVInputActive"), _T("org.rdk.AVInput"), message);

    dsHdmiEventHandler(
        IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG, nullptr, 0);

    handler.Unsubscribe(0, _T("onAVInputActive"), _T("org.rdk.AVInput"), message);

    dispatcher->Deactivate();

    dispatcher->Release();

    // Deinitialize

    plugin->Deinitialize(nullptr);
}
