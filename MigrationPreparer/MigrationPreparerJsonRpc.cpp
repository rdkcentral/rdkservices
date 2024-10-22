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
        Register<JsonObject, WriteentryResultInfo>(_T("setComponentReadiness"), &MigrationPreparer::endpoint_setComponentReadiness, this);
        Register<ResetParamsData, WriteentryResultInfo>(_T("reset"), &MigrationPreparer::endpoint_reset, this);
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
        // check if required filds - name, value exists
        if (!parameters.HasLabel("name") || !parameters.HasLabel("value")) {
            LOGERR("Invalid input - missing name or value lables");
            return Core::ERROR_BAD_REQUEST;
        }
        // check if provided params are strigified
        if ((JsonValue::type::STRING != parameters["name"].Content()) || (JsonValue::type::STRING != parameters["value"].Content())) {
            LOGERR("Invalid input - name or value is not stringified");
            return Core::ERROR_BAD_REQUEST;
        }
        // check if provided params are empty
        if (parameters["name"].String().empty() || parameters["value"].String().empty()) {
            LOGERR("Invalid input - name or value is empty");
            return Core::ERROR_BAD_REQUEST;
        }

        auto result = _migrationPreparer->writeEntry(
            parameters["name"].String(),
            parameters["value"].String());
        if (result == Core::ERROR_NONE) {
            response["success"] = true;
        }

        return result;
    }

    uint32_t MigrationPreparer::endpoint_read(const JsonObject& parameters, JsonObject& response)
    {
        // check if required filds - name, value exists
        if (!parameters.HasLabel("name")) {
            LOGERR("Invalid input - missing name");
            return Core::ERROR_BAD_REQUEST;
        }
        // check if provided params are strigified
        if (JsonValue::type::STRING != parameters["name"].Content()) {
            LOGERR("Invalid input - name is not stringified");
            return Core::ERROR_BAD_REQUEST;
        }
         // check if provided params are empty
        if (parameters["name"].String().empty()) {
            LOGERR("Invalid input - name is empty");
            return Core::ERROR_BAD_REQUEST;
        }

        string value;
        auto result = _migrationPreparer->readEntry(
                            parameters["name"].String(),
                            value);
        if (result == Core::ERROR_NONE) {
            response["value"] = value;
            response["success"] = true;
        }

        return result;
    }

    uint32_t MigrationPreparer::endpoint_delete(const JsonObject& parameters, JsonObject& response) {
        // check if required filds - name, value exists
        if (!parameters.HasLabel("name")) {
            LOGERR("Invalid input - missing name");
            return Core::ERROR_BAD_REQUEST;
        }
        // check if provided params are strigified
        if (JsonValue::type::STRING != parameters["name"].Content()) {
            LOGERR("Invalid input - name is not stringified");
            return Core::ERROR_BAD_REQUEST;
        }
        // check if provided params are empty
        if (parameters["name"].String().empty()) {
            LOGERR("Invalid input - name is empty");
            return Core::ERROR_BAD_REQUEST;
        }

        auto result = _migrationPreparer->deleteEntry(
                            parameters["name"].String());

        if (result == Core::ERROR_NONE) {
            response["success"] = true;
        }

        return result;
    }

    
    uint32_t MigrationPreparer::endpoint_getComponentReadiness(GetcomponentreadinessResultData& response) {

        RPC::IStringIterator* componentList = nullptr;
        auto result = _migrationPreparer->getComponentReadiness(componentList);
        if (result == Core::ERROR_NONE) {
            string component;
            while (componentList->Next(component) == true) {
                response.ComponentList.Add() = component;
            }
            componentList->Release();
            response.Success = true;
        }
        return result;
    }
    

    uint32_t MigrationPreparer::endpoint_setComponentReadiness(const JsonObject& parameters, WriteentryResultInfo& response) {        
        // check if required filds - name, value exists
        if (!parameters.HasLabel("componentName")) {
            LOGERR("Invalid input - missing ComponentName");
            return Core::ERROR_BAD_REQUEST;
        }
        // check if provided params are strigified
        if (JsonValue::type::STRING != parameters["componentName"].Content()) {
            LOGERR("Invalid input - ComponentName is not stringified");
            return Core::ERROR_BAD_REQUEST;
        }
        // check if provided params are empty
        if (parameters["componentName"].String().empty()) {
            LOGERR("Invalid input - ComponentName is empty");
            return Core::ERROR_BAD_REQUEST;
        }

        auto result = _migrationPreparer->setComponentReadiness(
                            parameters["componentName"].String());

        if (result == Core::ERROR_NONE) {
            response.Success = true;
        }
        return result;
    }


    uint32_t MigrationPreparer::endpoint_reset(const ResetParamsData& params, WriteentryResultInfo& response) {
        // check if provided params are available
        if (params.ResetType.IsNull()) {
            return Core::ERROR_INVALID_INPUT_LENGTH;
        }

        // check if provided params are empty
        if (params.ResetType.Value() == string()) {
            return Core::ERROR_INVALID_INPUT_LENGTH;
        }
        
        //No checks included to check if provided params is string or not

        auto result = _migrationPreparer->reset(params.ResetType.Value());
        if (result == Core::ERROR_NONE) {
            response.Success = true;
        }

        return result;
    }
    
} // namespace Plugin
} // namespace WPEFramework
