/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright Synamedia, All rights reserved
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

#include <algorithm>
#include <regex>
#include "Module.h"
#include "UnifiedCASManagement.h"
#include "LibMediaPlayerImpl.h"

#include "UtilsCStr.h"
#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 2
#define API_VERSION_NUMBER_PATCH 0

const string WPEFramework::Plugin::UnifiedCASManagement::METHOD_MANAGE = "manage";
const string WPEFramework::Plugin::UnifiedCASManagement::METHOD_UNMANAGE = "unmanage";
const string WPEFramework::Plugin::UnifiedCASManagement::METHOD_SEND = "send";
const string WPEFramework::Plugin::UnifiedCASManagement::EVENT_DATA = "data";

namespace WPEFramework
{

namespace Plugin
{

UnifiedCASManagement* UnifiedCASManagement::_instance = nullptr;

UnifiedCASManagement::UnifiedCASManagement()
{
#ifdef LMPLAYER_FOUND
    m_player = std::make_shared<LibMediaPlayerImpl>(this);
#else
    m_player = nullptr;
    LOGERR("NO VALID PLAYER AVAILABLE TO USE");
#endif
    _instance = this;
    RegisterAll();
}

UnifiedCASManagement::~UnifiedCASManagement()
{
    UnregisterAll();
    UnifiedCASManagement::_instance = nullptr;
}

const string UnifiedCASManagement::Initialize(PluginHost::IShell * /* service */)
{
    return (string());
}

void UnifiedCASManagement::Deinitialize(PluginHost::IShell * /* service */)
{
    UnifiedCASManagement::_instance = nullptr;
}

string UnifiedCASManagement::Information() const
{
    return (string());
}

//Registration
SERVICE_REGISTRATION(UnifiedCASManagement, 1, 0);

void UnifiedCASManagement::RegisterAll()
{
    Register(METHOD_MANAGE, &UnifiedCASManagement::manage, this);
    Register(METHOD_UNMANAGE, &UnifiedCASManagement::unmanage, this);
    Register(METHOD_SEND, &UnifiedCASManagement::send, this);
}

void UnifiedCASManagement::UnregisterAll()
{
    Unregister(METHOD_MANAGE);
    Unregister(METHOD_UNMANAGE);
    Unregister(METHOD_SEND);
}

// API implementation

// Method: manage - Manage a well-known CAS
// Return codes:
//  - ERROR_NONE: Success
uint32_t UnifiedCASManagement::manage(const JsonObject& params, JsonObject& response)
{
    bool success = false;

    if(nullptr == m_player)
    {
        LOGERR("NO VALID PLAYER AVAILABLE TO USE");
        returnResponse(success);
    }

    returnIfStringParamNotFound(params, "mode");
    returnIfStringParamNotFound(params, "manage");

    const std::string& mediaurl = params["mediaurl"].String();
    const std::string& mode = params["mode"].String();
    const std::string& manage = params["manage"].String();
    const std::string& casinitdata = params["casinitdata"].String();
    const std::string& casocdmid = params["casocdmid"].String();

    LOGINFO("media URL:%s, ocdmid = %s", mediaurl.c_str(), casocdmid.c_str());

    if(mode != "MODE_NONE")
    {
        LOGERR("mode must be MODE_NONE for CAS Management");
    }
    else if (manage != "MANAGE_FULL" && manage != "MANAGE_NO_PSI" && manage != "MANAGE_NO_TUNER")
    {
        LOGERR("manage must be MANAGE_ ... FULL, NO_PSI or NO_TUNER for CAS MAnagement");
    }
    else if(casocdmid.empty())
    {
        LOGERR("ocdmcasid is mandatory for CAS management session");
    }
    else
    {
        success = true;
    }

    if(success == false)
    {
        LOGERR("UnifiedCASManagement Open Session Failed");
    }
    else
    {
        JsonObject jsonParams;
        jsonParams["mediaurl"] = mediaurl;
        jsonParams["mode"] = mode;
        jsonParams["manage"] = manage;
        jsonParams["casinitdata"] = casinitdata;
        jsonParams["casocdmid"] = casocdmid;

        std::string openParams;
        jsonParams.ToString(openParams);
        LOGINFO("OpenData = %s\n", openParams.c_str());

        if (false == m_player->openMediaPlayer(openParams, manage))
        {
            LOGERR("Failed to open MediaPlayer");
            success = false;
        }
    }
    returnResponse(success);
}

// Method: unmanage - Destroy a management session
// Return codes:
//  - ERROR_NONE: Success
uint32_t UnifiedCASManagement::unmanage(const JsonObject& params, JsonObject& response)
{
    bool success = false;

    if(nullptr == m_player)
    {
        LOGERR("NO VALID PLAYER AVAILABLE TO USE");
        returnResponse(success);
    }

    if (false == m_player->closeMediaPlayer())
    {
         LOGERR("Failed to close MediaPlayer");
         LOGWARN("Error in destroying CAS Management Session...\n");
    }
    else
    {
         LOGINFO("Successful in destroying CAS Management Session...\n");
         success = true;
    }
    returnResponse(success);
}

// Method: send - Sends data to the remote CAS
// Return codes:
//  - ERROR_NONE: Success
uint32_t UnifiedCASManagement::send(const JsonObject& params, JsonObject& response)
{
    bool success = false;

    if(nullptr == m_player)
    {
        LOGERR("NO VALID PLAYER AVAILABLE TO USE");
        returnResponse(success);
    }

    const std::string& payload = params["payload"].String();
    const std::string& source = params["source"].String();
    JsonObject jsonParams;
    jsonParams["payload"] = payload;
    jsonParams["source"] = source;

    std::string data;
    jsonParams.ToString(data);
    LOGINFO("Send Data = %s\n", data.c_str());

    if (false == m_player->requestCASData(data))
    {
        LOGERR("requestCASData failed");
    }
    else
    {
        LOGINFO("UnifiedCASManagement send Data succeeded.. Calling Play\n");
        success = true;
    }
    returnResponse(success);
}

// Event: data - Sent when the CAS needs to send data to the caller
void UnifiedCASManagement::event_data(const std::string& payload, const std::string& source)
{
    JsonObject params;
    params["payload"] = payload;
    params["source"] = source;
    sendNotify(EVENT_DATA.c_str(), params);
}

} // namespace

} // namespace
