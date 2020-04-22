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
        virtual ~IGeography() {}

        virtual string Country() const = 0;
        virtual string City() const = 0;
        virtual string Region() const = 0;
        virtual string TimeZone() const = 0;
        virtual string Latitude() const = 0;
        virtual string Longitude() const = 0;
        virtual string IP() const = 0;
        virtual void Updated() = 0;
    };

    class Geography : public IGeography, public Core::JSON::Container {

    private:
        Geography& operator=(const Geography&) = delete;

    public:
        Geography()
            : Core::JSON::Container()
            , _country()
            , _city()
            , _region()
            , _timeZone()
            , _latitude()
            , _IP()
            , _longitude()
        {
            Add(_T("country"), &_country);
            Add(_T("city"), &_city);
            Add(_T("region"), &_region);

            // ip-api.com specific
            Add(_T("tz"), &_timeZone);
            Add(_T("ll"), &_latitude);

            // ip-api.com specific
            Add(_T("timezone"), &_timeZone);
            Add(_T("query"), &_IP);
            Add(_T("lat"), &_latitude);
            Add(_T("lon"), &_longitude);
        }
        Geography(const Geography& copy)
            : Core::JSON::Container()
            , _country(copy._country)
            , _city(copy._city)
            , _region(copy._region)
            , _timeZone(copy._timeZone)
            , _latitude(copy._latitude)
            , _IP(copy._IP)
            , _longitude(copy._longitude)
        {
            Add(_T("country"), &_country);
            Add(_T("city"), &_city);
            Add(_T("region"), &_region);

            // ip-api.com specific
            Add(_T("tz"), &_timeZone);
            Add(_T("ll"), &_latitude);

            // ip-api.com specific
            Add(_T("timezone"), &_timeZone);
            Add(_T("query"), &_IP);
            Add(_T("lat"), &_latitude);
            Add(_T("lon"), &_longitude);
        }
        virtual ~Geography()
        {
        }

    public:
        virtual string Country() const
        {
            return (_country.Value());
        }
        virtual string City() const
        {
            return (_city.Value());
        }
        virtual string Region() const
        {
            return (_region.Value());
        }
        virtual string TimeZone() const
        {
            return (_timeZone.Value());
        }
        virtual string Latitude() const
        {
            return (_latitude.Value());
        }
        virtual string Longitude() const
        {
            return (_longitude.Value());
        }
        virtual string IP() const
        {
            return (_IP.Value());
        }
        virtual void Updated()
        {
        }

        inline void Location(const string& latitude, const string& longitude)
        {
            _latitude = latitude;
            _longitude = longitude;
        }

    private:
        Core::JSON::String _country;
        Core::JSON::String _city;
        Core::JSON::String _region;
        Core::JSON::String _timeZone;
        Core::JSON::String _latitude;

        // ip-api.com specific
        Core::JSON::String _IP;
        Core::JSON::String _longitude;
    };

    class Response : public IGeography, public Core::JSON::Container {
    private:
        Response(const Response&) = delete;
        Response& operator=(const Response&) = delete;

    public:
        Response()
            : Core::JSON::Container()
            , _IP()
            , _ping()
            , _geo()
        {
            Add(_T("ip"), &_IP);
            Add(_T("ping"), &_ping);
            Add(_T("geo"), &_geo);
        }
        virtual ~Response()
        {
        }

    public:
        virtual string Country() const
        {
            return (_geo.Country());
        }
        virtual string City() const
        {
            return (_geo.City());
        }
        virtual string Region() const
        {
            return (_geo.Region());
        }
        virtual string TimeZone() const
        {
            return (_geo.TimeZone());
        }
        virtual string Latitude() const
        {
            return (_geo.Latitude());
        }
        virtual string Longitude() const
        {
            return (_geo.Longitude());
        }
        virtual string IP() const
        {
            return (_IP.Value());
        }
        virtual void Updated()
        {
            string info = _geo.Latitude();

            size_t location = info.find(',', 1);

            if (location != string::npos) {
                _geo.Location(info.substr(1, location - 1), info.substr(location + 1, info.length() - location - 2));
            }
        }

    private:
        Core::JSON::String _IP;
        Core::JSON::Boolean _ping;
        Geography _geo;
    };

    static Core::ProxyPoolType<Web::Response> g_Factory(1);

    static Core::NodeId FindLocalIPV6()
    {
        Core::NodeId result;
        Core::AdapterIterator interfaces;

        while ((result.IsValid() == false) && (interfaces.Next() == true)) {

            Core::IPV6AddressIterator index(interfaces.IPV6Addresses());

            TRACE_L1("Checking interface: %s", interfaces.Name().c_str());

            while ((result.IsValid() == false) && (index.Next() == true)) {
                if ((index.IsValid() == true) && (index.Address().IsUnicast())) {
                    TRACE_L1("Found IPV6 address: %s", index.Address().HostAddress().c_str());
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
        { _TXT("jsonip.metrological.com"), []() -> Core::ProxyType<IGeography> { return Core::ProxyType<Plugin::IGeography>(Core::ProxyType<Web::JSONBodyType<Response>>::Create()); } },
        { _TXT("ip-api.com"), []() -> Core::ProxyType<IGeography> { return Core::ProxyType<Plugin::IGeography>(Core::ProxyType<Web::JSONBodyType<Geography>>::Create()); } }
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

    /* virtual */ LocationService::~LocationService()
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

                    string fullRequest; _request->ToString(fullRequest);
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
    /* virtual */ void LocationService::LinkBody(Core::ProxyType<Web::Response>& element)
    {
        if (element->ErrorCode == Web::STATUS_OK) {

            ASSERT(_infoCarrier.IsValid() == true);

            element->Body<Web::IBody>(Core::proxy_cast<Web::IBody>(_infoCarrier));
        }
    }

    /* virtual */ void LocationService::Received(Core::ProxyType<Web::Response>& element)
    {
        if (element->HasBody() == true) {

            // ASSERT(element->Body<Web::JSONBodyType<IGeography> >() == _response);

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

            if (node.IsValid() == true) {

                if (node.Type() == Core::NodeId::TYPE_IPV4) {
                    Core::NodeId::ClearIPV6Enabled();
                }

                TRACE_L1("Network connectivity established on %s. ip: %s, tz: %s, country: %s",
                    node.Type() == Core::NodeId::TYPE_IPV4 ? _T("IPv4") : _T("IP6"),
                    _publicIPAddress.c_str(),
                    _timeZone.c_str(),
                    _country.c_str());

                TRACE(Trace::Information, (_T("LocationSync: Network connectivity established. Type: %s, on %s"), (node.Type() == Core::NodeId::TYPE_IPV6 ? _T("IPv6") : _T("IPv4")), node.HostAddress().c_str()));
                _callback->Dispatch();
            }

            ASSERT(_infoCarrier.IsValid() == true);

            _infoCarrier.Release();

            _adminLock.Unlock();
        } else {
            TRACE_L1("Got a response but had an empty body. %d", __LINE__);
        }

        // Finish the cycle..
        _activity.Submit();
    }

    /* virtual */ void LocationService::Send(const Core::ProxyType<Web::Request>& element)
    {
        // Not much to do, just so we know we are done...
        ASSERT(element == _request);
    }

    // Signal a state change, Opened, Closed or Accepted
    /* virtual */ void LocationService::StateChange()
    {
        if (Link().IsOpen() == true) {

            // Send out a trigger to send the request
            Submit(_request);
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

        if ((IsClosed() == false) || (Close(100) != Core::ERROR_NONE)) {

            result = 500; // ms...Check again..
        } else {

            _adminLock.Lock();

            if (_state == IPV4_INPROGRESS) {
                _state = (_retries-- == 0 ? FAILED : ACTIVE);
            }

            if ((_state != LOADED) && (_state != FAILED)) {

                Core::NodeId remote(_remoteId.c_str(), ((_state == ACTIVE) && (Core::NodeId::IsIPV6Enabled()) ? Core::NodeId::TYPE_IPV6 : Core::NodeId::TYPE_IPV4));

                if (remote.IsValid() == false) {

                    TRACE_L1("DNS resolving failed. Sleep for %d mS for attempt %d", _tryInterval, _retries);

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

                        TRACE_L1("Sending out a network package on %s. Attempt: %d", (remote.Type() == Core::NodeId::TYPE_IPV6 ? _T("IPv6") : _T("IPv4")), _retries);

                        // We need to get a response in the given time..
                        result = _tryInterval;
                    } else {
                        TRACE_L1("Failed on network %s. Reschedule for the next attempt: %d", (remote.Type() == Core::NodeId::TYPE_IPV6 ? _T("IPv6") : _T("IPv4")), _retries);

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

            TRACE(Trace::Information, (_T("LocationSync: Network connectivity could *NOT* be established. Falling back to IPv4. %d"), __LINE__));
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
                TRACE_L1("URL is not valid. %s", remoteNode.c_str());
            }
        } else {
            TRACE_L1("Domain is not valid. %s", remoteNode.c_str());
        }

        return (result);
    }

} // namespace Plugin
} // namespace WPEFramework
