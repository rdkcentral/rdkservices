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

#ifndef ControlSettingsSTB_H
#define ControlSettingsSTB_H

#include "string.h"

#include "tvTypes.h"
#include "tvLog.h"
#include "tvSettings.h"
#include <pthread.h>
#include "Module.h"
#include "tvTypes.h"
#include "tvError.h"

#include "utils.h"
#include "ControlSettingsCommon.h"
#include "libIARM.h"
#include "libIBusDaemon.h"
#include "libIBus.h"
#include "iarmUtil.h"

namespace WPEFramework {
namespace Plugin {

class ControlSettingsSTB : public PluginHost::IPlugin, public PluginHost::JSONRPC {
    private:
        ControlSettingsSTB(const ControlSettingsSTB&) = delete;
        ControlSettingsSTB& operator=(const ControlSettingsSTB&) = delete;

	DECLARE_JSON_RPC_METHOD(getVolume)
        DECLARE_JSON_RPC_METHOD(setVolume)

    public:
	ControlSettingsSTB();
	~ControlSettingsSTB();
        ControlSettingsSTB *instance;
        void Initialize();
        void Deinitialize();
        virtual int UpdatePQParamsToCache( const char *action, const char *tr181ParamName, const char *pqmode, const char *source, const char *format, tvPQParameterIndex_t pqParamIndex, int params[] ) = 0;
    
    protected:
        static void dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
};

}//namespace Plugin
}//namespace WPEFramework
#endif
