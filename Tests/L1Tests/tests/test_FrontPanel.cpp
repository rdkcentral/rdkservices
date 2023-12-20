#include <gtest/gtest.h>

#include "FrontPanel.h"
#include "frontpanel.h"


#include "FactoriesImplementation.h"

#include "FrontPanelIndicatorMock.h"
#include "FrontPanelTextDisplayMock.h"
#include "FrontPanelConfigMock.h"
#include "IarmBusMock.h"
#include "ServiceMock.h"
#include "ColorMock.h"

using namespace WPEFramework;

using testing::Eq;

class FrontPanelTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::FrontPanel> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    FrontPanelTest()
        : plugin(Core::ProxyType<Plugin::FrontPanel>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
    }
    virtual ~FrontPanelTest() = default;
};

class FrontPanelDsTest : public FrontPanelTest {
protected:
    testing::NiceMock<FrontPanelIndicatorMock> frontPanelIndicatorMock;
    testing::NiceMock<FrontPanelIndicatorMock> frontPanelTextDisplayIndicatorMock;
    FrontPanelConfigMock   *p_frontPanelConfigImplMock = nullptr;
    FrontPanelTextDisplayMock   *p_frontPanelTextDisplayMock = nullptr;

    FrontPanelDsTest()
        : FrontPanelTest()
    {
        device::FrontPanelIndicator::getInstance().impl = &frontPanelIndicatorMock;

        p_frontPanelConfigImplMock  = new testing::NiceMock <FrontPanelConfigMock>;
        device::FrontPanelConfig::setImpl(p_frontPanelConfigImplMock);

        p_frontPanelTextDisplayMock  = new testing::NiceMock <FrontPanelTextDisplayMock>;
        device::FrontPanelTextDisplay::setImpl(p_frontPanelTextDisplayMock);

        device::FrontPanelTextDisplay::getInstance().FrontPanelIndicator::impl = &frontPanelTextDisplayIndicatorMock;


    }
    virtual ~FrontPanelDsTest() override
    {
        device::FrontPanelIndicator::getInstance().impl = nullptr;
        device::FrontPanelTextDisplay::getInstance().FrontPanelIndicator::impl = nullptr;
        if (p_frontPanelTextDisplayMock != nullptr)
        {
            delete p_frontPanelTextDisplayMock;
            p_frontPanelTextDisplayMock = nullptr;
        }
        if (p_frontPanelConfigImplMock != nullptr)
        {
            delete p_frontPanelConfigImplMock;
            p_frontPanelConfigImplMock = nullptr;
        }

    }
};

class FrontPanelInitializedTest : public FrontPanelTest {
protected:
    IarmBusImplMock   *p_iarmBusImplMock = nullptr ;
    FrontPanelConfigMock   *p_frontPanelConfigImplMock = nullptr;
    FrontPanelTextDisplayMock   *p_frontPanelTextDisplayMock = nullptr;
    IARM_EventHandler_t dsPanelEventHandler;
    testing::NiceMock<FrontPanelIndicatorMock> frontPanelIndicatorMock;
    testing::NiceMock<FrontPanelIndicatorMock> frontPanelTextDisplayIndicatorMock;

    IARM_EventHandler_t dsFrontPanelModeChange;

    FrontPanelInitializedTest()
        : FrontPanelTest()
    {


        p_iarmBusImplMock  = new testing::NiceMock <IarmBusImplMock>;
        IarmBus::setImpl(p_iarmBusImplMock);

        p_frontPanelConfigImplMock  = new testing::NiceMock <FrontPanelConfigMock>;
        device::FrontPanelConfig::setImpl(p_frontPanelConfigImplMock);

        device::FrontPanelIndicator::getInstance().impl = &frontPanelIndicatorMock;
        device::FrontPanelTextDisplay::getInstance().FrontPanelIndicator::impl = &frontPanelTextDisplayIndicatorMock;

        p_frontPanelTextDisplayMock  = new testing::NiceMock <FrontPanelTextDisplayMock>;
        device::FrontPanelTextDisplay::setImpl(p_frontPanelTextDisplayMock);

        //Needs to be set at initiative time, as the function gets called when FrontPanel is intialized.
        ON_CALL(frontPanelIndicatorMock, getInstanceString)
            .WillByDefault(::testing::Invoke(
                [&](const std::string& name) -> device::FrontPanelIndicator& {
                    EXPECT_EQ("Power", name);
                    return device::FrontPanelIndicator::getInstance();
                }));

        ON_CALL(*p_frontPanelConfigImplMock, getIndicators())
            .WillByDefault(::testing::Return(device::List<device::FrontPanelIndicator>({ device::FrontPanelIndicator::getInstance() })));



        ON_CALL(*p_iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    if ((string(IARM_BUS_PWRMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_PWRMGR_EVENT_MODECHANGED)) {
                        EXPECT_TRUE(handler != nullptr);
                        dsFrontPanelModeChange = handler;
                    }

                    return IARM_RESULT_SUCCESS;
                }));

