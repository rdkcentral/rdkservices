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
        _fileExist = false;
        WPEFramework::Core::File dataStore;         
		dataStore = string("/opt/secure/migration/migration_data_store.json");
        if(dataStore.Exists()) {
            _fileExist = true;
            storeKeys();
        }
    }

    MigrationPreparerImplementation::~MigrationPreparerImplementation()
    {
    }

    /*Helper's: Begin*/
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
            escapedString = "\":"+ escapedString;
        }
        else if(type == REPLACEMENT) {
            // Iterate through each character in the string
            for(char ch : input) {
                // If ch is a special character prepend it with a backslash
                if (specialChars.find(ch) != specialChars.end())
                    escapedString += "\\";

                escapedString += ch;
            }
            escapedString = "\":"+ escapedString;
        }
        return escapedString;
    }

    void MigrationPreparerImplementation::storeKeys(void) {
        _dataStoreMutex.lock();
        std::ifstream inputFile(DATASTORE_PATH);
        string key, line;
        size_t start, end, colPos;
        LINE_NUMBER_TYPE lineIndex = 1;
        while(std::getline(inputFile, line)) {
            // trim white space, new line and comma from the line
            start = line.find_first_not_of(" \n");
            end = line.find_last_not_of(" \n,");
            line =  line.substr(start, end-start+1);
            if(start == string::npos || end == string::npos) {
                LOGWARN("Invalid line in dataStore file, skipping");
                continue;
            }
            ASSERT(start > end);
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
            _lineNumber[key] = lineIndex;

            string value = line.substr(colPos+1);
            _valueEntry.push_back(value);
        }
        // update the last line index
        _curLineIndex = lineIndex;
        _dataStoreMutex.unlock();
    }


    bool MigrationPreparerImplementation::resetDatastore(void){
        WPEFramework::Core::File dataStore;
        dataStore = string("/opt/secure/migration/migration_data_store.json");
        if(!dataStore.Exists()) {
            LOGWARN("DataStore file does not exist");
            return true;
        }
        _dataStoreMutex.lock();
        // remove dataStore file
        if(!dataStore.Destroy()){
            LOGERR("Unable to delete dataStore file");
            return false;
        }
        // clear the _lineNumber map and _valueEntry vector internal data structures
        if (!_lineNumber.empty())
            _lineNumber.clear();
        _curLineIndex = 1;
        if (!_valueEntry.empty())
            _valueEntry.clear();
        _dataStoreMutex.unlock();
        return true;
    }
    bool MigrationPreparerImplementation::resetMigrationready(void){
        WPEFramework::Core::File migrationReady;         
		migrationReady = string("/opt/secure/migration/migrationready.txt");
        if(!migrationReady.Exists()) {
            LOGWARN("MigrationReady file does not exist");
            return true;
        }
        _adminLock.Lock();
        // remove migrationReady file
        LOGWARN("Deleting migrationReady file");
        if(!migrationReady.Destroy()){
            LOGERR("Unable to delete migrationReady file");
            return false;
        }
        _adminLock.Unlock();
        return true;
    }

    int8_t MigrationPreparerImplementation::split(std::list<string>& list, string& value, std::string delimiter) {
        // LOGINFO("split: %s", value.c_str());
        string::size_type start = 0, pos = 0; 
        while ((pos = value.find(delimiter, start) )!= std::string::npos) {
            list.emplace_back(value.substr(start, pos - start));
            start = pos + 1; // Move start to the next character after the delimiter
        }
        // add the last element or the only element to the list 
        list.emplace_back(value.substr(start));
        return Core::ERROR_NONE;
    }

    int8_t MigrationPreparerImplementation::join(string& value, std::list<string>& list, std::string delimiter) {
        //if the list is with only one element also it won't be a problem
        if (list.empty()) return Core::ERROR_NONE;  // Handle empty list case
        auto it = list.begin();
        // Use the for loop to concatenate elements with '_'
        for (; std::next(it) != list.end(); ++it) {
        value += *it + delimiter;  // Concatenate element with underscore
        }
        // Add the last element without an underscore
        value += *it;
        // LOGINFO("join: %s", value.c_str());
        return Core::ERROR_NONE;
    }

    bool MigrationPreparerImplementation::isDuplicate(string& searchString, std::list<string>& list) {
        // Use std::find to check if the string is present
        if (list.empty()) return Core::ERROR_NONE;  // Handle empty list case
        auto it = std::find(list.begin(), list.end(), searchString);
        if (it != list.end()) {
            return true;
        } 
        return false;
    }
   
    /*Helper's: End*/

    uint32_t MigrationPreparerImplementation::write(const string& name, const string &value) {

        // LOGINFO("[WRITE] params={name: %s, value: %s}", name.c_str(), value.c_str());
        LOGINFO("Writing entry for name: %s", name.c_str());
        string entry;
        string key = name;
        string newValue = value;
         WPEFramework::Core::File dataStore;         
		dataStore = string("/opt/secure/migration/migration_data_store.json");
        WPEFramework::Core::Directory dataStoreDir(DATASTORE_DIR);
        
        // check if someone deletes the dataStore in the middle of the operation
        if(!_lineNumber.empty() && !dataStore.Exists()) {
            LOGWARN("Migration datastore %s deleted in the middle of the operation, resetting the internal data structures", DATASTORE_PATH);
            _lineNumber.clear();
            _curLineIndex = 1;
            if (!_valueEntry.empty())
                _valueEntry.clear();
        }
        
        _dataStoreMutex.lock();
        // Handle first Write request
        if (_lineNumber.empty()) {
            _curLineIndex = 1;
            if(!dataStore.Exists()) {
                // check if the directory exist
                if(!dataStoreDir.Exists())
                    // assuming /opt/secure path already exist and "migration" folder needs to be under /opt/secure/
                    dataStoreDir.Create();
                
                if(!dataStore.Create()) {
                    LOGERR("Failed to create migration datastore %s, errno: %d, reason: %s", DATASTORE_PATH, errno, strerror(errno));
                    _dataStoreMutex.unlock();
                    return ERROR_CREATE;
                }
                _fileExist = true;
            }
            
            if(!dataStore.Open(false)) {
                LOGERR("Failed to open migration datastore %s, errno: %d, reason: %s", DATASTORE_PATH, errno, strerror(errno));
                // LOGERR("Failed to create entry for {%s:%s} in migration datastore", key.c_str(), newValue.c_str());
                LOGERR("Failed to create entry for name: %s in migration datastore", key.c_str());
                _dataStoreMutex.unlock();
                return ERROR_OPEN;
            }
            // write entry to the dataStore
            entry = string("{\n \"") + key + string("\":") + newValue + string("\n}");
            dataStore.Write(reinterpret_cast<const uint8_t*>(&entry[0]), entry.size());
            // update the entry in _lineNumber map
            _lineNumber[key] = ++_curLineIndex;
            _valueEntry.push_back(newValue);
            dataStore.Close();
            _dataStoreMutex.unlock();
            return Core::ERROR_NONE;
        }

        // Handle Update Request
        if(_lineNumber.find(key) != _lineNumber.end()) {

            string oldValue = _valueEntry[_lineNumber[key] - 2];

            if(oldValue == newValue) {
                // LOGWARN("Entry {%s:%s} is already existing in the dataStore, returning success", oldValue.c_str(), key.c_str());
                LOGWARN("Entry for name: %s is already existing in the dataStore, returning success", key.c_str());
                _dataStoreMutex.unlock();
                return Core::ERROR_NONE;
            }
            
            // since v_secure_system syscall wrapper has set limitation on command length             
            if(newValue.length() > 200 || oldValue.length() > 200) {
                _dataStoreMutex.unlock();
                LOGINFO("[WRITE] Value for the key %s is long, hence deleting existing entry and adding it as new entry at the end of dataStore", key.c_str());
                if(Delete(key) != Core::ERROR_NONE)
                    return ERROR_WRITE;
                    
                return write(key, newValue);
            }

            // sed command to replace value in the dataStore
            oldValue = escapeSed(oldValue, PATTERN);
            string escapedNewValue = escapeSed(newValue, REPLACEMENT);
            int result = v_secure_system("/bin/sed -i -E '%ss/%s/%s/' %s", std::to_string(_lineNumber[key]).c_str(), oldValue.c_str(), escapedNewValue.c_str(), DATASTORE_PATH);
            
            if (result != -1 && WIFEXITED(result)) {
                _valueEntry[_lineNumber[key] - 2] = newValue;
                _dataStoreMutex.unlock();
                return Core::ERROR_NONE;
            }
            result = WEXITSTATUS(result);
            // LOGERR("Failed to update entry for {%s:%s} in migration datastore, v_secure_system failed with error %d",key.c_str(), newValue.c_str(), result);
            LOGERR("Failed to update entry for name: %s in migration datastore, v_secure_system failed with error %d",key.c_str(), result);
            _dataStoreMutex.unlock();
            return ERROR_WRITE;
        }

        // Handle subsequent Write request
        if(!dataStore.Open(false)) {
            LOGERR("Failed to create migration datastore %s, errno: %d, reason: %s", DATASTORE_PATH, errno, strerror(errno));
            // LOGERR("Failed to create entry for {%s:%s} in migration datastore", key.c_str(), newValue.c_str());
            LOGERR("Failed to create entry for name: %s in migration datastore", key.c_str());
            _dataStoreMutex.unlock();
            return ERROR_OPEN;
        }

        // append new key-value pair to the dataStore
        if(!dataStore.Position(false, dataStore.Size() - 2)) {
            LOGERR("DataStore truncate failed with errno: %d, reason: %s\n", errno, strerror(errno));
            // LOGERR("Failed to create entry for {%s:%s} in migration datastore", key.c_str(), newValue.c_str());
            LOGERR("Failed to create entry for %s in migration datastore", key.c_str());
            dataStore.Close();
            _dataStoreMutex.unlock();
            return ERROR_WRITE;
        }
        entry = string(",\n \"") + key + string("\":") + newValue + string("\n}"); 
        dataStore.Write(reinterpret_cast<const uint8_t*>(&entry[0]), entry.size());
        _lineNumber[key] = ++_curLineIndex;
        _valueEntry.push_back(newValue);
        dataStore.Close();
        _dataStoreMutex.unlock();
        return Core::ERROR_NONE;
    }
    
    uint32_t MigrationPreparerImplementation::read(const string& name, string &result) {

        LOGINFO("Reading entry for name: %s", name.c_str());
        string key = name;

        if(!_fileExist) { 
            LOGERR("Failed to read key: %s, migration dataStore %s do not exist", key.c_str(), DATASTORE_PATH);
            return ERROR_NOFILE;
        }

        if(_lineNumber.empty()) {
            LOGERR("Failed to read key: %s, migration dataStore %s is empty", key.c_str(), DATASTORE_PATH);
            return ERROR_FILEEMPTY;
        }

        // check if list is not empty and _lineNumber for given key exists
        if(_lineNumber.find(key) == _lineNumber.end()) {
            LOGERR("Failed to read key: %s, Key do not exist in migration dataStore", key.c_str());
            return ERROR_NAME;
        }
        
        result = _valueEntry[_lineNumber[key] - 2];
        return Core::ERROR_NONE;
    }

    uint32_t MigrationPreparerImplementation::Delete(const string& name) {

        LOGINFO("Deleting entry for name: %s", name.c_str());
        string key = name;
        int result;

        if(_lineNumber.empty()) {
            LOGERR("Failed to delete key: %s, migration dataStore %s is empty", key.c_str(), DATASTORE_PATH);
            return ERROR_FILEEMPTY;
        }

        _dataStoreMutex.lock();
        if(_lineNumber.find(key) != _lineNumber.end()) {
            // sed command to delete an key-value entry in the dataStore            
            result = v_secure_system("/bin/sed -i '%sd' %s", std::to_string(_lineNumber[key]).c_str(), DATASTORE_PATH);

            if (result != -1 && WIFEXITED(result)) {
                // check if last entry is deleted
                if(_lineNumber[key] == _curLineIndex) {
                     WPEFramework::Core::File dataStore;         
					dataStore = string("/opt/secure/migration/migration_data_store.json");
                    dataStore.Append();
                    // if last entry is deleted remove comma from the previous line
                    dataStore.Position(false, dataStore.Size());
                    if(!dataStore.SetSize(dataStore.Size() - 3)) {
                        LOGERR("DataStore truncate failed with errno: %d, reason: %s\n", errno, strerror(errno));
                    }
                    string entry;
                    if(_lineNumber.size() == 1) // if no other entries left
                        entry = string("{\n}"); 
                    else
                        entry = string("\n}");

                    dataStore.Write(reinterpret_cast<const uint8_t*>(&entry[0]), entry.size()); 
                    dataStore.Close();             
                }
                // Adjust line line number for other entries
                for (auto it = _lineNumber.begin(); it != _lineNumber.end(); ++it) {
                    if(it->second > _lineNumber[key]) {
                        it->second--;
                    }
                }

                _valueEntry.erase(_valueEntry.begin() + _lineNumber[key] - 2);

                // remove key from the _lineNumber map
                _lineNumber.erase(key);
                _curLineIndex--;
                _dataStoreMutex.unlock();
                return Core::ERROR_NONE;
            }
            
            result = WEXITSTATUS(result);
            LOGERR("Failed to delete entry for key: %s in migration datastore, v_secure_system failed with error %d",key.c_str(), result);
            _dataStoreMutex.unlock();
            return ERROR_DELETE;
        }

        LOGERR("Failed to delete entry for key: %s in migration datastore, Key does not exist in dataStore",key.c_str());       
        _dataStoreMutex.unlock();
        return ERROR_NAME;    
    }

    
   uint32_t MigrationPreparerImplementation::setComponentReadiness(const string& compName)
    {
        uint32_t status = Core::ERROR_GENERAL;
        string _compName = compName;
        _adminLock.Lock();
        LOGINFO("Component Name: %s", _compName.c_str());
        WPEFramework::Core::File migrationReady;         
		migrationReady = string("/opt/secure/migration/migrationready.txt");
        WPEFramework::Core::Directory migrationReadyDir(MIGRATIONREADY_DIR);

        // check whether file is exist or not, if not create one
        if(!migrationReady.Exists()) {
            // check if the directory exist
            if(!migrationReadyDir.Exists()){
                // assuming /opt/secure path already exist and "migration" folder needs to be under /opt/secure/
                if(migrationReadyDir.Create()){
                    LOGERR("Failed to create migration ready %s, errno: %d, reason: %s", MIGRATIONREADY_PATH, errno, strerror(errno));
                    _adminLock.Unlock();
                    return ERROR_CREATE;                    
                }
            }
            if(!migrationReady.Create()) {
                LOGERR("Failed to create migration ready %s, errno: %d, reason: %s", MIGRATIONREADY_PATH, errno, strerror(errno));
                _adminLock.Unlock();
                return ERROR_CREATE;
            }
        }

        // open file in append mode   
        if(!migrationReady.Open(false)) {
            LOGERR("Failed to open migration ready %s, errno: %d, reason: %s", MIGRATIONREADY_PATH, errno, strerror(errno));
            LOGERR("Failed to add component{%s} in migration ready", _compName.c_str());
            _adminLock.Unlock();
            return ERROR_OPEN;
        }
        // read component list and add new component in order into file
        string outComponentString = _compName;
        // Lambda to compare strings in lexicographical order
        auto componentComparator = [](const std::string& a, const std::string& b) {
            return a < b;
        };            
        if(migrationReady.Size() > 0){
            uint8_t datatoread[migrationReady.Size()+1]={'\0'};
            uint32_t size = migrationReady.Read(datatoread, static_cast<uint32_t>(sizeof(datatoread)));
            if(size){
                std::string inComponentString(reinterpret_cast<char*>(datatoread));
                std::list<std::string> componentlist;
                outComponentString = "";
                split(componentlist, inComponentString);
                // check whether it is duplicate or not
                if(isDuplicate(_compName, componentlist)){
                    migrationReady.Close();
                    status = Core::ERROR_NONE;
                    _adminLock.Unlock();
                    return status;                        
                }
                componentlist.push_back(_compName);
                componentlist.sort(componentComparator);
                // Print component of the set
                for (const auto& str : componentlist) {
                    std::cout << str << " ";
                }
                std::cout << std::endl;
                join(outComponentString, componentlist);
            }else{
                LOGERR("Failed to read migration ready %s, errno: %d, reason: %s", MIGRATIONREADY_PATH, errno, strerror(errno));
                LOGERR("Failed to add component{%s} in migration ready", _compName.c_str());
                _adminLock.Unlock();
                return ERROR_SET;
            }
        }
        uint8_t datatowrite[outComponentString.size()];
        ::memcpy(datatowrite, outComponentString.data(), outComponentString.size());
        // write componentList to the migration ready file from begining 
        migrationReady.Position(false, 0);
        migrationReady.Write(datatowrite, static_cast<uint32_t>(sizeof(datatowrite)));
        migrationReady.Close();
        status = Core::ERROR_NONE;         
        _adminLock.Unlock();
        return status;
    }
    
    
    uint32_t MigrationPreparerImplementation::getComponentReadiness(RPC::IStringIterator*& compList)
    {
        uint32_t status = Core::ERROR_GENERAL;
        _adminLock.Lock();
        std::list<string> componentlist;      
        WPEFramework::Core::File migrationReady;         
		migrationReady = string("/opt/secure/migration/migrationready.txt");
        WPEFramework::Core::Directory migrationReadyDir(MIGRATIONREADY_DIR);

        // check whether file is exist or not, if not return empty string
        if(!migrationReady.Exists()) {
            LOGERR("MigrationReady file %s does not exist", MIGRATIONREADY_PATH);
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        // open file in append mode   
        if(!migrationReady.Open(false)) {
            LOGERR("Failed to open migration ready %s, errno: %d, reason: %s", MIGRATIONREADY_PATH, errno, strerror(errno));
            _adminLock.Unlock();
            return Core::ERROR_GENERAL;
        }
        // read component list and add new component in order into file
        if(migrationReady.Size() > 0){
            uint8_t datatoread[migrationReady.Size() + 1] ={'\0'};
            uint32_t size = migrationReady.Read(datatoread, static_cast<uint32_t>(sizeof(datatoread)));
            if(size){ 
                std::string inComponentString(reinterpret_cast<char*>(datatoread));
                split(componentlist, inComponentString);
            }           
        }      
        compList = (Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(componentlist));
        status = Core::ERROR_NONE;
        LOGINFO("Component status[%d]", status);
        _adminLock.Unlock();
        return status;
    }
    

    uint32_t MigrationPreparerImplementation::reset(const string& resetType)
    {
        string empty;
        _adminLock.Lock();
        LOGINFO("[RESET] params={resetType: %s}", resetType.c_str());
        if(resetType == "RESET_ALL") {
            if(!resetDatastore())
                return ERROR_RESET;
            if(!resetMigrationready())
                return ERROR_RESET;
        }
        else if (resetType == "RESET_DATA") {
            if(!resetDatastore())
                return ERROR_RESET;
        }
        else if (resetType == "RESET_READINESS") {
            if(!resetMigrationready())
                return ERROR_RESET;        
        }
        else {
            // Invalid parameter
            return ERROR_INVALID;
        }
        _adminLock.Unlock();
        
        return Core::ERROR_NONE;
    }    

} // namespace Plugin
} // namespace WPEFramework
