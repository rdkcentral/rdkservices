/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2024 RDK Management
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

#include "UserSettingsImplementation.h"
#include <sys/prctl.h>
#include "UtilsJsonRpc.h"
#include <mutex>
#include "tracing/Logging.h"

#ifdef HAS_RBUS
#define RBUS_COMPONENT_NAME "UserSettingsThunderPlugin"
#define RBUS_PRIVACY_MODE_EVENT_NAME "Device.X_RDKCENTRAL-COM_UserSettings.PrivacyModeChanged"
#endif

namespace WPEFramework {
namespace Plugin {

const std::map<string, string> UserSettingsImplementation::usersettingsDefaultMap = {{USERSETTINGS_AUDIO_DESCRIPTION_KEY, "false"},
                                                                 {USERSETTINGS_PREFERRED_AUDIO_LANGUAGES_KEY, ""},
                                                                 {USERSETTINGS_PRESENTATION_LANGUAGE_KEY, ""},
                                                                 {USERSETTINGS_CAPTIONS_KEY, "false"},
                                                                 {USERSETTINGS_PREFERRED_CAPTIONS_LANGUAGES_KEY, ""},
                                                                 {USERSETTINGS_PREFERRED_CLOSED_CAPTIONS_SERVICE_KEY, "AUTO"},
                                                                 {USERSETTINGS_PIN_CONTROL_KEY, "false"},
                                                                 {USERSETTINGS_VIEWING_RESTRICTIONS_KEY, ""},
                                                                 {USERSETTINGS_VIEWING_RESTRICTIONS_WINDOW_KEY, ""},
                                                                 {USERSETTINGS_LIVE_WATERSHED_KEY, "false"},
                                                                 {USERSETTINGS_PLAYBACK_WATERSHED_KEY, "false"},
                                                                 {USERSETTINGS_BLOCK_NOT_RATED_CONTENT_KEY, "false"},
                                                                 {USERSETTINGS_PIN_ON_PURCHASE_KEY, "false"},
                                                                 {USERSETTINGS_HIGH_CONTRAST_KEY, "false"},
                                                                 {USERSETTINGS_VOICE_GUIDANCE_KEY, "false"},
                                                                 {USERSETTINGS_VOICE_GUIDANCE_RATE_KEY, "1"},
                                                                 {USERSETTINGS_VOICE_GUIDANCE_HINTS_KEY, "false"}};

SERVICE_REGISTRATION(UserSettingsImplementation, 1, 0);

UserSettingsImplementation::UserSettingsImplementation()
: _adminLock()
, _remotStoreObject(nullptr)
, _storeNotification(*this)
, _registeredEventHandlers(false)
, _service(nullptr)
#ifdef HAS_RBUS
, _rbusHandleStatus(RBUS_ERROR_NOT_INITIALIZED)
#endif
{
    LOGINFO("Create UserSettingsImplementation Instance");

    UserSettingsImplementation::instance(this);
}

uint32_t UserSettingsImplementation::Configure(PluginHost::IShell* service)
{
    uint32_t result = Core::ERROR_GENERAL;

    if (service != nullptr )
    {
        _service = service;
        _service->AddRef();
        result = Core::ERROR_NONE;

        _remotStoreObject = _service->QueryInterfaceByCallsign<WPEFramework::Exchange::IStore2>("org.rdk.PersistentStore");
        if (_remotStoreObject != nullptr)
        {
            registerEventHandlers();
        }
        else
        {
            LOGERR("_remotStoreObject is null \n");
        }
    }
    else
    {
        LOGERR("service is null \n");
    }

    return result;
}

UserSettingsImplementation* UserSettingsImplementation::instance(UserSettingsImplementation *UserSettingsImpl)
{
   static UserSettingsImplementation *UserSettingsImpl_instance = nullptr;


   if (UserSettingsImpl != nullptr)
   {
      UserSettingsImpl_instance = UserSettingsImpl;
   }
   else
   {
      LOGERR("UserSettingsImpl is null \n");
   }
   return(UserSettingsImpl_instance);
}

UserSettingsImplementation::~UserSettingsImplementation()
{


    if(_remotStoreObject)
    {
        _remotStoreObject->Release();
    }
    if (_service != nullptr)
    {
       _service->Release();
       _service = nullptr;
    }
    _registeredEventHandlers = false;
    
#ifdef HAS_RBUS
    if (RBUS_ERROR_SUCCESS == _rbusHandleStatus)
    {
        rbus_close(_rbusHandle);
        _rbusHandleStatus = RBUS_ERROR_NOT_INITIALIZED;
    }

#endif
}

void UserSettingsImplementation::registerEventHandlers()
{

    if(!_registeredEventHandlers && _remotStoreObject)
    {
        _registeredEventHandlers = true;
        _remotStoreObject->Register(&_storeNotification);
    }
    else
    {
        LOGERR("_remotStoreObject is null or _registeredEventHandlers is true");
    }
}

/**
 * Register a notification callback
 */
uint32_t UserSettingsImplementation::Register(Exchange::IUserSettings::INotification *notification)
{
    _adminLock.Lock();

    // Make sure we can't register the same notification callback multiple times
    if (std::find(_userSettingNotification.begin(), _userSettingNotification.end(), notification) == _userSettingNotification.end())
    {
        LOGINFO("Register notification");
        _userSettingNotification.push_back(notification);
        notification->AddRef();
    }
    else
    {
        LOGERR("notification is already available in the _userSettingNotification");
    }

    _adminLock.Unlock();

    return Core::ERROR_NONE;
}

/**
 * Unregister a notification callback
 */
uint32_t UserSettingsImplementation::Unregister(Exchange::IUserSettings::INotification *notification )
{
    uint32_t status = Core::ERROR_GENERAL;

    _adminLock.Lock();

    // Make sure we can't unregister the same notification callback multiple times
    auto itr = std::find(_userSettingNotification.begin(), _userSettingNotification.end(), notification);
    if (itr != _userSettingNotification.end())
    {
        (*itr)->Release();
        LOGINFO("Unregister notification");
        _userSettingNotification.erase(itr);
        status = Core::ERROR_NONE;
    }
    else
    {
        LOGERR("notification not found");
    }

    _adminLock.Unlock();

    return status;
}

void UserSettingsImplementation::dispatchEvent(Event event, const JsonValue &params)
{
    Core::IWorkerPool::Instance().Submit(Job::Create(this, event, params));
}

void UserSettingsImplementation::Dispatch(Event event, const JsonValue params)
{
     _adminLock.Lock();

     std::list<Exchange::IUserSettings::INotification*>::const_iterator index(_userSettingNotification.begin());

     switch(event) {
         case AUDIO_DESCRIPTION_CHANGED:
             while (index != _userSettingNotification.end())
             {
                 (*index)->OnAudioDescriptionChanged(params.Boolean());
                 ++index;
             }
         break;

         case PREFERRED_AUDIO_CHANGED:
             while (index != _userSettingNotification.end())
             {
                 (*index)->OnPreferredAudioLanguagesChanged(params.String());
                 ++index;
             }
         break;

         case PRESENTATION_LANGUAGE_CHANGED:
             while (index != _userSettingNotification.end())
             {
                 (*index)->OnPresentationLanguageChanged(params.String());
                 ++index;
             }
         break;

         case CAPTIONS_CHANGED:
             while (index != _userSettingNotification.end())
             {
                 (*index)->OnCaptionsChanged(params.Boolean());
                 ++index;
             }
         break;

         case PREFERRED_CAPTIONS_LANGUAGE_CHANGED:
             while (index != _userSettingNotification.end())
             {
                 (*index)->OnPreferredCaptionsLanguagesChanged(params.String());
                 ++index;
             }
         break;

         case PREFERRED_CLOSED_CAPTIONS_SERVICE_CHANGED:
             while (index != _userSettingNotification.end())
             {
                 (*index)->OnPreferredClosedCaptionServiceChanged(params.String());
                 ++index;
             }
         break;

         case PRIVACY_MODE_CHANGED:
             while (index != _userSettingNotification.end())
             {
                 (*index)->OnPrivacyModeChanged(params.String());
                 ++index;
             }
         break;

         case PIN_CONTROL_CHANGED:
              while (index != _userSettingNotification.end())
              {
                  (*index)->OnPinControlChanged(params.Boolean());
                  ++index;
              }
         break;

         case VIEWING_RESTRICTIONS_CHANGED:
              while (index != _userSettingNotification.end())
              {
                  (*index)->OnViewingRestrictionsChanged(params.String());
                  ++index;
              }
         break;

         case VIEWING_RESTRICTIONS_WINDOW_CHANGED:
              while (index != _userSettingNotification.end())
              {
                  (*index)->OnViewingRestrictionsWindowChanged(params.String());
                  ++index;
              }
         break;

         case LIVE_WATERSHED_CHANGED:
              while (index != _userSettingNotification.end())
              {
                  (*index)->OnLiveWatershedChanged(params.Boolean());
                  ++index;
              }
         break;

         case PLAYBACK_WATERSHED_CHANGED:
              while (index != _userSettingNotification.end())
              {
                  (*index)->OnPlaybackWatershedChanged(params.Boolean());
                  ++index;
              }
         break;

         case BLOCK_NOT_RATED_CONTENT_CHANGED:
              while (index != _userSettingNotification.end())
              {
                  (*index)->OnBlockNotRatedContentChanged(params.Boolean());
                  ++index;
              }
         break;

         case PIN_ON_PURCHASE_CHANGED:
              while (index != _userSettingNotification.end())
              {
                  (*index)->OnPinOnPurchaseChanged(params.Boolean());
                  ++index;
              }
         break;

         case HIGH_CONTRAST_CHANGED:
              while (index != _userSettingNotification.end())
              {
                  (*index)->OnHighContrastChanged(params.Boolean());
                  ++index;
              }
         break;

         case VOICE_GUIDANCE_CHANGED:
              while (index != _userSettingNotification.end())
              {
                  (*index)->OnVoiceGuidanceChanged(params.Boolean());
                  ++index;
              }
         break;

         case VOICE_GUIDANCE_RATE_CHANGED:
              while (index != _userSettingNotification.end())
              {
                  (*index)->OnVoiceGuidanceRateChanged(params.Double());
                  ++index;
              }
         break;

         case VOICE_GUIDANCE_HINTS_CHANGED:
              while (index != _userSettingNotification.end())
              {
                  (*index)->OnVoiceGuidanceHintsChanged(params.Boolean());
                  ++index;
              }
         break;

         default:
           break;
     }

     _adminLock.Unlock();
}

void UserSettingsImplementation::ValueChanged(const Exchange::IStore2::ScopeType scope, const string& ns, const string& key, const string& value)
{
    LOGINFO("ns:%s key:%s value:%s", ns.c_str(), key.c_str(), value.c_str());

    if(0 == (ns.compare(USERSETTINGS_NAMESPACE)) && (0 == key.compare(USERSETTINGS_AUDIO_DESCRIPTION_KEY)))
    {
        dispatchEvent(AUDIO_DESCRIPTION_CHANGED, JsonValue((bool)(value.compare("true")==0)?true:false));
    }
    else if(0 == (ns.compare(USERSETTINGS_NAMESPACE)) && (0 == key.compare(USERSETTINGS_PREFERRED_AUDIO_LANGUAGES_KEY)))
    {
        dispatchEvent(PREFERRED_AUDIO_CHANGED, JsonValue((string)value));
    }
    else if(0 == (ns.compare(USERSETTINGS_NAMESPACE)) && (0 == key.compare(USERSETTINGS_PRESENTATION_LANGUAGE_KEY)))
    {
        dispatchEvent(PRESENTATION_LANGUAGE_CHANGED, JsonValue((string)value));
    }
    else if(0 == (ns.compare(USERSETTINGS_NAMESPACE)) && (0 == key.compare(USERSETTINGS_CAPTIONS_KEY)))
    {
        dispatchEvent(CAPTIONS_CHANGED, JsonValue((bool)(value.compare("true")==0)?true:false));
    }
    else if(0 == (ns.compare(USERSETTINGS_NAMESPACE)) && (0 == key.compare(USERSETTINGS_PREFERRED_CAPTIONS_LANGUAGES_KEY)))
    {
        dispatchEvent(PREFERRED_CAPTIONS_LANGUAGE_CHANGED, JsonValue((string)value));
    }
    else if(0 == (ns.compare(USERSETTINGS_NAMESPACE)) && (0 == key.compare(USERSETTINGS_PREFERRED_CLOSED_CAPTIONS_SERVICE_KEY)))
    {
        dispatchEvent(PREFERRED_CLOSED_CAPTIONS_SERVICE_CHANGED, JsonValue((string)value));
    }
    else if((ns.compare(USERSETTINGS_NAMESPACE) == 0) && (key.compare(USERSETTINGS_PRIVACY_MODE_KEY) == 0))
    {
        dispatchEvent(PRIVACY_MODE_CHANGED, JsonValue((string)value));
    }
    else if(0 == (ns.compare(USERSETTINGS_NAMESPACE)) && (0 == key.compare(USERSETTINGS_PIN_CONTROL_KEY)))
    {
        dispatchEvent(PIN_CONTROL_CHANGED, JsonValue((bool)(value.compare("true")==0)?true:false));
    }
    else if(0 == (ns.compare(USERSETTINGS_NAMESPACE)) && (0 == key.compare(USERSETTINGS_VIEWING_RESTRICTIONS_KEY)))
    {
        dispatchEvent(VIEWING_RESTRICTIONS_CHANGED, JsonValue((string)value));
    }
    else if(0 == (ns.compare(USERSETTINGS_NAMESPACE)) && (0 == key.compare(USERSETTINGS_VIEWING_RESTRICTIONS_WINDOW_KEY)))
    {
        dispatchEvent(VIEWING_RESTRICTIONS_WINDOW_CHANGED, JsonValue((string)value));
    }
    else if(0 == (ns.compare(USERSETTINGS_NAMESPACE)) && (0 == key.compare(USERSETTINGS_LIVE_WATERSHED_KEY)))
    {
        dispatchEvent(LIVE_WATERSHED_CHANGED, JsonValue((bool)(value.compare("true")==0)?true:false));
    }
    else if(0 == (ns.compare(USERSETTINGS_NAMESPACE)) && (0 == key.compare(USERSETTINGS_PLAYBACK_WATERSHED_KEY)))
    {
        dispatchEvent(PLAYBACK_WATERSHED_CHANGED, JsonValue((bool)(value.compare("true")==0)?true:false));
    }
    else if(0 == (ns.compare(USERSETTINGS_NAMESPACE)) && (0 == key.compare(USERSETTINGS_BLOCK_NOT_RATED_CONTENT_KEY)))
    {
        dispatchEvent(BLOCK_NOT_RATED_CONTENT_CHANGED, JsonValue((bool)(value.compare("true")==0)?true:false));
    }
    else if(0 == (ns.compare(USERSETTINGS_NAMESPACE)) && (0 == key.compare(USERSETTINGS_PIN_ON_PURCHASE_KEY)))
    {
        dispatchEvent(PIN_ON_PURCHASE_CHANGED, JsonValue((bool)(value.compare("true")==0)?true:false));
    }
    else if(0 == (ns.compare(USERSETTINGS_NAMESPACE)) && (0 == key.compare(USERSETTINGS_HIGH_CONTRAST_KEY)))
    {
        dispatchEvent(HIGH_CONTRAST_CHANGED, JsonValue((bool)(value.compare("true")==0)?true:false));
    }
    else if(0 == (ns.compare(USERSETTINGS_NAMESPACE)) && (0 == key.compare(USERSETTINGS_VOICE_GUIDANCE_KEY)))
    {
        dispatchEvent(VOICE_GUIDANCE_CHANGED, JsonValue((bool)(value.compare("true")==0)?true:false));
    }
    else if(0 == (ns.compare(USERSETTINGS_NAMESPACE)) && (0 == key.compare(USERSETTINGS_VOICE_GUIDANCE_RATE_KEY)))
    {
        dispatchEvent(VOICE_GUIDANCE_RATE_CHANGED, JsonValue((double)(std::stod(value))));
    }
    else if(0 == (ns.compare(USERSETTINGS_NAMESPACE)) && (0 == key.compare(USERSETTINGS_VOICE_GUIDANCE_HINTS_KEY)))
    {
        dispatchEvent(VOICE_GUIDANCE_HINTS_CHANGED, JsonValue((bool)(value.compare("true")==0)?true:false));
    }
    else
    {
        LOGERR("Not supported");
    }
}

uint32_t UserSettingsImplementation::SetUserSettingsValue(const string& key, const string& value)
{
    uint32_t status = Core::ERROR_GENERAL;
    _adminLock.Lock();

    LOGINFO("Key[%s] value[%s]", key.c_str(), value.c_str());
    if (nullptr != _remotStoreObject)
    {
        status = _remotStoreObject->SetValue(Exchange::IStore2::ScopeType::DEVICE, USERSETTINGS_NAMESPACE, key, value, 0);
    }
    else
    {
        LOGERR("_remotStoreObject is null");
    }

    _adminLock.Unlock();
    return status;
}

uint32_t UserSettingsImplementation::GetUserSettingsValue(const string& key, string &value) const
{
    uint32_t status = Core::ERROR_GENERAL;
    uint32_t ttl = 0;
    _adminLock.Lock();

    LOGINFO("Key[%s]", key.c_str());
    if (nullptr != _remotStoreObject)
    {
        status = _remotStoreObject->GetValue(Exchange::IStore2::ScopeType::DEVICE, USERSETTINGS_NAMESPACE, key, value, ttl);

        LOGINFO("Key[%s] value[%s] status[%d]", key.c_str(), value.c_str(), status);
        if(Core::ERROR_UNKNOWN_KEY == status || Core::ERROR_NOT_EXIST == status)
        {
            if(usersettingsDefaultMap.find(key)!=usersettingsDefaultMap.end())
            {
                value = usersettingsDefaultMap.find(key)->second;
                status = Core::ERROR_NONE;
            }
            else
            {
                LOGERR("Default value is not found in usersettingsDefaultMap for '%s' Key", key.c_str());
            }
        }
    }
    else
    {
        LOGERR("_remotStoreObject is null");
    }
    _adminLock.Unlock();

    return status;
}

uint32_t UserSettingsImplementation::SetAudioDescription(const bool enabled)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("enabled: %d", enabled);
    status = SetUserSettingsValue(USERSETTINGS_AUDIO_DESCRIPTION_KEY, (enabled)?"true":"false");
    return status;
}

uint32_t UserSettingsImplementation::GetAudioDescription(bool &enabled) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_AUDIO_DESCRIPTION_KEY, value);
    if(Core::ERROR_NONE == status)
    {
        if (0 == value.compare("true"))
        {
            enabled = true;
        }
        else
        {
            enabled = false;
        }
    }

    return status;
}

