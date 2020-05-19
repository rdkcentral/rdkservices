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

#ifndef TTIMER_H
#define TTIMER_H

//#include <core/Timer.h>
#include <plugins/plugins.h>

namespace WPEFramework
{

    namespace Plugin
    {
        class TpTimer;
        
        class TpTimerJob
        {
        private:
            TpTimerJob() = delete;
            TpTimerJob& operator=(const TpTimerJob& RHS) = delete;

        public:
            TpTimerJob(WPEFramework::Plugin::TpTimer* tpt) : m_tptimer(tpt) { }
            TpTimerJob(const TpTimerJob& copy) : m_tptimer(copy.m_tptimer) { }
            ~TpTimerJob() {}

            inline bool operator==(const TpTimerJob& RHS) const
            {
                return(m_tptimer == RHS.m_tptimer);
            }

        public:
            uint64_t Timed(const uint64_t scheduledTime);

        private:
            WPEFramework::Plugin::TpTimer* m_tptimer;
        };

        class TpTimer
        {
        public:
            TpTimer();
            ~TpTimer();
            
            bool isActive();
            void stop();
            void start();
            void start(int msec);
            void setSingleShot(bool val);
            void setInterval(int msec);
            
            void connect(std::function< void() > callback);
            
        private:
            
            void Timed();
            
            WPEFramework::Core::TimerType<TpTimerJob> baseTimer;
            TpTimerJob m_timerJob;
            bool m_isActive;
            bool m_isSingleShot;
            int m_intervalInMs;
            
            std::function< void() > onTimeoutCallback;
            
            friend class TpTimerJob;
        };
    }
    
}

#endif

