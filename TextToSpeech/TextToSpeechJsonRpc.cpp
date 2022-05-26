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
            string params, result;
            parameters.ToString(params);
            uint32_t ret= _tts->Enable(params, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }

    uint32_t TextToSpeech::ListVoices(const JsonObject& parameters, JsonObject& response)
    {
        if(_tts) {
            string params, result;
            parameters.ToString(params);
            uint32_t ret = _tts->ListVoices(params, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }

    uint32_t TextToSpeech::SetConfiguration(const JsonObject& parameters, JsonObject& response)
    {
        if(_tts) {
            string params, result;
            parameters.ToString(params);
            uint32_t ret= _tts->SetConfiguration(params, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }

    uint32_t TextToSpeech::GetConfiguration(const JsonObject& parameters, JsonObject& response)
    {
        if(_tts) {
            string result;
            uint32_t ret = _tts->GetConfiguration(result, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }

    uint32_t TextToSpeech::IsEnabled(const JsonObject& parameters ,JsonObject& response)
    {
        if(_tts) {
            string result;
            uint32_t ret = _tts->IsEnabled(result, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }

    uint32_t TextToSpeech::Speak(const JsonObject& parameters, JsonObject& response)
    {
        std::string callsign = parameters["callsign"].String();
        // if setACL() not called,  we ignore speak's callsign parameter
        if(!m_AclCalled || (m_AclCalled && HasAccess("speak",callsign)))
        {
            if(_tts) {
                string params, result;
                parameters.ToString(params);
                uint32_t ret= _tts->Speak(params, result);
                response.FromString(result);
                return ret;
            }
        }
        TTSLOG_WARNING("No Speak access for callsign %s\n",callsign.c_str());
        response["speechid"] = (int) -1;
        response["TTS_Status"] = static_cast<uint32_t>(TTS::TTS_NO_ACCESS);
        returnResponse(false);
    }

    uint32_t TextToSpeech::Cancel(const JsonObject& parameters, JsonObject& response)
    {
        if(_tts) {
            string params, result;
            parameters.ToString(params);
            uint32_t ret= _tts->Cancel(params, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }


    uint32_t TextToSpeech::Pause(const JsonObject& parameters, JsonObject& response)
    {
        if(_tts) {
            string params, result;
            parameters.ToString(params);
            uint32_t ret= _tts->Pause(params, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }

    uint32_t TextToSpeech::Resume(const JsonObject& parameters, JsonObject& response)
    {
        if(_tts) {
            string params, result;
            parameters.ToString(params);
            uint32_t ret= _tts->Resume(params, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }

    uint32_t TextToSpeech::IsSpeaking(const JsonObject& parameters, JsonObject& response)
    {
        if(_tts) {
            string params, result;
            parameters.ToString(params);
            uint32_t ret = _tts->IsSpeaking(params, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }

    uint32_t TextToSpeech::GetSpeechState(const JsonObject& parameters, JsonObject& response)
    {
        if(_tts) {
            string params, result;
            parameters.ToString(params);
            uint32_t ret = _tts->GetSpeechState(params, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }

    uint32_t TextToSpeech::getapiversion(const JsonObject& parameters, JsonObject& response)
    {
        UNUSED(parameters);

        response["version"] = _apiVersionNumber;

        returnResponse(true);
    }

    void TextToSpeech::dispatchJsonEvent(const char *event, const string &data)
    {
        TTSLOG_WARNING("Notify %s %s", event, data.c_str());
        JsonObject params;
        params.FromString(data);
        Notify(event, params);
    }

} // namespace Plugin
} // namespace WPEFramework
