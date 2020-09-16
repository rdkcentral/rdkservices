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

#include <interfaces/json/JsonData_Browser.h>
#include <interfaces/json/JsonData_StateControl.h>
#include "./interfaces/json/JsonData_WebKitBrowser.h"

#include "WebKitBrowser.h"
#include "Module.h"

namespace WPEFramework {


namespace Plugin {

    using namespace JsonData::Browser;
    using namespace JsonData::WebKitBrowser;
    using namespace JsonData::StateControl;
    using namespace WPEFramework::Exchange;

    // Registration
    //

    void WebKitBrowser::RegisterAll()
    {
        Property<Core::JSON::String>(_T("url"), &WebKitBrowser::get_url, &WebKitBrowser::set_url, this); /* Browser */
        Property<Core::JSON::EnumType<VisibilityType>>(_T("visibility"), &WebKitBrowser::get_visibility, &WebKitBrowser::set_visibility, this); /* Browser */
        Property<Core::JSON::DecUInt32>(_T("fps"), &WebKitBrowser::get_fps, nullptr, this); /* Browser */
        Property<Core::JSON::EnumType<StateType>>(_T("state"), &WebKitBrowser::get_state, &WebKitBrowser::set_state, this); /* StateControl */

        Property<Core::JSON::String>(_T("useragent"), &WebKitBrowser::get_useragent, &WebKitBrowser::set_useragent, this);
        Property<Core::JSON::EnumType<HttpcookieacceptpolicyType>>(_T("httpcookieacceptpolicy"), &WebKitBrowser::get_httpcookieacceptpolicy, &WebKitBrowser::set_httpcookieacceptpolicy, this);
        Property<Core::JSON::Boolean>(_T("localstorageenabled"), &WebKitBrowser::get_localstorageenabled, &WebKitBrowser::set_localstorageenabled, this);
        Property<Core::JSON::ArrayType<Core::JSON::String>>(_T("languages"), &WebKitBrowser::get_languages, &WebKitBrowser::set_languages, this);
        Property<Core::JSON::ArrayType<HeadersData>>(_T("headers"), &WebKitBrowser::get_headers, &WebKitBrowser::set_headers, this);
        Register<Core::JSON::String,void>(_T("bridgereply"), &WebKitBrowser::endpoint_bridgereply, this);
        Register<Core::JSON::String,void>(_T("bridgeevent"), &WebKitBrowser::endpoint_bridgeevent, this);
    }

    void WebKitBrowser::UnregisterAll()
    {
        Unregister(_T("state"));
        Unregister(_T("fps"));
        Unregister(_T("visibility"));
        Unregister(_T("url"));
        Unregister(_T("headers"));
        Unregister(_T("languages"));
        Unregister(_T("localstorageenabled"));
        Unregister(_T("httpcookieacceptpolicy"));
        Unregister(_T("useragent"));
        Unregister(_T("bridgereply"));
    }

    // API implementation
    //

    // Method: bridgereply - Send response to legacy $badger
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t WebKitBrowser::endpoint_bridgereply(const Core::JSON::String& params)
    {
        ASSERT(_browser != nullptr);

        _browser->BridgeReply(params.Value());

        return Core::ERROR_NONE;
    }

    // Method: bridgeevent - Send legacy $badger event
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t WebKitBrowser::endpoint_bridgeevent(const Core::JSON::String& params)
    {
        ASSERT(_browser != nullptr);

        _browser->BridgeEvent(params.Value());

        return Core::ERROR_NONE;
    }

    // Property: url - URL loaded in the browser
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t WebKitBrowser::get_url(Core::JSON::String& response) const /* Browser */
    {
        ASSERT(_browser != nullptr);

        response = _browser->GetURL();

        return Core::ERROR_NONE;
    }

    // Property: url - URL loaded in the browser
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_INCORRECT_URL: Incorrect URL given
    uint32_t WebKitBrowser::set_url(const Core::JSON::String& param) /* Browser */
    {
        ASSERT(_browser != nullptr);

        uint32_t result = Core::ERROR_INCORRECT_URL;

        if (param.IsSet() && !param.Value().empty()) {
            _browser->SetURL(param.Value());
            result = Core::ERROR_NONE;
        }

        return result;
    }

    // Property: visibility - Current browser visibility
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t WebKitBrowser::get_visibility(Core::JSON::EnumType<VisibilityType>& response) const /* Browser */
    {
        auto state = _browser->GetVisibility();

        response = (state == Exchange::IWebKitBrowser::Visibility::VISIBLE)
                ? VisibilityType::VISIBLE
                : VisibilityType::HIDDEN;

        return Core::ERROR_NONE;
    }

