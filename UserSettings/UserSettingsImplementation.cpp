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
                                                                 {USERSETTINGS_PIN_ON_PURCHASE_KEY, "false"}};

SERVICE_REGISTRATION(UserSettingsImplementation, 1, 0);

UserSettingsImplementation::UserSettingsImplementation()
: _adminLock()
, _engine(Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create())
, _communicatorClient(Core::ProxyType<RPC::CommunicatorClient>::Create(Core::NodeId("/tmp/communicator"), Core::ProxyType<Core::IIPCServer>(_engine)))
, _controller(nullptr)
, _remotStoreObject(nullptr)
, _storeNotification(*this)
, _registeredEventHandlers(false)
#ifdef HAS_RBUS
, _rbusHandleStatus(RBUS_ERROR_NOT_INITIALIZED)
#endif
{
    LOGINFO("Create UserSettingsImplementation Instance");

    UserSettingsImplementation::instance(this);

     if (!_communicatorClient.IsValid())
     {
         LOGWARN("Invalid _communicatorClient\n");
    }
    else
    {

#if ((THUNDER_VERSION == 2) || ((THUNDER_VERSION == 4) && (THUNDER_VERSION_MINOR == 2)))
        _engine->Announcements(_communicatorClient->Announcement());
#endif

        LOGINFO("Connect the COM-RPC socket\n");
        _controller = _communicatorClient->Open<PluginHost::IShell>(_T("org.rdk.PersistentStore"), ~0, 3000);

        if (_controller)
        {
             _remotStoreObject = _controller->QueryInterface<Exchange::IStore2>();

             if(_remotStoreObject)
             {
                 _remotStoreObject->AddRef();
             }
        }
        else
        {
            LOGERR("Failed to create PersistentStore Controller\n");
        }

        registerEventHandlers();
    }
}

UserSettingsImplementation* UserSettingsImplementation::instance(UserSettingsImplementation *UserSettingsImpl)
{
   static UserSettingsImplementation *UserSettingsImpl_instance = nullptr;

   ASSERT ((nullptr == UserSettingsImpl_instance) || (nullptr == UserSettingsImpl));

   if (UserSettingsImpl != nullptr)
   {
      UserSettingsImpl_instance = UserSettingsImpl;
   }

   return(UserSettingsImpl_instance);
}

