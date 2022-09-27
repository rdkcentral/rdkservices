/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
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
**/

#include "FrontPanel.h"
#include "frontpanel.h"
#include <algorithm>

#include "frontPanelIndicator.hpp"
#include "frontPanelConfig.hpp"
#include "frontPanelTextDisplay.hpp"

#include "libIBus.h"
#include "rdk/iarmmgrs-hal/pwrMgr.h"

#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"

#define SERVICE_NAME "FrontPanelService"
#define METHOD_FP_SET_BRIGHTNESS "setBrightness"
#define METHOD_FP_GET_BRIGHTNESS "getBrightness"
#define METHOD_FP_POWER_LED_ON "powerLedOn"
#define METHOD_FP_POWER_LED_OFF "powerLedOff"
#define METHOD_CLOCK_SET_BRIGHTNESS "setClockBrightness"
#define METHOD_CLOCK_GET_BRIGHTNESS "getClockBrightness"
#define METHOD_GET_FRONT_PANEL_LIGHTS "getFrontPanelLights"
#define METHOD_FP_GET_PREFERENCES "getPreferences"
#define METHOD_FP_SET_PREFERENCES "setPreferences"
#define METHOD_FP_SET_LED "setLED"
#define METHOD_FP_SET_BLINK "setBlink"
#define METHOD_FP_SET_24_HOUR_CLOCK "set24HourClock"
#define METHOD_FP_IS_24_HOUR_CLOCK "is24HourClock"
#define METHOD_FP_SET_CLOCKTESTPATTERN "setClockTestPattern"

#define DATA_LED "data_led"
#define RECORD_LED "record_led"
#define POWER_LED "power_led"
#ifdef CLOCK_BRIGHTNESS_ENABLED
#define CLOCK_LED "clock_led"
#define TEXT_LED "Text"
#endif

#ifdef USE_EXTENDED_ALL_SEGMENTS_TEXT_PATTERN
#define ALL_SEGMENTS_TEXT_PATTERN "88:88"
#else
#define ALL_SEGMENTS_TEXT_PATTERN "8888"
#endif

#define DEFAULT_TEXT_PATTERN_UPDATE_INTERVAL 5

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 1

namespace
{

    struct Mapping
    {
        const char *IArmBusName;
        const char *SvcManagerName;
    };

    static struct Mapping name_mappings[] = {
        { "Record" , "record_led"},
        { "Message" , "data_led"},
        { "Power" , "power_led"},
        { "Text" , "clock_led"},
        // TODO: add your mappings here
        // { <IARM_NAME>, <SVC_MANAGER_API_NAME> },
        { 0,  0}
    };

    string svc2iarm(const string &name)
    {
        const char *s = name.c_str();

        int i = 0;
        while (name_mappings[i].SvcManagerName)
        {
            if (strcmp(s, name_mappings[i].SvcManagerName) == 0)
                return name_mappings[i].IArmBusName;
            i++;
        }
        return name;
    }

    string iarm2svc(const string &name)
    {
        const char *s = name.c_str();

        int i = 0;
        while (name_mappings[i].IArmBusName)
        {
            if (strcmp(s, name_mappings[i].IArmBusName) == 0)
                return name_mappings[i].SvcManagerName;
            i++;
        }
        return name;
    }

    JsonObject getFrontPanelIndicatorInfo(device::FrontPanelIndicator &indicator)
    {
        JsonObject returnResult;
        int levels=0, min=0, max=0;
        JsonObject indicatorInfo;
        string range;

        indicator.getBrightnessLevels(levels, min, max);
        range = (levels <=2)?"boolean":"int";
        indicatorInfo["range"] = range;

        indicatorInfo["min"] = JsonValue(min);
        indicatorInfo["max"] = JsonValue(max);

        if (range == ("int"))
        {
            indicatorInfo["step"] = JsonValue((max-min)/levels);
        }
        JsonArray availableColors;
        const device::List <device::FrontPanelIndicator::Color> colorsList = indicator.getSupportedColors();
        for (uint j = 0; j < colorsList.size(); j++)
        {
            availableColors.Add(colorsList.at(j).getName());
        }
        if (availableColors.Length() > 0)
        {
            indicatorInfo["colors"] = availableColors;
        }

        indicatorInfo["colorMode"] = indicator.getColorMode();
        return indicatorInfo;
    }
}

