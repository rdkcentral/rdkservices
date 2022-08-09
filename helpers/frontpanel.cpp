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

/**
* @defgroup servicemanager
* @{
* @defgroup src
* @{
**/

//#define USE_DS //TODO - this was defined in servicemanager.pro for all STB builds.  Not sure where to put it except here for now
//#define HAS_API_POWERSTATE

#include "frontpanel.h"
#ifdef USE_DS
    #include "frontPanelConfig.hpp"
    #include "frontPanelTextDisplay.hpp"
    #include "manager.hpp"
#endif

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>

#if defined(HAS_API_POWERSTATE)
#include "libIBus.h"
#include "pwrMgr.h"
#endif

#include "UtilsJsonRpc.h"
#include "UtilsLogging.h"
#include "UtilssyncPersistFile.h"

#define FP_SETTINGS_FILE_JSON "/opt/fp_service_preferences.json"

/*
Requirement now
    Ability to get/set Led brightness
    Ability to power off/on a led

*/

namespace WPEFramework
{

    namespace Plugin
    {
        CFrontPanel* CFrontPanel::s_instance = NULL;
        static int globalLedBrightness = 100;
#ifdef CLOCK_BRIGHTNESS_ENABLED
        static int clockBrightness = 100;
#endif
        static int initDone = 0;
        static bool isMessageLedOn = false;
        static bool isRecordLedOn = false;
#ifdef CLOCK_BRIGHTNESS_ENABLED
        static bool isClockOn;
#endif
        static bool powerStatus = false;     //Check how this works on xi3 and rng's
        static bool started = false;
        static int m_numberOfBlinks = 0;
        static int m_maxNumberOfBlinkRepeats = 0;
        static int m_currentBlinkListIndex = 0;
        static std::vector<std::string> m_lights;
        static device::List <device::FrontPanelIndicator> fpIndicators;

        static Core::TimerType<BlinkInfo> blinkTimer(64 * 1024, "BlinkTimer");

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
                // TODO: add your mappings here
                // { <IARM_NAME>, <SVC_MANAGER_API_NAME> },
                { 0,  0}
            };

