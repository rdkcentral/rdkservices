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

#ifndef AVOutputSTB_H
#define AVOutputSTB_H

#include "string.h"

#include <pthread.h>
#include "Module.h"

#include "AVOutputBase.h"
#include "libIARM.h"
#include "libIBusDaemon.h"
#include "libIBus.h"
#include "iarmUtil.h"
#include "UtilsLogging.h"
#include "UtilsJsonRpc.h"
#include "dsError.h"
#include "dsMgr.h"
#include "hdmiIn.hpp"


namespace WPEFramework {
namespace Plugin {

//class AVOutputSTB : public PluginHost::IPlugin, public PluginHost::JSONRPC {
class AVOutputSTB : public AVOutputBase {
    private:
        AVOutputSTB(const AVOutputSTB&) = delete;
        AVOutputSTB& operator=(const AVOutputSTB&) = delete;

	DECLARE_JSON_RPC_METHOD(getVolume)
        DECLARE_JSON_RPC_METHOD(setVolume)

    public:
	AVOutputSTB();
	~AVOutputSTB();
        static AVOutputSTB *instance;
	static AVOutputSTB* getInstance() { return instance; }
};

}//namespace Plugin
}//namespace WPEFramework
#endif