uint32_t UserSettingsImplementation::SetPreferredAudioLanguages(const string& preferredLanguages)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("preferredLanguages: %s", preferredLanguages.c_str());
    status = SetUserSettingsValue(USERSETTINGS_PREFERRED_AUDIO_LANGUAGES_KEY, preferredLanguages);
    return status;
}

uint32_t UserSettingsImplementation::GetPreferredAudioLanguages(string &preferredLanguages) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_PREFERRED_AUDIO_LANGUAGES_KEY, preferredLanguages);
    return status;
}

uint32_t UserSettingsImplementation::SetPresentationLanguage(const string& presentationLanguage)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("presentationLanguage: %s", presentationLanguage.c_str());
    status = SetUserSettingsValue(USERSETTINGS_PRESENTATION_LANGUAGE_KEY, presentationLanguage);
    return status;
}

uint32_t UserSettingsImplementation::GetPresentationLanguage(string &presentationLanguage) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_PRESENTATION_LANGUAGE_KEY, presentationLanguage);
    return status;
}

uint32_t UserSettingsImplementation::SetCaptions(const bool enabled)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("enabled: %d", enabled);
    status = SetUserSettingsValue(USERSETTINGS_CAPTIONS_KEY, (enabled)?"true":"false");
    return status;
}

uint32_t UserSettingsImplementation::GetCaptions(bool &enabled) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_CAPTIONS_KEY, value);

    if(Core::ERROR_NONE == status)
    {
        if (0 == value.compare("true"))
        {
            enabled = true;
        }
        else
        {
            enabled = false;
        }
    }
    return status;
}