        EXPECT_EQ(string(""), plugin->Initialize(nullptr));
    }
    virtual ~FrontPanelInitializedTest() override
    {
        device::FrontPanelIndicator::getInstance().impl = nullptr;
        device::FrontPanelTextDisplay::getInstance().FrontPanelIndicator::impl = nullptr;

        plugin->Deinitialize(nullptr);
        //Clearing out out of scope variables, and setting initDone to 0.
        Plugin::CFrontPanel::initDone = 0;
        IarmBus::setImpl(nullptr);
        if (p_iarmBusImplMock != nullptr)
        {
            delete p_iarmBusImplMock;
            p_iarmBusImplMock = nullptr;
        }
        device::FrontPanelTextDisplay::setImpl(nullptr);
        if (p_frontPanelTextDisplayMock != nullptr)
        {
            delete p_frontPanelTextDisplayMock;
            p_frontPanelTextDisplayMock = nullptr;
        }
        device::FrontPanelConfig::setImpl(nullptr);
        if (p_frontPanelConfigImplMock != nullptr)
        {
            delete p_frontPanelConfigImplMock;
            p_frontPanelConfigImplMock = nullptr;
        }

    }
};

class FrontPanelInitializedEventTest : public FrontPanelInitializedTest {
protected:
    testing::NiceMock<ServiceMock> service;
    Core::JSONRPC::Message message;
    FactoriesImplementation factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    FrontPanelInitializedEventTest()
        : FrontPanelInitializedTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }

    virtual ~FrontPanelInitializedEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();

        PluginHost::IFactories::Assign(nullptr);
    }
};

class FrontPanelInitializedEventDsTest : public FrontPanelInitializedEventTest {
protected:

    ColorMock      *p_colorImplMock = nullptr ;

    FrontPanelInitializedEventDsTest()
        : FrontPanelInitializedEventTest()
    {
        p_colorImplMock  = new testing::NiceMock <ColorMock>;
        device::FrontPanelIndicator::Color::setImpl(p_colorImplMock);

        //Variable that needs to be set is set through this call
        IARM_Bus_PWRMgr_EventData_t eventData;
        eventData.data.state.newState =IARM_BUS_PWRMGR_POWERSTATE_ON;
        eventData.data.state.curState =IARM_BUS_PWRMGR_POWERSTATE_STANDBY;
        dsFrontPanelModeChange(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, &eventData , 0);

    }

    virtual ~FrontPanelInitializedEventDsTest() override
    {
        device::FrontPanelIndicator::Color::setImpl(nullptr);
        if (p_colorImplMock != nullptr)
        {
            delete p_colorImplMock;
            p_colorImplMock = nullptr;
        }
    }
};

TEST_F(FrontPanelTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setBrightness")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getBrightness")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("powerLedOn")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("powerLedOff")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setClockBrightness")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getClockBrightness")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getFrontPanelLights")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPreferences")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setPreferences")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setLED")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setBlink")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("set24HourClock")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("is24HourClock")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setClockTestPattern")));
}

