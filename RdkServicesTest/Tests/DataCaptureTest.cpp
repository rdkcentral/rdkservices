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

#include <future>
#include <thread>

#include "DataCapture.h"
#include "IarmBusMock.h"

namespace {
const std::string iarmName = _T("Thunder_Plugins");
constexpr const char answer[16] = "TESTING STREAM!";

void runSocket(std::promise<bool> ready, const std::string& fileName)
{
    auto sd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;
    memcpy(serveraddr.sun_path, fileName.c_str(), sizeof(serveraddr.sun_path));

    ASSERT_FALSE(bind(sd, (struct sockaddr*)&serveraddr, SUN_LEN(&serveraddr)) < 0);
    ASSERT_FALSE(listen(sd, 10) < 0);
    ready.set_value(true);

    auto sd2 = accept(sd, NULL, NULL);
 
    write(sd2, answer, 16);

    close(sd2);
    close(sd);
    unlink(fileName.c_str());
}

void runServer()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_TRUE(sockfd != -1);

    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(9999);

    ASSERT_FALSE(bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0);
    ASSERT_FALSE(listen(sockfd, 10) < 0);

    auto addrlen = sizeof(sockaddr);
    const int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
    ASSERT_FALSE(connection < 0);

    char buffer[2048];
    ASSERT_TRUE(read(connection, buffer, 2048) > 0);
    // Expect we got correct message
    EXPECT_TRUE(strstr(buffer, answer) != nullptr);

    std::string response = "POST / HTTP/1.1 200";
    send(connection, response.c_str(), response.size(), 0);

    close(connection);
    close(sockfd);
}
}

using testing::_;
using testing::AtLeast;
using testing::NiceMock;
using testing::Return;
using testing::Test;

namespace WPEFramework {

class DataCaptureTest : public Test {
public:
    DataCaptureTest()
        : dataCapture_(Core::ProxyType<Plugin::DataCapture>::Create())
        , connection_(1, 0)
        , handler_(*dataCapture_)
    {
    }

    virtual void SetUp()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock_;
    }

    virtual void TearDown()
    {
        IarmBus::getInstance().impl = nullptr;
    }

    void initService()
    {
        EXPECT_CALL(iarmBusImplMock_, IARM_Bus_IsConnected)
            .Times(AtLeast(2))
            .WillOnce([](const char* memberName, int* isRegistered) {
                if (iarmName == string(memberName)) {
                    *isRegistered = 0;
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            })
            .WillOnce([](const char* memberName, int* isRegistered) {
                if (iarmName == string(memberName)) {
                    *isRegistered = 1;
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            })
            .WillOnce([](const char* memberName, int* isRegistered) {
                if (iarmName == string(memberName)) {
                    *isRegistered = 1;
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            });

        EXPECT_CALL(iarmBusImplMock_, IARM_Bus_Init)
            .WillOnce([](const char* name) {
                if (iarmName == string(name)) {
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            });

        EXPECT_CALL(iarmBusImplMock_, IARM_Bus_Connect)
            .WillOnce(Return(IARM_RESULT_SUCCESS));

        EXPECT_EQ(string(""), dataCapture_->Initialize(nullptr));
    }

protected:
    Core::ProxyType<Plugin::DataCapture> dataCapture_;
    Core::JSONRPC::Connection connection_;
    Core::JSONRPC::Handler& handler_;
    NiceMock<IarmBusImplMock> iarmBusImplMock_;
};

TEST_F(DataCaptureTest, ShouldRegisterMethod)
{
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("enableAudioCapture")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("getAudioClip")));
}

TEST_F(DataCaptureTest, ShouldReturnErrorWhenParamsAreEmpty)
{
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

TEST_F(DataCaptureTest, ShouldTurnOnAudioCapture)
{
    initService();

    string response;
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler_.Invoke(connection_, _T("enableAudioCapture"), _T("{\"bufferMaxDuration\":6}"), response));
    EXPECT_EQ(response, _T("{\"error\":0,\"success\":true}"));

    EXPECT_EQ(WPEFramework::Core::ERROR_NONE,
        handler_.Invoke(connection_,
            _T("getAudioClip"),
            _T("{\"clipRequest\":{\"stream\":\"primary\",\"url\":\"https://192.168.0.1\",\"duration\":8,\"captureMode\":\"preCapture\"}}"),
            response));
    EXPECT_EQ(response, _T("{\"error\":0,\"success\":true}"));

    dataCapture_->Deinitialize(nullptr);
}

TEST_F(DataCaptureTest, ShouldTurnOffAudioCapture)
{
    initService();

    string response;
    // Turn on audio capture
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler_.Invoke(connection_, _T("enableAudioCapture"), _T("{\"bufferMaxDuration\":6}"), response));
    EXPECT_EQ(response, _T("{\"error\":0,\"success\":true}"));

    // Turn off audio capture
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler_.Invoke(connection_, _T("enableAudioCapture"), _T("{\"bufferMaxDuration\":0}"), response));
    EXPECT_EQ(response, _T("{\"error\":0,\"success\":true}"));
}

TEST_F(DataCaptureTest, ShouldUploadData)
{
    constexpr const char dataLocator[] = "dataLocator123";
    constexpr const char owner[] = "DataCaptureTest";

    std::thread(runServer).detach();

    std::promise<bool> ready;
    auto future = ready.get_future();
    std::thread server(runSocket, std::move(ready), std::string(dataLocator));
    future.wait();

    EXPECT_EQ(std::string{}, dataCapture_->Initialize(nullptr));

    // setup http://127.0.0.1:9999 as url
    string response;
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler_.Invoke(connection_, _T("enableAudioCapture"), _T("{\"bufferMaxDuration\":6}"), response));
    EXPECT_EQ(response, _T("{\"error\":0,\"success\":true}"));

    EXPECT_EQ(WPEFramework::Core::ERROR_NONE,
        handler_.Invoke(connection_,
            _T("getAudioClip"),
            _T("{\"clipRequest\":{\"stream\":\"primary\",\"url\":\"http://127.0.0.1:9999\",\"duration\":8,\"captureMode\":\"preCapture\"}}"),
            response));
    EXPECT_EQ(response, _T("{\"error\":0,\"success\":true}"));

    // Trigger event, curl message will be check by server thread
    iarmbus_notification_payload_t data;
    data.dataLocator = dataLocator;
    dataCapture_->eventHandler(owner, DATA_CAPTURE_IARM_EVENT_AUDIO_CLIP_READY, static_cast<void*>(&data), sizeof(data));

    dataCapture_->Deinitialize(nullptr);
    server.join();
}

} // namespace WPEFramework