uint32_t UserSettingsImplementation::SetPreferredCaptionsLanguages(const string& preferredLanguages)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("preferredLanguages: %s", preferredLanguages.c_str());
    status = SetUserSettingsValue(USERSETTINGS_PREFERRED_CAPTIONS_LANGUAGES_KEY, preferredLanguages);
    return status;
}

uint32_t UserSettingsImplementation::GetPreferredCaptionsLanguages(string &preferredLanguages) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_PREFERRED_CAPTIONS_LANGUAGES_KEY, preferredLanguages);
    return status;
}

uint32_t UserSettingsImplementation::SetPreferredClosedCaptionService(const string& service)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("service: %s", service.c_str());
    status = SetUserSettingsValue(USERSETTINGS_PREFERRED_CLOSED_CAPTIONS_SERVICE_KEY, service);
    return status;
}

uint32_t UserSettingsImplementation::GetPreferredClosedCaptionService(string &service) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_PREFERRED_CLOSED_CAPTIONS_SERVICE_KEY, service);
    return status;
}

uint32_t UserSettingsImplementation::SetPrivacyMode(const string& privacyMode)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("privacyMode: %s", privacyMode.c_str());

    if (privacyMode != "SHARE" && privacyMode != "DO_NOT_SHARE")
    {
        LOGERR("Wrong privacyMode value: '%s', returning default", privacyMode.c_str());
        return status;
    }

    _adminLock.Lock();

    ASSERT (nullptr != _remotStoreObject);

    if (nullptr != _remotStoreObject)
    {
        uint32_t ttl = 0;
        string oldPrivacyMode;
        status = _remotStoreObject->GetValue(Exchange::IStore2::ScopeType::DEVICE, USERSETTINGS_NAMESPACE, USERSETTINGS_PRIVACY_MODE_KEY, oldPrivacyMode, ttl);
        LOGINFO("oldPrivacyMode: %s", oldPrivacyMode.c_str());

        if (privacyMode != oldPrivacyMode)
        {
#ifdef HAS_RBUS
            if (Core::ERROR_NONE == status)
            {
                if (RBUS_ERROR_SUCCESS != _rbusHandleStatus)
                {
                    _rbusHandleStatus = rbus_open(&_rbusHandle, RBUS_COMPONENT_NAME);
                }

                if (RBUS_ERROR_SUCCESS == _rbusHandleStatus)
                {
                    rbusValue_t value;
                    rbusSetOptions_t opts = {true, 0};

                    rbusValue_Init(&value);
                    rbusValue_SetString(value, privacyMode.c_str());
                    int rc = rbus_set(_rbusHandle, RBUS_PRIVACY_MODE_EVENT_NAME, value, &opts);
                    if (rc != RBUS_ERROR_SUCCESS)
                    {
                        std::stringstream str;
                        str << "Failed to set property " << RBUS_PRIVACY_MODE_EVENT_NAME << ": " << rc;
                        LOGERR("%s", str.str().c_str());
                    }
                    rbusValue_Release(value);
                }
                else
                {
                    std::stringstream str;
                    str << "rbus_open failed with error code " << _rbusHandleStatus;
                    LOGERR("%s", str.str().c_str());
                }
            }
#endif
            status = _remotStoreObject->SetValue(Exchange::IStore2::ScopeType::DEVICE, USERSETTINGS_NAMESPACE, USERSETTINGS_PRIVACY_MODE_KEY, privacyMode, 0);
        }
    }

    _adminLock.Unlock();

    return status;
}

