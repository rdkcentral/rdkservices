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

#include <gtest/gtest.h>

#include <future>
#include <thread>

#include "DataCapture.h"
#include "FactoriesImplementation.h"
#include "IarmBusMock.h"
#include "ServiceMock.h"

using ::testing::NiceMock;

namespace {
const std::string iarmName = _T("Thunder_Plugins");
constexpr const char answer[16] = "TESTING STREAM!";

void runSocket(std::promise<bool> ready, const std::string& fileName)
{
    auto sd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;
    memcpy(serveraddr.sun_path, fileName.c_str(), fileName.size() + 1);

    ASSERT_FALSE(bind(sd, (struct sockaddr*)&serveraddr, SUN_LEN(&serveraddr)) < 0);
    ASSERT_FALSE(listen(sd, 10) < 0);
    ready.set_value(true);

    auto sd2 = accept(sd, NULL, NULL);

    write(sd2, answer, 16);

    close(sd2);
    close(sd);
    unlink(fileName.c_str());
}

void runServer(std::promise<bool> ready)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_TRUE(sockfd != -1);

    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(9999);

    ASSERT_FALSE(bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0);
    ASSERT_FALSE(listen(sockfd, 10) < 0);
    ready.set_value(true);

    auto addrlen = sizeof(sockaddr);
    const int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
    ASSERT_FALSE(connection < 0);

    char buffer[2048];
    ASSERT_TRUE(read(connection, buffer, 2048) > 0);
    // Expect we got correct message
    EXPECT_TRUE(strstr(buffer, answer) != nullptr);

    // Return the simplest response
    std::string response = "HTTP/1.1 200\n\rContent-Length: 0";
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

using namespace WPEFramework;

class DataCaptureTest : public Test {
protected:
    Core::ProxyType<Plugin::DataCapture> dataCapture_;
    Core::JSONRPC::Connection connection_;
    Core::JSONRPC::Handler& handler_;

    DataCaptureTest()
        : dataCapture_(Core::ProxyType<Plugin::DataCapture>::Create())
        , connection_(1, 0)
        , handler_(*dataCapture_)
    {
    }
    virtual ~DataCaptureTest() = default;
};

class DataCaptureInitializedTest : public DataCaptureTest {
protected:
    IarmBusImplMock   *p_iarmBusImplMock = nullptr ;

    DataCaptureInitializedTest()
        : DataCaptureTest()
    {
        p_iarmBusImplMock  = new NiceMock <IarmBusImplMock>;
        IarmBus::setImpl(p_iarmBusImplMock);


        EXPECT_EQ(string(""), dataCapture_->Initialize(nullptr));
    }
    virtual ~DataCaptureInitializedTest() override
    {
        dataCapture_->Deinitialize(nullptr);

        IarmBus::setImpl(nullptr);
        if (p_iarmBusImplMock != nullptr)
        {
            delete p_iarmBusImplMock;
            p_iarmBusImplMock = nullptr;
        }

    }
};

class DataCaptureInitializedEnableAudioCaptureTest : public DataCaptureInitializedTest {
protected:
    DataCaptureInitializedEnableAudioCaptureTest()
        : DataCaptureInitializedTest()
    {
        ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
            .WillByDefault(
                [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                    if (strcmp(methodName, IARMBUS_AUDIOCAPTUREMGR_OPEN) == 0) {
                        auto* param = static_cast<iarmbus_acm_arg_t*>(arg);
                        param->session_id = 10;
                        param->result = 0;
                    } else if (strcmp(methodName, IARMBUS_AUDIOCAPTUREMGR_GET_OUTPUT_PROPS) == 0) {
                        auto* param = static_cast<iarmbus_acm_arg_t*>(arg);
                        param->details.arg_output_props.output.max_buffer_duration = 6;
                        param->result = 0;
                    } else if (strcmp(methodName, IARMBUS_AUDIOCAPTUREMGR_SET_OUTPUT_PROPERTIES) == 0) {
                        auto* param = static_cast<iarmbus_acm_arg_t*>(arg);
                        param->result = 0;
                    } else if (strcmp(methodName, IARMBUS_AUDIOCAPTUREMGR_GET_AUDIO_PROPS) == 0) {
                        auto* param = static_cast<iarmbus_acm_arg_t*>(arg);
                        audiocapturemgr::audio_properties_ifce_t answer;
                        answer.format = acmFormate16BitStereo;
                        answer.sampling_frequency = acmFreqe48000;
                        param->details.arg_audio_properties = answer;
                        param->result = 0;
                    } else if (strcmp(methodName, IARMBUS_AUDIOCAPTUREMGR_START) == 0) {
                        auto* param = static_cast<iarmbus_acm_arg_t*>(arg);
                        param->result = 0;
                    }
                    return IARM_RESULT_SUCCESS;
                });

        string response;
        EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("enableAudioCapture"), _T("{\"bufferMaxDuration\":6}"), response));
        EXPECT_EQ(response, _T("{\"error\":0,\"success\":true}"));
    }
    virtual ~DataCaptureInitializedEnableAudioCaptureTest() override = default;
};

class DataCaptureInitializedEnableAudioCaptureEventTest : public DataCaptureInitializedEnableAudioCaptureTest {
protected:
    ServiceMock service_;
    Core::JSONRPC::Message message_;
    FactoriesImplementation factoriesImplementation_;
    PluginHost::IDispatcher* dispatcher;

    DataCaptureInitializedEnableAudioCaptureEventTest()
        : DataCaptureInitializedEnableAudioCaptureTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation_);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            dataCapture_->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service_);
    }
    virtual ~DataCaptureInitializedEnableAudioCaptureEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();

        PluginHost::IFactories::Assign(nullptr);
    }
};

