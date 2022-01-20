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

namespace RdkServicesTest {

class Config {
private:
    class JSONConfig: public WPEFramework::Core::JSON::Container {
    public:
    public:
        JSONConfig(const Config&) = delete;
        JSONConfig& operator=(const Config&) = delete;

        JSONConfig()
                :Prefix(_T("Service")),
                 PersistentPath(), DataPath(), VolatilePath(_T("/tmp")), ProxyStubPath()
        {
            Add(_T("prefix"), &Prefix);
            Add(_T("persistentpath"), &PersistentPath);
            Add(_T("datapath"), &DataPath);
            Add(_T("volatilepath"), &VolatilePath);
            Add(_T("proxystubpath"), &ProxyStubPath);
        }
        ~JSONConfig() override = default;

    public:
        WPEFramework::Core::JSON::String Prefix;
        WPEFramework::Core::JSON::String PersistentPath;
        WPEFramework::Core::JSON::String DataPath;
        WPEFramework::Core::JSON::String VolatilePath;
        WPEFramework::Core::JSON::String ProxyStubPath;
    };

public:
    Config() = delete;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    Config(WPEFramework::Core::File& file, WPEFramework::Core::OptionalType <WPEFramework::Core::JSON::Error>& error)
    {
        JSONConfig config;

        config.IElement::FromFile(file, error);

        if (error.IsSet()==false) {
            _webPrefix = '/'+config.Prefix.Value();
            _volatilePath = WPEFramework::Core::Directory::Normalize(config.VolatilePath.Value());
            _persistentPath = WPEFramework::Core::Directory::Normalize(config.PersistentPath.Value());
            _dataPath = WPEFramework::Core::Directory::Normalize(config.DataPath.Value());
            _proxyStubPath = WPEFramework::Core::Directory::Normalize(config.ProxyStubPath.Value());
        }
    }
    ~Config()
    {
    }

public:
    inline const string& WebPrefix() const
    {
        return (_webPrefix);
    }
    inline const string& VolatilePath() const
    {
        return (_volatilePath);
    }
    inline const string& PersistentPath() const
    {
        return (_persistentPath);
    }
    inline const string& DataPath() const
    {
        return (_dataPath);
    }
    inline const string& ProxyStubPath() const
    {
        return (_proxyStubPath);
    }

private:
    string _webPrefix;
    string _volatilePath;
    string _persistentPath;
    string _dataPath;
    string _proxyStubPath;
};

} // namespace RdkServicesTest
