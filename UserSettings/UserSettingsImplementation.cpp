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

#define USERSETTINGS_NAMESPACE "UserSettings"

#define USERSETTINGS_AUDIO_DESCRIPTION_KEY                    "audioDescription"
#define USERSETTINGS_PREFERRED_AUDIO_LANGUAGES_KEY            "preferredAudioLanguages"
#define USERSETTINGS_PRESENTATION_LANGUAGE_KEY                "presentationLanguage"
#define USERSETTINGS_CAPTIONS_KEY                             "captions"
#define USERSETTINGS_PREFERRED_CAPTIONS_LANGUAGES_KEY         "preferredCaptionsLanguages"
#define USERSETTINGS_PREFERRED_CLOSED_CAPTIONS_SERVICE_KEY    "preferredClosedCaptionsService"

namespace WPEFramework {
namespace Plugin {

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
        _engine->Announcements(_communicatorClient->Announcement());

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
    else
    {
        LOGERR("Not supported");
    }
}

uint32_t UserSettingsImplementation::SetAudioDescription(const bool enabled)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("enabled: %d", enabled);

    _adminLock.Lock();

    ASSERT (nullptr != _remotStoreObject);

    if (nullptr != _remotStoreObject)
    {
        status = _remotStoreObject->SetValue(Exchange::IStore2::ScopeType::DEVICE, USERSETTINGS_NAMESPACE, USERSETTINGS_AUDIO_DESCRIPTION_KEY, (enabled)?"true":"false", 0);
    }

    _adminLock.Unlock();

    return status;
}

uint32_t UserSettingsImplementation::GetAudioDescription(bool &enabled) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";
    uint32_t ttl = 0;

    _adminLock.Lock();

    ASSERT (nullptr != _remotStoreObject);

    if (nullptr != _remotStoreObject)
    {
        status = _remotStoreObject->GetValue(Exchange::IStore2::ScopeType::DEVICE, USERSETTINGS_NAMESPACE, USERSETTINGS_AUDIO_DESCRIPTION_KEY, value, ttl);

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
    }

    _adminLock.Unlock();

    return status;
}

uint32_t UserSettingsImplementation::SetPreferredAudioLanguages(const string preferredLanguages)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("preferredLanguages: %s", preferredLanguages.c_str());

    _adminLock.Lock();

    ASSERT (nullptr != _remotStoreObject);

    if (nullptr != _remotStoreObject)
    {
        status = _remotStoreObject->SetValue(Exchange::IStore2::ScopeType::DEVICE, USERSETTINGS_NAMESPACE, USERSETTINGS_PREFERRED_AUDIO_LANGUAGES_KEY, preferredLanguages, 0);
    }

    _adminLock.Unlock();

    return status;
}

uint32_t UserSettingsImplementation::GetPreferredAudioLanguages(string &preferredLanguages) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";
    uint32_t ttl = 0;

    _adminLock.Lock();

    ASSERT (nullptr != _remotStoreObject);

    if (nullptr != _remotStoreObject)
    {
        status = _remotStoreObject->GetValue(Exchange::IStore2::ScopeType::DEVICE, USERSETTINGS_NAMESPACE, USERSETTINGS_PREFERRED_AUDIO_LANGUAGES_KEY, preferredLanguages, ttl);
    }

    _adminLock.Unlock();

    return status;
}

uint32_t UserSettingsImplementation::SetPresentationLanguage(const string presentationLanguages)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("presentationLanguages: %s", presentationLanguages.c_str());

    _adminLock.Lock();

    ASSERT (nullptr != _remotStoreObject);

    if (nullptr != _remotStoreObject)
    {
        status = _remotStoreObject->SetValue(Exchange::IStore2::ScopeType::DEVICE, USERSETTINGS_NAMESPACE, USERSETTINGS_PRESENTATION_LANGUAGE_KEY, presentationLanguages, 0);
    }

    _adminLock.Unlock();

    return status;
}

