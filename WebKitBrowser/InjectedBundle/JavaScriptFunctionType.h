/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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
 
#ifndef __JAVASCRIPTFUNCTIONTYPE_H
#define __JAVASCRIPTFUNCTIONTYPE_H

#include "ClassDefinition.h"
#include "JavaScriptFunction.h"

namespace WPEFramework {

namespace JavaScript {

    // Wrapper for JavaScript handler, takes care of messy function pointer details.
    template <typename ActualJavaScriptFunction>
    class JavaScriptFunctionType : public JavaScriptFunction {
    public:
        // Constructor, also registers to ClassDefinition.
        JavaScriptFunctionType(const string& jsClassName, bool shouldNotEnum = false)
            : JavaScriptFunction(Core::ClassNameOnly(typeid(ActualJavaScriptFunction).name()).Data(), function, shouldNotEnum)
            , JsClassName(jsClassName)
        {
            ClassDefinition::Instance(JsClassName).Add(this);
        }

        // Destructor, also unregisters function.
        ~JavaScriptFunctionType()
        {
            ClassDefinition::Instance(JsClassName).Remove(this);
        }

    private:
        // Callback function.
        static JSValueRef function(JSContextRef context, JSObjectRef function,
            JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
        {
            return Handler.HandleMessage(context, function, thisObject, argumentCount, arguments, exception);
        }

        static ActualJavaScriptFunction Handler;
        string JsClassName;
    };

    template <typename ActualJavaScriptFunction>
    ActualJavaScriptFunction JavaScriptFunctionType<ActualJavaScriptFunction>::Handler;
}
}

#endif // __JAVASCRIPTFUNCTIONTYPE_H
