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

#include "definitions.h"

namespace RdkServicesTest {
namespace Tests {

class SecurityAgentTest : public ::testing::Test {
protected:
  SecurityAgentTest()
      : link("SecurityAgent.1"),
        web("SecurityAgent") {}

  Fixtures::Link link;
  Fixtures::Web web;
  JsonObject result;
};

TEST_F(SecurityAgentTest, activate) {
  EXPECT_ERROR_NONE(link.activate());
}

TEST_F(SecurityAgentTest, validate) {
  EXPECT_ERROR_NONE(link.invoke("validate", "{\"token\":\"" + securityToken() + "\"}", result));

  EXPECT_TRUE(result.Get("valid").Boolean());
}

TEST_F(SecurityAgentTest, validateWeb) {
  EXPECT_ERROR_NONE(web.get("Valid"));
}

} // namespace Tests
} // namespace RdkServicesTest
