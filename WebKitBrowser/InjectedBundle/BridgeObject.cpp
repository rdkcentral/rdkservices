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

#include <vector>
#include <string>
#include <utility>
#include <glib.h>

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

const char kInjectBadgerSrc[] = R"jssrc(
var scriptUrl = url;
window.addEventListener('load', (event) => {
  if (typeof $badger === 'undefined') {
    var script = document.createElement('script');
    script.src = scriptUrl;
    document.getElementsByTagName('head')[0].appendChild(script);
  }
});
)jssrc";

static std::string JSStringToString(JSStringRef str)
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
    std::string errorStr = JSStringToString(exceptStr);
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

struct PatternSpec
{
    PatternSpec() = delete;
    PatternSpec(const PatternSpec&) = delete;
    PatternSpec & operator=(const PatternSpec&) = delete;
    PatternSpec & operator=(PatternSpec&&) = delete;

    PatternSpec(const std::string& pattern)
        : _text(pattern)
    {
        _spec = g_pattern_spec_new(_text.c_str());
    }
    PatternSpec(PatternSpec&& o)
        : _text(std::move(o._text))
        , _spec(o._spec)
    {
        o._spec = nullptr;
    }
    ~PatternSpec()
    {
        if (_spec)
            g_pattern_spec_free(_spec);
    }
    std::string _text;
    GPatternSpec* _spec {nullptr};
};

static std::string g_badgerScriptUrl;
static std::vector<PatternSpec> g_injectBadgerFor;

void Initialize()
{
    auto requestConfig = []() -> std::string
    {
        std::string utf8MessageName(std::string(Tags::Config) + "badger");
        WKStringRef jsMessageName = WKStringCreateWithUTF8CString(utf8MessageName.c_str());
        WKMutableArrayRef messageBody = WKMutableArrayCreate();
        WKTypeRef returnData;
        WKBundlePostSynchronousMessage(WebKit::Utils::GetBundle(), jsMessageName, messageBody, &returnData);
        std::string json (WebKit::Utils::WKStringToString(static_cast<WKStringRef>(returnData)));
        WKRelease(returnData);
        WKRelease(messageBody);
        WKRelease(jsMessageName);
        return json;
    };

    auto parseConfig = [](const string& json)
    {
        struct BadgerConfig : public Core::JSON::Container
        {
            BadgerConfig()
                : Core::JSON::Container()
            {
                Add(_T("scripturl"), &ScriptUrl);
                Add(_T("injectfor"), &InjectFor);
            }
            Core::JSON::String ScriptUrl;
            Core::JSON::ArrayType<Core::JSON::String> InjectFor;
        };

        Core::OptionalType<Core::JSON::Error> error;
        BadgerConfig config;
        if (!config.FromString(json, error))
        {
            SYSLOG(Trace::Error,
                   (_T("Failed to parse $badger config, error='%s', json='%s'\n"),
                    (error.IsSet() ? error.Value().Message().c_str() : "unknown"), json.c_str()));
            return false;
        }

        g_badgerScriptUrl = config.ScriptUrl.Value();
        for (auto it = config.InjectFor.Elements(); it.Next();) {
            if (!it.IsValid())
                continue;
            const auto &data  = it.Current();
            g_injectBadgerFor.push_back(data.Value());
        }
        return true;
    };

    std::string json = requestConfig();
    if (parseConfig(json)) {
        SYSLOG(Trace::Information, (_T("Configured $badger script url: '%s'\n"), g_badgerScriptUrl.c_str()));
        for (const auto& p : g_injectBadgerFor) {
            SYSLOG(Trace::Information, (_T("Enable $badger script injection for: '%s'\n"), p._text.c_str()));
        }
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

    auto getProvisionalUrl = [](WKBundleFrameRef frame) {
        std::string result;
        auto frameUrl = WKBundleFrameCopyURL(frame);
        if (frameUrl) {
            auto urlString = WKURLCopyString(frameUrl);
            result = WebKit::Utils::WKStringToString(urlString);
            WKRelease(urlString);
            WKRelease(frameUrl);
        }
        return result;
    };

    auto shouldInjectBadgerScript = [](const std::string url) {
        for (const auto& p : g_injectBadgerFor) {
            if (g_pattern_match_string(p._spec, url.c_str()))
                return true;
        }
        return false;
    };

    std::string frameUrl = getProvisionalUrl(frame);
    if ( shouldInjectBadgerScript(frameUrl) ) {
        SYSLOG(Trace::Information, (_T("Injecting $badger script for: '%s'\n"), frameUrl.c_str()));

        JSStringRef mbScriptStr = JSStringCreateWithUTF8CString(kInjectBadgerSrc);
        JSStringRef paramNameStr = JSStringCreateWithUTF8CString("url");

        JSObjectRef fun = JSObjectMakeFunction(context, nullptr, 1, &paramNameStr, mbScriptStr, nullptr, 1, &exception);

        JSStringRelease(mbScriptStr);
        JSStringRelease(paramNameStr);
        if (exception) {
            LogException(context, exception);
            return;
        }

        JSStringRef scriptSrcStr = JSStringCreateWithUTF8CString(g_badgerScriptUrl.c_str());
        JSValueRef argValue = JSValueMakeString(context, scriptSrcStr);
        JSObjectCallAsFunction(context, fun, nullptr, 1, &argValue, &exception);
        JSStringRelease(scriptSrcStr);
        if (exception) {
            LogException(context, exception);
            return;
        }
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
