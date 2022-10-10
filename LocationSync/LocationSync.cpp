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

#include <interfaces/json/JTimeZone.h>

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 1

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(LocationSync, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    static Core::ProxyPoolType<Web::Response> responseFactory(4);
    static Core::ProxyPoolType<Web::JSONBodyType<LocationSync::Data>> jsonResponseFactory(4);

PUSH_WARNING(DISABLE_WARNING_THIS_IN_MEMBER_INITIALIZER_LIST)
    LocationSync::LocationSync()
        : _skipURL(0)
        , _source()
        , _sink(this)
        , _service(nullptr)
        , _timezoneoverride()
        , _adminLock()
        , _timezoneoberservers()
    {
    }
POP_WARNING()

    const string LocationSync::Initialize(PluginHost::IShell* service) /* override */
    {
        string result;
        Config config;
        config.FromString(service->ConfigLine());

        if( ( config.TimeZone.IsSet() == true ) && ( config.TimeZone.Value().empty() == false ) ) {
            Core::SystemInfo::Instance().SetTimeZone(config.TimeZone.Value());
            _timezoneoverride = config.TimeZone.Value();
        }

        if (LocationService::IsSupported(config.Source.Value()) == Core::ERROR_NONE) {
            _skipURL = static_cast<uint16_t>(service->WebPrefix().length());
            _source = config.Source.Value();
            _service = service;
            _service->AddRef();
            
            _sink.Initialize(config.Source.Value(), config.Interval.Value(), config.Retries.Value());

            RegisterAll();
            Exchange::JTimeZone::Register(*this, this);

        } else {
            result = _T("URL for retrieving location is incorrect !!!");
        }

        // On success return empty, to indicate there is no error text.
        return (result);
    }

    void LocationSync::Deinitialize(PluginHost::IShell* service VARIABLE_IS_NOT_USED) /* override */
    {
        ASSERT(_service == service);

        UnregisterAll();
        Exchange::JTimeZone::Unregister(*this);

        _sink.Deinitialize();

        Config config;
        config.FromString(service->ConfigLine());

        PluginHost::IController* controller = nullptr;
        if( ( _timezoneoverride != config.TimeZone.Value() ) && 
            ( ( controller = service->QueryInterfaceByCallsign<PluginHost::IController>(_T("")) ) != nullptr ) 
          ) {
            config.TimeZone = _timezoneoverride;
            string newconfig;
            config.ToString(newconfig);
            service->ConfigLine(newconfig);
            controller->Persist();
            controller->Release();
        }

        _service->Release();
        _service = nullptr;
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
                response->TimeZone = CurrentTimeZone();
                response->Region = location->Region();
                response->Country = location->Country();
                response->City = location->City();

                result->ContentType = Web::MIMETypes::MIME_JSON;
                result->Body(Core::ProxyType<Web::IBody>(response));
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

    uint32_t LocationSync::TimeZone(string& timeZone /* @out */) const {
        timeZone = CurrentTimeZone();
        return Core::ERROR_NONE;
    }

    uint32_t LocationSync::TimeZone(const string& timeZone) {
        _adminLock.Lock();
        _timezoneoverride = timeZone;
        _adminLock.Unlock();
        Core::SystemInfo::Instance().SetTimeZone(_timezoneoverride);
        TimeZoneChanged(timeZone);
        return Core::ERROR_NONE;
    }

    string LocationSync::CurrentTimeZone() const {
        string timezone;
        _adminLock.Lock();
        if( _timezoneoverride.empty() == true ) {
            _adminLock.Unlock();
            PluginHost::ISubSystem* subSystem = _service->SubSystems();
            ASSERT(subSystem != nullptr);
            const PluginHost::ISubSystem::ILocation* location(subSystem->Get<PluginHost::ISubSystem::ILocation>());
            timezone = location->TimeZone();
        } else {
            timezone = _timezoneoverride;
            _adminLock.Unlock();
        }
        return timezone;
    }

    void LocationSync::TimeZoneChanged(const string& timezone) const {
        _adminLock.Lock();
        for (auto observer : _timezoneoberservers) {
            observer->TimeZoneChanged(timezone);
        }
        _adminLock.Unlock();
        Exchange::JTimeZone::Event::TimeZoneChanged(const_cast<PluginHost::JSONRPC&>(static_cast<const PluginHost::JSONRPC&>(*this)), timezone);
    }

    void LocationSync::SyncedLocation()
    {
        PluginHost::ISubSystem* subSystem = _service->SubSystems();

        ASSERT(subSystem != nullptr);

        if (subSystem != nullptr) {

            subSystem->Set(PluginHost::ISubSystem::INTERNET, _sink.Network());
            subSystem->Set(PluginHost::ISubSystem::LOCATION, _sink.Location());
            subSystem->Release();

            if ((_sink.Location() != nullptr) && (_sink.Location()->TimeZone().empty() == false)) {
                _adminLock.Lock();
                if( _timezoneoverride.empty() == true ) {
                    _adminLock.Unlock();
                    Core::SystemInfo::Instance().SetTimeZone(_sink.Location()->TimeZone());
                    TimeZoneChanged(_sink.Location()->TimeZone());
                } else {
                    _adminLock.Unlock();
                }
                SYSLOG(Logging::Startup, (_T("Local time %s."),Core::Time::Now().ToRFC1123(true).c_str()));
                event_locationchange();
            }
        }
    }

} // namespace Plugin
} // namespace WPEFramework
