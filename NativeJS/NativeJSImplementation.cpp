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

#include "NativeJSImplementation.h"

using namespace std;

namespace WPEFramework
{
    namespace Plugin
    {

        std::string gPendingIdRequest("");
        std::string gPendingIdOptionsRequest("");
	std::string gPendingUrl("");
	SERVICE_REGISTRATION(NativeJSImplementation, 1, 0);

        NativeJSImplementation::NativeJSImplementation()
            : mRunning(true)
        {
            TRACE(Trace::Information, (_T("Constructing NativeJSImplementation Service: %p"), this));
        }

        NativeJSImplementation::~NativeJSImplementation()
        {
            TRACE(Trace::Information, (_T("Destructing NativeJSImplementation Service: %p"), this));
            mNativeJSRenderer = nullptr;
        }

        Core::hresult NativeJSImplementation::Initialize(string waylandDisplay)
        {   
            std::cout << "initialize called on nativejs implementation " << std::endl;
            mRenderThread = std::thread([=](std::string waylandDisplay) {
                mNativeJSRenderer = std::make_shared<NativeJSRenderer>(waylandDisplay);
                //if (!gPendingUrlRequest.empty())
                //{
		//    ModuleSettings moduleSettings;
                //    moduleSettings.fromString(gPendingUrlOptionsRequest);
                //    mNativeJSRenderer->launchApplication(gPendingUrlRequest, moduleSettings);
		//    gPendingUrlRequest = "";
                //    gPendingUrlOptionsRequest = "";
                //}
		
		mNativeJSRenderer->run();
		
		printf("After launch application execution ... \n"); fflush(stdout);
		mNativeJSRenderer.reset();

            }, waylandDisplay);
            return (Core::ERROR_NONE);
        }

        Core::hresult NativeJSImplementation::Deinitialize()
        {
           LOGINFO("deinitializing NativeJS process");
           if (mNativeJSRenderer)
           {
               mNativeJSRenderer->terminate();
               if (mRenderThread.joinable())
               {
                   mRenderThread.join();
               }
           }
	   return (Core::ERROR_NONE);
        }

	Core::hresult NativeJSImplementation::CreateApplication(const std::string options, uint32_t& id)
	{
		LOGINFO("createApplication invoked");
		if(mNativeJSRenderer)
		{
			std::string optionsVal(options);
			ModuleSettings moduleSettings;
			moduleSettings.fromString(optionsVal);
			id = mNativeJSRenderer->createApplication(moduleSettings);
		}
		else 
		{
			gPendingIdOptionsRequest = options;
		}
		return (Core::ERROR_NONE);
	}

	Core::hresult NativeJSImplementation::RunApplication(uint32_t id, const std::string url)
	{
		LOGINFO("runApplication invoked");
		if(mNativeJSRenderer)
		{
			std::string Url(url);
			mNativeJSRenderer->runApplication(id, Url);
		}
		else
		{
			gPendingUrl = url;
			LOGINFO("runApplication Couldn't execute");
		}
		return (Core::ERROR_NONE);
	}

	Core::hresult NativeJSImplementation::RunJavaScript(uint32_t id, const std::string code)
	{
		LOGINFO("runJavaScript invoked");
		if(mNativeJSRenderer)
		{
			std::string Code(code);
			mNativeJSRenderer->runJavaScript(id, Code);
		}
		else
		{
			LOGINFO("runJavaScript couldn't execute");
		}
		return (Core::ERROR_NONE);
	}

	Core::hresult NativeJSImplementation::GetApplications()
	{
		LOGINFO("getApplication invoked");
		if(mNativeJSRenderer)
		{
			mNativeJSRenderer->getApplications();
		}
		else
		{
			LOGINFO("getApplication couldn't execute");
		}
		return (Core::ERROR_NONE);
	}

	Core::hresult NativeJSImplementation::TerminateApplication(uint32_t id)
	{
		LOGINFO("terminateApplication invoked");
		if(mNativeJSRenderer)
		{
			mNativeJSRenderer->terminateApplication(id);
		}
		else
		{
			LOGINFO("Application couldn't be terminated");
		}
		return (Core::ERROR_NONE);
	}
} // namespace Plugin
} // namespace WPEFramework
