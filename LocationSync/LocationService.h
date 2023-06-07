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
 
#ifndef LOCATIONSYNC_LOCATIONSERVICE_H
#define LOCATIONSYNC_LOCATIONSERVICE_H

#include "Module.h"

namespace WPEFramework {

namespace Plugin {

    struct IGeography;

    class EXTERNAL LocationService
        : public PluginHost::ISubSystem::ILocation,
          public PluginHost::ISubSystem::IInternet,
          public Web::WebLinkType<Core::SocketStream, Web::Response, Web::Request, Core::ProxyPoolType<Web::Response>&> {

    private:
        enum state {
            IDLE,
            ACTIVE,
            IPV6_INPROGRESS,
            IPV4_INPROGRESS,
            LOADED,
            FAILED
        };

        using Job = Core::ThreadPool::JobType<LocationService>;

    private:
        LocationService() = delete;
        LocationService(const LocationService&) = delete;
        LocationService& operator=(const LocationService&) = delete;

        typedef Web::WebLinkType<Core::SocketStream, Web::Response, Web::Request, Core::ProxyPoolType<Web::Response>&> BaseClass;

    public:
#ifndef USE_THUNDER_R4
        LocationService(Core::IDispatchType<void>* update);
#else
        LocationService(Core::IDispatch* update);
#endif /* USE_THUNDER_R4 */
        ~LocationService() override;

    public:
        BEGIN_INTERFACE_MAP(LocationService)
        INTERFACE_ENTRY(PluginHost::ISubSystem::ILocation)
        INTERFACE_ENTRY(PluginHost::ISubSystem::IInternet)
        END_INTERFACE_MAP

        // Retry TimeSpan is in Seconds.

        static uint32_t IsSupported(const string& remoteNode);
        uint32_t Probe(const string& remoteNode, const uint32_t retries, const uint32_t retryTimeSpan);
        void Stop();

        /*
        * ------------------------------------------------------------------------------------------------------------
        * ISubSystem::INetwork methods
        * ------------------------------------------------------------------------------------------------------------
        */
        string PublicIPAddress() const override
        {
            return (_publicIPAddress);
        }
        network_type NetworkType() const override
        {
            return (_publicIPAddress.empty() == true ? PluginHost::ISubSystem::IInternet::UNKNOWN : (Core::NodeId::IsIPV6Enabled() ? PluginHost::ISubSystem::IInternet::IPV6 : PluginHost::ISubSystem::IInternet::IPV4));
        }
        /*
        * ------------------------------------------------------------------------------------------------------------
        * ISubSystem::ILocation methods
        * ------------------------------------------------------------------------------------------------------------
        */
        string TimeZone() const override
        {
            return (_timeZone);
        }
        string Country() const override
        {
            return (_country);
        }
        string Region() const override
        {
            return (_region);
        }
        string City() const override
        {
            return (_city);
        }

    private:
        // Notification of a Partial Request received, time to attach a body..
        void LinkBody(Core::ProxyType<Web::Response>& element) override;
        void Received(Core::ProxyType<Web::Response>& element) override;
        void Send(const Core::ProxyType<Web::Request>& element) override;

        // Signal a state change, Opened, Closed or Accepted
        void StateChange() override;

        friend Core::ThreadPool::JobType<LocationService&>;
        void Dispatch();

    private:
        Core::CriticalSection _adminLock;
        state _state;
        string _remoteId;
        Core::NodeId _sourceNode;
        uint32_t _tryInterval;
        uint32_t _retries;
        Core::IDispatch* _callback;
        string _publicIPAddress;
        string _timeZone;
        string _country;
        string _region;
        string _city;
        Core::WorkerPool::JobType<LocationService&> _activity;
        Core::ProxyType<IGeography> _infoCarrier;
        Core::ProxyType<Web::Request> _request;
    };
}
} // namespace WPEFramework:Plugin

#endif // LOCATIONSYNC_LOCATIONSERVICE_H