uint32_t UserSettingsImplementation::GetPrivacyMode(string &privacyMode) const
{
    uint32_t status = Core::ERROR_NONE;
    std::string value = "";
    uint32_t ttl = 0;
    privacyMode = "";

    _adminLock.Lock();

    ASSERT (nullptr != _remotStoreObject);

    if (nullptr != _remotStoreObject)
    {
        _remotStoreObject->GetValue(Exchange::IStore2::ScopeType::DEVICE, USERSETTINGS_NAMESPACE, USERSETTINGS_PRIVACY_MODE_KEY, privacyMode, ttl);
    }

    _adminLock.Unlock();
    
    if (privacyMode != "SHARE" && privacyMode != "DO_NOT_SHARE") 
    {
        LOGWARN("Wrong privacyMode value: '%s', returning default", privacyMode.c_str());
        privacyMode = "SHARE";
    }

    return status;
}

uint32_t UserSettingsImplementation::SetPinControl(const bool pinControl)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("pinControl: %d", pinControl);
    status = SetUserSettingsValue(USERSETTINGS_PIN_CONTROL_KEY, (pinControl)?"true":"false");
    return status;
}

uint32_t UserSettingsImplementation::GetPinControl(bool &pinControl) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_PIN_CONTROL_KEY, value);

    if(Core::ERROR_NONE == status)
    {
        if (0 == value.compare("true"))
        {
            pinControl = true;
        }
        else
        {
            pinControl = false;
        }
    }
    return status;

}

