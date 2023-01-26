#include <gtest/gtest.h>

#include "FrontPanel.h"
#include "frontpanel.h"

//required to set initDone to 0, so we reset values in CFrontPanel instance.
#include "frontpanel.cpp"

#include "FactoriesImplementation.h"

#include "FrontPanelIndicatorMock.h"
#include "FrontPanelTextDisplayMock.h"
#include "FrontPanelConfigMock.h"
#include "IarmBusMock.h"
#include "ServiceMock.h"
#include "ColorMock.h"

using namespace WPEFramework;

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
    FrontPanelIndicatorMock frontPanelIndicatorImplMock;
    ColorMock colorImplMock;
    FrontPanelTextDisplayMock frontPanelTextDisplayImplMock;
    FrontPanelTextDisplayMock frontPanelTextDisplayImplIntMock;
    FrontPanelConfigMock frontPanelConfigImplMock;
    FrontPanelDsTest()
        : FrontPanelTest()
    {
        device::FrontPanelIndicator::getInstance().impl = &frontPanelIndicatorImplMock;
	    device::FrontPanelConfig::getInstance().impl = &frontPanelConfigImplMock;
        device::FrontPanelTextDisplay::getInstance().impl = &frontPanelTextDisplayImplMock;

    }
    virtual ~FrontPanelDsTest() override
    {
        device::FrontPanelIndicator::getInstance().impl = nullptr;
	    device::FrontPanelConfig::getInstance().impl = nullptr;
        device::FrontPanelTextDisplay::getInstance().impl = nullptr;


    }
};

class FrontPanelInitializedTest : public FrontPanelTest {
protected:
    IarmBusImplMock iarmBusImplMock;
    IARM_EventHandler_t dsPanelEventHandler;
    FrontPanelConfigMock frontPanelConfigImplMock;
    FrontPanelIndicatorMock frontPanelIndicatorImplMock;
    FrontPanelTextDisplayMock frontPanelTextDisplayImplMock;

    IARM_EventHandler_t dsFrontPanelModeChange;

    FrontPanelInitializedTest()
        : FrontPanelTest()
    {

        //Needs to be run before plugin start for fpindicators to get set.

        testing::NiceMock<FrontPanelIndicatorMock> indicatorMock;
    device::FrontPanelIndicator::getInstance().impl = &indicatorMock;

    //Needs to be set at initiative time, as the function gets called when FrontPanel is intialized.
    ON_CALL(indicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));

        device::FrontPanelIndicator indicatorList;
        indicatorList.impl = &frontPanelIndicatorImplMock;
    

        ON_CALL(frontPanelConfigImplMock, getIndicators())
            .WillByDefault(::testing::Return(device::List<device::FrontPanelIndicator>({ indicatorList })));
    
        IarmBus::getInstance().impl = &iarmBusImplMock;
        device::FrontPanelConfig::getInstance().impl = &frontPanelConfigImplMock;
        device::FrontPanelIndicator::getInstance().impl = &frontPanelIndicatorImplMock;
        device::FrontPanelTextDisplay::getInstance().impl = &frontPanelTextDisplayImplMock;


        ON_CALL(iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
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
        device::FrontPanelConfig::getInstance().impl = nullptr;
        device::FrontPanelIndicator::getInstance().impl = nullptr;
        device::FrontPanelTextDisplay::getInstance().impl = nullptr;


	    plugin->Deinitialize(nullptr);
        //Clearing out out of scope variables, and setting initDone to 0.
        Plugin::initDone = 0;
        Plugin::fpIndicators.clear();
        IarmBus::getInstance().impl = nullptr;
    }
};

class FrontPanelInitializedEventTest : public FrontPanelInitializedTest {
protected:
    ServiceMock service;
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

    ColorMock colorImplMock;

    FrontPanelInitializedEventDsTest()
        : FrontPanelInitializedEventTest()
    {
        device::FrontPanelIndicator::Color::getInstance().impl = &colorImplMock;   
    }

