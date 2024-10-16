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
        Register<WriteentryParamsData, WriteentryResultInfo>(_T("write"), &MigrationPreparer::endpoint_write, this);
        Register<DeleteentryParamsInfo, ReadentryResultData>(_T("read"), &MigrationPreparer::endpoint_read, this);
        Register<DeleteentryParamsInfo, WriteentryResultInfo>(_T("delete"), &MigrationPreparer::endpoint_delete, this);
        Register<void, GetcomponentreadinessResultData>(_T("getComponentReadiness"), &MigrationPreparer::endpoint_getComponentReadiness, this);
        Register<SetcomponentreadinessParamsData, WriteentryResultInfo>(_T("setComponentReadiness"), &MigrationPreparer::endpoint_setComponentReadiness, this);
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

    uint32_t MigrationPreparer::endpoint_write(const WriteentryParamsData& params, WriteentryResultInfo& response)
    {
        // check if provided params are available
        if (params.Name.IsNull() || params.Value.IsNull()) {
            return Core::ERROR_INVALID_INPUT_LENGTH;
        }

        // check if provided params are empty
        if (params.Name.Value() == string() || params.Value.Value() == string()) {
            return Core::ERROR_INVALID_INPUT_LENGTH;
        }
        
        //No checks included to check if provided params is string or not

        auto result = _migrationPreparer->writeEntry(
            params.Name.Value(),
            params.Value.Value());
        if (result == Core::ERROR_NONE) {
            response.Success = true;
        }

        return result;
    }

    uint32_t MigrationPreparer::endpoint_read(const DeleteentryParamsInfo& params, ReadentryResultData& response)
    {
        // check if provided params are available
        if (params.Name.IsNull()) {
            return Core::ERROR_INVALID_INPUT_LENGTH;
        }

        // check if provided params are empty
        if (params.Name.Value() == string()) {
            return Core::ERROR_INVALID_INPUT_LENGTH;
        }

        //No checks included to check if provided params is string or not

        string value;
        auto result = _migrationPreparer->readEntry(
                            params.Name.Value(),
                            value);
        if (result == Core::ERROR_NONE) {
            response.Value = value;
            response.Success = true;
        }

        return result;
    }

    uint32_t MigrationPreparer::endpoint_delete(const DeleteentryParamsInfo& params, WriteentryResultInfo& response) {
        // check if provided params are available
        if (params.Name.IsNull()) {
            return Core::ERROR_INVALID_INPUT_LENGTH;
        }

        // check if provided params are empty
        if (params.Name.Value() == string()) {
            return Core::ERROR_INVALID_INPUT_LENGTH;
        }
        
        //No checks included to check if provided params is string or not

        auto result = _migrationPreparer->deleteEntry(
                            params.Name.Value());

        if (result == Core::ERROR_NONE) {
            response.Success = true;
        }

        return result;
    }

    
    uint32_t MigrationPreparer::endpoint_getComponentReadiness(GetcomponentreadinessResultData& response) {

        RPC::IStringIterator* componentList;
        auto result = _migrationPreparer->getComponentReadiness(componentList);
        if (result == Core::ERROR_NONE) {
            string component;
            while (componentList->Next(component) == true) {
                response.componentList.Add() = component;
            }
            componentList->Release();
            response.Success = true;
        }
        return result;
    }
    

    uint32_t MigrationPreparer::endpoint_setComponentReadiness(const SetcomponentreadinessParamsData& params, WriteentryResultInfo& response) {
        // check if provided params are available
        if (params.componentName.IsNull()) {
            return Core::ERROR_INVALID_INPUT_LENGTH;
        }

        // check if provided params are empty
        if (params.componentName.Value() == string()) {
            return Core::ERROR_INVALID_INPUT_LENGTH;
        }
        
        //No checks included to check if provided params is string or not

        auto result = _migrationPreparer->setComponentReadiness(
                            params.componentName.Value());

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