uint32_t UserSettingsImplementation::SetViewingRestrictions(const string& viewingRestrictions)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("viewingRestrictions: %s", viewingRestrictions.c_str());
    status = SetUserSettingsValue(USERSETTINGS_VIEWING_RESTRICTIONS_KEY, viewingRestrictions);
    return status;

}

uint32_t UserSettingsImplementation::GetViewingRestrictions(string &viewingRestrictions) const
{
    uint32_t status = Core::ERROR_GENERAL;

    status = GetUserSettingsValue(USERSETTINGS_VIEWING_RESTRICTIONS_KEY, viewingRestrictions);
    return status;

}

uint32_t UserSettingsImplementation::SetViewingRestrictionsWindow(const string& viewingRestrictionsWindow)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("viewingRestrictionsWindow: %s", viewingRestrictionsWindow.c_str());
    status = SetUserSettingsValue(USERSETTINGS_VIEWING_RESTRICTIONS_WINDOW_KEY, viewingRestrictionsWindow);
    return status;

}

uint32_t UserSettingsImplementation::GetViewingRestrictionsWindow(string &viewingRestrictionsWindow) const
{
    uint32_t status = Core::ERROR_GENERAL;

    status = GetUserSettingsValue(USERSETTINGS_VIEWING_RESTRICTIONS_WINDOW_KEY, viewingRestrictionsWindow);
    return status;
}

