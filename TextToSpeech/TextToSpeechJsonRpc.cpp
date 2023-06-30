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

#include "TextToSpeech.h"
#include "UtilsJsonRpc.h"
#include "UtilsUnused.h"
#include "impl/TTSCommon.h"

#define GET_STR(map, key, def) ((map.HasLabel(key) && !map[key].String().empty() && map[key].String() != "null") ? map[key].String() : def)

namespace WPEFramework {
namespace Plugin {

    void TextToSpeech::RegisterAll()
    {
        Register("enabletts", &TextToSpeech::Enable, this);
        Register("listvoices", &TextToSpeech::ListVoices, this);
        Register("setttsconfiguration", &TextToSpeech::SetConfiguration, this);
        Register("getttsconfiguration", &TextToSpeech::GetConfiguration, this);
        Register("isttsenabled", &TextToSpeech::IsEnabled, this);
        Register("speak", &TextToSpeech::Speak, this);
        Register("cancel", &TextToSpeech::Cancel, this);
        Register("pause", &TextToSpeech::Pause, this);
        Register("resume", &TextToSpeech::Resume, this);
        Register("isspeaking", &TextToSpeech::IsSpeaking, this);
        Register("getspeechstate", &TextToSpeech::GetSpeechState, this);
        Register("setACL", &TextToSpeech::SetACL, this);
        Register("getapiversion", &TextToSpeech::getapiversion, this);
    }
    
    bool TextToSpeech::AddToAccessList(const string &key,const string &value)
    {
        std::map<std::string,std::string>::iterator itr;
        itr = m_AccessList.find(key);
        if (itr != m_AccessList.end())
        {  
            TTSLOG_INFO("method %s found in accesslist...replacing value with %s\n",key.c_str(),value.c_str());
            itr->second= value;
            return 1;
        }
        else
        {   
            TTSLOG_INFO("method %s not found..inserting to accesslist with value %s\n",key.c_str(),value.c_str());
            m_AccessList.insert(std::make_pair(key,value));
            return 0;
        }
    }   

    bool TextToSpeech::HasAccess(const string &method,string &app)
    {
        std::map<std::string,std::string>::iterator itr;
        itr = m_AccessList.find(method);
        if (itr != m_AccessList.end())
        {   
            string value = itr->second;
            string app_quote =  '\"' + app + '\"';//wrap it with double quote
            std::string::size_type pos = value.find(app_quote);
            TTSLOG_INFO("method %s found in accesslist and can be accessed by %s\n",method.c_str(),value.c_str());
            if(pos != std::string::npos)
            {
                TTSLOG_INFO("%s app has access to method  %s\n",app.c_str(),method.c_str());
                return true;
            }

            TTSLOG_WARNING("%s app does not have access to method  %s\n",app.c_str(),method.c_str());
            return false;
        }
        else
        {
            TTSLOG_WARNING("method :%s not found in accesslist\n",method.c_str());
            return false;
        }
    }

    uint32_t TextToSpeech::SetACL(const JsonObject& parameters, JsonObject& response)
    {
        CHECK_TTS_PARAMETER_RETURN_ON_FAIL("accesslist");
        TTSLOG_INFO("SetACL request:%s\n",parameters["accesslist"].String().c_str());
        JsonArray list = parameters["accesslist"].Array();
        JsonArray::Iterator it = list.Elements();
        while(it.Next())
        {
            JsonObject accesslist = it.Current().Object();
            if (accesslist.HasLabel("method") && accesslist.HasLabel("apps"))
            {
                m_AccessMutex.lock();
                AddToAccessList(accesslist["method"].String(),accesslist["apps"].String());
                m_AccessMutex.unlock();
            }
            else
            {
                TTSLOG_WARNING("SetACL wrong input parameters\n");
                returnResponse(false);
            }
        }
        m_AclCalled = true;
        returnResponse(true);
    }

    uint32_t TextToSpeech::Enable(const JsonObject& parameters, JsonObject& response)
    {
        if(_tts) {
            CHECK_TTS_PARAMETER_RETURN_ON_FAIL("enabletts");
            _tts->Enable(parameters["enabletts"].Boolean());
            response["TTS_Status"] = static_cast<uint32_t> (TTS::TTS_OK);
            returnResponse(true);
        }
        return Core::ERROR_NONE;
    }

