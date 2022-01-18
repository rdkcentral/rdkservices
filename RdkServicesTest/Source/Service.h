/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/

#pragma once

#include "Module.h"

#include "Config.h"
#include "SystemInfo.h"

namespace RdkServicesTest {

class Service: public WPEFramework::PluginHost::IShell {
private:
    class Config {
    public:
        Config() = delete;
        Config(const Config&) = delete;
        Config& operator=(const Config&) = delete;

        Config(const WPEFramework::Plugin::Config& plugin, const string& webPrefix,
                const string& persistentPath, const string& dataPath, const string& volatilePath)
        {
            const string& callSign(plugin.Callsign.Value());

            _webPrefix = webPrefix+'/'+callSign;
            _persistentPath = plugin.PersistentPath(persistentPath);
            _dataPath = plugin.DataPath(dataPath);
            _volatilePath = plugin.VolatilePath(volatilePath);

            // Volatile means that the path could not have been created, create it for now.
            WPEFramework::Core::Directory(_volatilePath.c_str()).CreatePath();

            Update(plugin);
        }
        ~Config()
        {
        }

    public:
        inline bool IsSupported(const uint8_t number) const
        {
            return (std::find(_versions.begin(), _versions.end(), number)!=_versions.end());
        }
        inline void Configuration(const string& value)
        {
            _config.Configuration = value;
        }
        inline void AutoStart(const bool value)
        {
            _config.AutoStart = value;
        }
        inline const WPEFramework::Plugin::Config& Configuration() const
        {
            return (_config);
        }

        inline const string& WebPrefix() const
        {
            return (_webPrefix);
        }

        inline const string& PersistentPath() const
        {
            return (_persistentPath);
        }

        inline const string& VolatilePath() const
        {
            return (_volatilePath);
        }

        inline const string& DataPath() const
        {
            return (_dataPath);
        }

        inline void Update(const WPEFramework::Plugin::Config& config)
        {
            _config = config;

            _versions.clear();
            if (_versions.empty()==true) {
                _versions.push_back(1);
            }
        }

    private:
        WPEFramework::Plugin::Config _config;

        string _webPrefix;
        string _persistentPath;
        string _volatilePath;
        string _dataPath;
        string _accessor;
        std::list <uint8_t> _versions;
    };

public:
    Service() = delete;
    Service(const Service&) = delete;
    Service& operator=(const Service&) = delete;

    Service(const RdkServicesTest::Config& server, const WPEFramework::Plugin::Config& plugin)
            :_state(DEACTIVATED),
             _config(plugin, server.WebPrefix(), server.PersistentPath(), server.DataPath(), server.VolatilePath())
    {
        if ((plugin.Startup.IsSet()==true) && (plugin.Startup.Value()==WPEFramework::Plugin::Config::UNAVAILABLE)) {
            _state = UNAVAILABLE;
        }
    }
    ~Service() override = default;

public:
    string Versions() const override
    {
        return (_config.Configuration().Versions.Value());
    }
    uint32_t StartupOrder() const
    {
        return (_config.Configuration().StartupOrder.Value());
    }
    string Locator() const override
    {
        return (_config.Configuration().Locator.Value());
    }
    string ClassName() const override
    {
        return (_config.Configuration().ClassName.Value());
    }
    string Callsign() const override
    {
        return (_config.Configuration().Callsign.Value());
    }
    string WebPrefix() const override
    {
        return (_config.WebPrefix());
    }
    string ConfigLine() const override
    {
        return (_config.Configuration().Configuration.Value());
    }
    string PersistentPath() const override
    {
        return (_config.PersistentPath());
    }
    string VolatilePath() const override
    {
        return (_config.VolatilePath());
    }
    string DataPath() const override
    {
        return (_config.DataPath());
    }
    state State() const override
    {
        return (_state);
    }
    bool AutoStart() const override
    {
        bool result = _config.Configuration().AutoStart.Value();

        if (_config.Configuration().Startup.IsSet()==true) {
            WPEFramework::Plugin::Config::startup value = _config.Configuration().Startup.Value();
            result = (value==WPEFramework::Plugin::Config::startup::SUSPENDED)
                    || (value==WPEFramework::Plugin::Config::startup::RESUMED);
        }

        return (result);
    }
    bool Resumed() const override
    {
        bool result = (_config.Configuration().Resumed.IsSet() ? _config.Configuration().Resumed.Value() : (
                _config.Configuration().AutoStart.Value()==false));

        if (_config.Configuration().Startup.IsSet()==true) {
            result = (_config.Configuration().Startup.Value()==WPEFramework::Plugin::Config::startup::RESUMED);
        }

        return (result);
    }
    bool IsSupported(const uint8_t number) const override
    {
        return (_config.IsSupported(number));
    }

    void EnableWebServer(const string& postFixURL, const string& fileRootPath) override
    {
    }
    void DisableWebServer() override
    {
    }

public:
    WPEFramework::PluginHost::ISubSystem* SubSystems() override
    {
        return (&subSystem);
    }
    uint32_t Submit(const uint32_t id, const WPEFramework::Core::ProxyType <WPEFramework::Core::JSON::IElement>& response) override
    {
        return (WPEFramework::Core::ERROR_NONE);
    }
    void Notify(const string& message) override { }
    void* QueryInterface(const uint32_t id) override
    {
        return (nullptr);
    }
    void* QueryInterfaceByCallsign(const uint32_t id, const string& name) override
    {
        return (nullptr);
    }
    void Register(WPEFramework::PluginHost::IPlugin::INotification* sink) override { }
    void Unregister(WPEFramework::PluginHost::IPlugin::INotification* sink) override { }
    string Version() const override
    {
        return (string());
    }
    string Model() const override
    {
        return (string());
    }
    bool Background() const override
    {
        return (false);
    }
    string Accessor() const override
    {
        return (string());
    }
    string ProxyStubPath() const override
    {
        return (string());
    }
    string HashKey() const override
    {
        return (string());
    }
    string Substitute(const string& input) const override
    {
        return (string());
    }
    bool PostMortemAllowed(WPEFramework::PluginHost::IShell::reason why) const
    {
        return (false);
    }
    WPEFramework::PluginHost::IShell::ICOMLink* COMLink() override
    {
        return (nullptr);
    }
    uint32_t Activate(const reason) override { return (WPEFramework::Core::ERROR_NONE); }
    uint32_t Deactivate(const reason) override { return (WPEFramework::Core::ERROR_NONE); }
    uint32_t Unavailable(const reason) override { return (WPEFramework::Core::ERROR_NONE); }
    reason Reason() const override
    {
        return (_reason);
    }
private:
    state _state;
    Config _config;
    reason _reason;
    WPEFramework::Core::Sink<SystemInfo> subSystem;
};

} // namespace RdkServicesTest
