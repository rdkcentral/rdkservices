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
#include "BridgeObjectGLib.h"
#include "Tags.h"

namespace WPEFramework {
namespace JavaScript {
namespace BridgeObject {

const char kServiceManagerSrc[] = R"jssrc(
window.ServiceManager = {
  version: 2.1,
  getServiceForJavaScript: function(name,readyCb) {
    if (name === 'com.comcast.BridgeObject_1')
      readyCb({ JSMessageChanged: (msg) => window.ServiceManager.BridgeQuery(btoa(msg)) })
    else
      console.error('Requested service not supported')
  }
}
)jssrc";

const char kBadgerReplySrc[] = R"jssrc(
  var obj = JSON.parse(atob(payload))
  window.$badger.callback(obj.pid, obj.success, obj.json)
)jssrc";

const char kBadgerEventSrc[] = R"jssrc(
  var obj = JSON.parse(atob(payload))
  window.$badger.event(obj.handlerId, obj.json)
)jssrc";

static void CallBridge(WebKitWebPage* page, const char* scriptSrc, WebKitUserMessage* message)
{
    GVariant* payload;
    const char* payloadPtr;

    payload = webkit_user_message_get_parameters(message);
    if (!payload) {
        return;
    }
    g_variant_get(payload, "&s", &payloadPtr);

    WebKitFrame* frame = webkit_web_page_get_main_frame(page);
    JSCContext* jsContext = webkit_frame_get_js_context(frame);

    JSCValue *payloadVal = jsc_value_new_string(jsContext, payloadPtr);
    jsc_context_set_value(jsContext, "payload", payloadVal);
    g_object_unref(payloadVal);

    JSCValue* script = jsc_context_evaluate(jsContext, scriptSrc, -1);
    g_object_unref(script);

    g_object_unref(jsContext);
}

static void OnBridgeQuery(const char* arg, gpointer userData)
{
    WebKitWebPage* page = reinterpret_cast<WebKitWebPage*>(userData);
    webkit_web_page_send_message_to_view(page,
            webkit_user_message_new(Tags::BridgeObjectQuery,
                    g_variant_new("s", arg)), nullptr, nullptr, nullptr);
}

void InjectJS(WebKitScriptWorld* world, WebKitWebPage* page, WebKitFrame* frame)
{
    if (webkit_frame_is_main_frame(frame) == false)
        return;

    JSCContext* jsContext = webkit_frame_get_js_context_for_script_world(frame, world);

    JSCValue* script = jsc_context_evaluate(jsContext, kServiceManagerSrc, -1);
    g_object_unref(script);

    JSCValue* jsObject = jsc_context_get_value(jsContext, "ServiceManager");
    JSCValue* jsFunction = jsc_value_new_function(jsContext, nullptr,
            reinterpret_cast<GCallback>(OnBridgeQuery), (gpointer) page,
            nullptr, G_TYPE_NONE, 1, G_TYPE_STRING);
    jsc_value_object_set_property(jsObject, "BridgeQuery", jsFunction);
    g_object_unref(jsFunction);
    g_object_unref(jsObject);

    g_object_unref(jsContext);
}

bool HandleMessageToPage(WebKitWebPage* page, const char* messageName, WebKitUserMessage* message)
{
    if (g_strcmp0(messageName, Tags::BridgeObjectReply) == 0) {
        CallBridge(page, kBadgerReplySrc, message);
        return true;
    }
    else if (g_strcmp0(messageName, Tags::BridgeObjectEvent) == 0) {
        CallBridge(page, kBadgerEventSrc, message);
        return true;
    }
    return false;
}

}  // namespace BridgeObject
}  // namespace JavaScript
}  // namespace WPEFramework
