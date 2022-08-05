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

#include "logger.h"
#include "TTSCommon.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sstream>
#include <iostream>
#include <fstream>
#include <map>

#include "Module.h"

#include "UtilsLogging.h"

#include <securityagent/SecurityTokenUtil.h>
#define MAX_LENGTH 1024

#define TTSSRV_CALLSIGN "org.rdk.TextToSpeech.1"
#define SERVER_DETAILS  "127.0.0.1:9998"
#define MAX_STRING_LENGTH 2048

using namespace std;
using namespace WPEFramework;

using namespace TTS;

volatile unsigned short g_connectedToTTSEventCount = 0;
volatile bool g_connectedToTTS = false;

bool manualExecution = false;
static int currentSpeechId = 0;

#define OPT_ENABLE_TTS            1
#define OPT_VOICE_LIST            2
#define OPT_SET_CONFIG            3
#define OPT_GET_CONFIG            4
#define OPT_TTS_ENABLED           5
#define OPT_SPEAK                 6
#define OPT_CANCEL                7
#define OPT_PAUSE                 8
#define OPT_RESUME                9
#define OPT_IS_SPEAKING           10
#define OPT_SPEECH_STATE          11
#define OPT_EXIT                  12
#define OPT_BLOCK_TILL_INPUT      13
#define OPT_SLEEP                 14

/* Declare module name */
MODULE_NAME_DECLARATION(BUILD_REFERENCE)

void showMenu()
{
    cout << endl;
    cout << "------------------------" << endl;
    cout << OPT_ENABLE_TTS           << ".enableTTS" << endl;
    cout << OPT_VOICE_LIST           << ".listVoices" << endl;
    cout << OPT_SET_CONFIG           << ".setTTSConfiguration" << endl;
    cout << OPT_GET_CONFIG           << ".getTTSConfiguration" << endl;
    cout << OPT_TTS_ENABLED          << ".isTTSEnabled" << endl;
    cout << OPT_SPEAK                << ".speak" << endl;
    cout << OPT_CANCEL               << ".cancel" << endl;
    cout << OPT_PAUSE                << ".pause" << endl;
    cout << OPT_RESUME               << ".resume" << endl;
    cout << OPT_IS_SPEAKING          << ".isspeaking" << endl;
    cout << OPT_SPEECH_STATE         << ".speechstate" << endl;
    cout << OPT_EXIT                 << ".exit" << endl;
    cout << OPT_BLOCK_TILL_INPUT     << ".dummyInput" << endl;
    cout << OPT_SLEEP                << ".sleep" << endl;
    cout << "------------------------" << endl;
}

void Delay(uint32_t delay_us) {
    usleep(1000 * delay_us);
}

struct MyStream {
    MyStream() : myfile(NULL), in(&cin) {
    }