    virtual ~FrontPanelInitializedEventDsTest() override
    {
        device::FrontPanelIndicator::Color::getInstance().impl = nullptr;
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

TEST_F(FrontPanelInitializedEventDsTest, powerModeChange)
{
    //powerModeChange does not make any external function calls, and simply sets the variable powerStatus.
    //The tests that need powerState set to true may not work if helper/frontpanel.cpp
    //ever gets fixed to be released from memory/deinitialized.
    ASSERT_TRUE(dsFrontPanelModeChange != nullptr);

    IARM_Bus_PWRMgr_EventData_t eventData;
    eventData.data.state.newState =IARM_BUS_PWRMGR_POWERSTATE_ON;
    eventData.data.state.curState =IARM_BUS_PWRMGR_POWERSTATE_STANDBY;

    dsFrontPanelModeChange(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, &eventData , 0);
    
}

TEST_F(FrontPanelInitializedEventDsTest, setBrightnessWIndex)
{
    //device::FrontPanelIndicator panel;
    testing::NiceMock<FrontPanelIndicatorMock> indicatorMock;
    device::FrontPanelIndicator::getInstance().impl = &indicatorMock;


    ON_CALL(indicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));

        
    ON_CALL(frontPanelIndicatorImplMock, getName())
        .WillByDefault(::testing::Return("Power"));


    EXPECT_CALL(indicatorMock, setBrightness(::testing::_, ::testing::_))
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

    testing::NiceMock<FrontPanelIndicatorMock> indicatorMock;
    device::FrontPanelIndicator::getInstance().impl = &indicatorMock;

    FrontPanelIndicatorMock frontPanelIndicatorMock;

    device::FrontPanelIndicator indicatorList;
    indicatorList.impl = &indicatorMock;

    testing::NiceMock<FrontPanelTextDisplayMock> displayMock;
    device::FrontPanelTextDisplay::getInstance().impl = &displayMock;


    ON_CALL(displayMock, getInstanceByName)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelTextDisplay& {
                EXPECT_EQ("Text", name);
                return device::FrontPanelTextDisplay::getInstance();
            }));



    ON_CALL(frontPanelConfigImplMock, getIndicators())
        .WillByDefault(::testing::Return(device::List<device::FrontPanelIndicator>({indicatorList})));
    ON_CALL(indicatorMock, getName())
        .WillByDefault(::testing::Return("red"));

    
    EXPECT_CALL(displayMock, setTextBrightness(::testing::_))
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

    testing::NiceMock<FrontPanelIndicatorMock> indicatorMock;
    device::FrontPanelIndicator::getInstance().impl = &indicatorMock;

    ON_CALL(indicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));
    
    ON_CALL(frontPanelIndicatorImplMock, getName())
        .WillByDefault(::testing::Return("Power"));

    EXPECT_CALL(indicatorMock, setBrightness(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](int brightness, bool toPersist) {
                EXPECT_EQ(brightness, 1);
            }));
       

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setBrightness"), _T("{\"brightness\": 1}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(FrontPanelInitializedEventDsTest, setBrightnessFalse)
{
    
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setBrightness"), _T("{}"), response));
    EXPECT_EQ(response, string(""));
}
TEST_F(FrontPanelInitializedEventDsTest, setBrightnessNeg1)
{


    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setBrightness"), _T("{\"brightness\": -1}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(FrontPanelInitializedEventDsTest, getBrightnessWIndex)
{
    testing::NiceMock<FrontPanelIndicatorMock> indicatorMock;
    device::FrontPanelIndicator::getInstance().impl = &indicatorMock;

    ON_CALL(indicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));
    ON_CALL(indicatorMock, getBrightness())
        .WillByDefault(::testing::Return(50));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getBrightness"), _T("{\"index\": \"power_led\"}"), response));
    EXPECT_EQ(response, string("{\"brightness\":50,\"success\":true}"));
}

