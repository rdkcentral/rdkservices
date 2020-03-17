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

#include "Proxies.h"

#include <regex>
#include <glib.h>
#include <glib/gstdio.h>
#include <sys/stat.h>

#define PROXIES_CONFIGURATION_PATH  "/opt/apps/common"
#define PROXIES_CONFIGURATION_FILE PROXIES_CONFIGURATION_PATH "/proxies.conf"

using namespace std;

namespace WPEFramework {

    namespace Plugin {

        SERVICE_REGISTRATION(Proxies, 1, 0);

        Proxies* Proxies::_instance = nullptr;

        Proxies::Proxies()
            : AbstractPlugin()
        {
            LOGINFO("ctor");
            Proxies::_instance = this;
            registerMethod("getProxy", &Proxies::getProxy, this);
            registerMethod("setProxy", &Proxies::setProxy, this);
        }

        Proxies::~Proxies()
        {
            LOGINFO("dtor");
            Proxies::_instance = nullptr;
        }

        //Begin methods
        uint32_t Proxies::getProxy(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            returnIfStringParamNotFound(parameters, "name");

            string name = parameters["name"].String();

            if (regex_search(name, regex("[^A-Za-z0-9_-]+")))
            {
                LOGERR("Proxy name '%s' has not allowed symbols and should have only letters, numbers, underscores and hyphens", name.c_str());
                returnResponse(false);
            }

            if (!createKeyFileIfNotExists())
            {
                returnResponse(false);
            }

            GKeyFile * proxies_conf_file = g_key_file_new();
            if(!proxies_conf_file)
            {
                LOGERR("Failed to g_key_file_new()");
                returnResponse(false);
            }

            GError * error = nullptr;
            if(!g_key_file_load_from_file(proxies_conf_file, PROXIES_CONFIGURATION_FILE, G_KEY_FILE_KEEP_COMMENTS, &error))
            {
                LOGERR("Failed to load from file '%s': %s", PROXIES_CONFIGURATION_FILE, error->message);
                returnResponse(false);
            }

            gchar * group = g_key_file_get_start_group(proxies_conf_file);

            if (group == nullptr)
            {
                LOGERR("Unable to read group 'Proxies' from file '%s'", PROXIES_CONFIGURATION_FILE);
                returnResponse(false);
            }

            if(0 != strcmp(group, "Proxies"))
            {
                LOGERR("No group 'Proxies' in file '%s'", PROXIES_CONFIGURATION_FILE);
                g_free(group);
                g_key_file_free(proxies_conf_file);
                returnResponse(false);
            }

            gchar * uri = g_key_file_get_string(proxies_conf_file, group, name.c_str(), &error);
            g_free(group);

            response["uri"] = NULL == uri ? "" : string(uri);
            g_free(uri);

            g_key_file_free(proxies_conf_file);
            returnResponse(true);
        }

        uint32_t Proxies::setProxy(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            returnIfStringParamNotFound(parameters, "name");
            returnIfStringParamNotFound(parameters, "uri");

            string name = parameters["name"].String();

            if (name.length() == 0)
            {
                LOGERR("Proxy name should not be empty");
                returnResponse(false);
            }

            if (regex_search(name, regex("[^A-Za-z0-9_-]+")))
            {
                LOGERR("Proxy name '%s' has not allowed symbols and should have only letters, numbers, underscores and hyphens", name.c_str());
                returnResponse(false);
            }

            string uri = parameters["uri"].String();

            int uriLength = uri.length();
            if (uriLength > 255)
            {
                LOGERR("Length of proxy uri '%s' is %d, but should be less than 256", uri.c_str(), uri.length());
                returnResponse(false);
            }

            if (!createKeyFileIfNotExists())
            {
                returnResponse(false);
            }

            GKeyFile * proxies_conf_file = g_key_file_new();
            if(!proxies_conf_file)
            {
                LOGERR("Failed to g_key_file_new()");
                returnResponse(false);
            }

            GError * error = nullptr;
            if(!g_key_file_load_from_file(proxies_conf_file, PROXIES_CONFIGURATION_FILE, G_KEY_FILE_KEEP_COMMENTS, &error))
            {
                LOGERR("Failed to load from file '%s': %s", PROXIES_CONFIGURATION_FILE, error->message);
                returnResponse(false);
            }

            gchar * group = g_key_file_get_start_group(proxies_conf_file);
            if(0 != strcmp(group, "Proxies"))
            {
                LOGERR("No group 'Proxies' in file '%s'", PROXIES_CONFIGURATION_FILE);
                g_free(group);
                g_key_file_free(proxies_conf_file);
                returnResponse(false);
            }

            if (0 == uriLength)
            {
                g_key_file_remove_key(proxies_conf_file, group, (gchar *)name.c_str(), &error);

                gsize length;
                gchar * str = g_key_file_to_data(proxies_conf_file, &length, &error);
                g_file_set_contents(PROXIES_CONFIGURATION_FILE, str, length, &error);
                g_free(str);
                g_free(group);
                g_key_file_free(proxies_conf_file);

                LOGWARN("Proxy with name '%s' has been removed from '%s' file, if it was there", name.c_str(), PROXIES_CONFIGURATION_FILE);
                returnResponse(true);
            }

            g_key_file_set_string(proxies_conf_file, group, (gchar *)name.c_str(), (gchar *)uri.c_str());

            gsize length;
            gchar * str = g_key_file_to_data(proxies_conf_file, &length, &error);
            g_file_set_contents(PROXIES_CONFIGURATION_FILE, str, length, &error);
            g_free(str);
            g_free(group);
            g_key_file_free(proxies_conf_file);

            returnResponse(true);
        }
        //End methods

        //Begin events
        //End events

        bool Proxies::createKeyFileIfNotExists()
        {
            if (0 != access(PROXIES_CONFIGURATION_FILE, F_OK))
            {
                g_mkdir_with_parents(PROXIES_CONFIGURATION_PATH, 0755);
                FILE * confFile = fopen(PROXIES_CONFIGURATION_FILE, "w");
                if (confFile == NULL)
                {
                    LOGERR("Can't create empty file '%s'", PROXIES_CONFIGURATION_FILE);
                    return false;
                }

                fprintf(confFile, "[Proxies]");
                fclose(confFile);
            }

            return true;
        }

    } // namespace Plugin
} // namespace WPEFramework
