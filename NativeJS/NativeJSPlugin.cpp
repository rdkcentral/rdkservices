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

#include "NativeJSPlugin.h"

const string WPEFramework::Plugin::NativeJS::SERVICE_NAME = "org.rdk.NativeJS";

using namespace std;

namespace WPEFramework {
    namespace Plugin {

        SERVICE_REGISTRATION(NativeJS, 1, 0);

        NativeJS* NativeJS::_instance = nullptr;

        NativeJS::NativeJS()
            : PluginHost::IPlugin(), PluginHost::JSONRPC(), mService(nullptr), mNativeJS(nullptr)
        {
            NativeJS::_instance = this;
        }

        NativeJS::~NativeJS()
        {
        }

        const string NativeJS::Initialize(PluginHost::IShell* service )
        {
            string message;

            ASSERT(mService == nullptr);
            ASSERT(mNativeJS == nullptr);

            mConnectionId = 0;
            mService = service;

            mNativeJS = mService->Root<Exchange::INativeJS>(mConnectionId, 5000, _T("NativeJSImplementation"));

            if (mNativeJS == nullptr)
            {
                message = _T("NativeJS implementation could not be instantiated.");
                mService = nullptr;
                return message;
            }
            std::string waylandDisplay("");
            mConfig.FromString(service->ConfigLine());
            if (mConfig.Display.IsSet() == true)
            {
                waylandDisplay = mConfig.Display.Value();
            }
            else
            {
                const char* display = getenv("WAYLAND_DISPLAY");
                if (NULL != display)
                {
                   waylandDisplay = display;
                }
            }
            mNativeJS->Initialize(waylandDisplay);
	    Exchange::JNativeJS::Register(*this, mNativeJS);
            return "";
        }

        void NativeJS::Deinitialize(PluginHost::IShell* /* service */)
        {
	    LOGINFO("Deinitializing NativeJS instance");	
            if (mNativeJS != nullptr) {

                Exchange::JNativeJS::Unregister(*this);
                mNativeJS->Deinitialize();

                mNativeJS->Release();
                mNativeJS = nullptr;
                mService = nullptr;
                NativeJS::_instance = nullptr;
                LOGINFO("NativeJS deinitialized successfully");

            }
	}

        string NativeJS::Information() const
        {
            return(string("{\"service\": \"") + SERVICE_NAME + string("\"}"));
        }
        
    } // namespace Plugin
} // namespace WPEFramework
