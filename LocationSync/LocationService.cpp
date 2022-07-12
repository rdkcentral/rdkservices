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
 
#include "LocationService.h"

namespace WPEFramework {

namespace Plugin {

    struct IGeography {
        virtual ~IGeography() = default;

        virtual string Country() const = 0;
        virtual string City() const = 0;
        virtual string Region() const = 0;
        virtual string TimeZone() const = 0;
        virtual string Latitude() const = 0;
        virtual string Longitude() const = 0;
        virtual string IP() const = 0;
        virtual void FromString(const string&) = 0;
    };

    // http://ip-api.com/json
    // {
    //  "status":"success",
    //  "country":"Netherlands",
    //  "countryCode":"NL",
    //  "region":"GE",
    //  "regionName":"Gelderland",
    //  "city":"Wijchen",
    //  "lat":"51.798",
    //  "lon":"5.726",
    //  "zip":"6605",
    //  "timezone":"Europe/Amsterdam",
    //  "isp":"T-Mobile Thuis BV",
    //  "org":"T-Mobile Thuis BV",
    //  "as":"AS50266 T-Mobile Thuis BV",
    //  "query":"85.146.148.211"
    // }
    class IPAPI: public IGeography {
    private:
        class Data : public Core::JSON::Container {
        public:
            Data(const Data&) = delete;
            Data& operator=(const Data&) = delete;

            Data()
                : Core::JSON::Container()
                , Country()
                , City()
                , Region()
                , TimeZone()
                , Latitude()
                , Longitude()
                , IP()
            {
                Add(_T("country"), &Country);
                Add(_T("city"), &City);
                Add(_T("regionName"), &Region);
                Add(_T("timezone"), &TimeZone);
                Add(_T("latitude"), &Latitude);
                Add(_T("longitude"), &Longitude);
                Add(_T("query"), &IP);
            }
            ~Data() override = default;

        public:
            Core::JSON::String Country;
            Core::JSON::String City;
            Core::JSON::String Region;
            Core::JSON::String TimeZone;
            Core::JSON::String Latitude;
            Core::JSON::String Longitude;
            Core::JSON::String IP;
        };

    public:
        IPAPI(const IPAPI&) = delete;
        IPAPI& operator= (const IPAPI&) = delete;

        IPAPI() = default;
        ~IPAPI() override = default;

    public:
        string Country() const override
        {
            return (_data.Country.Value());
        }
        string City() const override
        {
            return (_data.City.Value());
        }
        string Region() const override
        {
            return (_data.Region.Value());
        }
        string TimeZone() const override
        {
            return (_data.TimeZone.Value());
        }
        string Latitude() const override
        {
            return (_data.Latitude.Value());
        }
        string Longitude() const override
        {
            return (_data.Longitude.Value());
        }
        string IP() const override
        {
            return (_data.IP.Value());
        }
        void FromString(const string& data) override {
            TRACE(Trace::Information, (_T("IPAPI: Received a response: [%s]!"), data.c_str()));
            _data.FromString(data);
        }

    private:
        Data _data;
    };

    // http://jsonip.metrological.com/?maf=true
    // {
    //  "ip":"85.146.148.211",
    //  "ping":true,
    //  "geo":
    //  {
    //   "country":"NL",
    //   "city":"Elst",
    //   "region":"GE",
    //   "tz":"CET-1CEST,M3.5.0,M10.5.0/3",
    //   "ll":
    //   [
    //    51.9201,
    //    5.836
    //   ]
    //  }
    // }
    class Metrological : public IGeography {
    private:
        class Data : public Core::JSON::Container {
        public:
            class Geography : public Core::JSON::Container {
            public:
                Geography(const Geography&) = delete;
                Geography& operator=(const Geography&) = delete;

                Geography()
                    : Core::JSON::Container()
                    , Country()
                    , City()
                    , Region()
                    , TimeZone()
                    , Latitude()
                    , Longitude()
                    , _LL()
                {
                    Add(_T("country"), &Country);
                    Add(_T("city"), &City);
                    Add(_T("region"), &Region);
                    Add(_T("tz"), &TimeZone);
                    Add(_T("lat"), &Latitude);
                    Add(_T("lon"), &Longitude);
                    Add(_T("ll"), &_LL);
                }
                ~Geography() override = default;

          public:
                Core::JSON::String Country;
                Core::JSON::String City;
                Core::JSON::String Region;
                Core::JSON::String TimeZone;
                Core::JSON::String Latitude;
                Core::JSON::String Longitude;

