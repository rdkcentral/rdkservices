/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
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

#include "../impl/logger.h"
#include "../impl/TTSCommon.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sstream>
#include <iostream>
#include <fstream>
#include <map>

#include "Module.h"

#define TTSSRV_CALLSIGN "org.rdk.TextToSpeech"
#define SERVER_DETAILS  "127.0.0.1:9998"

using namespace std;
using namespace WPEFramework;

using namespace TTS;

volatile unsigned short g_connectedToTTSEventCount = 0;
volatile bool g_connectedToTTS = false;

#define OPT_ENABLE_TTS          1
#define OPT_VOICE_LIST          2
#define OPT_SET_CONFIG          3
#define OPT_GET_CONFIG          4
#define OPT_TTS_ENABLED         5
#define OPT_SESSION_ACTIVE      6
#define OPT_ACQUIRE_RESOURCE    7
#define OPT_CLAIM_RESOURCE      8
#define OPT_RELEASE_RESOURCE    9
#define OPT_CREATE_SESSION      10
#define OPT_IS_ACTIVE_SESSION   11
#define OPT_SET_PREEMPTIVE      12
#define OPT_REQ_EXT_EVENTS      13
#define OPT_SPEAK               14
#define OPT_PAUSE               15
#define OPT_RESUME              16
#define OPT_ABORT               17
#define OPT_IS_SPEAKING         18
#define OPT_SPEECH_STATE        19
#define OPT_DESTROY_SESSION     20
#define OPT_EXIT                21
#define OPT_BLOCK_TILL_INPUT    22
#define OPT_SLEEP               23

void showMenu()
{
    cout << "------------------------" << endl;
    cout << OPT_ENABLE_TTS          << ".enableTTS" << endl;
    cout << OPT_VOICE_LIST          << ".listVoices" << endl;
    cout << OPT_SET_CONFIG          << ".setTTSConfiguration" << endl;
    cout << OPT_GET_CONFIG          << ".getTTSConfiguration" << endl;
    cout << OPT_TTS_ENABLED         << ".isTTSEnabled" << endl;
    cout << OPT_SESSION_ACTIVE      << ".isSessionActiveForApp" << endl;
    cout << "-" << endl;
    cout << OPT_ACQUIRE_RESOURCE    << ".acquireResource" << endl;
    cout << OPT_CLAIM_RESOURCE      << ".claimResource" << endl;
    cout << OPT_RELEASE_RESOURCE    << ".releaseResource" << endl;
    cout << "-" << endl;
    cout << OPT_CREATE_SESSION      << ".createSession" << endl;
    cout << OPT_IS_ACTIVE_SESSION   << ".isActiveSession" << endl;
    cout << OPT_SET_PREEMPTIVE      << ".setPreemptiveSpeech" << endl;
    cout << OPT_REQ_EXT_EVENTS      << ".requestExtendedEvents" << endl;
    cout << OPT_SPEAK               << ".speak" << endl;
    cout << OPT_PAUSE               << ".pause" << endl;
    cout << OPT_RESUME              << ".resume" << endl;
    cout << OPT_ABORT               << ".abort" << endl;
    cout << OPT_IS_SPEAKING         << ".isSpeaking" << endl;
    cout << OPT_SPEECH_STATE        << ".getSpeechState" << endl;
    cout << OPT_DESTROY_SESSION     << ".destroySession" << endl;
    cout << OPT_EXIT                << ".exit" << endl;
    cout << OPT_BLOCK_TILL_INPUT    << ".dummyInput" << endl;
    cout << OPT_SLEEP               << ".sleep" << endl;
    cout << "------------------------" << endl;
}

struct AppInfo {
    AppInfo(uint32_t sid) : m_sessionId(sid) {}
    uint32_t m_sessionId;
};

struct MyStream {
    MyStream() : myfile(NULL), in(&cin) {
    }

    MyStream(string fname) : myfile(new ifstream(fname)) {
        if(myfile->is_open()) {
            cout << "Reading from file" << endl;
            in = myfile;
        } else {
            cout << "Reading from std::cin" << endl;
            in = &cin;
        }
    }

