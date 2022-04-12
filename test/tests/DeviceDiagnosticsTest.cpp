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

#include "DeviceDiagnostics.h"

using namespace WPEFramework;

class DeviceDiagnosticsTest : public ::testing::Test {
public:
    DeviceDiagnosticsTest()
        : deviceDiagnostic_(Core::ProxyType<Plugin::DeviceDiagnostics>::Create())
        , handler_(*deviceDiagnostic_)
        , connection_(1, 0)
    {
    }

protected:
    Core::ProxyType<Plugin::DeviceDiagnostics> deviceDiagnostic_;
    Core::JSONRPC::Handler& handler_;
    Core::JSONRPC::Connection connection_;
};

TEST_F(DeviceDiagnosticsTest, RegisterMethod)
{
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("getConfiguration")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("getAVDecoderStatus")));
}

TEST_F(DeviceDiagnosticsTest, ShouldReturnResponse)
{
    string response;
    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getConfiguration"), _T("{\"names\":[\"test\"]}"), response));
    // TODO pass here expected response
    EXPECT_EQ(response, _T("{\"paramList\":[\"Device.X_CISCO_COM_LED.RedPwm\":123],\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getAVDecoderStatus"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"avDecoderStatus\":\"IDLE\",\"success\":true}"));
}
