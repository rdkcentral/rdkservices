/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 Metrological
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

#include "FireboltPrivacy.h"
#include <interfaces/IConfiguration.h>

namespace WPEFramework {

namespace Plugin
{

    namespace {

        static Metadata<FireboltPrivacy> metadata(
            // Version 
            1, 0, 0,
            // Preconditions
            {},     
            // Terminations
            {},     
            // Controls
            {}
        );      
    }

    static Core::ProxyPoolType<Web::Response> responseFactory(4);
    static Core::ProxyPoolType<Web::JSONBodyType<Core::JSON::ArrayType<Core::JSON::String>>> jsonStringListFactory(1);

    FireboltPrivacy::FireboltPrivacy()
        : _connectionId(0)
        , _service(nullptr)
        , _fireboltPrivacy()
        , _connectionNotification(this)
        , _fireboltPrivacyNotification(this)
    {
    }

    /* virtual */ const string FireboltPrivacy::Initialize(PluginHost::IShell * service)
    {
        string message;

        ASSERT(service != nullptr);
        ASSERT(_service == nullptr);
        ASSERT(_connectionId == 0);
        ASSERT(_fireboltPrivacy == nullptr);
        _service = service;
        _service->AddRef();

        // Register the Process::Notification stuff. The Remote process might die before we get a
        // change to "register" the sink for these events !!! So do it ahead of instantiation.
        _service->Register(&_connectionNotification);

        _fireboltPrivacy = service->Root<Exchange::IFireboltPrivacy>(_connectionId, 2000, _T("FireboltPrivacyImplementation"));
        if (_fireboltPrivacy != nullptr) {
            _fireboltPrivacy->Register(&_fireboltPrivacyNotification);
            Exchange::JFireboltPrivacy::Register(*this, _fireboltPrivacy);

            Exchange::IConfiguration* configFireboltPrivacy = _fireboltPrivacy->QueryInterface<Exchange::IConfiguration>();
            if (configFireboltPrivacy != nullptr) {
                configFireboltPrivacy->Configure(service);
                configFireboltPrivacy->Release();
            }
            //RegisterAll();
        }
        else {
            message = _T("FireboltPrivacy could not be instantiated.");
        }

        // On success return empty, to indicate there is no error text.
        return (message);
    }

    /* virtual */ void FireboltPrivacy::Deinitialize(PluginHost::IShell* service VARIABLE_IS_NOT_USED)
    {
        if (_service != nullptr) {
	    ASSERT(_service == service);

            _service->Unregister(&_connectionNotification);

            if (_fireboltPrivacy != nullptr) {
                Exchange::JFireboltPrivacy::Unregister(*this);
                _fireboltPrivacy->Unregister(&_fireboltPrivacyNotification);

                RPC::IRemoteConnection* connection(_service->RemoteConnection(_connectionId));
                VARIABLE_IS_NOT_USED uint32_t result = _fireboltPrivacy->Release();
                _fireboltPrivacy = nullptr;
                ASSERT(result == Core::ERROR_DESTRUCTION_SUCCEEDED);

                // The connection can disappear in the meantime...
                if (connection != nullptr) {
                    // But if it did not dissapear in the meantime, forcefully terminate it. Shoot to kill :-)
                    connection->Terminate();
                    connection->Release();
                }
            }

            _service->Release();
            _service = nullptr;
            _connectionId = 0;
        }
    }

    /* virtual */ string FireboltPrivacy::Information() const
    {
        // No additional info to report.
        return (string());
    }

    /* virtual */ void FireboltPrivacy::Inbound(Web::Request& /*request*/)
    {
        SYSLOG(Logging::Notification, (_T("Inside Inbound ")));
        #if 0
        Core::TextSegmentIterator index(Core::TextFragment(request.Path, _skipURL,
                                        static_cast<uint32_t>(request.Path.length() - _skipURL)),
                                        false, '/');

        // If it is a put, we expect a list of commands, receive them !!!!
        if (request.Verb == Web::Request::HTTP_POST) {
            index.Next();

            if ((index.IsValid() == true) && (index.Next() == true)) {
                if (index.Current().Text() == _T("DNS")) {
                    request.Body(jsonStringListFactory.Element());
                } else if (index.Current().Text() == _T("Network")) {
                    request.Body(jsonNetworksFactory.Element());
                }
            }
        }
        #endif
    }

    /* virtual */ Core::ProxyType<Web::Response>
    FireboltPrivacy::Process(const Web::Request& /* request */)
    {

        Core::ProxyType<Web::Response> result(PluginHost::IFactories::Instance().Response());
#if 0
        Core::TextSegmentIterator index(Core::TextFragment(request.Path, _skipURL, static_cast<uint16_t>(request.Path.length()) - _skipURL), false, '/');

        // By default, we assume everything works..
        result->ErrorCode = Web::STATUS_BAD_REQUEST;
        result->Message = _T("Unsupported request for the [FireboltPrivacy] service.");

        // By default, we skip the first slash
        index.Next();

        if (request.Verb == Web::Request::HTTP_GET) {
            result = GetMethod(index);
        } else if (request.Verb == Web::Request::HTTP_PUT) {
            result = PutMethod(index);
        } else if (request.Verb == Web::Request::HTTP_POST) {
            result = PostMethod(index, request);
        }
#endif
        return result;
    }

    Core::ProxyType<Web::Response> FireboltPrivacy::GetMethod(Core::TextSegmentIterator& /* index */) const
    {
        Core::ProxyType<Web::Response> result(PluginHost::IFactories::Instance().Response());
        result->ErrorCode = Web::STATUS_BAD_REQUEST;
        result->Message = _T("Unsupported GET request.");

        return result;
    }

    Core::ProxyType<Web::Response> FireboltPrivacy::PutMethod(Core::TextSegmentIterator& /* index */)
    {
        Core::ProxyType<Web::Response> result(PluginHost::IFactories::Instance().Response());
        result->ErrorCode = Web::STATUS_BAD_REQUEST;
        result->Message = _T("Unsupported PUT request.");

        return result;
    }

    Core::ProxyType<Web::Response> FireboltPrivacy::PostMethod(Core::TextSegmentIterator& /* index */, const Web::Request& /* request */)
    {
        Core::ProxyType<Web::Response> result(PluginHost::IFactories::Instance().Response());
        result->ErrorCode = Web::STATUS_BAD_REQUEST;
        result->Message = _T("Unsupported POST request.");
        return result;
    }

    void FireboltPrivacy::Deactivated(RPC::IRemoteConnection* connection)
    {
        // This can potentially be called on a socket thread, so the deactivation (wich in turn kills this object) must be done
        // on a seperate thread. Also make sure this call-stack can be unwound before we are totally destructed.
        if (_connectionId == connection->Id()) {
            ASSERT(_service != nullptr);
            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
        }
    }
} // namespace Plugin
} // namespace WPEFramework