            private:
                Core::JSON::ArrayType<Core::JSON::Double> _LL;
            };

        public:
            Data(const Data&) = delete;
            Data& operator= (const Data&) = delete;

            Data()
                : Core::JSON::Container()
                , IP()
                , Geo()
                , Ping()
            {
                Add(_T("ip"), &IP);
                Add(_T("geo"), &Geo);
                Add(_T("ping"), &Ping);
            }
            ~Data() override = default;

        public:
            Core::JSON::String IP;
            Geography Geo;
            Core::JSON::Boolean Ping;
        };

    public:
        Metrological(const Metrological&) = delete;
        Metrological& operator= (const Metrological&) = delete;

        Metrological() = default;
        ~Metrological() override = default;

    public:
        string Country() const override
        {
            return (_data.Geo.Country.Value());
        }
        string City() const override
        {
            return (_data.Geo.City.Value());
        }
        string Region() const override
        {
            return (_data.Geo.Region.Value());
        }
        string TimeZone() const override
        {
            return (_data.Geo.TimeZone.Value());
        }
        string Latitude() const override
        {
            return (_data.Geo.Latitude.Value());
        }
        string Longitude() const
        {
            return (_data.Geo.Longitude.Value());
        }
        string IP() const override
        {
            return (_data.IP.Value());
        }
        void FromString(const string& data) override {
            TRACE(Trace::Information, (_T("Metrological: Received a response: [%s]!"), data.c_str()));

            _data.IElement::FromString(data);

            string parsed;
            _data.IElement::ToString(parsed);
            TRACE(Trace::Information, (_T("Metrological: reverted response: [%s]!"), parsed.c_str()));
        }

    private:
        Data _data;
    };

    static Core::ProxyPoolType<Web::Response> g_Factory(1);

    static Core::NodeId FindLocalIPV6()
    {
        Core::NodeId result;
        Core::AdapterIterator interfaces;

        while ((result.IsValid() == false) && (interfaces.Next() == true)) {

            Core::IPV6AddressIterator index(interfaces.IPV6Addresses());

            TRACE_GLOBAL(Trace::Information, (_T("Checking interface: %s"), interfaces.Name().c_str()));

            while ((result.IsValid() == false) && (index.Next() == true)) {
                if ((index.IsValid() == true) && (index.Address().IsUnicast())) {
                    TRACE_GLOBAL(Trace::Information, (_T("Found IPV6 address: %s"), index.Address().HostAddress().c_str()));
                    result = index.Address();
                }
            }
        }

        return (result);
    }

    struct DomainConstructor {
        const TCHAR* domainName;
        const uint32_t length;
        Core::ProxyType<IGeography> (*factory)();
    };

    DomainConstructor g_domainFactory[] = {
        { _TXT("jsonip.metrological.com"), []() -> Core::ProxyType<IGeography> { return (Core::ProxyType<IGeography>(Core::ProxyType<Plugin::Metrological>::Create())); } },
        { _TXT("ip-api.com"), []() -> Core::ProxyType<IGeography> { return (Core::ProxyType<IGeography>(Core::ProxyType<Plugin::IPAPI>::Create())); } }
    };

    static DomainConstructor* FindDomain(const Core::URL& domain)
    {
        uint32_t index = 0;

        while ((index < (sizeof(g_domainFactory) / sizeof(DomainConstructor))) && (domain.IsDomain(g_domainFactory[index].domainName, g_domainFactory[index].length) == false)) {
            index++;
        }

        return (index < (sizeof(g_domainFactory) / sizeof(DomainConstructor)) ? &(g_domainFactory[index]) : nullptr);
    }

#ifdef __WINDOWS__
#pragma warning(disable : 4355)
#endif
    LocationService::LocationService(Core::IDispatchType<void>* callback)
        : BaseClass(1, g_Factory, false, Core::NodeId(), Core::NodeId(), 256, 1024)
        , _adminLock()
        , _state(IDLE)
        , _remoteId()
        , _sourceNode()
        , _tryInterval(0)
        , _callback(callback)
        , _publicIPAddress()
        , _timeZone()
        , _country()
        , _region()
        , _city()
        , _activity(*this)
        , _infoCarrier()
        , _request(Core::ProxyType<Web::Request>::Create())
    {
    }
#ifdef __WINDOWS__
#pragma warning(default : 4355)
#endif

    LocationService::~LocationService() /* override */
    {

        Stop();

        Close(Core::infinite);
    }

