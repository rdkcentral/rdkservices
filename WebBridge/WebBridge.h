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

namespace WPEFramework {
    namespace Plugin {

        class WebBridge :
            public PluginHost::IPluginExtended,
            public PluginHost::IDispatcher,
            public PluginHost::IWebSocket,
        {
        private:
            enum class state {
                STATE_INCORRECT_HANDLER,
                STATE_INCORRECT_VERSION,
                STATE_UNKNOWN_METHOD,
                STATE_REGISTRATION,
                STATE_UNREGISTRATION,
                STATE_EXISTS,
                STATE_NONE_EXISTING,
                STATE_CUSTOM
            };
            class Request {
            public:
                Request() = delete;
                Request(Request&&) = delete;
                Request(const Request&) = delete;
                Request& operator=(const Request&) = delete;

                Request(IDispatcher::ICallback* callback, const uint32_t channelId, const uint32_t sequenceId, const Core::Time& timeOut)
                    : _callback(callback)
                    , _channelId(channelId)
                    , _sequenceId(sequenceId)
                    , _issued(timeOut) {
                    _callback->AddRef();
                }
                ~Request() {
                    ASSERT(_callback != nullptr);
                    _callback->Release();
                    _callback = nullptr;
                }
                bool operator== (const IDispatcher::ICallback* callback) const {
                    return (_callback == callback);
                }
                bool operator!= (const IDispatcher::ICallback* callback) const {
                    return (!operator==(callback));
                }

            public:
                IDispatcher::ICallback* Callback() {
                    _callback->AddRef();
                    return (_callback);
                }
                uint32_t ChannelId() const {
                    return (_channelId);
                }
                uint32_t SequenceId() const {
                    return (_sequenceId);
                }
                const Core::Time& Issued() const {
                    return (_issued);
                }

            private:
                IDispatcher::ICallback* _callback;
                uint32_t _channelId;
                uint32_t _sequenceId;
                Core::Time _issued;
            };
            using PendingMap = std::unordered_map<uint32_t, Request>;

        public:
            enum context : uint8_t {
                NONE,
                ADDED,
                WRAPPED
            };
            class Config : public Core::JSON::Container {
            public:
                Config(Config&&) = delete;
                Config(const Config&) = delete;
                Config& operator= (const Config&) = delete;

                Config()
                    : Core::JSON::Container()
                    , TimeOut(3000)
                    , Context(context::NONE)
                {
                    Add(_T("timeout"), &TimeOut);
                    Add(_T("context"), &Context);
                }
                ~Config() override = default;

            public:
                Core::JSON::DecUInt16 TimeOut;
                Core::JSON::EnumType<context> Context;
            };

        public:
            WebBridge(WebBridge&&) = delete;
            WebBridge(const WebBridge&) = delete;
            WebBridge& operator=(const WebBridge&) = delete;

            #ifdef __WINDOWS__
            #pragma warning(disable: 4355)
            #endif
            WebBridge()
                : _adminLock()
                , _skipURL(0)
                , _mode(context::NONE)
                , _service(nullptr)
                , _callsign()
                , _pendingRequests()
                , _javascriptService(0)
                , _sequenceId(1)
                , _timeOut(0)
                , _cleaner(*this)
            {
           }
           #ifdef __WINDOWS__
           #pragma warning(default: 4355)
           #endif
           ~WebBridge() override = default;

            BEGIN_INTERFACE_MAP(WebBridge)
                INTERFACE_ENTRY(PluginHost::IPlugin)
                INTERFACE_ENTRY(PluginHost::IPluginExtended)
                INTERFACE_ENTRY(PluginHost::IWebSocket)
                INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

        public:
            //   IPlugin methods
            // -------------------------------------------------------------------------------------------------------
            //! ==================================== CALLED ON THREADPOOL THREAD ======================================
            const string Initialize(PluginHost::IShell* service) override;
            //! ==================================== CALLED ON THREADPOOL THREAD ======================================
            void Deinitialize(PluginHost::IShell* service) override;
            //! ==================================== CALLED ON THREADPOOL THREAD ======================================
            string Information() const override;

            // IDispatcher (override message)
            // -------------------------------------------------------------------------------------------------------
#fndef USE_THUNDER_R4
            //! ==================================== CALLED ON THREADPOOL THREAD ======================================
            Core::ProxyType<Core::JSONRPC::Message> Invoke(const string& token, const uint32_t channelId, const Core::JSONRPC::Message& message) override;
            //! ==================================== CALLED ON THREADPOOL THREAD ======================================
            void Activate(PluginHost::IShell* service) override;
            //! ==================================== CALLED ON THREADPOOL THREAD ======================================
            void Deactivate() override;
#else
            //! ==================================== CALLED ON THREADPOOL THREAD ======================================
            Core::hresult Invoke(const uint32_t channelId, const uint32_t id, const string& token, const string& method, const string& parameters, string& response) override;
            Core::hresult Revoke(ICallback* callback) override;
#endif
            // IPluginExtended
            // -------------------------------------------------------------------------------------------------------
            //! ================================== CALLED ON COMMUNICATION THREAD =====================================
            bool Attach(PluginHost::Channel& channel) override;
            //! ================================== CALLED ON COMMUNICATION THREAD =====================================
            void Detach(PluginHost::Channel& channel) override;

            // IWebSocket
            // -------------------------------------------------------------------------------------------------------
            //! ================================== CALLED ON COMMUNICATION THREAD =====================================
            Core::ProxyType<Core::JSON::IElement> Inbound(const string& identifier) override;
            //! ==================================== CALLED ON THREADPOOL THREAD ======================================
            Core::ProxyType<Core::JSON::IElement> Inbound(const uint32_t ID, const Core::ProxyType<Core::JSON::IElement>& element) override;

        private:
            friend Core::ThreadPool::JobType<WebBridge&>;
            void Dispatch();
            bool InternalMessage(const Core::ProxyType<Core::JSONRPC::Message>& message);

        private:
            Core::CriticalSection _adminLock;
            uint8_t _skipURL;
            context _mode;
            PluginHost::IShell* _service;
            string _callsign;
            PendingMap _pendingRequests;
            uint32_t _javascriptService;
            uint32_t _sequenceId;
            uint32_t _timeOut;
            Core::WorkerPool::JobType<WebBridge&> _cleaner;
        };

    } // namespace Plugin
} // namespace WPEFramework
