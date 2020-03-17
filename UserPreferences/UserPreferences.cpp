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
#include "utils.h"

using namespace std;

namespace WPEFramework {

    namespace Plugin {

        SERVICE_REGISTRATION(UserPreferences, 1, 0);

        UserPreferences* UserPreferences::_instance = nullptr;

        UserPreferences::UserPreferences()
            : AbstractPlugin()
            , _language("")
        {
            LOGINFO("ctor");
            UserPreferences::_instance = this;
            registerMethod("getUILanguage", &UserPreferences::getUILanguage, this);
            registerMethod("setUILanguage", &UserPreferences::setUILanguage, this);
        }

        UserPreferences::~UserPreferences()
        {
            LOGINFO("dtor");
            UserPreferences::_instance = nullptr;
        }

        //Begin methods
        uint32_t UserPreferences::getUILanguage(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            response["language"] = _language;
            returnResponse(true);
        }

        uint32_t UserPreferences::setUILanguage(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            returnIfStringParamNotFound(parameters, "language");
            _language = parameters["language"].String();
            returnResponse(true);
        }
        //End methods

        //Begin events
        //End events

    } // namespace Plugin
} // namespace WPEFramework
