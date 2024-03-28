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
 
#include "PackagerImplementation.h"

#if defined (DO_NOT_USE_DEPRECATED_API)
#include <opkg_cmd.h>
#else
#include <opkg.h>
#endif
#include <opkg_download.h>
#include <pkg.h>

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(PackagerImplementation, 1, 0);

    void PackagerImplementation::UpdateConfig() const {
        ASSERT(!_configFile.empty() && !_tempPath.empty() && !_cachePath.empty());
        opkg_config->conf_file = strdup(_configFile.c_str());
        opkg_config->tmp_dir = strdup(_tempPath.c_str());
        opkg_config->host_cache_dir = 1;
        opkg_config->cache_dir = strdup(_cachePath.c_str());
        opkg_config->verbosity = _verbosity;
        opkg_config->nodeps = _noDeps;

        if (_volatileCache == true) {
            opkg_config->volatile_cache = 1;
        }

        if (_skipSignatureChecking == true) {
            opkg_config->check_pkg_signature = 0;
        } else {
            opkg_config->check_pkg_signature = 1;
            opkg_config->signature_type = strdup("provision");
        }
    }

    uint32_t PackagerImplementation::Configure(PluginHost::IShell* service)
    {
        uint32_t result = Core::ERROR_NONE;
        Config config;

        ASSERT (_servicePI == nullptr);
        _servicePI = service;
        _servicePI->AddRef();

        config.FromString(service->ConfigLine());

        if ((config.ConfigFile.IsSet() == true) && (config.ConfigFile.Value().empty() == false)) {
            _configFile = config.ConfigFile.Value();
        } else {
            _configFile = service->DataPath() + _T("opkg.conf");
        }

        if ((config.TempDir.IsSet() == true) && (config.TempDir.Value().empty() == false)) {
            _tempPath = Core::Directory::Normalize(config.TempDir.Value());
        } else {
            _tempPath = service->VolatilePath() + service->Callsign();
        }

        if ((config.CacheDir.IsSet() == true) && (config.CacheDir.Value().empty() == false)) {
            _cachePath = Core::Directory::Normalize(config.CacheDir.Value());
        } else {
            _cachePath = service->PersistentPath() + service->Callsign();
        }

        if (config.Verbosity.IsSet() == true) {
            _verbosity = config.Verbosity.Value();
        }

        if (config.NoDeps.IsSet() == true) {
            _noDeps = config.NoDeps.Value();
        }

        if (config.NoSignatureCheck.IsSet() == true) {
            _skipSignatureChecking = config.NoSignatureCheck.Value();
        }

         if (config.AlwaysUpdateFirst.IsSet() == true) {
            _alwaysUpdateFirst = config.AlwaysUpdateFirst.Value();
         }

         if (config.MakeCacheVolatile.IsSet() == true) {
             _volatileCache = config.MakeCacheVolatile.Value();
         }

        if (Core::File(_configFile).Exists() == false) {
            result = Core::ERROR_GENERAL;
        } else if (Core::Directory(_tempPath.c_str()).CreatePath() == false) {
            result = Core::ERROR_GENERAL;
        } else if (Core::Directory(_cachePath.c_str()).CreatePath() == false) {
            result = Core::ERROR_GENERAL;
        } else {
            /* See Install() for explanation why it's not done here.
            if (InitOPKG() == false) {
                result = Core::ERROR_GENERAL;
            }
            */
        }

        return (result);
    }

    PackagerImplementation::~PackagerImplementation()
    {
        FreeOPKG();
        _servicePI->Release();
        _servicePI = nullptr;
    }

    void PackagerImplementation::Register(Exchange::IPackager::INotification* notification)
    {
        ASSERT(notification);
        _adminLock.Lock();
        notification->AddRef();
        _notifications.push_back(notification);
        if (_inProgress.Install != nullptr) {
            ASSERT(_inProgress.Package != nullptr);
            notification->StateChange(_inProgress.Package, _inProgress.Install);
        }
        _adminLock.Unlock();
    }

    void PackagerImplementation::Unregister(const Exchange::IPackager::INotification* notification)
    {
        ASSERT(notification);
        _adminLock.Lock();
        auto item = std::find(_notifications.begin(), _notifications.end(), notification);
        ASSERT(item != _notifications.end());
        (*item)->Release();
        _notifications.erase(item);
        _adminLock.Unlock();
    }

    uint32_t PackagerImplementation::Install(const string& name, const string& version, const string& arch)
    {
        return DoWork(&name, &version, &arch);
    }

    uint32_t PackagerImplementation::SynchronizeRepository()
    {
        return DoWork(nullptr, nullptr, nullptr);
    }

    uint32_t PackagerImplementation::DoWork(const string* name, const string* version, const string* arch)
    {
        uint32_t result = Core::ERROR_INPROGRESS;

        _adminLock.Lock();
        if (_inProgress.Install == nullptr && _isSyncing == false) {
            ASSERT(_inProgress.Package == nullptr);
            result = Core::ERROR_NONE;
            // OPKG bug: it marks it checked dependency for a package as cyclic dependency handling fix
            // but since in our case it's not an process which dies when done, this info survives and makes the
            // deps check to be skipped on subsequent calls. This is why hash_deinit() is called below
            // and needs to be initialized here agian.
            if (_opkgInitialized == true)  // it was initialized
                FreeOPKG();
            _opkgInitialized = InitOPKG();

            if (_opkgInitialized) {
                if (name && version && arch) {
                    _inProgress.Package = Core::Service<PackageInfo>::Create<PackageInfo>(*name, *version, *arch);
                    _inProgress.Install = Core::Service<InstallInfo>::Create<InstallInfo>();
                } else {
                    _isSyncing = true;
                }
                _worker.Run();
            } else {
                result = Core::ERROR_GENERAL;
            }
        }
        _adminLock.Unlock();

        return result;

    }

    void PackagerImplementation::BlockingInstallUntilCompletionNoLock() {
        ASSERT(_inProgress.Install != nullptr && _inProgress.Package != nullptr);

#if defined (DO_NOT_USE_DEPRECATED_API)
        opkg_cmd_t* command = opkg_cmd_find("install");
        if (command) {
            _inProgress.Install->SetState(Exchange::IPackager::INSTALLING);
            NotifyStateChange();
            opkg_config->pfm = command->pfm;
            std::unique_ptr<char[]> targetCopy(new char [_inProgress.Package->Name().length() + 1]);
            std::copy_n(_inProgress.Package->Name().begin(), _inProgress.Package->Name().length(), targetCopy.get());
            (targetCopy.get())[_inProgress.Package->Name().length()] = 0;
            const char* argv[1];
            argv[0] = targetCopy.get();
            if (opkg_cmd_exec(command, 1, argv) == 0) {
                _inProgress.Install->SetProgress(100);
                _inProgress.Install->SetState(Exchange::IPackager::INSTALLED);
            } else {
                _inProgress.Install->SetError(Core::ERROR_GENERAL);
            }
            NotifyStateChange();
        } else {
            _inProgress.Install->SetError(Core::ERROR_GENERAL);
            NotifyStateChange();
        }
#else
        _isUpgrade = false;
        opkg_package_callback_t checkUpgrade = [](pkg* pkg, void* user_data) {
            PackagerImplementation* self = static_cast<PackagerImplementation*>(user_data);
            if (self->_isUpgrade == false) {
                self->_isUpgrade = self->_inProgress.Package->Name() == pkg->name;
                if (self->_isUpgrade && self->_inProgress.Package->Version().empty() == false) {
                    self->_isUpgrade = opkg_compare_versions(pkg->version,
                                                             self->_inProgress.Package->Version().c_str()) < 0;
                }
            }
        };
        opkg_list_upgradable_packages(checkUpgrade, this);

        typedef int (*InstallFunction)(const char *, opkg_progress_callback_t, void *);
        InstallFunction installFunction = opkg_install_package;
        if (_isUpgrade) {
            installFunction = opkg_upgrade_package;
        }
        _isUpgrade = false;

        if (installFunction(_inProgress.Package->Name().c_str(), PackagerImplementation::InstallationProgessNoLock,
                            this) != 0) {
            _inProgress.Install->SetError(Core::ERROR_GENERAL);
            NotifyStateChange();
        }
#endif
    }

