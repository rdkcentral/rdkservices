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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "SqliteStore.h"

using namespace WPEFramework;

namespace {
class StoreNotificationMock : public WPEFramework::Exchange::IStore::INotification {
public:
    virtual ~StoreNotificationMock() = default;

    MOCK_METHOD(void, ValueChanged, (const string&, const string&, const string&), (override));
    MOCK_METHOD(void, StorageExceeded, (), (override));

    BEGIN_INTERFACE_MAP(StoreNotificationMock)
    INTERFACE_ENTRY(WPEFramework::Exchange::IStore::INotification)
    END_INTERFACE_MAP
};
}

class SqliteStoreTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::SqliteStore> store;

    SqliteStoreTest()
        : store(Core::ProxyType<Plugin::SqliteStore>::Create())
    {
    }
    virtual ~SqliteStoreTest() = default;
};

class SqliteStoreNotificationTest : public SqliteStoreTest {
protected:
    Core::ProxyType<StoreNotificationMock> notification;

    SqliteStoreNotificationTest()
        : SqliteStoreTest()
        , notification(Core::ProxyType<StoreNotificationMock>::Create())
    {
        EXPECT_EQ(Core::ERROR_NONE, store->Register(&*notification));
    }
    virtual ~SqliteStoreNotificationTest() override
    {
        EXPECT_EQ(Core::ERROR_NONE, store->Unregister(&*notification));
    }
};

TEST_F(SqliteStoreTest, interface)
{
    string value;
    std::vector<string> namespaces;
    std::map<string, uint64_t> namespaceSizes;
    std::vector<string> keys;

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

TEST_F(SqliteStoreNotificationTest, valueChanged)
{
    EXPECT_CALL(*notification, ValueChanged("test", "a", "12345"))
        .Times(1)
        .WillOnce(::testing::Return());

    EXPECT_EQ(Core::ERROR_NONE, store->Open("/tmp/rdkservicestore", "", 20, 10));
    EXPECT_EQ(Core::ERROR_NONE, store->SetValue("test", "a", "12345"));
    EXPECT_EQ(Core::ERROR_NONE, store->DeleteNamespace("test"));
    EXPECT_EQ(Core::ERROR_NONE, store->Term());
}

TEST_F(SqliteStoreNotificationTest, storageExceeded)
{
    string value;

    EXPECT_CALL(*notification, ValueChanged("test", "a", "123456789123456789"))
        .Times(1)
        .WillOnce(::testing::Return());
    EXPECT_CALL(*notification, StorageExceeded())
        .Times(1)
        .WillOnce(::testing::Return());

    EXPECT_EQ(Core::ERROR_NONE, store->Open("/tmp/rdkservicestore", "", 20, 20));
    EXPECT_EQ(Core::ERROR_NONE, store->SetValue("test", "a", "123456789123456789"));
    EXPECT_EQ(Core::ERROR_WRITE_ERROR, store->SetValue("test", "b", "1"));
    EXPECT_EQ(Core::ERROR_NONE, store->GetValue("test", "a", value));
    EXPECT_EQ(value, "123456789123456789");
    EXPECT_EQ(Core::ERROR_NONE, store->DeleteNamespace("test"));
    EXPECT_EQ(Core::ERROR_NONE, store->Term());
}

TEST_F(SqliteStoreTest, maxValue)
{
    EXPECT_EQ(Core::ERROR_NONE, store->Open("/tmp/rdkservicestore", "", 20, 10));
    EXPECT_EQ(Core::ERROR_INVALID_INPUT_LENGTH, store->SetValue("test", "a", "123456789123456789"));
    EXPECT_EQ(Core::ERROR_NONE, store->Term());
}

TEST_F(SqliteStoreTest, corrupt)
{
    string value;

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

TEST_F(SqliteStoreTest, replaceValue)
{
    string value;

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

TEST_F(SqliteStoreTest, unknownKey)
{
    string value;

    EXPECT_EQ(Core::ERROR_NONE, store->Open("/tmp/rdkservicestore", "", 20, 10));
    EXPECT_EQ(Core::ERROR_GENERAL, store->GetValue("test", "unknown", value));
    EXPECT_EQ(Core::ERROR_NONE, store->Term());
}
