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

#include "Module.h"
#include "LocationService.h"
#include <interfaces/json/JsonData_LocationSync.h>
#include <interfaces/ITimeZone.h>

#include <limits>

namespace WPEFramework {
namespace Plugin {

    class LocationSync : public PluginHost::IPlugin, public Exchange::ITimeZone, public PluginHost::IWeb, public PluginHost::JSONRPC {
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
                ASSERT(_locator != nullptr);
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
            bool Valid() const {
                ASSERT(_locator != nullptr);
                return _locator->Valid();
            }


        private:
            inline uint32_t Probe()
            {

                ASSERT(_locator != nullptr);

                return (_locator != nullptr ? _locator->Probe(_source, (_retries == 0 ? UINT32_MAX : _retries), _interval) : static_cast<uint32_t>(Core::ERROR_UNAVAILABLE));
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
                , ActivateOnFailure(true) // as in some cases startup of the system depends on the Internet and Locatioin subsystems to be flagged this enables the activation of these subsystems even though probing was unsuccesfull  (and for backward compatibility it is even the default)
                , Source()
                , TimeZone()
#ifdef USE_THUNDER_R4
                , Latitude(51977956) // Divider 1.000.000
                , Longitude(5726384) // Divider 1.000.000
#endif
            {
                Add(_T("interval"), &Interval);
                Add(_T("retries"), &Retries);
                Add(_T("activateonfailure"), &ActivateOnFailure);
                Add(_T("source"), &Source);
                Add(_T("timezone"), &TimeZone);
#ifdef USE_THUNDER_R4
                Add(_T("latitude"), &Latitude);
                Add(_T("longitude"), &Longitude);
#endif
            }
            ~Config()
            {
            }

        public:
            Core::JSON::DecUInt16 Interval;
            Core::JSON::DecUInt8 Retries;
            Core::JSON::Boolean ActivateOnFailure; // as in some cases startup of the system depends on the Internet and Locatioin subsystems to be flagged this enables the activation of these subsystems even though probing was unsuccesfull  (and for backward compatibility it is even the default)
            Core::JSON::String Source;
            Core::JSON::String TimeZone;
#ifdef USE_THUNDER_R4
            Core::JSON::DecSInt32 Latitude;
            Core::JSON::DecSInt32 Longitude;
#endif
        };

        class LocationInfo :  public PluginHost::ISubSystem::ILocation {
        public:
            LocationInfo(const LocationInfo&) = default;
            LocationInfo(LocationInfo&&) = default;
            LocationInfo& operator=(const LocationInfo&) = default;
            LocationInfo& operator=(LocationInfo&&) = default;

            LocationInfo()
                : _timeZone()
                , _country()
                , _region()
                , _city()
#ifdef USE_THUNDER_R4
                , _latitude(std::numeric_limits<int32_t>::min())
                , _longitude(std::numeric_limits<int32_t>::min())
#endif
            {
            }
            LocationInfo(int32_t latitude, int32_t longitude)
                : _timeZone()
                , _country()
                , _region()
                , _city()
#ifdef USE_THUNDER_R4
                , _latitude(latitude)
                , _longitude(longitude)
#endif
            {
            }
            ~LocationInfo() override = default;

        public:
            BEGIN_INTERFACE_MAP(Location)
            INTERFACE_ENTRY(PluginHost::ISubSystem::ILocation)
            END_INTERFACE_MAP

        public:
            string TimeZone() const override { return _timeZone; }
            void TimeZone(const string& timezone) { _timeZone = timezone; }
            string Country() const override { return _country; }
            void Country(const string& country) { _country = country; }
            string Region() const override { return _region; }
            void Region(const string& region) { _region = region; }
            string City() const override { return _city; }
            void City(const string& city) { _city = city; }
#ifdef USE_THUNDER_R4
            void Latitude(const int32_t latitude) { _latitude = latitude; }
            int32_t Latitude() const override { return _latitude; }
            int32_t Longitude() const override { return _longitude; }
            void Longitude(const int32_t longitude) { _longitude = longitude; }
#endif

        private:
            string _timeZone;
            string _country;
            string _region;
            string _city;
#ifdef USE_THUNDER_R4
            int32_t _latitude;
            int32_t _longitude;
#endif
        };

    public:
        LocationSync(const LocationSync&) = delete;
        LocationSync& operator=(const LocationSync&) = delete;

        LocationSync();
        ~LocationSync() override = default;

        // Build QueryInterface implementation, specifying all possible interfaces to be returned.
        BEGIN_INTERFACE_MAP(LocationSync)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IWeb)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            INTERFACE_ENTRY(Exchange::ITimeZone)
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

        //   ITimeZone methods
        // -------------------------------------------------------------------------------------------------------
        uint32_t Register(ITimeZone::INotification* sink) override ;
        uint32_t Unregister(ITimeZone::INotification* sink) override;
#ifdef USE_THUNDER_R4
        uint32_t TimeZone(string& timeZone ) const override;
        uint32_t TimeZone(const string& timeZone) override;
#else
        uint32_t GetTimeZone(string& timeZone) const override;
        uint32_t SetTimeZone(const string& timeZone) override;
#endif

    private:
        string CurrentTimeZone() const;
        void NotifyTimeZoneChanged(const string& timezone) const;
        void SetLocationSubsystem(PluginHost::ISubSystem& subsystem, bool update);
        void RegisterAll();
        void UnregisterAll();
        uint32_t endpoint_sync();
        uint32_t get_location(JsonData::LocationSync::LocationData& response) const;
        void event_locationchange();

        void SyncedLocation();
        void UpdateSystemTimeZone(const string& timezone);

    private:
        using TimeZoneObservers = std::list<Exchange::ITimeZone::INotification*>;

        uint16_t _skipURL;
        string _source;
        Core::Sink<Notification> _sink;
        PluginHost::IShell* _service;
        bool _timezoneoverriden;
        Core::Sink<LocationInfo> _locationinfo;
        mutable Core::CriticalSection _adminLock;
        TimeZoneObservers _timezoneoberservers;
        bool _activateOnFailure;
    };

} // namespace Plugin
} // namespace WPEFramework

#endif // LOCATIONSYNC_LOCATIONSYNC_H
