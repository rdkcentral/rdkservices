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
        Property<VideoplaybackresolutionData>(_T("getcurrentresolution"), &DisplayInfo::getCurrentResolutionJrpc, nullptr, this);
        Property<VideooutputportnameData>(_T("getconnectedvideodisplays"), &DisplayInfo::getConnectedVideoDisplayJrpc, nullptr, this);
        Property<TvhdrcapabilitiesData>(_T("gettvhdrcapabilities"), &DisplayInfo::getTVHDRCapabilitiesJrpc, nullptr, this);
        Property<StbhdrcapabilitiesData>(_T("getsettophdrcapabilities"), &DisplayInfo::getSettopHDRSupportJrpc, nullptr, this);
        Property<OutputhdrstatusData>(_T("isoutputhdr"), &DisplayInfo::IsOutputHDRJrpc, nullptr, this);
        Property<HdmipreferencesData>(_T("hdmipreferences"), &DisplayInfo::getHdmiPreferencesJrpc, &DisplayInfo::setHdmiPreferencesJrpc, this);
        Property<AudioequivalencestatusData>(_T("isaudioequivalenceenabled"), &DisplayInfo::isAudioEquivalenceEnabledJrpc, nullptr, this);
        Property<EdidData>(_T("readedid"), &DisplayInfo::readEDIDJrpc, nullptr, this);
    }

    void DisplayInfo::UnregisterAll()
    {
        Unregister(_T("displayinfo"));
        Unregister(_T("getcurrentresolution"));
        Unregister(_T("getconnectedvideodisplays"));
        Unregister(_T("gettvhdrcapabilities"));
        Unregister(_T("getsettophdrcapabilities"));
        Unregister(_T("isoutputhdr"));
        Unregister(_T("hdmipreferences"));
        Unregister(_T("isaudioequivalenceenabled"));
        Unregister(_T("readedid"));
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

    uint32_t DisplayInfo::getCurrentResolutionJrpc(VideoplaybackresolutionData& response) const
    {
        GetCurrentResolution(response);
        return Core::ERROR_NONE;
    }

    uint32_t DisplayInfo::getConnectedVideoDisplayJrpc(VideooutputportnameData& response) const
    {
        GetConnectedVideoDisplay(response);
        return Core::ERROR_NONE;
    }

    uint32_t DisplayInfo::getTVHDRCapabilitiesJrpc(TvhdrcapabilitiesData& response) const
    {
        GetTvHdrCapabilities(response);
        return Core::ERROR_NONE;
    }

    uint32_t DisplayInfo::getSettopHDRSupportJrpc(StbhdrcapabilitiesData& response) const
    {
        GetStbHdrCapabilities(response);
        return Core::ERROR_NONE;
    }

    uint32_t DisplayInfo::IsOutputHDRJrpc(OutputhdrstatusData& response) const
    {
        IsOutputHDR(response);
        return Core::ERROR_NONE;
    }

    uint32_t DisplayInfo::getHdmiPreferencesJrpc(HdmipreferencesData& response) const
    {
        GetHdmiPreferences(response);
        return Core::ERROR_NONE;
    }

    uint32_t DisplayInfo::setHdmiPreferencesJrpc(const HdmipreferencesData& param)
    {
        SetHdmiPreferences(param);
        return Core::ERROR_NONE;
    }

    uint32_t DisplayInfo::isAudioEquivalenceEnabledJrpc(AudioequivalencestatusData& response) const
    {
        IsAudioEquivalenceEnabled(response);
        return Core::ERROR_NONE;
    }

    uint32_t DisplayInfo::readEDIDJrpc(EdidData& response) const
    {
        ReadEDID(response);
        return Core::ERROR_NONE;
    }

    // Event: updated - Notifies about a change/update in the connection
   void DisplayInfo::event_updated()
   {
        Notify(_T("updated"));
   }

    void DisplayInfo::resolutionPreChangedJrpc()
   {
       Notify(_T("ResolutionPreChange"));
   }

   void DisplayInfo::resolutionChangedJrpc(const int32_t& height, const int32_t& width, const string& videoOutputPort, const string& resolution)
   {
       ResolutionchangedParamsData params;
       params.Height = height;
       params.Width = width;
       params.Videooutputport = videoOutputPort;
       params.Resolution = resolution;
       Notify(_T("ResolutionPostChange"), params);
   }


} // namespace Plugin

}

