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
 
#include <interfaces/json/JsonData_Browser.h>
#include <interfaces/json/JsonData_StateControl.h>
#include "Spark.h"
#include "Module.h"

namespace WPEFramework {

namespace Plugin {

    using namespace JsonData::Browser;
    using namespace JsonData::StateControl;

    // Registration
    //

    void Spark::RegisterAll()
    {
        Property<Core::JSON::String>(_T("url"), &Spark::get_url, &Spark::set_url, this); /* Browser */
        Property<Core::JSON::EnumType<VisibilityType>>(_T("visibility"), &Spark::get_visibility, &Spark::set_visibility, this); /* Browser */
        Property<Core::JSON::DecUInt32>(_T("fps"), &Spark::get_fps, nullptr, this); /* Browser */
        Property<Core::JSON::EnumType<StateType>>(_T("state"), &Spark::get_state, &Spark::set_state, this); /* StateControl */

    }

    void Spark::UnregisterAll()
    {
        Unregister(_T("state"));
        Unregister(_T("fps"));
        Unregister(_T("visibility"));
        Unregister(_T("url"));
    }

    // API implementation
    //

   // Property: url - URL loaded in the browser
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t Spark::get_url(Core::JSON::String& response) const /* Browser */
    {
        ASSERT(_spark != nullptr);

        response = _spark->GetURL();

        return Core::ERROR_NONE;
    }

    // Property: url - URL loaded in the browser
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_INCORRECT_URL: Incorrect URL given
    uint32_t Spark::set_url(const Core::JSON::String& param) /* Browser */
    {
        ASSERT(_spark != nullptr);

        uint32_t result = Core::ERROR_INCORRECT_URL;

        if (param.IsSet() && !param.Value().empty()) {
            _spark->SetURL(param.Value());
            result = Core::ERROR_NONE;
        }

        return result;
    }

    // Property: visibility - Current browser visibility
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t Spark::get_visibility(Core::JSON::EnumType<VisibilityType>& response) const /* Browser */
    {
        response = (_hidden? VisibilityType::HIDDEN : VisibilityType::VISIBLE);

        return Core::ERROR_NONE;
    }

    // Property: visibility - Current browser visibility
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t Spark::set_visibility(const Core::JSON::EnumType<VisibilityType>& param) /* Browser */
    {
        ASSERT(_spark != nullptr);

        uint32_t result = Core::ERROR_BAD_REQUEST;

        if (param.IsSet()) {
            if (param == VisibilityType::VISIBLE) {
                _spark->Hide(true);
            }
            else {
                _spark->Hide(false);
            }

            result =  Core::ERROR_NONE;
        }

        return result;
    }

    // Property: fps - Current number of frames per second the browser is rendering
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t Spark::get_fps(Core::JSON::DecUInt32& response) const /* Browser */
    {
        ASSERT(_spark != nullptr);

        response = _spark->GetFPS();

        return Core::ERROR_NONE;
    }

    // Property: state - Running state of the service
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t Spark::get_state(Core::JSON::EnumType<StateType>& response) const /* StateControl */
    {
        ASSERT(_spark != nullptr);

        PluginHost::IStateControl* stateControl(_spark->QueryInterface<PluginHost::IStateControl>());
        
        // If this is running out-of-process, it might have crashed and thus return a nullptr
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
    uint32_t Spark::set_state(const Core::JSON::EnumType<StateType>& param) /* StateControl */
    {
        ASSERT(_spark != nullptr);

        uint32_t result = Core::ERROR_BAD_REQUEST;

        if (param.IsSet()) {
            PluginHost::IStateControl* stateControl(_spark->QueryInterface<PluginHost::IStateControl>());
            
            // If this is running out-of-process, it might have crashed and thus return a nullptr
            if (stateControl != nullptr) {

                stateControl->Request(param == StateType::SUSPENDED? PluginHost::IStateControl::SUSPEND : PluginHost::IStateControl::RESUME);

                stateControl->Release();
            }
            
            result = Core::ERROR_NONE;
        }

        return result;
    }

    // Event: urlchange - Signals a URL change in the browser
    void Spark::event_urlchange(const string& url, const bool& loaded) /* Browser */
    {
        UrlchangeParamsData params;
        params.Url = url;
        params.Loaded = loaded;

        Notify(_T("urlchange"), params);
    }

    // Event: visibilitychange - Signals a visibility change of the browser
    void Spark::event_visibilitychange(const bool& hidden) /* Browser */
    {
        VisibilitychangeParamsData params;
        params.Hidden = hidden;

        Notify(_T("visibilitychange"), params);
    }

    // Event: statechange - Signals a state change of the service
    void Spark::event_statechange(const bool& suspended) /* StateControl */
    {
        StatechangeParamsData params;
        params.Suspended = suspended;

        Notify(_T("statechange"), params);
    }

} // namespace Plugin

} // namespace WPEFramework