UserSettingsImplementation::~UserSettingsImplementation()
{
    if (_controller)
    {
        _controller->Release();
        _controller = nullptr;
    }

    LOGINFO("Disconnect from the COM-RPC socket\n");
    // Disconnect from the COM-RPC socket
    _communicatorClient->Close(RPC::CommunicationTimeOut);
    if (_communicatorClient.IsValid())
    {
        _communicatorClient.Release();
    }

    if(_engine.IsValid())
    {
        _engine.Release();
    }

    if(_remotStoreObject)
    {
        _remotStoreObject->Release();
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
    ASSERT (nullptr != _remotStoreObject);

    if(!_registeredEventHandlers && _remotStoreObject) {
        _registeredEventHandlers = true;
        _remotStoreObject->Register(&_storeNotification);
    }
}

/**
 * Register a notification callback
 */
uint32_t UserSettingsImplementation::Register(Exchange::IUserSettings::INotification *notification)
{
    ASSERT (nullptr != notification);

    _adminLock.Lock();

    // Make sure we can't register the same notification callback multiple times
    if (std::find(_userSettingNotification.begin(), _userSettingNotification.end(), notification) == _userSettingNotification.end())
    {
        LOGINFO("Register notification");
        _userSettingNotification.push_back(notification);
        notification->AddRef();
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

    ASSERT (nullptr != notification);

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
                 (*index)->onAudioDescriptionChanged(params.Boolean());
                 ++index;
             }
         break;

         case PREFERRED_AUDIO_CHANGED:
             while (index != _userSettingNotification.end())
             {
                 (*index)->onPreferredAudioLanguagesChanged(params.String());
                 ++index;
             }
         break;

         case PRESENTATION_LANGUAGE_CHANGED:
             while (index != _userSettingNotification.end())
             {
                 (*index)->onPresentationLanguageChanged(params.String());
                 ++index;
             }
         break;

         case CAPTIONS_CHANGED:
             while (index != _userSettingNotification.end())
             {
                 (*index)->onCaptionsChanged(params.Boolean());
                 ++index;
             }
         break;

         case PREFERRED_CAPTIONS_LANGUAGE_CHANGED:
             while (index != _userSettingNotification.end())
             {
                 (*index)->onPreferredCaptionsLanguagesChanged(params.String());
                 ++index;
             }
         break;

         case PREFERRED_CLOSED_CAPTIONS_SERVICE_CHANGED:
             while (index != _userSettingNotification.end())
             {
                 (*index)->onPreferredClosedCaptionServiceChanged(params.String());
                 ++index;
             }
         break;

         case PRIVACY_MODE_CHANGED:
             while (index != _userSettingNotification.end())
             {
                 (*index)->onPrivacyModeChanged(params.String());
                 ++index;
             }
         break;

         case PIN_CONTROL_CHANGED:
              while (index != _userSettingNotification.end())
              {
                  (*index)->onPinControlChanged(params.Boolean());
                  ++index;
              }
         break;

         case VIEWING_RESTRICTIONS_CHANGED:
              while (index != _userSettingNotification.end())
              {
                  (*index)->onViewingRestrictionsChanged(params.String());
                  ++index;
              }
         break;

         case VIEWING_RESTRICTIONS_WINDOW_CHANGED:
              while (index != _userSettingNotification.end())
              {
                  (*index)->onViewingRestrictionsWindowChanged(params.String());
                  ++index;
              }
         break;

         case LIVE_WATERSHED_CHANGED:
              while (index != _userSettingNotification.end())
              {
                  (*index)->onLiveWatershedChanged(params.Boolean());
                  ++index;
              }
         break;

         case PLAYBACK_WATERSHED_CHANGED:
              while (index != _userSettingNotification.end())
              {
                  (*index)->onPlaybackWatershedChanged(params.Boolean());
                  ++index;
              }
         break;

         case BLOCK_NOT_RATED_CONTENT_CHANGED:
              while (index != _userSettingNotification.end())
              {
                  (*index)->onBlockNotRatedContentChanged(params.Boolean());
                  ++index;
              }
         break;

         case PIN_ON_PURCHASE_CHANGED:
              while (index != _userSettingNotification.end())
              {
                  (*index)->onPinOnPurchaseChanged(params.Boolean());
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

    if((ns.compare(USERSETTINGS_NAMESPACE) == 0) && (key.compare(USERSETTINGS_AUDIO_DESCRIPTION_KEY) == 0))
    {
        dispatchEvent(AUDIO_DESCRIPTION_CHANGED, JsonValue((bool)(value.compare("true")==0)?true:false));
    }
    else if((ns.compare(USERSETTINGS_NAMESPACE) == 0) && (key.compare(USERSETTINGS_PREFERRED_AUDIO_LANGUAGES_KEY) == 0))
    {
        dispatchEvent(PREFERRED_AUDIO_CHANGED, JsonValue((string)value));
    }
    else if((ns.compare(USERSETTINGS_NAMESPACE) == 0) && (key.compare(USERSETTINGS_PRESENTATION_LANGUAGE_KEY) == 0))
    {
        dispatchEvent(PRESENTATION_LANGUAGE_CHANGED, JsonValue((string)value));
    }
    else if((ns.compare(USERSETTINGS_NAMESPACE) == 0) && (key.compare(USERSETTINGS_CAPTIONS_KEY) == 0))
    {
        dispatchEvent(CAPTIONS_CHANGED, JsonValue((bool)(value.compare("true")==0)?true:false));
    }
    else if((ns.compare(USERSETTINGS_NAMESPACE) == 0) && (key.compare(USERSETTINGS_PREFERRED_CAPTIONS_LANGUAGES_KEY) == 0))
    {
        dispatchEvent(PREFERRED_CAPTIONS_LANGUAGE_CHANGED, JsonValue((string)value));
    }
    else if((ns.compare(USERSETTINGS_NAMESPACE) == 0) && (key.compare(USERSETTINGS_PREFERRED_CLOSED_CAPTIONS_SERVICE_KEY) == 0))
    {
        dispatchEvent(PREFERRED_CLOSED_CAPTIONS_SERVICE_CHANGED, JsonValue((string)value));
    }
    else if((ns.compare(USERSETTINGS_NAMESPACE) == 0) && (key.compare(USERSETTINGS_PRIVACY_MODE_KEY) == 0))
    {
        dispatchEvent(PRIVACY_MODE_CHANGED, JsonValue((string)value));
    }
    else if((ns.compare(USERSETTINGS_NAMESPACE) == 0) && (key.compare(USERSETTINGS_PIN_CONTROL_KEY) == 0))
    {
        dispatchEvent(PIN_CONTROL_CHANGED, JsonValue((bool)(value.compare("true")==0)?true:false));
    }
    else if((ns.compare(USERSETTINGS_NAMESPACE) == 0) && (key.compare(USERSETTINGS_VIEWING_RESTRICTIONS_KEY) == 0))
    {
        dispatchEvent(VIEWING_RESTRICTIONS_CHANGED, JsonValue((string)value));
    }
    else if((ns.compare(USERSETTINGS_NAMESPACE) == 0) && (key.compare(USERSETTINGS_VIEWING_RESTRICTIONS_WINDOW_KEY) == 0))
    {
        dispatchEvent(VIEWING_RESTRICTIONS_WINDOW_CHANGED, JsonValue((string)value));
    }
    else if((ns.compare(USERSETTINGS_NAMESPACE) == 0) && (key.compare(USERSETTINGS_LIVE_WATERSHED_KEY) == 0))
    {
        dispatchEvent(LIVE_WATERSHED_CHANGED, JsonValue((bool)(value.compare("true")==0)?true:false));
    }
    else if((ns.compare(USERSETTINGS_NAMESPACE) == 0) && (key.compare(USERSETTINGS_PLAYBACK_WATERSHED_KEY) == 0))
    {
        dispatchEvent(PLAYBACK_WATERSHED_CHANGED, JsonValue((bool)(value.compare("true")==0)?true:false));
    }
    else if((ns.compare(USERSETTINGS_NAMESPACE) == 0) && (key.compare(USERSETTINGS_BLOCK_NOT_RATED_CONTENT_KEY) == 0))
    {
        dispatchEvent(BLOCK_NOT_RATED_CONTENT_CHANGED, JsonValue((bool)(value.compare("true")==0)?true:false));
    }
    else if((ns.compare(USERSETTINGS_NAMESPACE) == 0) && (key.compare(USERSETTINGS_PIN_ON_PURCHASE_KEY) == 0))
    {
        dispatchEvent(PIN_ON_PURCHASE_CHANGED, JsonValue((bool)(value.compare("true")==0)?true:false));
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

    ASSERT (nullptr != _remotStoreObject);
    if (nullptr != _remotStoreObject)
    {
        status = _remotStoreObject->SetValue(Exchange::IStore2::ScopeType::DEVICE, USERSETTINGS_NAMESPACE, key, value, 0);
    }

    _adminLock.Unlock();
    return status;
}

uint32_t UserSettingsImplementation::GetUserSettingsValue(const string& key, string &value) const
{
    uint32_t status = Core::ERROR_GENERAL;
    uint32_t ttl = 0;
    _adminLock.Lock();

    ASSERT (nullptr != _remotStoreObject);
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
    _adminLock.Unlock();

    return status;
}

uint32_t UserSettingsImplementation::setAudioDescription(const bool enabled)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("enabled: %d", enabled);
    status = SetUserSettingsValue(USERSETTINGS_AUDIO_DESCRIPTION_KEY, (enabled)?"true":"false");
    return status;
}

uint32_t UserSettingsImplementation::getAudioDescription(bool &enabled) const
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

uint32_t UserSettingsImplementation::setPreferredAudioLanguages(const string& preferredLanguages)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("preferredLanguages: %s", preferredLanguages.c_str());
    status = SetUserSettingsValue(USERSETTINGS_PREFERRED_AUDIO_LANGUAGES_KEY, preferredLanguages);
    return status;
}

uint32_t UserSettingsImplementation::getPreferredAudioLanguages(string &preferredLanguages) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_PREFERRED_AUDIO_LANGUAGES_KEY, preferredLanguages);
    return status;
}

