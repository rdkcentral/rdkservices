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

#pragma once

#include "Module.h"
#include <interfaces/json/JsonData_MigrationPreparer.h>
#include <interfaces/json/JMigrationPreparer.h>
#include <interfaces/IMigrationPreparer.h>
#include "UtilsLogging.h"
#include "tracing/Logging.h"
#include <mutex>

namespace WPEFramework {
namespace Plugin {

    class MigrationPreparer: public PluginHost::IPlugin, public PluginHost::JSONRPC
    {
        public:
            // We do not allow this plugin to be copied !!
            MigrationPreparer(const MigrationPreparer&) = delete;
            MigrationPreparer& operator=(const MigrationPreparer&) = delete;

            MigrationPreparer();
            virtual ~MigrationPreparer();

            BEGIN_INTERFACE_MAP(MigrationPreparer)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            INTERFACE_AGGREGATE(Exchange::IMigrationPreparer, _migrationPreparer)
            END_INTERFACE_MAP

            //  IPlugin methods
            // -------------------------------------------------------------------------------------------------------
            const string Initialize(PluginHost::IShell* service) override;
            void Deinitialize(PluginHost::IShell* service) override;
            string Information() const override;

        private:
            void RegisterAll();
            void UnregisterAll();

            uint32_t endpoint_write(const JsonData::MigrationPreparer::WriteentryParamsData& params, JsonData::MigrationPreparer::WriteentryResultInfo& response);
            uint32_t endpoint_read(const JsonData::MigrationPreparer::DeleteentryParamsInfo& params, JsonData::MigrationPreparer::ReadentryResultData& response);
            uint32_t endpoint_delete(const JsonData::MigrationPreparer::DeleteentryParamsInfo& params, JsonData::MigrationPreparer::WriteentryResultInfo& response);
            uint32_t endpoint_getComponentReadiness(JsonData::MigrationPreparer::GetcomponentreadinessResultData& response);
            uint32_t endpoint_setComponentReadiness(const JsonData::MigrationPreparer::SetcomponentreadinessParamsData& params, JsonData::MigrationPreparer::WriteentryResultInfo& response);
            uint32_t endpoint_reset(const JsonData::MigrationPreparer::ResetParamsData& params, JsonData::MigrationPreparer::WriteentryResultInfo& response);
            
        private:
            PluginHost::IShell* _service{};
            uint32_t _connectionId{};
            Exchange::IMigrationPreparer* _migrationPreparer{};
    };

} // namespace Plugin
} // namespace WPEFramework