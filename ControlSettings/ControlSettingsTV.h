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

#ifndef ControlSettingsTV_H
#define ControlSettingsTV_H

#include "string.h"

#include "tvTypes.h"
#include "tvLog.h"
#include "tvSettings.h"
#include <pthread.h>
#include "Module.h"
#include "tvTypes.h"
#include "tvError.h"

#include "AbstractPlugin.h"
#include "utils.h"
#include "ControlSettingsCommon.h"

namespace WPEFramework {
namespace Plugin {

class ControlSettingsTV : public AbstractPlugin {
    private:
        ControlSettingsTV(const ControlSettingsTV&) = delete;
        ControlSettingsTV& operator=(const ControlSettingsTV&) = delete;

	DECLARE_JSON_RPC_METHOD(getBacklight)
        DECLARE_JSON_RPC_METHOD(setBacklight)


    public:
        ControlSettingsTV();
        ~ControlSettingsTV();
        ControlSettingsTV *instance;
        void Initialize();
        void Deinitialize();
   
    protected:
        static void dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
};

}//namespace Plugin
}//namespace WPEFramework
#endif
