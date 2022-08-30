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
#include "UtilsSecurityToken.h"
#include <memory>

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0
#define SERVER_DETAILS  "127.0.0.1:9998"

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(LocationSync, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    static Core::ProxyPoolType<Web::Response> responseFactory(4);
    static Core::ProxyPoolType<Web::JSONBodyType<LocationSync::Data>> jsonResponseFactory(4);

#ifdef __WINDOWS__
#pragma warning(disable : 4355)
#endif
    LocationSync::LocationSync()
        : _skipURL(0)
        , _source()
                , _interval(0)
                , _retries(0)
        , _sink(this)
        , _service(nullptr)
                , _networkReady(false)
    {
        RegisterAll();
            _netControlTimer.connect(std::bind(&LocationSync::onNetControlTimer, this));
    }
#ifdef __WINDOWS__
#pragma warning(default : 4355)
#endif

    LocationSync::~LocationSync() /* override */
    {
        UnregisterAll();
    }

    const string LocationSync::Initialize(PluginHost::IShell* service) /* override */
    {
        string result;
        Config config;
        config.FromString(service->ConfigLine());
        string version = service->Version();

        if (LocationService::IsSupported(config.Source.Value()) == Core::ERROR_NONE) {
            _skipURL = static_cast<uint16_t>(service->WebPrefix().length());
            _source = config.Source.Value();
                _interval = config.Interval.Value();
                _retries = config.Retries.Value();
            _service = service;
            TRACE(Trace::Information, (_T("Starting netcontrol timer. Source: %s, interval: %d, retries: %d, network check every %d ms")
                    , _source.c_str()
                    , _interval
                    , _retries
                    , _interval * 1000
            ));
            if(_netControlTimer.isActive()) {
                _netControlTimer.stop();
            }
            _netControlTimer.start(_interval * 1000);
        } else {
            result = _T("URL for retrieving location is incorrect !!!");
        }

        // On success return empty, to indicate there is no error text.
        return (result);
    }

    void LocationSync::Deinitialize(PluginHost::IShell* service VARIABLE_IS_NOT_USED) /* override */
    {
        ASSERT(_service == service);

            if(_netControlTimer.isActive()) {
                _netControlTimer.stop();
            }
        _sink.Deinitialize();
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
                result->Body(Core::proxy_cast<Web::IBody>(response));
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

    void LocationSync::SyncedLocation()
    {
        PluginHost::ISubSystem* subSystem = _service->SubSystems();

        ASSERT(subSystem != nullptr);

        if (subSystem != nullptr) {

            subSystem->Set(PluginHost::ISubSystem::INTERNET, _sink.Network());
            subSystem->Set(PluginHost::ISubSystem::LOCATION, _sink.Location());
            subSystem->Release();

            if ((_sink.Location() != nullptr) && (_sink.Location()->TimeZone().empty() == false)) {
                Core::SystemInfo::SetEnvironment(_T("TZ"), _sink.Location()->TimeZone());
                event_locationchange();
            }
        }
    }

        void LocationSync::onNetControlTimer()
        {
            static uint8_t remainingAttempts = _retries;
            bool networkReachable = getConnectivity();
            remainingAttempts--;
            TRACE(Trace::Information, (_T("Network is %s"), networkReachable ? "REACHABLE" : "UNREACHABLE"));
            if (networkReachable || remainingAttempts <= 0)
            {
                _netControlTimer.stop();
                TRACE(Trace::Information, (_T("Network reachability monitoring stopped.")));
                TRACE(Trace::Information, (_T("Proceeding with LocationService init.")));
                _sink.Initialize(_source, _interval, _retries);
            } else {
                TRACE(Trace::Information, (_T("Doing one more reachability check in %d sec, remaining attempts: %d"), _interval, remainingAttempts));
            }
        }
        bool LocationSync::getConnectivity()
        {
            JsonObject joGetParams;
            JsonObject joGetResult;
            std::string callsign = "org.rdk.Network.1";
            std::string token;
            /* check if plugin active */
            if (false == Utils::isPluginActivated("org.rdk.Network")) {
                TRACE(Trace::Fatal, ("Network plugin is not activated \n"));
                return false;
            }

            Utils::SecurityToken::getSecurityToken(token);
            string query = "token=" + token;
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), _T(SERVER_DETAILS));
            auto *thunder_client = new WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>(callsign.c_str(),"",false, query);

            if (thunder_client != nullptr) {
                uint32_t status = thunder_client->Invoke<JsonObject, JsonObject>(5000, "isConnectedToInternet", joGetParams, joGetResult);
                if (status > 0) {
                    TRACE(Trace::Fatal, ("%s call failed %d", callsign.c_str(), status));
                    return false;
                } else if (joGetResult.HasLabel("connectedToInternet")) {
                    TRACE(Trace::Information, ("connectedToInternet status %s",(joGetResult["connectedToInternet"].Boolean())? "true":"false"));
                    return joGetResult["connectedToInternet"].Boolean();
                } else {
                    return false;
                }
                delete thunder_client;
            }
            TRACE(Trace::Fatal, ("thunder client failed"));
            return false;
        }
        // TIMER
        Timer::Timer() :
                baseTimer(64 * 1024, "ThunderPluginBaseTimer")
                , m_timerJob(this)
                , m_isActive(false)
                , m_isSingleShot(false)
                , m_intervalInMs(-1)
        {}

        Timer::~Timer()
        {
            stop();
        }

        bool Timer::isActive()
        {
            return m_isActive;
        }

        void Timer::stop()
        {
            baseTimer.Revoke(m_timerJob);
            m_isActive = false;
        }

        void Timer::start()
        {
            baseTimer.Revoke(m_timerJob);
            baseTimer.Schedule(Core::Time::Now().Add(m_intervalInMs), m_timerJob);
            m_isActive = true;
        }

        void Timer::start(int msec)
        {
            setInterval(msec);
            start();
        }

        void Timer::setSingleShot(bool val)
        {
            m_isSingleShot = val;
        }

        void Timer::setInterval(int msec)
        {
            m_intervalInMs = msec;
        }

        void Timer::connect(std::function< void() > callback)
        {
            onTimeoutCallback = callback;
        }

        void Timer::Timed()
        {
            if(onTimeoutCallback != nullptr) {
                onTimeoutCallback();
            }
            // stop in case of a single shot call; start again if it has not been stopped
            if (m_isActive) {
                if(m_isSingleShot) {
                    stop();
                } else{
                    start();
                }
            }
        }

        uint64_t TimerJob::Timed(const uint64_t scheduledTime)
        {
            if(m_timer) {
                m_timer->Timed();
            }
            return 0;
        }
} // namespace Plugin
} // namespace WPEFramework
