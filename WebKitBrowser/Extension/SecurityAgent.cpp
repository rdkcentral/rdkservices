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
#include "SecurityAgent.h"

#include <securityagent/securityagent.h>

namespace WPEFramework {
namespace JavaScript {
namespace SecurityAgent {

static char* thunderToken(gpointer userData)
{
    WebKitFrame* frame = reinterpret_cast<WebKitFrame*>(userData);
    const char *uri = webkit_frame_get_uri(frame);
    if (uri) {
        uint8_t buffer[2 * 1024];
        size_t input_length = std::min ( strlen(uri), sizeof(buffer) );

        ::memset (buffer, 0, sizeof(buffer));
        ::memcpy (buffer, uri, input_length);

        int length = GetToken(static_cast<uint16_t>(sizeof(buffer)), input_length, buffer);
        if (length > 0) {
            return g_strndup(reinterpret_cast<const char*>(buffer), length);
        }
    }
    return nullptr;
}

void InjectJS(WebKitScriptWorld* world, WebKitFrame* frame)
{
    if (webkit_frame_is_main_frame(frame) == false)
        return;

    JSCContext* jsContext = webkit_frame_get_js_context_for_script_world(frame, world);
    JSCValue* jsObject = jsc_value_new_object(jsContext, nullptr, nullptr);
    JSCValue* jsFunction = jsc_value_new_function(jsContext, nullptr, reinterpret_cast<GCallback>(thunderToken),
            (gpointer)frame, nullptr, G_TYPE_STRING, 0, G_TYPE_NONE);
    jsc_value_object_set_property(jsObject, "token", jsFunction);
    g_object_unref(jsFunction);
    jsc_context_set_value(jsContext, "thunder", jsObject);
    g_object_unref(jsObject);
    g_object_unref(jsContext);
}

}  // SecurityAgent
}  // JavaScript
}  // WPEFramework
