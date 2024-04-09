/* If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2023 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
*/

#include <string>
#include <sstream>

#include "Module.h"
#include "L2Tests.h"
#include "UtilsJsonRpc.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

using namespace WPEFramework;

// TODO: remove this
#define registerMethod(...) for (uint8_t i = 1; GetHandler(i); i++) GetHandler(i)->Register<JsonObject, JsonObject>(__VA_ARGS__)
namespace WPEFramework
{
    namespace {

        static Plugin::Metadata<Plugin::L2Tests> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }

    namespace Plugin
    {
        SERVICE_REGISTRATION(L2Tests, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        L2Tests::L2Tests()
            : PluginHost::JSONRPC()
        {

           Register("PerformL2Tests", &L2Tests::PerformL2Tests, this);
           ::testing::InitGoogleTest();
        }

        L2Tests::~L2Tests()
        {
                LOGINFO("L2 test plugin is terminating\n");
        }

        const string L2Tests::Initialize(PluginHost::IShell* /* service */)
        {
               return (string());
        }

        void L2Tests::Deinitialize(PluginHost::IShell* /* service */)
        {

        }

        uint32_t L2Tests::PerformL2Tests(const JsonObject& parameters, JsonObject& response)
        {
            uint32_t status;

            /* Options are passed in params if user wants to set gtest filter to run specific suite */
            if(parameters.HasLabel("test_suite_list"))
            {
                const std::string &message = parameters["test_suite_list"].String();
                LOGINFO("Paramaters passed for gtest filter: :%s\n",
                        message.c_str());
               ::testing::GTEST_FLAG(filter) = message;
            }
            status = RUN_ALL_TESTS();
            LOGINFO("Completed running L2 tests and running status = %d\n",status);

            return status;
        }
    }
}

