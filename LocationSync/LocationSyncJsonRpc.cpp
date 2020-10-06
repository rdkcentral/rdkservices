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
 
#include <interfaces/json/JsonData_LocationSync.h>
#include "LocationSync.h"
#include "Module.h"

namespace WPEFramework {

namespace Plugin {

    using namespace JsonData::LocationSync;

    // Registration
    //

    void LocationSync::RegisterAll()
    {
        Register<void,void>(_T("sync"), &LocationSync::endpoint_sync, this);
        Property<LocationData>(_T("location"), &LocationSync::get_location, nullptr, this);

        Register(_T("getTimeZones"), &LocationSync::getTimeZones, this);
    }

    void LocationSync::UnregisterAll()
    {
        Unregister(_T("sync"));
        Unregister(_T("location"));
        Unregister(_T("getTimeZones"));
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
    uint32_t LocationSync::get_location(LocationData& response) const
    {
        PluginHost::ISubSystem* subSystem = _service->SubSystems();
        ASSERT(subSystem != nullptr);

        const PluginHost::ISubSystem::IInternet* internet(subSystem->Get<PluginHost::ISubSystem::IInternet>());
        const PluginHost::ISubSystem::ILocation* location(subSystem->Get<PluginHost::ISubSystem::ILocation>());

        ASSERT(internet != nullptr);
        ASSERT(location != nullptr);

        response.Publicip = internet->PublicIPAddress();
        response.Timezone = location->TimeZone();
        response.Region = location->Region();
        response.Country = location->Country();
        response.City = location->City();

        return Core::ERROR_NONE;
    }

    // Event: locationchange - Signals a location change
    void LocationSync::event_locationchange()
    {
        Notify(_T("locationchange"));
    }

} // namespace Plugin

}