TEST_F(FrontPanelInitializedEventDsTest, getBrightnessOtherName)
{
    testing::NiceMock<FrontPanelIndicatorMock> indicatorMock;
    device::FrontPanelIndicator::getInstance().impl = &indicatorMock;

    ON_CALL(indicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("other", name);
                return device::FrontPanelIndicator::getInstance();
            }));
    ON_CALL(indicatorMock, getBrightness())
        .WillByDefault(::testing::Return(50));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getBrightness"), _T("{\"index\": \"other\"}"), response));
    EXPECT_EQ(response, string("{\"brightness\":50,\"success\":true}"));
}

TEST_F(FrontPanelInitializedEventDsTest, getBrightnessWIndexClock)
{
    testing::NiceMock<FrontPanelTextDisplayMock> displayMock;
    device::FrontPanelTextDisplay::getInstance().impl = &displayMock;


    ON_CALL(displayMock, getInstanceByName)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelTextDisplay& {
                EXPECT_EQ("Text", name);
                return device::FrontPanelTextDisplay::getInstance();
            }));


    ON_CALL(displayMock, getTextBrightness())
        .WillByDefault(::testing::Return(50));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getBrightness"), _T("{\"index\": \"clock_led\"}"), response));
    EXPECT_EQ(response, string("{\"brightness\":50,\"success\":true}"));
}


TEST_F(FrontPanelInitializedEventDsTest, getBrightness)
{

    testing::NiceMock<FrontPanelIndicatorMock> indicatorMock;
    device::FrontPanelIndicator::getInstance().impl = &indicatorMock;

    ON_CALL(indicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));
    ON_CALL(indicatorMock, getBrightness())
        .WillByDefault(::testing::Return(50));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getBrightness"), _T(""), response));
    EXPECT_EQ(response, string("{\"brightness\":50,\"success\":true}"));
}
TEST_F(FrontPanelInitializedEventDsTest, getClockBrightness)
{
    testing::NiceMock<FrontPanelTextDisplayMock> displayMock;
    device::FrontPanelTextDisplay::getInstance().impl = &displayMock;


    ON_CALL(displayMock, getInstanceByName)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelTextDisplay& {
                EXPECT_EQ("Text", name);
                return device::FrontPanelTextDisplay::getInstance();
            }));

    ON_CALL(displayMock, getTextBrightness())
        .WillByDefault(::testing::Return(50));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getClockBrightness"), _T(""), response));
    EXPECT_EQ(response, string("{\"brightness\":50,\"success\":true}"));
}

