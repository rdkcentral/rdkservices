/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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

#include "MigrationPreparer.h"

namespace WPEFramework {
namespace Plugin {

    using namespace JsonData::MigrationPreparer;

    void MigrationPreparer::RegisterAll()
    {
        Register(_T("write"), &MigrationPreparer::endpoint_write, this);
        Register(_T("read"), &MigrationPreparer::endpoint_read, this);
        Register(_T("delete"), &MigrationPreparer::endpoint_delete, this);
        Register<void, GetcomponentreadinessResultData>(_T("getComponentReadiness"), &MigrationPreparer::endpoint_getComponentReadiness, this);
        Register(_T("setComponentReadiness"), &MigrationPreparer::endpoint_setComponentReadiness, this);
        Register(_T("reset"), &MigrationPreparer::endpoint_reset, this);
    }

    void MigrationPreparer::UnregisterAll()
    {
        Unregister(_T("write"));
        Unregister(_T("read"));
        Unregister(_T("delete"));
        Unregister(_T("getComponentReadiness"));
        Unregister(_T("setComponentReadiness"));
        Unregister(_T("reset"));
    }

    uint32_t MigrationPreparer::endpoint_write(const JsonObject& parameters, JsonObject& response)
    {
        bool status = false;
        JsonObject error;

        // check if required filds - name, value exists
        if (!parameters.HasLabel("name") || !parameters.HasLabel("value")) {
            LOGERR("Invalid input - missing name or value lables");
            error["message"] = "Missing param";
            error["code"] = "-32001";
            response["error"] = error;
            returnResponse(status);
        }
        // check if provided params are strigified
        if ((JsonValue::type::STRING != parameters["name"].Content()) || (JsonValue::type::STRING != parameters["value"].Content())) {
            LOGERR("Invalid input - name or value is not stringified eg. \"params\": {\"name\":\"key\", \"value\":\"val\"}");
            error["message"] = "Param not stringified";
            error["code"] = "-32002";
            response["error"] = error;
            returnResponse(status);
        }
        // check if provided params are empty
        if (parameters["name"].String().empty() || parameters["value"].String().empty()) {
            LOGERR("Invalid input - name or value is empty");
            error["message"] = "Missing param";
            error["code"] = "-32003";
            response["error"] = error;
            returnResponse(status);
        }

        auto result = _migrationPreparer->writeEntry(
            parameters["name"].String(),
            parameters["value"].String());
        
        switch(result) {
            case ERROR_CREATE:
                error["message"] = "File create Failed";
                error["code"] = "-32004";
                response["error"] = error;
                break;
            case ERROR_OPEN:
                error["message"] = "File Open Failed";
                error["code"] = "-32005";
                response["error"] = error;
                break;
            case ERROR_WRITE:
                error["message"] = "Write Failed";
                error["code"] = "-32006";
                response["error"] = error;
                break;
            case Core::ERROR_NONE:
                status = true;
                break;
            default:
                error["message"] = "Unknown Error";
                error["code"] = "-32099";
                response["error"] = error;
        }

        returnResponse(status);
    }

    uint32_t MigrationPreparer::endpoint_read(const JsonObject& parameters, JsonObject& response)
    {
        bool status = false;
        JsonObject error;

        // check if required filds - name, value exists
        if (!parameters.HasLabel("name")) {
            LOGERR("Invalid input - missing name");
            error["message"] = "Missing param";
            error["code"] = "-32001";
            response["error"] = error;
            returnResponse(status);
        }
        // check if provided params are strigified
        if (JsonValue::type::STRING != parameters["name"].Content()) {
            LOGERR("Invalid input - name param is not stringified eg. \"params\": {\"name\":\"key\"}");
            error["message"] = "Param not stringified";
            error["code"] = "-32002";
            response["error"] = error;
            returnResponse(status);
        }
         // check if provided params are empty
        if (parameters["name"].String().empty()) {
            LOGERR("Invalid input - name param is empty");
            error["message"] = "Empty param";
            error["code"] = "-32003";
            response["error"] = error;
            returnResponse(status);
        }

        string value;
        auto result = _migrationPreparer->readEntry(
                            parameters["name"].String(),
                            value);

        switch(result) {
            case ERROR_NOFILE:
                error["message"] = "Error file not exist";
                error["code"] = "-32012";
                response["error"] = error;
                break;
            case ERROR_FILEEMPTY:
                error["message"] = "Error file empty";
                error["code"] = "-32013";
                response["error"] = error;
                break;
            case ERROR_READ:
                error["message"] = "Name not found";
                error["code"] = "-32007";
                response["error"] = error;
                break;
            case Core::ERROR_NONE:
                status = true;
                response["value"] = value;
                break;
            default:
                error["message"] = "Unknown Error";
                error["code"] = "-32099";
                response["error"] = error;
        }

        returnResponse(status);
    }

