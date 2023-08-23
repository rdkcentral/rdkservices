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
#include "AVOutputSTB.h"

#define registerMethod(...) Register(__VA_ARGS__);GetHandler(2)->Register<JsonObject, JsonObject>(__VA_ARGS__)

namespace WPEFramework {
namespace Plugin {

    AVOutputSTB* AVOutputSTB::instance = nullptr;

    AVOutputSTB::AVOutputSTB():PluginHost::JSONRPC()
    {
        LOGINFO("Entry\n"); 
        instance = this;
        CreateHandler({ 2 });

        registerMethod("getVolume", &AVOutputSTB::getVolume, this );
        registerMethod("setVolume", &AVOutputSTB::setVolume, this);

        LOGINFO("Exit\n");
    }
    
    AVOutputSTB :: ~AVOutputSTB()
    {
        LOGINFO();
    }

    void AVOutputSTB::Initialize()
    {
       LOGINFO("Entry\n");
       //Space for Device specific Init Sequence
       LOGINFO("Exit\n");
    }

    void AVOutputSTB::Deinitialize()
    {
       LOGINFO("Entry\n");
       LOGINFO("Exit\n");
    }

    //Event
    void AVOutputSTB::dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        LOGINFO("Entry %s\n",__FUNCTION__);
        LOGINFO("Exit %s\n",__FUNCTION__);
    }

    void AVOutputSTB::dsHdmiStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        LOGINFO("Entry %s\n",__FUNCTION__);
        LOGINFO("Exit %s\n",__FUNCTION__);
    }

    void AVOutputSTB::dsHdmiVideoModeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        LOGINFO("Entry %s\n",__FUNCTION__);
        LOGINFO("Exit %s\n",__FUNCTION__);
    }
    uint32_t AVOutputSTB::getVolume(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
        //PLUGIN_Lock(Lock);
        LOGINFO("Exit : %s\n",__FUNCTION__);
        returnResponse(true);
    }

    uint32_t AVOutputSTB::setVolume(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
        //PLUGIN_Lock(Lock);
        LOGINFO("Exit : %s\n",__FUNCTION__);
        returnResponse(true);
    }


}//namespace Plugin
}//namespace WPEFramework
