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

#include "MigrationPreparerImplementation.h"

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(MigrationPreparerImplementation, 1, 0);

    MigrationPreparerImplementation::MigrationPreparerImplementation()
    : _adminLock()
    {
        LOGINFO("Create MigrationPreparerImplementation Instance");
         // Init
        WPEFramework::Core::File dataStore(DATASTORE_PATH);
        if(dataStore.Exists())
            storeKeys();  
    }

    MigrationPreparerImplementation::~MigrationPreparerImplementation()
    {
    }

    uint32_t MigrationPreparerImplementation::Register(Exchange::IMigrationPreparer::INotification *notification)
    {
        ASSERT (nullptr != notification);

        _adminLock.Lock();

        // Make sure we can't register the same notification callback multiple times
        if (std::find(_migrationPreparerNotification.begin(), _migrationPreparerNotification.end(), notification) == _migrationPreparerNotification.end())
        {
            LOGINFO("Register notification");
            _migrationPreparerNotification.push_back(notification);
            notification->AddRef();
        }

        _adminLock.Unlock();

        return Core::ERROR_NONE;
    }

    uint32_t MigrationPreparerImplementation::Unregister(Exchange::IMigrationPreparer::INotification *notification )
    {
        uint32_t status = Core::ERROR_GENERAL;

        ASSERT (nullptr != notification);

        _adminLock.Lock();

        // Make sure we can't unregister the same notification callback multiple times
        auto itr = std::find(_migrationPreparerNotification.begin(), _migrationPreparerNotification.end(), notification);
        if (itr != _migrationPreparerNotification.end())
        {
            (*itr)->Release();
            LOGINFO("Unregister notification");
            _migrationPreparerNotification.erase(itr);
            status = Core::ERROR_NONE;
        }
        else
        {
            LOGERR("notification not found");
        }

        _adminLock.Unlock();

        return status;
    }

    void MigrationPreparerImplementation::ValueChanged(const string &name, const string &value)
    {
         LOGINFO("name:%s value:%s",name.c_str(), value.c_str());
         _adminLock.Lock();
         std::list<Exchange::IMigrationPreparer::INotification*>::const_iterator index(_migrationPreparerNotification.begin());
         while (index != _migrationPreparerNotification.end())
         {
            (*index)->ValueChanged(name.c_str(), value.c_str());
            index++;
         }
         _adminLock.Unlock();         
    }

    /*Helper's: Begin*/
    void MigrationPreparerImplementation::Unstringfy(string& input) {
        // transform \" to "
        string::size_type pos = 0;
        while ((pos = input.find("\\\"")) != string::npos) {
            input.replace(pos, 2, "\"");
        }
    }

    #ifdef MEMORY_OPTIMIZED
    string MigrationPreparerImplementation::getValue(string key) {
        dataStoreMutex.lock();
        
        std::ifstream inputFile(DATASTORE_PATH);
        string line, value;
        size_t start, end, colPos;
        LINE_NUMBER_TYPE lineIndex = 0;
        LINE_NUMBER_TYPE stop = lineNumber[key];
        // Get the requested line
        while(std::getline(inputFile, line)) 
            if(++lineIndex == stop)
                break;
        // trim white space, new line and comma from the line
        start = line.find_first_not_of(" \n");
        end = line.find_last_not_of(" \n,");
        line = line.substr(start, end-start+1);
        
        // line should not be empty or { or } if so assert
        ASSERT(line.empty());

        // below is the dataStore json format
        // {
        // <space>"key1":value1,
        // <space>"key2":value2    
        // }
        
        // get the value from line
        colPos = line.find(":");
        value = line.substr(colPos+1);
        dataStoreMutex.unlock();
        return value;
    }
    #endif

    string MigrationPreparerImplementation::escapeSed(string input, enum sedType type) {
        // Set of special characters in sed that need to be escaped
        std::unordered_set<char> specialChars = {
            '.', '*', '+', '?', '^', '$', '(', ')', '[', ']', '{', '}', '\\', '|', '/', '&'
        };

        std::string escapedString;

        if(type == PATTERN) {
            // Iterate through each character in the string
            for(char ch : input) {
                // If ch is a special character replace with .
                if (specialChars.find(ch) != specialChars.end())
                    escapedString += ".";
                else
                    escapedString += ch;
            }
        }
        else if(type == REPLACEMENT) {
            // Iterate through each character in the string
            for(char ch : input) {
                // If ch is a special character prepend it with a backslash
                if (specialChars.find(ch) != specialChars.end())
                    escapedString += "\\";

                escapedString += ch;
            }
        }
        return escapedString;
    }

    void MigrationPreparerImplementation::storeKeys(void) {
        dataStoreMutex.lock();
        std::ifstream inputFile(DATASTORE_PATH);
        string key, line;
        size_t start, end, colPos;
        LINE_NUMBER_TYPE lineIndex = 1;
        while(std::getline(inputFile, line)) {
            // trim white space, new line and comma from the line
            start = line.find_first_not_of(" \n");
            end = line.find_last_not_of(" \n,");
            line =  line.substr(start, end-start+1);
            // if line is empty or { or } continue
            if(line.empty() || line == "{" || line == "}")
		        continue;
            
            // increment line index
            lineIndex++;
            //extract key
            // below is the dataStore json format
            // {
            // <space>"key1":value1,
            // <space>"key2":value2    
            // }
            colPos = line.find(":");
            key = line.substr(1,colPos-2);
            
            //add key to the map
            lineNumber[key] = lineIndex;

            #ifdef CPU_OPTIMIZED
            string value = line.substr(colPos+1);
            valueEntry.push_back(value);
            #endif
        }
        // update the last line index
        curLineIndex = lineIndex;
        dataStoreMutex.unlock();
    }


    bool MigrationPreparerImplementation::resetDatastore(void){
        WPEFramework::Core::File dataStore(DATASTORE_PATH);
        if(!dataStore.Exists()) {
            LOGERR("DataStore file does not exist");
            return false;
        }
        dataStoreMutex.lock();
        // remove dataStore file
        LOGWARN("Deleting dataStore file itself since all entries are deleted");
        if(!dataStore.Destroy()){
            LOGERR("Unable to delete dataStore file");
            return false;
        }
        // clear the lineNumber map and valueEntry vector internal data structures
        if (!lineNumber.empty())
            lineNumber.clear();
        curLineIndex = 1;
        #ifdef CPU_OPTIMIZED
         if (!valueEntry.empty())
            valueEntry.clear();
        #endif
        dataStoreMutex.unlock();
        return true;
    }

     void MigrationPreparerImplementation::get_components(std::list<string>& list, string& value, string input) {
            LOGINFO("get_components: %s and input: %s", value.c_str(), input.c_str());
            string::size_type start = 0, pos = 0; 
            while ((pos = value.find('_', start) )!= std::string::npos) {
                list.emplace_back(value.substr(start, pos - start));
                start = pos + 1; // Move start to the next character after the delimiter
            }
            // add the last element or the only element to the list 
            list.emplace_back(value.substr(start));
            if(input != "") 
                if (find(list.begin(), list.end(), input) == list.end()) // duplicate check
                    list.emplace_back(input);
            list.sort();
        }

        void MigrationPreparerImplementation::tokenize(string& value, std::list<string>& list) {
        	//if the list is with only one element also it won't be a problem
            if (list.empty()) return;  // Handle empty list case
            auto it = list.begin();
            // Use the for loop to concatenate elements with '_'
            for (; std::next(it) != list.end(); ++it) {
                value += *it + "_";  // Concatenate element with underscore
            }
            // Add the last element without an underscore
            value += *it;
	    }
    
    /*Helper's: End*/

    uint32_t MigrationPreparerImplementation::writeEntry(const string& name, const string &value) {

        LOGINFO("[WRITE] params={name: %s, value: %s}", name.c_str(), value.c_str());
        string entry;
        string key = name;
        string newValue = value;
        WPEFramework::Core::File dataStore(DATASTORE_PATH);
        WPEFramework::Core::Directory dataStoreDir(DATASTORE_DIR);
        Unstringfy(newValue);

        // check if someone deletes the dataStore in the middle of the operation
        if(!lineNumber.empty() && !dataStoreDir.Exists()) {
            LOGWARN("Migration datastore %s deleted in the middle of the operation, resetting the internal data structures", DATASTORE_PATH);
            lineNumber.clear();
            curLineIndex = 1;
            #ifdef CPU_OPTIMIZED
            if (!valueEntry.empty())
                valueEntry.clear();
            #endif
        }
        
        dataStoreMutex.lock();
        // Handle first Write request
        if (lineNumber.empty()) {
            curLineIndex = 1;
            if(!dataStore.Exists()) {
                // check if the directory exist
                if(!dataStoreDir.Exists())
                    // assuming /opt/secure path already exist and "migration" folder needs to be under /opt/secure/
                    dataStoreDir.Create();
                
                if(!dataStore.Create()) {
                    LOGERR("Failed to create migration datastore %s, errno: %d, reason: %s", DATASTORE_PATH, errno, strerror(errno));
                    dataStoreMutex.unlock();
                    return Core::ERROR_GENERAL;
                }
            }
            
            if(!dataStore.Open(false)) {
                LOGERR("Failed to open migration datastore %s, errno: %d, reason: %s", DATASTORE_PATH, errno, strerror(errno));
                LOGERR("Failed to create entry for {%s:%s} in migration datastore", key.c_str(), newValue.c_str());
                dataStoreMutex.unlock();
                return Core::ERROR_GENERAL;
            }
            // write entry to the dataStore
            entry = string("{\n \"") + key + string("\":") + newValue + string("\n}");
            dataStore.Write(reinterpret_cast<const uint8_t*>(&entry[0]), entry.size());
            // update the entry in lineNumber map
            lineNumber[key] = ++curLineIndex;
            dataStore.Close();
            dataStoreMutex.unlock();
            return Core::ERROR_NONE;
        }

        // Handle Update Request
        if(lineNumber.find(key) != lineNumber.end()) {

            #ifdef MEMORY_OPTIMIZED
            dataStoreMutex.unlock();
            string oldValue = getValue(key);
            dataStoreMutex.lock();
            #else
            string oldValue = valueEntry[lineNumber[key] - 1];
            #endif

            if(oldValue == newValue) {
                LOGWARN("Entry {%s:%s} is already existing in the dataStore, returning success", oldValue.c_str(), key.c_str());
                dataStoreMutex.unlock();
                return Core::ERROR_NONE;
            }
            
            // sed command to replace value in the dataStore
            oldValue = escapeSed(oldValue, PATTERN);
            newValue = escapeSed(newValue, REPLACEMENT);
            int result = v_secure_system("/bin/sed -i -E '%ss/%s/%s/' %s", std::to_string(lineNumber[key]).c_str(), oldValue.c_str(), newValue.c_str(), DATASTORE_PATH);
            
            if (result != -1 && WIFEXITED(result)) {
                #ifdef CPU_OPTIMIZED
                valueEntry[lineNumber[key] - 2] = newValue;
                #endif
                dataStoreMutex.unlock();
                return Core::ERROR_NONE;
            }
            result = WEXITSTATUS(result);
            LOGERR("Failed to update entry for {%s:%s} in migration datastore, v_secure_system failed with error %d",key.c_str(), newValue.c_str(), result);
            dataStoreMutex.unlock();
            return Core::ERROR_GENERAL;
        }

        // Handle subsequent Write request
        if(!dataStore.Open(false)) {
            LOGERR("Failed to create migration datastore %s, errno: %d, reason: %s", DATASTORE_PATH, errno, strerror(errno));
            LOGERR("Failed to create entry for {%s:%s} in migration datastore", key.c_str(), newValue.c_str());
            dataStoreMutex.unlock();
            return Core::ERROR_GENERAL;
        }

        // append new key-value pair to the dataStore
        if(!dataStore.Position(false, dataStore.Size() - 2)) {
            LOGERR("DataStore truncate failed with errno: %d, reason: %s\n", errno, strerror(errno));
            LOGERR("Failed to create entry for {%s:%s} in migration datastore", key.c_str(), newValue.c_str());
            dataStore.Close();
            dataStoreMutex.unlock();
            return Core::ERROR_GENERAL;
        }
        entry = string(",\n \"") + key + string("\":") + newValue + string("\n}"); 
        dataStore.Write(reinterpret_cast<const uint8_t*>(&entry[0]), entry.size());
        lineNumber[key] = ++curLineIndex;
        #ifdef CPU_OPTIMIZED
        valueEntry.push_back(newValue);
        #endif
        dataStore.Close();
        dataStoreMutex.unlock();
        return Core::ERROR_NONE;
    }
    
    uint32_t MigrationPreparerImplementation::readEntry(const string& name, string &result) {

        LOGINFO("[READ] params={name: %s}", name.c_str());
        string key = name;

        // check if list is not empty and lineNumber for given key exists
        if(lineNumber.empty() || (lineNumber.find(key) == lineNumber.end())) {
            LOGERR("Failed to read key: %s, Key do not exist in migration dataStore", key.c_str());
            return Core::ERROR_GENERAL;
        }
        
        #ifdef MEMORY_OPTIMIZED
        result = getValue(key);
        #else
        result = valueEntry[lineNumber[key] - 2];
        #endif
        return Core::ERROR_NONE;
    }

    uint32_t MigrationPreparerImplementation::deleteEntry(const string& name) {

        LOGINFO("[DELETE] params={name: %s}", name.c_str());
        string key = name;
        int result;

        dataStoreMutex.lock();
        if(!lineNumber.empty() && (lineNumber.find(key) != lineNumber.end())) {
            // sed command to delete an key-value entry in the dataStore            
            result = v_secure_system("/bin/sed -i '%sd' %s", std::to_string(lineNumber[key]).c_str(), DATASTORE_PATH);

            if (result != -1 && WIFEXITED(result)) {
                // check if last entry is deleted
                if(lineNumber[key] == curLineIndex) {
                    WPEFramework::Core::File dataStore(DATASTORE_PATH);
                    dataStore.Append();
                    // if last entry is deleted remove comma from the previous line
                    dataStore.Position(false, dataStore.Size());
                    if(!dataStore.SetSize(dataStore.Size() - 3)) {
                        LOGERR("DataStore truncate failed with errno: %d, reason: %s\n", errno, strerror(errno));
                    }
                    string entry = "\n}"; 
                    dataStore.Write(reinterpret_cast<const uint8_t*>(&entry[0]), entry.size()); 
                    dataStore.Close();             
                }
                // Adjust line line number for other entries
                for (auto it = lineNumber.begin(); it != lineNumber.end(); ++it) {
                    if(it->second > lineNumber[key]) {
                        it->second--;
                    }
                }
                #ifdef CPU_OPTIMIZED
                valueEntry.erase(lineNumber[key] - 2);
                #endif
                // remove key from the lineNumber map
                lineNumber.erase(key);
                curLineIndex--;
                dataStoreMutex.unlock();
                return Core::ERROR_NONE;
            }
            LOGERR("Failed to delete entry for key: %s in migration datastore, v_secure_system failed with error %d",key.c_str(), result);
        }

        result = WEXITSTATUS(result);
        LOGERR("Failed to delete entry for key: %s in migration datastore, Key does not exist in dataStore",key.c_str());
        dataStoreMutex.unlock();
        return Core::ERROR_GENERAL;
    }

    
    uint32_t MigrationPreparerImplementation::setComponentReadiness(const string& compName)
    {
        uint32_t status = Core::ERROR_GENERAL;
        string _compName = compName;
        _adminLock.Lock();
        LOGINFO("Component Name: %s", _compName.c_str());
        RFC_ParamData_t param;
        WDMP_STATUS wdmpStatus;
        wdmpStatus = getRFCParameter((char *)MIGRATIONPREPARER_NAMESPACE,TR181_MIGRATION_READY, &param);
        if (wdmpStatus != WDMP_SUCCESS) {
            LOGINFO("setting the first RFC parameter");
            wdmpStatus = setRFCParameter((char *)MIGRATIONPREPARER_NAMESPACE, TR181_MIGRATION_READY, _compName.c_str(), WDMP_STRING);  
            status=(wdmpStatus == WDMP_SUCCESS)?Core::ERROR_NONE:Core::ERROR_GENERAL;
        }
        else {
              std::string paramValue;
              paramValue = param.value;
              LOGINFO("component %s is already present", paramValue.c_str());
              std::list<std::string> component_list;
              get_components(component_list, paramValue, _compName);
              _compName= "";
              tokenize(_compName, component_list);
              wdmpStatus = setRFCParameter((char *)MIGRATIONPREPARER_NAMESPACE, TR181_MIGRATION_READY, _compName.c_str(), WDMP_STRING);  
              status=(wdmpStatus == WDMP_SUCCESS)?Core::ERROR_NONE:Core::ERROR_GENERAL;
        }
        _adminLock.Unlock();
        return status;
    }
    
    
    uint32_t MigrationPreparerImplementation::getComponentReadiness(RPC::IStringIterator*& compList)
    {
        uint32_t status = Core::ERROR_GENERAL;
        _adminLock.Lock();
        RFC_ParamData_t param;
        std::list<string> components;
        //getting the RFC parameter to check if any component is ready for migration
        WDMP_STATUS wdmpStatus = getRFCParameter((char *)MIGRATIONPREPARER_NAMESPACE, TR181_MIGRATION_READY, &param);
        if (wdmpStatus != WDMP_SUCCESS) {
                LOGINFO("No component is ready for migration");
                status = Core::ERROR_NONE;
        }
        else{
            //if one or more component is ready then it gets the list of components
            string paramValue;
            paramValue = param.value;
            get_components(components, paramValue);
            LOGINFO("Components are Ready");
            status = Core::ERROR_NONE;
        }
        compList = (Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(components));
        LOGINFO("Component status[%d]", status);
        _adminLock.Unlock();
        return status;
    }
    

    uint32_t MigrationPreparerImplementation::reset(const string& resetType)
    {
        string empty;
        _adminLock.Lock();
        if(resetType == "RESET_ALL") {
            LOGINFO("[RESET] params={resetType: %s}", resetType.c_str());
            if(!resetDatastore())
                return Core::ERROR_GENERAL;
            setRFCParameter((char *)MIGRATIONPREPARER_NAMESPACE, TR181_MIGRATION_READY, empty.c_str(), WDMP_STRING);  
        }
        else if (resetType == "RESET_DATA") {
            LOGINFO("[RESET] params={resetType: %s}", resetType.c_str());
            if(!resetDatastore())
                return Core::ERROR_GENERAL;
        }
        else if (resetType == "RESET_READINESS") {
            LOGINFO("[RESET] params={resetType: %s}", resetType.c_str());
            setRFCParameter((char *)MIGRATIONPREPARER_NAMESPACE, TR181_MIGRATION_READY, empty.c_str(), WDMP_STRING);  
        }
        else {
            // Invalid parameter
            return Core::ERROR_INVALID_SIGNATURE;
        }
        _adminLock.Unlock();
        
        return Core::ERROR_NONE;
    }
    

} // namespace Plugin
} // namespace WPEFramework