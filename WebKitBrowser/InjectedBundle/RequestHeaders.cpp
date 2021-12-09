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

#include "RequestHeaders.h"

#include <WPE/WebKit.h>
#include <WPE/WebKit/WKType.h>
#include <WPE/WebKit/WKString.h>
#include <WPE/WebKit/WKBundleFrame.h>
#include <WPE/WebKit/WKURL.h>

#include "Module.h"
#include <interfaces/json/JsonData_WebKitBrowser.h>

#include "Utils.h"

namespace WPEFramework {
namespace WebKit {

namespace
{

typedef std::vector<std::pair<std::string, std::string>> Headers;
typedef std::unordered_map<WKBundlePageRef, Headers> PageHeaders;
static PageHeaders s_pageHeaders;

bool ParseHeaders(const string& json, Headers& out)
{
    Core::OptionalType<Core::JSON::Error> error;
    Core::JSON::ArrayType<JsonData::WebKitBrowser::HeadersData> array;
    if (array.FromString(json, error)) {
        for (auto it = array.Elements(); it.Next();) {
            if (!it.IsValid())
                continue;
            const auto &data  = it.Current();
            out.emplace_back(data.Name.Value(), data.Value.Value());
            TRACE_GLOBAL(Trace::Information, (_T("header: '%s: %s'\n"), data.Name.Value().c_str(), data.Value.Value().c_str()));
        }
        return true;
    } else {
        TRACE_GLOBAL(Trace::Error,
                     (_T("Failed to parse headers array, error='%s', json='%s'\n"),
                      (error.IsSet() ? error.Value().Message().c_str() : "unknown"), json.c_str()));
    }

    return false;
}

} // namespace

void RemoveRequestHeaders(WKBundlePageRef page)
{
    s_pageHeaders.erase(page);
}

void SetRequestHeaders(WKBundlePageRef page, WKTypeRef messageBody)
{
    if (WKGetTypeID(messageBody) != WKStringGetTypeID())
        return;

    string message = WPEFramework::WebKit::Utils::WKStringToString(static_cast<WKStringRef>(messageBody));
    if (message.empty()) {
        RemoveRequestHeaders(page);
        return;
    }

    Headers newHeaders;
    if (ParseHeaders(message, newHeaders)) {
        if (newHeaders.empty())
            RemoveRequestHeaders(page);
        else
            s_pageHeaders[page] = std::move(newHeaders);
    }
}

void ApplyRequestHeaders(WKBundlePageRef page, WKURLRequestRef requestRef)
{
    auto it = s_pageHeaders.find(page);
    if (it == s_pageHeaders.end())
        return;

    for (const auto& h : it->second) {
        auto key = WKStringCreateWithUTF8CString(h.first.c_str());
        auto value = WKStringCreateWithUTF8CString(h.second.c_str());
        WKURLRequestSetHTTPHeaderField(requestRef, key, value);
        WKRelease(key);
        WKRelease(value);
    }
}

}  // WebKit
}  // WPEFramework
