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
#include "FireboltOSEndpoint.h"

#ifdef ENABLE_SECURITY_AGENT
#include <securityagent/securityagent.h>
#endif

#include <vector>

namespace WPEFramework {
namespace JavaScript {
namespace FireboltOSEndpoint {

static char* OnGetEndpoint(gpointer userData)
{
    WebKitFrame* frame = reinterpret_cast<WebKitFrame*>(userData);

    string thunderAccess;
    if ( !Core::SystemInfo::GetEnvironment(_T("THUNDER_ACCESS"), thunderAccess) )
      return nullptr;

    string resultString = "ws://" + thunderAccess + "/jsonrpc";

#ifdef ENABLE_SECURITY_AGENT
    string tokenAsString;
    const char *uri = webkit_frame_get_uri(frame);
    if (uri) {
      std::vector<uint8_t> buffer( 2 * 1024 );
      uint16_t inputLen = static_cast<uint16_t>(std::min(strlen(uri), buffer.size()));
      ::memcpy (buffer.data(), uri, inputLen);
      int outputLen = ::GetToken(buffer.size(), inputLen, buffer.data());
      if (outputLen > 0) {
        tokenAsString = std::string(reinterpret_cast<const char*>(buffer.data()), outputLen);
      }
    }
    if (!tokenAsString.empty())
      resultString += "?token=" + tokenAsString;
#endif

    return g_strdup(resultString.c_str());
}

void InjectJS(WebKitScriptWorld* world, WebKitFrame* frame)
{
    if (webkit_frame_is_main_frame(frame) == false)
        return;

    JSCContext* jsContext = webkit_frame_get_js_context_for_script_world(frame, world);
    JSCValue* jsObject = jsc_value_new_object(jsContext, nullptr, nullptr);
    JSCValue* jsFunction = jsc_value_new_function(
      jsContext, nullptr, reinterpret_cast<GCallback>(OnGetEndpoint),
      (gpointer)frame, nullptr, G_TYPE_STRING, 0, G_TYPE_NONE);
    jsc_value_object_set_property(jsObject, "endpoint", jsFunction);
    g_object_unref(jsFunction);
    jsc_context_set_value(jsContext, "__firebolt", jsObject);
    g_object_unref(jsObject);
    g_object_unref(jsContext);
}

}  // namespace BridgeObject
}  // namespace JavaScript
}  // namespace WPEFramework
