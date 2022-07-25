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

#include "TextToSpeechImplementation.h"
#include <sys/prctl.h>
#include "UtilsJsonRpc.h"
#include <mutex>

#define TTS_MAJOR_VERSION 1
#define TTS_MINOR_VERSION 0

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

    SERVICE_REGISTRATION(TextToSpeechImplementation, TTS_MAJOR_VERSION, TTS_MINOR_VERSION);

    TTS::TTSManager* TextToSpeechImplementation::_ttsManager = NULL;

    TextToSpeechImplementation::TextToSpeechImplementation() : _adminLock()
    {
        if(!_ttsManager)
            _ttsManager = TTS::TTSManager::create(this);
    }

    TextToSpeechImplementation::~TextToSpeechImplementation()
    {
        if(_ttsManager) {
            delete _ttsManager;
            _ttsManager = NULL;
        }
    }

    uint32_t TextToSpeechImplementation::Configure(PluginHost::IShell* service)
    {
        if(!_ttsManager)
            return 0;

        JsonObject config;
        config.FromString(service->ConfigLine());

        TTS::TTSConfiguration *ttsConfig = _ttsManager->configuration();
        ttsConfig->setEndPoint(GET_STR(config, "endpoint", ""));
        ttsConfig->setSecureEndPoint(GET_STR(config, "secureendpoint", ""));
        ttsConfig->setLanguage(GET_STR(config, "language", "en-US"));
        ttsConfig->setVoice(GET_STR(config, "voice", ""));
        ttsConfig->setVolume(std::stod(GET_STR(config, "volume", "100")));
        ttsConfig->setRate(std::stoi(GET_STR(config, "rate", "50")));

        if(config.HasLabel("voices")) {
            JsonObject voices = config["voices"].Object();
            JsonObject::Iterator it = voices.Variants();
            while(it.Next())
                ttsConfig->m_others["voice_for_" + string(it.Label())] = it.Current().String();

            if(!config.HasLabel("voice"))
                ttsConfig->setVoice(ttsConfig->voice());
        } else {
            TTSLOG_WARNING("Doesn't find default voice configuration");
        }
        ttsConfig->loadFromConfigStore();
        TTSLOG_INFO("TTSEndPoint : %s", ttsConfig->endPoint().c_str());
        TTSLOG_INFO("SecureTTSEndPoint : %s", ttsConfig->secureEndPoint().c_str());
        TTSLOG_INFO("Language : %s", ttsConfig->language().c_str());
        TTSLOG_INFO("Voice : %s", ttsConfig->voice().c_str());
        TTSLOG_INFO("Volume : %lf", ttsConfig->volume());
        TTSLOG_INFO("Rate : %u", ttsConfig->rate());
        TTSLOG_INFO("TTS is %s", ttsConfig->enabled()? "Enabled" : "Disabled");

        auto it = ttsConfig->m_others.begin();
        while( it != ttsConfig->m_others.end()) {
            TTSLOG_INFO("%s : %s", it->first.c_str(), it->second.c_str());
            ++it;
        }

        _ttsManager->enableTTS(ttsConfig->enabled());
        return 0;
    }

    void TextToSpeechImplementation::Register(Exchange::ITextToSpeech::INotification* sink)
    {
        _adminLock.Lock();

        // Make sure a sink is not registered multiple times.
        ASSERT(std::find(_notificationClients.begin(), _notificationClients.end(), sink) == _notificationClients.end());

        _notificationClients.push_back(sink);
        sink->AddRef();

        _adminLock.Unlock();

        TRACE_L1("Registered a sink on the browser %p", sink);
    }

    void TextToSpeechImplementation::Unregister(Exchange::ITextToSpeech::INotification* sink)
    {
        _adminLock.Lock();

        std::list<Exchange::ITextToSpeech::INotification*>::iterator index(std::find(_notificationClients.begin(), _notificationClients.end(), sink));

        // Make sure you do not unregister something you did not register !!!
        ASSERT(index != _notificationClients.end());

        if (index != _notificationClients.end()) {
            (*index)->Release();
            _notificationClients.erase(index);
            TRACE_L1("Unregistered a sink on the browser %p", sink);
        }

        _adminLock.Unlock();
    }

    uint32_t TextToSpeechImplementation::Enable(const string &input, string &output)
    {
        CONVERT_PARAMETERS_TOJSON();
        CHECK_TTS_MANAGER_RETURN_ON_FAIL();
        CHECK_TTS_PARAMETER_RETURN_ON_FAIL("enabletts");

        _adminLock.Lock();

        auto status = _ttsManager->enableTTS(parameters["enabletts"].Boolean());

        _adminLock.Unlock();

        logResponse(status, response);
        returnResponse(status == TTS::TTS_OK);
    }

    uint32_t TextToSpeechImplementation::ListVoices(const string &input, string &output)
    {
        CONVERT_PARAMETERS_TOJSON();
        CHECK_TTS_MANAGER_RETURN_ON_FAIL();
        CHECK_TTS_PARAMETER_RETURN_ON_FAIL("language");

        _adminLock.Lock();

        std::vector<std::string> voice;
        auto status = _ttsManager->listVoices(parameters["language"].String(), voice);

        _adminLock.Unlock();

        if(status == TTS::TTS_OK)
            setResponseArray(response, "voices", voice);

        logResponse(status,response);
        returnResponse(status == TTS::TTS_OK);
    }

    uint32_t TextToSpeechImplementation::SetConfiguration(const string &input, string &output)
    {
        CONVERT_PARAMETERS_TOJSON();
        CHECK_TTS_MANAGER_RETURN_ON_FAIL();

        TTS::Configuration config;
        config.ttsEndPoint = GET_STR(parameters, "ttsendpoint", "");
        config.ttsEndPointSecured = GET_STR(parameters, "ttsendpointsecured", "");
        config.language = GET_STR(parameters, "language", "");
        config.voice = GET_STR(parameters, "voice", "");
        config.volume = std::stod(GET_STR(parameters, "volume", "0.0"));

        if(parameters.HasLabel("rate")) {
            int rate=0;
            getNumberParameter("rate", rate);
            config.rate = static_cast<uint8_t>(rate);
        }

        if(parameters.HasLabel("authinfo")) {
            JsonObject auth;
            auth = parameters["authinfo"].Object();
            if(((auth["type"].String()).compare("apikey")) == 0)
            {
                config.apiKey = GET_STR(auth,"value", "");
            }
        }

        if(parameters.HasLabel("fallbacktext")) {
            JsonObject fallback;
            fallback = parameters["fallbacktext"].Object();
            config.data.scenario = fallback["scenario"].String();
            config.data.value    = fallback["value"].String();
            config.data.path     = GET_STR(fallback,"path", "");
        }

        _adminLock.Lock();

        auto status = _ttsManager->setConfiguration(config);

        _adminLock.Unlock();

        logResponse(status, response);
        returnResponse(status == TTS::TTS_OK);
    }

    uint32_t TextToSpeechImplementation::GetConfiguration(const string &input, string &output)
    {
        CONVERT_PARAMETERS_TOJSON();
        CHECK_TTS_MANAGER_RETURN_ON_FAIL();

        _adminLock.Lock();

        TTS::Configuration ttsConfig;
        auto status = _ttsManager->getConfiguration(ttsConfig);

        _adminLock.Unlock();

        if(status == TTS::TTS_OK) {
            response["ttsendpoint"]         = ttsConfig.ttsEndPoint;
            response["ttsendpointsecured"]  = ttsConfig.ttsEndPointSecured;
            response["language"]            = ttsConfig.language;
            response["voice"]               = ttsConfig.voice;
            response["rate"]                = (int) ttsConfig.rate;
            response["volume"]              = std::to_string(ttsConfig.volume);
        }

        logResponse(status, response);
        returnResponse(status == TTS::TTS_OK);
    }

    uint32_t TextToSpeechImplementation::IsEnabled(const string &input, string &output)
    {
        CONVERT_PARAMETERS_TOJSON();
        CHECK_TTS_MANAGER_RETURN_ON_FAIL();

        _adminLock.Lock();

        response["isenabled"] = JsonValue((bool)_ttsManager->isTTSEnabled());

        _adminLock.Unlock();

        logResponse(TTS::TTS_OK,response);
        returnResponse(true);
    }

    uint32_t nextSpeechId() {
        static uint32_t counter = 0;

        if(counter >= 0xFFFFFFFF)
            counter = 0;

        return ++counter;
    }

    uint32_t TextToSpeechImplementation::Speak(const string &input, string &output)
    {
        CONVERT_PARAMETERS_TOJSON();
        CHECK_TTS_MANAGER_RETURN_ON_FAIL();
        CHECK_TTS_PARAMETER_RETURN_ON_FAIL("text");

        _adminLock.Lock();

        uint32_t speechId = nextSpeechId();
        auto status = _ttsManager->speak(speechId, parameters["text"].String());

        _adminLock.Unlock();

        if(status == TTS::TTS_OK)
            response["speechid"] = (int) speechId;
        else
            response["speechid"] = (int) -1;

        logResponse(status, response);
        returnResponse(status == TTS::TTS_OK);
    }

    uint32_t TextToSpeechImplementation::Cancel(const string &input, string &output)
    {
        CONVERT_PARAMETERS_TOJSON();
        CHECK_TTS_MANAGER_RETURN_ON_FAIL();
        CHECK_TTS_PARAMETER_RETURN_ON_FAIL("speechid");

        _adminLock.Lock();

        auto status = _ttsManager->shut(parameters["speechid"].Number());

        _adminLock.Unlock();

        logResponse(status, response);
        returnResponse(status == TTS::TTS_OK);
    }


    uint32_t TextToSpeechImplementation::Pause(const string &input, string &output)
    {
        CONVERT_PARAMETERS_TOJSON();
        CHECK_TTS_MANAGER_RETURN_ON_FAIL();
        CHECK_TTS_PARAMETER_RETURN_ON_FAIL("speechid");

        _adminLock.Lock();

        auto status = _ttsManager->pause(parameters["speechid"].Number());

        _adminLock.Unlock();

        logResponse(status, response);
        returnResponse(status == TTS::TTS_OK);
    }

    uint32_t TextToSpeechImplementation::Resume(const string &input, string &output)
    {
        CONVERT_PARAMETERS_TOJSON();
        CHECK_TTS_MANAGER_RETURN_ON_FAIL();
        CHECK_TTS_PARAMETER_RETURN_ON_FAIL("speechid");

        _adminLock.Lock();

        auto status = _ttsManager->resume(parameters["speechid"].Number());

        _adminLock.Unlock();

        logResponse(status, response);
        returnResponse(status == TTS::TTS_OK);
    }

    uint32_t TextToSpeechImplementation::IsSpeaking(const string &input, string &output)
    {
        CONVERT_PARAMETERS_TOJSON();
        CHECK_TTS_MANAGER_RETURN_ON_FAIL();
        CHECK_TTS_PARAMETER_RETURN_ON_FAIL("speechid");

        _adminLock.Lock();

        bool speaking = false;
        auto status = _ttsManager->isSpeaking(parameters["speechid"].Number(), speaking);

        _adminLock.Unlock();

        response["speaking"] = speaking;
        logResponse(status, response);
        returnResponse(status == TTS::TTS_OK);
    }

    uint32_t TextToSpeechImplementation::GetSpeechState(const string &input, string &output)
    {
        CONVERT_PARAMETERS_TOJSON();
        CHECK_TTS_MANAGER_RETURN_ON_FAIL();
        CHECK_TTS_PARAMETER_RETURN_ON_FAIL("speechid");

        _adminLock.Lock();

        TTS::SpeechState state;
        auto status = _ttsManager->getSpeechState(parameters["speechid"].Number(), state);

        _adminLock.Unlock();

        if(status == TTS::TTS_OK)
            response["speechstate"] = (int) state;

        logResponse(status, response);
        returnResponse(status == TTS::TTS_OK);
    }

    void TextToSpeechImplementation::setResponseArray(JsonObject& response, const char* key, const std::vector<std::string>& items)
    {
        JsonArray arr;
        for(auto& i : items) arr.Add(JsonValue(i));

        response[key] = arr;
    }

    void TextToSpeechImplementation::dispatchEvent(Event event, JsonObject &params)
    {
        string data;
        params.ToString(data);
        Core::IWorkerPool::Instance().Submit(Job::Create(this, event, data));
    }

    void TextToSpeechImplementation::Dispatch(Event event, string data)
    {
        _adminLock.Lock();

        std::list<Exchange::ITextToSpeech::INotification*>::iterator index(_notificationClients.begin());

        while (index != _notificationClients.end()) {
            switch(event) {
                case STATE_CHANGED:     (*index)->StateChanged(data); break;
                case VOICE_CHANGED:     (*index)->VoiceChanged(data); break;
                case WILL_SPEAK:        (*index)->WillSpeak(data); break;
                case SPEECH_START:      (*index)->SpeechStart(data); break;
                case SPEECH_PAUSE:      (*index)->SpeechPause(data); break;
                case SPEECH_RESUME:     (*index)->SpeechResume(data); break;
                case SPEECH_CANCEL:     (*index)->SpeechCancelled(data); break;
                case SPEECH_INTERRUPT:  (*index)->SpeechInterrupted(data); break;
                case NETWORK_ERROR:     (*index)->NetworkError(data); break;
                case PLAYBACK_ERROR:    (*index)->PlaybackError(data); break;
                case SPEECH_COMPLETE:   (*index)->SpeechComplete(data); break;
            }
            ++index;
        }

        _adminLock.Unlock();

    }

    void TextToSpeechImplementation::onTTSStateChanged(bool state)
    {
        JsonObject params;
        params["state"] = JsonValue((bool)state);
        dispatchEvent(STATE_CHANGED, params);
    }

    void TextToSpeechImplementation::onVoiceChanged(std::string voice)
    {
        JsonObject params;
        params["voice"] = voice;
        dispatchEvent(VOICE_CHANGED, params);
    }

    void TextToSpeechImplementation::onWillSpeak(TTS::SpeechData &data)
    {
        JsonObject params;
        params["speechid"]  = JsonValue((int)data.id);
        params["text"]      = data.text;
        dispatchEvent(WILL_SPEAK, params);
    }

    void TextToSpeechImplementation::onSpeechStart(TTS::SpeechData &data)
    {
        JsonObject params;
        params["speechid"]  = JsonValue((int)data.id);
        params["text"]      = data.text;
        dispatchEvent(SPEECH_START, params);
    }

    void TextToSpeechImplementation::onSpeechPause(uint32_t speechId)
    {
        JsonObject params;
        params["speechid"]  = JsonValue((int)speechId);
        dispatchEvent(SPEECH_PAUSE, params);
    }

    void TextToSpeechImplementation::onSpeechResume(uint32_t speechId)
    {
        JsonObject params;
        params["speechid"]  = JsonValue((int)speechId);
        dispatchEvent(SPEECH_RESUME, params);
    }

    void TextToSpeechImplementation::onSpeechCancelled(std::vector<uint32_t> speechIds)
    {
        std::stringstream ss;
        for(auto it = speechIds.begin(); it != speechIds.end(); ++it)
        {
            if(it != speechIds.begin())
                ss << ",";
            ss << *it;
        }
        JsonObject params;
        params["speechid"]  = ss.str();
        dispatchEvent(SPEECH_CANCEL, params);
    }

    void TextToSpeechImplementation::onSpeechInterrupted(uint32_t speechId)
    {
        JsonObject params;
        params["speechid"]  = JsonValue((int)speechId);
        dispatchEvent(SPEECH_INTERRUPT, params);
    }

    void TextToSpeechImplementation::onNetworkError(uint32_t speechId)
    {
        JsonObject params;
        params["speechid"]  = JsonValue((int)speechId);
        dispatchEvent(NETWORK_ERROR, params);
    }

    void TextToSpeechImplementation::onPlaybackError(uint32_t speechId)
    {
        JsonObject params;
        params["speechid"]  = JsonValue((int)speechId);
        dispatchEvent(PLAYBACK_ERROR, params);
    }

    void TextToSpeechImplementation::onSpeechComplete(TTS::SpeechData &data)
    {
        JsonObject params;
        params["speechid"]  = JsonValue((int)data.id);
        params["text"]      = data.text;
        dispatchEvent(SPEECH_COMPLETE, params);
    }

    void logResponse(TTS::TTS_Error X, JsonObject& response)
    {
        response["TTS_Status"] = static_cast<uint32_t>(X);
        switch (X){
            case (TTS::TTS_OK):
                TTSLOG_VERBOSE("%s api operational success with TTS_OK code = %d", __func__, X);
                break;
            case (TTS::TTS_FAIL):
                TTSLOG_ERROR("%s api failed with TTS_FAIL error code = %d", __func__, X);
                break;
            case (TTS::TTS_NOT_ENABLED):
                TTSLOG_ERROR("%s api failed with TTS_NOT_ENABLED error code = %d", __func__, X);
                break;
            case (TTS::TTS_INVALID_CONFIGURATION):
                TTSLOG_ERROR("%s api failed with TTS_INVALID_CONFIGURATION error code = %d", __func__, X);
                break;
            default:
                TTSLOG_ERROR("%s api failed with unknow error code = %d", __func__, X);
                response["TTS_Status"] = static_cast<uint32_t>(TTS::TTS_FAIL);
        }
    }

    bool _readFromFile(std::string filename, TTS::TTSConfiguration &ttsConfig)
    {
        Core::File file(filename);
        if(file.Open()) {
            JsonObject config;
            if(config.IElement::FromFile(file)) {
                Core::JSON::Boolean enabled = config.Get("enabled").Boolean();
                ttsConfig.setEnabled(enabled.Value());
                ttsConfig.setVolume(std::stod(GET_STR(config,"volume","0.0")));
                ttsConfig.setRate(static_cast<uint8_t>(std::stoi(GET_STR(config,"rate","0"))));
                ttsConfig.setVoice(GET_STR(config,"voice",""));
                ttsConfig.setLanguage(GET_STR(config,"language",""));
                if(config.HasLabel("fallbacktext")) {
                    JsonObject fallback;
                    FallbackData data;
                    fallback = config["fallbacktext"].Object();
                    data.scenario = fallback["scenario"].String();
                    data.value    = fallback["value"].String();
                    data.path     = fallback["path"].String();
                    ttsConfig.setFallBackText(data);
                }    
            return true;
            }
        file.Close();
        }
        return false;
    }   

    std::mutex fileMutex;
    bool _writeToFile(std::string filename, TTS::TTSConfiguration &ttsConfig)
    {
        std::lock_guard<std::mutex> lock(fileMutex);
        Core::File file(filename);
        JsonObject config;
        file.Create();
        config["enabled"] = JsonValue((bool)ttsConfig.enabled());
        config["volume"] = std::to_string(ttsConfig.volume());
        config["rate"] = std::to_string(ttsConfig.rate());
        config["voice"] = ttsConfig.voice();
        config["language"] = ttsConfig.language();
        if(ttsConfig.isFallbackEnabled())
        {
            JsonObject fallbackconfig;
            fallbackconfig["scenario"] = ttsConfig.getFallbackScenario();
            fallbackconfig["value"] = ttsConfig.getFallbackValue();
            fallbackconfig["path"] =ttsConfig.getFallbackPath();
            config["fallbacktext"] = fallbackconfig;
        }
        config.IElement::ToFile(file);
        fsync((int)file);
        file.Close();
        return true;
    }

} // namespace Plugin
} // namespace WPEFramework
