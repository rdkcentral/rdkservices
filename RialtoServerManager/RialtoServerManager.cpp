/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
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

#include "RialtoServerManager.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace {
namespace {
    static const string s_gstPluginsSrcPath = "/usr/lib/gstreamer-1.0";

    static const string s_rialtoDir = "/tmp/rialto";
    static const string s_rialtoGstPluginsSubdir = "/gst";
    static const string s_rialtoLibSubdir = "/lib";
}
}

namespace WPEFramework {
namespace {
    static Plugin::Metadata<Plugin::RialtoServerManager> metadata(
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

    SERVICE_REGISTRATION(RialtoServerManager, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    bool RialtoServerManager::setupRialtoEnvironment()
    {
        if (_config.RialtoDir.IsSet() && !_config.RialtoDir.Value().empty()) {
            _rialtoHome = _config.RialtoDir.Value();
        } else {
            _rialtoHome = s_rialtoDir;
        }

        Core::Directory dir(_rialtoHome.c_str());
        if (!dir.CreatePath())
            return false;

        if (!prepareGstPlugins())
            return false;

        // create addional libs links
        if (_config.Libs.IsSet() && _config.Libs.Length() > 0) {
            const string rialtoLibDir = _rialtoHome + s_rialtoLibSubdir;
            Core::Directory dir(rialtoLibDir.c_str());
            if (!dir.CreatePath()) {
                return false;
            }

            Core::JSON::ArrayType<RialtoServerManagerConfig::Config::FileLink>::Iterator index(_config.Libs.Elements());
            while (index.Next()) {
                Core::File file(index.Current().Src.Value());
                if (!file.Exists())
                    continue;
                string target = rialtoLibDir + "/" + index.Current().Src.Value();
                // if "dest" is set, use it as target
                if (index.Current().Dest.IsSet() && !index.Current().Dest.Value().empty()) {
                    target = rialtoLibDir + "/" + index.Current().Dest.Value();
                }
                file.Link(target);
            }
        }

        auto envs = RialtoServerManagerConfig::parseEnvs(_config.HostEnvVars);
        for (auto& var : envs) {
            string value;
            bool isSet = Core::SystemInfo::GetEnvironment(var.first, value);
            _environment.push_back({var.first, isSet ? Core::OptionalType<string>(value) : Core::OptionalType<string>()});

            Core::SystemInfo::SetEnvironment(var.first, var.second);
        }
        return true;
    }

    void RialtoServerManager::restoreEnvironment()
    {
        Core::File file(_rialtoHome);
        Core::Directory dir(file.Name().c_str());
        dir.Destroy();
        // destroy top dir
        file.Destroy();

        // restore previous environment
        for (auto& var : _environment) {
            if (var.second.IsSet()) {
                Core::SystemInfo::SetEnvironment(var.first, var.second.Value());
            } else {
                Core::SystemInfo::SetEnvironment(var.first, nullptr);
            }
        }
    }

    bool RialtoServerManager::prepareGstPlugins() {
        if (!_config.GstPlugins.IsSet() || _config.GstPlugins.Length() == 0) {
            return true;
        }
        // create soft links /tmp/rialto/* /usr/lib/gstreamer-1.0/*
        const string rialtoGstPluginsDir = _rialtoHome + s_rialtoGstPluginsSubdir;
        Core::Directory dir(rialtoGstPluginsDir.c_str());
        if (!dir.CreatePath()) {
            return false;
        }

        Core::JSON::ArrayType<Core::JSON::String>::Iterator index(_config.GstPlugins.Elements());
        while (index.Next()) {
            Core::File file(s_gstPluginsSrcPath + "/" + index.Current().Value());
            if (!file.Exists())
                continue;
            file.Link(rialtoGstPluginsDir + "/" + index.Current().Value());
        }
        return true;
    }

    void RialtoServerManager::cleanUp(bool releaseObject)
    {
        if (releaseObject) {
            if (_object != nullptr) {
                _object->Release();
            }
        }
        _object = nullptr;
        _service->Unregister(&_notification);
        _service = nullptr;
    }

    /* virtual */ const string RialtoServerManager::Initialize(PluginHost::IShell* service)
    {
        string result;

        ASSERT(_service == nullptr);

        _connectionId = 0;

        if (!_config.FromString(service->ConfigLine())) {
            result = _T("Failed to parse config line");
            return result;
        }

        _service = service;

        // Register the Process::Notification stuff. The Remote process might die before we get a
        // change to "register" the sink for these events !!! So do it ahead of instantiation.
        _service->Register(&_notification);

        _object = _service->Root<Exchange::IConfiguration>(_connectionId, WPEFramework::RPC::CommunicationTimeOut, _T("RialtoServerManagerImpl"));
        ASSERT(_connectionId != 0);

        if (_object == nullptr) {
            result = _T("RialtoServerManager could not be instantiated.");
            cleanUp(false);
            return result;
        }

        if (_object->Configure(_service) != 0) {
            result = _T("Failed to configure RialtoServerManager.");
            cleanUp(true);
            return result;
        }

        if (!setupRialtoEnvironment()) {
            result = _T("Failed to setup Rialto environment.");
            return result;
        }

        TRACE(Trace::Information, (_T("RialtoServerManager is running")));
        return result;
    }

    /*virtual*/ void RialtoServerManager::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);
        ASSERT(_object != nullptr);

        _service->Unregister(&_notification);

        RPC::IRemoteConnection* connection(_service->RemoteConnection(_connectionId));
        uint32_t result = _object->Release();
        ASSERT(result == Core::ERROR_DESTRUCTION_SUCCEEDED);

        if (connection != nullptr) {
            connection->Terminate();
            connection->Release();
        }

        // Deinitialize what we initialized..
        _object = nullptr;
        _service = nullptr;

        restoreEnvironment();
        TRACE(Trace::Information, (_T("RialtoServerManager is closed")));
    }

    /* virtual */ string RialtoServerManager::Information() const
    {
        // No additional info to report.
        return (nullptr);
    }

    void RialtoServerManager::Deactivated(RPC::IRemoteConnection* connection)
    {
        if (connection->Id() == _connectionId) {

            ASSERT(_service != nullptr);

            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service,
                PluginHost::IShell::DEACTIVATED,
                PluginHost::IShell::FAILURE));
        }
    }
}
} //namespace WPEFramework::Plugin
