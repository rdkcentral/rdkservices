/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2022 Sky UK
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

#include <string>
#include "ControlSettingsTV.h"

namespace WPEFramework {
namespace Plugin {

    ControlSettingsTV::ControlSettingsTV()
    {
        LOGINFO("Entry\n");
        registerMethod("getBacklight", &ControlSettingsTV::getBacklight, this, {2});
        registerMethod("setBacklight", &ControlSettingsTV::setBacklight, this, {2});	
        LOGINFO("Exit\n");
    }
    
    ControlSettingsTV :: ~ControlSettingsTV()
    {
        LOGINFO();    
    }

    void ControlSettingsTV::Initialize()
    {
        LOGINFO();
	//Platform specific Init Sequence
    }

    void ControlSettingsTV::DeInitialize()
    {
        LOGINFO();
    }

    void ControlSettingsTV::AddRef() const
    {
        LOGINFO("Entry\n");
        LOGINFO("Exit\n");
    }

    uint32_t ControlSettingsTV::Release() const
    {
        return 0;
    }

    uint32_t ControlSettingsTV::getBacklight(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry %s\n",__FUNCTION__);
        PLUGIN_Lock(tvLock);
        tvError_t ret = tvERROR_NONE;

      //  ret = devicePtr->getBacklight();

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            LOGINFO("Exit : %s\n",__FUNCTION__);
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::setBacklight(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        tvError_t ret = tvERROR_NONE;

    //    ret = devicePtr->setBacklight();

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            LOGINFO("Exit : %s\n",__FUNCTION__);
            returnResponse(true, "success");
        }
    }
}//namespace Plugin
}//namespace WPEFramework
