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
 
#include "DeviceIdentification.h"

namespace WPEFramework {

namespace Plugin {

    using namespace JsonData::DeviceIdentification;

    // Registration
    //

    void DeviceIdentification::RegisterAll()
    {
        Property<DeviceidentificationData>(_T("deviceidentification"), &DeviceIdentification::get_deviceidentification, nullptr, this);
    }

    void DeviceIdentification::UnregisterAll()
    {
        Unregister(_T("deviceidentification"));
    }

    // API implementation
    //

    // Property: deviceidentification - Device paltform specific information
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t DeviceIdentification::get_deviceidentification(DeviceidentificationData& response) const
    {
        Info(response);

        return Core::ERROR_NONE;
    }

} // namespace Plugin

}

