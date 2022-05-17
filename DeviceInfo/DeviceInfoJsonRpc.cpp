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

#include "DeviceInfo.h"

namespace WPEFramework {

namespace Plugin {

    using namespace JsonData::DeviceInfo;

    // Registration
    //

    void DeviceInfo::RegisterAll()
    {
        Property<SysteminfoData>(_T("systeminfo"), &DeviceInfo::get_systeminfo, nullptr, this);
        Property<Core::JSON::ArrayType<AddressesData>>(_T("addresses"), &DeviceInfo::get_addresses, nullptr, this);
        Property<SocketinfoData>(_T("socketinfo"), &DeviceInfo::get_socketinfo, nullptr, this);
        Property<FirmwareversionData>(_T("firmwareversion"), &DeviceInfo::get_firmwareversion, nullptr, this);
        Property<SerialnumberData>(_T("serialnumber"), &DeviceInfo::get_serialnumber, nullptr, this);
        Property<ModelidData>(_T("modelid"), &DeviceInfo::get_modelid, nullptr, this);
        Property<MakeData>(_T("make"), &DeviceInfo::get_make, nullptr, this);
        Property<ModelnameData>(_T("modelname"), &DeviceInfo::get_modelname, nullptr, this);
        Property<DevicetypeData>(_T("devicetype"), &DeviceInfo::get_devicetype, nullptr, this);
        Property<DistributoridData>(_T("distributorid"), &DeviceInfo::get_distributorid, nullptr, this);
        Property<SupportedaudioportsData>(_T("supportedaudioports"), &DeviceInfo::get_supportedaudioports, nullptr, this);
        Property<SupportedvideodisplaysData>(_T("supportedvideodisplays"), &DeviceInfo::get_supportedvideodisplays, nullptr, this);
        Property<HostedidData>(_T("hostedid"), &DeviceInfo::get_hostedid, nullptr, this);
        Register<SupportedresolutionsParamsInfo, DefaultresolutionResultData>(_T("defaultresolution"), &DeviceInfo::endpoint_defaultresolution, this);
        Register<SupportedresolutionsParamsInfo, SupportedresolutionsResultData>(_T("supportedresolutions"), &DeviceInfo::endpoint_supportedresolutions, this);
        Register<SupportedresolutionsParamsInfo, SupportedhdcpResultData>(_T("supportedhdcp"), &DeviceInfo::endpoint_supportedhdcp, this);
        Register<SupportedresolutionsParamsInfo, DisplaysettingsResultData>(_T("displaysettings"), &DeviceInfo::endpoint_displaysettings, this);
        Register<AudiocapabilitiesParamsInfo, AudiocapabilitiesResultData>(_T("audiocapabilities"), &DeviceInfo::endpoint_audiocapabilities, this);
        Register<AudiocapabilitiesParamsInfo, Ms12capabilitiesResultData>(_T("ms12capabilities"), &DeviceInfo::endpoint_ms12capabilities, this);
        Register<AudiocapabilitiesParamsInfo, Supportedms12audioprofilesResultData>(_T("supportedms12audioprofiles"), &DeviceInfo::endpoint_supportedms12audioprofiles, this);
    }

    void DeviceInfo::UnregisterAll()
    {
        Unregister(_T("socketinfo"));
        Unregister(_T("addresses"));
        Unregister(_T("systeminfo"));
        Unregister(_T("firmwareversion"));
        Unregister(_T("serialnumber"));
        Unregister(_T("modelid"));
        Unregister(_T("make"));
        Unregister(_T("modelname"));
        Unregister(_T("devicetype"));
        Unregister(_T("distributorid"));
        Unregister(_T("supportedaudioports"));
        Unregister(_T("supportedvideodisplays"));
        Unregister(_T("hostedid"));
        Unregister(_T("defaultresolution"));
        Unregister(_T("supportedresolutions"));
        Unregister(_T("supportedhdcp"));
        Unregister(_T("displaysettings"));
        Unregister(_T("audiocapabilities"));
        Unregister(_T("ms12capabilities"));
        Unregister(_T("supportedms12audioprofiles"));
    }

    // API implementation
    //

    // Property: systeminfo - System general information
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t DeviceInfo::get_systeminfo(SysteminfoData& response) const
    {
        SysInfo(response);
        return Core::ERROR_NONE;
    }

    // Property: addresses - Network interface addresses
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t DeviceInfo::get_addresses(Core::JSON::ArrayType<AddressesData>& response) const
    {
        AddressInfo(response);
        return Core::ERROR_NONE;
    }

    // Property: socketinfo - Socket information
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t DeviceInfo::get_socketinfo(SocketinfoData& response) const
    {
        SocketPortInfo(response);
        return Core::ERROR_NONE;
    }

    // Property: firmwareversion - Versions maintained in version.txt
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_GENERAL: General error
    uint32_t DeviceInfo::get_firmwareversion(FirmwareversionData& response) const
    {
        return FirmwareVersion(response);
    }

