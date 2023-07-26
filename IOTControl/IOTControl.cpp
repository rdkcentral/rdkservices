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

#include "IOTControl.h"
#include "IOTConnector.h"
#include "UtilsJsonRpc.h"
#include "AvahiClient.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework
{
    // namespace
    // {

    //     static Plugin::Metadata<Plugin::IOTControl> metadata(
    //         // Version (Major, Minor, Patch)
    //         API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
    //         // Preconditions
    //         {},
    //         // Terminations
    //         {},
    //         // Controls
    //         {});
    // }

    namespace Plugin
    {

        const string METHOD_IOT_GET_AVAILABLE_DEVICES = "getAvailableDevices";
        const string METHOD_IOT_GET_DEVICE_DETAILS = "getDeviceDetails";
        const string METHOD_IOT_GET_DEVICE_PROPERTIES = "getDeviceProperties";
        const string METHOD_IOT_GET_DEVICE_PROPERTY = "getDeviceProperty";
        const string METHOD_IOT_SEND_COMMAND = "sendCommand";

        std::string getRemoteAddress()
        {
            if (avahi::initialize())
            {
                std::list<std::shared_ptr<avahi::RDKDevice> > devices;
                if (avahi::discoverDevices(devices) > 0)
                {
                    // Find the one with ipv4 address and return
                    for (std::list<std::shared_ptr<avahi::RDKDevice> >::iterator it = devices.begin(); it != devices.end(); ++it)
                    {
                        std::shared_ptr<avahi::RDKDevice> device = *it;
                        if (device->addrType == avahi::IP_ADDR_TYPE::IPV4)
                            return "tcp://" + device->ipAddress + ":" + std::to_string(device->port);
                    }
                }
            }
            return "";
        }

        SERVICE_REGISTRATION(IOTControl, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        IOTControl *IOTControl::_instance = nullptr;

        IOTControl::IOTControl()
            : PluginHost::JSONRPC(), m_apiVersionNumber(API_VERSION_NUMBER_MAJOR)
        {

            Register(METHOD_IOT_GET_AVAILABLE_DEVICES, &IOTControl::getAvailableDevicesWrapper, this);
            Register(METHOD_IOT_GET_DEVICE_DETAILS, &IOTControl::getDeviceDetailsWrapper, this);
            Register(METHOD_IOT_GET_DEVICE_PROPERTIES, &IOTControl::getDeviceProperties, this);
            Register(METHOD_IOT_GET_DEVICE_PROPERTY, &IOTControl::getDeviceProperty, this);
            Register(METHOD_IOT_SEND_COMMAND, &IOTControl::sendCommand, this);
        }

        IOTControl::~IOTControl()
        {
        }

        const string IOTControl::Initialize(PluginHost::IShell * /* service */)
        {
            IOTControl::_instance = this;
            return (string());
        }
        void IOTControl::Deinitialize(PluginHost::IShell * /* service */)
        {
        }
        string IOTControl::Information() const
        {
            return (string());
        }

        // Supported methods
        uint32_t IOTControl::getAvailableDevicesWrapper(const JsonObject &parameters, JsonObject &response)
        {
            bool success = false;

            returnResponse(success);
        }

        uint32_t IOTControl::getDeviceDetailsWrapper(const JsonObject &parameters, JsonObject &response)
        {
            bool success = false;

            returnResponse(success);
        }
        uint32_t IOTControl::getDeviceProperties(const JsonObject &parameters, JsonObject &response)
        {
            bool success = false;

            returnResponse(success);
        }
        uint32_t IOTControl::getDeviceProperty(const JsonObject &parameters, JsonObject &response)
        {
            bool success = false;

            returnResponse(success);
        }
        uint32_t IOTControl::sendCommand(const JsonObject &parameters, JsonObject &response)
        {
            bool success = false;

            returnResponse(success);
        }

    } // namespace Plugin
} // namespace WPEFramework