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
        Property<VideoPlaybackResolutionData>(_T("getCurrentResolution"), &DisplayInfo::getCurrentResolutionJrpc, nullptr, this);
        Property<VideoOutputPortNameData>(_T("getConnectedVideoDisplays"), &DisplayInfo::getConnectedVideoDisplayJrpc, nullptr, this);
        Property<TvHdrCapabilitesData>(_T("getTVHDRCapabilities"), &DisplayInfo::getTVHDRCapabilitiesJrpc, nullptr, this);
        Property<StbHdrCapabilitiesData>(_T("getSettopHDRSupport"), &DisplayInfo::getSettopHDRSupportJrpc, nullptr, this);
        Property<OutputHdrStatusData>(_T("IsOutputHDR"), &DisplayInfo::IsOutputHDRJrpc, nullptr, this);
        Property<HdmiPreferencesData>(_T("HdmiPreferences"), &DisplayInfo::getHdmiPreferencesJrpc, &DisplayInfo::setHdmiPreferencesJrpc, this);
        Property<AudioEquivalenceStatusData>(_T("isAudioEquivalenceEnabled"), &DisplayInfo::isAudioEquivalenceEnabledJrpc, nullptr, this);
        Property<EDIDData>(_T("readEDID"), &DisplayInfo::readEDIDJrpc, nullptr, this);
    }

    void DisplayInfo::UnregisterAll()
    {
        Unregister(_T("displayinfo"));
        Unregister(_T("getCurrentResolution"));
        Unregister(_T("getConnectedVideoDisplays"));
        Unregister(_T("getTVHDRCapabilities"));
        Unregister(_T("getSettopHDRSupport"));
        Unregister(_T("IsOutputHDR"));
        Unregister(_T("HdmiPreferences"));
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

    uint32_t DisplayInfo::getCurrentResolutionJrpc(VideoPlaybackResolutionData& response) const
    {
        getCurrentResolution(response);
        return Core::ERROR_NONE;
    }

    uint32_t DisplayInfo::getConnectedVideoDisplayJrpc(VideoOutputPortNameData& response) const
    {
        getConnectedVideoDisplay(response);
        return Core::ERROR_NONE;
    }

    uint32_t DisplayInfo::getTVHDRCapabilitiesJrpc(TvHdrCapabilitesData& response) const
    {
        getTVHDRCapabilities(response);
        return Core::ERROR_NONE;
    }

    uint32_t DisplayInfo::getSettopHDRSupportJrpc(StbHdrCapabilitiesData& response) const
    {
        getSettopHDRSupport(response);
        return Core::ERROR_NONE;
    }

    uint32_t DisplayInfo::IsOutputHDRJrpc(OutputHdrStatusData& response) const
    {
        IsOutputHDR(response);
        return Core::ERROR_NONE;
    }

    uint32_t DisplayInfo::getHdmiPreferencesJrpc(HdmiPreferencesData& response) const
    {
        getHdmiPreferences(response);
        return Core::ERROR_NONE;
    }

    uint32_t DisplayInfo::setHdmiPreferencesJrpc(const HdmiPreferencesData& param)
    {
        setHdmiPreferences(param);
        return Core::ERROR_NONE;
    }

    uint32_t DisplayInfo::isAudioEquivalenceEnabledJrpc(AudioEquivalenceStatusData& response) const
    {
        isAudioEquivalenceEnabled(response);
        return Core::ERROR_NONE;
    }

    uint32_t DisplayInfo::readEDIDJrpc(EDIDData& response) const
    {
        readEDID(response);
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
       ResolutionChangedParamsData params;
       params.Height = height;
       params.Width = width;
       params.VideoOutputPort = videoOutputPort;
       params.Resolution = resolution;
       Notify(_T("ResolutionPostChange"), params);
   }


} // namespace Plugin

}

