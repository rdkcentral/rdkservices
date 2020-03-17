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

#include "tptimer.h"

namespace WPEFramework
{

    namespace Plugin
    {    
        WPEFramework::Core::TimerType<TpTimerJob>& TpTimer::baseTimer = WPEFramework::Core::SingletonType < WPEFramework::Core::TimerType<TpTimerJob> >::Instance(64 * 1024, "ThunderPluginBaseTimer");

        TpTimer::TpTimer() :
          m_timerJob(this)
        , m_isActive(false)
        , m_isSingleShot(false)
        , m_intervalInMs(-1)
        {
            
        }

        TpTimer::~TpTimer()
        {
            stop();
        }

        bool TpTimer::isActive()
        {
            return m_isActive;
        }

        void TpTimer::stop()
        {
            baseTimer.Revoke(m_timerJob);
            
            m_isActive = false;
        }
        
        void TpTimer::start()
        {
            baseTimer.Revoke(m_timerJob);
            baseTimer.Schedule(Core::Time::Now().Add(m_intervalInMs), m_timerJob);
            
            m_isActive = true;
        }

        void TpTimer::start(int msec)
        {
            setInterval(msec);
            
            start();
        }
        
        void TpTimer::setSingleShot(bool val)
        {
            m_isSingleShot = val;
        }
        
        void TpTimer::setInterval(int msec)
        {
            m_intervalInMs = msec;
            
            //needs to restart timer?
        }
        
        void TpTimer::connect(std::function< void() > callback)
        {
            onTimeoutCallback = callback;
        }

        void TpTimer::Timed()
        {
            if(onTimeoutCallback != nullptr)
                onTimeoutCallback();
            
            //restart timer?
            if(!m_isSingleShot)
                start();
            else
                m_isActive = false;
        }
        
        uint64_t TpTimerJob::Timed(const uint64_t scheduledTime)
        {
            if(m_tptimer)
                m_tptimer->Timed();
            
            return 0;
        }
    }
    
}