uint32_t UserSettingsImplementation::setPresentationLanguage(const string& presentationLanguages)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("presentationLanguages: %s", presentationLanguages.c_str());
    status = SetUserSettingsValue(USERSETTINGS_PRESENTATION_LANGUAGE_KEY, presentationLanguages);
    return status;
}

uint32_t UserSettingsImplementation::getPresentationLanguage(string &presentationLanguages) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_PRESENTATION_LANGUAGE_KEY, presentationLanguages);
    return status;
}

uint32_t UserSettingsImplementation::setCaptions(const bool enabled)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("enabled: %d", enabled);
    status = SetUserSettingsValue(USERSETTINGS_CAPTIONS_KEY, (enabled)?"true":"false");
    return status;
}

uint32_t UserSettingsImplementation::getCaptions(bool &enabled) const
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

uint32_t UserSettingsImplementation::setPreferredCaptionsLanguages(const string& preferredLanguages)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("preferredLanguages: %s", preferredLanguages.c_str());
    status = SetUserSettingsValue(USERSETTINGS_PREFERRED_CAPTIONS_LANGUAGES_KEY, preferredLanguages);
    return status;
}

uint32_t UserSettingsImplementation::getPreferredCaptionsLanguages(string &preferredLanguages) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_PREFERRED_CAPTIONS_LANGUAGES_KEY, preferredLanguages);
    return status;
}

