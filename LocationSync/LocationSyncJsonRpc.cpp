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
#include <interfaces/json/JsonData_LocationSync.h>
#include "LocationSync.h"

namespace WPEFramework {

namespace Plugin {

    using namespace JsonData::LocationSync;

    // Registration
    //

    void LocationSync::RegisterAll()
    {
        PluginHost::JSONRPC::Register<void,void>(_T("sync"), &LocationSync::endpoint_sync, this);
        PluginHost::JSONRPC::Property<LocationData>(_T("location"), &LocationSync::get_location, nullptr, this);
    }

    void LocationSync::UnregisterAll()
    {
        PluginHost::JSONRPC::Unregister(_T("sync"));
        PluginHost::JSONRPC::Unregister(_T("location"));
    }

    // API implementation
    //

    // Method: sync - Runs sync command
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_GENERAL: General error
    //  - ERROR_UNAVAILABLE: Unavailable locator
    //  - ERROR_INCORRECT_URL: Incorrect URL
    //  - ERROR_INPROGRESS: Probing in progress
    uint32_t LocationSync::endpoint_sync()
    {
        uint32_t result = Core::ERROR_NONE;

        if (_source.empty() == false) {
            result = _sink.Probe(_source, 1, 1);
        } else {
            result = Core::ERROR_GENERAL;
        }

        return result;
    }

    // Property: location - Location information
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_UNAVAILABLE: Internet/Location info not available
    uint32_t LocationSync::get_location(LocationData& response) const
    {
        uint32_t status = Core::ERROR_UNAVAILABLE;
        PluginHost::ISubSystem* subSystem = _service->SubSystems();
        ASSERT(subSystem != nullptr);

        const PluginHost::ISubSystem::IInternet* internet(subSystem->Get<PluginHost::ISubSystem::IInternet>());
        const PluginHost::ISubSystem::ILocation* location(subSystem->Get<PluginHost::ISubSystem::ILocation>());

        if ((internet != nullptr) && (location != nullptr)) {
            response.Publicip = internet->PublicIPAddress();

            response.Timezone = location->TimeZone();
            response.Region = location->Region();
            response.Country = location->Country();
            response.City = location->City();
            status = Core::ERROR_NONE;
        }

        return status;
    }

    // Event: locationchange - Signals a location change
    void LocationSync::event_locationchange()
    {
        Notify(_T("locationchange"));
    }

} // namespace Plugin

}