uint32_t UserSettingsImplementation::GetPresentationLanguage(string &presentationLanguages) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";
    uint32_t ttl = 0;

    _adminLock.Lock();

    ASSERT (nullptr != _remotStoreObject);

    if (nullptr != _remotStoreObject)
    {
        status = _remotStoreObject->GetValue(Exchange::IStore2::ScopeType::DEVICE, USERSETTINGS_NAMESPACE, USERSETTINGS_PRESENTATION_LANGUAGE_KEY, presentationLanguages, ttl);
    }

    _adminLock.Unlock();

    return status;
}

uint32_t UserSettingsImplementation::SetCaptions(const bool enabled)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("enabled: %d", enabled);

    _adminLock.Lock();

    ASSERT (nullptr != _remotStoreObject);

    if (nullptr != _remotStoreObject)
    {
        status = _remotStoreObject->SetValue(Exchange::IStore2::ScopeType::DEVICE, USERSETTINGS_NAMESPACE, USERSETTINGS_CAPTIONS_KEY, (enabled)?"true":"false", 0);
    }

    _adminLock.Unlock();

    return status;
}

uint32_t UserSettingsImplementation::GetCaptions(bool &enabled) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";
    uint32_t ttl = 0;

    _adminLock.Lock();

    ASSERT (nullptr != _remotStoreObject);

    if (nullptr != _remotStoreObject)
    {
        status = _remotStoreObject->GetValue(Exchange::IStore2::ScopeType::DEVICE, USERSETTINGS_NAMESPACE, USERSETTINGS_CAPTIONS_KEY, value, ttl);

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
    }

    _adminLock.Unlock();

    return status;
}

uint32_t UserSettingsImplementation::SetPreferredCaptionsLanguages(const string preferredLanguages)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("preferredLanguages: %s", preferredLanguages.c_str());

    _adminLock.Lock();

    ASSERT (nullptr != _remotStoreObject);

    if (nullptr != _remotStoreObject)
    {
        status = _remotStoreObject->SetValue(Exchange::IStore2::ScopeType::DEVICE, USERSETTINGS_NAMESPACE, USERSETTINGS_PREFERRED_CAPTIONS_LANGUAGES_KEY, preferredLanguages, 0);
    }

    _adminLock.Unlock();

    return status;
}

uint32_t UserSettingsImplementation::GetPreferredCaptionsLanguages(string &preferredLanguages) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";
    uint32_t ttl = 0;

    _adminLock.Lock();

    ASSERT (nullptr != _remotStoreObject);

    if (nullptr != _remotStoreObject)
    {
        status = _remotStoreObject->GetValue(Exchange::IStore2::ScopeType::DEVICE, USERSETTINGS_NAMESPACE, USERSETTINGS_PREFERRED_CAPTIONS_LANGUAGES_KEY, preferredLanguages, ttl);
    }

    _adminLock.Unlock();

    return status;
}

uint32_t UserSettingsImplementation::SetPreferredClosedCaptionService(const string service)
{
    uint32_t status = Core::ERROR_GENERAL;

    LOGINFO("service: %s", service.c_str());

    _adminLock.Lock();

    ASSERT (nullptr != _remotStoreObject);

    if (nullptr != _remotStoreObject)
    {
        status = _remotStoreObject->SetValue(Exchange::IStore2::ScopeType::DEVICE, USERSETTINGS_NAMESPACE, USERSETTINGS_PREFERRED_CLOSED_CAPTIONS_SERVICE_KEY, service, 0);
    }

    _adminLock.Unlock();

    return status;
}

uint32_t UserSettingsImplementation::GetPreferredClosedCaptionService(string &service) const
{
    uint32_t status = Core::ERROR_GENERAL;
    std::string value = "";
    uint32_t ttl = 0;

    _adminLock.Lock();

    ASSERT (nullptr != _remotStoreObject);

    if (nullptr != _remotStoreObject)
    {
        status = _remotStoreObject->GetValue(Exchange::IStore2::ScopeType::DEVICE, USERSETTINGS_NAMESPACE, USERSETTINGS_PREFERRED_CLOSED_CAPTIONS_SERVICE_KEY, service, ttl);
    }

    _adminLock.Unlock();

    return status;
}
} // namespace Plugin
} // namespace WPEFramework
