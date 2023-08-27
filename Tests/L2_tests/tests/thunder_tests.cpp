# If not stated otherwise in this file or this component's LICENSE file the
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

#include <cstdio>
#include "Module.h"
#include "thunder_test_base.h"

class ThunderTest : public ThunderTestBase
{
};

/**
 * @brief Start and stop the Thunder process
 */
TEST_F(ThunderTest, SetupThunder)
{
}

/**
 * @brief Test duplicate activation of the plugin
 *
 * Note that the plugin is first activated in ThunderTestBase::SetUp.
 */
TEST_F(ThunderTest, DuplicateActivate)
{
    uint32_t status;

    status = Activate("org.rdk.System");
    EXPECT_EQ(0U, status);
}

/**
 * @brief Test duplicate deactivation of plugin
 *
 * Note that the plugin is also deactivated in ThunderTestBase::TearDown.
 */
TEST_F(ThunderTest, DuplicateDeactivate)
{
    uint32_t status;

    status = Deactivate("org.rdk.System");
    EXPECT_EQ(0U, status);
}

/**
 * @brief Test SystemService hasRebootBeenRequested methods
 *
*/
TEST_F(ThunderTest, SystemServicehasRebootBeenRequested)
{
    JsonObject params;
    JsonObject result;
    uint32_t status;

    status = InvokeMethod("org.rdk.System.1", "hasRebootBeenRequested", params, result);
    EXPECT_EQ(0U, status);
}
/**
 * @brief Test SystemService set and get temperature methods
 *
*/
TEST_F(ThunderTest, SystemServiceSetGetTemperature)
{
    JsonObject params;
    JsonObject result;
    uint32_t status;

    status = InvokeMethod("org.rdk.System.1", "getCoreTemperature", params, result);
    EXPECT_EQ(0U, status);
}




