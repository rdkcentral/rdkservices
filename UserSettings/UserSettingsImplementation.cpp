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

} // namespace Plugin
} // namespace WPEFramework
