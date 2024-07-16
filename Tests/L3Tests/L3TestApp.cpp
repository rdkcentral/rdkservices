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

#include "TTSTest.h"

#define TTSTEST         1
#define SAPTEST         2
#define TESTAPPEXIT     3

static void showParentMenu()
{
    system("clear");
    cout << endl;
    cout << "------------------------" << endl;
    cout << TTSTEST             << ".TTSTests" << endl;
    cout << SAPTEST             << ".SAPTests" << endl;
    cout << TESTAPPEXIT         << ".TestAppExit" << endl;
    cout << "------------------------" << endl;
}

int main(int argc, char *argv[]) {
    int choice;
    std::thread *m_ttsThread = NULL;
    MyStream stream((argc > 1 ? argv[1] : "example.txt"));

    while (true) {
        showParentMenu();
        stream.getInput(choice, "Enter your choice : ");
        switch (choice) {
            case TTSTEST:{
                m_ttsThread = new std::thread(ttsL3ThreadFunc, &stream);
                if(m_ttsThread) {
                    m_ttsThread->join();
                    delete m_ttsThread;
                }
            }
            break;

            case SAPTEST: {
                cout << "yet to be included" << endl;
            }
            break;

            case TESTAPPEXIT: {
                cout << "Exiting from TTS L3 test" <<endl;
                exit(0);
            }

            default: {
                cout << "Invalid Input" <<endl;
            }
        }
    }
}