uint32_t UserSettingsImplementation::SetLiveWatershed(const bool liveWatershed)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("liveWatershed: %d", liveWatershed);
    status = SetUserSettingsValue(USERSETTINGS_LIVE_WATERSHED_KEY, (liveWatershed)?"true":"false");
    return status;

}

uint32_t UserSettingsImplementation::GetLiveWatershed(bool &liveWatershed) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_LIVE_WATERSHED_KEY, value);

    if(Core::ERROR_NONE == status)
    {
        if (0 == value.compare("true"))
        {
            liveWatershed = true;
        }
        else
        {
            liveWatershed = false;
        }
    }
    return status;
}

uint32_t UserSettingsImplementation::SetPlaybackWatershed(const bool playbackWatershed)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("playbackWatershed: %d", playbackWatershed);
    status = SetUserSettingsValue(USERSETTINGS_PLAYBACK_WATERSHED_KEY, (playbackWatershed)?"true":"false");
    return status;
}

uint32_t UserSettingsImplementation::GetPlaybackWatershed(bool &playbackWatershed) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_PLAYBACK_WATERSHED_KEY, value);

    if(Core::ERROR_NONE == status)
    {
        if (0 == value.compare("true"))
        {
            playbackWatershed = true;
        }
        else
        {
            playbackWatershed = false;
        }
    }
    return status;
}

