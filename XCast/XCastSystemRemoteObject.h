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

#pragma once

#include <rtRemote.h>
#include <rtObject.h>
#include <rtError.h>
#include "RtNotifier.h"

#define XCAST_SYSTEM_OBJECT_SERVICE_NAME "com.comcast.xcast_system"

class XCastSystemRemoteObject : public rtObject {

    public:
        XCastSystemRemoteObject() {
        }
        ~XCastSystemRemoteObject() {
        }
        XCastSystemRemoteObject(const XCastSystemRemoteObject&) = delete;
        void operator=(const XCastSystemRemoteObject&) = delete;
        rtDeclareObject(XCastSystemRemoteObject, rtObject);
        rtMethod1ArgAndNoReturn("systemRequest", systemRequest, rtObjectRef);

        void registerRemoteObject(rtRemoteEnvironment* env);
        void setService(RtNotifier * service);
        rtError systemRequest(const rtObjectRef &params);
        void unregisterRemoteObject(rtRemoteEnvironment* env);

    private:
        RtNotifier * m_observer;
};

class XCastSystemRemoteObjectReferenceWrapper {
public:
    XCastSystemRemoteObject *obj;
    XCastSystemRemoteObjectReferenceWrapper() : obj(new XCastSystemRemoteObject) {
        obj->AddRef();
    }
    ~XCastSystemRemoteObjectReferenceWrapper() {
        obj->Release();
        obj = nullptr;
    }

    XCastSystemRemoteObjectReferenceWrapper(const XCastSystemRemoteObjectReferenceWrapper&) = delete;
    void operator=(const XCastSystemRemoteObjectReferenceWrapper&) = delete;

    XCastSystemRemoteObject* operator->() {
        return obj;
    }
};