    // Property: visibility - Current browser visibility
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t WebKitBrowser::set_visibility(const Core::JSON::EnumType<VisibilityType>& param) /* Browser */
    {
        ASSERT(_browser != nullptr);

        uint32_t result = Core::ERROR_BAD_REQUEST;

        if (param.IsSet()) {
            auto state = (param == VisibilityType::VISIBLE)
                ? Exchange::IWebKitBrowser::Visibility::VISIBLE
                : Exchange::IWebKitBrowser::Visibility::HIDDEN;

            _browser->SetVisibility(state);

            result = Core::ERROR_NONE;
        }

        return result;
    }

    // Property: fps - Current number of frames per second the browser is rendering
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t WebKitBrowser::get_fps(Core::JSON::DecUInt32& response) const /* Browser */
    {
        ASSERT(_browser != nullptr);

        response = _browser->GetFPS();

        return Core::ERROR_NONE;
    }

    // Property: state - Running state of the service
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t WebKitBrowser::get_state(Core::JSON::EnumType<StateType>& response) const /* StateControl */
    {
        ASSERT(_browser != nullptr);

        PluginHost::IStateControl* stateControl(_browser->QueryInterface<PluginHost::IStateControl>());

        // In the mean time an out-of-process plugin might have crashed and thus return a nullptr.
        if (stateControl != nullptr) {

            PluginHost::IStateControl::state currentState = stateControl->State();
            response = (currentState == PluginHost::IStateControl::SUSPENDED? StateType::SUSPENDED : StateType::RESUMED);

            stateControl->Release();
        }

        return Core::ERROR_NONE;
    }

    // Property: state - Running state of the service
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t WebKitBrowser::set_state(const Core::JSON::EnumType<StateType>& param) /* StateControl */
    {
        ASSERT(_browser != nullptr);

        uint32_t result = Core::ERROR_BAD_REQUEST;

        if (param.IsSet()) {
            PluginHost::IStateControl* stateControl(_browser->QueryInterface<PluginHost::IStateControl>());

            // In the mean time an out-of-process plugin might have crashed and thus return a nullptr.
            if (stateControl != nullptr) {

                stateControl->Request(param == StateType::SUSPENDED? PluginHost::IStateControl::SUSPEND : PluginHost::IStateControl::RESUME);

                stateControl->Release();
            }

            result = Core::ERROR_NONE;
        }

        return result;
    }

    // Event: urlchange - Signals a URL change in the browser
    void WebKitBrowser::event_urlchange(const string& url, const bool& loaded) /* Browser */
    {
        UrlchangeParamsData params;
        params.Url = url;
        params.Loaded = loaded;

        Notify(_T("urlchange"), params);
    }

    // Event: visibilitychange - Signals a visibility change of the browser
    void WebKitBrowser::event_visibilitychange(const bool& hidden) /* Browser */
    {
        VisibilitychangeParamsData params;
        params.Hidden = hidden;

        Notify(_T("visibilitychange"), params);
    }

    // Event: pageclosure - Notifies that the web page requests to close its window
    void WebKitBrowser::event_pageclosure() /* Browser */
    {
        Notify(_T("pageclosure"));
    }

    // Property: useragent - The UserAgent string used by browser
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t WebKitBrowser::get_useragent(Core::JSON::String& response) const
    {
        ASSERT(_browser != nullptr);

        string useragent = _browser->GetUserAgent();

        response = useragent;

        return Core::ERROR_NONE;
    }

    // Property: useragent - The UserAgent string used by browser
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t WebKitBrowser::set_useragent(const Core::JSON::String& param)
    {
        ASSERT(_browser != nullptr);

        _browser->SetUserAgent(param.Value());

        return Core::ERROR_NONE;
    }

    // Property: httpcookieacceptpolicy - HTTP cookies accept policy
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_UNKNOWN_KEY: Unknown policy
    uint32_t WebKitBrowser::get_httpcookieacceptpolicy(Core::JSON::EnumType<HttpcookieacceptpolicyType>& response) const
    {
        ASSERT(_browser != nullptr);

        auto policy = _browser->GetHTTPCookieAcceptPolicy();

        switch(policy) {
            case Exchange::IWebKitBrowser::ALWAYS:
                response = HttpcookieacceptpolicyType::ALWAYS;
                break;
            case Exchange::IWebKitBrowser::NEVER:
                response = HttpcookieacceptpolicyType::NEVER;
                break;
            case Exchange::IWebKitBrowser::ONLY_FROM_MAIN_DOCUMENT_DOMAIN:
                response = HttpcookieacceptpolicyType::ONLYFROMMAINDOCUMENTDOMAIN;
                break;
            case Exchange::IWebKitBrowser::EXCLUSIVELY_FROM_MAIN_DOCUMENT_DOMAIN:
                response = HttpcookieacceptpolicyType::EXCLUSIVELYFROMMAINDOCUMENTDOMAIN;
                break;
            default:
                ASSERT(false);
                return Core::ERROR_UNKNOWN_KEY;
        }

        return Core::ERROR_NONE;
    }

