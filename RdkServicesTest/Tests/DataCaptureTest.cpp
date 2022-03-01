/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2022 RDK Management
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

namespace WPEFramework {

class DataCaptureTest : public ::testing::Test {
public:
    DataCaptureTest(): dataCapture_(Core::ProxyType<Plugin::DataCapture>::Create()), handler_(*dataCapture_), connection_(1, 0) {
    }

protected:
    Core::ProxyType<Plugin::DataCapture> dataCapture_;
    Core::JSONRPC::Handler& handler_;
    Core::JSONRPC::Connection connection_;
};

TEST_F(DataCaptureTest, ShouldRegisterMethod) {
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("enableAudioCapture")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("getAudioClip")));
}

TEST_F(DataCaptureTest, ShouldReturnErrorWhenParamsAreEmpty) {
    string response;
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler_.Invoke(connection_, _T("enableAudioCapture"), _T(""), response));
    EXPECT_EQ(response, _T("{\"success\":false}"));

    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, 
              handler_.Invoke(connection_,
                             _T("getAudioClip"),
                             _T(""),
                             response));
    EXPECT_EQ(response, _T("{\"success\":false}"));
}

TEST_F(DataCaptureTest, ShouldTurnOnAudioCapture) {
    string response;
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler_.Invoke(connection_, _T("enableAudioCapture"), _T("{\"bufferMaxDuration\":6}"), response));
    EXPECT_EQ(response, _T("{\"error\":0,\"success\":true}"));

    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, 
              handler_.Invoke(connection_,
                             _T("getAudioClip"),
                             _T("{\"clipRequest\":{\"stream\":\"primary\",\"url\":\"https://192.168.0.1\",\"duration\":8,\"captureMode\":\"preCapture\"}}"),
                             response));
    EXPECT_EQ(response, _T("{\"error\":0,\"success\":true}"));
}

TEST_F(DataCaptureTest, ShouldTurnOffAudioCapture) {
    string response;
    // Turn on audio capture
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler_.Invoke(connection_, _T("enableAudioCapture"), _T("{\"bufferMaxDuration\":6}"), response));
    EXPECT_EQ(response, _T("{\"error\":0,\"success\":true}"));

    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, 
              handler_.Invoke(connection_,
                             _T("getAudioClip"),
                             _T("{\"clipRequest\":{\"stream\":\"primary\",\"url\":\"https://192.168.0.1\",\"duration\":8,\"captureMode\":\"preCapture\"}}"),
                             response));
    EXPECT_EQ(response, _T("{\"error\":0,\"success\":true}"));

    // Turn off audio capture
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler_.Invoke(connection_, _T("enableAudioCapture"), _T("{\"bufferMaxDuration\":0}"), response));
    EXPECT_EQ(response, _T("{\"error\":0,\"success\":true}"));

    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, 
              handler_.Invoke(connection_,
                             _T("getAudioClip"),
                             _T("{\"clipRequest\":{\"stream\":\"primary\",\"url\":\"https://192.168.0.1\",\"duration\":8,\"captureMode\":\"preCapture\"}}"),
                             response));
    EXPECT_EQ(response, _T("{\"error\":0,\"success\":true}"));
}

} // namespace WPEFramework