TEST_F(DataCaptureTest, ShouldRegisterMethod)
{
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("enableAudioCapture")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("getAudioClip")));
}

TEST_F(DataCaptureTest, ShouldReturnErrorWhenParamsAreEmpty)
{
    string response;
    EXPECT_EQ(Core::ERROR_GENERAL, handler_.Invoke(connection_, _T("enableAudioCapture"), _T(""), response));
    EXPECT_EQ(Core::ERROR_GENERAL, handler_.Invoke(connection_, _T("getAudioClip"), _T(""), response));
}

TEST_F(DataCaptureInitializedEnableAudioCaptureTest, ShouldTurnOnAudioCapture)
{
    ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                if (strcmp(methodName, IARMBUS_AUDIOCAPTUREMGR_REQUEST_SAMPLE) == 0) {
                    auto* param = static_cast<iarmbus_acm_arg_t*>(arg);
                    param->result = 0;
                }
                return IARM_RESULT_SUCCESS;
            });

    string response;
    EXPECT_EQ(Core::ERROR_NONE,
        handler_.Invoke(connection_,
            _T("getAudioClip"),
            _T("{\"clipRequest\":{\"stream\":\"primary\",\"url\":\"https://192.168.0.1\",\"duration\":8,\"captureMode\":\"preCapture\"}}"),
            response));
    EXPECT_EQ(response, _T("{\"error\":0,\"success\":true}"));
}

TEST_F(DataCaptureInitializedEnableAudioCaptureTest, ShouldTurnOffAudioCapture)
{
    ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                if (strcmp(methodName, IARMBUS_AUDIOCAPTUREMGR_STOP) == 0) {
                    auto* param = static_cast<iarmbus_acm_arg_t*>(arg);
                    param->result = 0;
                } else if (strcmp(methodName, IARMBUS_AUDIOCAPTUREMGR_CLOSE) == 0) {
                    auto* param = static_cast<iarmbus_acm_arg_t*>(arg);
                    param->result = 0;
                }
                return IARM_RESULT_SUCCESS;
            });

    string response;
    // Turn off audio capture
    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("enableAudioCapture"), _T("{\"bufferMaxDuration\":0}"), response));
    EXPECT_EQ(response, _T("{\"error\":0,\"success\":true}"));
}

/**
 * fails without valgrind
 * 2022-12-22T13:10:14.1251887Z Expected equality of these values:
2022-12-22T13:10:14.1252364Z   text
2022-12-22T13:10:14.1254516Z     Which is: "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.dataCapture.onAudioClipReady\",\"params\":{\"fileName\":\"dataLocator123\",\"status\":false,\"message\":\"Upload Failed: 7:'Couldn't connect to server'\"}}"
2022-12-22T13:10:14.1256178Z   string("{" "\"jsonrpc\":\"2.0\"," "\"method\":\"org.rdk.dataCapture.onAudioClipReady\"," "\"params\":{\"fileName\":\"dataLocator123\",\"status\":true,\"message\":\"Success\"}" "}")
2022-12-22T13:10:14.1257672Z     Which is: "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.dataCapture.onAudioClipReady\",\"params\":{\"fileName\":\"dataLocator123\",\"status\":true,\"message\":\"Success\"}}"
 */
TEST_F(DataCaptureInitializedEnableAudioCaptureEventTest, DISABLED_ShouldUploadData)
{
    constexpr const char dataLocator[] = "dataLocator123";
    constexpr const char owner[] = "DataCaptureTest";

    std::promise<bool> serverReady;
    auto serverReadyFuture = serverReady.get_future();
    std::thread serverThread(runServer, std::move(serverReady));

    std::promise<bool> socketReady;
    auto socketReadyFuture = socketReady.get_future();
    std::thread socketThread(runSocket, std::move(socketReady), std::string(dataLocator));

    // Wait for server and socket thread
    serverReadyFuture.wait();
    socketReadyFuture.wait();

    handler_.Subscribe(0, _T("onAudioClipReady"), _T("org.rdk.dataCapture"), message_);

    EXPECT_CALL(service_, Submit)
        .WillOnce(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.dataCapture.onAudioClipReady\","
                                          "\"params\":{\"fileName\":\"dataLocator123\",\"status\":true,\"message\":\"Success\"}"
                                          "}")));

                return Core::ERROR_NONE;
            });

    ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                if (strcmp(methodName, IARMBUS_AUDIOCAPTUREMGR_REQUEST_SAMPLE) == 0) {
                    auto* param = static_cast<iarmbus_acm_arg_t*>(arg);
                    param->result = 0;
                }
                return IARM_RESULT_SUCCESS;
            });

    // setup http://127.0.0.1:9999 as url
    string response;
    EXPECT_EQ(Core::ERROR_NONE,
        handler_.Invoke(connection_,
            _T("getAudioClip"),
            _T("{\"clipRequest\":{\"stream\":\"primary\",\"url\":\"http://127.0.0.1:9999\",\"duration\":8,\"captureMode\":\"preCapture\"}}"),
            response));
    EXPECT_EQ(response, _T("{\"error\":0,\"success\":true}"));

    // Trigger event, curl message will be check by server thread
    iarmbus_notification_payload_t data;
    strncpy(data.dataLocator, dataLocator, sizeof(data.dataLocator));
    dataCapture_->eventHandler(owner, DATA_CAPTURE_IARM_EVENT_AUDIO_CLIP_READY, static_cast<void*>(&data), sizeof(data));

    handler_.Unsubscribe(0, _T("onAudioClipReady"), _T("org.rdk.dataCapture"), message_);

    serverThread.join();
    socketThread.join();
}