namespace WPEFramework
{
    namespace {

        static Plugin::Metadata<Plugin::FrontPanel> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }
    
    namespace Plugin
    {
        SERVICE_REGISTRATION(FrontPanel, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        FrontPanel* FrontPanel::_instance = nullptr;

        static Core::TimerType<TestPatternInfo> patternUpdateTimer(64 * 1024, "PatternUpdateTimer");
        int FrontPanel::m_savedClockBrightness = -1;
        int FrontPanel::m_LedDisplayPatternUpdateTimerInterval = DEFAULT_TEXT_PATTERN_UPDATE_INTERVAL;

        FrontPanel::FrontPanel()
        : PluginHost::JSONRPC()
        , m_updateTimer(this)
        {
            FrontPanel::_instance = this;

            Register(METHOD_FP_SET_BRIGHTNESS, &FrontPanel::setBrightnessWrapper, this);
            Register(METHOD_FP_GET_BRIGHTNESS, &FrontPanel::getBrightnessWrapper, this);
            Register(METHOD_FP_POWER_LED_ON, &FrontPanel::powerLedOnWrapper, this);
            Register(METHOD_FP_POWER_LED_OFF, &FrontPanel::powerLedOffWrapper, this);
            Register(METHOD_CLOCK_SET_BRIGHTNESS, &FrontPanel::setClockBrightnessWrapper, this);
            Register(METHOD_CLOCK_GET_BRIGHTNESS, &FrontPanel::getClockBrightnessWrapper, this);
            Register(METHOD_GET_FRONT_PANEL_LIGHTS, &FrontPanel::getFrontPanelLightsWrapper, this);
            Register(METHOD_FP_GET_PREFERENCES, &FrontPanel::getPreferencesWrapper, this);
            Register(METHOD_FP_SET_PREFERENCES, &FrontPanel::setPreferencesWrapper, this);
            Register(METHOD_FP_SET_LED, &FrontPanel::setLEDWrapper, this);
            Register(METHOD_FP_SET_BLINK, &FrontPanel::setBlinkWrapper, this);
            Register(METHOD_FP_SET_24_HOUR_CLOCK, &FrontPanel::set24HourClockWrapper, this);
            Register(METHOD_FP_IS_24_HOUR_CLOCK, &FrontPanel::is24HourClockWrapper, this);
            Register(METHOD_FP_SET_CLOCKTESTPATTERN, &FrontPanel::setClockTestPatternWrapper, this);

            InitializeIARM();

            CFrontPanel::instance()->start();
            CFrontPanel::instance()->addEventObserver(this);
            loadPreferences();
        }

        FrontPanel::~FrontPanel()
        {
        }

        void FrontPanel::Deinitialize(PluginHost::IShell* /* service */)
        {
            FrontPanel::_instance = nullptr;

            {
                std::lock_guard<std::mutex> lock(m_updateTimerMutex);
                m_runUpdateTimer = false;
            }
            patternUpdateTimer.Revoke(m_updateTimer);

            DeinitializeIARM();
        }
        void FrontPanel::powerModeChange(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if (strcmp(owner, IARM_BUS_PWRMGR_NAME)  == 0) {
               if (eventId == IARM_BUS_PWRMGR_EVENT_MODECHANGED ) {
                   IARM_Bus_PWRMgr_EventData_t *param = (IARM_Bus_PWRMgr_EventData_t *)data;
                   LOGINFO("Event IARM_BUS_PWRMGR_EVENT_MODECHANGED: State Changed %d -- > %d\r",
                               param->data.state.curState, param->data.state.newState);
                   if(param->data.state.newState == IARM_BUS_PWRMGR_POWERSTATE_ON)
                   {
                       LOGINFO("setPowerStatus true");
                       CFrontPanel::instance()->setPowerStatus(true);
                   }
                   else
                   {
                       LOGINFO("setPowerStatus false");
                       CFrontPanel::instance()->setPowerStatus(false);
                   }
               }
            }
        }

        const void FrontPanel::InitializeIARM()
        {
            if (Utils::IARM::init())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME,IARM_BUS_PWRMGR_EVENT_MODECHANGED, powerModeChange) );
            }
        }

        void FrontPanel::DeinitializeIARM()
        {
           if (Utils::IARM::isConnected())
           {
              IARM_Result_t res;
              IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_PWRMGR_NAME,IARM_BUS_PWRMGR_EVENT_MODECHANGED) );
           }
        }

        void setResponseArray(JsonObject& response, const char* key, const vector<string>& items)
        {
            JsonArray arr;
            for (auto& i : items) arr.Add(JsonValue(i));

            response[key] = arr;

            string json;
            response.ToString(json);
            LOGINFO("%s: result json %s\n", __FUNCTION__, json.c_str());
        }

        /**
         * @brief sets the brightness of the specified LED. Brightness must be a value support by
         * the LED and the value of the brightness for this led must be persisted.
         *
         * @param[in] brightness Brightness value.
         *
         * @return Returns the result which will be true when the call to set brightness succeeded.
         * @ingroup SERVMGR_FRONTPANEL_API
         */
        bool FrontPanel::setBrightness( int brightness )
        {
            bool ok;
            ok = CFrontPanel::instance()->setBrightness(brightness);
            return ok;
        }

        uint32_t FrontPanel::setBrightnessWrapper(const JsonObject& parameters, JsonObject& response)
        {
            CFrontPanel::instance()->stopBlinkTimer();
            int brightness = -1;
            bool ok = false;

            if (!parameters.HasLabel("brightness"))
            {
                LOGERR("Parameter 'brightness' wasn't passed");
                returnResponse(ok);
            }

            getNumberParameter("brightness", brightness);

            if (parameters.HasLabel("index"))
            {
                /* frontpanel_3 */
                if (parameters.HasLabel("index"))
                {
                    string fp_ind;
                    fp_ind = svc2iarm(parameters["index"].String());
                    LOGWARN("FP calling setBrightness of %s", fp_ind.c_str());

                    if (brightness >= 0)
                    {
                        LOGWARN("FP calling setBrightness of %s to %d", fp_ind.c_str(), brightness);
#ifdef CLOCK_BRIGHTNESS_ENABLED
                        if (TEXT_LED == fp_ind)
                        {
                            setClockBrightness(int(brightness));
                            ok = true;
                        }
                        else
#endif
                        {
                            try
                            {
                                device::FrontPanelIndicator::getInstance(fp_ind.c_str()).setBrightness(int(brightness));
                                ok = true;
                            }
                            catch (...)
                            {
                                ok = false;
                            }
                        }
                    }
                }
            }
            else
            {

                if (brightness >= 0 && brightness <= 100)
                {
                    LOGWARN("calling setBrightness");
                    ok = setBrightness(brightness);
                }
                else
                {
                    LOGWARN("Invalid brightnessLevel passed to method setBrightness CallMethod");
                }
            }

            returnResponse(ok);
        }

        /**
         * @brief Gets the brightness value of the specified LED.
         *
         * @return brightness Integer.
         */
        int FrontPanel::getBrightness()
        {
            int brightness = -1;
            brightness = CFrontPanel::instance()->getBrightness();
            return brightness;
        }

        /**
         * @brief Gets the brightness of the specified LED.
         *
         * @param[in] argList List of arguments (Not used).
         *
         * @return Returns a ServiceParams object containing brightness value and function result.
         * @ingroup SERVMGR_FRONTPANEL_API
         */
        uint32_t FrontPanel::getBrightnessWrapper(const JsonObject& parameters, JsonObject& response)
        {
            int brightness = -1;
            bool ok = false;
            LOGWARN("calling getBrightness");

            if (parameters.HasLabel("index"))
            {
                string fp_ind;
                fp_ind = svc2iarm(parameters["index"].String());
                LOGWARN("FP3 calling getBrightness of %s", fp_ind.c_str());
#ifdef CLOCK_BRIGHTNESS_ENABLED
                if (TEXT_LED == fp_ind)
                {
                    brightness = getClockBrightness();
                }
                else
#endif
                    try
                    {
                        brightness = device::FrontPanelIndicator::getInstance(fp_ind.c_str()).getBrightness();
                    }
                    catch (...)
                    {
                        LOGWARN("Exception thrown from ds while calling getBrightness");
                    }
            }
            else
                brightness = getBrightness();

            response["brightness"] = brightness;
            ok = brightness >= 0 ? true : false;

            returnResponse(ok);
        }

        /**
         * @brief This function is used to switches ON the particular LED. The LED must be powere ON
         * prior to setting its brightness. It is done by invoking the powerOnLed function of CFrontPanel.
         *
         * @param[in] fp_indicator Possible enum values are "0"-clock, "1"-message, "2"-power, "3"-record,
         * "4"-remote etc.
         *
         * @return true if the LED is switches ON successfully else false.
         */
        bool FrontPanel::powerLedOn(frontPanelIndicator fp_indicator)
        {
            return CFrontPanel::instance()->powerOnLed(fp_indicator);
        }

        uint32_t FrontPanel::powerLedOnWrapper(const JsonObject& parameters, JsonObject& response )
        {
            string fp_ind;
            bool ok = false;

            if (parameters.HasLabel("index"))
                fp_ind = parameters["index"].String();
            if (fp_ind.compare(DATA_LED) == 0)
            {
                LOGWARN("calling powerOnLed");
                ok = powerLedOn(FRONT_PANEL_INDICATOR_MESSAGE);
            }
            else if (fp_ind.compare(RECORD_LED) == 0)
            {
                LOGWARN("calling powerOnLed");
                ok = powerLedOn(FRONT_PANEL_INDICATOR_RECORD);
            }
	    else if (fp_ind.compare(POWER_LED) == 0)
            {
                LOGWARN("calling powerOnLed");
                ok = powerLedOn(FRONT_PANEL_INDICATOR_POWER);
            }
            returnResponse(ok);
        }

        /**
         * @brief This function is used to switches OFF the particular LED. It is done by invoking
         * the powerOffLed function of CFrontPanel.
         *
         * @param[in] fp_indicator Possible enum values are "0"-clock, "1"-message, "2"-power, "3"-record,
         * "4"-remote etc.
         *
         * @return true if the LED is switches OFF successfully else false.
         */
        bool FrontPanel::powerLedOff(frontPanelIndicator fp_indicator)
        {
            bool ok;
            ok = CFrontPanel::instance()->powerOffLed(fp_indicator);
            return ok;
        }

        uint32_t FrontPanel::powerLedOffWrapper(const JsonObject& parameters, JsonObject& response)
        {
            string fp_ind;
            bool ok = false;

            if (parameters.HasLabel("index"))
                fp_ind = parameters["index"].String();
            if (fp_ind.compare(DATA_LED) == 0)
            {
                LOGWARN("calling powerOffLed");
                ok = powerLedOff(FRONT_PANEL_INDICATOR_MESSAGE);
            }
            else if (fp_ind.compare(RECORD_LED) == 0)
            {
                LOGWARN("calling powerOffLed");
                ok = powerLedOff(FRONT_PANEL_INDICATOR_RECORD);
            }
	    else if (fp_ind.compare(POWER_LED) == 0)
            {
                LOGWARN("calling powerOffLed");
                ok = powerLedOff(FRONT_PANEL_INDICATOR_POWER);
            }
            returnResponse(ok);
        }

        bool FrontPanel::setClockBrightness(int brightness )
        {
#ifdef CLOCK_BRIGHTNESS_ENABLED
            bool ok;
            ok = CFrontPanel::instance()->setClockBrightness(brightness);
            return ok;
#else
            return false;
#endif
        }

        uint32_t FrontPanel::setClockBrightnessWrapper(const JsonObject& parameters, JsonObject& response)
        {
            bool ok = false;

#ifdef CLOCK_BRIGHTNESS_ENABLED
            int brightness = -1;

            if (parameters.HasLabel("brightness"))
                getNumberParameter("brightness", brightness);

            if (brightness >= 0 && brightness <= 100)
            {
                LOGWARN("calling setClockBrightness");
                ok = setClockBrightness(brightness);
            }
            else
            {
                LOGWARN("Invalid brightnessLevel passed to method setBrightness CallMethod");
            }
#else
            LOGWARN("No operation for setClockBrightness");
#endif


            returnResponse(ok);
        }


        /**
         * @brief get the clock brightness of the specified LED. Brightness must be a value support by the
         * LED and the value of the brightness for this led must be persisted.
         *
         * @return The brightness integer value if clock brightness enable macro is true else it will return -1.
         */
        int FrontPanel::getClockBrightness()
        {
#ifdef CLOCK_BRIGHTNESS_ENABLED
            int brightness = -1;
            brightness = CFrontPanel::instance()->getClockBrightness();
            return brightness;
#else
            return -1;
#endif
        }

        uint32_t FrontPanel::getClockBrightnessWrapper(const JsonObject& parameters, JsonObject& response)
        {
            bool ok = false;
#ifdef CLOCK_BRIGHTNESS_ENABLED
            int brightness = -1;
            LOGWARN("calling getClockBrightness");
            brightness = getClockBrightness();
            if (0 <= brightness)
                ok = true;
            response["brightness"] = brightness;
#else
            LOGWARN("No operation for getClockBrightness");
#endif
            returnResponse(ok);
        }

        /**
         * @brief getFrontPanelLights This returns an object containing attributes of front panel
         * light: success, supportedLights, and supportedLightsInfo.
         * supportedLights defines the LED lights that can be controlled through the Front Panel API.
         * supportedLightsInfo defines a hash of objects describing each LED light.
         * success - false if the supported lights info was unable to be determined.
         *
         * @return Returns a list of front panel lights parameter.
         * @ingroup SERVMGR_FRONTPANEL_API
         */
        std::vector<std::string> FrontPanel::getFrontPanelLights()
        {
            std::vector<std::string> lights;
            device::List <device::FrontPanelIndicator> fpIndicators = device::FrontPanelConfig::getInstance().getIndicators();
            for (uint i = 0; i < fpIndicators.size(); i++)
            {
                string IndicatorNameIarm = fpIndicators.at(i).getName();
                string MappedName = iarm2svc(IndicatorNameIarm);
                if (MappedName != IndicatorNameIarm) lights.push_back(MappedName);
            }
#ifdef CLOCK_BRIGHTNESS_ENABLED
            try
            {
                device::List <device::FrontPanelTextDisplay> fpTextDisplays = device::FrontPanelConfig::getInstance().getTextDisplays();
                for (uint i = 0; i < fpTextDisplays.size(); i++)
                {
                    string TextDisplayNameIarm = fpTextDisplays.at(i).getName();
                    string MappedName = iarm2svc(TextDisplayNameIarm);
                    if (MappedName != TextDisplayNameIarm)
                    {
                        lights.push_back(MappedName);
                    }
                }
            }
            catch (...)
            {
                LOGERR("Exception while getFrontPanelLights");
            }
#endif
            return lights;
        }

        /**
         * @brief getFrontPanelLightsInfo This returns an object containing attributes of front
         * panel light: success, supportedLights, and supportedLightsInfo.
         * supportedLightsInfo defines a hash of objects describing each LED light properties such as
         * -"range" Determines the types of values that can be expected in min and max value.
         * -"min" The minimum value is equivalent to off i.e "0".
         * -"max" The maximum value is when the LED is on i.e "1" and at its brightest.
         * -"step" The step or interval between the min and max values supported by the LED.
         * -"colorMode" Defines enum of "0" LED's color cannot be changed, "1"  LED can be set to any color
         * (using rgb-hex code),"2"  LED can be set to an enumeration of colors as specified by the
         * supportedColors property.
         *
         * @return Returns a serviceParams list of front panel lights info.
         */
        JsonObject FrontPanel::getFrontPanelLightsInfo()
        {
            JsonObject returnResult;
            JsonObject indicatorInfo;
            string IndicatorNameIarm, MappedName;

            device::List <device::FrontPanelIndicator> fpIndicators = device::FrontPanelConfig::getInstance().getIndicators();
            for (uint i = 0; i < fpIndicators.size(); i++)
            {
                IndicatorNameIarm = fpIndicators.at(i).getName();
                MappedName = iarm2svc(IndicatorNameIarm);
                if (MappedName != IndicatorNameIarm)
                {
                    indicatorInfo = getFrontPanelIndicatorInfo(fpIndicators.at(i));
                    returnResult[MappedName.c_str()] = indicatorInfo;
                }
            }

#ifdef CLOCK_BRIGHTNESS_ENABLED
            try
            {
                indicatorInfo = getFrontPanelIndicatorInfo(device::FrontPanelConfig::getInstance().getTextDisplay(0));
                returnResult[CLOCK_LED] = indicatorInfo;
            }
            catch (...)
            {
                LOGERR("Exception while getFrontPanelLightsInfo");
            }
#endif
            return returnResult;
        }


        uint32_t FrontPanel::getFrontPanelLightsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            /*CID 17850  removing logically dead code */

            LOGWARN("sending getFrontPanelLights");
            setResponseArray(response, "supportedLights", getFrontPanelLights());
            response["supportedLightsInfo"] = getFrontPanelLightsInfo();
            returnResponse(true);
        }


        /**
         * @brief Returns the preferences object as stored in setPreferences.
         *
         * @return Attribute key value pair list.
         */
        JsonObject FrontPanel::getPreferences()
        {
            return CFrontPanel::instance()->getPreferences();
        }

        /**
         * @brief This method stores the preferences into persistent storage.
         * It does not change the colour of any LEDs. It invoking the setPreferences
         * method of cFrontPanel class to set the preference data.
         *
         * @param[in] preferences Key value pair of preferences data.
         *
         * @return Returns the success code of underlying method.
         */
        void FrontPanel::setPreferences(const JsonObject& preferences)
        {
            CFrontPanel::instance()->setPreferences(preferences);
        }

        void FrontPanel::loadPreferences()
        {
            CFrontPanel::instance()->loadPreferences();
        }

        uint32_t FrontPanel::getPreferencesWrapper(const JsonObject& parameters, JsonObject& response)
        {
            response["preferences"] = getPreferences();
            returnResponse(true);
        }

        /**
         * @brief This method stores the preferences into persistent storage.
         * It does not change the color of any LEDs. The preferences object is not validated.
         * It is up to the client of this API to ensure that preference values are valid.
         *
         * @param[in] argList List of preferences.
         * @return Returns the success code of underlying method.
         * @ingroup SERVMGR_FRONTPANEL_API
         */
        uint32_t FrontPanel::setPreferencesWrapper(const JsonObject& parameters, JsonObject& response)
        {
            bool success = false;

            if (parameters.HasLabel("preferences"))
            {
                JsonObject preferences = parameters["preferences"].Object();
                setPreferences(preferences);
                success = true;
            }
            returnResponse(success);
        }

        /**
         * @brief Sets the brightness and color properties of the specified LED.
         * The supported properties of the info object passed in will be determined by the color
         * mode of the LED. If the colorMode of an LED is 0 color values will be ignored. If the
         * brightness of the LED is unspecified or value = -1, then the persisted or default
         * value for the system is used.
         *
         * @param[in] properties Key value pair of properties data.
         *
         * @return Returns success value of the helper method, returns false in case of failure.
         */
        bool FrontPanel::setLED(const JsonObject& properties)
        {
            bool success = false;
            success = CFrontPanel::instance()->setLED(properties);
            return success;
        }

        /**
         * @brief Specifies a blinking pattern for an LED. This method returns immediately, but starts
         * a process of iterating through each element in the array and lighting the LED with the specified
         * brightness and color (if applicable) for the given duration (in milliseconds).
         *
         * @param[in] blinkInfo Object containing Indicator name, blink pattern and duration.
         * @ingroup SERVMGR_FRONTPANEL_API
         */
        void FrontPanel::setBlink(const JsonObject& blinkInfo)
        {
            CFrontPanel::instance()->setBlink(blinkInfo);
        }

        /**
         * @brief Specifies the 24 hour clock format.
         *
         * @param[in] is24Hour true if 24 hour clock format.
         * @ingroup SERVMGR_FRONTPANEL_API
         */
        void FrontPanel::set24HourClock(bool is24Hour)
        {
            CFrontPanel::instance()->set24HourClock(is24Hour);
        }

        /**
         * @brief Get the 24 hour clock format.
         *
         * @return true if 24 hour clock format is used.
         * @ingroup SERVMGR_FRONTPANEL_API
         */
        bool FrontPanel::is24HourClock()
        {
            bool is24Hour = false;
            is24Hour = CFrontPanel::instance()->is24HourClock();
            return is24Hour;
        }


        /**
         * @brief Enable or disable showing test pattern 88:88 on stbs with clock displays.
         *
         * @param[in] show true to show pattern, false to restore display to default behavior, usual it's clock.
         * @param[in] interval (optional) interval in seconds to check and update LED display with pattern, when it's overridden by external API.
         *            from 1 to 60 seconds. 0 and other outbound values mean that timer isn't used and isn't activated by this call.
         *            Optionally the timer is enabled for 5 seconds interval.
         * @return true if method succeeded.
         * @ingroup SERVMGR_FRONTPANEL_API
         */
        void FrontPanel::setClockTestPattern(bool show)
        {
        #ifdef CLOCK_BRIGHTNESS_ENABLED
            try{
                device::FrontPanelTextDisplay& display = device::FrontPanelConfig::getInstance().getTextDisplay("Text");

                if (show)
                {
                    if (m_LedDisplayPatternUpdateTimerInterval > 0 && m_LedDisplayPatternUpdateTimerInterval < 61)
                    {
                        {
                            std::lock_guard<std::mutex> lock(m_updateTimerMutex);
                            m_runUpdateTimer = true;
                        }
                        patternUpdateTimer.Schedule(Core::Time::Now().Add(m_LedDisplayPatternUpdateTimerInterval * 1000), m_updateTimer);

                        LOGWARN("%s: LED FP display update timer activated with interval %ds", __FUNCTION__, m_LedDisplayPatternUpdateTimerInterval);
                    }
                    else
                    {
                        LOGWARN("%s: LED FP display update timer didn't used for interval value %d. To activate it, interval should be in bound of values from 1 till 60"
                                , __FUNCTION__, m_LedDisplayPatternUpdateTimerInterval);

                        {
                            std::lock_guard<std::mutex> lock(m_updateTimerMutex);
                            m_runUpdateTimer = false;
                        }
                        patternUpdateTimer.Revoke(m_updateTimer);
                    }

                    if (-1 == m_savedClockBrightness)
                    {
                        m_savedClockBrightness = getClockBrightness();
                        LOGWARN("%s: brightness of LED FP display %d was saved", __FUNCTION__, m_savedClockBrightness);
                    }

                    display.setMode(1); //Set Front Panel Display to Text Mode
                    display.setText(ALL_SEGMENTS_TEXT_PATTERN);
                    setClockBrightness(100);
                    LOGWARN("%s: pattern " ALL_SEGMENTS_TEXT_PATTERN " activated on LED FP display with max brightness", __FUNCTION__);
                }
                else
                {
                    {
                        std::lock_guard<std::mutex> lock(m_updateTimerMutex);
                        m_runUpdateTimer = false;
                    }
                    patternUpdateTimer.Revoke(m_updateTimer);

                    display.setMode(0);//Set Front Panel Display to Default Mode
                    display.setText("    ");
                    LOGWARN("%s: pattern " ALL_SEGMENTS_TEXT_PATTERN " deactivated on LED FP display", __FUNCTION__);

                    if (-1 != m_savedClockBrightness)
                    {
                        setClockBrightness(m_savedClockBrightness);
                        LOGWARN("%s: brightness %d of LED FP display restored", __FUNCTION__, m_savedClockBrightness);
                        m_savedClockBrightness = -1;
                    }
                }
            }
            catch (...)
            {
                LOGERR("Exception while getTextDisplay");
            }
        #else
            LOGWARN("%s: disabled for this platform", __FUNCTION__);
        #endif
        }




        uint32_t FrontPanel::setLEDWrapper(const JsonObject& parameters, JsonObject& response)
        {
            bool success = false;

            /*if (parameters.HasLabel("properties"))
            {
                JsonObject properties = parameters["properties"].Object();
                success = setLED(properties);
            }*/
            success = setLED(parameters);
            returnResponse(success);
        }

        uint32_t FrontPanel::setBlinkWrapper(const JsonObject& parameters, JsonObject& response)
        {
            bool success = false;

            if (parameters.HasLabel("blinkInfo"))
            {
                JsonObject blinkInfo = parameters["blinkInfo"].Object();
                setBlink(blinkInfo);
                success = true;
            }
            returnResponse(success);
        }

        uint32_t FrontPanel::set24HourClockWrapper(const JsonObject& parameters, JsonObject& response)
        {
            bool success = false;

            if (parameters.HasLabel("is24Hour"))
            {
                bool is24Hour = false;
                getBoolParameter("is24Hour", is24Hour );
                set24HourClock(is24Hour);
                success = true;
            }
            returnResponse(success);
        }

        uint32_t FrontPanel::is24HourClockWrapper(const JsonObject& parameters, JsonObject& response)
        {
            bool is24Hour = is24HourClock();
            response["is24Hour"] = is24Hour;
            returnResponse(true);
        }

        uint32_t FrontPanel::setClockTestPatternWrapper(const JsonObject& parameters, JsonObject& response)
        {
            if (!parameters.HasLabel("show"))
            {
                LOGWARN("'show' parameter wasn't passed");
                returnResponse(false);
            }

            bool show = false;
            getBoolParameter("show", show);

            m_LedDisplayPatternUpdateTimerInterval = DEFAULT_TEXT_PATTERN_UPDATE_INTERVAL;
            if (parameters.HasLabel("timerInterval"))
                getNumberParameter("timerInterval", m_LedDisplayPatternUpdateTimerInterval);

            setClockTestPattern(show);

            returnResponse(true);
        }

        void FrontPanel::updateLedTextPattern()
        {
            LOGWARN("%s: override FP LED display with text pattern " ALL_SEGMENTS_TEXT_PATTERN, __FUNCTION__);

            if (getClockBrightness() != 100)
            {
                setClockBrightness(100);
            }

            device::FrontPanelConfig::getInstance().getTextDisplay("Text").setText(ALL_SEGMENTS_TEXT_PATTERN);
            LOGWARN("%s: LED display updated by pattern " ALL_SEGMENTS_TEXT_PATTERN, __FUNCTION__);

            {
                std::lock_guard<std::mutex> lock(m_updateTimerMutex);
                if (m_runUpdateTimer)
                    patternUpdateTimer.Schedule(Core::Time::Now().Add(m_LedDisplayPatternUpdateTimerInterval * 1000), m_updateTimer);
            }
        }

        uint64_t TestPatternInfo::Timed(const uint64_t scheduledTime)
        {
            uint64_t result = 0;
            m_frontPanel->updateLedTextPattern();
            return(result);
        }


    } // namespace Plugin
} // namespace WPEFramework