            std::string svc2iarm(const std::string &name)
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
        }

        CFrontPanel::CFrontPanel()
        : m_blinkTimer(this)
        , m_isBlinking(false)
        , mFrontPanelHelper(new FrontPanelHelper())
        {
        }

        CFrontPanel* CFrontPanel::instance()
        {
            if (!initDone)
            {
                if (!s_instance)
                    s_instance = new CFrontPanel;

#ifdef USE_DS
                try
                {
                    LOGINFO("Front panel init");
                    fpIndicators = device::FrontPanelConfig::getInstance().getIndicators();

                    for (uint i = 0; i < fpIndicators.size(); i++)
                    {
                        std::string IndicatorNameIarm = fpIndicators.at(i).getName();

                        auto it = std::find(m_lights.begin(), m_lights.end(), IndicatorNameIarm);
                        if (m_lights.end() == it)
                        {
                            m_lights.push_back(IndicatorNameIarm);
                        }
                    }

#if defined(HAS_API_POWERSTATE)
                    {
                        IARM_Bus_PWRMgr_GetPowerState_Param_t param;
                        IARM_Result_t res = IARM_Bus_Call(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_API_GetPowerState,
                            (void*)&param, sizeof(param));

                        if (res == IARM_RESULT_SUCCESS) {
                            if (param.curState == IARM_BUS_PWRMGR_POWERSTATE_ON)
                                powerStatus = true;
                        }
                    }
#endif
#ifdef CLOCK_BRIGHTNESS_ENABLED
                    clockBrightness =  device::FrontPanelTextDisplay::getInstance("Text").getTextBrightness();
                    device::FrontPanelTextDisplay::getInstance("Text").setTextBrightness(clockBrightness);
#endif
                    globalLedBrightness = device::FrontPanelIndicator::getInstance("Power").getBrightness();
                    LOGINFO("Power light brightness, %d, power status %d", globalLedBrightness, powerStatus);

                    for (uint i = 0; i < fpIndicators.size(); i++)
                    {
                        LOGWARN("Initializing light %s", fpIndicators.at(i).getName().c_str());
                        if (powerStatus)
                            device::FrontPanelIndicator::getInstance(fpIndicators.at(i).getName()).setBrightness(globalLedBrightness);

                        device::FrontPanelIndicator::getInstance(fpIndicators.at(i).getName()).setState(false);
                    }

                    if (powerStatus)
                        device::FrontPanelIndicator::getInstance("Power").setState(true);

                }
                catch (...)
                {
                    LOGERR("Exception Caught during [CFrontPanel::instance]\r\n");
                }
                initDone=1;
#endif
            }

            return s_instance;
        }

        bool CFrontPanel::start()
        {
            LOGWARN("Front panel start");
            try
            {
                if (powerStatus)
                    device::FrontPanelIndicator::getInstance("Power").setState(true);

                device::List <device::FrontPanelIndicator> fpIndicators = device::FrontPanelConfig::getInstance().getIndicators();
                for (uint i = 0; i < fpIndicators.size(); i++)
                {
                    std::string IndicatorNameIarm = fpIndicators.at(i).getName();

                    auto it = std::find(m_lights.begin(), m_lights.end(), IndicatorNameIarm);
                    if (m_lights.end() == it)
                        m_lights.push_back(IndicatorNameIarm);
                }
            }
            catch (...)
            {
                LOGERR("Frontpanel Exception Caught during [%s]\r\n", __func__);
            }
            if (!started)
            {
                m_numberOfBlinks = 0;
                m_maxNumberOfBlinkRepeats = 0;
                m_currentBlinkListIndex = 0;
                started = true;
            }
            return true;
        }

        bool CFrontPanel::stop()
        {
            stopBlinkTimer();
            return true;
        }

        void CFrontPanel::setPowerStatus(bool bPowerStatus)
        {
            powerStatus = bPowerStatus;
        }

        std::string CFrontPanel::getLastError()
        {
            return lastError_;
        }

        void CFrontPanel::addEventObserver(FrontPanel* o)
        {

            auto it = std::find(observers_.begin(), observers_.end(), o);

            if (observers_.end() == it)
                observers_.push_back(o);
        }

        void CFrontPanel::removeEventObserver(FrontPanel* o)
        {
            observers_.remove(o);
        }

        bool CFrontPanel::setBrightness(int fp_brightness)
        {
            stopBlinkTimer();
            globalLedBrightness = fp_brightness;

            try
            {
                for (uint i = 0; i < fpIndicators.size(); i++)
                {
                    device::FrontPanelIndicator::getInstance(fpIndicators.at(i).getName()).setBrightness(globalLedBrightness);
                }
            }
            catch (...)
            {
                LOGERR("Frontpanel Exception Caught during [%s]\r\n",__func__);
            }

            powerOnLed(FRONT_PANEL_INDICATOR_ALL);
            return true;
        }

        int CFrontPanel::getBrightness()
        {
            try
            {
                globalLedBrightness = device::FrontPanelIndicator::getInstance("Power").getBrightness();
                LOGWARN("Power light brightness, %d\n", globalLedBrightness);
            }
            catch (...)
            {
                LOGERR("Frontpanel Exception Caught during [%s]\r\n", __func__);
            }

            return globalLedBrightness;
        }

#ifdef CLOCK_BRIGHTNESS_ENABLED
        bool CFrontPanel::setClockBrightness(int brightness)
        {
            clockBrightness = brightness;
            powerOnLed(FRONT_PANEL_INDICATOR_CLOCK);
            return true;
        }

        int CFrontPanel::getClockBrightness()
        {
            try
            {
                clockBrightness =  device::FrontPanelTextDisplay::getInstance("Text").getTextBrightness();
            }
            catch (...)
            {
                LOGERR("FrontPanel Exception Caught during [%s]\r\n", __func__);
            }

            return clockBrightness;
        }
