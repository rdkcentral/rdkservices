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

#include "PersistentStore.h"

namespace RdkServicesTest {

TEST(PersistentStoreTest, test) {
    // create plugin

    WPEFramework::Core::ProxyType <WPEFramework::Plugin::PersistentStore> persistentStore;
    persistentStore = WPEFramework::Core::ProxyType <WPEFramework::Plugin::PersistentStore>::Create();

    WPEFramework::Core::JSONRPC::Handler& handler = *persistentStore;

    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Exists(_T("setValue")));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Exists(_T("getValue")));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Exists(_T("deleteKey")));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Exists(_T("deleteNamespace")));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Exists(_T("getKeys")));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Exists(_T("getNamespaces")));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Exists(_T("getStorageSize")));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Exists(_T("flushCache")));

    // init plugin

    EXPECT_EQ(string(""), persistentStore->Initialize(nullptr));

    // invoke plugin

    WPEFramework::Core::JSONRPC::Connection connection(1, 0);

    string response;
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Invoke(connection, _T("setValue"), _T("{\"namespace\":\"test\",\"key\":\"a\",\"value\":\"1\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Invoke(connection, _T("getValue"), _T("{\"namespace\":\"test\",\"key\":\"a\"}"), response));
    EXPECT_EQ(response, _T("{\"value\":\"1\",\"success\":true}"));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Invoke(connection, _T("getNamespaces"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"namespaces\":[\"test\"],\"success\":true}"));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Invoke(connection, _T("getStorageSize"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"namespaceSizes\":{\"test\":2},\"success\":true}"));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Invoke(connection, _T("getKeys"), _T("{\"namespace\":\"test\"}"), response));
    EXPECT_EQ(response, _T("{\"keys\":[\"a\"],\"success\":true}"));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Invoke(connection, _T("deleteKey"), _T("{\"namespace\":\"test\",\"key\":\"a\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Invoke(connection, _T("deleteNamespace"), _T("{\"namespace\":\"test\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(WPEFramework::Core::ERROR_NONE, handler.Invoke(connection, _T("flushCache"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));

    // clean up

    persistentStore->Deinitialize(nullptr);
}

} // namespace RdkServicesTest
