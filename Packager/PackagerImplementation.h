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
#include <interfaces/IPackager.h>

#include <list>
#include <string>

// Forward declarations so we do not need to include the OPKG headers here.
struct opkg_conf;
struct _opkg_progress_data_t;

namespace WPEFramework {
namespace Plugin {

    class PackagerImplementation : public Exchange::IPackager {
    public:
        PackagerImplementation(const PackagerImplementation&) = delete;
        PackagerImplementation& operator=(const PackagerImplementation&) = delete;

        class EXTERNAL Config : public Core::JSON::Container {
        public:
            Config()
                : Core::JSON::Container()
                , TempDir()                     // Specify tmp-dir.
                , CacheDir()                    // Specify cache directory
                , MakeCacheVolatile(false)      // Use volatile cache. Volatile cache will be cleared on exit
                , Verbosity()
                , NoDeps()
                , NoSignatureCheck()
                , AlwaysUpdateFirst()
            {
                Add(_T("config"), &ConfigFile);
                Add(_T("temppath"), &TempDir);
                Add(_T("cachepath"), &CacheDir);
                Add(_T("volatilecache"), &MakeCacheVolatile);
                Add(_T("verbosity"), &Verbosity);
                Add(_T("nodeps"), &NoDeps);
                Add(_T("nosignaturecheck"), &NoSignatureCheck);
                Add(_T("alwaysupdatefirst"), &AlwaysUpdateFirst);
            }

            ~Config() override
            {
            }

            Config(const Config&) = delete;
            Config& operator=(const Config&) = delete;

            Core::JSON::String  ConfigFile;
            Core::JSON::String  TempDir;
            Core::JSON::String  CacheDir;
            Core::JSON::Boolean MakeCacheVolatile;
            Core::JSON::DecUInt8 Verbosity;
            Core::JSON::Boolean NoDeps;
            Core::JSON::Boolean NoSignatureCheck;
            Core::JSON::Boolean AlwaysUpdateFirst;
        };

        PackagerImplementation()
            : _adminLock()
            , _configFile()
            , _tempPath()
            , _cachePath()
            , _verbosity(0)
            , _noDeps(false)
            , _skipSignatureChecking(false)
            , _alwaysUpdateFirst(false)
            , _volatileCache(false)
            , _opkgInitialized(false)
            , _servicePI(nullptr)
            , _worker(this)
            , _isUpgrade(false)
            , _isSyncing(false)
        {
        }

        ~PackagerImplementation() override;

        class PluginConfig : public Core::JSON::Container {
        public:
            PluginConfig(const PluginConfig&) = delete;
            PluginConfig& operator=(const PluginConfig&) = delete;

            PluginConfig()
                : SystemRootPath()
            {
                Add(_T("systemrootpath"), &SystemRootPath);
            }

            Core::JSON::String SystemRootPath;

            ~PluginConfig() override
            {
            }
        };

        BEGIN_INTERFACE_MAP(PackagerImplementation)
            INTERFACE_ENTRY(Exchange::IPackager)
        END_INTERFACE_MAP

        //   IPackager methods
        void Register(Exchange::IPackager::INotification* observer) override;
        void Unregister(const Exchange::IPackager::INotification* observer) override;
        uint32_t Configure(PluginHost::IShell* service) override;
        uint32_t Install(const string& name, const string& version, const string& arch) override;
        uint32_t SynchronizeRepository() override;

    private:
        class PackageInfo : public Exchange::IPackager::IPackageInfo {
        public:
            PackageInfo(const PackageInfo&) = delete;
            PackageInfo& operator=(const PackageInfo&) = delete;

            ~PackageInfo() override
            {
            }

            PackageInfo(const std::string& name,
                        const std::string& version,
                        const std::string& arch)
                : _name(name)
                , _version(version)
                , _arch(arch)
            {
            }

            BEGIN_INTERFACE_MAP(PackageInfo)
                INTERFACE_ENTRY(Exchange::IPackager::IPackageInfo)
            END_INTERFACE_MAP

            // IPackageInfo methods
            string Name() const override
            {
                return _name;
            }

            string Version() const override
            {
                return _version;
            }

            string Architecture() const override
            {
                return _arch;
            }

        private:
            std::string _name;
            std::string _version;
            std::string _arch;
        };

        class InstallInfo : public Exchange::IPackager::IInstallationInfo {
        public:
            InstallInfo(const PackageInfo&) = delete;
            InstallInfo& operator=(const PackageInfo&) = delete;

            ~InstallInfo() override
            {
            }

            InstallInfo() = default;

            BEGIN_INTERFACE_MAP(InstallInfo)
                INTERFACE_ENTRY(Exchange::IPackager::IInstallationInfo)
            END_INTERFACE_MAP

            // IInstallationInfo methods
            Exchange::IPackager::state State() const override
            {
                return _state;
            }

            uint8_t Progress() const override
            {
                return _progress;
            }

            string AppName() const override
            {
                return _appname;
            }

            uint32_t ErrorCode() const override
            {
                return _error;
            }

            uint32_t Abort() override
            {
                return _error != 0 ? Core::ERROR_NONE : Core::ERROR_UNAVAILABLE;
            }

