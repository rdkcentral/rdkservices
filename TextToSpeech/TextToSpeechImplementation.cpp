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

#include "TextToSpeechValidator.h"

#define TTS_MAJOR_VERSION 1
#define TTS_MINOR_VERSION 0

#define GET_STR(map, key, def) ((map.HasLabel(key) && !map[key].String().empty() && map[key].String() != "null") ? map[key].String() : def)

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

        InputValidation::Instance().setLogger([] (const char *log) { TTSLOG_WARNING(log); });
        InputValidation::Instance().addValidator("ttsendpoint", ExpectedValues<std::string>("^https?://[a-zA-Z0-9]+.*"));
        InputValidation::Instance().addValidator("ttsendpointsecured", ExpectedValues<std::string>("^https://[a-zA-Z0-9]+.*"));
        InputValidation::Instance().addValidator("double_str", ExpectedValues<std::string>("^-?[0-9]+(\\.[0-9]+)?"));
        InputValidation::Instance().addValidator("speechid", ExpectedValues<uint32_t>(1, UINT32_MAX));
        InputValidation::Instance().addValidator("speechrate", ExpectedValues<std::string>({"slow", "medium", "fast", "faster", "fastest"}));
        InputValidation::Instance().addValidator("rate", ExpectedValues<uint8_t>(0, 100));
        InputValidation::Instance().addValidator("volume", ExpectedValues<uint8_t>(0, 100));
        InputValidation::Instance().addValidator("primvolduckpercent", ExpectedValues<std::string>("^-?[0-9]+$"));
        InputValidation::Instance().addValidator("setPrimaryVolDuck", ExpectedValues<uint8_t>(0, 100));

        JsonObject config;
        config.FromString(service->ConfigLine());

        TTS::TTSConfiguration *ttsConfig = _ttsManager->configuration();
        ttsConfig->setEndPoint(GET_STR(config, "endpoint", ""));
        ttsConfig->setSecureEndPoint(GET_STR(config, "secureendpoint", ""));
        ttsConfig->setLocalEndPoint(GET_STR(config, "localendpoint", ""));
        ttsConfig->setLanguage(GET_STR(config, "language", "en-US"));
        ttsConfig->setVoice(GET_STR(config, "voice", ""));
        ttsConfig->setEndpointType(GET_STR(config, "endpoint_type", "TTS1"));
        ttsConfig->setSpeechRate(GET_STR(config, "speechrate", ""));
        ttsConfig->setVolume(std::stod(GET_STR(config, "volume", "100")));
        ttsConfig->setRate(std::stoi(GET_STR(config, "rate", "50")));
        ttsConfig->setPrimVolDuck(std::stoi(GET_STR(config,"primvolduckpercent", "25")));
        ttsConfig->setSATPluginCallsign(GET_STR(config, "satplugincallsign", ""));

        std::set<std::string> expectedLanguageSet;
        std::set<std::string> expectedVoicesSet;

        expectedLanguageSet.insert("*");

        if(config.HasLabel("voices")) {
            JsonObject voices = config["voices"].Object();
            for(JsonObject::Iterator it = voices.Variants(); it.Next(); ) {
                ttsConfig->m_others["voice_for_" + string(it.Label())] = it.Current().String();
                expectedLanguageSet.insert(toLower(string(it.Label())));
                expectedVoicesSet.insert(toLower(it.Current().String()));
            }

            if(!config.HasLabel("voice"))
                ttsConfig->setVoice(ttsConfig->voice());
        } else {
            TTSLOG_WARNING("Doesn't find default voice configuration");
        }

        if(config.HasLabel("local_voices")) {
            JsonObject voices = config["local_voices"].Object();
            for(JsonObject::Iterator it = voices.Variants(); it.Next(); ) {
                ttsConfig->m_others["voice_for_local_" + string(it.Label())] = it.Current().String();
                expectedLanguageSet.insert(toLower(string(it.Label())));
                expectedVoicesSet.insert(toLower(it.Current().String()));
            }
        }

#ifndef UNIT_TESTING
        InputValidation::Instance().addValidator("language", ExpectedValues<std::string>(expectedLanguageSet));
        InputValidation::Instance().addValidator("voice", ExpectedValues<std::string>(expectedVoicesSet));