uint32_t UserSettingsImplementation::SetBlockNotRatedContent(const bool blockNotRatedContent)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("blockNotRatedContent: %d", blockNotRatedContent);
    status = SetUserSettingsValue(USERSETTINGS_BLOCK_NOT_RATED_CONTENT_KEY, (blockNotRatedContent)?"true":"false");
    return status;
}

uint32_t UserSettingsImplementation::GetBlockNotRatedContent(bool &blockNotRatedContent) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_BLOCK_NOT_RATED_CONTENT_KEY, value);

    if(Core::ERROR_NONE == status)
    {
        if (0 == value.compare("true"))
        {
            blockNotRatedContent = true;
        }
        else
        {
            blockNotRatedContent = false;
        }
    }
    return status;
}

uint32_t UserSettingsImplementation::SetPinOnPurchase(const bool pinOnPurchase)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("pinOnPurchase: %d", pinOnPurchase);
    status = SetUserSettingsValue(USERSETTINGS_PIN_ON_PURCHASE_KEY, (pinOnPurchase)?"true":"false");
    return status;
}

uint32_t UserSettingsImplementation::GetPinOnPurchase(bool &pinOnPurchase) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_PIN_ON_PURCHASE_KEY, value);

    if(Core::ERROR_NONE == status)
    {
        if (0 == value.compare("true"))
        {
            pinOnPurchase = true;
        }
        else
        {
            pinOnPurchase = false;
        }
    }
    return status;
}

