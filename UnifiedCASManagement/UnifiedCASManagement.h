/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright Synamedia, All rights reserved
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

#ifndef UNIFIEDCASMANAGEMENT_H
#define UNIFIEDCASMANAGEMENT_H

#include "Module.h"
#include "utils.h"
#include <websocket/websocket.h>
#include <interfaces/json/JsonData_UnifiedCASManagement.h>

#include "RTPlayer.h"
#include "UnifiedPlayerNotify.h"

namespace WPEFramework {

    namespace Plugin {
        using namespace JsonData::UnifiedCASManagement;

        class UnifiedCASManagement : public PluginHost::IPlugin, PluginHost::JSONRPC, public WPEFramework::UnifiedPlayerNotify {
        public:
            UnifiedCASManagement();
            UnifiedCASManagement(const UnifiedCASManagement& orig) = delete;
            virtual ~UnifiedCASManagement();

            BEGIN_INTERFACE_MAP(UnifiedCASManagement)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

        public/*members*/:
            //   IPlugin methods
            // -------------------------------------------------------------------------------------------------------
            virtual const string Initialize(PluginHost::IShell *service) override;
            virtual void Deinitialize(PluginHost::IShell *service) override;
            virtual string Information() const override;

            static UnifiedCASManagement* _instance;
        private/*registered methods*/:
            void RegisterAll();
            void UnregisterAll();
            uint32_t endpoint_manage(const JsonData::UnifiedCASManagement::ManagerequestData& params, JsonData::UnifiedCASManagement::ResultInfo& response);
            uint32_t endpoint_unmanage(const Core::JSON::String& params, JsonData::UnifiedCASManagement::ResultInfo& response);
            uint32_t endpoint_send(const JsonData::UnifiedCASManagement::XferinfoInfo& params, JsonData::UnifiedCASManagement::ResultInfo& response);
            void event_data(const string& payload, const JsonData::UnifiedCASManagement::SourceType& source);

            // Event: oncasdata - CAS public data is sent asynchronously from CAS System
            void event_oncasdata(const uint32_t& sessionid, const string& casData) override;
        private/*members*/:
            std::shared_ptr<WPEFramework::RTPlayer>  m_RTPlayer;
	    uint32_t m_sessionId;
        };
    } // namespace Plugin

} // namespace WPEFramework
#endif /* UNIFIEDCASMANAGEMENT_H */

