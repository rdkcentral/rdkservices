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
#include "gtest/gtest.h"
#include "FrameRate.h"
#include "ServiceMock.h"

namespace RdkServicesTest{

using namespace WPEFramework;	
class FrameRateTestFixture : public ::testing::Test
{
	protected:
		Core::ProxyType<Plugin::FrameRate> plugin;
		Core::JSONRPC::Handler &handler;	//for version 1
		Core::JSONRPC::Handler &handler2;	//for version 2
		Core::JSONRPC::Connection connection;
		string response;
        	ServiceMock service;
	
		FrameRateTestFixture() : 
			plugin(Core::ProxyType <Plugin::FrameRate>::Create()),
			handler(*plugin),
			handler2(*(plugin->GetHandler(2))),
			connection(1,0)
		{
		}

		~FrameRateTestFixture()
		{
		}

};

TEST_F(FrameRateTestFixture, registeredMethod)
{
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Exists(_T("setCollectionFrequency")));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Exists(_T("startFpsCollection")));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Exists(_T("stopFpsCollection")));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Exists(_T("updateFps")));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler2.Exists(_T("getDisplayFrameRate")));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler2.Exists(_T("setDisplayFrameRate")));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler2.Exists(_T("setFrmMode")));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler2.Exists(_T("getFrmMode")));

}

TEST_F(FrameRateTestFixture, params) {

    EXPECT_EQ(string(""), plugin->Initialize(nullptr));
 
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Invoke(connection, _T("setCollectionFrequency"), _T("{\"params\":1000}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Invoke(connection, _T("startFpsCollection"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Invoke(connection, _T("stopFpsCollection"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Invoke(connection, _T("updateFps"), _T("{\"newFpsValue\":60}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler2.Invoke(connection, _T("getDisplayFrameRate"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"framerate\":\"\",\"success\":true}"));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler2.Invoke(connection, _T("setDisplayFrameRate"), _T("{\"framerate\":\"3840x2160px48\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler2.Invoke(connection, _T("setFrmMode"), _T("{\"frmmode\":0}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler2.Invoke(connection, _T("getFrmMode"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"auto-frm-mode\":0,\"success\":true}"));

    plugin->Deinitialize(&service);

}

}

