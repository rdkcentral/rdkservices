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

MODULE_NAME_DECLARATION(BUILD_REFERENCE)

#include <securityagent/SecurityTokenUtil.h>
#define MAX_LENGTH 1024

#define TTSSRV_CALLSIGN "org.rdk.SystemAudioPlayer.1"
#define SERVER_DETAILS  "127.0.0.1:9998"
#define MAX_STRING_LENGTH 2048

using namespace std;
using namespace WPEFramework;

bool manualExecution = false;
static int currentSpeechId = 0;

#define OPT_OPEN           1
#define OPT_PLAY           2
#define OPT_STOP           3
#define OPT_CLOSE          4
#define OPT_PLAYBUFFER     5
#define OPT_SETMIXER       6
#define OPT_PAUSE          7
#define OPT_RESUME         8
#define OPT_CONFIG         9
#define OPT_GETSESSION     10

void showMenu()
{
    cout << endl;
    cout << "------------------------" << endl;
    cout << OPT_OPEN           << ".open" << endl;
    cout << OPT_PLAY           << ".play" << endl;
    cout << OPT_STOP           << ".stop" << endl;
    cout << OPT_CLOSE          << ".close" << endl;
    cout << OPT_PLAYBUFFER     << ".playbuffer" << endl;
    cout << OPT_SETMIXER       << ".setmixer" << endl;
    cout << OPT_PAUSE          << ".pause" << endl;
    cout << OPT_RESUME         << ".resume" << endl;
    cout << OPT_CONFIG         << ".config" << endl;
    cout << OPT_GETSESSION     << ".getPlayerSessionId" << endl;
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
    static void onSAPHandler(const JsonObject& params) {
        int Id     = params["id"].Number();
        std::string event = params["event"].String(); 
        cout << endl << "Event: id: " << Id << endl<<"event"<<event;
    }
    
}
int main(int argc, char *argv[]) {
    std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> > remoteObject = NULL;
    std::string reqPayload;

    int choice;
    uint32_t ret;
    string stext;

    MyStream stream((argc > 1 ? argv[1] : "example.txt"));

    Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(SERVER_DETAILS)));

    if (NULL == remoteObject) {
        cout << endl << "SAP Thunder Plugin call sign is " << TTSSRV_CALLSIGN << endl;

        unsigned char buffer[MAX_STRING_LENGTH] = {0};
        #if 0
        int res = GetSecurityToken(MAX_STRING_LENGTH,buffer); //for platco
                  
        if(res > 0)
        {
            string sToken = (char*)buffer;
            string query = "token=" + sToken;
            remoteObject = make_shared<WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>>(TTSSRV_CALLSIGN, "", false, query);
        }
        #endif
        string query;  //change for lama
        remoteObject = make_shared<WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>>(TTSSRV_CALLSIGN, "", false, query);
        if (NULL == remoteObject) {
            LOGERR("JSONRPC::Client initialization failed");
            return -1;
        } else {
            /* Register handlers for Event reception. */
            if (!remoteObject->Subscribe<Core::JSON::VariantContainer>(1000, _T("onsapevents"),
                        &Handlers::onSAPHandler) == Core::ERROR_NONE) {
                LOGERR("Failed to Subscribe notification handler : onSAPHandler");
            }
            
            while (true) {
                if(manualExecution)
                    showMenu();
                cout << "------------------------" << endl;
                stream.getInput(choice, "Enter your choice : ");

                switch (choice) {
                    case OPT_OPEN:
                    {
                       JsonObject params;
                       JsonObject result;
                       std::string audiotype;
                       std::string sourcetype;
                       std::string playmode;
                       stream.getInput(audiotype, "Enter Audio Type[pcm,mp3,wav] ");
                       params["audiotype"] = audiotype;
                       stream.getInput(sourcetype, "Enter Source Type[data,httpsrc,filesrc,websocket] ");
                       params["sourcetype"] = sourcetype; 
                       stream.getInput(playmode, "Enter PlayMode Type[system,app] ");
                       params["playmode"] = playmode;
                       ret = remoteObject->Invoke<JsonObject, JsonObject>(2000,
                                _T("open"), params, result);
                       if (result["success"].Boolean()) {
                           cout << "open call Success" << endl;
                           cout << "session id "<<result["id"].Number();
                       } else {
                           cout << "open call failed." << result["message"].String()<< endl;
                       }
                       Delay(100);
                    }
                    break;

                    case OPT_PLAY:
                    {
                        JsonObject params;
                        JsonObject result;
                        int id;
                        std::string url;
                        stream.getInput(id, "Enter id");
                        params["id"] = JsonValue(id);
                        stream.getInput(url,"Enter url");
                        params["url"] = url;
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(2000,
                                _T("play"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "play success: " << endl;
                        } else {
                            cout << "play failed" << endl << result["message"].String()<< endl;
                        }
                    }
                    break;

                    case OPT_STOP:
                    {
                        JsonObject params;
                        JsonObject result;
                        int id;
                        stream.getInput(id, "Enter id");
                        params["id"] = JsonValue(id);
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(2000,
                                _T("stop"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "stop success: " << endl;
                        } else {
                            cout << "stop failed" << endl;
                        }
                    
                    }   
                    break;

                    case OPT_CLOSE:
                    {
                        JsonObject params;
                        JsonObject result;
                        int id;
                        stream.getInput(id, "Enter id");
                        params["id"] = JsonValue(id);
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(2000,
                                _T("close"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "close success:" <<endl;
                        } else {
                            cout << "close failed" << endl;
                        }
                        
                    }
                    break;
                    
                    case OPT_PLAYBUFFER:
                    {
                        JsonObject params;
                        JsonObject result;
                        int id;
                        std::string data;
                        stream.getInput(id, "Enter id");
                        params["id"] = JsonValue(id);
                        stream.getInput(data, "Enter base64 encoded pcm buffer");
                        params["data"] = data;
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(2000,
                                _T("playbuffer"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "playbuffer success:" <<endl;
                        } else {
                            cout << "playbuffer failed" << endl;
                        }

                        
                    }
                    break;

                    case OPT_SETMIXER:
                    {
                        JsonObject params;
                        JsonObject result;
                        std::string primVol;
                        std::string thisVol;
                        int id;
                        stream.getInput(id, "Enter id");
                        params["id"] = JsonValue(id);
                        stream.getInput(primVol , "Enter Primary Volume[0-100] ");
                        params["primaryVolume"] = primVol ;
                        stream.getInput(thisVol, "Enter Player Volume[0-100] ");
                        params["playerVolume"] = thisVol;
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                _T("setMixerLevels"), params, result);
                        if (result["success"].Boolean()) {
                           cout << "setMixerlevels call Success" << endl;
                        } else {
                           cout << "setMixerlevels call failed." << endl;
                        }  
                    }  
		    break;
         
                    case OPT_PAUSE:
                    {
                        JsonObject params;
                        JsonObject result;
                        int id;
                        std::string url;
                        stream.getInput(id, "Enter id");
                        params["id"] = JsonValue(id);
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(2000,
                                _T("pause"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "pause success:" <<endl;
                        } else {
                            cout << "pause failed" << endl;
                        }

                    }
                    break;

                    case OPT_RESUME:
                    {
                        JsonObject params;
                        JsonObject result;
                        int id;
                        std::string url;
                        stream.getInput(id, "Enter id");
                        params["id"] = JsonValue(id);
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(2000,
                                _T("resume"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "resume success:" <<endl;
                        } else {
                            cout << "resume failed" << endl;
                        }

                    }
                    break;
                    case OPT_CONFIG:
                    {
                        JsonObject params;
                        JsonObject pcmconfig;
                        JsonObject result;
                        int id;
                        std::string format;
                        std::string rate;
                        std::string channels;
                        std::string layout;
                        stream.getInput(id, "Enter id");
                        params["id"] = JsonValue(id);
                        stream.getInput(format, "Enter pcm format eg:[S16LE] ");
                        pcmconfig["format"] = format;
                        stream.getInput(rate, "Enter pcm rate eg:[22050] ");
                        pcmconfig["rate"] = rate;                     
                        stream.getInput(channels, "Enter pcm channel eg:[1] ");
                        pcmconfig["channels"] = channels;
                        stream.getInput(layout, "Enter pcm layout eg:[interleaved] ");
                        pcmconfig["layout"] = layout;
                        params["pcmconfig"] = pcmconfig;                     
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(2000,
                                _T("config"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "config success:" <<endl;
                        } else {
                            cout << "config failed" << endl;
                        }

                    }
                    break;  
                    case OPT_GETSESSION:
                    {
                        JsonObject params;
                        JsonObject result;
                        std::string url;
                        stream.getInput(url,"Enter url");
                        params["url"] = url;
                        ret = remoteObject->Invoke<JsonObject, JsonObject>(2000,
                                _T("getPlayerSessionId"), params, result);
                        if (result["success"].Boolean()) {
                            cout << "getPlayerSessionId success: " << endl;
                            cout << "session id : "<<result["sessionId"].Number();  
                        } else {
                            cout << "getPlayerSessionId failed" << endl;
                        }
                    }
                    break;
                }
            }
        }
    }
    return 0;
}