TEST_F(FrontPanelInitializedEventDsTest, setClockBrightness)
{

    //setting powermode

    testing::NiceMock<FrontPanelTextDisplayMock> displayMock;
    device::FrontPanelTextDisplay::getInstance().impl = &displayMock;


    ON_CALL(displayMock, getInstanceByName)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelTextDisplay& {
                EXPECT_EQ("Text", name);
                return device::FrontPanelTextDisplay::getInstance();
            }));

    FrontPanelIndicatorMock frontPanelIndicatorMock;
    device::FrontPanelIndicator indicatorList;
    indicatorList.impl = &frontPanelIndicatorMock;
    ON_CALL(frontPanelConfigImplMock, getIndicators())
        .WillByDefault(::testing::Return(device::List<device::FrontPanelIndicator>({indicatorList})));
    ON_CALL(frontPanelIndicatorMock, getName())
        .WillByDefault(::testing::Return("red"));

    EXPECT_CALL(displayMock, setTextBrightness(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](int brightness) {
                EXPECT_EQ(brightness, 1);
            }));
      

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setClockBrightness"), _T("{\"brightness\": 1}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}
TEST_F(FrontPanelInitializedEventDsTest, setClockBrightnessInvalid)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setClockBrightness"), _T("{\"brightness\": -1}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(FrontPanelInitializedEventDsTest, getFrontPanelLights)
{
    FrontPanelIndicatorMock frontPanelIndicatorMock;

    device::FrontPanelIndicator indicatorList;
    indicatorList.impl = &frontPanelIndicatorMock;

    

    ON_CALL(frontPanelConfigImplMock, getIndicators())
        .WillByDefault(::testing::Return(device::List<device::FrontPanelIndicator>({ indicatorList })));

    ON_CALL(frontPanelIndicatorMock, getBrightnessLevels(::testing::_,::testing::_,::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](int &levels,int &min,int &max) {
                levels=1;
		min=0;
		max=2;
            }));

    FrontPanelTextDisplayMock frontPanelTextDisplayMock;
    
    device::FrontPanelTextDisplay displayList;
    displayList.impl = &frontPanelTextDisplayMock;
    displayList.FrontPanelIndicator::impl = &frontPanelIndicatorMock;

    ON_CALL(frontPanelConfigImplMock, getTextDisplays())
        .WillByDefault(::testing::Return(device::List<device::FrontPanelTextDisplay>({ displayList })));
    ON_CALL(frontPanelTextDisplayMock, getName())
        .WillByDefault(::testing::Return("Text"));
    ON_CALL(colorImplMock, getName())
        .WillByDefault(::testing::Return("white"));

   
	int test = 0;
    device::FrontPanelTextDisplay displayList2;
    displayList2.impl = &frontPanelTextDisplayMock;
    displayList2.FrontPanelIndicator::impl = &frontPanelIndicatorMock;

    ON_CALL(frontPanelConfigImplMock, getTextDisplay(test))
        .WillByDefault(::testing::ReturnRef(displayList2));

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
    device::FrontPanelTextDisplay displayList;
    displayList.impl = &frontPanelTextDisplayImplMock;
    displayList.FrontPanelIndicator::impl = &frontPanelIndicatorImplMock;

    std::string test = "Text";
    
    ON_CALL(frontPanelConfigImplMock, getTextDisplay(test))
        .WillByDefault(::testing::ReturnRef(displayList));
    ON_CALL(frontPanelTextDisplayImplMock, getCurrentTimeFormat())
        .WillByDefault(::testing::Return(dsFPD_TIME_12_HOUR));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("is24HourClock"), _T(""), response));
    EXPECT_EQ(response, string("{\"is24Hour\":false,\"success\":true}"));

    ON_CALL(frontPanelTextDisplayImplMock, getCurrentTimeFormat())
        .WillByDefault(::testing::Return(dsFPD_TIME_24_HOUR));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("is24HourClock"), _T(""), response));
    EXPECT_EQ(response, string("{\"is24Hour\":true,\"success\":true}"));

}

