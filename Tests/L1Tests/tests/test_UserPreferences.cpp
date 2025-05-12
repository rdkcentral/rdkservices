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

#if 0

#include "gtest/gtest.h"

#include "UserPreferences.h"

using namespace WPEFramework;

namespace {
const string userPrefFile = _T("/opt/user_preferences.conf");
const uint8_t userPrefLang[] = "[General]\nui_language=US_en\n";
}

class UserPreferencesTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::UserPreferences> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    UserPreferencesTest()
        : plugin(Core::ProxyType<Plugin::UserPreferences>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
    }
    virtual ~UserPreferencesTest() = default;
};

TEST_F(UserPreferencesTest, registeredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getUILanguage")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setUILanguage")));
}

TEST_F(UserPreferencesTest, paramsMissing)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setUILanguage"), _T("{}"), response));
}

TEST_F(UserPreferencesTest, getUILanguage)
{
    //Fail  case: File doesn't exists
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getUILanguage"), _T("{}"), response));

    Core::File file(userPrefFile);
    file.Destroy();
    file.Create();

    //Fail case: No key exists
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getUILanguage"), _T("{}"), response));

    file.Write(userPrefLang, sizeof(userPrefLang));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getUILanguage"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"ui_language\":\"US_en\",\"success\":true}"));

    file.Destroy();
}

TEST_F(UserPreferencesTest, setUILanguage)
{
    Core::File file(userPrefFile);
    file.Create();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setUILanguage"), _T("{\"ui_language\":\"US_en\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getUILanguage"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"ui_language\":\"US_en\",\"success\":true}"));

    file.Destroy();
}

#endif
