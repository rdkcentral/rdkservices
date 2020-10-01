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

#include "PackagerExImplementation.h"


// Forward declarations so we do not need to include the OPKG headers here.
struct opkg_conf;
struct _opkg_progress_data_t;

struct JobMeta_t; // fwd

namespace WPEFramework {
namespace Plugin {


    class PackagerImplementation : public Exchange::IPackager
    {
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
            , _worker(this)
            , _isUpgrade(false)
            , _isSyncing(false)
            , _taskNumber(0)
        {
            InitPackageDB();
        }

        ~PackagerImplementation() override;

        BEGIN_INTERFACE_MAP(PackagerImplementation)
            INTERFACE_ENTRY(Exchange::IPackager)
        END_INTERFACE_MAP

        //   IPackager methods
        void Register(Exchange::IPackager::INotification* observer) override;
        void Unregister(const Exchange::IPackager::INotification* observer) override;


        virtual void Register(PluginHost::IStateControl::INotification* sink)
        {
            _adminLock.Lock();

            // Make sure a sink is not registered multiple times.
            ASSERT(std::find(_stateControlClients.begin(), _stateControlClients.end(), sink) == _stateControlClients.end());

            _stateControlClients.push_back(sink);
            sink->AddRef();

            _adminLock.Unlock();

            TRACE_L1("Registered a sink on the browser %p", sink);
        }

        virtual void Unregister(PluginHost::IStateControl::INotification* sink)
        {
            _adminLock.Lock();

            std::list<PluginHost::IStateControl::INotification*>::iterator index(std::find(_stateControlClients.begin(), _stateControlClients.end(), sink));

            // Make sure you do not unregister something you did not register !!!
            ASSERT(index != _stateControlClients.end());

            if (index != _stateControlClients.end()) {
                (*index)->Release();
                _stateControlClients.erase(index);
                TRACE_L1("Unregistered a sink on the browser %p", sink);
            }

            _adminLock.Unlock();
        }

        uint32_t Configure(PluginHost::IShell* service) override;

        // Packager API
        uint32_t Install(const string& name, const string& version, const string& arch) override;
        uint32_t SynchronizeRepository() override;

        // DAC Installer API

        uint32_t Install(const string& pkgId, const string& type, const string& url, 
                         const string& token, const string& listener);

        uint32_t Remove(const string& pkgId, const string& listener);
        uint32_t Cancel(const string& task, const string& listener);

        uint32_t                   IsInstalled(const string& pkgId);
        uint32_t                   GetInstallProgress( const string& task);
        PackageInfoEx::IIterator*  GetInstalled();
        PackageInfoEx*             GetPackageInfo(const string& pkgId);
        int64_t                    GetAvailableSpace();

      //  uint32_t                   getNextTaskID()  { return _taskNumber++; }

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

            std::string _pkgId;
            std::string _type;
            std::string _url;
            std::string _token;
            std::string _listener;

            uint32_t    _refcount;
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
                TRACE_L1("Setting state to %d", state);
                _state = state;
            }

            void SetProgress(uint8_t progress)
            {
                TRACE_L1("Setting progress to %d", progress);
                _progress = progress;
            }

            void SetError(uint32_t err)
            {
                TRACE_L1("Setting error to %d", err);
                _error = err;
            }

        private:
            Exchange::IPackager::state _state = Exchange::IPackager::IDLE;
            uint32_t _error = 0u;
            uint8_t _progress = 0u;
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

            uint32_t Worker() override
            {
                while(IsRunning() == true)
                {
                    _parent->_adminLock.Lock(); // The parent may have lock when this starts so wait for it to release.
                    bool isInstall = _parent->_inProgress.Install != nullptr;
                    ASSERT(isInstall != true || _parent->_inProgress.Package != nullptr);
                    _parent->_adminLock.Unlock();
           
                    // After this point locking is not needed because API running on other threads only read if in
                    // progress is filled in.
                    _parent->BlockingSetupLocalRepoNoLock(isInstall == true ? RepoSyncMode::SETUP : RepoSyncMode::FORCED);
                    if (isInstall)
                    {
                        _parent->BlockingInstallUntilCompletionNoLock();
                    }

                    if (isInstall)
                    {
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
        void NotifyStateChange();
        void NotifyRepoSynced(uint32_t status);

        void InitPackageDB();
        void TermPackageDB();

        void NotifyIntallStep(Exchange::IPackager::state status, uint32_t task = 0, string id = "", int32_t code = 0);   // NOTIFY

        static const int64_t STORE_BYTES_QUOTA;
        static const char*   STORE_NAME;
        static const char*   STORE_KEY;

        uint32_t doInstall(const JobMeta_t &job);

        uint32_t doInstall(uint32_t taskId, 
                const string& pkgId, const string& type, const string& url,
                const string& token, const string& listener);

    private:
 
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
        std::vector<Exchange::IPackager::INotification*> _notifications;
        std::list<PluginHost::IStateControl::INotification*> _stateControlClients;

        InstallationData _inProgress;
        InstallThread _worker;
        bool _isUpgrade;
        bool _isSyncing;

        uint32_t _taskNumber;

        PackageList_t _packageList;
    };

}  // namespace Plugin
}  // namespace WPEFramework
