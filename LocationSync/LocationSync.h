/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
 */
 
#ifndef LOCATIONSYNC_LOCATIONSYNC_H
#define LOCATIONSYNC_LOCATIONSYNC_H

#include "LocationService.h"
#include <interfaces/json/JsonData_LocationSync.h>
#include "Module.h"


namespace WPEFramework {
namespace Plugin {
    class Timer;
    class TimerJob
    {
    private:
        TimerJob() = delete;
        TimerJob& operator=(const TimerJob& RHS) = delete;
    public:
        TimerJob(WPEFramework::Plugin::Timer* tpt) : m_timer(tpt) { }
        TimerJob(const TimerJob& copy) : m_timer(copy.m_timer) { }
        ~TimerJob() {}
        inline bool operator==(const TimerJob& RHS) const
        {
            return(m_timer == RHS.m_timer);
        }
    public:
        uint64_t Timed(const uint64_t scheduledTime);
    private:
        WPEFramework::Plugin::Timer* m_timer;
    };

    class Timer
    {
    public:
        Timer();
        ~Timer();
        bool isActive();
        void stop();
        void start();
        void start(int msec);
        void setSingleShot(bool val);
        void setInterval(int msec);
        void connect(std::function< void() > callback);
    private:
        void Timed();
        WPEFramework::Core::TimerType<TimerJob> baseTimer;
        TimerJob m_timerJob;
        bool m_isActive;
        bool m_isSingleShot;
        int m_intervalInMs;
        std::function< void() > onTimeoutCallback;
        friend class TimerJob;
    };

    class LocationSync : public PluginHost::IPlugin, public PluginHost::IWeb, public PluginHost::JSONRPC {
    public:
        class Data : public Core::JSON::Container {
        public:
            Data(Data const& other) = delete;
            Data& operator=(Data const& other) = delete;

            Data()
                : Core::JSON::Container()
                , PublicIp()
                , TimeZone()
                , Region()
                , Country()
                , City()
            {
                Add(_T("ip"), &PublicIp);
                Add(_T("timezone"), &TimeZone);
                Add(_T("region"), &Region);
                Add(_T("country"), &Region);
                Add(_T("city"), &Region);
            }

            ~Data() override
            {
            }

        public:
            Core::JSON::String PublicIp;
            Core::JSON::String TimeZone;
            Core::JSON::String Region;
            Core::JSON::String Country;
            Core::JSON::String City;
        };

    private:
        class Notification : public Core::IDispatch {
        private:
            Notification() = delete;
            Notification(const Notification&) = delete;
            Notification& operator=(const Notification&) = delete;

        public:
#ifdef __WINDOWS__
#pragma warning(disable : 4355)
#endif
            explicit Notification(LocationSync* parent)
                : _parent(*parent)
                , _source()
                , _interval()
                , _retries()
                , _locator(Core::Service<LocationService>::Create<LocationService>(this))
            {
                ASSERT(parent != nullptr);
            }
#ifdef __WINDOWS__
#pragma warning(default : 4355)
#endif
            ~Notification()
            {
                _locator->Release();
            }

        public:
            inline void Initialize(const string& source, const uint16_t interval, const uint8_t retries)
            {
                _source = source;
                _interval = interval;
                _retries = retries;

                Probe();
            }
            inline void Deinitialize()
            {
            }
            uint32_t Probe(const string& remoteNode, const uint32_t retries, const uint32_t retryTimeSpan)
            {
                _source = remoteNode;
                _interval = retryTimeSpan;
                _retries = retries;

                return (Probe());
            }

            inline PluginHost::ISubSystem::ILocation* Location()
            {
                return (_locator);
            }
            inline PluginHost::ISubSystem::IInternet* Network()
            {
                return (_locator);
            }

        private:
            inline uint32_t Probe()
            {

                ASSERT(_locator != nullptr);

                return (_locator != nullptr ? _locator->Probe(_source, _retries, _interval) : static_cast<uint32_t>(Core::ERROR_UNAVAILABLE));
            }

            void Dispatch() override
            {
                _parent.SyncedLocation();
            }

        private:
            LocationSync& _parent;
            string _source;
            uint16_t _interval;
            uint8_t _retries;
            LocationService* _locator;
        };

        class Config : public Core::JSON::Container {
        private:
            Config(const Config&) = delete;
            Config& operator=(const Config&) = delete;

        public:
            Config()
                : Interval(30)
                , Retries(8)
                , Source()
            {
                Add(_T("interval"), &Interval);
                Add(_T("retries"), &Retries);
                Add(_T("source"), &Source);
            }
            ~Config()
            {
            }

        public:
            Core::JSON::DecUInt16 Interval;
            Core::JSON::DecUInt8 Retries;
            Core::JSON::String Source;
        };

    private:
        LocationSync(const LocationSync&) = delete;
        LocationSync& operator=(const LocationSync&) = delete;

    public:
        LocationSync();
        ~LocationSync() override;

        // Build QueryInterface implementation, specifying all possible interfaces to be returned.
        BEGIN_INTERFACE_MAP(LocationSync)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IWeb)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        END_INTERFACE_MAP

    public:
        //   IPlugin methods
        // -------------------------------------------------------------------------------------------------------
        const string Initialize(PluginHost::IShell* service) override;
        void Deinitialize(PluginHost::IShell* service) override;
        string Information() const override;

        //   IWeb methods
        // -------------------------------------------------------------------------------------------------------
        void Inbound(Web::Request& request) override;
        Core::ProxyType<Web::Response> Process(const Web::Request& request) override;

    private:
        void RegisterAll();
        void UnregisterAll();
        uint32_t endpoint_sync();
        uint32_t get_location(JsonData::LocationSync::LocationData& response) const;
        void event_locationchange();
        void SyncedLocation();
        enum WPE_INET_Result : int {
            WPE_INET_UNDEFINED = -50
            , WPE_INET_UNRESOLVED = -3
            , WPE_INET_TIMEDOUT = -2
            , WPE_INET_UNKNOWN = -1
            , WPE_INET_DISCONNECTED = 0
            , WPE_INET_CONNECTED = 1
        };
        WPE_INET_Result getConnectivity(const std::string&);
        void onNetControlTimer();

    private:
        uint16_t _skipURL;
        string _source;
        uint16_t _interval;
        uint8_t _retries;
        Core::Sink<Notification> _sink;
        PluginHost::IShell* _service;
        bool _networkReady;
        Timer _netControlTimer;
    };

} // namespace Plugin
} // namespace WPEFramework

#endif // LOCATIONSYNC_LOCATIONSYNC_H
