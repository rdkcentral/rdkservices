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

#include "WebKitBrowser.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 1
#define API_VERSION_NUMBER_PATCH 26

namespace WPEFramework {

namespace {

    static Plugin::Metadata<Plugin::WebKitBrowser> metadata(
        // Version (Major, Minor, Patch)
        API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
        // Preconditions
        {},
        // Terminations
        {},
        // Controls
        {}
    );
}

namespace Plugin {

    SERVICE_REGISTRATION(WebKitBrowser, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    /* virtual */ const string WebKitBrowser::Initialize(PluginHost::IShell* service)
    {
        string message;

        ASSERT(_service == nullptr);
        ASSERT(_browser == nullptr);
        ASSERT(_memory == nullptr);
        ASSERT(_application == nullptr);

        _connectionId = 0;
        _service = service;
        _skipURL = _service->WebPrefix().length();

        _persistentStoragePath = _service->PersistentPath();

        // Register the Connection::Notification stuff. The Remote process might die before we get a
        // change to "register" the sink for these events !!! So do it ahead of instantiation.
        _service->Register(&_notification);

        _browser = service->Root<Exchange::IWebBrowser>(_connectionId, 2000, _T("WebKitImplementation"));

        if (_browser != nullptr) {
            PluginHost::IStateControl* stateControl(_browser->QueryInterface<PluginHost::IStateControl>());

            // We see that sometimes the WPE implementation crashes before it reaches this point, than there is
            // no StateControl. Cope with this situation.
            if (stateControl == nullptr) {
                _browser->Release();
                _browser = nullptr;

            } else {
                _application = _browser->QueryInterface<Exchange::IApplication>();
                if (_application != nullptr) {
                    _browser->Register(&_notification);

                    const RPC::IRemoteConnection *connection = _service->RemoteConnection(_connectionId);
                    _memory = WPEFramework::WebKitBrowser::MemoryObserver(connection);
                    ASSERT(_memory != nullptr);
                    if (connection != nullptr) {
                        connection->Release();
                    }

                    if (stateControl->Configure(_service) != Core::ERROR_NONE) {
                        if (_memory != nullptr) {
                            _memory->Release();
                            _memory = nullptr;
                        }
                        _application->Release();
                        _application = nullptr;
                        _browser->Unregister(&_notification);
                        _browser->Release();
                        _browser = nullptr;
                    } else {
                        stateControl->Register(&_notification);
                    }
                    stateControl->Release();
                } else {
                    stateControl->Release();
                    _browser->Release();
                    _browser = nullptr;
                }
            }
        }

        if (_browser == nullptr) {
            message = _T("WebKitBrowser could not be instantiated.");
            _service->Unregister(&_notification);
            _service = nullptr;
        } else {
            RegisterAll();
            Exchange::JWebBrowser::Register(*this, _browser);

            _cookieJar = _browser->QueryInterface<Exchange::IBrowserCookieJar>();
            if (_cookieJar) {
                _cookieJar->Register(&_notification);
            }

            _browserScripting = _browser->QueryInterface<Exchange::IBrowserScripting>();
            if (_browserScripting) {
                Exchange::JBrowserScripting::Register(*this, _browserScripting);
            }
        }

        return message;
    }

    /* virtual */ void WebKitBrowser::Deinitialize(PluginHost::IShell* service VARIABLE_IS_NOT_USED)
    {
        ASSERT(_service == service);
        ASSERT(_browser != nullptr);
        ASSERT(_application != nullptr);
        ASSERT(_memory != nullptr);

        if (_browser == nullptr)
            return;

        // Make sure we get no longer get any notifications, we are deactivating..
        _service->Unregister(&_notification);
        _browser->Unregister(&_notification);
        _memory->Release();
        _application->Release();
        Exchange::JWebBrowser::Unregister(*this);
        if (_browserScripting) {
            Exchange::JBrowserScripting::Unregister(*this);
            _browserScripting->Release();
        }
        if (_cookieJar) {
            _cookieJar->Unregister(&_notification);
            _cookieJar->Release();
        }
        UnregisterAll();

        PluginHost::IStateControl* stateControl(_browser->QueryInterface<PluginHost::IStateControl>());

        // In case WPE rpcprocess crashed, there is no access to the statecontrol interface, check it !!
        if (stateControl != nullptr) {
            stateControl->Unregister(&_notification);
            stateControl->Release();
        }

        RPC::IRemoteConnection* connection(_service->RemoteConnection(_connectionId));

        // Stop processing of the browser:
        VARIABLE_IS_NOT_USED uint32_t result = _browser->Release();

        // It should have been the last reference we are releasing,
        // so it should end up in a DESCRUCTION_SUCCEEDED, if not we
        // are leaking...
        ASSERT(result == Core::ERROR_DESTRUCTION_SUCCEEDED);

        // If this was running in a (container) process...
        if (connection != nullptr) {
            // Lets trigger a cleanup sequence for
            // out-of-process code. Which will guard
            // that unwilling processes, get shot if
            // not stopped friendly :~)
            connection->Terminate();
            connection->Release();
        }

        _service = nullptr;
        _browser = nullptr;
        _application = nullptr;
        _memory = nullptr;
    }

    /* virtual */ string WebKitBrowser::Information() const
    {
        // No additional info to report.
        return { };
    }

    /* virtual */ void WebKitBrowser::Inbound(Web::Request& request)
    {
        if (request.Verb == Web::Request::HTTP_POST) {
            request.Body(_jsonBodyDataFactory.Element());
        }
    }

    /* virtual */ Core::ProxyType<Web::Response> WebKitBrowser::Process(const Web::Request& request)
    {
        ASSERT(_skipURL <= request.Path.length());

        TRACE(Trace::Information, (string(_T("Received request"))));

        Core::ProxyType<Web::Response> result(PluginHost::IFactories::Instance().Response());
        Core::TextSegmentIterator index(
            Core::TextFragment(request.Path, static_cast<uint32_t>(_skipURL), static_cast<uint32_t>(request.Path.length() - _skipURL)), false, '/');

        result->ErrorCode = Web::STATUS_BAD_REQUEST;
        result->Message = "Unknown error";

        if (_browser != nullptr) {

            PluginHost::IStateControl* stateControl(_browser->QueryInterface<PluginHost::IStateControl>());

            ASSERT(stateControl != nullptr);
            ASSERT(_application != nullptr);

            if (request.Verb == Web::Request::HTTP_GET) {
                bool visible = false;
                static_cast<const WPEFramework::Exchange::IApplication*>(_application)->Visible(visible);
                PluginHost::IStateControl::state currentState = stateControl->State();
                Core::ProxyType<Web::JSONBodyType<WebKitBrowser::Data>> body(_jsonBodyDataFactory.Element());
                string url;
                static_cast<const WPEFramework::Exchange::IWebBrowser*>(_browser)->URL(url);
                body->URL = url;
                uint8_t fps = 0;
                _browser->FPS(fps);
                body->FPS = fps;
                body->Suspended = (currentState == PluginHost::IStateControl::SUSPENDED);
                body->Hidden = !visible;
                result->ErrorCode = Web::STATUS_OK;
                result->Message = "OK";
                result->Body<Web::JSONBodyType<WebKitBrowser::Data>>(body);
            } else if ((request.Verb == Web::Request::HTTP_POST) && (index.Next() == true) && (index.Next() == true)) {
                result->ErrorCode = Web::STATUS_OK;
                result->Message = "OK";

                // We might be receiving a plugin download request.
                if (index.Remainder() == _T("Suspend")) {
                    stateControl->Request(PluginHost::IStateControl::SUSPEND);
                } else if (index.Remainder() == _T("Resume")) {
                    stateControl->Request(PluginHost::IStateControl::RESUME);
                } else if (index.Remainder() == _T("Hide")) {
                    _browser->Visibility(Exchange::IWebBrowser::VisibilityType::HIDDEN);
                } else if (index.Remainder() == _T("Show")) {
                    _browser->Visibility(Exchange::IWebBrowser::VisibilityType::VISIBLE);
                } else if ((index.Remainder() == _T("URL")) && (request.HasBody() == true) && (request.Body<const Data>()->URL.Value().empty() == false)) {
                    const string url = request.Body<const Data>()->URL.Value();
                    _browser->URL(url);
                } else if ((index.Remainder() == _T("Delete")) && (request.HasBody() == true) && (request.Body<const Data>()->Path.Value().empty() == false)) {
                    if (DeleteDir(request.Body<const Data>()->Path.Value()) != Core::ERROR_NONE) {
                        result->ErrorCode = Web::STATUS_BAD_REQUEST;
                        result->Message = "Unknown error";
                    }
                } else {
                    result->ErrorCode = Web::STATUS_BAD_REQUEST;
                    result->Message = "Unknown error";
                }
            }
            stateControl->Release();
        }

        return result;
    }

    uint32_t WebKitBrowser::DeleteDir(const string& path)
    {
        uint32_t result = Core::ERROR_NONE;

        if (path.empty() == false) {
            string fullPath = _persistentStoragePath + path;
            Core::Directory dir(fullPath.c_str());
#if defined(THUNDER_VERSION) && THUNDER_VERSION >= 4
            bool success = dir.Destroy();
#else
            bool success = dir.Destroy(true);
#endif
            if (!success) {
                SYSLOG(Logging::Error, (_T("Failed to delete %s\n"), fullPath.c_str()));
                result = Core::ERROR_GENERAL;
            }
        }

        return result;
    }

    void WebKitBrowser::LoadFinished(const string& URL, int32_t code)
    {
        string message(string("{ \"url\": \"") + URL + string("\", \"loaded\":true, \"httpstatus\":") + Core::NumberType<int32_t>(code).Text() + string(" }"));
        TRACE(Trace::Information, (_T("LoadFinished: %s"), message.c_str()));
        _service->Notify(message);
        Exchange::JWebBrowser::Event::LoadFinished(*this, URL, code);
        URLChange(URL, true);
    }

    void WebKitBrowser::LoadFailed(const string& URL)
    {
        string message(string("{ \"url\": \"") + URL + string("\" }"));
        TRACE(Trace::Information, (_T("LoadFailed: %s"), message.c_str()));
        _service->Notify(message);
        Exchange::JWebBrowser::Event::LoadFailed(*this, URL);
    }

    void WebKitBrowser::URLChange(const string& URL, bool loaded)
    {
        string message(string("{ \"url\": \"") + URL + string("\", \"loaded\": ") + (loaded ? string("true") : string("false")) + string(" }"));
        TRACE(Trace::Information, (_T("URLChanged: %s"), message.c_str()));
        _lastURL.assign(URL);
        _service->Notify(message);
        Exchange::JWebBrowser::Event::URLChange(*this, URL, loaded);
    }

    void WebKitBrowser::VisibilityChange(const bool hidden)
    {
        TRACE(Trace::Information, (_T("VisibilityChange: { \"hidden\": \"%s\"}"), (hidden ? "true" : "false")));
        string message(string("{ \"hidden\": ") + (hidden ? _T("true") : _T("false")) + string("}"));
        _service->Notify(message);
        Exchange::JWebBrowser::Event::VisibilityChange(*this, hidden);
    }

    void WebKitBrowser::PageClosure()
    {
        TRACE(Trace::Information, (_T("Closure: \"true\"")));
        _service->Notify(_T("{\"Closure\": true }"));
        Exchange::JWebBrowser::Event::PageClosure(*this);
    }

    void WebKitBrowser::BridgeQuery(const string& message)
    {
        event_bridgequery(message);
    }

    void WebKitBrowser::CookieJarChanged()
    {
        Notify(_T("cookiejarchanged"));
    }

    void WebKitBrowser::StateChange(const PluginHost::IStateControl::state state)
    {
        TRACE(Trace::Information, (_T("StateChange: { \"State\": %d }"), state));
        string message(string("{ \"suspended\": ") + (state == PluginHost::IStateControl::SUSPENDED ? _T("true") : _T("false")) + string(" }"));
        _service->Notify(message);
        event_statechange(state == PluginHost::IStateControl::SUSPENDED);
    }

    void WebKitBrowser::Deactivated(RPC::IRemoteConnection* connection)
    {
        if (connection->Id() == _connectionId) {

            TRACE(Trace::Information, (_T("WebKitBrowser::Deactivated: { \"URL\": %.*s }"), 80, _lastURL.c_str()));

            ASSERT(_service != nullptr);

            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
        }
    }
}  // namespace Plugin

namespace WebKitBrowser {

