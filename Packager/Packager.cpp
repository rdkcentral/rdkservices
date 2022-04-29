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
 
#include "Packager.h"

namespace WPEFramework {
namespace Plugin {
namespace {

    constexpr int kMaxValueLength = 256;

}

    SERVICE_REGISTRATION(Packager, 1, 0);

    const string Packager::Initialize(PluginHost::IShell* service) {
        ASSERT (_service == nullptr);
        ASSERT (service != nullptr);

        _service = service;
        _skipURL = static_cast<uint8_t>(service->WebPrefix().length());
        _service->Register(&_notification);

         string result;
        _implementation = _service->Root<Exchange::IPackager>(_connectionId, 2000, _T("PackagerImplementation"));
        if (_implementation == nullptr) {
            result = _T("Couldn't create package instance");
            _service->Unregister(&_notification);
        } else if (_implementation->Configure(_service) != Core::ERROR_NONE) {
            result = _T("Couldn't initialize package instance");
            _service->Unregister(&_notification);
        }

        return (result);
    }

    void Packager::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);

        _service->Unregister(&_notification);

        if (_implementation->Release() != Core::ERROR_DESTRUCTION_SUCCEEDED) {

            ASSERT(_connectionId != 0);

            RPC::IRemoteConnection* connection(_service->RemoteConnection(_connectionId));

            // The process can disappear in the meantime...
            if (connection != nullptr) {

                // But if it did not dissapear in the meantime, forcefully terminate it. Shoot to kill :-)
                connection->Terminate();
                connection->Release();
            }
        }

        _service = nullptr;
        _implementation = nullptr;
    }

    string Packager::Information() const
    {
        return (string());
    }

    void Packager::Inbound(Web::Request& request)
    {
    }

    Core::ProxyType<Web::Response> Packager::Process(const Web::Request& request)
    {
        ASSERT(_skipURL <= request.Path.length());

        Core::ProxyType<Web::Response> result(PluginHost::IFactories::Instance().Response());
        Core::TextSegmentIterator index(Core::TextFragment(request.Path, _skipURL, request.Path.length() - _skipURL), false, '/');

        // Always skip the first one, it is an empty part because we start with a '/' if there are more parameters.
        index.Next();

        result->ErrorCode = Web::STATUS_BAD_REQUEST;
        result->Message = _T("Invalid request to packager plugin.");

        if (index.Next() && (request.Verb == Web::Request::HTTP_POST || request.Verb == Web::Request::HTTP_PUT)) {
            uint32_t status = Core::ERROR_UNAVAILABLE;
            if (index.Current().Text() == "Install") {
                std::array<char, kMaxValueLength> package {0};
                std::array<char, kMaxValueLength> version {0};
                std::array<char, kMaxValueLength> arch {0};
                Core::URL::KeyValue options(request.Query.Value());
                if (options.Exists(_T("Package"), true) == true) {
                    const string name (options[_T("Package")].Text());
                    Core::URL::Decode (name.c_str(), name.length(), package.data(), package.size());
                }
                if (options.Exists(_T("Architecture"), true) == true) {
                                    const string name (options[_T("Architecture")].Text());
                                    Core::URL::Decode (name.c_str(), name.length(), arch.data(), arch.size());
                }
                if (options.Exists(_T("Version"), true) == true) {
                    const string name (options[_T("Version")].Text());
                    Core::URL::Decode (name.c_str(), name.length(), version.data(), version.size());
                }

                status = _implementation->Install(package.data(), version.data(), arch.data());
            } else if (index.Current().Text() == "SynchronizeRepository") {
                status = _implementation->SynchronizeRepository();
            }

            if (status == Core::ERROR_NONE) {
                result->ErrorCode = Web::STATUS_OK;
                result->Message = _T("OK");
            } else if (status == Core::ERROR_INPROGRESS) {
                result->Message = _T("Some operation already in progress. Only one at a time is allowed");
            }
        }

        return(result);
    }

    void Packager::Deactivated(RPC::IRemoteConnection* connection)
    {
        if (connection->Id() == _connectionId) {
            ASSERT(_service != nullptr);
            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service,
                PluginHost::IShell::DEACTIVATED,
                PluginHost::IShell::FAILURE));
        }
    }
}  // namespace Plugin
}  // namespace WPEFramework