TEST_F(FrontPanelInitializedEventDsTest, setBrightnessWIndex)
{

    ON_CALL(frontPanelIndicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));


    ON_CALL(frontPanelIndicatorMock, getName())
        .WillByDefault(::testing::Return("Power"));


    EXPECT_CALL(frontPanelIndicatorMock, setBrightness(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](int brightness, bool toPersist) {
                EXPECT_EQ(brightness, 1);
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setBrightness"), _T("{\"brightness\": 1,\"index\": \"power_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}
TEST_F(FrontPanelInitializedEventDsTest, setBrightnessClock)
{
    ON_CALL(*p_frontPanelTextDisplayMock, getInstanceByName)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelTextDisplay& {
                EXPECT_EQ("Text", name);
                return device::FrontPanelTextDisplay::getInstance();
            }));



    ON_CALL(*p_frontPanelConfigImplMock, getIndicators())
        .WillByDefault(::testing::Return(device::List<device::FrontPanelIndicator>({device::FrontPanelIndicator::getInstance()})));
    ON_CALL(frontPanelIndicatorMock, getName())
        .WillByDefault(::testing::Return("red"));

    EXPECT_CALL(*p_frontPanelTextDisplayMock, setTextBrightness(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](int brightness) {
                EXPECT_EQ(brightness, 1);
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setBrightness"), _T("{\"brightness\": 1,\"index\": \"clock_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(FrontPanelInitializedEventDsTest, setBrightness)
{

    ON_CALL(frontPanelIndicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));

    ON_CALL(frontPanelIndicatorMock, getName())
        .WillByDefault(::testing::Return("Power"));

    EXPECT_CALL(frontPanelIndicatorMock, setBrightness(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](int brightness, bool toPersist) {
                EXPECT_EQ(brightness, 1);
            }));


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setBrightness"), _T("{\"brightness\": 1}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(FrontPanelInitializedEventDsTest, getBrightnessWIndex)
{

    ON_CALL(frontPanelIndicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));
    ON_CALL(frontPanelIndicatorMock, getBrightness())
        .WillByDefault(::testing::Return(50));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getBrightness"), _T("{\"index\": \"power_led\"}"), response));
    EXPECT_EQ(response, string("{\"brightness\":50,\"success\":true}"));
}

TEST_F(FrontPanelInitializedEventDsTest, getBrightnessOtherName)
{

    ON_CALL(frontPanelIndicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("other", name);
                return device::FrontPanelIndicator::getInstance();
            }));
    ON_CALL(frontPanelIndicatorMock, getBrightness())
        .WillByDefault(::testing::Return(50));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getBrightness"), _T("{\"index\": \"other\"}"), response));
    EXPECT_EQ(response, string("{\"brightness\":50,\"success\":true}"));
}

TEST_F(FrontPanelInitializedEventDsTest, getBrightnessWIndexClock)
{

    ON_CALL(*p_frontPanelTextDisplayMock, getInstanceByName)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelTextDisplay& {
                EXPECT_EQ("Text", name);
                return device::FrontPanelTextDisplay::getInstance();
            }));


    ON_CALL(*p_frontPanelTextDisplayMock, getTextBrightness())
        .WillByDefault(::testing::Return(50));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getBrightness"), _T("{\"index\": \"clock_led\"}"), response));
    EXPECT_EQ(response, string("{\"brightness\":50,\"success\":true}"));
}


TEST_F(FrontPanelInitializedEventDsTest, getBrightness)
{

    ON_CALL(frontPanelIndicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));
    ON_CALL(frontPanelIndicatorMock, getBrightness())
        .WillByDefault(::testing::Return(50));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getBrightness"), _T(""), response));
    EXPECT_EQ(response, string("{\"brightness\":50,\"success\":true}"));
}
TEST_F(FrontPanelInitializedEventDsTest, getClockBrightness)
{

    ON_CALL(*p_frontPanelTextDisplayMock, getInstanceByName)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelTextDisplay& {
                EXPECT_EQ("Text", name);
                return device::FrontPanelTextDisplay::getInstance();
            }));

    ON_CALL(*p_frontPanelTextDisplayMock, getTextBrightness())
        .WillByDefault(::testing::Return(50));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getClockBrightness"), _T(""), response));
    EXPECT_EQ(response, string("{\"brightness\":50,\"success\":true}"));
}

