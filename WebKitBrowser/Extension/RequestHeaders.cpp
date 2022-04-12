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
#include "RequestHeaders.h"

#include <interfaces/json/JsonData_WebKitBrowser.h>

#include <core/JSON.h>

#if defined(ENABLE_AAMP_JSBINDINGS)
#include "AAMPJSBindings.h"
#endif

namespace WPEFramework {
namespace WebKit {

namespace
{

typedef std::vector<std::pair<std::string, std::string>> Headers;
typedef std::unordered_map<WebKitWebPage*, Headers> PageHeaders;
static PageHeaders s_pageHeaders;

bool ParseHeaders(const string& json, Headers& out)
{
    Core::OptionalType<Core::JSON::Error> error;
    Core::JSON::ArrayType<JsonData::WebKitBrowser::HeadersData> array;
    if (array.FromString(json, error)) {
        for (auto it = array.Elements(); it.Next();) {
            if (!it.IsValid()) {
                continue;
            }
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

void RemoveRequestHeaders(WebKitWebPage* page)
{
    s_pageHeaders.erase(page);
}

void SetRequestHeaders(WebKitWebPage* page, WebKitUserMessage* message)
{
    GVariant* parameters;
    const char* headersPtr;

    parameters = webkit_user_message_get_parameters(message);
    if (!parameters) {
        return;
    }
    g_variant_get(parameters, "&s", &headersPtr);
    string headersStr = headersPtr;

#if defined(ENABLE_AAMP_JSBINDINGS)
    // Pass on HTTP headers to AAMP , if empty, AAMP should clear previose headers set
    JavaScript::AAMP::SetHttpHeaders(headersStr.c_str());
#endif

    if (headersStr.empty()) {
        RemoveRequestHeaders(page);
        return;
    }

    Headers newHeaders;
    if (ParseHeaders(headersStr, newHeaders)) {
        if (newHeaders.empty()) {
            RemoveRequestHeaders(page);
        } else {
            s_pageHeaders[page] = std::move(newHeaders);
        }
    }
}

void ApplyRequestHeaders(WebKitWebPage* page, WebKitURIRequest* request)
{
    auto it = s_pageHeaders.find(page);
    if (it == s_pageHeaders.end())
        return;

    SoupMessageHeaders *headers = webkit_uri_request_get_http_headers(request);
    if (!headers)
        return;

    for (const auto& h : it->second) {
        soup_message_headers_append(headers, h.first.c_str(), h.second.c_str());
    }
}

}  // WebKit
}  // WPEFramework
