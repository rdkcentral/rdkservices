/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2020 RDK Management
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

namespace WPEFramework {

    namespace Plugin {
        class NetworkManagerTimer {
            private:
                class NetworkManagerTimerJob {
                    private:
                        NetworkManagerTimerJob() = delete;
                        NetworkManagerTimerJob& operator=(const NetworkManagerTimerJob& RHS) = delete;

                    public:
                        NetworkManagerTimerJob(NetworkManagerTimer* tpt)
                            : m_tptimer(tpt)
                        {
                        }
                        NetworkManagerTimerJob(const NetworkManagerTimerJob& copy)
                            : m_tptimer(copy.m_tptimer)
                        {
                        }
                        ~NetworkManagerTimerJob() {}

                        inline bool operator==(const NetworkManagerTimerJob& RHS) const
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
                        NetworkManagerTimer* m_tptimer;
                };

            public:
                NetworkManagerTimer()
                    : baseTimer(64 * 1024, "TimerUtility")
                      , m_timerJob(this)
                      , m_isActive(false)
                      , m_isSingleShot(false)
                      , m_intervalInMs(-1)
            {
            }
                ~NetworkManagerTimer()
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

                WPEFramework::Core::TimerType<NetworkManagerTimerJob> baseTimer;
                NetworkManagerTimerJob m_timerJob;
                bool m_isActive;
                bool m_isSingleShot;
                int m_intervalInMs;

                std::function<void()> onTimeoutCallback;
        };
    }
}
