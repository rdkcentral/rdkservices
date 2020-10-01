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
            result = _T("Couldn't create PACKAGER instance ");

        } else {
            if (_implementation->Configure(_service) != Core::ERROR_NONE) {
                result = _T("Couldn't initialize PACKAGER instance");
            }
        }

        return (result);
    }

    void Packager::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);

        _service->Unregister(&_notification);
        _implementation->Unregister(&_notification);

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

    // JSONRPC

    void Packager::event_installstep(Exchange::IPackager::state status, uint32_t task, string id, int32_t code)
    {
        JsonObject params;
        params["pkgId"]  = id;
        params["task"]   = std::to_string( task );
        params["status"] = std::to_string( status );
        params["code"]   = std::to_string( code );

        std::string str("empty");

        switch(status)
        {
            case Exchange::IPackager::DOWNLOADING:         str = "onDownloadCommence";    break;
            case Exchange::IPackager::DOWNLOADED:          str = "onDownloadComplete";    break;
            case Exchange::IPackager::VERIFYING:           str = "onExtractCommence";     break;
            case Exchange::IPackager::VERIFIED:            str = "onExtractComplete";     break;
            case Exchange::IPackager::INSTALLING:          str = "onInstallCommence";     break;
            case Exchange::IPackager::INSTALLED:           str = "onInstallComplete";     break;
            case Exchange::IPackager::DOWNLOAD_FAILED:     str = "onDownload_FAILED";     break;
            case Exchange::IPackager::DECRYPTION_FAILED:   str = "onDecryption_FAILED";   break;
            case Exchange::IPackager::EXTRACTION_FAILED:   str = "onExtraction_FAILED";   break;
            case Exchange::IPackager::VERIFICATION_FAILED: str = "onVerification_FAILED"; break;
            case Exchange::IPackager::INSTALL_FAILED:      str = "onInstall_FAILED";      break;

            default: str = "NotifyInstallStep";  break;
        }

        printf("\n #######  event_installstep() >>>   %s", str.c_str());
        
        Notify(str, params);
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

        if (index.Next() && (request.Verb == Web::Request::HTTP_POST || request.Verb == Web::Request::HTTP_PUT))
        {
            uint32_t status = Core::ERROR_UNAVAILABLE;

            Core::URL::KeyValue options(request.Query.Value());

            ////////////////////////////////////////////////
            //
            // Packager >> Install
            //
            if (index.Current().Text() == "Install")
            {
                if( ( options.Exists(_T("Package"),      true) == true ) &&
                    ( options.Exists(_T("Architecture"), true) == true ) &&
                    ( options.Exists(_T("Version"),      true) == true ) )
                {
                    std::array<char, kMaxValueLength> package {0};
                    std::array<char, kMaxValueLength> version {0};
                    std::array<char, kMaxValueLength> arch {0};

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

                    // Packager API
                    status = _implementation->Install(package.data(), version.data(), arch.data());
                }
                else
                if( ( options.Exists(_T("pkgId"), true) == true ) &&
                    ( options.Exists(_T("type"),  true) == true ) &&
                    ( options.Exists(_T("url"),   true) == true )  )
                {
                    std::array<char, kMaxValueLength> pkgId    {0};
                    std::array<char, kMaxValueLength> type     {0};
                    std::array<char, kMaxValueLength> url      {0};
                    std::array<char, kMaxValueLength> token    {0};
                    std::array<char, kMaxValueLength> listener {0};

                    if (options.Exists(_T("pkgId"), true) == true) {
                        const string name (options[_T("pkgId")].Text());
                        Core::URL::Decode (name.c_str(), name.length(), pkgId.data(), pkgId.size());
                    }
                    if (options.Exists(_T("type"), true) == true) {
                        const string name (options[_T("type")].Text());
                        Core::URL::Decode (name.c_str(), name.length(), type.data(), type.size());
                    }
                    if (options.Exists(_T("url"), true) == true) {
                        const string name (options[_T("url")].Text());
                        Core::URL::Decode (name.c_str(), name.length(), url.data(), url.size());
                    }
                    if (options.Exists(_T("token"), true) == true) {
                        const string name (options[_T("token")].Text());
                        Core::URL::Decode (name.c_str(), name.length(), token.data(), token.size());
                    }
                    if (options.Exists(_T("listener"), true) == true) {
                        const string name (options[_T("listener")].Text());
                        Core::URL::Decode (name.c_str(), name.length(), listener.data(), listener.size());
                    }

                    // DAC Installer API
                    status = _implementation->Install(pkgId.data(), type.data(), url.data(), token.data(), listener.data());
                }
            }
            else
            ////////////////////////////////////////////////
            //
            // Packager >> SynchronizeRepository
            //
            if (index.Current().Text() == "SynchronizeRepository")
            {
                status = _implementation->SynchronizeRepository();
            }
            else
            ////////////////////////////////////////////////
            //
            // DAC Installer >> Remove
            //
            if (index.Current().Text() == "Remove")
            {
                std::array<char, kMaxValueLength> pkgId    {0};
                std::array<char, kMaxValueLength> listener {0};

                Core::URL::KeyValue options(request.Query.Value());

                if (options.Exists(_T("PkgId"), true) == true) {
                    const string name (options[_T("PkgId")].Text());
                    Core::URL::Decode (name.c_str(), name.length(), pkgId.data(), pkgId.size());
                }

                if (options.Exists(_T("Listener"), true) == true) {
                    const string name (options[_T("Listener")].Text());
                    Core::URL::Decode (name.c_str(), name.length(), listener.data(), listener.size());
                }

                status = _implementation->Remove(pkgId.data(), listener.data());
            }
            else
            ////////////////////////////////////////////////
            //
            // DAC Installer >> Cancel
            //
            if (index.Current().Text() == "Cancel")
            {
                std::array<char, kMaxValueLength> task     {0};
                std::array<char, kMaxValueLength> listener {0};

                Core::URL::KeyValue options(request.Query.Value());

                if (options.Exists(_T("Task"), true) == true) {
                                    const string name (options[_T("Task")].Text());
                                    Core::URL::Decode (name.c_str(), name.length(), task.data(), task.size());
                }
                if (options.Exists(_T("Listener"), true) == true) {
                    const string name (options[_T("Listener")].Text());
                    Core::URL::Decode (name.c_str(), name.length(), listener.data(), listener.size());
                }

                status = _implementation->Cancel(task.data(), listener.data());
            }
            else
            ////////////////////////////////////////////////
            //
            // DAC Installer >> IsInstalled
            //
            if (index.Current().Text() == "IsInstalled")
            {
                std::array<char, kMaxValueLength> pkgId {0};

                Core::URL::KeyValue options(request.Query.Value());

                if (options.Exists(_T("PkgId"), true) == true) {
                    const string name (options[_T("PkgId")].Text());
                    Core::URL::Decode (name.c_str(), name.length(), pkgId.data(), pkgId.size());
                }

                status = _implementation->IsInstalled(pkgId.data());
            }
            else
            ////////////////////////////////////////////////
            //
            // DAC Installer >> GetInstallProgress
            //
            if (index.Current().Text() == "GetInstallProgress")
            {
                std::array<char, kMaxValueLength> task {0};

                Core::URL::KeyValue options(request.Query.Value());

                if (options.Exists(_T("Task"), true) == true) {
                    const string name (options[_T("Task")].Text());
                    Core::URL::Decode (name.c_str(), name.length(), task.data(), task.size());
                }

                float pc = _implementation->GetInstallProgress(task.data());

                char str[255];
                snprintf(str, 255, "%0.2f%%", pc);

                result->Message = string(str);
            }
            else
            ////////////////////////////////////////////////
            //
            // DAC Installer >> GetInstalled
            //
            if (index.Current().Text() == "GetInstalled")
            {
                /* status = */ _implementation->GetInstalled();
            }
            else
            ////////////////////////////////////////////////
            //
            // DAC Installer >> GetPackageInfo
            //
            if (index.Current().Text() == "GetPackageInfo")
            {
                std::array<char, kMaxValueLength> pkgId {0};

                Core::URL::KeyValue options(request.Query.Value());

                if (options.Exists(_T("PkgId"), true) == true) {
                    const string name (options[_T("PkgId")].Text());
                    Core::URL::Decode (name.c_str(), name.length(), pkgId.data(), pkgId.size());
                }

                /* status = */ _implementation->GetPackageInfo(pkgId.data());
            }
            else
            ////////////////////////////////////////////////
            //
            // DAC Installer >> GetAvailableSpace
            //
            if (index.Current().Text() == "GetAvailableSpace")
            {
                // fprintf(stderr, "\n >>>>> Call ... Pacakger::GetAvailableSpace() ... " );

                /* status = */ _implementation->GetAvailableSpace();
            }
            ////////////////////////////////////////////////

            if (status == Core::ERROR_NONE) {
                result->ErrorCode = Web::STATUS_OK;
                result->Message = _T("OK");
            } else if (status == Core::ERROR_INPROGRESS) {
                result->Message = _T("Some operation already in progress. Only one at a time is allowed");
            }
        }

        return(result);
    }

    void Packager::IntallStep(Exchange::IPackager::state status, uint32_t task, string id, int32_t code)
    {
        // LOGINFO("Packager::IntallStep(uint32_t status)  >>> %u", status);
        event_installstep(status, task, id, code);
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
