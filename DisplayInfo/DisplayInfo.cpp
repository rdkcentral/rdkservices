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

#include "DisplayInfo.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 5

namespace WPEFramework {
namespace {
    static Plugin::Metadata<Plugin::DisplayInfo> metadata(
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

    SERVICE_REGISTRATION(DisplayInfo, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    static Core::ProxyPoolType<Web::Response> responseFactory(4);
    static Core::ProxyPoolType<Web::JSONBodyType<JsonData::DisplayInfo::DisplayinfoData>> jsonResponseFactory(4);

    /* virtual */ const string DisplayInfo::Initialize(PluginHost::IShell* service)
    {
        string message;

        ASSERT(service != nullptr);
        ASSERT(_service == nullptr);
        ASSERT(_connectionProperties == nullptr);
        ASSERT(_connectionId == 0);
        ASSERT(_graphicsProperties == nullptr);
        ASSERT(_hdrProperties == nullptr);

        _service = service;
        _service->AddRef();
        _service->Register(&_notification);
        _skipURL = static_cast<uint8_t>(service->WebPrefix().length());

        _connectionProperties = service->Root<Exchange::IConnectionProperties>(_connectionId, 2000, _T("DisplayInfoImplementation"));
        if (_connectionProperties != nullptr) {
            _connectionProperties->Register(&_notification);
            Exchange::JConnectionProperties::Register(*this, _connectionProperties);

            Exchange::IConfiguration* configConnection = _connectionProperties->QueryInterface<Exchange::IConfiguration>();
            if (configConnection != nullptr) {
                configConnection->Configure(service);
                configConnection->Release();
            }

            _graphicsProperties = _connectionProperties->QueryInterface<Exchange::IGraphicsProperties>();
            if (_graphicsProperties == nullptr) {
                message = _T("DisplayInfo could not be instantiated. Could not acquire GraphicsProperties interface");
            } else {
                Exchange::JGraphicsProperties::Register(*this, _graphicsProperties);
                _hdrProperties = _connectionProperties->QueryInterface<Exchange::IHDRProperties>();
                if (_hdrProperties == nullptr) {
                    message = _T("DisplayInfo could not be instantiated. Could not acquire HDRProperties interface");
                } else {
                    Exchange::JHDRProperties::Register(*this, _hdrProperties);

                    // The code execution should proceed regardless of the _displayProperties
                    // value, as it is not a essential.
                    // The relevant JSONRPC endpoints will return ERROR_UNAVAILABLE,
                    // if it hasn't been initialized.
                    _displayProperties = _connectionProperties->QueryInterface<Exchange::IDisplayProperties>();
                    if (_displayProperties == nullptr) {
                        SYSLOG(Logging::Startup, (_T("Display Properties service is unavailable.")));
                    } else {
                        Exchange::JDisplayProperties::Register(*this, _displayProperties);
                    }
                }
            }
        } else {
            message = _T("DisplayInfo could not be instantiated. Could not acquire ConnectionProperties interface");
        }

#ifndef USE_THUNDER_R4
        if (message.length() != 0) {
            Deinitialize(service);
        }
#endif

        return message;
    }

    void DisplayInfo::Deinitialize(PluginHost::IShell* service) /* override */
    {
        if (_service != nullptr) {
            ASSERT(service == _service);

            _service->Unregister(&_notification);

            if (_connectionProperties != nullptr) {
                _connectionProperties->Unregister(&_notification);
                Exchange::JConnectionProperties::Unregister(*this);

                if (_hdrProperties != nullptr) {
                    Exchange::JHDRProperties::Unregister(*this);
                    _hdrProperties->Release();
                    _hdrProperties = nullptr;
                }

                if (_graphicsProperties != nullptr) {
                    Exchange::JGraphicsProperties::Unregister(*this);
                    _graphicsProperties->Release();
                    _graphicsProperties = nullptr;
                }

                if (_displayProperties != nullptr) {
                    _displayProperties->Release();
                    Exchange::JDisplayProperties::Unregister(*this);
                    _displayProperties = nullptr;
                }

                // Stop processing:
                RPC::IRemoteConnection* connection = service->RemoteConnection(_connectionId);
                VARIABLE_IS_NOT_USED uint32_t result = _connectionProperties->Release();
                _connectionProperties = nullptr;

                // It should have been the last reference we are releasing, 
                // so it should endup in a DESTRUCTION_SUCCEEDED, if not we
                // are leaking...
                ASSERT(result == Core::ERROR_DESTRUCTION_SUCCEEDED);

                // If this was running in a (container) process...
                if (connection != nullptr) {
                    // Lets trigger the cleanup sequence for
                    // out-of-process code. Which will guard
                    // that unwilling processes, get shot if
                    // not stopped friendly :-)
                    connection->Terminate();
                    connection->Release();
                }
            }
            _connectionId = 0;
            _service->Release();
            _service = nullptr;
        }
    }

    string DisplayInfo::Information() const /* override */
    {
        // No additional info to report.
        return (string());
    }

    /* virtual */ void DisplayInfo::Inbound(Web::Request& /* request */)
    {
    }

    /* virtual */ Core::ProxyType<Web::Response> DisplayInfo::Process(const Web::Request& request)
    {
        ASSERT(_skipURL <= request.Path.length());

        Core::ProxyType<Web::Response> result(PluginHost::IFactories::Instance().Response());

        // By default, we assume everything works..
        result->ErrorCode = Web::STATUS_OK;
        result->Message = "OK";

        // <GET> - currently, only the GET command is supported, returning system info
        if (request.Verb == Web::Request::HTTP_GET) {

            Core::ProxyType<Web::JSONBodyType<JsonData::DisplayInfo::DisplayinfoData>> response(jsonResponseFactory.Element());

            Core::TextSegmentIterator index(Core::TextFragment(request.Path, _skipURL, static_cast<uint32_t>(request.Path.length()) - _skipURL), false, '/');

            // Always skip the first one, it is an empty part because we start with a '/' if there are more parameters.
            index.Next();

            Info(*response);
            result->ContentType = Web::MIMETypes::MIME_JSON;
            result->Body(Core::ProxyType<Web::IBody>(response));
        } else {
            result->ErrorCode = Web::STATUS_BAD_REQUEST;
            result->Message = _T("Unsupported request for the [DisplayInfo] service.");
        }

        return result;
    }

    void DisplayInfo::Info(JsonData::DisplayInfo::DisplayinfoData& displayInfo) const
    {
        uint64_t ram = 0;
        if (_graphicsProperties->TotalGpuRam(ram) == Core::ERROR_NONE) {
            displayInfo.Totalgpuram = ram;
        }
        ram = 0;
        if (_graphicsProperties->FreeGpuRam(ram) == Core::ERROR_NONE) {
            displayInfo.Freegpuram = ram;
        }

        bool status = false;
        if (_connectionProperties->IsAudioPassthrough(status) == Core::ERROR_NONE) {
            displayInfo.Audiopassthrough = status;
        }
        status = false;
        if (_connectionProperties->Connected(status) == Core::ERROR_NONE) {
            displayInfo.Connected = status;
        }

        uint32_t value = 0;
        if (_connectionProperties->Width(value) == Core::ERROR_NONE) {
            displayInfo.Width = value;
        }
        value = 0;
        if (_connectionProperties->Height(value) == Core::ERROR_NONE) {
            displayInfo.Height = value;
        }

        Exchange::IConnectionProperties::HDCPProtectionType hdcpProtection(Exchange::IConnectionProperties::HDCPProtectionType::HDCP_Unencrypted);
        if ((const_cast<const Exchange::IConnectionProperties*>(_connectionProperties))->HDCPProtection(hdcpProtection) == Core::ERROR_NONE) {
            displayInfo.Hdcpprotection = static_cast<JsonData::DisplayInfo::DisplayinfoData::HdcpprotectionType>(hdcpProtection);
        }

        Exchange::IHDRProperties::HDRType hdrType(Exchange::IHDRProperties::HDRType::HDR_OFF);
        if (_hdrProperties->HDRSetting(hdrType) == Core::ERROR_NONE) {
            displayInfo.Hdrtype = static_cast<JsonData::DisplayInfo::DisplayinfoData::HdrtypeType>(hdrType);
        }
    }

    void DisplayInfo::Deactivated(RPC::IRemoteConnection* connection)
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
