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
* @defgroup include
* @{
**/


#ifndef FRONTPANEL_H
#define FRONTPANEL_H

#include <string>
#include <list>
#include <vector>

#include <plugins/plugins.h>

namespace WPEFramework
{

    namespace Plugin
    {

        class FrontPanel;
        class FrontPanelHelper;
        class CFrontPanel;

        class BlinkInfo
        {
        private:
            BlinkInfo() = delete;
            BlinkInfo& operator=(const BlinkInfo& RHS) = delete;

        public:
            BlinkInfo(CFrontPanel* fp)
            : m_frontPanel(fp)
            {
            }
            BlinkInfo(const BlinkInfo& copy)
            : m_frontPanel(copy.m_frontPanel)
            {
            }
            ~BlinkInfo() {}

            inline bool operator==(const BlinkInfo& RHS) const
            {
                return(m_frontPanel == RHS.m_frontPanel);
            }

        public:
            uint64_t Timed(const uint64_t scheduledTime);

        private:
            CFrontPanel* m_frontPanel;
        };


        typedef struct _FrontPanelBlinkInfo
        {
            std::string ledIndicator;
            std::string colorName;
            unsigned int colorValue;
            int brightness;
            int durationInMs;
            int colorMode;
        } FrontPanelBlinkInfo;

        typedef enum _frontPanelIndicator
        {
            FRONT_PANEL_INDICATOR_CLOCK,
            FRONT_PANEL_INDICATOR_MESSAGE,
            FRONT_PANEL_INDICATOR_POWER,
            FRONT_PANEL_INDICATOR_RECORD,
            FRONT_PANEL_INDICATOR_REMOTE,
            FRONT_PANEL_INDICATOR_RFBYPASS,
            FRONT_PANEL_INDICATOR_ALL
        } frontPanelIndicator;

        class CFrontPanel
        {
        public:
            static CFrontPanel* instance();
            bool start();
            bool stop();
            std::string getLastError();
            void addEventObserver(FrontPanel* o);
            void removeEventObserver(FrontPanel* o);
            bool setBrightness(int fp_brighness);
            int getBrightness();
#ifdef CLOCK_BRIGHTNESS_ENABLED
            bool setClockBrightness(int brightness);
            int getClockBrightness();
#endif
            bool powerOffLed(frontPanelIndicator fp_indicator);
            bool powerOnLed(frontPanelIndicator fp_indicator);
            bool powerOffAllLed();
            bool powerOnAllLed();
            void setPowerStatus(bool powerStatus);
            JsonObject getPreferences();
            void setPreferences(const JsonObject& preferences);
            bool setLED(const JsonObject& blinkInfo);
            void setBlink(const JsonObject& blinkInfo);
            void loadPreferences();
            void stopBlinkTimer();
            bool remoteLedOn();
            bool remoteLedOff();
            void set24HourClock(bool is24Hour);
            bool is24HourClock();

            void onBlinkTimer();
            static int initDone;

        private:
            CFrontPanel();
            static CFrontPanel* s_instance;
            void startBlinkTimer(int numberOfBlinkRepeats);
            void setBlinkLed(FrontPanelBlinkInfo blinkInfo);
            JsonObject m_preferencesHash;  // is this needed

            BlinkInfo m_blinkTimer;
            bool m_isBlinking;
            std::vector<FrontPanelBlinkInfo> m_blinkList;
            std::list<FrontPanel*> observers_;

            std::string lastError_;
            FrontPanelHelper* mFrontPanelHelper;
        };

        class FrontPanelHelper
        {
            void setRemoteLedState(bool state);

        public:
            ~FrontPanelHelper();

        };

    } // namespace Plugin
} // namespace WPEFramework


#endif


/** @} */
/** @} */
