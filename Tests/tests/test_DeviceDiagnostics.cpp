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
protected:
    Core::ProxyType<Plugin::DeviceDiagnostics> deviceDiagnostic_;
    Core::JSONRPC::Handler& handler_;
    string response;
    Core::JSONRPC::Connection connection_;

    DeviceDiagnosticsTest()
        : deviceDiagnostic_(Core::ProxyType<Plugin::DeviceDiagnostics>::Create())
        , handler_(*deviceDiagnostic_)
        , connection_(1, 0)
    {
    }
};

TEST_F(DeviceDiagnosticsTest, RegisterMethod)
{
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("getConfiguration")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("getAVDecoderStatus")));
}

/**
 * fails without valgrind
 * 2022-12-22T13:10:15.1620146Z Value of: bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0
2022-12-22T13:10:15.1620468Z   Actual: true
2022-12-22T13:10:15.1620732Z Expected: false
2022-12-22T13:10:15.1635528Z [  FAILED  ] DeviceDiagnosticsTest.getConfiguration (9 ms)
 */
TEST_F(DeviceDiagnosticsTest, DISABLED_getConfiguration)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_TRUE(sockfd != -1);
    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(10999);
    ASSERT_FALSE(bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0);
    ASSERT_FALSE(listen(sockfd, 10) < 0);

    std::thread thread = std::thread([&]() {
        auto addrlen = sizeof(sockaddr);
        const int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
        ASSERT_FALSE(connection < 0);
        char buffer[2048] = { 0 };
        ASSERT_TRUE(read(connection, buffer, 2048) > 0);
        EXPECT_EQ(string(buffer), string(_T("POST / HTTP/1.1\r\nHost: 127.0.0.1:10999\r\nAccept: */*\r\nContent-Length: 31\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n{\"paramList\":[{\"name\":\"test\"}]}")));
        std::string response = _T("HTTP/1.1 200\n\rContent-type: application/json\n\r{\"paramList\":[\"Device.X_CISCO_COM_LED.RedPwm\":123],\"success\":true}");
        send(connection, response.c_str(), response.size(), 0);
        close(connection);
    });

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getConfiguration"), _T("{\"names\":[\"test\"]}"), response));
    EXPECT_EQ(response, _T("{\"paramList\":[\"Device.X_CISCO_COM_LED.RedPwm\":123],\"success\":true}"));

    thread.join();

    close(sockfd);
}

TEST_F(DeviceDiagnosticsTest, getAVDecoderStatus)
{
    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getAVDecoderStatus"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"avDecoderStatus\":\"IDLE\",\"success\":true}"));
}
