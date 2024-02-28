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
 
#include "LocationSync.h"

#ifdef USE_THUNDER_R4
#include <interfaces/json/JTimeZone.h>
#endif

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework {
namespace Plugin {

    namespace {

        static Metadata<LocationSync> metadata(
            // Version
            1, 0, 0,
            // Preconditions
            {  subsystem::NETWORK },
            // Terminations
            {},
            // Controls
            { subsystem::INTERNET, subsystem::LOCATION }
        );
    }

    SERVICE_REGISTRATION(LocationSync, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    static Core::ProxyPoolType<Web::Response> responseFactory(4);
    static Core::ProxyPoolType<Web::JSONBodyType<LocationSync::Data>> jsonResponseFactory(4);

#ifdef USE_THUNDER_R4
    namespace {
        constexpr TCHAR FactorySetTimeZone[] = _T("Factory");
    }
#endif
#ifdef __WINDOWS__
#pragma warning(disable : 4355)
#endif
    LocationSync::LocationSync()
        : _skipURL(0)
        , _source()
        , _sink(this)
        , _service(nullptr)
        , _timezoneoverriden(false)
        , _locationinfo()
        , _adminLock()
        , _timezoneoberservers()
        , _activateOnFailure(true)
    {
    }
#ifdef __WINDOWS__
#pragma warning(default : 4355)
#endif

    const string LocationSync::Initialize(PluginHost::IShell* service) /* override */
    {
        string result;
        Config config;
        ASSERT(service != nullptr);
        config.FromString(service->ConfigLine());

        if (LocationService::IsSupported(config.Source.Value()) == Core::ERROR_NONE) {
            if ((config.TimeZone.IsSet() == true) && (config.TimeZone.Value().empty() == false)) {
                _locationinfo.TimeZone(config.TimeZone.Value());
                _timezoneoverriden = true;
                UpdateSystemTimeZone(config.TimeZone.Value());
            }

#ifdef USE_THUNDER_R4
            if ((config.Latitude.IsSet() == true) && (config.Longitude.IsSet() == true)) {
                _locationinfo.Latitude(config.Latitude.Value());
                _locationinfo.Longitude(config.Longitude.Value());
            }
#endif

            _skipURL = static_cast<uint16_t>(service->WebPrefix().length());
            _source = config.Source.Value();
            _activateOnFailure = config.ActivateOnFailure.Value();
            _service = service;
            _service->AddRef();

            _sink.Initialize(config.Source.Value(), config.Interval.Value(), config.Retries.Value());

            RegisterAll();
#ifdef USE_THUNDER_R4
            Exchange::JTimeZone::Register(*this, this);
#endif

        } else {
            result = _T("URL for retrieving location is incorrect !!!");
        }

#ifndef USE_THUNDER_R4
        if (result.length() != 0) {
            Deinitialize(service);
        }
#endif

        // On success return empty, to indicate there is no error text.
        return (result);
    }

    void LocationSync::Deinitialize(PluginHost::IShell* service VARIABLE_IS_NOT_USED) /* override */
    {
        if (_service != nullptr) {
            ASSERT(_service == service);

            UnregisterAll();
#ifdef USE_THUNDER_R4
            Exchange::JTimeZone::Unregister(*this);
#endif

            _sink.Deinitialize();

            Config config;
            config.FromString(_service->ConfigLine());

#ifdef USE_THUNDER_R4
            Exchange::Controller::IConfiguration* controller = nullptr;
            if ( (_timezoneoverriden == true) &&
               ( _locationinfo.TimeZone() != config.TimeZone.Value()) &&
               ( ( controller = _service->QueryInterfaceByCallsign<Exchange::Controller::IConfiguration>(_T(""))) != nullptr)
            ) {
                config.TimeZone = _locationinfo.TimeZone();
                string newconfig;
                config.ToString(newconfig);
                _service->ConfigLine(newconfig);

                ASSERT(controller != nullptr);
                controller->Persist();
                controller->Release();
            }
#endif
            _service->Release();
            _service = nullptr;
        }
    }

    string LocationSync::Information() const /* override */
    {
        // No additional info to report.
        return (string());
    }

    void LocationSync::Inbound(Web::Request& /* request */) /* override */
    {
    }

    Core::ProxyType<Web::Response>
    LocationSync::Process(const Web::Request& request) /* override */
    {
        Core::ProxyType<Web::Response> result(PluginHost::IFactories::Instance().Response());
        Core::TextSegmentIterator index(
            Core::TextFragment(request.Path, _skipURL, static_cast<uint16_t>(request.Path.length()) - _skipURL),
            false,
            '/');

        // By default, we assume everything works..
        result->ErrorCode = Web::STATUS_OK;
        result->Message = "OK";

        if (request.Verb == Web::Request::HTTP_GET) {
            Core::ProxyType<Web::JSONBodyType<Data>> response(jsonResponseFactory.Element());

            PluginHost::ISubSystem* subSystem = _service->SubSystems();

            ASSERT(subSystem != nullptr);

            const PluginHost::ISubSystem::IInternet* internet(subSystem->Get<PluginHost::ISubSystem::IInternet>());
            const PluginHost::ISubSystem::ILocation* location(subSystem->Get<PluginHost::ISubSystem::ILocation>());

            if ((internet != nullptr) && (location != nullptr)) {
                response->PublicIp = internet->PublicIPAddress();
                response->TimeZone = location->TimeZone();
                response->Region = location->Region();
                response->Country = location->Country();
                response->City = location->City();

                result->ContentType = Web::MIMETypes::MIME_JSON;
#ifndef USE_THUNDER_R4
                result->Body(Core::proxy_cast<Web::IBody>(response));
#else
                result->Body(Core::ProxyType<Web::IBody>(response));
#endif /* USE_THUNDER_R4 */
            } else {
                result->ErrorCode = Web::STATUS_SERVICE_UNAVAILABLE;
                result->Message = _T("Internet and Location Service not yet available");
            }
        } else if (request.Verb == Web::Request::HTTP_POST) {
            index.Next();
            if (index.Next()) {
                if ((index.Current() == "Sync") && (_source.empty() == false)) {
                    uint32_t error = _sink.Probe(_source, 1, 1);

                    if (error != Core::ERROR_NONE) {
                        result->ErrorCode = Web::STATUS_INTERNAL_SERVER_ERROR;
                        result->Message = _T("Probe failed with error code: ") + Core::NumberType<uint32_t>(error).Text();
                    }
                }
            }
        } else {
            result->ErrorCode = Web::STATUS_BAD_REQUEST;
            result->Message = _T("Unsupported request for the [LocationSync] service.");
        }

        return result;
    }

    uint32_t LocationSync::Register(Exchange::ITimeZone::INotification* sink) {
        _adminLock.Lock();
        TimeZoneObservers::iterator index = std::find(_timezoneoberservers.begin(), _timezoneoberservers.end(), sink);
        ASSERT (index == _timezoneoberservers.end());
        if (index == _timezoneoberservers.end()) {
            sink->AddRef();
            _timezoneoberservers.emplace_back(sink);
        }
        _adminLock.Unlock();
        return Core::ERROR_NONE;
    }

    uint32_t LocationSync::Unregister(Exchange::ITimeZone::INotification* sink) {
        _adminLock.Lock();
        TimeZoneObservers::iterator index = std::find(_timezoneoberservers.begin(), _timezoneoberservers.end(), sink);
        ASSERT (index != _timezoneoberservers.end());
        if (index != _timezoneoberservers.end()) {
            sink->Release();
            _timezoneoberservers.erase(index);
        }
        _adminLock.Unlock();
        return Core::ERROR_NONE;
    }

#ifdef USE_THUNDER_R4
    uint32_t LocationSync::TimeZone(string& timeZone) const
#else
    uint32_t LocationSync::GetTimeZone(string& timeZone) const
#endif
    {
        timeZone = CurrentTimeZone();
        return Core::ERROR_NONE;
    }

#ifdef USE_THUNDER_R4
    uint32_t LocationSync::TimeZone(const string& timeZone)
#else
    uint32_t LocationSync::SetTimeZone(const string& timeZone)
#endif
    {
        _adminLock.Lock();

        _timezoneoverriden = true;

        if (_locationinfo.TimeZone() != timeZone) {

            _locationinfo.TimeZone(timeZone);

            _adminLock.Unlock();

            UpdateSystemTimeZone(timeZone);

            // let's check if we need to update the subsystem. As there is no support in this plugin for unsetting the Location subsystem that is not taken into account
            PluginHost::ISubSystem* subSystem = _service->SubSystems();
            ASSERT(subSystem != nullptr);
            if (subSystem != nullptr) {
                SetLocationSubsystem(*subSystem, true);
                subSystem->Release();
            }

            NotifyTimeZoneChanged(timeZone);

        } else {
            _adminLock.Unlock();
        }

        return Core::ERROR_NONE;
    }

    string LocationSync::CurrentTimeZone() const {
        Core::SafeSyncType<Core::CriticalSection> guard(_adminLock);
        return _locationinfo.TimeZone();
    }

    void LocationSync::NotifyTimeZoneChanged(const string& timezone) const {
        _adminLock.Lock();
        for (auto observer : _timezoneoberservers) {
            observer->TimeZoneChanged(timezone);
        }
        _adminLock.Unlock();

#ifdef USE_THUNDER_R4
        Exchange::JTimeZone::Event::TimeZoneChanged(const_cast<PluginHost::JSONRPC&>(static_cast<const PluginHost::JSONRPC&>(*this)), timezone);
#endif
        SYSLOG(Logging::Startup, (_T("TimeZone change to \"%s\", local date time is now %s."), timezone.c_str(), Core::Time::Now().ToRFC1123(true).c_str()));
    }

    void LocationSync::SetLocationSubsystem(PluginHost::ISubSystem& subsystem, bool update) /* cannot be const due to subsystem Set*/ {
        if (update == false) {
            _adminLock.Lock();
            subsystem.Set(PluginHost::ISubSystem::LOCATION, &_locationinfo);
            _adminLock.Unlock();
        } else {
            _adminLock.Lock();
            if (subsystem.IsActive(PluginHost::ISubSystem::LOCATION) == true) {
                subsystem.Set(PluginHost::ISubSystem::LOCATION, &_locationinfo);
            }
            _adminLock.Unlock();
        }
    }

    void LocationSync::SyncedLocation()
    {
        if ((_sink.Location() != nullptr) && (_sink.Valid() == true)) {  // _sink.Location() != nullptr basically is always true
            string newtimezone;
            _adminLock.Lock();

#ifdef USE_THUNDER_R4
            if ((_locationinfo.Latitude() == std::numeric_limits<int32_t>::min()) || (_locationinfo.Longitude() == std::numeric_limits<int32_t>::min())) {
                _locationinfo.Latitude(_sink.Location()->Latitude());
                _locationinfo.Longitude(_sink.Location()->Longitude());
            }
#endif
            _locationinfo.Country(_sink.Location()->Country());
            _locationinfo.Region(_sink.Location()->Region());
            _locationinfo.City(_sink.Location()->City());
            if ((_sink.Location()->TimeZone().empty() == false) && (_timezoneoverriden == false)) {
                newtimezone = _sink.Location()->TimeZone();
                _locationinfo.TimeZone(newtimezone);
            }
            _adminLock.Unlock();

            if (newtimezone.empty() == false) {
                UpdateSystemTimeZone(newtimezone);
                NotifyTimeZoneChanged(newtimezone);
            }
#ifdef USE_THUNDER_R4
        } else {
            _adminLock.Lock();
            // if they are not overriden in the config and we cannot get them from the lookup, set them to default
            if ((_locationinfo.Latitude() == std::numeric_limits<int32_t>::min()) || (_locationinfo.Longitude() == std::numeric_limits<int32_t>::min())) {
                _locationinfo.Latitude(51977956);
                _locationinfo.Longitude(5726384);
            }
            _adminLock.Unlock();
#endif
        }

        PluginHost::ISubSystem* subSystem = _service->SubSystems();
        ASSERT(subSystem != nullptr);

        if (subSystem != nullptr) {
            if ((_activateOnFailure == true) || (_sink.Location() == nullptr) || (_sink.Valid() == true)) { // again _sink.Location() == nullptr should not happen but added to make it backards compatibe
                subSystem->Set(PluginHost::ISubSystem::INTERNET, _sink.Network());
                SetLocationSubsystem(*subSystem, false);
                event_locationchange();
            } else if (_timezoneoverriden == true) { // if the probing failed but the timezone was explicitely set we only set the location subsystem to pass on the timezone info
                SetLocationSubsystem(*subSystem, false);
                event_locationchange();
            }
            subSystem->Release();
        }
    }

    void LocationSync::UpdateSystemTimeZone(const string& newtimezone)
    {
#ifdef USE_THUNDER_R4
        if (newtimezone != FactorySetTimeZone) {
            Core::SystemInfo::Instance().SetTimeZone(newtimezone, false);
        }
#endif
    }

} // namespace Plugin
} // namespace WPEFramework