    // Property: httpcookieacceptpolicy - HTTP cookies accept policy
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_UNKNOWN_KEY: Unknown policy
    uint32_t WebKitBrowser::set_httpcookieacceptpolicy(const Core::JSON::EnumType<HttpcookieacceptpolicyType>& param)
    {
        ASSERT(_browser != nullptr);

        Exchange::IWebKitBrowser::HTTPCookieAcceptPolicy policy;

        switch(param.Value()) {
            case HttpcookieacceptpolicyType::ALWAYS:
                policy = Exchange::IWebKitBrowser::ALWAYS;
                break;
            case HttpcookieacceptpolicyType::NEVER:
                policy = Exchange::IWebKitBrowser::NEVER;
                break;
            case HttpcookieacceptpolicyType::ONLYFROMMAINDOCUMENTDOMAIN:
                policy = Exchange::IWebKitBrowser::ONLY_FROM_MAIN_DOCUMENT_DOMAIN;
                break;
            case HttpcookieacceptpolicyType::EXCLUSIVELYFROMMAINDOCUMENTDOMAIN:
                policy = Exchange::IWebKitBrowser::EXCLUSIVELY_FROM_MAIN_DOCUMENT_DOMAIN;
                break;
            default:
                ASSERT(false);
                return Core::ERROR_UNKNOWN_KEY;
        }

        _browser->SetHTTPCookieAcceptPolicy(policy);
        return Core::ERROR_NONE;
    }

    // Property: localstorageenabled - Controls the local storage availability
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t WebKitBrowser::get_localstorageenabled(Core::JSON::Boolean& response) const
    {
        ASSERT(_browser != nullptr);
        response = _browser->GetLocalStorageEnabled();
        return Core::ERROR_NONE;
    }

    // Property: localstorageenabled - Controls the local storage availability
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t WebKitBrowser::set_localstorageenabled(const Core::JSON::Boolean& param)
    {
        ASSERT(_browser != nullptr);
        _browser->SetLocalStorageEnabled(param.Value());
        return Core::ERROR_NONE;
    }

    // Property: languages - Browser prefered languages
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t WebKitBrowser::get_languages(Core::JSON::ArrayType<Core::JSON::String>& response) const
    {
        ASSERT(_browser != nullptr);

        string langs = _browser->GetLanguages();
        response.FromString(langs);

        return Core::ERROR_NONE;
    }

    // Property: languages - Browser prefered languages
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t WebKitBrowser::set_languages(const Core::JSON::ArrayType<Core::JSON::String>& param)
    {
        ASSERT(_browser != nullptr);

        string langs;
        if ( param.IsSet() ) {
            param.ToString(langs);
        }
        _browser->SetLanguages(langs);

        return Core::ERROR_NONE;
    }

    // Property: headers - Headers to send on all requests that the browser makes
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t WebKitBrowser::get_headers(Core::JSON::ArrayType<JsonData::WebKitBrowser::HeadersData>& response) const
    {
        ASSERT(_browser != nullptr);
        string headers = _browser->GetHeaders();
        response.FromString(headers);
        return Core::ERROR_NONE;
    }

    // Property: headers - Headers to send on all requests that the browser makes
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t WebKitBrowser::set_headers(const Core::JSON::ArrayType<JsonData::WebKitBrowser::HeadersData>& param)
    {
        ASSERT(_browser != nullptr);

        string headers;

        if ( param.IsSet() ) {
            param.ToString(headers);
        }

        _browser->SetHeaders(headers);
        return Core::ERROR_NONE;
    }

    // Event: loadfinished - Signals initial HTML document has been completely loaded and parsed
    void WebKitBrowser::event_loadfinished(const string& url, const int32_t& httpstatus)
    {
        LoadfinishedParamsData params;
        params.Url = url;
        params.Httpstatus = httpstatus;

        Notify(_T("loadfinished"), params);
    }

    // Event: loadfailed - Browser failed to load page
    void WebKitBrowser::event_loadfailed(const string& url)
    {
        LoadfailedParamsData params;
        params.Url = url;

        Notify(_T("loadfailed"), params);
    }

    // Event: bridgequery - A message from legacy $badger bridge
    void WebKitBrowser::event_bridgequery(const string& message)
    {
        Core::JSON::String params;
        params = message;
        Notify(_T("bridgequery"), params);
    }

    // Event: statechange - Signals a state change of the service
    void WebKitBrowser::event_statechange(const bool& suspended) /* StateControl */
    {
        StatechangeParamsData params;
        params.Suspended = suspended;

        Notify(_T("statechange"), params);
    }

} // namespace Plugin

} // namespace WPEFramework