#else
        InputValidation::Instance().addValidator("language", ExpectedValues<std::string>(expectedLanguageSetCollection));
        InputValidation::Instance().addValidator("voice", ExpectedValues<std::string>(expectedVoicesSetCollection));
#endif

        ttsConfig->loadFromConfigStore();
        TTSLOG_INFO("TTSEndPoint : %s", ttsConfig->endPoint().c_str());
        TTSLOG_INFO("SecureTTSEndPoint : %s", ttsConfig->secureEndPoint().c_str());
        TTSLOG_INFO("LocalTTSEndPoint : %s", ttsConfig->localEndPoint().c_str());
        TTSLOG_INFO("Language : %s", ttsConfig->language().c_str());
        TTSLOG_INFO("Voice : %s", ttsConfig->voice().c_str());
        TTSLOG_INFO("Volume : %lf", ttsConfig->volume());
        TTSLOG_INFO("Speech Rate : %s", ttsConfig->speechRate().c_str());
        TTSLOG_INFO("Rate : %u", ttsConfig->rate());
        TTSLOG_INFO("PrimaryVolumeDuck percentage : %d", ttsConfig->primVolDuck());
        TTSLOG_INFO("TTS is %s", ttsConfig->enabled()? "Enabled" : "Disabled");

        auto it = ttsConfig->m_others.begin();
        while( it != ttsConfig->m_others.end()) {
            TTSLOG_INFO("%s : %s", it->first.c_str(), it->second.c_str());
            ++it;
        }

        if(ttsConfig->hasValidLocalEndpoint()) {
            TTSLOG_INFO("Online/offline endpoint switch enabled");
            std::vector<std::string> localVoices;
            _ttsManager->listLocalVoices(ttsConfig->language(), localVoices);
            if(localVoices.empty()) {
              TTSLOG_WARNING("Local Voice is empty and no voices are defined for the specified language ('%s')!!!", ttsConfig->language().c_str());
              return TTS::TTS_FAIL;
            } else {
              ttsConfig->setLocalVoice(localVoices.front());
            }
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

        if (index != _notificationClients.end()) {
            (*index)->Release();
            _notificationClients.erase(index);
            TRACE_L1("Unregistered a sink on the browser %p", sink);
        }
         
        //callsign based unregister
        std::map<string,Exchange::ITextToSpeech::INotification*>::iterator callsignIndex;
        for (callsignIndex = _notificationCallsignClients.begin(); callsignIndex != _notificationCallsignClients.end(); ++callsignIndex)
        {
           if (callsignIndex->second == sink)
           {
               (callsignIndex->second)->Release();
               _notificationCallsignClients.erase(callsignIndex);
               TRACE_L1("Unregistered a sink on the browser %p", sink);
           }
        }

        _adminLock.Unlock();
    }

    void TextToSpeechImplementation::RegisterWithCallsign(const string callsign,Exchange::ITextToSpeech::INotification* sink)
    {
        _adminLock.Lock();
        TTSLOG_INFO("TTS thunder RegisterWithCallsign %s\n",callsign);

        _notificationCallsignClients[callsign] = sink;

        sink->AddRef();

        _adminLock.Unlock();

        TRACE_L1("Registered a sink on the browser %p", sink);
    }

    uint32_t TextToSpeechImplementation::Request(PluginHost::IStateControl::command command)
    {
        // Not implemented
        return Core::ERROR_GENERAL;
    }

    uint32_t TextToSpeechImplementation::Enable(const bool enable)
    {
        CHECK_TTS_MANAGER_RETURN_ON_FAIL();

        _adminLock.Lock();
        auto status = _ttsManager->enableTTS(enable);
        _adminLock.Unlock();

        TTSLOG_INFO("Enable TTS %s", enable ? "Enabled" : "Disabled");
        logResponse(status);
        return (status == TTS::TTS_OK) ? (Core::ERROR_NONE) : (Core::ERROR_GENERAL);
    }

    uint32_t TextToSpeechImplementation::ListVoices(const string language,RPC::IStringIterator*& voices) const
    {
        CHECK_TTS_MANAGER_RETURN_ON_FAIL();
        std::vector<std::string> voice;
        auto status = TTS::TTS_FAIL;

        if(InputValidation::Instance().validate("language", toLower(language))) {
            _adminLock.Lock();
            status = _ttsManager->listVoices(language, voice);
            _adminLock.Unlock();
        }

        voices = (Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(voice));
        logResponse(status);
        return (status == TTS::TTS_OK) ? (Core::ERROR_NONE) : (Core::ERROR_GENERAL);
    }

    uint32_t TextToSpeechImplementation::SetConfiguration(const Exchange::ITextToSpeech::Configuration &object,Exchange::ITextToSpeech::TTSErrorDetail &ttsStatus)
    {
        ttsStatus = (Exchange::ITextToSpeech::TTSErrorDetail) Exchange::ITextToSpeech::TTS_INVALID_CONFIGURATION;

        if((!object.ttsEndPoint.empty() && !InputValidation::Instance().validate("ttsendpoint", object.ttsEndPoint))
        || (!object.ttsEndPointSecured.empty() && !InputValidation::Instance().validate("ttsendpointsecured", object.ttsEndPointSecured))
        || (!object.speechRate.empty() && !InputValidation::Instance().validate("speechrate", object.speechRate))
        || (!object.language.empty() && !InputValidation::Instance().validate("language", toLower(object.language)))
        || (!object.voice.empty() && !InputValidation::Instance().validate("voice", toLower(object.voice)))
        || (!InputValidation::Instance().validate("rate", object.rate))
        || (!InputValidation::Instance().validate("volume", object.volume))) {
            TTSLOG_WARNING("Input configuration(s) are invalid");
            return Core::ERROR_GENERAL;
        }

        TTS::Configuration config;
        config.ttsEndPoint = object.ttsEndPoint;
        config.ttsEndPointSecured = object.ttsEndPointSecured;
        config.language =  object.language;
        config.voice =  object.voice;
        config.speechRate = object.speechRate;
        config.volume = (double) object.volume;
        config.rate =  object.rate;

        _adminLock.Lock();
        auto status = _ttsManager->setConfiguration(config);
        ttsStatus = (Exchange::ITextToSpeech::TTSErrorDetail) status;
        _adminLock.Unlock();

        TTSLOG_INFO("Set Configuration invoked\n");
        TTSLOG_INFO("TTSEndPoint : %s", config.ttsEndPoint.c_str());
        TTSLOG_INFO("SecureTTSEndPoint : %s", config.ttsEndPointSecured.c_str());
        TTSLOG_INFO("Language : %s", config.language.c_str());
        TTSLOG_INFO("Voice : %s", config.voice.c_str());
        TTSLOG_INFO("Volume : %lf",config.volume);
        TTSLOG_INFO("Rate : %u", config.rate);
        TTSLOG_INFO("SpeechRate : %s", config.speechRate.c_str());

        logResponse(status);
        return (status == TTS::TTS_OK) ? (Core::ERROR_NONE) : (Core::ERROR_GENERAL);
    }

    uint32_t TextToSpeechImplementation::SetFallbackText(const string scenario,const string value)
    {
        FallbackData data;
        data.scenario = scenario;
        data.value = value;
        data.path ="";

        _adminLock.Lock();
        _ttsManager->setFallbackText(data);
        _adminLock.Unlock();

        TTSLOG_INFO("Fallback text updated scenario %s.. value %s\n",scenario.c_str(),value.c_str());
        return (Core::ERROR_NONE);
    }

    uint32_t TextToSpeechImplementation::SetPrimaryVolDuck(const uint8_t prim)
    {
        if(!InputValidation::Instance().validate("setPrimaryVolDuck", prim))
            return Core::ERROR_GENERAL;

        _adminLock.Lock();
        _ttsManager->setPrimaryVolDuck((int8_t)prim);
        _adminLock.Unlock();

        TTSLOG_INFO("prim volume duck updated %d\n",prim);
        return (Core::ERROR_NONE);
    }

    uint32_t TextToSpeechImplementation::SetAPIKey(const string apikey)
    {
        _adminLock.Lock();
        _ttsManager->setAPIKey(apikey);
        _adminLock.Unlock();

        TTSLOG_INFO("api key updated\n");
        return (Core::ERROR_NONE);
    }

    uint32_t TextToSpeechImplementation::SetACL(const string method,const string apps)
    {
        CHECK_TTS_MANAGER_RETURN_ON_FAIL();

        if(method != "speak")
            return Core::ERROR_GENERAL;

        if(apps.empty() || apps.c_str()[0] == ' ' || apps == "NULL")
           return Core::ERROR_GENERAL;

        _adminLock.Lock();
        auto status = _ttsManager->setACL(method,apps);
        _adminLock.Unlock();

        TTSLOG_INFO("setACL invoked with method %s and app list %s",method.c_str(),apps.c_str());
        logResponse(status);
        return (status == TTS::TTS_OK) ? (Core::ERROR_NONE) : (Core::ERROR_GENERAL);
    }
    
    uint32_t TextToSpeechImplementation::GetConfiguration(Exchange::ITextToSpeech::Configuration &exchangeConfig) const
    {
        TTS::Configuration ttsConfig;

        _adminLock.Lock();
        auto status = _ttsManager->getConfiguration(ttsConfig);
        _adminLock.Unlock();

        if(status == TTS::TTS_OK) {
            exchangeConfig.ttsEndPoint    = ttsConfig.ttsEndPoint;
            exchangeConfig.ttsEndPointSecured = ttsConfig.ttsEndPointSecured;
            exchangeConfig.language           = ttsConfig.language;
            exchangeConfig.voice              = ttsConfig.voice;
            exchangeConfig.speechRate         = ttsConfig.speechRate;
            exchangeConfig.rate               = (uint8_t) ttsConfig.rate;
            exchangeConfig.volume             = (uint8_t) ttsConfig.volume;
        }

        TTSLOG_INFO("Get Configuration invoked\n");
        TTSLOG_INFO("TTSEndPoint : %s",  ttsConfig.ttsEndPoint.c_str());
        TTSLOG_INFO("SecureTTSEndPoint : %s", ttsConfig.ttsEndPointSecured.c_str());
        TTSLOG_INFO("Language : %s",  ttsConfig.language.c_str());
        TTSLOG_INFO("Voice : %s",  ttsConfig.voice.c_str());
        TTSLOG_INFO("Volume : %lf", ttsConfig.volume);
        TTSLOG_INFO("Rate : %u",  ttsConfig.rate);
        TTSLOG_INFO("SpeechRate : %s",  ttsConfig.speechRate.c_str());

        logResponse(status);
        return (status == TTS::TTS_OK) ? (Core::ERROR_NONE) : (Core::ERROR_GENERAL);
    }

    uint32_t TextToSpeechImplementation::Enable(bool &enable) const
    {
        CHECK_TTS_MANAGER_RETURN_ON_FAIL();

        _adminLock.Lock();
        enable = _ttsManager->isTTSEnabled();
        _adminLock.Unlock();

        TTSLOG_INFO("Is TTS Enabled %s", enable ? "Enabled" : "Disabled");
        return (Core::ERROR_NONE);
    }

    uint32_t nextSpeechId() {
        static uint32_t counter = 0;

        if(counter >= 0xFFFFFFFF)
            counter = 0;

        return ++counter;
    }

    uint32_t TextToSpeechImplementation::Speak(const string callsign,const string text,uint32_t &speechid,Exchange::ITextToSpeech::TTSErrorDetail &ttsStatus)
    {
        CHECK_TTS_MANAGER_RETURN_ON_FAIL();

        _adminLock.Lock();
        speechid = nextSpeechId();
        auto status = _ttsManager->speak(speechid,callsign,text);
        ttsStatus = (Exchange::ITextToSpeech::TTSErrorDetail) status;
        _adminLock.Unlock();

        if(status != TTS::TTS_OK)
            speechid = -1;

        TTSLOG_INFO("Speak invoked with text %s and speech id returned %d\n",text.c_str(),speechid);
        logResponse(status);
        return (status == TTS::TTS_OK) ? (Core::ERROR_NONE) : (Core::ERROR_GENERAL);
    }

    uint32_t TextToSpeechImplementation::Cancel(const uint32_t speechid)
    {
        CHECK_TTS_MANAGER_RETURN_ON_FAIL();
        auto status = TTS::TTS_FAIL;

        if(InputValidation::Instance().validate("speechid",speechid)) {
            _adminLock.Lock();
            status = _ttsManager->shut(speechid);
            _adminLock.Unlock();
        }

        TTSLOG_INFO("Cancel invoked with speechid %d",speechid);
        logResponse(status);
        return (status == TTS::TTS_OK) ? (Core::ERROR_NONE) : (Core::ERROR_GENERAL);
    }

    uint32_t TextToSpeechImplementation::Pause(const uint32_t speechid,Exchange::ITextToSpeech::TTSErrorDetail &ttsStatus)
    {
        CHECK_TTS_MANAGER_RETURN_ON_FAIL();

        _adminLock.Lock();
        auto status =  _ttsManager->pause(speechid);
        ttsStatus = (Exchange::ITextToSpeech::TTSErrorDetail) status;
        _adminLock.Unlock();

        TTSLOG_INFO("Pause invoked with speechid %d",speechid);
        logResponse(status);
        return (status == TTS::TTS_OK) ? (Core::ERROR_NONE) : (Core::ERROR_GENERAL);
    }

    uint32_t TextToSpeechImplementation::Resume(const uint32_t speechid,Exchange::ITextToSpeech::TTSErrorDetail &ttsStatus)
    {
        CHECK_TTS_MANAGER_RETURN_ON_FAIL();

        _adminLock.Lock();
        auto status = _ttsManager->resume(speechid);
        ttsStatus = (Exchange::ITextToSpeech::TTSErrorDetail) status;
        _adminLock.Unlock();

        TTSLOG_INFO("Resume invoked with speechid %d",speechid);
        logResponse(status);
        return (status == TTS::TTS_OK) ? (Core::ERROR_NONE) : (Core::ERROR_GENERAL);
    }


    uint32_t TextToSpeechImplementation::GetSpeechState(const  uint32_t speechid,Exchange::ITextToSpeech::SpeechState &estate)
    {
        CHECK_TTS_MANAGER_RETURN_ON_FAIL();
        TTS::SpeechState state;
        auto status = TTS::TTS_FAIL;

        if(InputValidation::Instance().validate("speechid", speechid)) {
            _adminLock.Lock();
            status = _ttsManager->getSpeechState(speechid, state);
            _adminLock.Unlock();
        }

        if(status == TTS::TTS_OK)
            estate = (Exchange::ITextToSpeech::SpeechState) state;

        logResponse(status);
        return (status == TTS::TTS_OK) ? (Core::ERROR_NONE) : (Core::ERROR_GENERAL);
    }

    void TextToSpeechImplementation::dispatchEvent(Event event, string callsign, const JsonValue &params)
    {
        Core::IWorkerPool::Instance().Submit(Job::Create(this, event, callsign, params));
    }

    void TextToSpeechImplementation::Dispatch(Event event, string callsign, const JsonValue params)
    {
        _adminLock.Lock();

        std::list<Exchange::ITextToSpeech::INotification*>::iterator index(_notificationClients.begin());

        while (index != _notificationClients.end()) {
            switch(event) {
                case STATE_CHANGED:     (*index)->Enabled(params.Boolean()); break;
                case VOICE_CHANGED:     (*index)->VoiceChanged(params.String()); break;
                case WILL_SPEAK:        (*index)->WillSpeak(params.Number()); break;
                case SPEECH_START:      (*index)->SpeechStart(params.Number()); break;
                case SPEECH_PAUSE:      (*index)->SpeechPause(params.Number()); break;
                case SPEECH_RESUME:     (*index)->SpeechResume(params.Number()); break;
                case SPEECH_INTERRUPT:  (*index)->SpeechInterrupted(params.Number()); break;
                case NETWORK_ERROR:     (*index)->NetworkError(params.Number()); break;
                case PLAYBACK_ERROR:    (*index)->PlaybackError(params.Number()); break;
                case SPEECH_COMPLETE:   (*index)->SpeechComplete(params.Number()); break;
                default: break;
            }
            ++index;
        }

        //notification to client with callsign
        std::map<string,Exchange::ITextToSpeech::INotification*>::iterator callsignindex(_notificationCallsignClients.find(callsign));
        if (callsignindex != _notificationCallsignClients.end()) {
             TTSLOG_INFO("Delivering event to callsign %s \n", (callsignindex->first).c_str());
             switch(event) {
                case WILL_SPEAK:        (callsignindex->second)->WillSpeak(params.Number()); break;
                case SPEECH_START:      (callsignindex->second)->SpeechStart(params.Number()); break;
                case SPEECH_PAUSE:      (callsignindex->second)->SpeechPause(params.Number()); break;
                case SPEECH_RESUME:     (callsignindex->second)->SpeechResume(params.Number()); break;
                case SPEECH_INTERRUPT:  (callsignindex->second)->SpeechInterrupted(params.Number()); break;
                case NETWORK_ERROR:     (callsignindex->second)->NetworkError(params.Number()); break;
                case PLAYBACK_ERROR:    (callsignindex->second)->PlaybackError(params.Number()); break;
                case SPEECH_COMPLETE:   (callsignindex->second)->SpeechComplete(params.Number()); break;
                default: break;            
             }
         }

        _adminLock.Unlock();

    }

    void TextToSpeechImplementation::onTTSStateChanged(bool state)
    {
        TTSLOG_INFO("Notify onttsstatechanged, state: %s", (state ? "true" : "false"));
        dispatchEvent(STATE_CHANGED, " ", JsonValue((bool)state));
    }

    void TextToSpeechImplementation::onVoiceChanged(std::string voice)
    {
        TTSLOG_INFO("Notify onvoicechanged, voice: %s", voice.c_str());
        dispatchEvent(VOICE_CHANGED, " ", JsonValue((std::string)voice));
    }

    void TextToSpeechImplementation::onWillSpeak(TTS::SpeechData &data)
    {
        TTSLOG_INFO("Notify onwillspeak, speechId: %d", data.id);
        dispatchEvent(WILL_SPEAK, data.callsign, JsonValue((int)data.id));
    }

    void TextToSpeechImplementation::onSpeechStart(TTS::SpeechData &data)
    {
        TTSLOG_INFO("Notify onspeechstart, speechId: %d", data.id);
        dispatchEvent(SPEECH_START, data.callsign, JsonValue((int)data.id));
    }

    void TextToSpeechImplementation::onSpeechPause(uint32_t speechId, string callsign)
    {
        TTSLOG_INFO("Notify onspeechpause, speechId: %d", speechId);
        dispatchEvent(SPEECH_PAUSE, callsign, JsonValue((int)speechId));
    }

    void TextToSpeechImplementation::onSpeechResume(uint32_t speechId, string callsign)
    {
        TTSLOG_INFO("Notify onspeechresume, speechId: %d", speechId);
        dispatchEvent(SPEECH_RESUME, callsign, JsonValue((int)speechId));
    }

    void TextToSpeechImplementation::onSpeechCancelled(std::vector<uint32_t> speechIds, string callsign)
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
        dispatchEvent(SPEECH_CANCEL, callsign, params);
    }

    void TextToSpeechImplementation::onSpeechInterrupted(uint32_t speechId, string callsign)
    {
        TTSLOG_INFO("Notify onspeechinterrupted, speechId: %d", speechId);
        dispatchEvent(SPEECH_INTERRUPT, callsign, JsonValue((int)speechId));
    }

    void TextToSpeechImplementation::onNetworkError(uint32_t speechId, string callsign)
    {
        TTSLOG_INFO("Notify onnetworkerror, speechId: %d", speechId);
        dispatchEvent(NETWORK_ERROR, callsign, JsonValue((int)speechId));
    }

    void TextToSpeechImplementation::onPlaybackError(uint32_t speechId, string callsign)
    {
        TTSLOG_INFO("Notify onplaybackerror, speechId: %d", speechId);
        dispatchEvent(PLAYBACK_ERROR, callsign, JsonValue((int)speechId));
    }

    void TextToSpeechImplementation::onSpeechComplete(TTS::SpeechData &data)
    {
        TTSLOG_INFO("Notify onspeechcomplete, speechId: %d", data.id);
        dispatchEvent(SPEECH_COMPLETE, data.callsign, JsonValue((int)data.id));
    }

    void logResponse(TTS::TTS_Error X)
    {
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
                ttsConfig.setSpeechRate(GET_STR(config,"speechrate",""));
                ttsConfig.setPrimVolDuck(static_cast<int8_t>(std::stoi(GET_STR(config,"primvolduckpercent","25"))));
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
        config["speechrate"] = ttsConfig.speechRate();
        config["primvolduckpercent"] = std::to_string(ttsConfig.primVolDuck());
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
