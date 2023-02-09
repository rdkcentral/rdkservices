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
#include "AccessControlList.h"

namespace WPEFramework {
namespace Plugin {

    class SecurityContext : public PluginHost::ISecurity {
    private:
        class Payload : public Core::JSON::Container {
        public:
            Payload(const Payload&) = delete;
            Payload& operator=(const Payload&) = delete;

            Payload()
                : Core::JSON::Container()
                , URL()
                , User()
                , Hash()
            {
                Add(_T("url"), &URL);
                Add(_T("user"), &User);
                Add(_T("hash"), &Hash);
            }
            ~Payload()
            {
            }

        public:
            Core::JSON::String URL;
            Core::JSON::String User;
            Core::JSON::String Hash;
        };

    public:
        SecurityContext() = delete;
        SecurityContext(const SecurityContext&) = delete;
        SecurityContext& operator=(const SecurityContext&) = delete;

        SecurityContext(const AccessControlList* acl, const uint16_t length, const uint8_t payload[], const string& servicePrefix);
        virtual ~SecurityContext();

        //! Allow a websocket upgrade to be checked if it is allowed to be opened.
        bool Allowed(const string& path) const override;

        //! Allow a request to be checked before it is offered for processing.
        bool Allowed(const Web::Request& request) const override;

        //! Allow a JSONRPC message to be checked before it is offered for processing.
        bool Allowed(const Core::JSONRPC::Message& message) const override;

        string Token() const override;

    private:
        // Build QueryInterface implementation, specifying all possible interfaces to be returned.
        BEGIN_INTERFACE_MAP(SecurityOfficer)
        INTERFACE_ENTRY(PluginHost::ISecurity)
        END_INTERFACE_MAP

    private:
        string _token;
        Payload _context;
        const AccessControlList* _accessControlList;
        string _servicePrefix;
    };
}
}
