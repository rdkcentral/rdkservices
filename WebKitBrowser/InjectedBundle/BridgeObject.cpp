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

#include "BridgeObject.h"

#include "Utils.h"

// Global handle to this bundle.
extern WKBundleRef g_Bundle;

extern "C" WK_EXPORT JSStringRef WKStringCopyJSString(WKStringRef string);

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

static string JSStringToString(JSStringRef str)
{
  if (!str)
    return string();
  size_t len = JSStringGetMaximumUTF8CStringSize(str);
  std::unique_ptr<char[]> buffer(new char[len]);
  len = JSStringGetUTF8CString(str, buffer.get(), len);
  return Core::ToString(buffer.get(), len);
}

static void LogException(JSContextRef ctx, JSValueRef exception)
{
    JSStringRef exceptStr = JSValueToStringCopy(ctx, exception, nullptr);
    string errorStr = JSStringToString(exceptStr);
    JSStringRelease(exceptStr);
    TRACE_GLOBAL(Trace::Error, (_T("Got Exception: %s"), errorStr.c_str()));
}

static JSValueRef OnBridgeQuery(
    JSContextRef context, JSObjectRef,
    JSObjectRef, size_t argumentCount,
    const JSValueRef arguments[], JSValueRef*)
{
    if (argumentCount > 0 && JSValueIsString(context, arguments[0])) {
        WKStringRef messageName = WKStringCreateWithUTF8CString(Tags::BridgeObjectQuery);
        JSStringRef jsString = JSValueToStringCopy(context, arguments[0], nullptr);
        WKStringRef messageBody = WKStringCreateWithJSString(jsString);
        JSStringRelease(jsString);

        WKBundlePostSynchronousMessage(g_Bundle, messageName, messageBody, nullptr);
        WKRelease(messageBody);
        WKRelease(messageName);
    }
    return JSValueMakeNull(context);
}

static void CallBridge(WKBundlePageRef page, const char* scriptSrc, WKTypeRef payload)
{
    if (WKGetTypeID(payload) != WKStringGetTypeID()) {
        TRACE_GLOBAL(Trace::Error, (_T("Message body must be string!")));
        return;
    }

    JSGlobalContextRef context = WKBundleFrameGetJavaScriptContext(WKBundlePageGetMainFrame(page));
    JSValueRef exception = nullptr;

    JSStringRef scriptStr = JSStringCreateWithUTF8CString(scriptSrc);
    JSStringRef paramNameStr = JSStringCreateWithUTF8CString("payload");
    JSObjectRef fun = JSObjectMakeFunction(context, nullptr, 1, &paramNameStr, scriptStr, nullptr, 1, &exception);
    JSStringRelease(scriptStr);
    JSStringRelease(paramNameStr);
    if (exception) {
        LogException(context, exception);
        return;
    }

    JSStringRef messageStr = WKStringCopyJSString(static_cast<WKStringRef>(payload));
    JSValueRef argValue = JSValueMakeString(context, messageStr);
    JSObjectCallAsFunction(context, fun, nullptr, 1, &argValue, &exception);
    JSStringRelease(messageStr);

    if (exception) {
        LogException(context, exception);
        return;
    }
}

void InjectJS(WKBundleFrameRef frame)
{
    if (!WKBundleFrameIsMainFrame(frame))
        return;

    JSValueRef exception = nullptr;
    JSGlobalContextRef context = WKBundleFrameGetJavaScriptContext(frame);
    JSStringRef smScriptStr = JSStringCreateWithUTF8CString(kServiceManagerSrc);
    JSEvaluateScript(context, smScriptStr, nullptr, nullptr, 0, &exception);
    JSStringRelease(smScriptStr);
    if (exception) {
        LogException(context, exception);
        return;
    }

    JSStringRef serviceManagerStr = JSStringCreateWithUTF8CString("ServiceManager");
    JSObjectRef windowObject = JSContextGetGlobalObject(context);
    JSObjectRef smObject = const_cast<JSObjectRef>(JSObjectGetProperty(context, windowObject, serviceManagerStr, &exception));
    JSStringRelease(serviceManagerStr);
    if (exception) {
        LogException(context, exception);
        return;
    }

    JSStringRef bridgeQueryStr = JSStringCreateWithUTF8CString("BridgeQuery");
    JSValueRef  bridgeQueryFun = JSObjectMakeFunctionWithCallback(context, bridgeQueryStr, OnBridgeQuery);
    JSObjectSetProperty(context, smObject, bridgeQueryStr, bridgeQueryFun,
        kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete | kJSPropertyAttributeDontEnum, &exception);
    JSStringRelease(bridgeQueryStr);
    if (exception) {
        LogException(context, exception);
        return;
    }
}

bool HandleMessageToPage(WKBundlePageRef page, WKStringRef messageName, WKTypeRef messageBody)
{
    if (WKStringIsEqualToUTF8CString(messageName, Tags::BridgeObjectReply)) {
        CallBridge(page, kBadgerReplySrc, messageBody);
        return true;
    }
    else if (WKStringIsEqualToUTF8CString(messageName, Tags::BridgeObjectEvent)) {
        CallBridge(page, kBadgerEventSrc, messageBody);
        return true;
    }
    return false;
}

}  // namespace BridgeObject
}  // namespace JavaScript
}  // namespace WPEFramework
