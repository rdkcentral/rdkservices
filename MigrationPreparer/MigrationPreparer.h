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
#include "UtilsJsonRpc.h"
#include <mutex>

namespace WPEFramework {
namespace Plugin {

    class MigrationPreparer: public PluginHost::IPlugin, public PluginHost::JSONRPC
    {
        private:
            class Notification : public RPC::IRemoteConnection::INotification,
                                public Exchange::IMigrationPreparer::INotification
            {
                private:
                    Notification() = delete;
                    Notification(const Notification&) = delete;
                    Notification& operator=(const Notification&) = delete;

                public:
                explicit Notification(MigrationPreparer* parent)
                    : _parent(*parent)
                    {
                        ASSERT(parent != nullptr);
                    }

                    virtual ~Notification()
                    {
                    }

                    BEGIN_INTERFACE_MAP(Notification)
                    INTERFACE_ENTRY(Exchange::IMigrationPreparer::INotification)
                    INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
                    END_INTERFACE_MAP

                    void Activated(RPC::IRemoteConnection*) override
                    {
                        LOGINFO("MigrationPreparer Notification Activated");
                    }

                    void Deactivated(RPC::IRemoteConnection *connection) override
                    {
                    LOGINFO("MigrationPreparer Notification Deactivated");
                    _parent.Deactivated(connection);
                    }

                    void ValueChanged(const string& name, const string& value) override
                    {
                        JsonData::MigrationPreparer::WriteentryParamsData params;
                        params.Name = name;
                        params.Value = value;

                        _parent.event_onValueChanged(params);
                    } 

                private:
                    MigrationPreparer& _parent;
            };         
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

            void Activated(RPC::IRemoteConnection* connection);
            void Deactivated(RPC::IRemoteConnection* connection);

            uint32_t endpoint_write(const JsonObject& parameters, JsonObject& response);
            uint32_t endpoint_read(const JsonObject& parameters, JsonObject& response);
            uint32_t endpoint_delete(const JsonObject& parameters, JsonObject& response);
            uint32_t endpoint_getComponentReadiness(JsonData::MigrationPreparer::GetcomponentreadinessResultData& response);
            uint32_t endpoint_setComponentReadiness(const JsonData::MigrationPreparer::SetcomponentreadinessParamsData& params, JsonData::MigrationPreparer::WriteentryResultInfo& response);
            uint32_t endpoint_reset(const JsonData::MigrationPreparer::ResetParamsData& params, JsonData::MigrationPreparer::WriteentryResultInfo& response);
            void event_onValueChanged(const JsonData::MigrationPreparer::WriteentryParamsData& params)
            {
                Notify(_T("onValueChanged"), params);
            }               
        private:
            PluginHost::IShell* _service{};
            uint32_t _connectionId{};
            Exchange::IMigrationPreparer* _migrationPreparer{};
            Core::Sink<Notification> _migrationPreparerNotification;
    };

} // namespace Plugin
} // namespace WPEFramework