    uint32_t TextToSpeech::ListVoices(const JsonObject& parameters, JsonObject& response)
    {
        if(_tts) {
            CHECK_TTS_PARAMETER_RETURN_ON_FAIL("language");
            RPC::IStringIterator* voices = nullptr;
            _tts->ListVoices(parameters["language"].String(),voices);
            JsonArray arr;
            string element;
            while (voices->Next(element) == true) {
                arr.Add(JsonValue(element));
            }
            response["voices"] = arr;
            response["TTS_Status"] = static_cast<uint32_t> (TTS::TTS_OK);
            voices->Release();
            returnResponse(true);
        }
        return Core::ERROR_NONE;
    }

    uint32_t TextToSpeech::GetConfiguration(const JsonObject& parameters, JsonObject& response)
    {
        if(_tts) {
            Exchange::ITextToSpeech::Configuration ttsConfig;
            _tts->GetConfiguration(ttsConfig);
            response["ttsendpoint"]         = ttsConfig.ttsEndPoint;
            response["ttsendpointsecured"]  = ttsConfig.ttsEndPointSecured;
            response["language"]            = ttsConfig.language;
            response["voice"]               = ttsConfig.voice;
            response["rate"]                = (int) ttsConfig.rate;
            response["volume"]              = std::to_string(ttsConfig.volume);
            response["TTS_Status"] = static_cast<uint32_t> (TTS::TTS_OK);
            returnResponse(true);
         }
         return Core::ERROR_NONE;

    }

    uint32_t TextToSpeech::SetConfiguration(const JsonObject& parameters, JsonObject& response)
    {
        if(_tts) {
        Exchange::ITextToSpeech::Configuration config;
        config.ttsEndPoint = GET_STR(parameters, "ttsendpoint", "");
        config.ttsEndPointSecured = GET_STR(parameters, "ttsendpointsecured", "");
        config.language = GET_STR(parameters, "language", "");
        config.voice = GET_STR(parameters, "voice", "");
        config.volume = (uint8_t) std::stod(GET_STR(parameters, "volume", "0.0"));

        if(parameters.HasLabel("rate")) {
            int rate=0;
            getNumberParameter("rate", rate);
            config.rate = static_cast<uint8_t>(rate);
        }

        if(parameters.HasLabel("authinfo")) {
            JsonObject auth;
            string apikey;
            auth = parameters["authinfo"].Object();
            if(((auth["type"].String()).compare("apikey")) == 0)
            {
                apikey = GET_STR(auth,"value", "");
                _tts->SetAPIKey(apikey);
            }
        }

        if(parameters.HasLabel("fallbacktext")) {
            JsonObject fallback;
            string scenario,value;
            fallback = parameters["fallbacktext"].Object();
            scenario = fallback["scenario"].String();
            value    = fallback["value"].String();
            _tts->SetFallbackText(scenario,value);
        }

        if(parameters.HasLabel("primvolduckpercent")) {
           int8_t  primVolDuck;
           primVolDuck = (int8_t) std::stoi(GET_STR(parameters,"primvolduckpercent","-1"));
           _tts->SetPrimaryVolDuck(primVolDuck);
        }
         Exchange::ITextToSpeech::TTSErrorDetail status;
         _tts->SetConfiguration(config,status);
         response["TTS_Status"] = static_cast<uint32_t>(status);
         returnResponse(status == Exchange::ITextToSpeech::TTSErrorDetail::TTS_OK);
     }
     return Core::ERROR_NONE;
}

    uint32_t TextToSpeech::IsEnabled(const JsonObject& parameters ,JsonObject& response)
    {
        if(_tts) {
             bool isenabled = false;
             static_cast<const WPEFramework::Exchange::ITextToSpeech*>(_tts)->Enable(isenabled);
             response["isenabled"] = isenabled;
             response["TTS_Status"] = static_cast<uint32_t>(TTS::TTS_OK);
             returnResponse(true);
        }
        return Core::ERROR_NONE;
    }

