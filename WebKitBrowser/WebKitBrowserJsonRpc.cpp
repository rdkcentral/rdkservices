/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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

#include "Module.h"
#include "WebKitBrowser.h"

#include <interfaces/json/JsonData_Browser.h>
#include <interfaces/json/JsonData_StateControl.h>

namespace WPEFramework {

namespace Plugin {

    using namespace JsonData::Browser;
    using namespace JsonData::WebBrowser;
    using namespace JsonData::StateControl;
    using namespace WPEFramework::Exchange;

    // Registration
    //

    void WebKitBrowser::RegisterAll()
    {
        Property<Core::JSON::EnumType<StateType>>(_T("state"), &WebKitBrowser::get_state, &WebKitBrowser::set_state, this); /* StateControl */
        Register<DeleteParamsData,void>(_T("delete"), &WebKitBrowser::endpoint_delete, this);
    }

    void WebKitBrowser::UnregisterAll()
    {
        Unregister(_T("state"));
        Unregister(_T("delete"));
    }

    // API implementation
    //

    // Method: endpoint_delete - delete dir
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t WebKitBrowser::endpoint_delete(const DeleteParamsData& params)
    {
        return DeleteDir(params.Path.Value());
    }

    // Property: state - Running state of the service
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t WebKitBrowser::get_state(Core::JSON::EnumType<StateType>& response) const /* StateControl */
    {
        ASSERT(_browser != nullptr);

        PluginHost::IStateControl* stateControl(_browser->QueryInterface<PluginHost::IStateControl>());

        // In the mean time an out-of-process plugin might have crashed and thus return a nullptr.
        if (stateControl != nullptr) {

            PluginHost::IStateControl::state currentState = stateControl->State();
            response = (currentState == PluginHost::IStateControl::SUSPENDED? StateType::SUSPENDED : StateType::RESUMED);

            stateControl->Release();
        }

        return Core::ERROR_NONE;
    }

    // Property: state - Running state of the service
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t WebKitBrowser::set_state(const Core::JSON::EnumType<StateType>& param) /* StateControl */
    {
        ASSERT(_browser != nullptr);

        uint32_t result = Core::ERROR_BAD_REQUEST;

        if (param.IsSet()) {
            PluginHost::IStateControl* stateControl(_browser->QueryInterface<PluginHost::IStateControl>());

            // In the mean time an out-of-process plugin might have crashed and thus return a nullptr.
            if (stateControl != nullptr) {

                stateControl->Request(param == StateType::SUSPENDED? PluginHost::IStateControl::SUSPEND : PluginHost::IStateControl::RESUME);

                stateControl->Release();
            }

            result = Core::ERROR_NONE;
        }

        return result;
    }

    // Event: statechange - Signals a state change of the service
    void WebKitBrowser::event_statechange(const bool& suspended) /* StateControl */
    {
        StatechangeParamsData params;
        params.Suspended = suspended;

        Notify(_T("statechange"), params);
    }

} // namespace Plugin

} // namespace WPEFramework
