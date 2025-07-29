/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/

#pragma once

#include "Module.h"
#include "UtilsLogging.h"
#include <interfaces/INativeJS.h>
#include <interfaces/Ids.h>

#include <mutex>
#include "NativeJSRenderer.h"
#include <thread>
#include <vector>
using namespace JsRuntime;

namespace WPEFramework
{
    namespace Plugin
    {

        class NativeJSImplementation : public Exchange::INativeJS
        {
        public:
            NativeJSImplementation();
            virtual ~NativeJSImplementation();
            virtual Core::hresult Initialize(const string waylandDisplay) override;
            virtual Core::hresult Deinitialize() override;
	    virtual Core::hresult CreateApplication(const std::string options, uint32_t& id) override;
	    virtual Core::hresult GetApplications() override;
	    virtual Core::hresult RunApplication(uint32_t id, const std::string url) override;
	    virtual Core::hresult RunJavaScript(uint32_t id, const std::string code) override;
	    virtual Core::hresult TerminateApplication(uint32_t id) override;
            
	    BEGIN_INTERFACE_MAP(NativeJSImplementation)
            INTERFACE_ENTRY(Exchange::INativeJS)
            END_INTERFACE_MAP

        private:
            std::thread mRenderThread;
            bool mRunning;
            std::shared_ptr<NativeJSRenderer> mNativeJSRenderer;
        };
    } // namespace Plugin
} // namespace WPEFramework
