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
    uint32_t DeviceInfo::get_firmwareversion(JsonData::DeviceInfo::FirmwareversionData& response) const
    {
        FirmwareVersion(response);
        return Core::ERROR_NONE;
    }

    // Property: serialnumber - Serial number set by manufacturer
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t DeviceInfo::get_serialnumber(JsonData::DeviceInfo::SerialnumberData& response) const
    {
        SerialNumber(response.Serialnumber);
        return Core::ERROR_NONE;
    }

    // Property: modelid - Device model number or SKU
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t DeviceInfo::get_modelid(JsonData::DeviceInfo::ModelidData& response) const
    {
        Sku(response.Sku);
        return Core::ERROR_NONE;
    }

    // Property: make - Device manufacturer
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t DeviceInfo::get_make(JsonData::DeviceInfo::MakeData& response) const
    {
        Make(response.Make);
        return Core::ERROR_NONE;
    }

    // Property: modelname - Friendly device model name
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t DeviceInfo::get_modelname(JsonData::DeviceInfo::ModelnameData& response) const
    {
        Model(response.Model);
        return Core::ERROR_NONE;
    }

    // Property: devicetype - Device type
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t DeviceInfo::get_devicetype(JsonData::DeviceInfo::DevicetypeData& response) const
    {
        DeviceType(response.Devicetype);
        return Core::ERROR_NONE;
    }

    // Property: distributorid - Partner ID or distributor ID for device
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t DeviceInfo::get_distributorid(JsonData::DeviceInfo::DistributoridData& response) const
    {
        DistributorId(response.Distributorid);
        return Core::ERROR_NONE;
    }

} // namespace Plugin

}
