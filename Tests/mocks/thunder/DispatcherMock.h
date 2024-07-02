/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2023 RDK Management
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

#ifndef DISPATCHERMOCK_H
#define DISPATCHERMOCK_H

#include <gmock/gmock.h>

#include "Module.h"

 class DispatcherMock: public WPEFramework::PluginHost::IDispatcher{
 public:
         virtual ~DispatcherMock() = default;
         MOCK_METHOD(void, AddRef, (), (const, override));
         MOCK_METHOD(uint32_t, Release, (), (const, override));
         MOCK_METHOD(void*, QueryInterface, (const uint32_t interfaceNummer), (override));
         //MOCK_METHOD(void, Activate, (WPEFramework::PluginHost::IShell* service));
         //MOCK_METHOD(void, Deactivate, ());
         MOCK_METHOD(WPEFramework::Core::hresult, Invoke, (WPEFramework::PluginHost::IDispatcher::ICallback* callback, const uint32_t channelId, const uint32_t id, const string& token, const string& method, const string& parameters /* @restrict:(4M-1) */, string& response /* @restrict:(4M-1) @out */), (override));
         MOCK_METHOD(WPEFramework::Core::hresult, Revoke, (WPEFramework::PluginHost::IDispatcher::ICallback* callback), (override));
         MOCK_METHOD(WPEFramework::Core::hresult, Validate, (const string& token, const string& method, const string& paramaters /* @restrict:(4M-1) */), (const, override));
         MOCK_METHOD(WPEFramework::PluginHost::ILocalDispatcher*, Local, (), (override));
};
#endif //DISPATCHERMOCK_H