    uint32_t MigrationPreparer::endpoint_delete(const JsonObject& parameters, JsonObject& response) {
        bool status = false;
        JsonObject error;

        // check if required filds - name, value exists
        if (!parameters.HasLabel("name")) {
            LOGERR("Invalid input - missing name");
            error["message"] = "Missing param";
            error["code"] = "-32001";
            response["error"] = error;
            returnResponse(status);
        }
        // check if provided params are strigified
        if (JsonValue::type::STRING != parameters["name"].Content()) {
            LOGERR("Invalid input - name param is not stringified eg. \"params\": {\"name\":\"key\"}");
            error["message"] = "Param not stringified";
            error["code"] = "-32002";
            response["error"] = error;
            returnResponse(status);
        }
        // check if provided params are empty
        if (parameters["name"].String().empty()) {
            LOGERR("Invalid input - name param is empty");
            error["message"] = "Empty param";
            error["code"] = "-32003";
            response["error"] = error;
            returnResponse(status);
        }

        auto result = _migrationPreparer->deleteEntry(
                            parameters["name"].String());

        switch(result) {
            case ERROR_DELETE:
                error["message"] = "Name not found";
                error["code"] = "-32008";
                response["error"] = error;
                break;
            case ERROR_NOFILE:
                error["message"] = "Error file not exist";
                error["code"] = "-32012";
                response["error"] = error;
                break;
            case ERROR_FILEEMPTY:
                error["message"] = "Error file empty";
                error["code"] = "-32013";
                response["error"] = error;
                break;
            case Core::ERROR_NONE:
                status = true;
                break;
            default:
                error["message"] = "Unknown Error";
                error["code"] = "-32099";
                response["error"] = error;
        }

        returnResponse(status);
    }

    
    uint32_t MigrationPreparer::endpoint_getComponentReadiness(GetcomponentreadinessResultData& response) {
        RPC::IStringIterator* componentList = nullptr;
        auto result = _migrationPreparer->getComponentReadiness(componentList);
        if (result == Core::ERROR_NONE) {
            string component;
            if(componentList != nullptr) {
                while (componentList->Next(component) == true) {
                    response.ComponentList.Add() = component;
                }
                componentList->Release();
            }
            else {
                response.ComponentList.Add() = string("\0"); //empty string array
            }
            response.Success = true;
        }
        return result;
    }
    

    uint32_t MigrationPreparer::endpoint_setComponentReadiness(const JsonObject& parameters, JsonObject& response) {        
        bool status = false;
        JsonObject error;

        // check if required filds - name, value exists
        if (!parameters.HasLabel("componentName")) {
            LOGERR("Invalid input - missing componentName");
            error["message"] = "Missing param";
            error["code"] = "-32001";
            response["error"] = error;
            returnResponse(status);
        }
        // check if provided params are strigified
        if (JsonValue::type::STRING != parameters["componentName"].Content()) {
            LOGERR("Invalid input - componentName param is not stringified eg. \"params\": {\"componentName\":\"RA01\"}");
            error["message"] = "Param not stringified";
            error["code"] = "-32002";
            response["error"] = error;
            returnResponse(status);
        }
        // check if provided params are empty
        if (parameters["componentName"].String().empty()) {
            LOGERR("Invalid input - componentName param is empty");
            error["message"] = "Empty param";
            error["code"] = "-32003";
            response["error"] = error;
            returnResponse(status);
        }
        
        auto result = _migrationPreparer->setComponentReadiness(
                            parameters["componentName"].String());

        switch(result) {
            case ERROR_CREATE:
                error["message"] = "File create Failed";
                error["code"] = "-32004";
                response["error"] = error;
                break;
            case ERROR_OPEN:
                error["message"] = "File Open Failed";
                error["code"] = "-32005";
                response["error"] = error;
                break;
            case ERROR_WRITE:
                error["message"] = "Write Failed";
                error["code"] = "-32006";
                response["error"] = error;
                break;
            case ERROR_SET:
                error["message"] = "Error creating file";
                error["code"] = "-32009";
                response["error"] = error;
                break;
            case Core::ERROR_NONE:
                status = true;
                break;
            default:
                error["message"] = "Unknown Error";
                error["code"] = "-32099";
                response["error"] = error;
        }

        returnResponse(status);
    }


    uint32_t MigrationPreparer::endpoint_reset(const JsonObject& parameters, JsonObject& response) {
        bool status = false;
        JsonObject error;

        // check if required filds - name, value exists
        if (!parameters.HasLabel("resetType")) {
            LOGERR("Invalid input - missing resetType");
            error["message"] = "Missing param";
            error["code"] = "-32001";
            response["error"] = error;
            returnResponse(status);
        }
        // check if provided params are strigified
        if (JsonValue::type::STRING != parameters["resetType"].Content()) {
            LOGERR("Invalid input - resetType param is not stringified eg. \"params\": {\"resetType\":\"RESET_ALL\"}");
            error["message"] = "Param not stringified";
            error["code"] = "-32002";
            response["error"] = error;
            returnResponse(status);
        }
        // check if provided params are empty
        if (parameters["resetType"].String().empty()) {
            LOGERR("Invalid input - resetType param is empty");
            error["message"] = "Empty param";
            error["code"] = "-32003";
            response["error"] = error;
            returnResponse(status);
        }

        auto result = _migrationPreparer->reset(parameters["resetType"].String());
        switch(result) {
            case ERROR_RESET:
                error["message"] = "Unexpected error while resetting";
                error["code"] = "-32010";
                response["error"] = error;
                break;
            case ERROR_INVALID:
                error["message"] = "Invalid parameter";
                error["code"] = "-32011";
                response["error"] = error;
                break;
            case Core::ERROR_NONE:
                status = true;
                break;
            default:
                error["message"] = "Unknown Error";
                error["code"] = "-32099";
                response["error"] = error;
        }

        returnResponse(status);
    }    
} // namespace Plugin
} // namespace WPEFramework