TEST_F(FrontPanelInitializedEventDsTest, powerLedOffPower)
{

    testing::NiceMock<FrontPanelIndicatorMock> indicatorMock;
    device::FrontPanelIndicator::getInstance().impl = &indicatorMock;

    ON_CALL(indicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));


    EXPECT_CALL(indicatorMock, setState(::testing::_))
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

    testing::NiceMock<FrontPanelIndicatorMock> indicatorMock;
    device::FrontPanelIndicator::getInstance().impl = &indicatorMock;

    ON_CALL(indicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Message", name);
                return device::FrontPanelIndicator::getInstance();
            }));


    EXPECT_CALL(indicatorMock, setState(::testing::_))
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
    testing::NiceMock<FrontPanelIndicatorMock> indicatorMock;
    device::FrontPanelIndicator::getInstance().impl = &indicatorMock;

    ON_CALL(indicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Record", name);
                return device::FrontPanelIndicator::getInstance();
            }));


    EXPECT_CALL(indicatorMock, setState(::testing::_))
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
FrontPanelIndicatorMock frontPanelIndicatorMock;

    device::FrontPanelIndicator indicatorList;
    indicatorList.impl = &frontPanelIndicatorMock;
    ON_CALL(frontPanelConfigImplMock, getIndicators())
        .WillByDefault(::testing::Return(device::List<device::FrontPanelIndicator>({indicatorList})));
    ON_CALL(frontPanelIndicatorMock, getName())
        .WillByDefault(::testing::Return("red"));

    testing::NiceMock<FrontPanelIndicatorMock> indicatorMock;
    device::FrontPanelIndicator::getInstance().impl = &indicatorMock;
    
    ON_CALL(indicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("powerLedOn"), _T("{\"index\": \"power_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    ON_CALL(indicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Record", name);
                return device::FrontPanelIndicator::getInstance();
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("powerLedOn"), _T("{\"index\": \"record_led\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    ON_CALL(indicatorMock, getInstanceString)
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
    device::FrontPanelTextDisplay displayList;
    displayList.impl = &frontPanelTextDisplayImplMock;
    displayList.FrontPanelIndicator::impl = &frontPanelIndicatorImplMock;

    std::string test = "Text";

    ON_CALL(frontPanelConfigImplMock, getTextDisplay(test))
        .WillByDefault(::testing::ReturnRef(displayList));
    ON_CALL(frontPanelTextDisplayImplMock, getCurrentTimeFormat())
        .WillByDefault(::testing::Return(dsFPD_TIME_24_HOUR));
    EXPECT_CALL(frontPanelTextDisplayImplMock, setTimeFormat(::testing::_))
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
    testing::NiceMock<FrontPanelIndicatorMock> indicatorMock;
    device::FrontPanelIndicator::getInstance().impl = &indicatorMock;

    ON_CALL(indicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));

    ON_CALL(indicatorMock, getBrightness())
        .WillByDefault(::testing::Return(50));
    ON_CALL(frontPanelTextDisplayImplMock, getTextBrightness())
        .WillByDefault(::testing::Return(50));

    ON_CALL(indicatorMock, getName())
        .WillByDefault(::testing::Return("Power"));

    EXPECT_CALL(indicatorMock, setColorInt(::testing::_, ::testing::_))
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
    testing::NiceMock<FrontPanelIndicatorMock> indicatorMock;
    device::FrontPanelIndicator::getInstance().impl = &indicatorMock;

    ON_CALL(indicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));


    testing::NiceMock<ColorMock> colorMock;
    device::FrontPanelIndicator::Color::getInstance().impl = &colorMock;

    ON_CALL(colorMock, getInstanceByName)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator::Color& {
                EXPECT_EQ("red", name);
                return device::FrontPanelIndicator::Color::getInstance();
            }));
        
    device::FrontPanelTextDisplay displayList;
    displayList.impl = &frontPanelTextDisplayImplMock;
    displayList.FrontPanelIndicator::impl = &frontPanelIndicatorImplMock;

    std::string test = "Text";

    ON_CALL(frontPanelConfigImplMock, getTextDisplay(test))
        .WillByDefault(::testing::ReturnRef(displayList));
    ON_CALL(frontPanelTextDisplayImplMock, getTextBrightness())
        .WillByDefault(::testing::Return(100));
    

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setClockTestPattern"), _T("{\"show\": true, \"timeInterval\": 4}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setClockTestPattern"), _T("{\"show\": false, \"timeInterval\": 4}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

}

TEST_F(FrontPanelInitializedEventDsTest, setLEDMode1)
{

    testing::NiceMock<FrontPanelIndicatorMock> indicatorMock;
    device::FrontPanelIndicator::getInstance().impl = &indicatorMock;

    ON_CALL(indicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));

    ON_CALL(frontPanelIndicatorImplMock, getName())
        .WillByDefault(::testing::Return("Power"));
            
    EXPECT_CALL(indicatorMock, setColorInt(::testing::_, ::testing::_))
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

    testing::NiceMock<FrontPanelIndicatorMock> indicatorMock;
    device::FrontPanelIndicator::getInstance().impl = &indicatorMock;

    ON_CALL(indicatorMock, getInstanceString)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::FrontPanelIndicator& {
                EXPECT_EQ("Power", name);
                return device::FrontPanelIndicator::getInstance();
            }));
    

    EXPECT_CALL(indicatorMock, setColorInt(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](uint32_t color, bool toPersist) {
                EXPECT_EQ(color, 66051);
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setLED"), _T("{\"ledIndicator\": \"power_led\", \"brightness\": 50, \"red\": 1, \"green\": 2, \"blue\":3}"), response));

        EXPECT_EQ(response, string("{\"success\":true}"));
}


