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
#include <interfaces/IMigrationPreparer.h>
#include "tracing/Logging.h"
#include <fstream>
#include <string>
#include <map>
#include <mutex>
#include <vector>
#include <cerrno>
#include <unordered_set>
#include <list>
#include "secure_wrapper.h"
#include "UtilsJsonRpc.h"
#include "Module.h"
#include "rfcapi.h"


#define MIGRATIONPREPARER_NAMESPACE "MigrationPreparer"
#define DATASTORE_PATH _T("/opt/secure/migration/migration_data_store.json")
#define DATASTORE_DIR _T("/opt/secure/migration/")
#define TR181_MIGRATION_READY "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Bootstrap.MigrationReady"
#define MIGRATIONREADY_PATH _T("/opt/secure/migration/migrationready.txt")
#define MIGRATIONREADY_DIR _T("/opt/secure/migration/")

// PLUGIN SPECIFIC CUSTOM ERROR CODES //bvch
#define ERROR_CREATE    4
#define ERROR_OPEN      5
#define ERROR_WRITE     6
#define ERROR_READ      7
#define ERROR_DELETE    8
#define ERROR_SET       9
#define ERROR_RESET     10
#define ERROR_INVALID   11
#define ERROR_NOFILE    12

typedef uint64_t LINE_NUMBER_TYPE;
using std::string;



namespace WPEFramework {
namespace Plugin {
    class MigrationPreparerImplementation : public Exchange::IMigrationPreparer
    {
        class DateStore2Notification : public Exchange::IMigrationPreparer::INotification {
            private:
                DateStore2Notification(const DateStore2Notification&) = delete;
                DateStore2Notification& operator=(const DateStore2Notification&) = delete;

            public:
                explicit DateStore2Notification(MigrationPreparerImplementation& parent)
                    : _parent(parent)
                {
                }
                ~DateStore2Notification() override = default;
            public:
                void ValueChanged(const string& name, const string& value) override
                {
                    _parent.ValueChanged(name, value);
                }            
            BEGIN_INTERFACE_MAP(DateStore2Notification)
            INTERFACE_ENTRY(Exchange::IMigrationPreparer::INotification)
            END_INTERFACE_MAP

            private:
                MigrationPreparerImplementation& _parent;    
        };

        private:     
            // We do not allow this plugin to be copied !!
            MigrationPreparerImplementation(const MigrationPreparerImplementation&) = delete;
            MigrationPreparerImplementation& operator=(const MigrationPreparerImplementation&) = delete; 

        public:
            MigrationPreparerImplementation();
            ~MigrationPreparerImplementation() override;
            
            /*Methods: Begin*/
            virtual uint32_t Register(Exchange::IMigrationPreparer::INotification *notification ) override ;
            virtual uint32_t Unregister(Exchange::IMigrationPreparer::INotification *notification ) override ;

            // DataStore - here represents a JSON File
            // API to write and update dataStore 
            uint32_t writeEntry(const string& name, const string &value) override;
            // API to delete dataStore entry
            uint32_t deleteEntry(const string& name)  override;
            // API to read dataStore entry
            uint32_t readEntry(const string& name, string &result)  override;
            

            uint32_t setComponentReadiness(const string& compName) override;
            uint32_t getComponentReadiness(RPC::IStringIterator*& compList) override;
            uint32_t reset(const string& resetType) override;
            void ValueChanged(const string& name, const string& value);
            /*Methods: End*/

            BEGIN_INTERFACE_MAP(MigrationPreparerImplementation)
            INTERFACE_ENTRY(Exchange::IMigrationPreparer)
            END_INTERFACE_MAP

    private:
        mutable Core::CriticalSection _adminLock;
        std::list<Exchange::IMigrationPreparer::INotification*> _migrationPreparerNotification;
        
        // A map to hold "Key" vs "Line Number" in the dataStore
        std::map<string, LINE_NUMBER_TYPE> lineNumber;
        // A mutex to protect the dataStore from concurrent read, write and delete access
        std::mutex dataStoreMutex;
        // A tracker for the last key-value line number in the dataStore
        LINE_NUMBER_TYPE curLineIndex;

        std::vector<string> valueEntry;

        /*Helpers: Begin*/
        // Fn. to transform \" to " in a string
        void Unstringfy(string&);
        //escape any special characters in string to make sed compatible pattern string and replacement string
        enum sedType {PATTERN, REPLACEMENT};
        string escapeSed(string, enum sedType);
        // Fn. to store the keys and their line numbers from dataStore to lineNumber map
        void storeKeys(void);
        // Fn. to delete dataStore
        bool resetDatastore(void);
        //Fn. to populate list with values from a string
        void get_components(std::list<string>& list, string& value, string input = "");
        //Fn. to populate value from a list with delimiter(_)
        void tokenize(string& value, std::list<string>& list);

        int8_t split(std::list<string>& list, string& value, std::string delimiter = "_");
        int8_t join(string& value, std::list<string>& list, std::string delimiter = "_");
        bool isDuplicate(string& searchString, std::list<string>& list);
        bool resetMigrationready(void);
        /*Helpers: End*/
    };
} // namespace Plugin
} // namespace WPEFramework