uint32_t UserSettingsImplementation::SetHighContrast(const bool enabled)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("enabled: %d", enabled);
    status = SetUserSettingsValue(USERSETTINGS_HIGH_CONTRAST_KEY, (enabled)?"true":"false");
    return status;
}

uint32_t UserSettingsImplementation::GetHighContrast(bool &enabled) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_HIGH_CONTRAST_KEY, value);

    if(Core::ERROR_NONE == status)
    {
        if (0 == value.compare("true"))
        {
            enabled = true;
        }
        else
        {
            enabled = false;
        }
    }
    return status;
}

uint32_t UserSettingsImplementation::SetVoiceGuidance(const bool enabled)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("enabled: %d", enabled);
    status = SetUserSettingsValue(USERSETTINGS_VOICE_GUIDANCE_KEY, (enabled)?"true":"false");
    return status;
}

uint32_t UserSettingsImplementation::GetVoiceGuidance(bool &enabled) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_VOICE_GUIDANCE_KEY, value);

    if(Core::ERROR_NONE == status)
    {
        if (0 == value.compare("true"))
        {
            enabled = true;
        }
        else
        {
            enabled = false;
        }
    }
    return status;
}

uint32_t UserSettingsImplementation::SetVoiceGuidanceRate(const double rate)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("rate: %lf", rate);
    status = SetUserSettingsValue(USERSETTINGS_VOICE_GUIDANCE_RATE_KEY, std::to_string(rate));
    return status;
}

uint32_t UserSettingsImplementation::GetVoiceGuidanceRate(double &rate) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_VOICE_GUIDANCE_RATE_KEY, value);
    if(Core::ERROR_NONE == status && !(value.empty()))
    {
        rate = std::stod(value);
    }
    return status;
}

uint32_t UserSettingsImplementation::SetVoiceGuidanceHints(const bool hints)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("hints: %d", hints);
    status = SetUserSettingsValue(USERSETTINGS_VOICE_GUIDANCE_HINTS_KEY, (hints)?"true":"false");
    return status;
}

uint32_t UserSettingsImplementation::GetVoiceGuidanceHints(bool &hints) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_VOICE_GUIDANCE_HINTS_KEY, value);
    if(Core::ERROR_NONE == status)
    {
        if (0 == value.compare("true"))
        {
            hints = true;
        }
        else
        {
            hints = false;
        }
    }
    return status;
}
} // namespace Plugin
} // namespace WPEFramework
