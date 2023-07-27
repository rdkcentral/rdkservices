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

#include "RIoTControl.h"
#include "UtilsJsonRpc.h"
#include "AvahiClient.h"
#include "RIoTConnector.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework
{
    namespace
    {

        static Plugin::Metadata<Plugin::RIoTControl> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {});
    }

    namespace Plugin
    {

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
                avahi::unInitialize();
            }
            else
            {
                std::cout << " Failed to initialize avahi " << std::endl;
            }
            return "";
        }

        SERVICE_REGISTRATION(RIoTControl, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        RIoTControl::RIoTControl()
            : PluginHost::JSONRPC(), m_apiVersionNumber(API_VERSION_NUMBER_MAJOR)
        {

            Register("getAvailableDevices", &RIoTControl::getAvailableDevicesWrapper, this);
            Register("getDeviceProperties", &RIoTControl::getDeviceProperties, this);
            Register("getDeviceProperty", &RIoTControl::getDeviceProperty, this);
            Register("sendCommand", &RIoTControl::sendCommand, this);
        }

        RIoTControl::~RIoTControl()
        {
        }

        const string RIoTControl::Initialize(PluginHost::IShell * /* service */)
        {
            string msg;
            remote_addr = getRemoteAddress();
            if(remote_addr.empty())
                msg = "Failed to retrieve remote address";
            return (msg);
        }
        void RIoTControl::Deinitialize(PluginHost::IShell * /* service */)
        {
        }
        string RIoTControl::Information() const
        {
            return (string());
        }

        // Supported methods
        uint32_t RIoTControl::getAvailableDevicesWrapper(const JsonObject &parameters, JsonObject &response)
        {
            bool success = false;
            if (!remote_addr.empty() && iotbridge::initializeIPC(remote_addr))
            {
                std::list<std::shared_ptr<WPEFramework::iotbridge::IOTDevice> > deviceList;
                JsonArray devArray;
                if (iotbridge::getDeviceList(deviceList) > 0)
                {

                    for (const auto &device : deviceList)
                    {
                        JsonObject deviceObj;
                        deviceObj["name"] = device->deviceName;
                        deviceObj["uuid"] = device->deviceId;
                        deviceObj["type"] = (device->devType == WPEFramework::iotbridge::IOT_DEVICE_TYPE::CAMERA) ? "Camera" : "Bulb";
                        devArray.Add(deviceObj);
                    }
                }
                response["devices"] = devArray;
                success = true;
                iotbridge::cleanupIPC();
            }
            else
            {
                response["message"] = "Failed to connect to IoT Gateway";
            }
            returnResponse(success);
        }

        uint32_t RIoTControl::getDeviceProperties(const JsonObject &parameters, JsonObject &response)
        {
            bool success = false;

            returnResponse(success);
        }
        uint32_t RIoTControl::getDeviceProperty(const JsonObject &parameters, JsonObject &response)
        {
            bool success = false;
            if (parameters.HasLabel("deviceId") && parameters.HasLabel("propName"))
            {
                response["message"] = "Missing parameters deviceId or propName";
            }
            else
            {
                if (!remote_addr.empty() && iotbridge::initializeIPC(remote_addr))
                {
                    std::string uuid = parameters["deviceId"].String();
                    std::string prop = parameters["propName"].String();
                    std::string propVal = iotbridge::getDeviceProperty(uuid, prop);
                    response["value"] = propVal;
                    success = true;
                    iotbridge::cleanupIPC();
                }
            }
            returnResponse(success);
        }
        uint32_t RIoTControl::sendCommand(const JsonObject &parameters, JsonObject &response)
        {
            bool success = false;
            if (parameters.HasLabel("deviceId") && parameters.HasLabel("command"))
            {
                response["message"] = "Missing parameters deviceId or command";
            }
            else
            {
                if (!remote_addr.empty() && iotbridge::initializeIPC(remote_addr))
                {
                    std::string uuid = parameters["deviceId"].String();
                    std::string cmd = parameters["command"].String();
                    if (0 == iotbridge::sendCommand(uuid, cmd))
                        success = true;

                    iotbridge::cleanupIPC();
                }
            }
            returnResponse(success);
        }

    } // namespace Plugin
} // namespace WPEFramework