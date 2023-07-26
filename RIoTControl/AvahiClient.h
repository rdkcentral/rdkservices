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
#include <mutex>
#include <condition_variable>
#include <iostream>

namespace avahi
{
    extern const std::string RDK_SERVICE_NAME;
    const int DD_TIMEOUT_MILLIS = 5000;
    const int DD_SCAN_ALREADY_IN_PROGRESS = -1;

    enum IP_ADDR_TYPE
    {
        IPV4,
        IPV6
    };
    typedef struct _RDKDevice
    {

        _RDKDevice()
        {
            std::cout << "Device created." << std::endl;
        }
        ~_RDKDevice()
        {
            std::cout << "Device removed." << std::endl;
        }
        // No copying
        _RDKDevice(const _RDKDevice &) = delete;
        _RDKDevice &operator=(const _RDKDevice &) = delete;

        std::string ipAddress;
        std::string deviceName;
        IP_ADDR_TYPE addrType;
        uint port;
    } RDKDevice;

    bool initialize(const std::string &serviceName = RDK_SERVICE_NAME);
    int discoverDevices(std::list<std::shared_ptr<RDKDevice> > &devices, int timeoutMillis = DD_TIMEOUT_MILLIS);
    bool unInitialize();
    bool stopDiscovery();
}