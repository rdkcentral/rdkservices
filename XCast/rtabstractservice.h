/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2018 RDK Management
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
#ifndef RTABSTRACT_SERVICE_H
#define RTABSTRACT_SERVICE_H
//#include "servicedelegate.h"  /*XCAST-123 specific change*/
#include <rtRemote.h>
#include <rtObject.h>
class rtAbstractService : public rtObject
{
public:
    rtDeclareObject(rtAbstractService, rtObject);
    rtReadOnlyProperty(name, name, rtString);
    rtReadOnlyProperty(version, version, uint32_t);
    rtReadOnlyProperty(quirks, quirks, rtValue);
    rtMethod1ArgAndNoReturn("ping", ping, rtObjectRef);
    rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);
    rtMethod2ArgAndNoReturn("delListener", delListener, rtString, rtFunctionRef);
    rtError name(rtString& v) const;
    rtError version(uint32_t& v) const;
    rtError ping(const rtObjectRef& params);
    rtError bye();
    virtual rtError quirks(rtValue& v) const;
    virtual rtError addListener(rtString eventName, const rtFunctionRef &f);
    virtual rtError delListener(rtString eventName, const rtFunctionRef &f);
protected:
    rtAbstractService(rtString serviceName);
    void setName(rtString n);
    void setApiVersion(uint32_t v);
    rtError notify(const rtString& eventName, rtObjectRef e);
public:
    virtual ~rtAbstractService();
private:
    rtString mServiceName;
    uint32_t mApiVersion;
    rtEmitRef mEmit;
};
#endif //RTABSTRACT_SERVICE_H
