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

#include "logger.h"
#include "TTSCommon.h"
#include "utils.h"

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

bool manualExecution = false;

#define OPT_ENABLE_TTS            1
#define OPT_VOICE_LIST            2
#define OPT_SET_CONFIG            3
#define OPT_GET_CONFIG            4
#define OPT_TTS_ENABLED           5
#define OPT_SPEAK                 6 
#define OPT_CANCEL                7
#define OPT_EXIT                  8
#define OPT_BLOCK_TILL_INPUT      9
#define OPT_SLEEP                 10

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
        int speechId     = params["speechId"].Number();
        std::string text = params["text"].String();
        cout << endl << "Event: onWillSpeak - speechid: (" << speechId << ") text (" << text << ")" << endl;        
    }
    static void onSpeechStartHandler(const JsonObject& params) {
        int speechId     = params["speechId"].Number();
        std::string text = params["text"].String();
        cout << endl << "Event: onSpeechStart - speechid: (" << speechId << ") text (" << text << ")" << endl;
    }
    static void onSpeechPauseHandler(const JsonObject& params) {
        int speechId     = params["speechId"].Number();
        cout << endl << "Event: onSpeechPause - speechid: (" << speechId << ")" << endl;
    }
    static void onSpeechResumeHandler(const JsonObject& params) {
        int speechId     = params["speechId"].Number();
        cout << endl << "Event: onSpeechResume - speechid: (" << speechId << ")" << endl;
    }
    static void onSpeechCancelledHandler(const JsonObject& params) {
        int speechId     = params["speechId"].Number();
        cout << endl << "Event: onSpeechCancel - speechid: (" << speechId << ")" << endl;
    }
    static void onSpeechInterruptedHandler(const JsonObject& params) {
        int speechId     = params["speechId"].Number();
        cout << endl << "Event: onSpeechInterrupt - speechid: (" << speechId << ")" << endl;
    }
    static void onNetworkErrorHandler(const JsonObject& params) {
        int speechId     = params["speechId"].Number();
        cout << endl << "Event: onNetworkError - speechid: (" << speechId << ")" << endl;
    }
    static void onPlaybackErrorHandler(const JsonObject& params) {
        int speechId     = params["speechId"].Number();
        cout << endl << "Event: onPlaybackError - speechid: (" << speechId << ")" << endl;
    }
    static void onSpeechCompleteHandler(const JsonObject& params) {
        int speechId = params["speechId"].Number();
        std::string text = params["text"].String();
        cout << endl << "Event: onSpeechComplete - speechid: (" << speechId << ") text (" << text << ")" << endl;
    }
}
int main(int argc, char *argv[]) {
    JSONRPC::Client* remoteObject = NULL;
    std::string reqPayload;

    int choice;
    JsonObject result;
    uint32_t ret;
    bool clearall = false;
    string stext;

    MyStream stream((argc > 1 ? argv[1] : "example.txt"));

    Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(SERVER_DETAILS)));

    if (NULL == remoteObject) {
        cout << endl << "TTS Thunder Plugin call sign is " << TTSSRV_CALLSIGN << endl;
        remoteObject = new JSONRPC::Client(_T(TTSSRV_CALLSIGN), _T(""));
        if (NULL == remoteObject) {
            std::cout << "JSONRPC::Client initialization failed" << std::endl;
        } else {
            /* Register handlers for Event reception. */
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onTTSStateChanged"),
                        &Handlers::onTTSStateChangedHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onTTSStateChangedHandler");
            }
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onVoiceChanged"),
                        &Handlers::onVoiceChangedHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onVoiceChangedHandler");
            }
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onWillSpeak"),
                        &Handlers::onWillSpeakHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onWillSpeakHandler");
            }
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onSpeechStart"),
                        &Handlers::onSpeechStartHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onSpeechStartHandler");
            }
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onSpeechPause"),
                        &Handlers::onSpeechPauseHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onSpeechPauseHandler");
            }
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onSpeechResume"),
                        &Handlers::onSpeechResumeHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onSpeechResumeHandler");
            }
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onSpeechCancelled"),
                        &Handlers::onSpeechCancelledHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onSpeechCancelledHandler");
            }
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onSpeechInterrupted"),
                        &Handlers::onSpeechInterruptedHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onSpeechInterruptedHandler");
            }
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onNetworkError"),
                        &Handlers::onNetworkErrorHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onNetworkErrorHandler");
            }
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onPlaybackError"),
                        &Handlers::onPlaybackErrorHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onPlaybackErrorHandler");
            }
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onSpeechComplete"),
                        &Handlers::onSpeechCompleteHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onSpeechCompleteHandler");
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
                       params["enableTTS"] = JsonValue((bool)enable);
                       ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                               _T("enableTTS"), params, result);
                       if (result["success"].Boolean()) {
                           cout << "TextToSpeech: enableTTS call Success" << endl;
                       } else {
                           cout << "TextToSpeech: enableTTS call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
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
                                _T("listVoices"), params, result);
                        if (result["success"].Boolean()) {
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

                    case OPT_SPEAK:
                    {
                        JsonObject params;
                        stream.getInput(stext, "Enter text to be spoken : ");
                        params["text"] = stext;                            
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                _T("speak"), params, result);
                        if (result["success"].Boolean()) {
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
                        stream.getInput(clearall, "Should clear pending speeches [true/false]: ");
                        params["clearPending"] = clearall;
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                _T("cancel"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "cancel call success" << endl;
                        } else {
                            cout << "cancel call failed. TTS_Status: " << result["TTS_Status"].String() << endl;
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

