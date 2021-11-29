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

class LoggingPreferencesTest : public ::testing::Test {
protected:
  LoggingPreferencesTest()
      : link("org.rdk.LoggingPreferences.1"),
        event(false, true),
        eventHandler([&](const JsonObject &parameters) -> void {
          eventParams = parameters;

          event.SetEvent();
        }) {}

  Fixtures::Link link;
  JsonObject result;

  WPEFramework::Core::Event event;
  std::function<void(const JsonObject &)> eventHandler;
  JsonObject eventParams;
};

TEST_F(LoggingPreferencesTest, activate) {
  EXPECT_ERROR_NONE(link.activate());
}

TEST_F(LoggingPreferencesTest, isKeystrokeMaskEnabled_default) {
  EXPECT_SUCCESS(result, link.invoke("isKeystrokeMaskEnabled", "{}", result));

  EXPECT_FALSE(result.Get("keystrokeMaskEnabled").Boolean());
}

TEST_F(LoggingPreferencesTest, setKeystrokeMaskEnabled_true) {
  EXPECT_ERROR_NONE(link->Subscribe<JsonObject>(1000, "onKeystrokeMaskEnabledChange", eventHandler));

  EXPECT_SUCCESS(result, link.invoke("setKeystrokeMaskEnabled", "{\"keystrokeMaskEnabled\":true}", result));

  EXPECT_ERROR_NONE(event.Lock(1000));

  EXPECT_TRUE(eventParams.Get("keystrokeMaskEnabled").Boolean());
}

TEST_F(LoggingPreferencesTest, isKeystrokeMaskEnabled_true) {
  EXPECT_SUCCESS(result, link.invoke("isKeystrokeMaskEnabled", "{}", result));
  
  EXPECT_TRUE(result.Get("keystrokeMaskEnabled").Boolean());
}

TEST_F(LoggingPreferencesTest, setKeystrokeMaskEnabled_false) {
  EXPECT_ERROR_NONE(link->Subscribe<JsonObject>(1000, "onKeystrokeMaskEnabledChange", eventHandler));

  EXPECT_SUCCESS(result, link.invoke("setKeystrokeMaskEnabled", "{\"keystrokeMaskEnabled\":false}", result));

  EXPECT_ERROR_NONE(event.Lock(1000));

  EXPECT_FALSE(eventParams.Get("keystrokeMaskEnabled").Boolean());
}

TEST_F(LoggingPreferencesTest, isKeystrokeMaskEnabled_false) {
  EXPECT_SUCCESS(result, link.invoke("isKeystrokeMaskEnabled", "{}", result));
  
  EXPECT_FALSE(result.Get("keystrokeMaskEnabled").Boolean());
}

} // namespace Tests
} // namespace RdkServicesTest