    // TODO: maybe nice to expose this in the config.json
    static const TCHAR* mandatoryProcesses[] = {
        _T("WPENetworkProcess"),
        _T("WPEWebProcess")
    };

    static constexpr uint16_t RequiredChildren = (sizeof(mandatoryProcesses) / sizeof(mandatoryProcesses[0]));
    using SteadyClock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<SteadyClock>;

    class MemoryObserverImpl : public Exchange::IMemory {
    private:
        MemoryObserverImpl();
        MemoryObserverImpl(const MemoryObserverImpl&);
        MemoryObserverImpl& operator=(const MemoryObserverImpl&);

        enum { TYPICAL_STARTUP_TIME = 10 }; /* in Seconds */
    public:
        MemoryObserverImpl(const RPC::IRemoteConnection* connection)
            : _main(connection == nullptr ? Core::ProcessInfo().Id() : connection->RemoteId())
            , _children(_main.Id())
            , _startTime(connection == nullptr ? (TimePoint::min()) : (SteadyClock::now() + std::chrono::seconds(TYPICAL_STARTUP_TIME)))
        { // IsOperation true till calculated time (microseconds)
        }
        ~MemoryObserverImpl()
        {
        }

    public:
        uint64_t Resident() const override
        {
            uint32_t result(0);

            if (_startTime != TimePoint::min()) {
                if (_children.Count() < RequiredChildren) {
                    _children = Core::ProcessInfo::Iterator(_main.Id());
                }

                result = _main.Resident();

                _children.Reset();

                while (_children.Next() == true) {
                    result += _children.Current().Resident();
                }
            }

            return (result);
        }
        uint64_t Allocated() const override
        {
            uint32_t result(0);

            if (_startTime != TimePoint::min()) {
                if (_children.Count() < RequiredChildren) {
                    _children = Core::ProcessInfo::Iterator(_main.Id());
                }

                result = _main.Allocated();

                _children.Reset();

                while (_children.Next() == true) {
                    result += _children.Current().Allocated();
                }
            }

            return (result);
        }
        uint64_t Shared() const override
        {
            uint32_t result(0);

            if (_startTime != TimePoint::min()) {
                if (_children.Count() < RequiredChildren) {
                    _children = Core::ProcessInfo::Iterator(_main.Id());
                }

                result = _main.Shared();

                _children.Reset();

                while (_children.Next() == true) {
                    result += _children.Current().Shared();
                }
            }

            return (result);
        }
        uint8_t Processes() const override
        {
            // Refresh the children list !!!
            _children = Core::ProcessInfo::Iterator(_main.Id());
            return ((_startTime == TimePoint::min()) || (_main.IsActive() == true) ? 1 : 0) + _children.Count();
        }
#if defined(THUNDER_VERSION) && THUNDER_VERSION >= 4
        bool IsOperational() const override
#else
        const bool IsOperational() const override
#endif
        {
            uint32_t requiredProcesses = 0;

            if (_startTime != TimePoint::min()) {

                //!< We can monitor a max of 32 processes, every mandatory process represents a bit in the requiredProcesses.
                // In the end we check if all bits are 0, what means all mandatory processes are still running.
                requiredProcesses = (0xFFFFFFFF >> (32 - RequiredChildren));

                if (_children.Count() < RequiredChildren) {
                    // Refresh the children list !!!
                    _children = Core::ProcessInfo::Iterator(_main.Id());
                }
                //!< If there are less children than in the the mandatoryProcesses struct, we are done and return false.
                if (_children.Count() >= RequiredChildren) {

                    _children.Reset();

                    //!< loop over all child processes as long as we are operational.
                    while ((requiredProcesses != 0) && (true == _children.Next())) {

                        uint8_t count(0);
                        string name(_children.Current().Name());

                        while ((count < RequiredChildren) && (name != mandatoryProcesses[count])) {
                            ++count;
                        }

                        //<! this is a mandatory process and if its still active reset its bit in requiredProcesses.
                        //   If not we are not completely operational.
                        if ((count < RequiredChildren) && (_children.Current().IsActive() == true)) {
                            requiredProcesses &= (~(1 << count));
                        }
                    }
                }
            }

            return (((requiredProcesses == 0) || (true == IsStarting())) && (true == _main.IsActive()));
        }

        BEGIN_INTERFACE_MAP(MemoryObserverImpl)
        INTERFACE_ENTRY(Exchange::IMemory)
        END_INTERFACE_MAP

    private:
        inline bool IsStarting() const
        {
            return (_startTime == TimePoint::min()) || (SteadyClock::now() < _startTime);
        }

    private:
        Core::ProcessInfo _main;
        mutable Core::ProcessInfo::Iterator _children;
        TimePoint _startTime; // !< Reference for monitor
    };

    Exchange::IMemory* MemoryObserver(const RPC::IRemoteConnection* connection)
    {
        Exchange::IMemory* result = Core::Service<MemoryObserverImpl>::Create<Exchange::IMemory>(connection);
        return (result);
    }
} // namespace WebKitBrowser




}  // WPEFramework
