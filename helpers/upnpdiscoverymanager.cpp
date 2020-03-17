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

#include "upnpdiscoverymanager.h"

/* XUPNP Based Macros */
#define  _IARM_XUPNP_NAME                            "XUPnP" /*!< Method to Get the Xupnp Info */
#define  IARM_BUS_XUPNP_API_GetXUPNPDeviceInfo        "GetXUPNPDeviceInfo" /*!< Method to Get the Xupnp Info */

using namespace std::placeholders; 
static CUpnpDiscoveryManager s_instance;
std::function <void (JsonObject) >_postUPNPUpdateFuncPtr;


CUpnpDiscoveryManager* CUpnpDiscoveryManager::instance()
{
    return &s_instance;
}

void CUpnpDiscoveryManager::start()
{
    if (!m_isActive)
    {
        m_isActive = true;
        _postUPNPUpdateFuncPtr = std::bind(&CUpnpDiscoveryManager::saveUpdatedDiscoveredDevices, CUpnpDiscoveryManager::instance(), _1);
        IARM_Bus_RegisterEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_XUPNP_DATA_UPDATE, monitorUpnpEvents);
        requestUpnpDeviceList();
    }
}

void CUpnpDiscoveryManager::stop()
{
    if (m_isActive)
    {
        m_isActive = false;
        IARM_Bus_UnRegisterEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_XUPNP_DATA_UPDATE);
    }
}

void CUpnpDiscoveryManager::register_deviceupdate_cb (std::function< void () > callback)
{
    onDeviceUpdateCallback = callback;
//    onDeviceUpdateCallback.push_back(std::make_shared<std::function< void () >> callback);
}

JsonObject CUpnpDiscoveryManager::getDiscoveredDevices()
{
    if (m_isActive)
    {
        return m_upnpJSONResults;
    }
    return JsonObject();
}

void CUpnpDiscoveryManager::saveUpdatedDiscoveredDevices(JsonObject upnpJSONResults)
{
    if (m_isActive)
    {
        /* Update the member variable */
        m_upnpJSONResults = upnpJSONResults;

        /* Notify to listeners */
        if (onDeviceUpdateCallback != nullptr)
            onDeviceUpdateCallback();
    }
}

void CUpnpDiscoveryManager::requestUpnpDeviceList()
{
    IARM_Bus_SYSMgr_EventData_t eventData;
    eventData.data.xupnpData.deviceInfoLength = 0;
    IARM_Bus_BroadcastEvent(IARM_BUS_SYSMGR_NAME, (IARM_EventId_t)IARM_BUS_SYSMGR_EVENT_XUPNP_DATA_REQUEST,(void *)&eventData, sizeof(eventData));
}

void CUpnpDiscoveryManager::monitorUpnpEvents(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
    if ((strcmp(owner, IARM_BUS_SYSMGR_NAME) == 0) && (eventId == IARM_BUS_SYSMGR_EVENT_XUPNP_DATA_UPDATE))
    {
        IARM_Bus_SYSMgr_EventData_t *eventData = (IARM_Bus_SYSMgr_EventData_t*)data;
        int messageLength = eventData->data.xupnpData.deviceInfoLength;
        if (messageLength > 0)
        {
            char upnpResults[messageLength+1];
            IARM_Bus_SYSMGR_GetXUPNPDeviceInfo_Param_t *param = NULL;
            IARM_Result_t ret = IARM_RESULT_IPCCORE_FAIL;

            /* Allocate enough to store the structure, the message and one more byte for the string termintor */
            IARM_Malloc(IARM_MEMTYPE_PROCESSLOCAL, sizeof(IARM_Bus_SYSMGR_GetXUPNPDeviceInfo_Param_t) + messageLength + 1, (void**)&param);
            param->bufLength = messageLength;

            ret = IARM_Bus_Call(_IARM_XUPNP_NAME,IARM_BUS_XUPNP_API_GetXUPNPDeviceInfo, (void *)param, sizeof(IARM_Bus_SYSMGR_GetXUPNPDeviceInfo_Param_t) + messageLength + 1);

            if(ret == IARM_RESULT_SUCCESS)
            {
                memcpy(upnpResults, ((char *)param + sizeof(IARM_Bus_SYSMGR_GetXUPNPDeviceInfo_Param_t)), param->bufLength);
                upnpResults[param->bufLength] = '\0';
                IARM_Free(IARM_MEMTYPE_PROCESSLOCAL, param);
            }

            /* Convert the message into JsonObject */
            if(ret == IARM_RESULT_SUCCESS)
            {
                JsonObject upnpJSONResults;
                upnpJSONResults.FromString(std::string (upnpResults));

                /* Notify the class */
                _postUPNPUpdateFuncPtr(upnpResults);
            }
        }
    }
}


