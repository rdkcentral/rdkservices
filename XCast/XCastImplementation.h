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

#pragma once

#include "Module.h"
#include <interfaces/Ids.h>
#include <interfaces/IXCast.h>
#include "tracing/Logging.h"
#include "RtXcastConnector.h"
#include "RtNotifier.h"
#include <vector>

namespace WPEFramework {
namespace Plugin {

    class XCastImplementation : public Exchange::IXCast, public PluginHost::IStateControl, public RtNotifier {
    public:
        enum Event {
                LAUNCH_REQUEST_WITH_PARAMS,
                LAUNCH_REQUEST,
                STOP_REQUEST,
                HIDE_REQUEST,
                STATE_REQUEST,
                RESUME_REQUEST
            };

        class EXTERNAL Job : public Core::IDispatch {
        protected:
             Job(XCastImplementation *tts, Event event,string callsign,JsonObject &params)
                : _xcast(tts)
                , _event(event)
                , _callsign(callsign)
                , _params(params) {
                if (_xcast != nullptr) {
                    _xcast->AddRef();
                }
            }

       public:
            Job() = delete;
            Job(const Job&) = delete;
            Job& operator=(const Job&) = delete;
            ~Job() {
                if (_xcast != nullptr) {
                    _xcast->Release();
                }
            }

       public:
            static Core::ProxyType<Core::IDispatch> Create(XCastImplementation *tts, Event event,string callsign,JsonObject params) {
#ifndef USE_THUNDER_R4
                return (Core::proxy_cast<Core::IDispatch>(Core::ProxyType<Job>::Create(tts, event, callsign, params)));
#else
                return (Core::ProxyType<Core::IDispatch>(Core::ProxyType<Job>::Create(tts, event, callsign, params)));
#endif
            }

            virtual void Dispatch() {
                _xcast->Dispatch(_event, _callsign, _params);
            }

        private:
            XCastImplementation *_xcast;
            const Event _event;
            const string _callsign;
            const JsonObject _params;
        };

    public:
        // We do not allow this plugin to be copied !!
        XCastImplementation(const XCastImplementation&) = delete;
        XCastImplementation& operator=(const XCastImplementation&) = delete;

        virtual void Register(Exchange::IXCast::INotification* sink) override ;
        virtual void Unregister(Exchange::IXCast::INotification* sink) override ;

        virtual PluginHost::IStateControl::state State() const override { return PluginHost::IStateControl::RESUMED; }
        virtual uint32_t Request(const command state) override;
        virtual void Register(IStateControl::INotification* notification) override {}
        virtual void Unregister(IStateControl::INotification* notification) override {}
        virtual uint32_t Configure(PluginHost::IShell* service) override;

        virtual uint32_t applicationStateChanged(const string& appName, const string& appstate, const string& appId, const string& error) const override;
        virtual uint32_t enableCastService(string friendlyname,bool enableService) const override;
        virtual uint32_t getProtocolVersion(string &protocolVersion) const override;
        virtual uint32_t registerApplications(Exchange::IXCast::IApplicationInfoIterator* const appLists) override;

        virtual void onXcastApplicationLaunchRequestWithLaunchParam (string appName, string strPayLoad, string strQuery, string strAddDataUrl) override ;
        virtual void onXcastApplicationLaunchRequest(string appName, string parameter) override ;
        virtual void onXcastApplicationStopRequest(string appName, string appId) override ;
        virtual void onXcastApplicationHideRequest(string appName, string appId) override ;
        virtual void onXcastApplicationResumeRequest(string appName, string appId) override ;
        virtual void onXcastApplicationStateRequest(string appName, string appId) override ;

        BEGIN_INTERFACE_MAP(XCastImplementation)
        INTERFACE_ENTRY(Exchange::IXCast)
        INTERFACE_ENTRY(PluginHost::IStateControl)
        END_INTERFACE_MAP

    private:
        static RtXcastConnector* _rtConnector;
        mutable Core::CriticalSection _adminLock;
        std::list<Exchange::IXCast::INotification*> _notificationClients;
        
        void dispatchEvent(Event,string callsign, const JsonObject &params);
        void Dispatch(Event event,string callsign, const JsonObject params);

    public:
        XCastImplementation();
        virtual ~XCastImplementation();

        friend class Job;
    };
} // namespace Plugin
} // namespace WPEFramework