            void SetState(Exchange::IPackager::state state)
            {
                FILE* logFile = fopen("/opt/logs/rdm_status.log", "a");
                if (logFile != nullptr) {
                    fprintf(logFile, "SetState >->->  Debug: Setting state to %d\n", state);
                    fclose(logFile);
                }
                TRACE_L1("Setting state to %d", state);
                _state = state;
            }

            void SetProgress(uint8_t progress)
            {
                FILE* logFile = fopen("/opt/logs/rdm_status.log", "a");
                if (logFile != nullptr) {
                    fprintf(logFile, "SetProgress >->->  Debug: Setting progress to %d\n", progress);
                    fclose(logFile);
                }
                TRACE_L1("Setting progress to %d", progress);
                _progress = progress;
            }

            void SetAppName(const TCHAR path[])
            {
                FILE* logFile = fopen("/opt/logs/rdm_status.log", "a");
                if (logFile != nullptr) {
                    fprintf(logFile, "SetAppName >->->  Debug: Logging SetAppName\n");
                    string _pathname = Core::File::PathName(string(path));
                    string _dirname = _pathname.substr(0,_pathname.size()-1);
                    _appname = Core::File::FileName(_dirname);
                    fprintf(logFile, "SetAppName >->->  Debug: AppName set to %s\n", _appname.c_str());
                    fclose(logFile);
                }
            }

            void SetError(uint32_t err)
            {
                FILE* logFile = fopen("/opt/logs/rdm_status.log", "a");
                if (logFile != nullptr) {
                    fprintf(logFile, "SetError >->->  Debug: Setting Error to %d\n", err);
                    fclose(logFile);
                }
                TRACE_L1("Setting error to %d", err);
                _error = err;
            }

        private:
            Exchange::IPackager::state _state = Exchange::IPackager::IDLE;
            uint32_t _error = 0u;
            uint8_t _progress = 0u;
            string _appname;
        };

        struct InstallationData {
            InstallationData(const InstallationData& other) = delete;
            InstallationData& operator=(const InstallationData& other) = delete;
            InstallationData() = default;

            ~InstallationData()
            {
                if (Package)
                    Package->Release();
                if (Install)
                    Install->Release();
            }
            PackageInfo* Package = nullptr;
            InstallInfo* Install = nullptr;
        };

        class InstallThread : public Core::Thread {
        public:
            InstallThread(PackagerImplementation* parent)
                : _parent(parent)
            {}

            InstallThread& operator=(const InstallThread&) = delete;
            InstallThread(const InstallThread&) = delete;

            uint32_t Worker() override {
                while(IsRunning() == true) {
                    _parent->_adminLock.Lock(); // The parent may have lock when this starts so wait for it to release.
                    bool isInstall = _parent->_inProgress.Install != nullptr;
                    ASSERT(isInstall != true || _parent->_inProgress.Package != nullptr);
                    _parent->_adminLock.Unlock();

                    // After this point locking is not needed because API running on other threads only read if in
                    // progress is filled in.
                    _parent->BlockingSetupLocalRepoNoLock(isInstall == true ? RepoSyncMode::SETUP : RepoSyncMode::FORCED);
                    if (isInstall)
                        _parent->BlockingInstallUntilCompletionNoLock();

                    if (isInstall) {
                        _parent->_adminLock.Lock();
                        _parent->_inProgress.Install->Release();
                        _parent->_inProgress.Package->Release();
                        _parent->_inProgress.Install = nullptr;
                        _parent->_inProgress.Package = nullptr;
                        _parent->_adminLock.Unlock();
                    }

                    Block();
                }

                return Core::infinite;
            }

        private:
            PackagerImplementation* _parent;
        };

        enum class RepoSyncMode {
            FORCED,
            SETUP
        };

        uint32_t DoWork(const string* name, const string* version, const string* arch);
        void UpdateConfig() const;
#if !defined (DO_NOT_USE_DEPRECATED_API)
        static void InstallationProgessNoLock(const _opkg_progress_data_t* progress, void* data);
#endif
        string GetMetadataFile(const string& appName);
        string GetCallsign(const string& mfilename);
        string GetInstallationPath(const string& appName);
        void DeactivatePlugin(const string& callsign, const string& appName);
        uint32_t UpdateConfiguration(const string& callsign, const string& appName);
        void NotifyStateChange();
        void NotifyRepoSynced(uint32_t status);
        void BlockingInstallUntilCompletionNoLock();
        void BlockingSetupLocalRepoNoLock(RepoSyncMode mode);
        bool InitOPKG();
        void FreeOPKG();

        Core::CriticalSection _adminLock;
        string _configFile;
        string _tempPath;
        string _cachePath;
        int _verbosity;
        bool _noDeps;
        bool _skipSignatureChecking;
        bool _alwaysUpdateFirst;
        bool _volatileCache;
        bool _opkgInitialized;
        PluginHost::IShell* _servicePI;
        std::vector<Exchange::IPackager::INotification*> _notifications;
        InstallationData _inProgress;
        InstallThread _worker;
        bool _isUpgrade;
        bool _isSyncing;
    };

}  // namespace Plugin
}  // namespace WPEFramework
