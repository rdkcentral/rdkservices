/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
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
 **/

#pragma once
#include <string>
#include <list>
namespace WPEFramework
{
    namespace iotbridge
    {
        enum IOT_DEVICE_TYPE
        {
            CAMERA,
            LIGHT_BULB
        };

        typedef struct _IOTDevice
        {
            std::string deviceName;
            std::string deviceId;
            IOT_DEVICE_TYPE devType;
        } IOTDevice;

        int getDeviceList(std::list<std::shared_ptr<IOTDevice> > &deviceList);
        int getDeviceProperties(std::shared_ptr<IOTDevice> iotDevice, std::list<std::string> &propList);
        int getDeviceProperty(std::shared_ptr<IOTDevice> iotDevice, const std::string &propertyName);
    }
}
