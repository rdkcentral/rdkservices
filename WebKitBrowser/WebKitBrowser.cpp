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

#undef TRACE
#define TRACE SYSLOG

namespace WPEFramework {

namespace Plugin {

    SERVICE_REGISTRATION(WebKitBrowser, 1, 0);

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

        // If DiskCacheDir value exist in plugins/WebKitBrowser.json DiskCacheDir
        // it should replace the original persistentStoragePath with its path data
        _persistentStoragePath = _service->PersistentPath();
        std::string configLine = _service->ConfigLine();
        if (!configLine.empty()) {
            JsonObject serviceConfig = JsonObject(configLine.c_str());
            if (serviceConfig.HasLabel("diskcachedir")) {
                _persistentStoragePath = serviceConfig["diskcachedir"].String();
            } else {
                TRACE(Trace::Error, (string(_T("Failed to overwrite persistentPath"))));
            }
        }

        // Register the Connection::Notification stuff. The Remote process might die before we get a
        // change to "register" the sink for these events !!! So do it ahead of instantiation.
        _service->Register(&_notification);

        _browser = service->Root<Exchange::IWebBrowser>(_connectionId, 20000, _T("WebKitImplementation"));

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

            _browserResources = _browser->QueryInterface<Exchange::IBrowserResources>();
            if (!_browserResources) {
                TRACE(Trace::Error, (_T("Failed to get IBrowserResources interface")));
            } else {
                Exchange::JBrowserResources::Register(*this, _browserResources);
            }

            _browserSecurity = _browser->QueryInterface<Exchange::IBrowserSecurity>();
            if (!_browserSecurity) {
                TRACE(Trace::Error, (_T("Failed to get IBrowserSecurity interface")));
            } else {
                Exchange::JBrowserSecurity::Register(*this, _browserSecurity);
            }
        }

        return message;
    }

    /* virtual */ void WebKitBrowser::Deinitialize(PluginHost::IShell* service)
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
        UnregisterAll();

        if (_browserSecurity) {
            Exchange::JBrowserSecurity::Unregister(*this);
            _browserSecurity->Release();
            _browserSecurity = nullptr;
        }

        if(_browserResources) {
            Exchange::JBrowserResources::Unregister(*this);
            _browserResources->Release();
            _browserResources = nullptr;
        }

        PluginHost::IStateControl* stateControl(_browser->QueryInterface<PluginHost::IStateControl>());

        // In case WPE rpcprocess crashed, there is no access to the statecontrol interface, check it !!
        if (stateControl != nullptr) {
            stateControl->Unregister(&_notification);
            stateControl->Release();
            stateControl = nullptr;
        }

        // Stop processing of the browser:
        if (_browser->Release() != Core::ERROR_DESTRUCTION_SUCCEEDED) {
            ASSERT(_connectionId != 0);

            TRACE(Trace::Information, (_T("Browser Plugin is not properly destructed. %d"), _connectionId));

            RPC::IRemoteConnection* connection(_service->RemoteConnection(_connectionId));
            // The process can disappear in the meantime...
            if (connection != nullptr) {
                // But if it did not dissapear in the meantime, forcefully terminate it. Shoot to kill :-)
                connection->Terminate();
                connection->Release();
            }
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
            Core::TextFragment(request.Path, _skipURL, request.Path.length() - _skipURL), false, '/');

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
            if (!dir.Destroy(true)) {
                TRACE(Trace::Error, (_T("Failed to delete %s\n"), fullPath.c_str()));
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
        TRACE(Trace::Information, (_T("BridgeQuery: %s"), message.c_str()));
        event_bridgequery(message);
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

            ASSERT(_service != nullptr);

            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
        }
    }
}  // namespace Plugin

}  // WPEFramework
