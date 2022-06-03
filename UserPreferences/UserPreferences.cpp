/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
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

#include "UserPreferences.h"
#include "UtilsJsonRpc.h"

#include <glib.h>
#include <glib/gstdio.h>

#define SETTINGS_FILE_NAME              "/opt/user_preferences.conf"
#define SETTINGS_FILE_KEY               "ui_language"
#define SETTINGS_FILE_GROUP              "General"

using namespace std;

namespace WPEFramework {

    namespace Plugin {

        SERVICE_REGISTRATION(UserPreferences, 1, 0);

        UserPreferences* UserPreferences::_instance = nullptr;

        UserPreferences::UserPreferences()
                : PluginHost::JSONRPC()
        {
            LOGINFO("ctor");
            UserPreferences::_instance = this;
            Register("getUILanguage", &UserPreferences::getUILanguage, this);
            Register("setUILanguage", &UserPreferences::setUILanguage, this);
        }

        UserPreferences::~UserPreferences()
        {
            //LOGINFO("dtor");
        }

        void UserPreferences::Deinitialize(PluginHost::IShell* /* service */)
        {
            LOGINFO("Deinitialize");
            UserPreferences::_instance = nullptr;
        }

        //Begin methods
        uint32_t UserPreferences::getUILanguage(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            g_autoptr(GKeyFile) file = g_key_file_new();

            g_autoptr(GError) error = nullptr;
            if (!g_key_file_load_from_file (file, SETTINGS_FILE_NAME, G_KEY_FILE_NONE, &error))
            {
                LOGERR("Unable to load from file '%s': %s", SETTINGS_FILE_NAME, error->message);
                returnResponse(false);
            }

            g_autofree gchar * val = g_key_file_get_string(file, SETTINGS_FILE_GROUP, SETTINGS_FILE_KEY, &error);
            if (val == nullptr)
            {
                LOGERR("Unable to get key '%s' for group '%s' from file '%s': %s"
                        , SETTINGS_FILE_KEY, SETTINGS_FILE_GROUP, SETTINGS_FILE_NAME, error->message);
                returnResponse(false);
            }

            response[SETTINGS_FILE_KEY] = string(val);

            returnResponse(true);
        }

        uint32_t UserPreferences::setUILanguage(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            returnIfStringParamNotFound(parameters, SETTINGS_FILE_KEY);
            string language = parameters[SETTINGS_FILE_KEY].String();

            g_autoptr(GKeyFile) file = g_key_file_new();
            g_key_file_set_string(file, SETTINGS_FILE_GROUP, SETTINGS_FILE_KEY, (gchar *)language.c_str());

            g_autoptr(GError) error = nullptr;
            if (!g_key_file_save_to_file (file, SETTINGS_FILE_NAME, &error))
            {
                LOGERR("Error to saving file '%s': %s", SETTINGS_FILE_NAME, error->message);
                returnResponse(false);
            }

            returnResponse(true);
        }
        //End methods

        //Begin events
        //End events

    } // namespace Plugin
} // namespace WPEFramework
