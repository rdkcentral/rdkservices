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

namespace WPEFramework {

namespace WebKitBrowser {

    // An implementation file needs to implement this method to return an operational browser, wherever that would be :-)
    extern Exchange::IMemory* MemoryObserver(const RPC::IRemoteConnection* connection);
}

namespace Plugin {

    SERVICE_REGISTRATION(WebKitBrowser, 1, 0);

    /* virtual */ const string WebKitBrowser::Initialize(PluginHost::IShell* service)
    {
        Config config;
        string message;

        ASSERT(_service == nullptr);
        ASSERT(_browser == nullptr);
        ASSERT(_memory == nullptr);

        _connectionId = 0;
        _service = service;
        _skipURL = _service->WebPrefix().length();

        config.FromString(_service->ConfigLine());

        // Register the Connection::Notification stuff. The Remote process might die before we get a
        // change to "register" the sink for these events !!! So do it ahead of instantiation.
        _service->Register(&_notification);

        _browser = service->Root<Exchange::IBrowser>(_connectionId, 2000, _T("WebKitImplementation"));

        if ((_browser != nullptr) && (_service != nullptr)) {
            PluginHost::IStateControl* stateControl(_browser->QueryInterface<PluginHost::IStateControl>());

            // We see that sometimes the WPE implementation crashes before it reaches this point, than there is
            // no StateControl. Cope with this situation.
            if (stateControl == nullptr) {
                _browser->Release();
                _browser = nullptr;

                stateControl->Release();

            } else {
                _browser->Register(&_notification);

                const RPC::IRemoteConnection *connection = _service->RemoteConnection(_connectionId);
                ASSERT(connection != nullptr);

                if (connection != nullptr) {
                    _memory = WPEFramework::WebKitBrowser::MemoryObserver(connection);

                    ASSERT(_memory != nullptr);

                    connection->Release();
                }

                stateControl->Configure(_service);
                stateControl->Register(&_notification);
                stateControl->Release();
            }
        }

        if (_browser == nullptr) {
            message = _T("WebKitBrowser could not be instantiated.");
            _service->Unregister(&_notification);
            _service = nullptr;
        }

        return message;
    }

    /* virtual */ void WebKitBrowser::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);
        ASSERT(_browser != nullptr);
        ASSERT(_memory != nullptr);

        // Make sure we get no longer get any notifications, we are deactivating..
        _service->Unregister(&_notification);
        _browser->Unregister(&_notification);
        _memory->Release();

        PluginHost::IStateControl* stateControl(_browser->QueryInterface<PluginHost::IStateControl>());

        // In case WPE rpcprocess crashed, there is no access to the statecontrol interface, check it !!
        if (stateControl != nullptr) {
            stateControl->Unregister(&_notification);
            stateControl->Release();
        }

        // Stop processing of the browser:
        _browser->Release();
        // FIXME: Destruction is not always properly reported, which leaves hanging processes (Bartjes Law)
        if (_connectionId != 0) {

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
        _memory = nullptr;
    }

    /* virtual */ string WebKitBrowser::Information() const
    {
        // No additional info to report.
        return (nullptr);
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

            if (request.Verb == Web::Request::HTTP_GET) {
                PluginHost::IStateControl::state currentState = stateControl->State();
                Core::ProxyType<Web::JSONBodyType<WebKitBrowser::Data>> body(_jsonBodyDataFactory.Element());
                body->URL = _browser->GetURL();
                body->FPS = _browser->GetFPS();
                body->Suspended = (currentState == PluginHost::IStateControl::SUSPENDED);
                body->Hidden = _hidden;
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
                    _browser->Hide(true);
                } else if (index.Remainder() == _T("Show")) {
                    _browser->Hide(false);
                } else if ((index.Remainder() == _T("URL")) && (request.HasBody() == true) && (request.Body<const Data>()->URL.Value().empty() == false)) {
                    _browser->SetURL(request.Body<const Data>()->URL.Value());
                } else {
                    result->ErrorCode = Web::STATUS_BAD_REQUEST;
                    result->Message = "Unknown error";
                }
            }
            stateControl->Release();
        }

        return result;
    }
    void WebKitBrowser::LoadFinished(const string& URL)
    {
        string message(string("{ \"url\": \"") + URL + string("\", \"loaded\":true }"));
        TRACE(Trace::Information, (_T("LoadFinished: %s"), message.c_str()));
        _service->Notify(message);

        event_urlchange(URL, true);
    }
    void WebKitBrowser::URLChanged(const string& URL)
    {
        string message(string("{ \"url\": \"") + URL + string("\" }"));
        TRACE(Trace::Information, (_T("URLChanged: %s"), message.c_str()));
        _service->Notify(message);

        event_urlchange(URL, false);
    }
    void WebKitBrowser::Hidden(const bool hidden)
    {
        TRACE(Trace::Information, (_T("Hidden: %s }"), (hidden ? "true" : "false")));
        string message(string("{ \"hidden\": ") + (hidden ? _T("true") : _T("false")) + string("}"));
        _hidden = hidden;
        _service->Notify(message);

        event_visibilitychange(hidden);
    }
    void WebKitBrowser::Closure()
    {
        TRACE(Trace::Information, (_T("Closure: \"true\"")));
        _service->Notify(_T("{\"Closure\": true }"));

        event_pageclosure();
    }
    void WebKitBrowser::StateChange(const PluginHost::IStateControl::state state)
    {
        TRACE(Trace::Information, (_T("StateChange: { \"State\": %d }"), state));

        string message(
            string("{ \"suspended\": ") + (state == PluginHost::IStateControl::SUSPENDED ? _T("true") : _T("false")) + string(" }"));

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
}
} // namespace Plugin
