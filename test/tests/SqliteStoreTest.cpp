/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
 */

#include <gtest/gtest.h>

#include "SqliteStore.h"

#include "StoreNotificationMock.h"

using namespace WPEFramework;

class SqliteStoreTestFixture : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::SqliteStore> store;
    Core::ProxyType<StoreNotificationMock> notification;
    string value;
    std::vector<string> namespaces;
    std::map<string, uint64_t> namespaceSizes;
    std::vector<string> keys;

    SqliteStoreTestFixture()
        : store(Core::ProxyType<Plugin::SqliteStore>::Create())
        , notification(Core::ProxyType<StoreNotificationMock>::Create())
    {
    }
    virtual ~SqliteStoreTestFixture()
    {
    }
};

TEST_F(SqliteStoreTestFixture, interface)
{
    EXPECT_EQ(Core::ERROR_NONE, store->Open("/tmp/rdkservicestore", "", 20, 10));
    EXPECT_EQ(Core::ERROR_NONE, store->SetValue("test", "a", "1"));
    EXPECT_EQ(Core::ERROR_NONE, store->GetValue("test", "a", value));
    EXPECT_EQ(value, "1");
    EXPECT_EQ(Core::ERROR_NONE, store->GetNamespaces(namespaces));
    EXPECT_EQ(namespaces.size(), 1);
    EXPECT_EQ(namespaces.at(0), "test");
    EXPECT_EQ(Core::ERROR_NONE, store->GetStorageSize(namespaceSizes));
    EXPECT_EQ(namespaceSizes.size(), 1);
    EXPECT_EQ(namespaceSizes.at("test"), 2);
    EXPECT_EQ(Core::ERROR_NONE, store->GetKeys("test", keys));
    EXPECT_EQ(keys.size(), 1);
    EXPECT_EQ(keys.at(0), "a");
    EXPECT_EQ(Core::ERROR_NONE, store->DeleteKey("test", "a"));
    EXPECT_EQ(Core::ERROR_NONE, store->DeleteNamespace("test"));
    EXPECT_EQ(Core::ERROR_NONE, store->FlushCache());
    EXPECT_EQ(Core::ERROR_NONE, store->Term());
}

TEST_F(SqliteStoreTestFixture, valueChanged)
{
    EXPECT_CALL(*notification, ValueChanged("test", "a", "12345"))
        .Times(1)
        .WillOnce(
            ::testing::Return());

    EXPECT_EQ(Core::ERROR_NONE, store->Register(&*notification));
    EXPECT_EQ(Core::ERROR_NONE, store->Open("/tmp/rdkservicestore", "", 20, 10));
    EXPECT_EQ(Core::ERROR_NONE, store->SetValue("test", "a", "12345"));
    EXPECT_EQ(Core::ERROR_NONE, store->DeleteNamespace("test"));
    EXPECT_EQ(Core::ERROR_NONE, store->Term());
    EXPECT_EQ(Core::ERROR_NONE, store->Unregister(&*notification));
}

TEST_F(SqliteStoreTestFixture, storageExceeded)
{
    EXPECT_CALL(*notification, ValueChanged("test", "a", "123456789123456789"))
        .Times(1)
        .WillOnce(
            ::testing::Return());
    EXPECT_CALL(*notification, StorageExceeded())
        .Times(1)
        .WillOnce(
            ::testing::Return());

    EXPECT_EQ(Core::ERROR_NONE, store->Register(&*notification));
    EXPECT_EQ(Core::ERROR_NONE, store->Open("/tmp/rdkservicestore", "", 20, 20));
    EXPECT_EQ(Core::ERROR_NONE, store->SetValue("test", "a", "123456789123456789"));
    EXPECT_EQ(Core::ERROR_WRITE_ERROR, store->SetValue("test", "b", "1"));
    EXPECT_EQ(Core::ERROR_NONE, store->GetValue("test", "a", value));
    EXPECT_EQ(value, "123456789123456789");
    EXPECT_EQ(Core::ERROR_NONE, store->DeleteNamespace("test"));
    EXPECT_EQ(Core::ERROR_NONE, store->Term());
    EXPECT_EQ(Core::ERROR_NONE, store->Unregister(&*notification));
}

TEST_F(SqliteStoreTestFixture, maxValue)
{
    EXPECT_EQ(Core::ERROR_NONE, store->Open("/tmp/rdkservicestore", "", 20, 10));
    EXPECT_EQ(Core::ERROR_INVALID_INPUT_LENGTH, store->SetValue("test", "a", "123456789123456789"));
    EXPECT_EQ(Core::ERROR_NONE, store->Term());
}

TEST_F(SqliteStoreTestFixture, corrupt)
{
    EXPECT_EQ(Core::ERROR_NONE, store->Open("/tmp/rdkservicestore", "", 20, 10));
    EXPECT_EQ(Core::ERROR_NONE, store->SetValue("test", "a", "1"));
    EXPECT_EQ(Core::ERROR_NONE, store->GetValue("test", "a", value));
    EXPECT_EQ(value, "1");

    EXPECT_TRUE(Core::File(string("/tmp/rdkservicestore")).Destroy());
    EXPECT_FALSE(Core::File(string("/tmp/rdkservicestore")).Exists());

    EXPECT_EQ(Core::ERROR_NONE, store->FlushCache());
    EXPECT_EQ(Core::ERROR_NONE, store->SetValue("test", "b", "1"));
    EXPECT_EQ(Core::ERROR_NONE, store->GetValue("test", "b", value));
    EXPECT_EQ(value, "1");
    EXPECT_EQ(Core::ERROR_GENERAL, store->GetValue("test", "a", value));
    EXPECT_EQ(Core::ERROR_NONE, store->DeleteNamespace("test"));
    EXPECT_EQ(Core::ERROR_NONE, store->Term());
}

TEST_F(SqliteStoreTestFixture, replaceValue)
{
    EXPECT_EQ(Core::ERROR_NONE, store->Open("/tmp/rdkservicestore", "", 20, 10));
    EXPECT_EQ(Core::ERROR_NONE, store->SetValue("test", "a", "1"));
    EXPECT_EQ(Core::ERROR_NONE, store->GetValue("test", "a", value));
    EXPECT_EQ(value, "1");
    EXPECT_EQ(Core::ERROR_NONE, store->SetValue("test", "a", "2"));
    EXPECT_EQ(Core::ERROR_NONE, store->GetValue("test", "a", value));
    EXPECT_EQ(value, "2");
    EXPECT_EQ(Core::ERROR_NONE, store->DeleteNamespace("test"));
    EXPECT_EQ(Core::ERROR_NONE, store->Term());
}

TEST_F(SqliteStoreTestFixture, unknownKey)
{
    EXPECT_EQ(Core::ERROR_NONE, store->Open("/tmp/rdkservicestore", "", 20, 10));
    EXPECT_EQ(Core::ERROR_GENERAL, store->GetValue("test", "unknown", value));
    EXPECT_EQ(Core::ERROR_NONE, store->Term());
}
