/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2024 RDK Management
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
#include <interfaces/IHelloWorld.h>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>
//#include "tracing/Logging.h"

//#include <com/com.h>
//#include <core/core.h>
//#include <plugins/plugins.h>

#define HELLOWORLD_NAMESPACE "HelloWorld"

namespace WPEFramework {
namespace Plugin {


    class HelloWorldImplementation : public Exchange::IHelloWorld {

    public:
        HelloWorldImplementation();
        ~HelloWorldImplementation() override;

        // We do not allow this plugin to be copied !!
        HelloWorldImplementation(const HelloWorldImplementation&) = delete;
        HelloWorldImplementation& operator=(const HelloWorldImplementation&) = delete;

        BEGIN_INTERFACE_MAP(HelloWorldImplementation)
        INTERFACE_ENTRY(Exchange::IHelloWorld)
        END_INTERFACE_MAP

    public:

        enum EventNames {
          ECHOMESSAGE
        };
        class EXTERNAL Job : public Core::IDispatch {
        protected:
             Job(HelloWorldImplementation *helloWorldImplementation, EventNames event, JsonValue &params)
                : _helloWorldImplementation(helloWorldImplementation)
                , _event(event)
                , _params(params) {
                if (_helloWorldImplementation != nullptr) {
                    _helloWorldImplementation->AddRef();
                }
            }

       public:
            Job() = delete;
            Job(const Job&) = delete;
            Job& operator=(const Job&) = delete;
            ~Job() {
                if (_helloWorldImplementation != nullptr) {
                    _helloWorldImplementation->Release();
                }
            }

            typedef websocketpp::client<websocketpp::config::asio_client> client;
            class websocket_endpoint {
            public:
                websocket_endpoint () {
                    m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
                    m_endpoint.clear_error_channels(websocketpp::log::elevel::all);
            
                    m_endpoint.init_asio();
                    m_endpoint.start_perpetual();
            
                    m_thread.reset(new websocketpp::lib::thread(&client::run, &m_endpoint));
                }
            private:
                client m_endpoint;
                websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;
            };
       public:
            static Core::ProxyType<Core::IDispatch> Create(HelloWorldImplementation *helloWorldImplementation, EventNames event, JsonValue params) {
#ifndef USE_THUNDER_R4
                return (Core::proxy_cast<Core::IDispatch>(Core::ProxyType<Job>::Create(helloWorldImplementation, event, params)));
#else
                return (Core::ProxyType<Core::IDispatch>(Core::ProxyType<Job>::Create(helloWorldImplementation, event, params)));
#endif
            }

            virtual void Dispatch() {
                _helloWorldImplementation->Dispatch(_event, _params);
            }
        private:
            HelloWorldImplementation *_helloWorldImplementation;
            const EventNames _event;
            const JsonValue _params;
        };

    public:
        virtual uint32_t Register(Exchange::IHelloWorld::INotification *notification ) override ;
        virtual uint32_t Unregister(Exchange::IHelloWorld::INotification *notification ) override ;

        virtual Core::hresult GetHelloWorldMessage(string& result) override;
        virtual Core::hresult SetHelloWorldMessage(const string& message) override;
        virtual Core::hresult LogHelloWorldMessage() override;
        virtual Core::hresult LogHelloWorldMessageComRpcOnly() override;
        virtual Core::hresult LogHelloWorldMessageJsonRpcOnly() override;
        virtual Core::hresult Echo(const string& message) override;

    private:
        mutable Core::CriticalSection _adminLock;
        std::list<Exchange::IHelloWorld::INotification*> _helloWorldNotification;
        string mMessage; 

        void dispatchEvent(EventNames, const JsonValue &params);
        void Dispatch(EventNames event, const JsonValue params);

        friend class Job;
    };
} // namespace Plugin
} // namespace WPEFramework