    template<class T>
    bool getInput(T &var, const char *prompt = NULL, bool console = false) {
        stringstream ss;
        static char cstr[5 * 1024];
        string str;

        istream *tin = in;
        if(console)
            in = &cin;

        do {
            if(prompt)
                cout << prompt;
            try {
                if(in == myfile && in->eof())
                    in = &cin;
                in->getline(cstr, sizeof(cstr)-1);
            } catch(...) {
                if(in == myfile)
                    in = &cin;
            }

            str = cstr;
            if(in == myfile)
                cout << cstr << endl;

            if((str.find('#') == string::npos || !str.erase(str.find('#')).empty()) && !str.empty()) {
                ss.str(str);
                ss >> var;
                break;
            }
        } while(1);
        in = tin;

        return true;
    }

private:
    ifstream *myfile;
    istream *in;
};

namespace Handlers {
    /* Event Handlers */
    static void onTTSStateChangedHandler(const JsonObject& params) {
        bool state = params["state"].Boolean();
        std::cout << "[TTSSrvEvt] " << __FUNCTION__ << ": " << state << std::endl;
    }
    static void onTTSSessionCreatedHandler(const JsonObject& params) {
        int appId     = params["appId"].Number();
        int sessionId = params["sessionId"].Number();
        std::cout << "[TTSSrvEvt] " << __FUNCTION__ << ":  appId: " << appId << "sessionId: "<< sessionId << std::endl;
    }
    static void onResourceAcquiredHandler(const JsonObject& params) {
        int appId     = params["appId"].Number();
        int sessionId = params["sessionId"].Number();
        std::cout << "[TTSSrvEvt] " << __FUNCTION__ << ":  appId: " << appId << "sessionId: "<< sessionId << std::endl;
    }
    static void onResourceReleasedHandler(const JsonObject& params) {
        int appId     = params["appId"].Number();
        int sessionId = params["sessionId"].Number();
        std::cout << "[TTSSrvEvt] " << __FUNCTION__ << ":  appId: " << appId << "sessionId: "<< sessionId << std::endl;
    }
    static void onWillSpeakHandler(const JsonObject& params) {
        int appId        = params["appId"].Number();
        int sessionId    = params["sessionId"].Number();
        int speechId     = params["speechId"].Number();
        std::string text = params["text"].String();
        std::cout << "[TTSSrvEvt] " << __FUNCTION__ << ":  appId: " << appId << "  sessionId: "<< sessionId << "  speechid: " << speechId << " text: " << text << std::endl;
    }
    static void onSpeechStartHandler(const JsonObject& params) {
        int appId        = params["appId"].Number();
        int sessionId    = params["sessionId"].Number();
        int speechId     = params["speechId"].Number();
        std::string text = params["text"].String();
        std::cout << "[TTSSrvEvt] " << __FUNCTION__ << ":  appId: " << appId << "  sessionId: "<< sessionId << "  speechid: " << speechId << " text: " << text << std::endl;
    }
    static void onSpeechPauseHandler(const JsonObject& params) {
        int appId        = params["appId"].Number();
        int sessionId    = params["sessionId"].Number();
        int speechId     = params["speechId"].Number();
        std::cout << "[TTSSrvEvt] " << __FUNCTION__ << ":  appId: " << appId << "  sessionId: "<< sessionId << "  speechid: " << speechId << std::endl;
    }
    static void onSpeechResumeHandler(const JsonObject& params) {
        int appId        = params["appId"].Number();
        int sessionId    = params["sessionId"].Number();
        int speechId     = params["speechId"].Number();
        std::cout << "[TTSSrvEvt] " << __FUNCTION__ << ":  appId: " << appId << "  sessionId: "<< sessionId << "  speechid: " << speechId << std::endl;
    }
    static void onSpeechCancelledHandler(const JsonObject& params) {
        int appId        = params["appId"].Number();
        int sessionId    = params["sessionId"].Number();
        int speechId     = params["speechId"].Number();
        std::cout << "[TTSSrvEvt] " << __FUNCTION__ << ":  appId: " << appId << "  sessionId: "<< sessionId << "  speechid: " << speechId << std::endl;
    }
    static void onSpeechInterruptedHandler(const JsonObject& params) {
        int appId        = params["appId"].Number();
        int sessionId    = params["sessionId"].Number();
        int speechId     = params["speechId"].Number();
        std::cout << "[TTSSrvEvt] " << __FUNCTION__ << ":  appId: " << appId << "  sessionId: "<< sessionId << "  speechid: " << speechId << std::endl;
    }
    static void onNetworkErrorHandler(const JsonObject& params) {
        int appId        = params["appId"].Number();
        int sessionId    = params["sessionId"].Number();
        int speechId     = params["speechId"].Number();
        std::cout << "[TTSSrvEvt] " << __FUNCTION__ << ":  appId: " << appId << "  sessionId: "<< sessionId << "  speechid: " << speechId << std::endl;
    }
    static void onPlaybackErrorHandler(const JsonObject& params) {
        int appId        = params["appId"].Number();
        int sessionId    = params["sessionId"].Number();
        int speechId     = params["speechId"].Number();
        std::cout << "[TTSSrvEvt] " << __FUNCTION__ << ":  appId: " << appId << "  sessionId: "<< sessionId << "  speechid: " << speechId << std::endl;
    }
    static void onSpeechCompleteHandler(const JsonObject& params) {
        int appId = params["appId"].Number();
        int sessionId = params["sessionId"].Number();
        int speechId = params["speechId"].Number();
        std::string text = params["text"].String();
        std::cout << "[TTSSrvEvt] " << __FUNCTION__ << ":  appId: " << appId << "sessionId: "<< sessionId << "speechid: " << speechId << "text: " << text << std::endl;
    }
}
int main(int argc, char *argv[]) {
    std::map<uint32_t, AppInfo*> appInfoMap;

    JSONRPC::Client* remoteObject = NULL;
    std::string reqPayload;

    int choice;
    JsonObject result;
    uint32_t ret;
    int sid = 0;
    int appid = 0;
    int secure = false;
    int sessionid = 0;
    char clearall = 'n';
    string stext;
    string appname;

    MyStream stream((argc > 1 ? argv[1] : "example.txt"));

    Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(SERVER_DETAILS)));

    if (NULL == remoteObject) {
        remoteObject = new JSONRPC::Client(_T(TTSSRV_CALLSIGN), _T(""));
        if (NULL == remoteObject) {
            std::cout << "JSONRPC::Client initialization failed" << std::endl;
        } else {
            /* Register handlers for Event reception. */
            if (remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onTTSStateChanged"),
                        &Handlers::onTTSStateChangedHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onTTSStateChangedHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onTTSStateChangedHandler" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onTTSSessionCreated"),
                        &Handlers::onTTSSessionCreatedHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onTTSSessionCreatedHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onTTSSessionCreatedHandler" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onResourceAcquired"),
                        &Handlers::onResourceAcquiredHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onResourceAcquiredHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onResourceAcquiredHandler" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onResourceReleased"),
                        &Handlers::onResourceReleasedHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onResourceReleasedHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onResourceReleasedHandler" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onWillSpeak"),
                        &Handlers::onWillSpeakHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onWillSpeakHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onWillSpeakHandler" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onSpeechStart"),
                        &Handlers::onSpeechStartHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onSpeechStartHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onSpeechStartHandler" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onSpeechPause"),
                        &Handlers::onSpeechPauseHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onSpeechPauseHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onSpeechPauseHandler" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onSpeechResume"),
                        &Handlers::onSpeechResumeHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onSpeechResumeHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onSpeechResumeHandler" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onSpeechCancelled"),
                        &Handlers::onSpeechCancelledHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onSpeechCancelledHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onSpeechCancelledHandler" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onSpeechInterrupted"),
                        &Handlers::onSpeechInterruptedHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onSpeechInterruptedHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onSpeechInterruptedHandler" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onNetworkError"),
                        &Handlers::onNetworkErrorHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onNetworkErrorHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onNetworkErrorHandler" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onPlaybackError"),
                        &Handlers::onPlaybackErrorHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onPlaybackErrorHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onPlaybackErrorHandler" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onSpeechComplete"),
                        &Handlers::onSpeechCompleteHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onSpeechCompleteHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onSpeechCompleteHandler" << std::endl;
            }

            while (true) {
                std::cout << std::endl;
                showMenu();
                stream.getInput(choice, "Enter your choice : ");

                switch (choice) {
                    case OPT_ENABLE_TTS:
                    {
                       JsonObject params;
                       bool enable;
                       stream.getInput(enable, "1.Enable/0.Disable TTS : ");
                       params["enableTTS"] = JsonValue((bool)enable);
                       ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                               _T("enableTTS"), params, result);
                       if (result["success"].Boolean()) {
                           cout << "TextToSpeech: enableTTS call Success" << endl;
                       } else {
                           cout << "TextToSpeech: enableTTS call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                       }
                       
                    }
                    break;

                    case OPT_VOICE_LIST:
                    {
                        JsonObject params;
                        std::string voices;
                        std::string language;
                        std::string json;
                        stream.getInput(language, "Enter the language [\"*\" - all voices, \".\" - current voice]: ");
                        params["language"] = language;
                        params.ToString(json);
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                _T("listVoices"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "TextToSpeech: listVoices call Success" << endl;
                            voices = result["voices"].String();
                            cout << "Supported voices for langauge: " << voices << endl;
                        } else {
                            cout << "TextToSpeech: listVoices call failed. TTS_Status: " << result["TTS_Status"].String()  << endl;
                        }
                    }
                    break;

                    case OPT_SET_CONFIG:
                    {
                        JsonObject params;
                        std::string voice;
                        std::string language;
                        std::string volume;
                        int rate = 0;
                        stream.getInput(language, "Enter language [en-US/es-MX] : ");
                        params["language"] = language;
                        stream.getInput(voice, "Enter voice [carol/Angelica] : ");
                        params["voice"] = voice;
                        stream.getInput(volume, "Enter volume [0.0-100.0] : ");
                        params["volume"] = volume;
                        stream.getInput(rate, "Enter speed [0-100] : ");
                        params["rate"] = JsonValue(rate);
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                _T("setTTSConfiguration"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "setTTSConfiguration call Success" << endl;
                        } else {
                            cout << "setTTSConfiguration call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                        }
                    }
                    break;

                    case OPT_GET_CONFIG:
                    {
                        JsonObject params;
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                _T("getTTSConfiguration"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "ttsEndPoint : " << result["ttsEndPoint"].String() << endl;
                            cout << "ttsEndPointSecured : " << result["ttsEndPointSecured"].String() << endl;
                            cout << "language : " << result["language"].String() << endl;
                            cout << "voice : " << result["voice"].String() << endl;
                            cout << "volume : " << result["volume"].String() << endl;
                            cout << "rate : " << result["rate"].Number() << endl;
                        } else {
                            cout << "getTTSConfiguration call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                        }
                    }
                    break;

                    case OPT_TTS_ENABLED:
                    {
                        JsonObject params;
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                _T("isTTSEnabled"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "TTS is " << (result["isEnabled"].Boolean()? "Enabled": "Disabled") << endl;
                        } else {
                            cout << "isTTSEnabled call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                        }
                    }
                    break;

                    case OPT_SESSION_ACTIVE:
                    {
                        JsonObject params;
                        stream.getInput(appid, "Enter app id : ");
                        params["appId"] = JsonValue(appid);
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                _T("isSessionActiveForApp"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "App " << appid << (result["isActive"].Boolean()? " has session & active" : " has no session / inactive") << endl;
                        } else {
                            cout << "isSessionActiveForApp call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                        }
                    }
                    break;

                    case OPT_ACQUIRE_RESOURCE:
                    {
                        JsonObject params;
                        stream.getInput(appid, "Enter app id : ");
                        params["appId"] = appid;
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                _T("acquireResource"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "acquireResource call Success" << endl;
                        } else {
                            cout << "acquireResource call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                        }
                    }
                    break;

                    case OPT_CLAIM_RESOURCE:
                    {
                        JsonObject params;
                        stream.getInput(appid, "Enter app id : ");
                        params["appId"] = appid;
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                _T("claimResource"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "claimResource call Success" << endl;
                        } else {
                            cout << "claimResource call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                        }
                    }
                    break;

                    case OPT_RELEASE_RESOURCE:
                    {
                        JsonObject params;
                        stream.getInput(appid, "Enter app id : ");
                        params["appId"] = appid;
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                _T("releaseResource"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "releaseResource call Success" << endl;
                        } else {
                            cout << "releaseResource call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                        }
                    }
                    break;

                    case OPT_CREATE_SESSION:
                    {
                        JsonObject params;
                        stream.getInput(appid, "Enter app id : ");
                        params["appId"] = JsonValue(appid);
                        stream.getInput(appname, "Enter app name : ");
                        params["appName"] = appname;
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                _T("createSession"), params, result);
                        if (result["success"].Boolean()) {
                            JsonValue id = result.Get("TTSSessionId");
                            sessionid = id.Number();
                            if(sessionid) {
                                appInfoMap[appid] = new AppInfo(sessionid);
                                cout << "Session (" << sessionid << ") created for app (" << appid << ")" << endl;
                            } else {
                                cout << "Session couldn't be created for app (" << appid << ") TTS_Status: "<< result["TTS_Status"].String() << endl;
                            }
                            
                        } else {
                            cout << "createSession call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                        }
                    }
                    break;

                    case OPT_IS_ACTIVE_SESSION:
                    {
                        JsonObject params;
                        stream.getInput(appid, "Enter app id : ");
                        if(appInfoMap.find(appid) != appInfoMap.end()) {
                            sessionid = appInfoMap.find(appid)->second->m_sessionId;
                            params["sessionId"] = sessionid;
                            params["forcefetch"] = true;
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("isActiveSession"), params, result);
                            if (result["success"].Boolean()) {
                                cout << "Session (" << sessionid << ") of app (" << appid << ") is " << (result["isActive"].Boolean() ? "active" : "inactive") << endl;
                            } else {
                                cout << "isActiveSession call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                            }
                        } else {
                            cout << "Session hasn't been created for app(" << appid << ")" << endl;
                        }
                    }
                    break;

                    case OPT_SET_PREEMPTIVE:
                    {
                        JsonObject params;
                        stream.getInput(appid, "Enter app id : ");
                        if(appInfoMap.find(appid) != appInfoMap.end()) {
                            bool preemptive = true;
                            stream.getInput(preemptive, "Enter preemptive speech [0/1] : ");
                            params["preemptive"] = preemptive;
                            sessionid = appInfoMap.find(appid)->second->m_sessionId;
                            params["sessionId"] = sessionid;
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("setPreemptiveSpeak"), params, result);
                            if (result["success"].Boolean()) {
                                cout << "setPreemptiveSpeak call success" << endl;
                            } else {
                                cout << "setPreemptiveSpeak call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                            }
                        } else {
                            cout << "Session hasn't been created for app(" << appid << ")" << endl;
                        }
                    }
                    break;

                    case OPT_REQ_EXT_EVENTS:
                    {
                        JsonObject params;
                        stream.getInput(appid, "Enter app id : ");
                        if(appInfoMap.find(appid) != appInfoMap.end()) {
                            int events = 0;
                            stream.getInput(events,
                                "Enter required events flag \n[LSB6-Playback Error, LSB5-Network Error, LSB4-Interrupted, LSB3-Cancelled, LSB2-Resumed, LSB1-Paused]\nEnter events flag [0-63] : ");
                            sessionid = appInfoMap.find(appid)->second->m_sessionId;
                            params["sessionId"] = sessionid;
                            params["extendedEvents"] = events;
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("requestExtendedEvents"), params, result);
                            if (result["success"].Boolean()) {
                                cout << "requestExtendedEvents call success" << endl;
                            } else {
                                cout << "requestExtendedEvents call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                            }
                        } else {
                            cout << "Session hasn't been created for app(" << appid << ")" << endl;
                        }
                    }
                    break;

                    case OPT_SPEAK:
                    {
                        JsonObject params;
                        stream.getInput(appid, "Enter app id : ");
                        if(appInfoMap.find(appid) != appInfoMap.end()) {
                            sessionid = appInfoMap.find(appid)->second->m_sessionId;
                            params["sessionId"] = sessionid;
                            stream.getInput(secure, "Secure/Plain Transfer [0/1] : ");
                            params["secure"] = secure;
                            stream.getInput(sid, "Speech Id (int) : ");
                            params["id"] = sid;
                            stream.getInput(stext, "Enter text to be spoken : ");
                            params["text"] = stext;                            
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("speak"), params, result);
                            if (result["success"].Boolean()) {
                                cout << "speak call success" << endl;
                            } else {
                                cout << "speak call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                            }
                        } else {
                            cout << "Session hasn't been created for app(" << appid << ")" << endl;
                        }
                    }
                    break;

                    case OPT_PAUSE:
                    {
                        JsonObject params;
                        stream.getInput(appid, "Enter app id : ");
                        if(appInfoMap.find(appid) != appInfoMap.end()) {
                            sessionid = appInfoMap.find(appid)->second->m_sessionId;
                            params["sessionId"] = sessionid;
                            stream.getInput(sid, "Speech Id (int) : ");
                            params["speechId"] = sid;
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("pause"), params, result);
                            if (result["success"].Boolean()) {
                                cout << "pause call success" << endl;
                            } else {
                                cout << "pause call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                            }
                        } else {
                            cout << "Session hasn't been created for app(" << appid << ")" << endl;
                        }
                    }
                    break;

                    case OPT_RESUME:
                    {
                        JsonObject params;
                        stream.getInput(appid, "Enter app id : ");
                        if(appInfoMap.find(appid) != appInfoMap.end()) {
                            sessionid = appInfoMap.find(appid)->second->m_sessionId;
                            params["sessionId"] = sessionid;
                            stream.getInput(sid, "Speech Id (int) : ");
                            params["speechId"] = sid;
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("resume"), params, result);
                            if (result["success"].Boolean()) {
                                cout << "resume call success" << endl;
                            } else {
                                cout << "resume call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                            }
                        } else {
                            cout << "Session hasn't been created for app(" << appid << ")" << endl;
                        }
                    }
                    break;

                    case OPT_ABORT:
                    {
                        JsonObject params;
                        stream.getInput(appid, "Enter app id : ");
                        if(appInfoMap.find(appid) != appInfoMap.end()) {
                            sessionid = appInfoMap.find(appid)->second->m_sessionId;
                            params["sessionId"] = sessionid;
                            stream.getInput(clearall, "Should clear pending speeches [y/n]: ");
                            params["clearPending"] = clearall;
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("abort"), params, result);
                            if (result["success"].Boolean()) {
                                cout << "abort call success" << endl;
                            } else {
                                cout << "abort call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                            }
                        } else {
                            cout << "Session hasn't been created for app(" << appid << ")" << endl;
                        }
                    }
                    break;

                    case OPT_IS_SPEAKING:
                    {
                        JsonObject params;
                        stream.getInput(appid, "Enter app id : ");
                        if(appInfoMap.find(appid) != appInfoMap.end()) {
                            sessionid = appInfoMap.find(appid)->second->m_sessionId;
                            params["sessionId"] = sessionid;
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("isSpeaking"), params, result);
                            if (result["success"].Boolean()) {
                                cout << "Session (" << sessionid << ") of app (" << appid << ") is " << (result["speaking"].Boolean() ? "speaking" : "not speaking") << endl;
                            } else {
                                cout << "isSpeaking call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                            }
                        } else {
                            cout << "Session hasn't been created for app(" << appid << ")" << endl;
                        }
                    }
                    break;

                    case OPT_SPEECH_STATE:
                    {
                        JsonObject params;
                        stream.getInput(appid, "Enter app id : ");
                        if(appInfoMap.find(appid) != appInfoMap.end()) {
                            sessionid = appInfoMap.find(appid)->second->m_sessionId;
                            params["sessionId"] = sessionid;
                            stream.getInput(sid, "Speech Id (int) : ");
                            params["speechId"] = sid;
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("getSpeechState"), params, result);
                            if (result["success"].Boolean()) {
                                string state;
                                SpeechState sstate;
                                sstate = result["speechState"].Number();
                                switch(sstate) {
                                    case SPEECH_PENDING: state = "Pending"; break;
                                    case SPEECH_IN_PROGRESS: state = "In Progress/Speaking"; break;
                                    case SPEECH_PAUSED: state = "Paused"; break;
                                    default: state = "Not found";
                                }
                                cout << "Speech Status : " << state << endl;
                            } else {
                                cout << "getSpeechState call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                            }
                        } else {
                            cout << "Session hasn't been created for app(" << appid << ")" << endl;
                        }
                    }
                    break;

                    case OPT_DESTROY_SESSION:
                    {
                        JsonObject params;
                        stream.getInput(appid, "Enter app id : ");
                        if(appInfoMap.find(appid) != appInfoMap.end()) {
                            sessionid = appInfoMap.find(appid)->second->m_sessionId;
                            params["sessionId"] = sessionid;
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("destroySession"), params, result);
                            if (result["success"].Boolean()) {
                                AppInfo *ai = appInfoMap.find(appid)->second;
                                appInfoMap.erase(appInfoMap.find(appid));
                                delete ai;
                                cout << "destroySession call success" << endl;
                            } else {
                                cout << "destroySession call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                            }
                        } else {
                            cout << "Session hasn't been created for app(" << appid << ")" << endl;
                        }
                    }
                    break;

                    case OPT_EXIT:
                        exit(0);

                    case OPT_BLOCK_TILL_INPUT: {
                        std::string in;
                        stream.getInput(in, "Enter any value to continue : ", true);
                    }
                    break;

                    case OPT_SLEEP:
                        stream.getInput(appid, "Enter delay (in secs) : ");
                        sleep(appid);
                        break;
                    }
            }
        }
    }
    return 0;
}

