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
        public PluginHost::IWebSocket
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
        class Observer {
        public:
            Observer(const Observer&) = delete;
            Observer& operator=(const Observer&) = delete;

            Observer(const uint32_t id, const string& designator)
                : _id(id)
                , _designator(designator)
            {
            }
            ~Observer() = default;

        public:
            bool operator==(const Observer& rhs) const
            {
                return ((rhs._id == _id) && (rhs._designator == _designator));
            }
            bool operator!=(const Observer& rhs) const
            {
                return (!operator==(rhs));
            }

            uint32_t Id() const
            {
                return (_id);
            }
            const string& Designator() const
            {
                return (_designator);
            }

        private:
            uint32_t _id;
            string _designator;
        };
        class Request {
        public:
            Request() = delete;
            Request(const Request&) = delete;
            Request& operator=(const Request&) = delete;

            Request(const uint32_t channelId, const uint32_t sequenceId, const Core::Time& timeOut)
                : _channelId(channelId)
                , _sequenceId(sequenceId)
                , _issued(timeOut) {
            }
            ~Request() = default;

        public:
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
            uint32_t _channelId;
            uint32_t _sequenceId;
            Core::Time _issued;
        };
        class Cleaner  {
        private:
            using BaseClass = Core::IWorkerPool::JobType<Cleaner>;

        public:
            Cleaner(const Cleaner&) = delete;
            Cleaner& operator=(const Cleaner&) = delete;

            Cleaner(WebBridge& parent) : _parent(parent) {
            }
            ~Cleaner() = default;

        public:
            void Dispatch() {
                _parent.Cleanup();
            }

        private:
            WebBridge& _parent;
        };

        using ObserverList = std::list<Observer>;
        using ObserverMap = std::map<string, ObserverList>;
        using MethodList = std::vector<string>;
        using VersionMap = std::map<uint8_t, MethodList>;
        using PendingMap = std::map<uint32_t, Request>;

    public:
        class Config : public Core::JSON::Container {
        public:
            Config()
                : Core::JSON::Container()
                , TimeOut(3000)
            {
                Add(_T("timeout"), &TimeOut);
            }
            ~Config() override = default;

        public:
            Core::JSON::String Bind;
            Core::JSON::DecUInt16 TimeOut;
        };

    public:
        WebBridge(const WebBridge&) = delete;
        WebBridge& operator=(const WebBridge&) = delete;

        #ifdef __WINDOWS__
        #pragma warning(disable: 4355)
        #endif
        WebBridge()
            : _adminLock()
            , _skipURL(0)
            , _service(nullptr)
            , _callsign()
            , _supportedVersions()
            , _observers()
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

        // IDispatcher
        // -------------------------------------------------------------------------------------------------------
        //! ==================================== CALLED ON THREADPOOL THREAD ======================================
        Core::ProxyType<Core::JSONRPC::Message> Invoke(const string& token, const uint32_t channelId, const Core::JSONRPC::Message& message) override;
        //! ==================================== CALLED ON THREADPOOL THREAD ======================================
        void Activate(PluginHost::IShell* service) override;
        //! ==================================== CALLED ON THREADPOOL THREAD ======================================
        void Deactivate() override;

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
        void Cleanup();
        bool InternalMessage(const Core::ProxyType<Core::JSONRPC::Message>& message);

        bool HasMethodSupport(const VersionMap::const_iterator& index, const string& method) const {
            bool result = false;

            if (index != _supportedVersions.cend()) {
                result = (std::find(index->second.cbegin(), index->second.cend(), method) != index->second.cend());
            }
            else {
                VersionMap::const_iterator index = _supportedVersions.begin();

                while ((result == false) && (index != _supportedVersions.end())) {
                    result = (std::find(index->second.cbegin(), index->second.cend(), method) != index->second.cend());
                    index++;
                }
            }

            return (result);
        }
        state Destination(const string& designator, string& handler) const
        {
            state result = state::STATE_INCORRECT_HANDLER;
            string callsign(Core::JSONRPC::Message::Callsign(designator));

            // If the message is routed through the controlelr, the callsign is empty by now!
            if ((callsign.empty()) || (callsign == _callsign)) {
                // Seems we are on the right handler..
                // now see if someone supports this version
                uint8_t version = Core::JSONRPC::Message::Version(designator);
                VersionMap::const_iterator methods = _supportedVersions.cend();

                // See if there was a version given..
                if (version != static_cast<uint8_t>(~0)) {
                    methods = _supportedVersions.find(version);
                    if (methods == _supportedVersions.cend()) {
                        result = state::STATE_INCORRECT_VERSION;
                    }
                }

                if (result == state::STATE_INCORRECT_HANDLER) {
                    string method = Core::JSONRPC::Message::Method(designator);

                    if (method == _T("register")) {
                        result = state::STATE_REGISTRATION;
                    }
                    else if (method == _T("unregister")) {
                        result = state::STATE_UNREGISTRATION;
                    }
                    else if (method == _T("exists")) {
                        result = HasMethodSupport(methods, method) ? state::STATE_EXISTS : state::STATE_NONE_EXISTING;
                    }
                    else if (HasMethodSupport(methods, method) == true) {
                        result = state::STATE_CUSTOM;
                        handler = method;
                    }
                    else {
                        result = state::STATE_UNKNOWN_METHOD;
                    }
                }
            }
            return (result);
        }
        void Subscribe(const uint32_t channelId, const string& eventName, const string& callsign, Core::JSONRPC::Message& response)
        {
            _adminLock.Lock();

            ObserverMap::iterator index = _observers.find(eventName);

            if (index == _observers.end()) {
                _observers[eventName].emplace_back(channelId, callsign);
                response.Result = _T("0");
            }
            else if (std::find(index->second.begin(), index->second.end(), Observer(channelId, callsign)) == index->second.end()) {
                index->second.emplace_back(channelId, callsign);
                response.Result = _T("0");
            }
            else {
                response.Error.SetError(Core::ERROR_DUPLICATE_KEY);
                response.Error.Text = _T("Duplicate registration. Only 1 remains!!!");
            }

            _adminLock.Unlock();
        }
        void Unsubscribe(const uint32_t channelId, const string& eventName, const string& callsign, Core::JSONRPC::Message& response)
        {
            _adminLock.Lock();

            ObserverMap::iterator index = _observers.find(eventName);

            if (index != _observers.end()) {
                ObserverList& clients = index->second;
                ObserverList::iterator loop = clients.begin();
                Observer key(channelId, callsign);

                while ((loop != clients.end()) && (*loop != key)) {
                    loop++;
                }

                if (loop != clients.end()) {
                    clients.erase(loop);
                    if (clients.empty() == true) {
                        _observers.erase(index);
                    }
                    response.Result = _T("0");
                }
            }

            if (response.Result.IsSet() == false) {
                response.Error.SetError(Core::ERROR_UNKNOWN_KEY);
                response.Error.Text = _T("Registration not found!!!");
            }

            _adminLock.Unlock();
        }

    private:
        Core::CriticalSection _adminLock;
        uint8_t _skipURL;
        PluginHost::IShell* _service;
        string _callsign;
        VersionMap _supportedVersions;
        ObserverMap _observers;
        PendingMap _pendingRequests;
        uint32_t _javascriptService;
        uint32_t _sequenceId;
        uint32_t _timeOut;
        Core::WorkerPool::JobType<Cleaner> _cleaner;
    };

} // namespace Plugin
} // namespace WPEFramework