uint32_t UserSettingsImplementation::setPreferredClosedCaptionService(const string& service)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("service: %s", service.c_str());
    status = SetUserSettingsValue(USERSETTINGS_PREFERRED_CLOSED_CAPTIONS_SERVICE_KEY, service);
    return status;
}

uint32_t UserSettingsImplementation::getPreferredClosedCaptionService(string &service) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_PREFERRED_CLOSED_CAPTIONS_SERVICE_KEY, service);
    return status;
}

uint32_t UserSettingsImplementation::setPrivacyMode(const string& privacyMode)
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

uint32_t UserSettingsImplementation::getPrivacyMode(string &privacyMode) const
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

uint32_t UserSettingsImplementation::setPinControl(const bool enabled)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("enabled: %d", enabled);
    status = SetUserSettingsValue(USERSETTINGS_PIN_CONTROL_KEY, (enabled)?"true":"false");
    return status;
}

uint32_t UserSettingsImplementation::getPinControl(bool &enabled) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_PIN_CONTROL_KEY, value);

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

uint32_t UserSettingsImplementation::setViewingRestrictions(const string& viewingRestrictions)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("viewingRestrictions: %s", viewingRestrictions.c_str());
    status = SetUserSettingsValue(USERSETTINGS_VIEWING_RESTRICTIONS_KEY, viewingRestrictions);
    return status;

}

uint32_t UserSettingsImplementation::getViewingRestrictions(string &viewingRestrictions) const
{
    uint32_t status = Core::ERROR_GENERAL;

    status = GetUserSettingsValue(USERSETTINGS_VIEWING_RESTRICTIONS_KEY, viewingRestrictions);
    return status;

}

uint32_t UserSettingsImplementation::setViewingRestrictionsWindow(const string& viewingRestrictionsWindow)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("viewingRestrictionsWindow: %s", viewingRestrictionsWindow.c_str());
    status = SetUserSettingsValue(USERSETTINGS_VIEWING_RESTRICTIONS_WINDOW_KEY, viewingRestrictionsWindow);
    return status;

}

uint32_t UserSettingsImplementation::getViewingRestrictionsWindow(string &viewingRestrictionsWindow) const
{
    uint32_t status = Core::ERROR_GENERAL;

    status = GetUserSettingsValue(USERSETTINGS_VIEWING_RESTRICTIONS_WINDOW_KEY, viewingRestrictionsWindow);
    return status;
}

uint32_t UserSettingsImplementation::setLiveWatershed(const bool enabled)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("enabled: %d", enabled);
    status = SetUserSettingsValue(USERSETTINGS_LIVE_WATERSHED_KEY, (enabled)?"true":"false");
    return status;

}

uint32_t UserSettingsImplementation::getLiveWatershed(bool &enabled) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_LIVE_WATERSHED_KEY, value);

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

uint32_t UserSettingsImplementation::setPlaybackWatershed(const bool enabled)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("enabled: %d", enabled);
    status = SetUserSettingsValue(USERSETTINGS_PLAYBACK_WATERSHED_KEY, (enabled)?"true":"false");
    return status;
}

uint32_t UserSettingsImplementation::getPlaybackWatershed(bool &enabled) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_PLAYBACK_WATERSHED_KEY, value);

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

uint32_t UserSettingsImplementation::setBlockNotRatedContent(const bool enabled)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("enabled: %d", enabled);
    status = SetUserSettingsValue(USERSETTINGS_BLOCK_NOT_RATED_CONTENT_KEY, (enabled)?"true":"false");
    return status;
}

uint32_t UserSettingsImplementation::getBlockNotRatedContent(bool &enabled) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_BLOCK_NOT_RATED_CONTENT_KEY, value);

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

uint32_t UserSettingsImplementation::setPinOnPurchase(const bool enabled)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("enabled: %d", enabled);
    status = SetUserSettingsValue(USERSETTINGS_PIN_ON_PURCHASE_KEY, (enabled)?"true":"false");
    return status;
}

uint32_t UserSettingsImplementation::getPinOnPurchase(bool &enabled) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";

    status = GetUserSettingsValue(USERSETTINGS_PIN_ON_PURCHASE_KEY, value);

    if(Core::ERROR_NONE == status)
    {
        LOGINFO("getPinOnPurchase: %d", enabled);

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

} // namespace Plugin
} // namespace WPEFramework
