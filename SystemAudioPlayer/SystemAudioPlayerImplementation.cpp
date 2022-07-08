/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
 */

#include "SystemAudioPlayerImplementation.h"
#include <sys/prctl.h>
#include "impl/Helper.h"
#include "base64.h"
#include "UtilsJsonRpc.h"

#define SAP_MAJOR_VERSION 1
#define SAP_MINOR_VERSION 0

#define GET_STR(map, key, def) ((map.HasLabel(key) && !map[key].String().empty() && map[key].String() != "null") ? map[key].String() : def)
#define CONVERT_PARAMETERS_TOJSON() JsonObject parameters, response; parameters.FromString(input);
#define CONVERT_PARAMETERS_FROMJSON() response.ToString(output);

#undef returnResponse
#define returnResponse(success) \
    response["success"] = success; \
    CONVERT_PARAMETERS_FROMJSON(); \
    LOGTRACEMETHODFIN(); \
    return (Core::ERROR_NONE);

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(SystemAudioPlayerImplementation, SAP_MAJOR_VERSION, SAP_MINOR_VERSION);


    SystemAudioPlayerImplementation::SystemAudioPlayerImplementation() : _adminLock()
    {
        AudioPlayer::Init(this);
        SAPLOG_INFO("SAP: SystemAudioPlayerImplementation Constructor\n");
    }

    SystemAudioPlayerImplementation::~SystemAudioPlayerImplementation()
    {
        AudioPlayer::DeInit();
        SAPLOG_INFO("SAP: SystemAudioPlayerImplementation Destructor\n");
    }

    uint32_t SystemAudioPlayerImplementation::Configure(PluginHost::IShell* service)
    {
     
    }

    void SystemAudioPlayerImplementation::Register(Exchange::ISystemAudioPlayer::INotification* sink)
    {
        _adminLock.Lock();

        // Make sure a sink is not registered multiple times.
        ASSERT(std::find(_notificationClients.begin(), _notificationClients.end(), sink) == _notificationClients.end());

        _notificationClients.push_back(sink);
        sink->AddRef();

        _adminLock.Unlock();

        TRACE_L1("Registered a sink on the browser %p", sink);
    }

    void SystemAudioPlayerImplementation::Unregister(Exchange::ISystemAudioPlayer::INotification* sink)
    {
        _adminLock.Lock();

        std::list<Exchange::ISystemAudioPlayer::INotification*>::iterator index(std::find(_notificationClients.begin(), _notificationClients.end(), sink));

        // Make sure you do not unregister something you did not register !!!
        ASSERT(index != _notificationClients.end());

        if (index != _notificationClients.end()) {
            (*index)->Release();
            _notificationClients.erase(index);
            TRACE_L1("Unregistered a sink on the browser %p", sink);
        }

        _adminLock.Unlock();
    }

    uint32_t SystemAudioPlayerImplementation::Open(const string &input, string &output)
    {   
        SAPLOG_INFO("SystemAudioPlayerImplementation Got Open request :%s\n",input.c_str());
        CONVERT_PARAMETERS_TOJSON();
        CHECK_SAP_PARAMETER_RETURN_ON_FAIL("audiotype");
        CHECK_SAP_PARAMETER_RETURN_ON_FAIL("sourcetype");
        SAPLOG_INFO("SAP: SystemAudioPlayerImplementation Open\n");
        AudioType audioType;
        SourceType sourceType;
        PlayMode playMode;
        std::string s_audioType,s_sourceType,s_playMode;
      
        s_audioType = parameters["audiotype"].String(); 
        s_sourceType = parameters["sourcetype"].String();
        s_playMode = parameters["playmode"].String();    
        audioType = audioTypeFromString( s_audioType);
        sourceType = sourceTypeFromString( s_sourceType);
        playMode = playModeFromString( s_playMode);

        int id;
        _adminLock.Lock();      
        OpenMapping(audioType,sourceType,playMode,id);        
        _adminLock.Unlock();
        response["id"] = (int) id;
        returnResponse(true);
    }

     uint32_t SystemAudioPlayerImplementation::Config(const string &input, string &output)
    {
        SAPLOG_INFO("SystemAudioPlayerImplementation Got Config request :%s\n",input.c_str());
        CONVERT_PARAMETERS_TOJSON();
        CHECK_SAP_PARAMETER_RETURN_ON_FAIL("id");
        int id, rate, channels;
        bool ret = false;
        string format, layout;
        AudioPlayer *player;
        getNumberParameter("id", id);
        _adminLock.Lock();
         player = getObjectFromMap(id);
        _adminLock.Unlock();
        if(player != NULL)
        {
            //TODO parse pcmconfig and pass
            if( player->getAudioType() == AudioType::PCM)
            {
                CHECK_SAP_PARAMETER_RETURN_ON_FAIL("pcmconfig");
                JsonObject config = parameters["pcmconfig"].Object();
                CHECK_SAP_CONFIG_RETURN_ON_FAIL("format");
                CHECK_SAP_CONFIG_RETURN_ON_FAIL("rate");
                CHECK_SAP_CONFIG_RETURN_ON_FAIL("channels");
                CHECK_SAP_CONFIG_RETURN_ON_FAIL("layout");
                format = config["format"].String();
                layout = config["layout"].String();
                getNumberConfigParameter("rate",rate);
                getNumberConfigParameter("channels",channels);
                SAPLOG_INFO("SAP: Do PCM config ");
                ret= player->configPCMCaps(format,rate,channels,layout);
            }
            if (player->getSourceType() == SourceType::WEBSOCKET && parameters.HasLabel("websocketsecparam"))
            {
                SAPLOG_INFO("Configuring security parameters for websocket audio player.");
                player->configWsSecParams(extractSecurityParams(parameters));
            }
            returnResponse(ret);

        }
        returnResponse(false);
    }
    
    uint32_t SystemAudioPlayerImplementation::GetPlayerSessionId(const string &input, string &output)
    {
        SAPLOG_INFO("SystemAudioPlayerImplementation Got GetPlayerSessionID request :%s\n",input.c_str());
        CONVERT_PARAMETERS_TOJSON();
        CHECK_SAP_PARAMETER_RETURN_ON_FAIL("url");
        string url;
        int playerid;
        url = parameters["url"].String();
        extractFileProtocol(url); //we do not store file:// for file playback
        _adminLock.Lock();
        if(GetSessionFromUrl(url,playerid))
        {
            response["sessionId"] = (int) playerid;
        }
        else
        {
            response["sessionId"] = (int) -1;
        }
        _adminLock.Unlock();
        returnResponse(true);
    }

    uint32_t SystemAudioPlayerImplementation::Play(const string &input, string &output)
    {
        SAPLOG_INFO("SystemAudioPlayerImplementation Got Play request :%s\n",input.c_str());
        CONVERT_PARAMETERS_TOJSON();
        CHECK_SAP_PARAMETER_RETURN_ON_FAIL("id");
        CHECK_SAP_PARAMETER_RETURN_ON_FAIL("url");
        SAPLOG_INFO("SAP: SystemAudioPlayerImplementation Play\n");
        int id;
        string url;
        AudioPlayer *player;
        url = parameters["url"].String(); 
        getNumberParameter("id", id);
        _adminLock.Lock();
         player = getObjectFromMap(id); 
        _adminLock.Unlock();
        if(player != NULL)
        {
            if(player->getSourceType() == SourceType::FILESRC)
            {
                if(!extractFileProtocol(url))
                {
                    returnResponse(false);
                }
            }
            _adminLock.Lock();
            if(SameModeNotPlaying(player,id))
            {
                _adminLock.Unlock();
                player->Play(url);
                returnResponse(true);
            }
            _adminLock.Unlock();
            response["message"] = "Hardware resource already acquired by session with  id "+ std::to_string(id);
            returnResponse(false);
        }
        returnResponse(false);
    }

    uint32_t SystemAudioPlayerImplementation::PlayBuffer(const string &input, string &output)
    {
        SAPLOG_INFO("SystemAudioPlayerImplementation Got PlayBuffer request :%s\n",input.c_str());
        CONVERT_PARAMETERS_TOJSON();
        AudioPlayer *player;
        int id;
        std::string data;
        LOGINFO("PlayBuffer request\n");
        getNumberParameter("id", id);
        data = parameters["data"].String();
        LOGINFO("data size %d\n",data.size());
        _adminLock.Lock();
         player = getObjectFromMap(id);
        _adminLock.Unlock();
        if(player != NULL)
        {           
            std::vector<uint8_t> dectokenVec(data.begin(), data.end());
            uint8_t *e = &dectokenVec[0];
            size_t decworkspace_size = b64_get_decoded_buffer_size(dectokenVec.size());
            uint8_t *decworkspace = new uint8_t[decworkspace_size];
            size_t decnum_chars = b64_decode(e, dectokenVec.size(),decworkspace);
            LOGINFO("decode size %d\n", decworkspace_size);
            player->PlayBuffer((const char*)decworkspace,decnum_chars);
            delete []decworkspace;
            
            returnResponse(true);
        }
        returnResponse(false);
    }

    uint32_t SystemAudioPlayerImplementation::Stop(const string &input, string &output)
    {
        SAPLOG_INFO("SystemAudioPlayerImplementation Got Stop request :%s\n",input.c_str());
        CONVERT_PARAMETERS_TOJSON();
        AudioPlayer *player;
        int id;
        getNumberParameter("id", id);
        SAPLOG_INFO("SAP: SystemAudioPlayerImplementation Stop\n");
        _adminLock.Lock();
         player = getObjectFromMap(id);
        _adminLock.Unlock();
         if(player != NULL)
        {
            player->Stop();;
            returnResponse(true);
        }
        returnResponse(false);
    }

    uint32_t SystemAudioPlayerImplementation::Close(const string &input, string &output)
    {
        SAPLOG_INFO("SystemAudioPlayerImplementation Got Close request :%s\n",input.c_str());
        CONVERT_PARAMETERS_TOJSON();
        int id;
        getNumberParameter("id", id);
        SAPLOG_INFO("SAP: SystemAudioPlayerImplementation Close\n");
        _adminLock.Lock();
        
          if(CloseMapping(id))
        {
             _adminLock.Unlock();
            returnResponse(true);
        }
        _adminLock.Unlock();
        returnResponse(false);
    }

    uint32_t nextId() {
        static uint32_t counter = 0;

        if(counter >= 0xFFFFFFFF)
            counter = 0;

        return ++counter;
    }

    uint32_t SystemAudioPlayerImplementation::SetMixerLevels(const string &input, string &output)
    {
        SAPLOG_INFO("SystemAudioPlayerImplementation Got SetMixerLevels request :%s\n",input.c_str());
        CONVERT_PARAMETERS_TOJSON();
        CHECK_SAP_PARAMETER_RETURN_ON_FAIL("id");
        CHECK_SAP_PARAMETER_RETURN_ON_FAIL("primaryVolume");
        CHECK_SAP_PARAMETER_RETURN_ON_FAIL("playerVolume");
        bool result = false;
        int primVol = -1;
        int thisVol = -1;
        AudioPlayer *player;
        int playerId;
        getNumberParameter("id", playerId);
        getNumberParameter("primaryVolume", primVol);
        getNumberParameter("playerVolume", thisVol);
        _adminLock.Lock();
        player = getObjectFromMap(playerId);
        _adminLock.Unlock();
        if (player != NULL &&  ( primVol >= 0 && thisVol >= 0 ) )
        {
            player->SetMixerLevels(primVol, thisVol);
            result = true;
        }
        else
        {
            SAPLOG_ERROR("SAP: setMixerLevels failed Player Obj=%p primvol=%d thisVol=%d", player,primVol,thisVol);
            result = false;
        }
        returnResponse(result);
    }

    uint32_t SystemAudioPlayerImplementation::SetSmartVolControl(const string &input, string &output)
    {
        SAPLOG_INFO("SystemAudioPlayerImplementation Got SetSmartVolControl request :%s\n",input.c_str());
        CONVERT_PARAMETERS_TOJSON();
        CHECK_SAP_PARAMETER_RETURN_ON_FAIL("id");
        CHECK_SAP_PARAMETER_RETURN_ON_FAIL("enable");
        CHECK_SAP_PARAMETER_RETURN_ON_FAIL("playerAudioLevelThreshold");
        CHECK_SAP_PARAMETER_RETURN_ON_FAIL("playerDetectTimeMs");
        CHECK_SAP_PARAMETER_RETURN_ON_FAIL("playerHoldTimeMs");
        CHECK_SAP_PARAMETER_RETURN_ON_FAIL("primaryDuckingPercent");
        bool result = false;
        double thresHold = 0.0;
        int detectTimeMs = -1;
        int holdTimeMs = -1;
        int duckPercent = -1;
        bool smartVolumeEnable = false;
        AudioPlayer *player;
        int playerId;
        getNumberParameter("id", playerId);
        getBoolParameter("enable",smartVolumeEnable);
        getFloatParameter("playerAudioLevelThreshold", thresHold);
        getNumberParameter("playerDetectTimeMs", detectTimeMs);
        getNumberParameter("playerHoldTimeMs", holdTimeMs);
        getNumberParameter("primaryDuckingPercent", duckPercent);

        SAPLOG_INFO("SAP: SetSmartVolControl Player Obj=%p isEnable=%d thresHold=%f detectTimeMs=%d holdTimeMs=%d duckPercent=%d", player,smartVolumeEnable,thresHold, detectTimeMs,holdTimeMs,duckPercent);

        _adminLock.Lock();
        player = getObjectFromMap(playerId);
        _adminLock.Unlock();
        if (player != NULL &&  ( thresHold >= 0.0 && detectTimeMs >= 0 && holdTimeMs >= 0 && duckPercent >= 0 ) )
        {
            player->SetSmartVolControl( smartVolumeEnable, thresHold, detectTimeMs, holdTimeMs, duckPercent);
            result = true;
        }
        else
        {
            SAPLOG_ERROR("SAP: SetSmartVolControl failed Player Obj=%p isActive=%d thresHold=%f detectTimeMs=%d holdTimeMs=%d duckPercent=%d", player,smartVolumeEnable, thresHold, detectTimeMs,holdTimeMs,duckPercent);
            result = false;
        }
        returnResponse(result);

    }

    uint32_t SystemAudioPlayerImplementation::Pause(const string &input, string &output)
    {
        SAPLOG_INFO("SystemAudioPlayerImplementation Got Pause request :%s\n",input.c_str());
        CONVERT_PARAMETERS_TOJSON();
        AudioPlayer *player;
        int id;
        bool ret = false;
        getNumberParameter("id", id);
        _adminLock.Lock();
        player = getObjectFromMap(id);
        _adminLock.Unlock();
        if(player != NULL)
        {
            ret = player->Pause();
        }
        returnResponse(ret);
    }

    uint32_t SystemAudioPlayerImplementation::Resume(const string &input, string &output)
    {
        SAPLOG_INFO("SystemAudioPlayerImplementation Got Resume request :%s\n",input.c_str());
        CONVERT_PARAMETERS_TOJSON();
        AudioPlayer *player;
        int id;
        bool ret = false;
        getNumberParameter("id", id);
        SAPLOG_INFO("SAP: SystemAudioPlayerImplementation Resume\n");
        _adminLock.Lock();
        player = getObjectFromMap(id);
        _adminLock.Unlock();
        if(player != NULL)
        {
            ret = player->Resume();;
        }
        returnResponse(ret);
    }

    uint32_t SystemAudioPlayerImplementation::IsPlaying(const string &input, string &output)
    {
        CONVERT_PARAMETERS_TOJSON();
        int id;
        getNumberParameter("id", id);
        _adminLock.Lock();

        _adminLock.Unlock();
    }

    impl::SecurityParameters SystemAudioPlayerImplementation::extractSecurityParams(const JsonObject& params) const
    {
        impl::SecurityParameters output;
        const auto& secParams = params.Get("websocketsecparam").Object();
        if (secParams.HasLabel("cafilenames"))
        {
            const auto& CAFileNames = secParams.Get("cafilenames").Array();
            for (int i = 0; i < CAFileNames.Length(); ++i)
            {
                if (CAFileNames[i].Object().HasLabel("cafilename"))
                {
                    output.CAFileNames.push_back(CAFileNames[i].Object().Get("cafilename").String());
                }
                else
                {
                    SAPLOG_ERROR("Malformed CA file name entry detected at index: %d.", i);
                }
            }
        }
        if (secParams.HasLabel("certfilename"))
        {
            output.certFileName = secParams.Get("certfilename").String();
        }
        if (secParams.HasLabel("keyfilename"))
        {
            output.keyFileName = secParams.Get("keyfilename").String();
        }
        return output;
    }

    void SystemAudioPlayerImplementation::dispatchEvent(Event event, JsonObject &params)
    {
        string data;
        params.ToString(data);
        Core::IWorkerPool::Instance().Submit(Job::Create(this, event, data));
    }

    void SystemAudioPlayerImplementation::Dispatch(Event event, string data)
    {
        _adminLock.Lock();
        std::list<Exchange::ISystemAudioPlayer::INotification*>::iterator index(_notificationClients.begin());

        while (index != _notificationClients.end()) {
            (*index)->OnSAPEvents(data);
            ++index;
        }
        _adminLock.Unlock();

    }

    void SystemAudioPlayerImplementation::onSAPEvent(uint32_t id,std::string message) 
    {
        JsonObject params;
        params["id"]  = JsonValue((int)id);
        params["event"] = message;
        dispatchEvent(ONSAPEVENT, params);
    }
    
    void SystemAudioPlayerImplementation::OpenMapping(AudioType audioType,SourceType sourceType,PlayMode mode,int &playerid)
    {
        playerid= nextId();
        AudioPlayer *obj=new AudioPlayer(audioType,sourceType,mode,playerid);
        objectMap[playerid] = obj;
        SAPLOG_INFO("SAP: SystemAudioPlayerImplementation New player created\n");
    }
   
    bool SystemAudioPlayerImplementation::GetSessionFromUrl(string url,int &playerid)
    {
        std::map<int,AudioPlayer*>::iterator it = objectMap.begin();
        while (it != objectMap.end())
        {
            AudioPlayer *iplayer = it->second;
            if((url.compare(iplayer->getUrl())) == 0)
            {
               playerid = iplayer->getObjectIdentifier();
               SAPLOG_INFO("SAP: GetSessionFromUrl url %s found in list id: %d \n",url.c_str(),playerid);
               return true;
            }
            it++;
        }
        SAPLOG_INFO("SAP: GetSessionFromUrl url %s Not found in list \n",url.c_str());
        return false;
    }

    /*
    If the same mode( app/system) player is playing already, do not allow play back for this player. Otherwise cleanup previous player's state and allow this player.
    */
    bool SystemAudioPlayerImplementation::SameModeNotPlaying(AudioPlayer *player,int &playerid)
    {
        std::map<int,AudioPlayer*>::iterator it = objectMap.begin();
        while (it != objectMap.end())
        {
            AudioPlayer *iplayer = it->second;
            if((player->getObjectIdentifier() != iplayer->getObjectIdentifier()) &&
               (player->getPlayMode() == iplayer->getPlayMode()))        
            {
                if(iplayer->isPlaying())
                {
                    SAPLOG_INFO("SystemAudioPlayerImplementation play request rejected access for id %d",player->getObjectIdentifier());
                    playerid = iplayer->getObjectIdentifier();
                    return false;
                }
                iplayer->Stop();
            }
            it++;        
        }
        SAPLOG_INFO("SystemAudioPlayerImplementation play request granded access for id %d",player->getObjectIdentifier());
        return true;
    }

    bool SystemAudioPlayerImplementation::CloseMapping(int key)
    {
        AudioPlayer *player;
        player = getObjectFromMap(key);
        if(player != NULL)
        {
            objectMap.erase(key);
            player->Stop();
            delete player;
            SAPLOG_INFO("SystemAudioPlayerImplementation closemapping success for key %d\n",key);
            return true;
        }
        SAPLOG_INFO("SystemAudioPlayerImplementation closemapping failed for key %d\n",key);
        return false;
    }

    AudioPlayer* SystemAudioPlayerImplementation::getObjectFromMap(int key)
    {
        std::map<int,AudioPlayer*>::iterator it;
        it = objectMap.find(key);
        if (it != objectMap.end())
        {
            AudioPlayer *player = it->second;
            SAPLOG_INFO("SystemAudioPlayerImplementation getObjectFromMap success for key %d\n",key);
            return player;
        }
        SAPLOG_INFO("SystemAudioPlayerImplementation getObjectFromMap failed for key %d\n",key);
        return NULL;
    }     
} // namespace Plugin
} // namespace WPEFramework