    // Property: serialnumber - Serial number set by manufacturer
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_GENERAL: General error
    uint32_t DeviceInfo::get_serialnumber(SerialnumberData& response) const
    {
        return SerialNumber(response.Serialnumber);
    }

    // Property: modelid - Device model number or SKU
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_GENERAL: General error
    uint32_t DeviceInfo::get_modelid(ModelidData& response) const
    {
        return Sku(response.Sku);
    }

    // Property: make - Device manufacturer
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_GENERAL: General error
    uint32_t DeviceInfo::get_make(MakeData& response) const
    {
        return Make(response.Make);
    }

    // Property: modelname - Friendly device model name
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_GENERAL: General error
    uint32_t DeviceInfo::get_modelname(ModelnameData& response) const
    {
        return Model(response.Model);
    }

    // Property: devicetype - Device type
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_GENERAL: General error
    uint32_t DeviceInfo::get_devicetype(DevicetypeData& response) const
    {
        return DeviceType(response.Devicetype);
    }

    // Property: distributorid - Partner ID or distributor ID for device
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_GENERAL: General error
    uint32_t DeviceInfo::get_distributorid(DistributoridData& response) const
    {
        return DistributorId(response.Distributorid);
    }

    // Property: supportedaudioports - Audio ports supported on the device (all ports that are physically present)
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_GENERAL: General error
    uint32_t DeviceInfo::get_supportedaudioports(SupportedaudioportsData& response) const
    {
        return SupportedAudioPorts(response.SupportedAudioPorts);
    }

    // Property: supportedvideodisplays - Video ports supported on the device (all ports that are physically present)
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_GENERAL: General error
    uint32_t DeviceInfo::get_supportedvideodisplays(SupportedvideodisplaysData& response) const
    {
        return SupportedVideoDisplays(response.SupportedVideoDisplays);
    }

    // Property: hostedid - EDID of the host
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_GENERAL: General error
    uint32_t DeviceInfo::get_hostedid(JsonData::DeviceInfo::HostedidData& response) const
    {
        return HostEDID(response.EDID);
    }

    // Method: defaultresolution - Default resolution on the selected video display port
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_GENERAL: General error
    uint32_t DeviceInfo::endpoint_defaultresolution(const SupportedresolutionsParamsInfo& params, DefaultresolutionResultData& response) const
    {
        return DefaultResolution(params.VideoDisplay.Value(), response.DefaultResolution);
    }

    // Method: supportedresolutions - Supported resolutions on the selected video display port
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_GENERAL: General error
    uint32_t DeviceInfo::endpoint_supportedresolutions(const SupportedresolutionsParamsInfo& params, SupportedresolutionsResultData& response) const
    {
        return SupportedResolutions(params.VideoDisplay.Value(), response.SupportedResolutions);
    }

    // Method: supportedhdcp - Supported HDCP version on the selected video display port
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_GENERAL: General error
    uint32_t DeviceInfo::endpoint_supportedhdcp(const SupportedresolutionsParamsInfo& params, SupportedhdcpResultData& response) const
    {
        return SupportedHdcp(params.VideoDisplay.Value(), response.SupportedHDCPVersion);
    }

    // Method: displaysettings - Current output settings on the selected video display port
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_GENERAL: General error
    uint32_t DeviceInfo::endpoint_displaysettings(const JsonData::DeviceInfo::SupportedresolutionsParamsInfo& params, JsonData::DeviceInfo::DisplaysettingsResultData& response) const
    {
        return DisplaySettings(params.VideoDisplay.Value(), response.ColorSpace, response.ColorDepth, response.MatrixCoefficients, response.VideoEOTF, response.QuantizationRange);
    }

    // Method: audiocapabilities - Audio capabilities for the specified audio port
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_GENERAL: General error
    uint32_t DeviceInfo::endpoint_audiocapabilities(const JsonData::DeviceInfo::AudiocapabilitiesParamsInfo& params, JsonData::DeviceInfo::AudiocapabilitiesResultData& response) const
    {
        return AudioCapabilities(params.AudioPort.Value(), response.AudioCapabilities);
    }

    // Method: ms12capabilities - MS12 audio capabilities for the specified audio port
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_GENERAL: General error
    uint32_t DeviceInfo::endpoint_ms12capabilities(const JsonData::DeviceInfo::AudiocapabilitiesParamsInfo& params, JsonData::DeviceInfo::Ms12capabilitiesResultData& response) const
    {
        return MS12Capabilities(params.AudioPort.Value(), response.MS12Capabilities);
    }

    // Method: supportedms12audioprofiles - Supported MS12 audio profiles for the specified audio port
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_GENERAL: General error
    uint32_t DeviceInfo::endpoint_supportedms12audioprofiles(const JsonData::DeviceInfo::AudiocapabilitiesParamsInfo& params, JsonData::DeviceInfo::Supportedms12audioprofilesResultData& response) const
    {
        return SupportedMS12AudioProfiles(params.AudioPort.Value(), response.SupportedMS12AudioProfiles);
    }

} // namespace Plugin

}