TEST_F(FrontPanelInitializedEventDsTest, setClockBrightness)
{

    ON_CALL(*p_frontPanelTextDisplayMock, getInstanceByName)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelTextDisplay& {
                EXPECT_EQ("Text", name);
                return device::FrontPanelTextDisplay::getInstance();
            }));


    ON_CALL(*p_frontPanelConfigImplMock, getIndicators())
        .WillByDefault(::testing::Return(device::List<device::FrontPanelIndicator>({device::FrontPanelIndicator::getInstance()})));
    ON_CALL(frontPanelIndicatorMock, getName())
        .WillByDefault(::testing::Return("red"));

    EXPECT_CALL(*p_frontPanelTextDisplayMock, setTextBrightness(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](int brightness) {
                EXPECT_EQ(brightness, 1);
            }));


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setClockBrightness"), _T("{\"brightness\": 1}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(FrontPanelInitializedEventDsTest, getFrontPanelLights)
{

    ON_CALL(*p_frontPanelConfigImplMock, getIndicators())
        .WillByDefault(::testing::Return(device::List<device::FrontPanelIndicator>({ device::FrontPanelIndicator::getInstance() })));

    ON_CALL(frontPanelIndicatorMock, getBrightnessLevels(::testing::_,::testing::_,::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](int &levels,int &min,int &max) {
                levels=1;
                min=0;
                max=2;
            }));
    ON_CALL(frontPanelTextDisplayIndicatorMock, getBrightnessLevels(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](int& levels, int& min, int& max) {
                levels = 1;
                min = 0;
                max = 2;
            }));

    ON_CALL(*p_frontPanelConfigImplMock, getTextDisplays())
        .WillByDefault(::testing::Return(device::List<device::FrontPanelTextDisplay>({ device::FrontPanelTextDisplay::getInstance() })));
    ON_CALL(frontPanelTextDisplayIndicatorMock, getName())
        .WillByDefault(::testing::Return("Text"));
    ON_CALL(*p_colorImplMock, getName())
        .WillByDefault(::testing::Return("white"));


        int test = 0;

    ON_CALL(*p_frontPanelConfigImplMock, getTextDisplay(test))
        .WillByDefault(::testing::ReturnRef(device::FrontPanelTextDisplay::getInstance()));

    ON_CALL(frontPanelIndicatorMock, getName())
        .WillByDefault(::testing::Return("Power"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFrontPanelLights"), _T(""), response));
    EXPECT_EQ(response, string("{\"supportedLights\":[\"power_led\",\"clock_led\"],\"supportedLightsInfo\":{\"power_led\":{\"range\":\"boolean\",\"min\":0,\"max\":2,\"colorMode\":0},\"clock_led\":{\"range\":\"boolean\",\"min\":0,\"max\":2,\"colorMode\":0}},\"success\":true}"));
}

TEST_F(FrontPanelInitializedEventDsTest, getPreferences)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setPreferences"), _T("{\"preferences\":{\"test\": true}}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPreferences"), _T(""), response));
    EXPECT_EQ(response, string("{\"preferences\":{\"test\":true},\"success\":true}"));
}

TEST_F(FrontPanelInitializedEventDsTest, is24HourClock)
{

    std::string test = "Text";

    ON_CALL(*p_frontPanelConfigImplMock, getTextDisplay(test))
        .WillByDefault(::testing::ReturnRef(device::FrontPanelTextDisplay::getInstance()));
    ON_CALL(*p_frontPanelTextDisplayMock, getCurrentTimeFormat())
        .WillByDefault(::testing::Return(dsFPD_TIME_12_HOUR));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("is24HourClock"), _T(""), response));
    EXPECT_EQ(response, string("{\"is24Hour\":false,\"success\":true}"));

    ON_CALL(*p_frontPanelTextDisplayMock, getCurrentTimeFormat())
        .WillByDefault(::testing::Return(dsFPD_TIME_24_HOUR));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("is24HourClock"), _T(""), response));
    EXPECT_EQ(response, string("{\"is24Hour\":true,\"success\":true}"));

}

