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
#include "AAMPJSBindings.h"

#include "Utils.h"

extern "C" {
    void aamp_LoadJSController(JSGlobalContextRef context);
    void aamp_UnloadJSController(JSGlobalContextRef context);
    void aamp_SetPageHttpHeaders(const char* headers);
}

namespace WPEFramework {
namespace JavaScript {
namespace AAMP {

namespace {

bool CanInjectJSBindings(WKURLRef url) {
    if (url == nullptr)
        return false;
    WKStringRef wkHost = WKURLCopyHostName(url);
    if (wkHost == nullptr)
        return false;
    std::string hostStr = WebKit::Utils::WKStringToString(wkHost);
    WKRelease(wkHost);
    if (hostStr.empty())
        return false;
    if (hostStr.find("youtube.com") != std::string::npos ||
        hostStr.find("atv-ext.amazon.com") != std::string::npos ||
        hostStr.find("ccast.api.amazonvideo.com") != std::string::npos ||
        hostStr.find("ccast.api.av-gamma.com") != std::string::npos)
        return false;
    return true;
}

}

void LoadJSBindings(WKBundleFrameRef frame) {
    if (WKBundleFrameIsMainFrame(frame)) {
        WKURLRef url = WKBundleFrameCopyURL(frame);
        bool canInject = CanInjectJSBindings(url);
        WKRelease(url);
        if (canInject) {
            JSGlobalContextRef context = WKBundleFrameGetJavaScriptContext(frame);
            aamp_LoadJSController(context);
        }
    }
}

void UnloadJSBindings(WKBundleFrameRef frame) {
    if (WKBundleFrameIsMainFrame(frame)) {
        JSGlobalContextRef context = WKBundleFrameGetJavaScriptContext(frame);
        JSObjectRef global = JSContextGetGlobalObject(context);
        JSStringRef aampStr = JSStringCreateWithUTF8CString("AAMP");
        if (JSObjectHasProperty(context, global, aampStr)) {
            aamp_UnloadJSController(context);
        }
        JSStringRelease(aampStr);
    }
}

// Just pass headers json to aamp plugin. SetHttpHeaders Called from RequestHeaders.cpp
void SetHttpHeaders(const char * headerJson)
{
    aamp_SetPageHttpHeaders(headerJson);
}

}  // namespace AAMP
}  // namespace JavaScript
}  // namespace WPEFramework
