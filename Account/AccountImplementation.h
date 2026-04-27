/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2026 RDK Management
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


#include <interfaces/Ids.h>
#include <interfaces/IStore2.h>
#include <interfaces/IAccount.h>
#include <interfaces/IConfiguration.h>

#include <com/com.h>
#include <core/core.h>


namespace WPEFramework {
namespace Plugin {
    class AccountImplementation : public Exchange::IAccount, public Exchange::IConfiguration
    {
    public:
        // We do not allow this plugin to be copied !!
        AccountImplementation();
        ~AccountImplementation() override;

        // We do not allow this plugin to be copied !!
        AccountImplementation(const AccountImplementation&) = delete;
        AccountImplementation& operator=(const AccountImplementation&) = delete;

        BEGIN_INTERFACE_MAP(AccountImplementation)
        INTERFACE_ENTRY(Exchange::IAccount)
        INTERFACE_ENTRY(Exchange::IConfiguration)
        END_INTERFACE_MAP

    public:

        // IConfiguration interface
        uint32_t Configure(PluginHost::IShell* service) override;

        // IAccount interface
        virtual Core::hresult GetLastCheckoutResetTime(GetLastCheckoutResetTimeResult& resetTime /* @out */) const override;
        virtual Core::hresult SetLastCheckoutResetTime(const uint64_t resetTime) override;

    private:
        PluginHost::IShell* _service;
        Exchange::IStore2* _store; 
    };
} // namespace Plugin
} // namespace WPEFramework