    uint32_t LocationService::Probe(const string& remote, const uint32_t retries, const uint32_t retryTimeSpan)
    {
        uint32_t result = Core::ERROR_INPROGRESS;

        _adminLock.Lock();

        if ((_state == IDLE) || (_state == FAILED) || (_state == LOADED)) {

            ASSERT(_infoCarrier.IsValid() == false);

            result = Core::ERROR_GENERAL;

            // Determine the request
            Core::URL info(remote.c_str());

            if (info.IsValid() == true) {

                result = Core::ERROR_INCORRECT_URL;

                DomainConstructor* constructor = FindDomain(info);

                if (constructor != nullptr) {

                    const string hostName(info.Host().Value());

                    _state = ACTIVE;

                    // it runs till zero, so subtract by definition 1 :-)
                    _retries = (retries - 1);
                    _tryInterval = retryTimeSpan * 1000; // Move from seconds to mS.
                    _request->Host = hostName;
                    _request->Verb = Web::Request::HTTP_GET;
                    _request->Path = _T("/");
                    if (info.Path().IsSet() == true) {
                        _request->Path += info.Path().Value();
                    }
                    _remoteId = hostName;

                    if (info.Port().IsSet() == true) {
                        _remoteId += ':' + Core::NumberType<uint16_t>(info.Port().Value()).Text();
                    }
                    else {
                        _remoteId += ':' + Core::NumberType<uint16_t>(Core::URL::Port(info.Type())).Text();
                    }

                    if (info.Query().IsSet() == true) {
                        _request->Query = info.Query().Value();
                    }

                    _infoCarrier = constructor->factory();

                    _activity.Submit();

                    result = Core::ERROR_NONE;
                }
            }
        }

        _adminLock.Unlock();

        return (result);
    }

    void LocationService::Stop()
    {
        _adminLock.Lock();

        _activity.Revoke();

        if ((_state != IDLE) && (_state != FAILED) && (_state != LOADED)) {

            if (IsClosed() == false) {
                Close(0);
            }

            _state = FAILED;
        }

        if(_infoCarrier.IsValid() == true) {
            _infoCarrier.Release();
        }

        _adminLock.Unlock();
    }

    // Methods to extract and insert data into the socket buffers
    void LocationService::LinkBody(Core::ProxyType<Web::Response>& element) /* override */
    {
        if (element->ErrorCode == Web::STATUS_OK) {

            ASSERT(_infoCarrier.IsValid() == true);

            element->Body<Web::IBody>(Core::proxy_cast<Web::IBody>(Core::ProxyType<Web::TextBody>::Create()));
        }
    }

    void LocationService::Received(Core::ProxyType<Web::Response>& element) /* override */
    {
        Core::ProxyType<Web::TextBody> textInfo = element->Body<Web::TextBody>();

        if (textInfo.IsValid() == false) {
            TRACE(Trace::Information, (_T("Got a response but had an empty body!")));
        }
        else {

            _infoCarrier->FromString(*textInfo);

            _adminLock.Lock();

            _timeZone = _infoCarrier->TimeZone();
            _country = _infoCarrier->Country();
            _region = _infoCarrier->Region();
            _city = _infoCarrier->City();

            if (_state == IPV6_INPROGRESS) {

                // For now the source IPV6 is not returned but as IPV6 is not NAT'ed our IF Address should be
                // the outside IP address as well.
                Core::NodeId localId(FindLocalIPV6());

                ASSERT(localId.IsValid() == true);

                _publicIPAddress = localId.HostAddress();
            } else {
                _publicIPAddress = _infoCarrier->IP();
            }
            _state = LOADED;

            ASSERT(!_publicIPAddress.empty());

            Core::NodeId node(_publicIPAddress.c_str(), Core::NodeId::TYPE_UNSPECIFIED);

            if (node.IsValid() != true) {
                TRACE(Trace::Information, (_T("Could not determine the external public IP address [%s]"), _publicIPAddress.c_str()));
            }
            else {
                if (node.Type() == Core::NodeId::TYPE_IPV4) {
                    Core::NodeId::ClearIPV6Enabled();
                }

                TRACE(Trace::Information, (_T("Network connectivity established. Type: %s, on %s"), (node.Type() == Core::NodeId::TYPE_IPV6 ? _T("IPv6") : _T("IPv4")), node.HostAddress().c_str()));
                _callback->Dispatch();
            }

            ASSERT(_infoCarrier.IsValid() == true);

            _infoCarrier.Release();

            _adminLock.Unlock();
        }

        // Finish the cycle..
        _activity.Submit();
    }

