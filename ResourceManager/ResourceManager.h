/**
 * * If not stated otherwise in this file or this component's LICENSE
 * * file the following copyright and licenses apply:
 * *
 * * Copyright 2020 RDK Management
 * *
 * * Licensed under the Apache License, Version 2.0 (the "License");
 * * you may not use this file except in compliance with the License.
 * * You may obtain a copy of the License at
 * *
 * * http://www.apache.org/licenses/LICENSE-2.0
 * *
 * * Unless required by applicable law or agreed to in writing, software
 * * distributed under the License is distributed on an "AS IS" BASIS,
 * * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * * See the License for the specific language governing permissions and
 * * limitations under the License.
 * **/

#pragma once

#include "Module.h"
#ifdef ENABLE_ERM
#include <map>
#include <essos-resmgr.h>
#endif
#include <string>
#include <iostream>

#include "UtilsJsonRpc.h"

namespace WPEFramework {

    namespace Plugin {

        class ResourceManager :  public PluginHost::IPlugin, public PluginHost::JSONRPC {
        public:
            ResourceManager();
            virtual ~ResourceManager();

            BEGIN_INTERFACE_MAP(ResourceManager)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

            // IPlugin methods
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override;

        public/*members*/:
            static ResourceManager* _instance;

        public /*constants*/:
            static const string SERVICE_NAME;
            //methods
            static const string RESOURCE_MANAGER_METHOD_SET_AV_BLOCKED;
            static const string RESOURCE_MANAGER_METHOD_GET_BLOCKED_AV_APPLICATIONS;
            static const string RESOURCE_MANAGER_METHOD_RESERVE_TTS_RESOURCE;

        private/*registered methods (wrappers)*/:
            //methods ("parameters" here is "params" from the curl request)
            uint32_t setAVBlockedWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getBlockedAVApplicationsWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t reserveTTSResourceWrapper(const JsonObject& parameters, JsonObject& response);

        private/*internal methods*/:
            ResourceManager(const ResourceManager&) = delete;
            ResourceManager& operator=(const ResourceManager&) = delete;

            bool setAVBlocked(const string& client, const bool blocked);
            bool getBlockedAVApplications(std::vector<std::string> &appsList);
            bool reserveTTSResource(const string& client);

            EssRMgr* mEssRMgr;
            bool mDisableBlacklist;
            bool mDisableReserveTTS;
            PluginHost::IShell* mCurrentService;
            std::map<std::string, bool> mAppsAVBlacklistStatus;

        };

    } // namespace Plugin
} // namespace WPEFramework