    MyStream(string fname) : myfile(new ifstream(fname)) {
        if(myfile->is_open()) {
            cout << "Reading from file" << endl;
            in = myfile;
            cout << endl;
            manualExecution = false;
        } else {
            cout << "Reading from std::cin" << endl;
            in = &cin;
            manualExecution = true;
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
        cout << endl << "Event: onTTSStateChanged - state (" << state << ")" << endl;
    }
    static void onVoiceChangedHandler(const JsonObject& params) {
        std::string voice     = params["voice"].String();
        cout << endl << "Event: onVoiceChanged - TTS voice got changed to (" << voice << ")" << endl << endl;
    }
    static void onWillSpeakHandler(const JsonObject& params) {
        int speechId     = params["speechid"].Number();
        std::string text = params["text"].String();
        cout << endl << "Event: onWillSpeak - speechid: (" << speechId << ") text (" << text << ")" << endl;
    }
    static void onSpeechStartHandler(const JsonObject& params) {
        int speechId     = params["speechid"].Number();
        std::string text = params["text"].String();
        cout << endl << "Event: onSpeechStart - speechid: (" << speechId << ") text (" << text << ")" << endl;
    }
    static void onSpeechPauseHandler(const JsonObject& params) {
        int speechId     = params["speechid"].Number();
        cout << endl << "Event: onSpeechPause - speechid: (" << speechId << ")" << endl;
    }
    static void onSpeechResumeHandler(const JsonObject& params) {
        int speechId     = params["speechid"].Number();
        cout << endl << "Event: onSpeechResume - speechid: (" << speechId << ")" << endl;
    }
    static void onSpeechInterruptedHandler(const JsonObject& params) {
        int speechId     = params["speechid"].Number();
        cout << endl << "Event: onSpeechInterrupt - speechid: (" << speechId << ")" << endl;
        currentSpeechId = 0;
    }
    static void onNetworkErrorHandler(const JsonObject& params) {
        int speechId     = params["speechid"].Number();
        cout << endl << "Event: onNetworkError - speechid: (" << speechId << ")" << endl;
        currentSpeechId = 0;
    }
    static void onPlaybackErrorHandler(const JsonObject& params) {
        int speechId     = params["speechid"].Number();
        cout << endl << "Event: onPlaybackError - speechid: (" << speechId << ")" << endl;
        currentSpeechId = 0;
    }
    static void onSpeechCompleteHandler(const JsonObject& params) {
        int speechId = params["speechid"].Number();
        std::string text = params["text"].String();
        cout << endl << "Event: onSpeechComplete - speechid: (" << speechId << ") text (" << text << ")" << endl;
        currentSpeechId = 0;
    }
}
int main(int argc, char *argv[]) {
    std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> > remoteObject = NULL;
    std::string reqPayload;

    int choice;
    JsonObject result;
    uint32_t ret;
    string stext;

    MyStream stream((argc > 1 ? argv[1] : "example.txt"));

    Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(SERVER_DETAILS)));

    if (NULL == remoteObject) {
        cout << endl << "TTS Thunder Plugin call sign is " << TTSSRV_CALLSIGN << endl;

        unsigned char buffer[MAX_STRING_LENGTH] = {0};
        int res = GetSecurityToken(MAX_STRING_LENGTH,buffer);

        if(res > 0)
        {
            string sToken = (char*)buffer;
            string query = "token=" + sToken;
            remoteObject = make_shared<WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>>(TTSSRV_CALLSIGN, "", false, query);
        }
        if (NULL == remoteObject) {
            LOGERR("JSONRPC::Client initialization failed");
            return -1;
        } else {
            /* Register handlers for Event reception. */
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onttsstatechanged"),
                        &Handlers::onTTSStateChangedHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onttsstatechangedHandler");
            }
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onvoicechanged"),
                        &Handlers::onVoiceChangedHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onvoicechangedHandler");
            }
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onwillspeak"),
                        &Handlers::onWillSpeakHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onwillspeakHandler");
            }
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onspeechstart"),
                        &Handlers::onSpeechStartHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onspeechstartHandler");
            }
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onspeechpause"),
                        &Handlers::onSpeechPauseHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onspeechpauseHandler");
            }
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onspeechresume"),
                        &Handlers::onSpeechResumeHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onspeechresumeHandler");
            }
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onspeechinterrupted"),
                        &Handlers::onSpeechInterruptedHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onspeechinterruptedHandler");
            }
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onnetworkerror"),
                        &Handlers::onNetworkErrorHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onnetworerrorHandler");
            }
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onplaybackerror"),
                        &Handlers::onPlaybackErrorHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onplaybackerrorHandler");
            }
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onspeechcomplete"),
                        &Handlers::onSpeechCompleteHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onspeechcompleteHandler");
            }

            while (true) {
                if(manualExecution)
                    showMenu();
                cout << "------------------------" << endl;
                stream.getInput(choice, "Enter your choice : ");

                switch (choice) {
                    case OPT_ENABLE_TTS:
                    {
                       JsonObject params;
                       bool enable;
                       stream.getInput(enable, "1.Enable/0.Disable TTS : ");
                       params["enabletts"] = JsonValue((bool)enable);
                       ret = remoteObject->Invoke<JsonObject, JsonObject>(2000,
                                _T("enabletts"), params, result);
                       if (result["success"].Boolean()) {
                           cout << "TextToSpeech: enabletts call Success" << endl;
                       } else {
                           cout << "TextToSpeech: enabletts call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                       }
                       Delay(100);
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
                                _T("listvoices"), params, result);
                        if (result["success"].Boolean()) {
                            voices = result["voices"].String();
                            cout << "Supported voices for langauge: " << voices << endl;
                        } else {
                            cout << "TextToSpeech: listvoices call failed. TTS_Status: " << result["TTS_Status"].String()  << endl;
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
                                _T("setttsconfiguration"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "setttsconfiguration call Success" << endl;
                        } else {
                            cout << "setttsconfiguration call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                        }
                    }
                    break;

                    case OPT_GET_CONFIG:
                    {
                        JsonObject params;
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                _T("getttsconfiguration"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "ttsEndPoint : " << result["ttsendpoint"].String() << endl;
                            cout << "ttsEndPointSecured : " << result["ttsendpointsecured"].String() << endl;
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
                                _T("isttsenabled"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "TTS is " << (result["isenabled"].Boolean()? "Enabled": "Disabled") << endl;
                        } else {
                            cout << "isTTSEnabled call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                        }
                    }
                    break;

                    case OPT_SPEAK:
                    {
                        JsonObject params;
                        stream.getInput(stext, "Enter text to be spoken : ");
                        params["text"] = stext;
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                _T("speak"), params, result);
                        if (result["success"].Boolean()) {
                            currentSpeechId = result["speechid"].Number();
                            cout << "speak call success" << endl;
                        } else {
                            cout << "speak call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                        }
                        Delay(100);
                    }
                    break;

                    case OPT_CANCEL:
                    {
                        JsonObject params;
                        params["speechid"] = currentSpeechId;
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                _T("cancel"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "cancel call success" << endl;
                        } else {
                            cout << "cancel call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                        }
                    }
                    break;

                    case OPT_PAUSE:
                    {
                        JsonObject params;
                        params["speechid"] = currentSpeechId;
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                _T("pause"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "pause call success" << endl;
                        } else {
                            cout << "pause call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                        }
                    }
                    break;

                    case OPT_RESUME:
                    {
                        JsonObject params;
                        params["speechid"] = currentSpeechId;
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                _T("resume"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "resume call success" << endl;
                        } else {
                            cout << "resume call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                        }
                    }
                    break;

                    case OPT_IS_SPEAKING:
                    {
                        JsonObject params;
                        params["speechid"] = currentSpeechId;
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                _T("isspeaking"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "isspeaking : (" << result["speaking"].String() << ")" << endl;
                        } else {
                            cout << "isspeaking call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                        }
                    }
                    break;

                    case OPT_SPEECH_STATE:
                    {
                        JsonObject params;
                        params["speechid"] = currentSpeechId;
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                _T("getspeechstate"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "Speech State : (" << result["speechstate"].String() << ")" << endl;
                        } else {
                            cout << "getspeechstate call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
                        }
                    }
                    break;

                    case OPT_EXIT: {
                        cout << "Test app is exiting" <<endl;
                        exit(0);
                    }

                    case OPT_BLOCK_TILL_INPUT: {
                        std::string in;
                        stream.getInput(in, "Enter any value to continue : ", true);
                    }
                    break;

                    case OPT_SLEEP:
                        int sec = 0;
                        stream.getInput(sec, "Enter delay (in secs) : ");
                        sleep(sec);
                        break;
                    }
            }
        }
    }
    return 0;
}