    void LocationService::Send(const Core::ProxyType<Web::Request>& element VARIABLE_IS_NOT_USED) /* override */
    {
        // Not much to do, just so we know we are done...
        ASSERT(element == _request);
    }

    // Signal a state change, Opened, Closed or Accepted
    void LocationService::StateChange() /* override */
    {
        if (Link().IsOpen() == true) {

            // Send out a trigger to send the request
            Submit(_request);
            TRACE(Trace::Information, (_T("Connection open, Location requets submitted.")));
        } else if (Link().HasError() == true) {
            Close(0);

            _activity.Submit();
        }
    }

    // The network might be down, keep on trying until we have connectivity.
    // We start using IPV6, preferred network...
    void LocationService::Dispatch()
    {
        uint32_t result = Core::infinite;

        if ((Close(100) != Core::ERROR_NONE) || (IsClosed() == false)) {

            result = 500; // ms...Check again..
        } else {

            _adminLock.Lock();

            if (_state == IPV4_INPROGRESS) {
                _state = (_retries-- == 0 ? FAILED : ACTIVE);
            }

            if ((_state != LOADED) && (_state != FAILED)) {

                Core::NodeId remote(_remoteId.c_str(), ((_state == ACTIVE) && (Core::NodeId::IsIPV6Enabled()) ? Core::NodeId::TYPE_IPV6 : Core::NodeId::TYPE_IPV4));

                if (remote.IsValid() == false) {

                    TRACE(Trace::Warning, (_T("DNS resolving failed. Sleep for %d mS for attempt %d"), _tryInterval, _retries));

                    // Name resolving does not even work. Retry this after a few seconds, if we still can..
                    if (_retries-- == 0)
                        _state = FAILED;
                    else
                        result = _tryInterval;
                } else {
                    Link().LocalNode(remote.AnyInterface());
                    Link().RemoteNode(remote);

                    TRACE(Trace::Information, (_T("Probing [%s:%d] on [%s]"), remote.HostAddress().c_str(), remote.PortNumber(), remote.Type() == Core::NodeId::TYPE_IPV6 ? _T("IPV6") : _T("IPv4")));
                    _state = (remote.Type() == Core::NodeId::TYPE_IPV6 ? IPV6_INPROGRESS : IPV4_INPROGRESS);

                    uint32_t status = Open(0);

                    if ((status == Core::ERROR_NONE) || (status == Core::ERROR_INPROGRESS)) {

                        TRACE(Trace::Information, (_T("Sending out a network package on %s. Attempt: %d"), (remote.Type() == Core::NodeId::TYPE_IPV6 ? _T("IPv6") : _T("IPv4")), _retries));

                        // We need to get a response in the given time..
                        result = _tryInterval;
                    } else {
                        TRACE(Trace::Warning, (_T("Failed on network %s. Reschedule for the next attempt: %d"), (remote.Type() == Core::NodeId::TYPE_IPV6 ? _T("IPv6") : _T("IPv4")), _retries));

                        // Seems we could not open this connection, move on to the next attempt.
                        Close(0);
                        result = 100;
                    }
                }
            }

            if (_state == FAILED) {
                _infoCarrier.Release();
            }

            _adminLock.Unlock();
        }

        if (_state == FAILED) {
            Core::NodeId::ClearIPV6Enabled();

            TRACE(Trace::Error, (_T("LocationSync: Network connectivity could *NOT* be established. Falling back to IPv4. %d"), __LINE__));
            _callback->Dispatch();
        }

        // See if we need rescheduling
        if (result != Core::infinite) {
            _activity.Schedule(Core::Time::Now().Add(result));
        }
    }

    /* static */ uint32_t LocationService::IsSupported(const string& remoteNode)
    {
        uint32_t result(Core::ERROR_GENERAL);
        Core::URL domain(remoteNode);

        if (domain.IsValid() == true) {

            result = Core::ERROR_INCORRECT_URL;

            if (FindDomain(domain) != nullptr) {
                result = Core::ERROR_NONE;
            } else {
                TRACE_GLOBAL(Trace::Error, (_T("URL is not valid. %s"), remoteNode.c_str()));
            }
        } else {
            TRACE_GLOBAL(Trace::Error, (_T("Domain is not valid. %s"), remoteNode.c_str()));
        }

        return (result);
    }

} // namespace Plugin
} // namespace WPEFramework
