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
#include <functional>
#include <curl/curl.h>

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(LocationSync, 1, 0);

    const u_int32_t NETCONTROL_CHECK_TIME_IN_MILLISECONDS(5500);
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
            TRACE(Trace::Fatal, (_T("Starting netcontrol timer. Source: %s, interval: %d, retries: %d, network check every %d ms")
                    , _source.c_str()
                    , _interval
                    , _retries
                    , NETCONTROL_CHECK_TIME_IN_MILLISECONDS
                    ));
            if(_netControlTimer.isActive()) {
                _netControlTimer.stop();
            }
            _netControlTimer.start(NETCONTROL_CHECK_TIME_IN_MILLISECONDS);
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
        WPE_INET_Result connectivity = getConnectivity(_source);
        bool networkReachable = (connectivity == WPE_INET_CONNECTED);
        TRACE(Trace::Fatal, (_T("Network is %s"), networkReachable ? "REACHABLE" : "UNREACHABLE"));
        if (networkReachable)
        {
            _netControlTimer.stop();
            TRACE(Trace::Fatal, (_T("Network reachability monitoring stopped.")));
            TRACE(Trace::Fatal, (_T("Proceeding with LocationService init.")));
            _sink.Initialize(_source, _interval, _retries);

        } else {
            TRACE(Trace::Information, (_T("Doing one more reachability check in %2.2f sec."),(float)NETCONTROL_CHECK_TIME_IN_MILLISECONDS / 1000));
        }
    }

    static size_t writeCurlResponse(void *ptr, size_t size, size_t nmemb, std::string stream)
    {
        size_t realsize = size * nmemb;
        std::string temp(static_cast<const char*>(ptr), realsize);
        stream.append(temp);
        return realsize;
    }

    LocationSync::WPE_INET_Result
    LocationSync::getConnectivity(const std::string& source)
    {
        TRACE(Trace::Information, (_T("Using internal connectivity check")));
        WPE_INET_Result connectivity = WPE_INET_DISCONNECTED;
        std::string response;
        long http_code = 200;
        CURLcode res = CURLE_OK;
        CURL *curl_handle = nullptr;
        struct curl_slist *list = nullptr;
        static unsigned int idx = 0;
        long connect_timeout = 2L;

        curl_handle = curl_easy_init();
        static std::vector<std::string>urls;
        if (source.length() > 0)
        {
            urls.emplace_back(source.c_str());
        } else {
            TRACE(Trace::Fatal, (_T("The supplied remote url is invalid, ignored")));
        }
        // some well known Internet addresses
        urls.emplace_back("comcast.net");
        urls.emplace_back("example.com");

        std::string url = urls.at(idx);
        TRACE(Trace::Information, (_T("Checking connectivity against %s. Test method: getting HTTP headers, max-time: %ld s"), url.c_str(), connect_timeout));

        list = curl_slist_append(list, "Connection: close");
        if (curl_handle &&
            !curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str()) &&
            !curl_easy_setopt(curl_handle, CURLOPT_HTTPGET,1) &&
            !curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, connect_timeout) &&
            !curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeCurlResponse) &&
            !curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, list) &&
            !curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response))
        {
            res = curl_easy_perform(curl_handle);
            if(curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code) != CURLE_OK)
            {
                TRACE(Trace::Fatal, (_T("curl_easy_getinfo failed")));
            }
            TRACE(Trace::Fatal, (_T("curl response code: %d; http response code: %ld"), res, http_code ));
            curl_slist_free_all(list);
            curl_easy_cleanup(curl_handle);
        } else {
            TRACE(Trace::Fatal, (_T("Could not perform connectivity check with curl. Reachability is unknown")));
            connectivity = WPE_INET_UNKNOWN;
        }

        if (res == CURLE_OK) {
            connectivity = WPE_INET_CONNECTED;
        } else {
            TRACE(Trace::Information, (_T("Reachability check failure; curl response: %d, http code: %ld"), res, http_code));
            TRACE(Trace::Information, (_T("Assuming Network is not ready yet")));
            connectivity = WPE_INET_DISCONNECTED;
        }
        ++idx;
        if(idx >= urls.size()) idx = 0;
        return connectivity;
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