#if !defined (DO_NOT_USE_DEPRECATED_API)
    /* static */ void PackagerImplementation::InstallationProgessNoLock(const opkg_progress_data_t* progress,
                                                                        void* data)
    {
        PackagerImplementation* self = static_cast<PackagerImplementation*>(data);
        self->_inProgress.Install->SetProgress(progress->percentage);
        if (progress->action == OPKG_INSTALL &&
            self->_inProgress.Install->State() == Exchange::IPackager::DOWNLOADING) {
            self->_inProgress.Install->SetState(Exchange::IPackager::DOWNLOADED);
            self->NotifyStateChange();
        }
        bool stateChanged = false;
        switch (progress->action) {
            case OPKG_DOWNLOAD:
                if (self->_inProgress.Install->State() != Exchange::IPackager::DOWNLOADING) {
                    self->_inProgress.Install->SetState(Exchange::IPackager::DOWNLOADING);
                    stateChanged = true;
                }
                break;
            case OPKG_INSTALL:
                if (self->_inProgress.Install->State() != Exchange::IPackager::INSTALLING) {
                    self->_inProgress.Install->SetState(Exchange::IPackager::INSTALLING);
                    stateChanged = true;
                }
                break;
        }

        if (stateChanged == true)
            self->NotifyStateChange();
        if (progress->percentage == 100) {
            self->_inProgress.Install->SetState(Exchange::IPackager::INSTALLED);
            self->NotifyStateChange();
            self->_inProgress.Install->SetAppName(progress->pkg->local_filename);
            string mfilename = self->GetMetadataFile(self->_inProgress.Install->AppName());
            string callsign = self->GetCallsign(mfilename);
            if(!callsign.empty()) {
                self->DeactivatePlugin(callsign, self->_inProgress.Install->AppName());
            }
        }
    }
