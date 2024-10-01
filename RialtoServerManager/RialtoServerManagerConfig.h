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

#ifndef __RIALTO_SERVER_MANAGER_CONFIG_H
#define __RIALTO_SERVER_MANAGER_CONFIG_H

#include "Module.h"

namespace WPEFramework {
namespace Plugin {
namespace RialtoServerManagerConfig {

    static std::vector<std::pair<string, string>> parseEnvs(const Core::JSON::String& envs) {
        if (!envs.IsSet() || envs.Value().empty())
            return {};

        string envsStr = envs.Value();
        std::vector<std::pair<string, string>> result;

        size_t pos = 0;
        while ((pos = envsStr.find(";")) != std::string::npos)
        {
            string singleEnvStr = envsStr.substr(0, pos);
            size_t equalPos = singleEnvStr.find("=");
            if (equalPos != std::string::npos) {
                string name = singleEnvStr.substr(0, equalPos);
                string value = singleEnvStr.substr(equalPos + 1);
                result.push_back({name, value});
            }
            envsStr.erase(0, pos + 1);
        }
        return result;
    }

    class Config : public Core::JSON::Container {
    private:
        Config(const Config&) = delete;
        Config& operator=(const Config&) = delete;
    public:
        class FileLink : public Core::JSON::Container {
            public:
                FileLink(const FileLink& origin)
                    : Core::JSON::Container()
                    , Src(origin.Src)
                    , Dest(origin.Dest)
                {
                    Add(_T("src"), &Src);
                    Add(_T("dest"), &Dest);
                }
                FileLink& operator=(const FileLink&) = delete;

                FileLink()
                    : Core::JSON::Container()
                    , Src("")
                    , Dest("")
                {
                    Add(_T("src"), &Src);
                    Add(_T("dest"), &Dest);
                }
                ~FileLink() = default;

            public:
                Core::JSON::String Src;
                Core::JSON::String Dest;
            };

        public:
            Config()
                : Core::JSON::Container()
                , RialtoDir()
                , GstPlugins()
                , Libs()
                , HostEnvVars()
                , SessionEnvs()
                , SessionPlatformEnvs()
            {
                Add(_T("rialtodir"), &RialtoDir);
                Add(_T("gstplugins"), &GstPlugins);
                Add(_T("libs"), &Libs);
                Add(_T("hostenvvars"), &HostEnvVars);

                Add(_T("sessionenvs"), &SessionEnvs);
                Add(_T("sessionplatformenvs"), &SessionPlatformEnvs);
            }
            ~Config()
            {
            }

        public:
            Core::JSON::String RialtoDir;
            Core::JSON::ArrayType<Core::JSON::String> GstPlugins;
            Core::JSON::ArrayType<FileLink> Libs;
            Core::JSON::String HostEnvVars;

            Core::JSON::String SessionEnvs;
            Core::JSON::String SessionPlatformEnvs;
        };

} // namespace RialtoServerManagerConfig
} // namespace Plugin
} // namespace WPEFramework

#endif // __RIALTO_SERVER_MANAGER_CONFIG_H