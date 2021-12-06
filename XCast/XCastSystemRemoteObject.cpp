/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
* Copyright 2021 Liberty Global Service B.V.
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

#include "XCastSystemRemoteObject.h"

#include "utils.h"

using namespace WPEFramework;

rtDefineMethod(XCastSystemRemoteObject, systemRequest);

rtDefineObject(XCastSystemRemoteObject, rtObject);

void XCastSystemRemoteObject::registerRemoteObject(rtRemoteEnvironment* env) {
    rtRemoteRegisterObject(env, XCAST_SYSTEM_OBJECT_SERVICE_NAME, this);
}

void XCastSystemRemoteObject::setService(RtNotifier * service){
    m_observer = service;
}

rtError XCastSystemRemoteObject::systemRequest(const rtObjectRef &params) {

    if (!m_observer) {
        LOGERR("XCastSystemRemoteObject::systemRequest init faiure\n");
        return RT_FAIL;
    }

    rtError ret = RT_ERROR_INVALID_ARG;
    const std::string action { params.get<rtString>("action")};

    if ("sleep" == action) {
        const std::string key {params.get<rtString>("key")};
        const bool succeeded = m_observer->onXcastSystemApplicationSleepRequest(key);
        return succeeded ? RT_OK : RT_FAIL;
    }

    return ret;
}

void XCastSystemRemoteObject::unregisterRemoteObject(rtRemoteEnvironment* env) {
    rtRemoteUnregisterObject(env, XCAST_SYSTEM_OBJECT_SERVICE_NAME);
}