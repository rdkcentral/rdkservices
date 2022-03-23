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

#include "AVInput.h"
#include "Source/WorkerPoolImplementation.h"

namespace WPEFramework {

class AVInputTest : public ::testing::Test {
public:
    AVInputTest(): avInput_(Core::ProxyType<Plugin::AVInput>::Create()), handler_(*avInput_), connection_(1, 0) {
        engine_ = Core::ProxyType<WorkerPoolImplementation>::Create(2, Core::Thread::DefaultStackSize(), 16);
        Core::IWorkerPool::Assign(&(*engine_));
        engine_->Run();
    }

    ~AVInputTest() {
        Core::IWorkerPool::Assign(nullptr);
        engine_.Release();
    }

protected:
    Core::ProxyType<Plugin::AVInput> avInput_;
    Core::JSONRPC::Handler& handler_;
    Core::JSONRPC::Connection connection_;
    Core::ProxyType<WorkerPoolImplementation> engine_;
};

TEST_F(AVInputTest, RegisterMethod) {
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("contentProtected")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("currentVideoMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("numberOfInputs")));
}

TEST_F(AVInputTest, ServiceInitialized) {
    // Initialize and activate plugin
    EXPECT_EQ(std::string(""), avInput_->Initialize(nullptr));

    std::string response;
    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("contentProtected"), _T(""), response));
    EXPECT_EQ(response, _T("{\"isContentProtected\":true,\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE,  handler_.Invoke(connection_, _T("currentVideoMode"),_T(""),response));
    EXPECT_EQ(response, _T("{\"currentVideoMode\":\"1080p60fps\",\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("numberOfInputs"), _T(""), response));
    EXPECT_EQ(response, _T("{\"numberOfInputs\":1,\"success\":true}"));

    // cleanup
    avInput_->Deinitialize(nullptr);
}

} // namespace RdkServicesTest