#endif

    string PackagerImplementation::GetMetadataFile(const string& appName)
    {
        char *dnld_loc = opkg_config->cache_dir;
        string mfilename = string(dnld_loc) + "/" + appName + "/etc/apps/" + appName + "_package.json";
        return mfilename;
    }

    string PackagerImplementation::GetCallsign(const string& mfilename)
    {
        string callsign = "";
        TRACE(Trace::Information, (_T("[Packager]: Metadata is %s"),mfilename.c_str()));
        Core::File file(mfilename);
        if(file.Open()) {
            JsonObject parameters;
            if(parameters.IElement::FromFile(file)) {
                if(parameters.HasLabel("type")) {
                    string type = parameters["type"].String();
                    if( 0 == type.compare("plugin")) {
                        if(parameters.HasLabel("callsign")) {
                            callsign = parameters["callsign"].String();
                        }
                        else {
                            TRACE(Trace::Information, (_T("[Packager]: callsign missing in metadata")));
                        }
                    }
                    else {
                        TRACE(Trace::Information, (_T("[Packager]: Package does not contain thunder plugin")));
                    }
                }
                else {
                    TRACE(Trace::Information, (_T("[Packager]: Metadata type not found")));
                }
            }
            else {
                TRACE(Trace::Error, (_T("[Packager]: Error in reading the file")));
            }
        }
        else {
            TRACE(Trace::Error, (_T("[Packager]: Error in opening the file")));
        }
        return callsign;
    }

    string PackagerImplementation::GetInstallationPath(const string& appname)
    {
        char *dnld_loc = opkg_config->cache_dir;
        string instPath = string(dnld_loc) + "/" + appname;
        return instPath;
    }

    uint32_t PackagerImplementation::UpdateConfiguration(const string& callsign, const string& installPath)
    {
        uint32_t result = Core::ERROR_GENERAL;
        ASSERT(callsign.empty() == false);
        ASSERT(_servicePI != nullptr);

        PluginConfig pluginconfig;

        PluginHost::IShell* shell = _servicePI->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
        if (shell != nullptr) {
            if (shell->SystemRootPath(installPath)  == Core::ERROR_NONE) {
                TRACE(Trace::Information, (_T("[Packager]: SystemRootPath for %s is %s"), callsign.c_str(), shell->SystemRootPath().c_str()));

                PluginHost::IController* controller = _servicePI->QueryInterfaceByCallsign<PluginHost::IController>(EMPTY_STRING);
                if (controller != nullptr) {
                    if (controller->Persist() == Core::ERROR_NONE) {
                        result = Core::ERROR_NONE;
                        TRACE(Trace::Information, (_T("[Packager]: Successfully stored %s plugin's config in peristent path"), callsign.c_str()));
                    }
                    controller->Release();
                }
                else {
                    TRACE(Trace::Error, (_T("[Packager]: Failed to find Controller interface")));
                }
            }
            shell->Release();
        }
        else {
            TRACE(Trace::Error, (_T("[Packager]: Failed to find Shell interface")));
        }
        return result;
    }

    void PackagerImplementation::DeactivatePlugin(const string& callsign, const string& appName)
    {
        ASSERT(callsign.empty() == false);
        ASSERT(_servicePI != nullptr);
        TRACE(Trace::Information, (_T("[Packager]: callsign from metadata is %s"), callsign.c_str()));
        PluginHost::IShell* dlPlugin = _servicePI->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);

        if (dlPlugin == nullptr) {
            TRACE(Trace::Error, (_T("[Packager]: Plugin %s is not configured in this setup"), callsign.c_str()));
        }
        else {
            PluginHost::IShell::state currentState(dlPlugin->State());
            if (currentState != PluginHost::IShell::UNAVAILABLE) {
                TRACE(Trace::Information, (_T("[Packager]: Plugin %s is not in Unavailable state. Hence, not deactivating it"),callsign.c_str()));
            }
            else {
                TRACE(Trace::Information, (_T("[Packager]: Plugin %s is in Unavailable state"), callsign.c_str()));
                uint32_t result = dlPlugin->Deactivate(PluginHost::IShell::REQUESTED);
                if (result == Core::ERROR_NONE) {
                    TRACE(Trace::Information, (_T("[Packager]: %s moved to Deactivated state"), callsign.c_str()));
                    string appInstallPath = GetInstallationPath(appName);
                    if (UpdateConfiguration(callsign, appInstallPath) != Core::ERROR_NONE) {
                        TRACE(Trace::Error, (_T("[Packager]: Failed to update SystemRootPath for %s"), callsign.c_str()));
                    }
                }
                else {
                    TRACE(Trace::Error, (_T("[Packager]: Failed to move %s to Deactivated state"), callsign.c_str()));
                }
            }
            dlPlugin->Release();
        }
    }

    void PackagerImplementation::NotifyStateChange()
    {
        _adminLock.Lock();
        TRACE_L1("State for %s changed to %d (%d %%, %d)", _inProgress.Package->Name().c_str(), _inProgress.Install->State(), _inProgress.Install->Progress(), _inProgress.Install->ErrorCode());
        for (auto* notification : _notifications) {
            notification->StateChange(_inProgress.Package, _inProgress.Install);
        }
        _adminLock.Unlock();
    }

    void PackagerImplementation::NotifyRepoSynced(uint32_t status)
    {
        _adminLock.Lock();
        _isSyncing = false;
        for (auto* notification : _notifications) {
            notification->RepositorySynchronize(status);
        }
        _adminLock.Unlock();
    }

    bool PackagerImplementation::InitOPKG()
    {
        UpdateConfig();
#if defined DO_NOT_USE_DEPRECATED_API
        return opkg_conf_init() == 0 && pkg_hash_load_feeds() == 0 && pkg_hash_load_status_files() == 0;
#else
        return opkg_new() == 0;
#endif
    }

    void PackagerImplementation::FreeOPKG()
    {
        if (_opkgInitialized == true) {
            opkg_download_cleanup();
            opkg_conf_deinit();
            _opkgInitialized = false;
        }
    }

    void PackagerImplementation::BlockingSetupLocalRepoNoLock(RepoSyncMode mode)
    {
        string dirPath = Core::ToString(opkg_config->lists_dir);
        Core::Directory dir(dirPath.c_str());
        bool containFiles = false;
        if (mode == RepoSyncMode::SETUP && _alwaysUpdateFirst == false) {
            while (dir.Next() == true) {
                if (dir.Name() != _T(".") && dir.Name() != _T("..") && dir.Name() != dirPath) {
                    containFiles = true;
                    break;
                }
            }
        }
        ASSERT(mode == RepoSyncMode::SETUP || _isSyncing == true);
        if (containFiles == false) {
            uint32_t result = Core::ERROR_NONE;
#if defined DO_NOT_USE_DEPRECATED_API
            opkg_cmd_t* command = opkg_cmd_find("update");
            if (command)
                opkg_config->pfm = command->pfm;
            if (command == nullptr || opkg_cmd_exec(command, 0, nullptr) != 0)
#else
            if (opkg_update_package_lists(nullptr, nullptr) != 0)
#endif
            {
                TRACE_L1("Failed to set up local repo. Installing might not work");
                result = Core::ERROR_GENERAL;
            }
            NotifyRepoSynced(result);
        }
    }

}  // namespace Plugin
}  // namespace WPEFramework
