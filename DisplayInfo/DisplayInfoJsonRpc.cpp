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

#include "DisplayInfo.h"

namespace WPEFramework {

namespace Plugin {

    using namespace JsonData::DisplayInfo;

    // Registration
    //

    void DisplayInfo::RegisterAll()
    {
        Property<DisplayinfoData>(_T("displayinfo"), &DisplayInfo::get_displayinfo, nullptr, this);
        Register(_T("getCurrentResolution"), &DisplayInfo::getCurrentResolution, this);
        Register(_T("getConnectedVideoDisplays"), &DisplayInfo::getConnectedVideoDisplays, this);
        Register(_T("getTVHDRCapabilities"), &DisplayInfo::getTVHDRCapabilities, this);
        Register(_T("getSettopHDRSupport"), &DisplayInfo::getSettopHDRSupport, this);
        Register(_T("IsOutputHDR"), &DisplayInfo::IsOutputHDR, this);
        Register(_T("setHdmiPreferences"), &DisplayInfo::setHdmiPreferences, this);
        Register(_T("getHdmiPreferences"), &DisplayInfo::getHdmiPreferences, this);
        Register(_T("isAudioEquivalenceEnabled"), &DisplayInfo::isAudioEquivalenceEnabled, this);
        Register(_T("readEDID"), &DisplayInfo::readEDID, this);
    }

    void DisplayInfo::UnregisterAll()
    {
        Unregister(_T("displayinfo"));
        Unregister(_T("getCurrentResolution"));
        Unregister(_T("getConnectedVideoDisplays"));
        Unregister(_T("getTVHDRCapabilities"));
        Unregister(_T("getSettopHDRSupport"));
        Unregister(_T("IsOutputHDR"));
        Unregister(_T("setHdmiPreferences"));
        Unregister(_T("getHdmiPreferences"));
        Unregister(_T("isAudioEquivalenceEnabled"));
        Unregister(_T("readEDID"));
    }

    // API implementation
    //

    // Property: displayinfo - Display general information
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t DisplayInfo::get_displayinfo(DisplayinfoData& response) const
    {
        Info(response);
        return Core::ERROR_NONE;
    }

    // Event: updated - Notifies about a change/update in the connection
   void DisplayInfo::event_updated()
   {
        Notify(_T("updated"));
   }
} // namespace Plugin

}

