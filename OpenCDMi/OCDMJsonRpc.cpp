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
#include "OCDM.h"
#include <interfaces/json/JsonData_OCDM.h>

namespace WPEFramework {

namespace Plugin {

    using namespace JsonData::OCDM;

    // Registration
    //

    void OCDM::RegisterAll()
    {
        Property<Core::JSON::ArrayType<DrmData>>(_T("drms"), &OCDM::get_drms, nullptr, this);
        Property<Core::JSON::ArrayType<Core::JSON::String>>(_T("keysystems"), &OCDM::get_keysystems, nullptr, this);
        Property<Core::JSON::ArrayType<SessionInfo>>(_T("sessions"), &OCDM::get_sessions, nullptr, this);
    }

    void OCDM::UnregisterAll()
    {
        Unregister(_T("keysystems"));
        Unregister(_T("drms"));
        Unregister(_T("sessions"));
    }

    bool OCDM::KeySystems(const string& name, Core::JSON::ArrayType<Core::JSON::String>& response) const
    {
        bool result = false;
        RPC::IStringIterator* keySystemsIter(_opencdmi->Designators(name));
        if (keySystemsIter != nullptr) {
            string element;
            while (keySystemsIter->Next(element) == true) {
                Core::JSON::String designator;
                designator = element;

                response.Add(designator);
            }

            keySystemsIter->Release();
            result = true;
        }

        return result;
    }

    // API implementation
    //

    // Property: drms - Supported DRM systems
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t OCDM::get_drms(Core::JSON::ArrayType<DrmData>& response) const
    {
        RPC::IStringIterator* drmsIter(_opencdmi->Systems());
        if (drmsIter != nullptr) {
            string element;
            while (drmsIter->Next(element) == true) {
                DrmData drm;
                drm.Name = element;
                KeySystems(element, drm.Keysystems);
                response.Add(drm);
            }

            drmsIter->Release();
        }

        return Core::ERROR_NONE;
    }

    // Property: keysystems - DRM key systems
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_BAD_REQUEST: Invalid DRM name
    uint32_t OCDM::get_keysystems(const string& index, Core::JSON::ArrayType<Core::JSON::String>& response) const
    {
        uint32_t result = Core::ERROR_BAD_REQUEST;

        if (KeySystems(index, response)) {
            result = Core::ERROR_NONE;
        }

        return result;
    }

    uint32_t OCDM::get_sessions(Core::JSON::ArrayType<SessionInfo>& response) const
    {
        RPC::IStringIterator* drmsIter(_opencdmi->Systems());
        if (drmsIter != nullptr) {
            string system;
            while (drmsIter->Next(system)) {
                RPC::IStringIterator * sessionsIter(_opencdmi->Sessions(system));
                string sessionItem;
                if (sessionsIter != nullptr) {
                    while (sessionsIter->Next(sessionItem)) {
                        SessionInfo session;
                        session.Drm = system;
                        response.Add(session);
                    }
                    sessionsIter->Release();
                }
            }
            drmsIter->Release();
        }
        return Core::ERROR_NONE;
    }
} // namespace Plugin

}

