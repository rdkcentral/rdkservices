/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2024 Sky UK
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
#include "AVOutputSTB.h"

#define registerMethod(...) for (uint8_t i = 1; GetHandler(i); i++) GetHandler(i)->Register<JsonObject, JsonObject>(__VA_ARGS__)

namespace WPEFramework {
namespace Plugin {

    AVOutputSTB* AVOutputSTB::instance = nullptr;

    AVOutputSTB::AVOutputSTB()
    {
        LOGINFO("Entry\n"); 
        instance = this;

        registerMethod("getVolume", &AVOutputSTB::getVolume, this );
        registerMethod("setVolume", &AVOutputSTB::setVolume, this);

        LOGINFO("Exit\n");
    }
    
    AVOutputSTB :: ~AVOutputSTB()
    {
        LOGINFO();
    }

    uint32_t AVOutputSTB::getVolume(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
        LOGINFO("Exit : %s\n",__FUNCTION__);
        returnResponse(true);
    }
    uint32_t AVOutputSTB::setVolume(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
        LOGINFO("Exit : %s\n",__FUNCTION__);
        returnResponse(true);
    }


}//namespace Plugin
}//namespace WPEFramework
