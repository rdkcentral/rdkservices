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

class PersistentStoreTest : public ::testing::Test {
protected:
  PersistentStoreTest()
      : link("org.rdk.PersistentStore.1") {}

  Fixtures::Link link;
  JsonObject result;
};

TEST_F(PersistentStoreTest, activate) {
  EXPECT_ERROR_NONE(link.activate());
}

TEST_F(PersistentStoreTest, setValue) {
  EXPECT_SUCCESS(result, link.invoke("setValue", "{\"namespace\":\"test\",\"key\":\"a\",\"value\":\"1\"}", result));
}

TEST_F(PersistentStoreTest, getValue) {
  EXPECT_SUCCESS(result, link.invoke("getValue", "{\"namespace\":\"test\",\"key\":\"a\"}", result));

  EXPECT_EQ(string("1"), result.Get("value").String());
}

TEST_F(PersistentStoreTest, getNamespaces) {
  EXPECT_SUCCESS(result, link.invoke("getNamespaces", "{}", result));

  EXPECT_TRUE(result.Get("namespaces").IsSet());
}

TEST_F(PersistentStoreTest, getStorageSize) {
  EXPECT_SUCCESS(result, link.invoke("getStorageSize", "{}", result));

  EXPECT_EQ(2, result.Get("namespaceSizes").Object().Get("test").Number());
}

TEST_F(PersistentStoreTest, getKeys) {
  EXPECT_SUCCESS(result, link.invoke("getKeys", "{\"namespace\":\"test\"}", result));

  EXPECT_EQ(1, result.Get("keys").Array().Length());
}

TEST_F(PersistentStoreTest, deleteKey) {
  EXPECT_SUCCESS(result, link.invoke("deleteKey", "{\"namespace\":\"test\",\"key\":\"a\"}", result));
}

TEST_F(PersistentStoreTest, deleteNamespace) {
  EXPECT_SUCCESS(result, link.invoke("deleteNamespace", "{\"namespace\":\"test\"}", result));
}

TEST_F(PersistentStoreTest, flushCache) {
  EXPECT_SUCCESS(result, link.invoke("flushCache", "{}", result, 5000));
}

} // namespace Tests
} // namespace RdkServicesTest
