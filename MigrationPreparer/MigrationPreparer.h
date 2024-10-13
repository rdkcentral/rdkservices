/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
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
 **/

#pragma once

#include <fstream>
#include <string>
#include <map>
#include <mutex>
#include <cerrno>
#include "secure_wrapper.h"
#include "UtilsJsonRpc.h"
#include "Module.h"

#define DATASTORE_PATH _T("/opt/migration_data_store.json")
#define MIGRATION_PREPARER_RFC_CALLER_ID "MigrationPreparer"
#define TR181_MIGRATION_READY "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Bootstrap.MigrationReady"

typedef uint64_t LINE_NUMBER_TYPE;
using std::string;

namespace WPEFramework {
    namespace Plugin {
        class MigrationPreparer : public PluginHost::IPlugin, public PluginHost::JSONRPC
        {
            private:
                // We do not allow this plugin to be copied !!
                MigrationPreparer(const MigrationPreparer&) = delete;
                MigrationPreparer& operator=(const MigrationPreparer&) = delete;
   
            public:
                MigrationPreparer();
                virtual ~MigrationPreparer();
                static MigrationPreparer* _instance;
                virtual const string Initialize(PluginHost::IShell* service) override;
                virtual void Deinitialize(PluginHost::IShell* service) override;
                virtual string Information() const override;

                BEGIN_INTERFACE_MAP(MigrationPreparer)
                INTERFACE_ENTRY(PluginHost::IPlugin)
                INTERFACE_ENTRY(PluginHost::IDispatcher)
                END_INTERFACE_MAP

                /*Methods: Begin*/
                // DataStore - here represents a JSON File
                // API to write and update dataStore 
                uint32_t endpoint_write(const JsonObject& parameters, JsonObject& response);
                // API to read dataStore entry
                uint32_t endpoint_read(const JsonObject& parameters, JsonObject& response);
                // API to delete dataStore entry
                uint32_t endpoint_delete(const JsonObject& parameters, JsonObject& response);
                // API to set component readiness
                uint32_t setComponentReadiness(const JsonObject& parameters, JsonObject& response);
                // API to get component readiness
                uint32_t getComponentReadiness(const JsonObject& parameters, JsonObject& response);
                /*Methods: End*/
            
            private:
                // A map to hold "Key" vs "Line Number" in the dataStore
                std::map<string, LINE_NUMBER_TYPE> lineNumber;
                // A mutex to protect the dataStore from concurrent read, write and delete access
                std::mutex dataStoreMutex;
                // A tracker for the last key-value line number in the dataStore
                LINE_NUMBER_TYPE curLineIndex;

                /*Helpers: Begin*/
                // Fn. to transform \" to " in a string
                bool Unstringfy(string&);
                // Fn. to get value of a key from the dataStore 
                string getValue(string);
                // Fn. to store the keys and their line numbers from dataStore to lineNumber map
                void storeKeys(void);
                // Fn. to construct JSON response from Vector.
                void setJSONResponseArray(JsonObject& response, const char* key, const std::vector<string>& items);
                /*Helpers: End*/
        };
    }
}