    uint32_t TextToSpeech::Speak(const JsonObject& parameters, JsonObject& response)
    {
        CHECK_TTS_PARAMETER_RETURN_ON_FAIL("text");
        std::string callsign = parameters["callsign"].String();
        // if setACL() not called,  we ignore speak's callsign parameter
        if(!m_AclCalled || (m_AclCalled && HasAccess("speak",callsign)))
        {
            if(_tts) {
                uint32_t speechid;
                Exchange::ITextToSpeech::TTSErrorDetail status;
                _tts->Speak(parameters["text"].String(),speechid,status);
                response["speechid"] = (int) speechid;
                response["TTS_Status"] = static_cast<uint32_t>(status);
                returnResponse(status ==  Exchange::ITextToSpeech::TTSErrorDetail::TTS_OK);
            }
        }
        TTSLOG_WARNING("No Speak access for callsign %s\n",callsign.c_str());
        response["speechid"] = (int) -1;
        response["TTS_Status"] = static_cast<uint32_t>(TTS::TTS_NO_ACCESS);
        returnResponse(false);
    }

    uint32_t TextToSpeech::Cancel(const JsonObject& parameters, JsonObject& response)
    {
        CHECK_TTS_PARAMETER_RETURN_ON_FAIL("speechid");
        if(_tts) {
            _tts->Cancel(parameters["speechid"].Number());
            response["TTS_Status"] = static_cast<uint32_t> (TTS::TTS_OK);
            returnResponse(true);
        }
        return Core::ERROR_NONE;
    }


    uint32_t TextToSpeech::Pause(const JsonObject& parameters, JsonObject& response)
    {
        CHECK_TTS_PARAMETER_RETURN_ON_FAIL("speechid");
        if(_tts) {
            Exchange::ITextToSpeech::TTSErrorDetail status;
            _tts->Pause(parameters["speechid"].Number(),status);
            response["TTS_Status"] = static_cast<uint32_t>(status);
            returnResponse(status ==  Exchange::ITextToSpeech::TTSErrorDetail::TTS_OK);
        }
        return Core::ERROR_NONE;
    }

    uint32_t TextToSpeech::Resume(const JsonObject& parameters, JsonObject& response)
    {
        CHECK_TTS_PARAMETER_RETURN_ON_FAIL("speechid");
        if(_tts) {
            Exchange::ITextToSpeech::TTSErrorDetail status;
            _tts->Resume(parameters["speechid"].Number(),status);
            response["TTS_Status"] = static_cast<uint32_t>(status);
            returnResponse(status == Exchange::ITextToSpeech::TTSErrorDetail::TTS_OK);
        }
        return Core::ERROR_NONE;
    }

    uint32_t TextToSpeech::IsSpeaking(const JsonObject& parameters, JsonObject& response)
    {
        if(_tts) {
            bool isspeaking = false;
            Exchange::ITextToSpeech::SpeechState state;
            _tts->GetSpeechState(parameters["speechid"].Number(),state);
            if(state == Exchange::ITextToSpeech::SpeechState::SPEECH_IN_PROGRESS)
            {
               isspeaking = true;
            }
            response["speaking"] = isspeaking;
            response["TTS_Status"] = static_cast<uint32_t> (TTS::TTS_OK);
            returnResponse(true);
        }
        return Core::ERROR_NONE;
    }

    uint32_t TextToSpeech::GetSpeechState(const JsonObject& parameters, JsonObject& response)
    {
        if(_tts) {
            Exchange::ITextToSpeech::SpeechState state;
            _tts->GetSpeechState(parameters["speechid"].Number(),state);
            response["speechstate"] = (int) state;
            response["TTS_Status"] = static_cast<uint32_t> (TTS::TTS_OK);
            returnResponse(true);
        }
        return Core::ERROR_NONE;
    }

    uint32_t TextToSpeech::getapiversion(const JsonObject& parameters, JsonObject& response)
    {
        UNUSED(parameters);

        response["version"] = _apiVersionNumber;

        returnResponse(true);
    }

} // namespace Plugin
} // namespace WPEFramework
