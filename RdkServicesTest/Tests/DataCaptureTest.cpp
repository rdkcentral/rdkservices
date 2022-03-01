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

#include "DataCapture.h"

namespace RdkServicesTest {

TEST(DataCaptureTest, test) {
    // create plugin
    auto dataCapture = WPEFramework::Core::ProxyType<WPEFramework::Plugin::DataCapture>::Create();
    WPEFramework::Core::JSONRPC::Handler& handler = *dataCapture;

    // expect methods
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Exists(_T("enableAudioCapture")));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Exists(_T("getAudioClip")));

    // invoke plugin
    WPEFramework::Core::JSONRPC::Connection connection(1, 0);

    // Turn on current session
    string response;
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Invoke(connection, _T("enableAudioCapture"), _T("{\"bufferMaxDuration\":6}"), response));
    EXPECT_EQ(response, _T("{\"error\":0,\"success\":true}"));

    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, 
              handler.Invoke(connection,
                             _T("getAudioClip"),
                             _T("{\"clipRequest\":{\"stream\":\"primary\",\"url\":\"https://192.168.0.1\",\"duration\":8,\"captureMode\":\"preCapture\"}}"),
                             response));
    EXPECT_EQ(response, _T("{\"error\":0,\"success\":true}"));

    // Turn off current session
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Invoke(connection, _T("enableAudioCapture"), _T("{\"bufferMaxDuration\":0}"), response));
    EXPECT_EQ(response, _T("{\"error\":0,\"success\":true}"));

    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, 
              handler.Invoke(connection,
                             _T("getAudioClip"),
                             _T("{\"clipRequest\":{\"stream\":\"primary\",\"url\":\"https://192.168.0.1\",\"duration\":8,\"captureMode\":\"preCapture\"}}"),
                             response));
    EXPECT_EQ(response, _T("{\"error\":0,\"success\":true}"));
}

} // namespace RdkServicesTest
