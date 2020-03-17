/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
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

#ifndef UPNPDISCOVERYMANAGER_H
#define UPNPDISCOVERYMANAGER_H 

#include <plugins/plugins.h>
#include <cjson/cJSON.h>
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#include "libIARM.h"
#include "libIBus.h"
#include "iarmUtil.h"
#include "libIARMCore.h"

#include "sysMgr.h"
}
#endif

//#include <functional>
#include <string>

class CUpnpDiscoveryManager
{
public:
    static CUpnpDiscoveryManager* instance();

    void start();
    void stop();
    void register_deviceupdate_cb (std::function< void () > callback);
    JsonObject getDiscoveredDevices();
private:
    std::function < void () > onDeviceUpdateCallback;
    //std::vector<std::shared_ptr<std::function < void () >>> onDeviceUpdateCallback;

    bool m_isActive;
    JsonObject m_upnpJSONResults;

    void requestUpnpDeviceList();
    void saveUpdatedDiscoveredDevices(JsonObject upnpJSONResults);
    static void monitorUpnpEvents(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
};

#endif



