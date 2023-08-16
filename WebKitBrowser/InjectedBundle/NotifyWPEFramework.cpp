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
 
#include "NotifyWPEFramework.h"

#include "Utils.h"

// Global handle to this bundle.
extern WKBundleRef g_Bundle;

namespace WPEFramework {

namespace JavaScript {

    namespace Functions {

        NotifyWPEFramework::NotifyWPEFramework()
        {
        }

        // Implementation of JS function: loops over arguments and passes all strings to WPEFramework.
        JSValueRef NotifyWPEFramework::HandleMessage(JSContextRef context, JSObjectRef,
            JSObjectRef, size_t argumentCount, const JSValueRef arguments[], JSValueRef*)
        {
            WKStringRef messageName = WKStringCreateWithUTF8CString(GetMessageName().c_str());

            // Build message body.
            WKMutableArrayRef messageBody = WKMutableArrayCreate();
            for (unsigned int index = 0; index < argumentCount; index++) {
                const JSValueRef& argument = arguments[index];

                // For now only pass along strings.
                if (!JSValueIsString(context, argument))
                    continue;

                JSStringRef jsString = JSValueToStringCopy(context, argument, nullptr);

                WKStringRef messageLine = WKStringCreateWithJSString(jsString);
                WKArrayAppendItem(messageBody, messageLine);

                WKRelease(messageLine);
                JSStringRelease(jsString);
            }

            WKBundlePostSynchronousMessage(g_Bundle, messageName, messageBody, nullptr);

            WKRelease(messageBody);
            WKRelease(messageName);

            return JSValueMakeNull(context);
        }

        static JavaScriptFunctionType<NotifyWPEFramework> _instance(_T("wpe"));
    }
}
}
