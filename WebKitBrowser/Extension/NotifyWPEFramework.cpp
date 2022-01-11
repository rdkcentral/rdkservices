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

#include "Module.h"
#include "NotifyWPEFramework.h"

namespace WPEFramework {
namespace JavaScript {
namespace NotifyWPEFramework {

void InjectJS(WebKitScriptWorld* world, WebKitFrame* frame)
{
    if (webkit_frame_is_main_frame(frame) == false)
        return;

    JSCContext* jsContext = webkit_frame_get_js_context_for_script_world(frame, world);

    static const char wpeNotifyWPEFramework[] = "var wpe = {};\n"
        "wpe.NotifyWPEFramework = function() {\n"
        "  let retval = new Array;\n"
        "  for (let i = 0; i < arguments.length; i++) {\n"
        "    retval[i] = arguments[i];\n"
        "  }\n"
        "  window.webkit.messageHandlers.wpeNotifyWPEFramework.postMessage(retval);\n"
        "}";
    JSCValue* result = jsc_context_evaluate(jsContext, wpeNotifyWPEFramework, -1);
    g_object_unref(result);

    g_object_unref(jsContext);
}

}  // NotifyWPEFramework
}  // JavaScript
}  // WPEFramework
