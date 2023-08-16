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

#include "JavaScriptFunctionType.h"
#include "Utils.h"
#include "../Tags.h"
#include <securityagent/securityagent.h>

namespace WPEFramework {
namespace JavaScript {
    namespace Functions {
        class token {
        public:
            token(const token&) = delete;
            token& operator= (const token&) = delete;
            token() {
            }
            ~token() {
            }

            JSValueRef HandleMessage(JSContextRef context, JSObjectRef,
                JSObjectRef, size_t argumentCount, const JSValueRef arguments[], JSValueRef*) {
                JSValueRef result;

                if (argumentCount != 0) {
                    TRACE_GLOBAL(Trace::Information, (_T("The Token Javascript command, does not take any paramaters!!!")));
                    result = JSValueMakeNull(context);
                }
                else {
                    uint8_t buffer[2 * 1024];

                    std::string url = WebKit::Utils::GetURL();

                    std::string tokenAsString;
                    if (url.length() < sizeof(buffer)) {
                        ::memset (buffer, 0, sizeof(buffer));
                        ::memcpy (buffer, url.c_str(), url.length());

                        int length = GetToken(static_cast<uint16_t>(sizeof(buffer)), url.length(), buffer);
                        if (length > 0) {
                           tokenAsString = std::string(reinterpret_cast<const char*>(buffer), length);
                        }
                    }

                    JSStringRef returnMessage = JSStringCreateWithUTF8CString(tokenAsString.c_str());
                    result = JSValueMakeString(context, returnMessage);

                    JSStringRelease(returnMessage);
                }

                return (result);
            }
        };

        static JavaScriptFunctionType<token> _instance(_T("thunder"));
    }
}
}
