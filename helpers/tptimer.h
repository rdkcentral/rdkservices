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

namespace WPEFramework {

namespace Plugin {
    class TpTimer {
    private:
        class TpTimerJob {
        private:
            TpTimerJob() = delete;
            TpTimerJob& operator=(const TpTimerJob& RHS) = delete;

        public:
            TpTimerJob(TpTimer* tpt)
                : m_tptimer(tpt)
            {
            }
            TpTimerJob(const TpTimerJob& copy)
                : m_tptimer(copy.m_tptimer)
            {
            }
            ~TpTimerJob() {}

            inline bool operator==(const TpTimerJob& RHS) const
            {
                return (m_tptimer == RHS.m_tptimer);
            }

        public:
            uint64_t Timed(const uint64_t scheduledTime)
            {
                if (m_tptimer) {
                    m_tptimer->Timed();
                }
                return 0;
            }

        private:
            TpTimer* m_tptimer;
        };

    public:
        TpTimer()
            : baseTimer(64 * 1024, "ThunderPluginBaseTimer")
            , m_timerJob(this)
            , m_isActive(false)
            , m_isSingleShot(false)
            , m_intervalInMs(-1)
        {
        }
        ~TpTimer()
        {
            stop();
        }

        bool isActive()
        {
            return m_isActive;
        }
        void stop()
        {
            baseTimer.Revoke(m_timerJob);
            m_isActive = false;
        }
        void start()
        {
            baseTimer.Revoke(m_timerJob);
            baseTimer.Schedule(Core::Time::Now().Add(m_intervalInMs), m_timerJob);
            m_isActive = true;
        }
        void start(int msec)
        {
            setInterval(msec);
            start();
        }
        void setSingleShot(bool val)
        {
            m_isSingleShot = val;
        }
        void setInterval(int msec)
        {
            m_intervalInMs = msec;
        }

        void connect(std::function<void()> callback)
        {
            onTimeoutCallback = callback;
        }

    private:
        void Timed()
        {
            if (onTimeoutCallback != nullptr) {
                onTimeoutCallback();
            }

            if (m_isActive) {
                if (m_isSingleShot) {
                    stop();
                } else {
                    start();
                }
            }
        }

        WPEFramework::Core::TimerType<TpTimerJob> baseTimer;
        TpTimerJob m_timerJob;
        bool m_isActive;
        bool m_isSingleShot;
        int m_intervalInMs;

        std::function<void()> onTimeoutCallback;
    };
}
}

#endif