TEST_F(FrontPanelInitializedEventDsTest, powerLedOffPower)
{

    ON_CALL(frontPanelIndicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));


    EXPECT_CALL(frontPanelIndicatorMock, setState(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](bool state) {
                EXPECT_EQ(state, false);
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("powerLedOff"), _T("{\"index\": \"power_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}
TEST_F(FrontPanelInitializedEventDsTest, powerLedOffData)
{

    ON_CALL(frontPanelIndicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Message", name);
                return device::FrontPanelIndicator::getInstance();
            }));


    EXPECT_CALL(frontPanelIndicatorMock, setState(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](bool state) {
                EXPECT_EQ(state, false);
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("powerLedOff"), _T("{\"index\": \"data_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}
TEST_F(FrontPanelInitializedEventDsTest, powerLedOffRecord)
{

    ON_CALL(frontPanelIndicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Record", name);
                return device::FrontPanelIndicator::getInstance();
            }));


    EXPECT_CALL(frontPanelIndicatorMock, setState(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](bool state) {
                EXPECT_EQ(state, false);
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("powerLedOff"), _T("{\"index\": \"record_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(FrontPanelInitializedEventDsTest, powerLedOnPower)
{

    ON_CALL(*p_frontPanelConfigImplMock, getIndicators())
        .WillByDefault(::testing::Return(device::List<device::FrontPanelIndicator>({device::FrontPanelIndicator::getInstance()})));
    ON_CALL(frontPanelIndicatorMock, getName())
        .WillByDefault(::testing::Return("red"));

 ON_CALL(frontPanelIndicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("powerLedOn"), _T("{\"index\": \"power_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    ON_CALL(frontPanelIndicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Record", name);
                return device::FrontPanelIndicator::getInstance();
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("powerLedOn"), _T("{\"index\": \"record_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    ON_CALL(frontPanelIndicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Message", name);
                return device::FrontPanelIndicator::getInstance();
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("powerLedOn"), _T("{\"index\": \"data_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));


}

TEST_F(FrontPanelInitializedEventDsTest, set24HourClock)
{
    std::string test = "Text";

    ON_CALL(*p_frontPanelConfigImplMock, getTextDisplay(test))
        .WillByDefault(::testing::ReturnRef(device::FrontPanelTextDisplay::getInstance()));
    ON_CALL(*p_frontPanelTextDisplayMock, getCurrentTimeFormat())
        .WillByDefault(::testing::Return(dsFPD_TIME_24_HOUR));
    EXPECT_CALL(*p_frontPanelTextDisplayMock, setTimeFormat(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const int iTimeFormat) {
                EXPECT_EQ(iTimeFormat, dsFPD_TIME_24_HOUR);
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("set24HourClock"), _T("{\"is24Hour\": true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

}

TEST_F(FrontPanelInitializedEventDsTest, setBlink)
{

    ON_CALL(frontPanelIndicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));

    ON_CALL(frontPanelIndicatorMock, getBrightness())
        .WillByDefault(::testing::Return(50));
    ON_CALL(*p_frontPanelTextDisplayMock, getTextBrightness())
        .WillByDefault(::testing::Return(50));

    ON_CALL(frontPanelIndicatorMock, getName())
        .WillByDefault(::testing::Return("Power"));

    EXPECT_CALL(frontPanelIndicatorMock, setColorInt(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](uint32_t color, bool persist) {

                EXPECT_EQ(color, 131586);
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setBlink"), _T("{\"blinkInfo\": {\"ledIndicator\": \"power_led\", \"iterations\": 10, \"pattern\": [{\"brightness\": 50, \"duration\": 100, \"red\": 2, \"green\":2, \"blue\":2}]}}"), response));

        EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(FrontPanelInitializedEventDsTest, setClockTestPattern)
{

    ON_CALL(frontPanelIndicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));


    ON_CALL(*p_colorImplMock, getInstanceByName)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator::Color& {
                EXPECT_EQ("red", name);
                return device::FrontPanelIndicator::Color::getInstance();
            }));

    std::string test = "Text";

    ON_CALL(*p_frontPanelConfigImplMock, getTextDisplay(test))
        .WillByDefault(::testing::ReturnRef(device::FrontPanelTextDisplay::getInstance()));
    ON_CALL(*p_frontPanelTextDisplayMock, getTextBrightness())
        .WillByDefault(::testing::Return(100));


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setClockTestPattern"), _T("{\"show\": true, \"timeInterval\": 4}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setClockTestPattern"), _T("{\"show\": false, \"timeInterval\": 4}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

}

TEST_F(FrontPanelInitializedEventDsTest, setLEDMode1)
{

    ON_CALL(frontPanelIndicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));

    ON_CALL(frontPanelIndicatorMock, getName())
        .WillByDefault(::testing::Return("Power"));

    EXPECT_CALL(frontPanelIndicatorMock, setColorInt(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](uint32_t color, bool toPersist) {
                EXPECT_EQ(color, 0);
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setLED"), _T("{\"ledIndicator\": \"power_led\", \"brightness\": 50, \"red\": 0, \"green\": 0, \"blue\":0}"), response));

        EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(FrontPanelInitializedEventDsTest, setLEDMode2)
{

    ON_CALL(frontPanelIndicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));


    EXPECT_CALL(frontPanelIndicatorMock, setColorInt(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](uint32_t color, bool toPersist) {
                EXPECT_EQ(color, 66051);
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setLED"), _T("{\"ledIndicator\": \"power_led\", \"brightness\": 50, \"red\": 1, \"green\": 2, \"blue\":3}"), response));

        EXPECT_EQ(response, string("{\"success\":true}"));
}


