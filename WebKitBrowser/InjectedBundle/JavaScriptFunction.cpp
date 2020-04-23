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
 
#include "JavaScriptFunction.h"

#include <iostream>

#include "ClassDefinition.h"

namespace WPEFramework {
namespace JavaScript {

    JavaScriptFunction::JavaScriptFunction(const string& name, const JSObjectCallAsFunctionCallback callback,
        bool shouldNotEnum /* = false */)
        : Name(name)
        , Callback(callback)
        , ShouldNotEnum(shouldNotEnum)
    {
        NameString = Core::ToString(Name.c_str());
    }

    // Fills JSStaticFunction struct for this function. For now the name field is valid as long as this
    // instance exists, this might change in the future.
    JSStaticFunction JavaScriptFunction::BuildJSStaticFunction() const
    {
        JSStaticFunction staticFunction;

        // @Zan: How long does "staticFunction.name" need to be valid?
        staticFunction.name = NameString.c_str();
        staticFunction.callAsFunction = Callback;

        // @Zan: assumption is that functions should always be read only and can't be deleted, is this true?
        staticFunction.attributes = kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete;

        // @Zan: does it make sense to make a JS function unenumerable?
        if (ShouldNotEnum)
            staticFunction.attributes |= kJSPropertyAttributeDontEnum;

        return staticFunction;
    }
} // namespace JavaScript
} // namespace WPEFramework