#endif

        bool CFrontPanel::powerOnLed(frontPanelIndicator fp_indicator)
        {
            stopBlinkTimer();
            try
            {
                if (powerStatus)
                {
                    switch (fp_indicator)
                    {
                    case FRONT_PANEL_INDICATOR_CLOCK:
#ifdef CLOCK_BRIGHTNESS_ENABLED
                        isClockOn = true;
                        device::FrontPanelTextDisplay::getInstance("Text").setTextBrightness(clockBrightness);
#endif
                        break;
                    case FRONT_PANEL_INDICATOR_MESSAGE:
                        isMessageLedOn = true;
                        device::FrontPanelIndicator::getInstance("Message").setState(true);
                        break;
                    case FRONT_PANEL_INDICATOR_RECORD:
                        isRecordLedOn = true;
                        device::FrontPanelIndicator::getInstance("Record").setState(true);
                        break;
                    case FRONT_PANEL_INDICATOR_REMOTE:
                        device::FrontPanelIndicator::getInstance("Remote").setState(true);
                        break;
                    case FRONT_PANEL_INDICATOR_RFBYPASS:
                        device::FrontPanelIndicator::getInstance("RfByPass").setState(true);
                        break;
                    case FRONT_PANEL_INDICATOR_ALL:
                        if (isMessageLedOn)
                            device::FrontPanelIndicator::getInstance("Message").setState(true);
                        if (isRecordLedOn)
                            device::FrontPanelIndicator::getInstance("Record").setState(true);
                        device::FrontPanelIndicator::getInstance("Power").setState(true);
                        break;
                    case FRONT_PANEL_INDICATOR_POWER:
                        //LOGWARN("CFrontPanel::powerOnLed() - FRONT_PANEL_INDICATOR_POWER not handled");
			device::FrontPanelIndicator::getInstance("Power").setState(true);
                        break;
                    }
                }
            }
            catch (...)
            {
                LOGERR("FrontPanel Exception Caught during [%s]\r\n", __func__);
                return false;
            }
            return true;
        }

        bool CFrontPanel::powerOffLed(frontPanelIndicator fp_indicator)
        {
            stopBlinkTimer();
            try
            {
                switch (fp_indicator)
                {
                case FRONT_PANEL_INDICATOR_CLOCK:
#ifdef CLOCK_BRIGHTNESS_ENABLED
                    isClockOn = false;
                    device::FrontPanelTextDisplay::getInstance("Text").setTextBrightness(0);
#endif
                    break;
                case FRONT_PANEL_INDICATOR_MESSAGE:
                    isMessageLedOn = false;
                    device::FrontPanelIndicator::getInstance("Message").setState(false);
                    break;
                case FRONT_PANEL_INDICATOR_RECORD:
                    isRecordLedOn = false;
                    device::FrontPanelIndicator::getInstance("Record").setState(false);
                    break;
                case FRONT_PANEL_INDICATOR_REMOTE:
                    device::FrontPanelIndicator::getInstance("Remote").setState(false);
                    break;
                case FRONT_PANEL_INDICATOR_RFBYPASS:
                    device::FrontPanelIndicator::getInstance("RfByPass").setState(false);
                    break;
                case FRONT_PANEL_INDICATOR_ALL:
                    for (uint i = 0; i < fpIndicators.size(); i++)
                    {
                        //LOGWARN("powerOffLed for Indicator %s", QString::fromStdString(fpIndicators.at(i).getName()).toUtf8().constData());
                        LOGWARN("powerOffLed for Indicator %s", fpIndicators.at(i).getName().c_str());
                        device::FrontPanelIndicator::getInstance(fpIndicators.at(i).getName()).setState(false);
                    }
                    break;
                case FRONT_PANEL_INDICATOR_POWER:
                    //LOGWARN("CFrontPanel::powerOffLed() - FRONT_PANEL_INDICATOR_POWER not handled");
		    device::FrontPanelIndicator::getInstance("Power").setState(false);
                    break;
                }
            }
            catch (...)
            {
                LOGERR("FrontPanel Exception Caught during [%s]\r\n", __func__);
                return false;
            }
            return true;
        }


        bool CFrontPanel::remoteLedOn()
        {
            try
            {
                auto it = std::find(m_lights.begin(), m_lights.end(), "Remote");

                if (m_lights.end() != it)
                {
                    device::FrontPanelIndicator::getInstance("Remote").setState(true);
                }
#ifndef USE_LEDMGR
                else
                {
                    int powerBrightness = device::FrontPanelIndicator::getInstance("Power").getBrightness();
                    if (powerBrightness > 0)
                    {
                        int adjustedBrightness = powerBrightness + 30;
                        if (adjustedBrightness > 100)
                            device::FrontPanelIndicator::getInstance("Power").setBrightness(100, false);
                        else
                            device::FrontPanelIndicator::getInstance("Power").setBrightness(adjustedBrightness, false);
                    }
                }
#endif
            }
            catch (...)
            {
                LOGERR("FrontPanel Exception Caught in remoteLedOn");
                return false;
            }
            return true;
        }


        bool CFrontPanel::remoteLedOff()
        {
            try
            {
                auto it = std::find(m_lights.begin(), m_lights.end(), "Remote");

                if (m_lights.end() != it)
                {
                    device::FrontPanelIndicator::getInstance("Remote").setState(false);
                }
#ifndef USE_LEDMGR
                else
                {
                    device::FrontPanelIndicator::getInstance("Power").setBrightness((device::FrontPanelIndicator::getInstance("Power").getBrightness()), false);
                }
#endif
            }
            catch (...)
            {
                LOGERR("FrontPanel Exception Caught in remoteLedOff");
                return false;
            }
            return true;
        }



        bool CFrontPanel::powerOffAllLed()
        {
            powerOffLed(FRONT_PANEL_INDICATOR_ALL);
            return true;
        }

        bool CFrontPanel::powerOnAllLed()
        {
            powerOnLed(FRONT_PANEL_INDICATOR_ALL);
            return true;
        }

        bool CFrontPanel::setLED(const JsonObject& parameters)
        {
            stopBlinkTimer();
            bool success = false;
            string ledIndicator = svc2iarm(parameters["ledIndicator"].String());
            int brightness = -1;

            if (parameters.HasLabel("brightness"))
                //brightness = properties["brightness"].Number();
                getNumberParameter("brightness", brightness);

            unsigned int color = 0;
            if (parameters.HasLabel("color")) //color mode 2
            {
                string colorString = parameters["color"].String();
                try
                {
                    device::FrontPanelIndicator::getInstance(ledIndicator.c_str()).setColor(device::FrontPanelIndicator::Color::getInstance(colorString.c_str()), false);
                    success = true;
                }
                catch (...)
                {
                    success = false;
                }
            }
            else if (parameters.HasLabel("red")) //color mode 1
            {
                unsigned int red,green,blue;

                getNumberParameter("red", red);
                getNumberParameter("green", green);
                getNumberParameter("blue", blue);

                color = (red << 16) | (green << 8) | blue;
                try
                {
                    device::FrontPanelIndicator::getInstance(ledIndicator.c_str()).setColor(color);
                    success = true;
                }
                catch (...)
                {
                    success = false;
                }
            }

            LOGWARN("setLed ledIndicator: %s brightness: %d", parameters["ledIndicator"].String().c_str(), brightness);
            try
            {
                if (brightness == -1)
                    brightness = device::FrontPanelIndicator::getInstance(ledIndicator.c_str()).getBrightness();

                device::FrontPanelIndicator::getInstance(ledIndicator.c_str()).setBrightness(brightness, false);
                success = true;
            }
            catch (...)
            {
                success = false;
            }
            return success;
        }

        void CFrontPanel::setBlink(const JsonObject& blinkInfo)
        {
            stopBlinkTimer();
            m_blinkList.clear();
            string ledIndicator = svc2iarm(blinkInfo["ledIndicator"].String());
            int iterations;
            getNumberParameterObject(blinkInfo, "iterations", iterations);
            JsonArray patternList = blinkInfo["pattern"].Array();
            for (int i = 0; i < patternList.Length(); i++)
            {
                JsonObject frontPanelBlinkHash = patternList[i].Object();
                FrontPanelBlinkInfo frontPanelBlinkInfo;
                frontPanelBlinkInfo.ledIndicator = ledIndicator;
                int brightness = -1;
                if (frontPanelBlinkHash.HasLabel("brightness"))
                    getNumberParameterObject(frontPanelBlinkHash, "brightness", brightness);

                int duration;
                getNumberParameterObject(frontPanelBlinkHash, "duration", duration);
                LOGWARN("setBlink ledIndicator: %s iterations: %d brightness: %d duration: %d", ledIndicator.c_str(), iterations, brightness, duration);
                frontPanelBlinkInfo.brightness = brightness;
                frontPanelBlinkInfo.durationInMs = duration;
                frontPanelBlinkInfo.colorValue = 0;
                if (frontPanelBlinkHash.HasLabel("color")) //color mode 2
                {
                    string color = frontPanelBlinkHash["color"].String();
                    frontPanelBlinkInfo.colorName = color;
                    frontPanelBlinkInfo.colorMode = 2;
                }
                else if (frontPanelBlinkHash.HasLabel("red")) //color mode 1
                {
                    unsigned int red,green,blue;

                    getNumberParameterObject(frontPanelBlinkHash, "red", red);
                    getNumberParameterObject(frontPanelBlinkHash, "green", green);
                    getNumberParameterObject(frontPanelBlinkHash, "blue", blue);

                    frontPanelBlinkInfo.colorValue = (red << 16) | (green << 8) | blue;
                    frontPanelBlinkInfo.colorMode = 1;
                }
                else
                {
                    frontPanelBlinkInfo.colorMode = 0;
                }
                m_blinkList.push_back(frontPanelBlinkInfo);
            }
            startBlinkTimer(iterations);
        }

        JsonObject CFrontPanel::getPreferences()
        {
            return m_preferencesHash;
        }

        void CFrontPanel::setPreferences(const JsonObject& preferences)
        {
            m_preferencesHash = preferences;

            Core::File file;
            file = FP_SETTINGS_FILE_JSON;

            file.Open(false);
            if (!file.IsOpen())
                file.Create();

            m_preferencesHash.IElement::ToFile(file);

            file.Close();
            Utils::syncPersistFile (FP_SETTINGS_FILE_JSON);
        }

        void CFrontPanel::loadPreferences()
        {
            m_preferencesHash.Clear();

            Core::File file;
            file = FP_SETTINGS_FILE_JSON;

            file.Open();
            m_preferencesHash.IElement::FromFile(file);

            file.Close();
        }

        void CFrontPanel::startBlinkTimer(int numberOfBlinkRepeats)
        {
            LOGWARN("startBlinkTimer numberOfBlinkRepeats: %d m_blinkList.length : %d", numberOfBlinkRepeats, m_blinkList.size());
            stopBlinkTimer();
            m_numberOfBlinks = 0;
            m_isBlinking = true;
            m_maxNumberOfBlinkRepeats = numberOfBlinkRepeats;
            m_currentBlinkListIndex = 0;
            if (m_blinkList.size() > 0)
            {
                FrontPanelBlinkInfo blinkInfo = m_blinkList.at(0);
                setBlinkLed(blinkInfo);
                if (m_isBlinking)
                    blinkTimer.Schedule(Core::Time::Now().Add(blinkInfo.durationInMs), m_blinkTimer);
            }
        }

        void CFrontPanel::stopBlinkTimer()
        {
            m_isBlinking = false;
            blinkTimer.Revoke(m_blinkTimer);
        }

        void CFrontPanel::setBlinkLed(FrontPanelBlinkInfo blinkInfo)
        {
            std::string ledIndicator = blinkInfo.ledIndicator;
            int brightness = blinkInfo.brightness;
            try
            {
                if (blinkInfo.colorMode == 1)
                {
                    device::FrontPanelIndicator::getInstance(ledIndicator.c_str()).setColor(blinkInfo.colorValue, false);
                }
                else if (blinkInfo.colorMode == 2)
                {
                    device::FrontPanelIndicator::getInstance(ledIndicator.c_str()).setColor(device::FrontPanelIndicator::Color::getInstance(blinkInfo.colorName.c_str()), false);
                }

            }
            catch (...)
            {}
            try
            {
                if (brightness == -1)
                    brightness = device::FrontPanelIndicator::getInstance(ledIndicator.c_str()).getBrightness();

                device::FrontPanelIndicator::getInstance(ledIndicator.c_str()).setBrightness(brightness, false);
            }
            catch (...)
            {
                LOGWARN("Exception caught in setBlinkLed for setBrightness ");
            }
        }

        void CFrontPanel::onBlinkTimer()
        {
            m_currentBlinkListIndex++;
            bool blinkAgain = true;
            if ((size_t)m_currentBlinkListIndex >= m_blinkList.size())
            {
                blinkAgain = false;
                m_currentBlinkListIndex = 0;
                m_numberOfBlinks++;
                if (m_maxNumberOfBlinkRepeats < 0 || m_numberOfBlinks <= m_maxNumberOfBlinkRepeats)
                {
                    blinkAgain = true;
                }
            }
            if (blinkAgain)
            {
                FrontPanelBlinkInfo blinkInfo = m_blinkList.at(m_currentBlinkListIndex);
                setBlinkLed(blinkInfo);
                if (m_isBlinking)
                    blinkTimer.Schedule(Core::Time::Now().Add(blinkInfo.durationInMs), m_blinkTimer);
            }

            //if not blink again then the led color should stay on the LAST element in the array as stated in the spec
        }

        void CFrontPanel::set24HourClock(bool is24Hour)
        {
            try
            {
                int newFormat = is24Hour ? device::FrontPanelTextDisplay::kModeClock24Hr : device::FrontPanelTextDisplay::kModeClock12Hr;
                device::FrontPanelTextDisplay &textDisplay = device::FrontPanelConfig::getInstance().getTextDisplay("Text");
                int currentFormat = textDisplay.getCurrentTimeFormat();
                LOGINFO("set24HourClock - Before setting %d - Time zone read from DS is %d", newFormat, currentFormat);
                textDisplay.setTimeFormat(newFormat);
                currentFormat = textDisplay.getCurrentTimeFormat();
                LOGINFO("set24HourClock - After setting %d - Time zone read from DS is %d", newFormat, currentFormat);
            }
            catch (...)
            {
                LOGERR("Exception Caught during set24HourClock");
            }
        }

        bool CFrontPanel::is24HourClock()
        {
            bool is24Hour = false;
            try
            {
                device::FrontPanelTextDisplay &textDisplay = device::FrontPanelConfig::getInstance().getTextDisplay("Text");
                int currentFormat = textDisplay.getCurrentTimeFormat();
                LOGINFO("is24HourClock - Time zone read from DS is %d", currentFormat);
                is24Hour = currentFormat == device::FrontPanelTextDisplay::kModeClock24Hr;
            }
            catch (...)
            {
                LOGERR("Exception Caught during is24HourClock");
            }
            return is24Hour;
        }

        void FrontPanelHelper::setRemoteLedState(bool state)
        {
            LOGINFO("setRemoteLedState state: %d", state);
#ifdef USE_RFBYPASS_LED_KEYPRESS
            if (state)
                CFrontPanel::instance()->powerOnLed(FRONT_PANEL_INDICATOR_RFBYPASS);
            else
                CFrontPanel::instance()->powerOffLed(FRONT_PANEL_INDICATOR_RFBYPASS);
#else
            if (state)
                CFrontPanel::instance()->remoteLedOn();
            else
                CFrontPanel::instance()->remoteLedOff();
#endif
        }

        uint64_t BlinkInfo::Timed(const uint64_t scheduledTime)
        {

            uint64_t result = 0;
            m_frontPanel->onBlinkTimer();
            return(result);
        }

    }
}

/** @} */
/** @} */
