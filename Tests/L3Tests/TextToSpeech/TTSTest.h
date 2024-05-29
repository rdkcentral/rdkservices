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

#ifndef _TTS_TEST_H_
#define _TTS_TEST_H_

#include <iostream>
#include <thread>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <map>

using namespace std;

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
public:
    bool manualExecution;
};

int ttsL3ThreadFunc(MyStream *stream);

#endif /* _TTS_TEST_H_ */

