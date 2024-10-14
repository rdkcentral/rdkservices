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

/**
 * @file  MigrationPreparer.cpp
 * @author Boopathi Vanavarayan
 * @brief Thunder plugin in RDKV ecosystem that can be used by the applications 
 *        to backup any data (such as settings) that needs to be made available 
 *        to the (new) applications after the Migration to RDKE (a.k.a., ENTOS).
 * @reference https://etwiki.sys.comcast.net/pages/viewpage.action?spaceKey=RDKV&title=MigrationPreparer.
 */

#include "MigrationPreparer.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

using namespace std;

namespace WPEFramework {
    namespace {
        static Plugin::Metadata<Plugin::MigrationPreparer> metadata (
            // Version (Major, Minor, Patch)
			API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
			// Preconditions
			{},
			// Terminations
			{},
			// Controls
            {}
        );
    }

    namespace Plugin {
        SERVICE_REGISTRATION(MigrationPreparer, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        MigrationPreparer *MigrationPreparer::_instance = nullptr;

        MigrationPreparer::MigrationPreparer()
        {
            MigrationPreparer::_instance = this;
        }

        MigrationPreparer::~MigrationPreparer()
        {
            MigrationPreparer::_instance = nullptr;
        }

        const string MigrationPreparer::Initialize(PluginHost::IShell* service)
        {
            Register(_T("write"), &MigrationPreparer::endpoint_write, this);
            Register(_T("read"), &MigrationPreparer::endpoint_read, this);
            Register(_T("delete"), &MigrationPreparer::endpoint_delete, this);
            Register(_T("setComponentReadiness"), &MigrationPreparer::setComponentReadiness, this);
            Register(_T("getComponentReadiness"), &MigrationPreparer::getComponentReadiness, this);

            // Init
            WPEFramework::Core::File dataStore(DATASTORE_PATH);
            if(dataStore.Exists())
                storeKeys();            
            return {};
        }

        void MigrationPreparer::Deinitialize(PluginHost::IShell* service)
        {
            Unregister(_T("write"));
            Unregister(_T("read"));
            Unregister(_T("delete"));
            Unregister(_T("setComponentReadiness"));
            Unregister(_T("getComponentReadiness"));
            return;
        }

        string MigrationPreparer::Information() const
		{
			return (string("{\"service\": \"") + string("org.rdk.MigrationPreparer") + string("\"}"));
		}
        
        /*Helper's: Begin*/

        bool MigrationPreparer::Unstringfy(string& input) {
            // transform \" to "
            string::size_type pos = 0;
            while ((pos = input.find("\\\"")) != string::npos) {
                input.replace(pos, 2, "\"");
            }
            return true;
        }

        string MigrationPreparer::getValue(string key) {
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
            
            // line should not be empty ot { or } if so assert
            ASSERT(line.empty() || line == "{" || line == "}");

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

        void MigrationPreparer::storeKeys(void) {
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
                colPos = line.find(":");
                key = line.substr(1,colPos-2);
                
                //add key to the map
                lineNumber[key] = lineIndex;
            }
            // update the last line index
            curLineIndex = lineIndex;
            dataStoreMutex.unlock();
        }

        void MigrationPreparer::setJSONResponseArray(JsonObject& response, const char* key, const std::vector<string>& items)
		{
			JsonArray arr;
			for (auto& i : items) {
			    arr.Add(JsonValue(i));
			}
			response[key] = arr;
		}

        void MigrationPreparer::get_components(std::vector<string>& list, string& value, string input) {
            string::size_type start = 0, pos = 0; 
            while ((pos = value.find('_', start) )!= std::string::npos) {
                list.emplace_back(value.substr(start, pos - start));
                start = pos + 1; // Move start to the next character after the delimiter
            }
            // add the last element or the only element to the list 
            list.emplace_back(value.substr(start));
            if(input != " ") 
                if (find(list.begin(), list.end(), input) == list.end()) // duplicate check
                    list.emplace_back(input);
            sort(list.begin(), list.end());
        }

        void MigrationPreparer::tokenize(string& value, std::vector<string>& list) {
        	//if the list is with only one element also it won't be a problem
            for(string::size_type i=0; i<list.size()-1; i++)
                value = value + list[i] + "_"; 
            value= value + list[list.size()-1];
	}
            
        /*Helper's: End*/
        


        /*API's: Begin*/
        uint32_t MigrationPreparer::endpoint_write(const JsonObject& parameters, JsonObject& response) {
            
            // check if required filds - name, value exists
            if (!parameters.HasLabel("name") || !parameters.HasLabel("value")) {
                return Core::ERROR_BAD_REQUEST;
            }

            // check if provided params are strigified
            if ((JsonValue::type::STRING != parameters["name"].Content()) || (JsonValue::type::STRING != parameters["value"].Content())) {
                return Core::ERROR_BAD_REQUEST;
            }

            // check if provided params are empty
            if (parameters["name"].String().empty() || parameters["value"].String().empty()) {
                return Core::ERROR_INVALID_INPUT_LENGTH;
            }
            LOGINFOMETHOD();

            string entry;
            string key = parameters["name"].String();
            string newValue = parameters["value"].String();
            WPEFramework::Core::File dataStore(DATASTORE_PATH);

            Unstringfy(newValue);

            // Handle first Write request
            if (lineNumber.empty()) {
                curLineIndex = 1;
                if(!dataStore.Exists())
                    dataStore.Create();
                
                dataStoreMutex.lock();
                
                if(!dataStore.Open(false)) {
                    LOGERR("DataStore open failed errno: %d, reason: %s", errno, strerror(errno));
                    dataStoreMutex.unlock();
                }

                // write entry to the dataStore
                entry = string("{\n \"") + key + string("\":") + newValue + string("\n}");
                dataStore.Write(reinterpret_cast<const uint8_t*>(&entry[0]), entry.size());

                // update the entry in lineNumber map
                lineNumber[key] = ++curLineIndex;
                dataStore.Close();

                dataStoreMutex.unlock();

                response["success"] = true;
                return Core::ERROR_NONE;
            }
            
            // Handle Update Request
            if(lineNumber.find(key) != lineNumber.end()) {
                string oldValue = getValue(key);
                
                if(oldValue == newValue) {
                    LOGWARN("Given value: %s for Key: %s is already existing, hence returning success", oldValue.c_str(), key.c_str());
                    response["success"] = true;
                    return Core::ERROR_NONE;
                }
                dataStoreMutex.lock();
                // sed command to replace value in the dataStore
                int result = v_secure_system("/bin/sed -i 's/%s/%s/' %s", oldValue.c_str(), newValue.c_str(), DATASTORE_PATH);
                if (result != -1 && WIFEXITED(result)) {
                    dataStoreMutex.unlock();
                    response["success"] = true;
                    return Core::ERROR_NONE;
                }
                dataStoreMutex.unlock();
                result = WEXITSTATUS(result);
                LOGERR("v_secure_system failed with error %d",result);
                return Core::ERROR_GENERAL;
            }

            // Handle subsequent Write request
            dataStoreMutex.lock();

            if(!dataStore.Open(false)) {
                LOGERR("DataStore open failed errno: %d, reason: %s\n", errno, strerror(errno));
                dataStoreMutex.unlock();
            }

            // append new key-value pair to the dataStore
            if(!dataStore.Position(false, dataStore.Size() - 2)) {
                LOGERR("DataStore truncate failed with errno: %d, reason: %s\n", errno, strerror(errno));
            }
            entry = string(",\n \"") + key + string("\":") + newValue + string("\n}"); 
            dataStore.Write(reinterpret_cast<const uint8_t*>(&entry[0]), entry.size());

            lineNumber[key] = ++curLineIndex;
            dataStore.Close();

            dataStoreMutex.unlock();

            response["success"] = true;
            return Core::ERROR_NONE;
        }

        uint32_t MigrationPreparer::endpoint_read(const JsonObject& parameters, JsonObject& response) {
            
            // check if required filds - name, value exists
            if (!parameters.HasLabel("name")) {
                return Core::ERROR_BAD_REQUEST;
            }

            // check if provided params are strigified
            if (JsonValue::type::STRING != parameters["name"].Content()) {
                return Core::ERROR_BAD_REQUEST;
            }

             // check if provided params are empty
            if (parameters["name"].String().empty()) {
                return Core::ERROR_INVALID_INPUT_LENGTH;
            }
            LOGINFOMETHOD();

            string value;
            string key = parameters["name"].String();

            // check if list is not empty and lineNumber for given key exists
            if(lineNumber.empty() || (lineNumber.find(key) == lineNumber.end())) {
                LOGERR("Value for not found for key: %s", key.c_str());
                return Core::ERROR_GENERAL;
            }

            value = getValue(key);
            response["value"] = value;
            response["success"] = true;
            return Core::ERROR_NONE;
        }

        uint32_t MigrationPreparer::endpoint_delete(const JsonObject& parameters, JsonObject& response) {
            
            // check if required filds - name, value exists
            if (!parameters.HasLabel("name")) {
                return Core::ERROR_BAD_REQUEST;
            }

            // check if provided params are strigified
            if (JsonValue::type::STRING != parameters["name"].Content()) {
                return Core::ERROR_BAD_REQUEST;
            }

            // check if provided params are empty
            if (parameters["name"].String().empty()) {
                return Core::ERROR_INVALID_INPUT_LENGTH;
            }
            LOGINFOMETHOD();

            string key = parameters["name"].String();
            int result;

            if(!lineNumber.empty() && (lineNumber.find(key) != lineNumber.end())) {
                // sed command to delete an key-value entry in the dataStore
                result = v_secure_system("/bin/sed -i '%sd' %s", std::to_string(lineNumber[key]).c_str(), DATASTORE_PATH);
                
                if (result != -1 && WIFEXITED(result)) {
                    // check if last entry is deleted
                    if(lineNumber[key] == curLineIndex) {
                        WPEFramework::Core::File dataStore(DATASTORE_PATH);
                        // check if all entries in dataStore are deleted
                        if(lineNumber.size() == 1) {
                            // remove dataStore file
                            LOGWARN("Deleting dataStore file itself since all entries are deleted");
                            dataStore.Destroy();
                        }
                        else {
                            dataStoreMutex.lock();
                            dataStore.Append();
                            // if last entry is deleted remove comma from the previous line
                            dataStore.Position(false, dataStore.Size());
                            if(!dataStore.SetSize(dataStore.Size() - 3)) {
                                LOGERR("DataStore truncate failed with errno: %d, reason: %s\n", errno, strerror(errno));
                            }
                            string entry = "\n}"; 
                            dataStore.Write(reinterpret_cast<const uint8_t*>(&entry[0]), entry.size()); 
                            dataStore.Close(); 
                            dataStoreMutex.unlock();
                        }              
                    }
                    // remove key from the lineNumber map
                    lineNumber.erase(key);
                    curLineIndex--;
                    response["success"] = result;
                    return Core::ERROR_NONE;
                }
                LOGERR("v_secure_system failed with error %d",result);
            }
            result = WEXITSTATUS(result);
            LOGERR("Key: %s does not exist in dataStore",key.c_str());
            return Core::ERROR_GENERAL;
        }

        uint32_t MigrationPreparer::setComponentReadiness(const JsonObject& parameters, JsonObject& response){

         // check if required filds - name, value exists
        if (!parameters.HasLabel("componentName")) 
             return Core::ERROR_BAD_REQUEST;
        
        // check if provided params are empty
        if (parameters["name"].String().empty()) 
            return Core::ERROR_INVALID_INPUT_LENGTH;
        
        LOGINFOMETHOD();

        RFC_ParamData_t param;
        uint32_t status = Core::ERROR_GENERAL;
        WDMP_STATUS wdmpStatus;
        std::string paramValue;
        std::string compName = parameters["componentName"].String(); //like this only in other methods
        
	wdmpStatus = getRFCParameter((char *)MIGRATION_PREPARER_RFC_CALLER_ID, TR181_MIGRATION_READY, &param);
        if (WDMP_SUCCESS == wdmpStatus) {
            paramValue = param.value;
            if(paramValue == ""){ 
                LOGINFO("no component is ready for migration, So setting the first component");
            }
            else {
                LOGINFO("component %s is already present", param.value);
		std::vector<std::string> component_list;
                get_components(component_list, paramValue, compName);
                compName= "";
                tokenize(compName, component_list);
                }
            wdmpStatus = setRFCParameter((char *)MIGRATION_PREPARER_RFC_CALLER_ID, TR181_MIGRATION_READY, compName.c_str(), WDMP_STRING);  
            status = (wdmpStatus == WDMP_SUCCESS)?Core::ERROR_NONE:Core::ERROR_GENERAL;
        }
        else { // let me confirm if getrfc is working always or not
            LOGERR("Failed to get RFC parameter");
        } 
        
	if(status == Core::ERROR_NONE){
            LOGINFO("Component readiness set successfully");
            response["success"] = true;
        }    
        else{
            response["success"] = false;
            LOGERR("Failed to set component readiness");
        }
        return status;
    }
    uint32_t MigrationPreparer::getComponentReadiness(const JsonObject& parameters, JsonObject& response)
    {
        RFC_ParamData_t param;
        string val;
        uint32_t status = Core::ERROR_GENERAL;
        std::vector<std::string> components;
        
	WDMP_STATUS wdmpStatus = getRFCParameter(const_cast<char *>("MigrationPreparer"),TR181_MIGRATION_READY, &param);
        if (WDMP_SUCCESS == wdmpStatus) {
            val = param.value;
            if( val == "") // if no component is ready for migration should we send an empty array or err message 
            {
                LOGERR("No component is ready for migration");
            }
            else {
                   get_components(components, val);
                   setJSONResponseArray(response, "componentList", components);
                   status = Core::ERROR_NONE;
                   LOGINFO("Components read successfully");
                    for (const auto &item: components)
                        std::cout<<item<<std::endl;
            }
        }
        else {
            LOGERR("Failed to get RFC parameter");
        }
        
	return status;
       }
        /*API's: End*/
    }
}
