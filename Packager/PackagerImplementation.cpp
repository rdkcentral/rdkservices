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

        ASSERT(_service == nullptr);
        ASSERT(service != nullptr);
        _service = service;
        _service->AddRef();

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
        ASSERT(_service != nullptr);
        _service->Release();
        _service = nullptr;
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

            ASSERT(opkg_config != nullptr);
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
        ASSERT(data != nullptr);
        ASSERT(progress != nullptr);
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
            string callsign = self->GetCallsignFromMetaDataFile(self->_inProgress.Install->AppName());
            if (!callsign.empty()) {
                self->DeactivatePlugin(callsign);
            }
        }
    }
#endif

    string PackagerImplementation::GetCallsignFromMetaDataFile(const string& appName)
    {
        string callsign = "";

        ASSERT(opkg_config != nullptr);
        string metaDataFileName = (string(opkg_config->cache_dir) + "/" + appName + string(AppPath) + appName + string(PackageJSONFile));
        TRACE(Trace::Information, (_T("[RDM]: Metadata is %s"), metaDataFileName.c_str()));
        Core::File metaDataFile(metaDataFileName);
        if (metaDataFile.Open()) {
            MetaData metaData;
            Core::OptionalType<Core::JSON::Error> error;
            if (metaData.IElement::FromFile(metaDataFile, error)) {
                if (error.IsSet() == true) {
                    TRACE(Trace::Error, (_T("Parsing failed with %s"), ErrorDisplayMessage(error.Value()).c_str()));
                } else {
                    if ((metaData.Type.IsSet() == true) && (metaData.Type.Value() == PackagerImplementation::PackageType::PLUGIN)) {
                        if (metaData.Callsign.IsSet() == true) {
                            callsign = metaData.Callsign.Value();
                        } else {
                            TRACE(Trace::Information, (_T("[RDM]: callsign missing in metadata")));
                        }
                    } else {
                        TRACE(Trace::Information, (_T("[RDM]: MetaData file does not contain plugin type")));
                    }
                }
            }
        } else {
            TRACE(Trace::Error, (_T("[RDM]: Error in opening the file")));
        }
        return callsign;
    }

    void PackagerImplementation::DeactivatePlugin(const string& callsign)
    {
        ASSERT(_service != nullptr);
        ASSERT(callsign.empty() == false);

        TRACE(Trace::Information, (_T("[RDM]: callsign from metadata is %s"), callsign.c_str()));
        PluginHost::IShell* plugin = _service->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);

        if (plugin != nullptr) {
            PluginHost::IShell::state currentState(plugin->State());
            if ((currentState == PluginHost::IShell::ACTIVATED) || (currentState == PluginHost::IShell::ACTIVATION) || (currentState == PluginHost::IShell::PRECONDITION)) {
                TRACE(Trace::Information, (_T("[RDM]: Plugin %s is activated state, so deactivating"), callsign.c_str()));
                uint32_t result = plugin->Deactivate(PluginHost::IShell::REQUESTED);
                if (result == Core::ERROR_NONE) {
                    TRACE(Trace::Information, (_T("[RDM]: %s moved to Deactivated state"), callsign.c_str()));
                }
                else {
                    TRACE(Trace::Error, (_T("[RDM]: Failed to move %s to Deactivated state"), callsign.c_str()));
                }
            } else if (currentState == PluginHost::IShell::UNAVAILABLE) {
                TRACE(Trace::Information, (_T("[RDM]: Plugin %s is unavailable"), callsign.c_str()));
            }
            else {
                TRACE(Trace::Information, (_T("[RDM]: Plugin %s is already in deactivated state"), callsign.c_str()));
            }

            plugin->Release();
        } else {
            TRACE(Trace::Error, (_T("[RDM]: Plugin %s is not configured in this setup"), callsign.c_str()));
        }
    }

    void PackagerImplementation::NotifyStateChange()
    {
        _adminLock.Lock();
        TRACE(Trace::Information, (_T("State for %s changed to %d (%d %%, %d)"), _inProgress.Package->Name().c_str(), _inProgress.Install->State(), _inProgress.Install->Progress(), _inProgress.Install->ErrorCode()));
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
        ASSERT(opkg_config != nullptr);
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
                TRACE(Trace::Error, (_T("Failed to set up local repo. Installing might not work")));
                result = Core::ERROR_GENERAL;
            }
            NotifyRepoSynced(result);
        }
    }

}  // namespace Plugin
ENUM_CONVERSION_BEGIN(Plugin::PackagerImplementation::PackageType)
    { Plugin::PackagerImplementation::PackageType::NONE, _TXT("none") },
    { Plugin::PackagerImplementation::PackageType::PLUGIN, _TXT("plugin") },
ENUM_CONVERSION_END(Plugin::PackagerImplementation::PackageType);
}  // namespace WPEFramework
