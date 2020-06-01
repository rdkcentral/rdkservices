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

#pragma once

#include "Module.h"
#include <interfaces/IBrowser.h>

#include <vector>

namespace WPEFramework {
namespace Exchange {

    struct IWebKitBrowser : virtual public Core::IUnknown {
        enum { ID = ID_WEB_BROWSER + 0x4000 };

        enum Visibility : uint8_t {
            HIDDEN = 0,
            VISIBLE = 1,
        };

        enum HTTPCookieAcceptPolicy : uint8_t {
            ALWAYS = 0,
            NEVER  = 1,
            ONLY_FROM_MAIN_DOCUMENT_DOMAIN = 2,
            EXCLUSIVELY_FROM_MAIN_DOCUMENT_DOMAIN = 3
        };

        struct INotification : virtual public Core::IUnknown {
            enum { ID = ID_BROWSER_NOTIFICATION + 0x4000 };

            virtual ~INotification() {}

            // Signal changes on the subscribed namespace..
            virtual void LoadFinished(const string& URL, const int32_t code) = 0;
            virtual void LoadFailed(const string& URL) = 0;
            virtual void URLChange(const string& URL, const bool loaded) = 0;
            virtual void VisibilityChange(const bool hidden) = 0;
            virtual void PageClosure() = 0;
            virtual void BridgeQuery(const string& message) = 0;
        };

        virtual ~IWebKitBrowser() { }

        virtual void Register(INotification* sink) = 0;
        virtual void Unregister(INotification* sink) = 0;

        virtual string GetURL() const = 0;
        virtual void SetURL(const string& url) = 0;

        virtual Visibility GetVisibility() const = 0;
        virtual void SetVisibility(const Visibility state) = 0;

        virtual uint32_t GetFPS() const = 0;

        virtual string GetHeaders() const = 0;
        virtual void SetHeaders(const string& headers) = 0;

        virtual string GetUserAgent() const = 0;
        virtual void SetUserAgent(const string& ua) = 0;

        virtual string GetLanguages() const = 0;
        virtual void SetLanguages(const string& langs) = 0;

        virtual bool GetLocalStorageEnabled() const = 0;
        virtual void SetLocalStorageEnabled(const bool enabled) = 0;

        virtual HTTPCookieAcceptPolicy GetHTTPCookieAcceptPolicy() = 0;
        virtual void SetHTTPCookieAcceptPolicy(const HTTPCookieAcceptPolicy policy) = 0;

        virtual void BridgeReply(const string& payload) = 0;
        virtual void BridgeEvent(const string& payload) = 0;
    };
}}
