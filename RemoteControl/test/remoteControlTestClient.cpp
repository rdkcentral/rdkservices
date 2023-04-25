/*
 * If not stated otherwise in this file or this component's license file
 * the following copyright and licenses apply:
 *
 * Copyright 2018 RDK Management
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

/**
 * @file remoteControlTestClient.cpp
 * @brief Thunder Plugin based Implementation of CPP Test Client for RemoteControl service API's.
 * @reference RDK-28622.
 */

#include <iostream>
#include <bits/stdc++.h>
#include <string>
#include <vector>

#include "Module.h"

#define SYSSRV_CALLSIGN "org.rdk.RemoteControl.1"
#define SERVER_DETAILS  "127.0.0.1:9998"
#define INVOKE_TIMEOUT  (5000)

using namespace std;
using namespace WPEFramework;
// Global data used to for arguments to pass to the plugin methods
int deviceID = 1;

string currentTVCode = "";
string currentAMPCode = "";
string currentModel = "";
string avDevType = "TV";
int infoFrameIndex = 0;
int netType = 1;
int timeout = 0;
int remoteId = 1;
string manufacturer;

const std::vector<std::string> beepLevels = {"off","mid","high"};
size_t beepLevelIdx = 0;
string beepLevel    = beepLevels[0];

/* Declare module name */
MODULE_NAME_DECLARATION(BUILD_REFERENCE)

/* This section can be used for API validation logic. */
void showMenu()
{
    std::cout<< std::endl;
    std::cout<< std::endl;
    std::cout<<"RemoteControl API Methods:             Current Parameter Settings:\n";
    std::cout<<"0.getQuirks                            a.Toggle 'netType' [0 for RF4CE | 1 for BLE]: " << netType << "\n";
    std::cout<<"1.getApiVersionNumber                  b.Enter the 'remoteId' values [1 thru 9]: " << remoteId << "\n";
    std::cout<<"2.startPairing                         c.Toggle 'avDevType' [TV | AMP]: " << avDevType << "\n";
    std::cout<<"3.getNetStatus                         d.Enter the 'timeout' value: [0 - use default | Greater than 0 - use value]: " << timeout << "\n";
    std::cout<<"4.getIRDBManufacturers                 e.Enter the 'tvCode' value: " << currentTVCode << "\n";
    std::cout<<"5.getIRDBModels                        f.Enter the 'avrCode' value: " << currentAMPCode << "\n";
    std::cout<<"6.getIRCodesByNames                    g.Enter the 'model' value (for getIRCodesByNames only): " << currentModel << "\n";
    std::cout<<"7.setIRCode                            h.Cycle thru 'level' values (for findMyRemote only): " << beepLevel << "\n";
    std::cout<<"8.clearIRCodes\n";
    std::cout<<"9.getIRCodesByAutoLookup\n";
    std::cout<<"10.getLastKeypressSource\n";
    std::cout<<"11.initializeIRDB\n";
    std::cout<<"12.findMyRemote\n";

    std::cout<<"\nEnter your choice: ";
}

void formatForDisplay(std::string& str)
{
    const int indent = 4;
    int level = 0;

    for (size_t i = 0; i < str.size(); i++)
    {
        if (str[i] == ',')
        {
            str.insert((i+1), 1, '\n'); // insert after
            if (level > 0)
            {
                str.insert((i+2), (level * indent), ' ');
            }
        }
        else if (str[i] == '}')
        {
            level--;
            if (level < 0) level = 0;
            str.insert((i), 1, '\n');   // insert before
            if (level > 0)
            {
                // after the newline, but before the curly brace
                str.insert((i+1), (level * indent), ' ');
            }
            i += (level * indent) + 1;    // put i back on the curly brace
        }
        else if (str[i] == '{')
        {
            level++;
            str.insert((i+1), 1, '\n'); // insert after
            if (level > 0)
            {
                str.insert((i+2), (level * indent), ' ');
            }
        }
    }
}

#define MAX_FORMATTED_COLUMNS   4
// Takes output from above (formatForDisplay) as input.
// Intended to format "remoteData" info to display in less vertical space.
void formatColumns(std::string& str)
{
    const int colsize = 52;
    int width, numcols;
    std::size_t inidx = 0;
    std::size_t idxRemoteData = std::string::npos;

    std::string outstr;
    std::string colstr[MAX_FORMATTED_COLUMNS];
    int         collines[MAX_FORMATTED_COLUMNS];

    // Auto-sense how many columns we can display
    struct winsize console;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &console);
    width = console.ws_col;
    numcols = width / colsize;
    if (numcols > MAX_FORMATTED_COLUMNS) numcols = MAX_FORMATTED_COLUMNS;

    //std::cout<<"width: "<< width <<"\n";

    if (numcols < 2)
    {
        // If we can't have more than one column, there is nothing to do here.
        return;
    }
    // Find index where "remoteData": is located
    idxRemoteData = str.find("remoteData");
    if (idxRemoteData == std::string::npos)
    {
        // Failure - no remoteData in the input string. Just bail now.
        return;
    }

    for (inidx = 0; inidx < str.size(); inidx++)    // outermost string transcribe character loop
    {
        outstr.push_back(str[inidx]);

        if (inidx > idxRemoteData)
        {
            if (str[inidx] == '{')
            {
                std::size_t idx;
                std::size_t pad;
                std::string outline;
                int outlines = 0;
                int totallines = 0;
                int baselines = 0;
                int xtra = 0;

                // We have encountered a remoteData block.
                // Transcribe to include the curly brace newline...
                do
                {
                    inidx++;
                    outstr.push_back(str[inidx]);
                }
                while(str[inidx] != '\n');
                // ...then update the input index to the first character after the newline.
                inidx++;
                // Count the number of lines between here and the next closing curly brace.
                for (std::size_t i = inidx; i < str.size(); i++)
                {
                    if (str[i] == '\n')
                        totallines++;
                    else if (str[i] == '}')
                        break;
                }
                // Allocate the lines to the number of columns we have.
                baselines = totallines / numcols;
                xtra = totallines % numcols;
                outlines = baselines + ((xtra > 0) ? 1 : 0);    // We'll use this later...
                for (int i = 0; i < numcols; i++)
                {
                    collines[i] = baselines + ((xtra > 0) ? 1 : 0);
                    if (xtra > 0) xtra--;
                }
                // Load the column strings from the input.
                // This will also advance the input index past the entire block.
                for (int i = 0; i < numcols; i++)
                {
                    colstr[i].clear();
                    for (; inidx < str.size(); inidx++)
                    {
                        if (collines[i] > 0)
                        {
                            colstr[i].push_back(str[inidx]);
                            if (str[inidx] == '\n')
                            {
                                collines[i]--;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                // At this point, the input index is set to the character AFTER the final newline
                // in the input block. But we want to leave the input index AT the final newline,
                // to get the correct continuation of the outer transcription loop. So we must
                // move it back by one, here.
                inidx--;

                // Build line strings encompassing all the columns,
                // and add them to the output string.
                for (int i = 0; i < outlines; i++)
                {
                    for (int j = 0; j < numcols; j++)
                    {
                        if (!colstr[j].empty() && (totallines > 0))
                        {
                            idx = colstr[j].find('\n');
                            if (idx != std::string::npos)
                            {
                                outline.append(colstr[j], 0, idx+1);
                                // Remove the line we just appended, from the start of the colstr.
                                colstr[j].erase(0, idx+1);
                                // Count the line as being handled - done with.
                                totallines--;
                                // Except for the last column, clobber the newline,
                                // and pad the line out to the column width, using spaces.
                                // We leave things as they are for the last column.
                                // We also leave the last line out of the total number
                                // of lines alone (newline intact), because even if it isn't
                                // in the rightmost column, there won't be any more lines!
                                if ((j < (numcols - 1)) && (totallines > 0))
                                {
                                    // Get rid of the newline
                                    outline.pop_back();     // We all agree to use C++11, right?
                                    // Determine how many spaces we need.
                                    pad = ((std::size_t)colsize) - idx;
                                    // Append spaces to the outline.
                                    outline.append(pad, ' ');
                                }
                            }
                        }
                    }
                    // Add this cross-column line to the overall output,
                    outstr.append(outline);
                    outline.clear();
                }
            }
        }
    }

    // Return the output in the original reference input string
    str.clear();
    str.append(outstr);
}

// Event parameters sent to JSONRPC::Client event handlers, have somehow
// acquired overall quotation marks, with internal quotes backslash-escaped.
// They weren't presented that way in the past, but now somehow they are.
// This function tries to detect and correct that, for console output.
// Should have no effect if the in/out string isn't inside double-quotes.
void removeJsonQuotes(std::string& str)
{
    if ((str.front() == '"') && (str.back() == '"'))
    {
        str.erase(0, 1);
        str.pop_back();

        for (size_t i = 0; i < str.size(); i++)
        {
            if ((i < (str.size() - 1)) && (str[i] == 0x5c) && (str[i+1] == '"'))
            {
                str.erase(i, 1);
            }
        }
    }
}

// Make changes in parameters
void handleParams(string& cmd)
{
    switch(cmd[0])
    {
        case 'a':
        {
            // netType
            if (netType == 1)
            {
                netType = 0;
            }
            else
            {
                netType = 1;
            }
        }
        break;

        case 'b':
        {
            // Enter rf4ce remoteId
            std::cout << "Enter the remoteId (in decimal): ";
            std::cin >> remoteId;
            if (std::cin.peek() == '\n')
            {
                string temp;
                std::getline(std::cin, temp);
            }
        }
        break;

        case 'c':
        {
            // avDevType
            if (avDevType == "AMP")
            {
                avDevType = "TV";
            }
            else
            {
                avDevType = "AMP";
            }
        }
        break;

        case 'd':
        {
            // Enter timeout
            std::cout << "Enter the timeout (0 for default): ";
            std::cin >> timeout;
            if (std::cin.peek() == '\n')
            {
                string temp;
                std::getline(std::cin, temp);
            }
        }
        break;

        case 'e':
        {
            // Enter TV code
            string digits;
            while (digits.empty())
            {
                std::cout << "Enter the TV code: ";
                std::getline(std::cin, digits);
            }
            currentTVCode = digits;
        }
        break;

        case 'f':
        {
            // Enter AVR code
            string digits;
            while (digits.empty())
            {
                std::cout << "Enter the AVR code: ";
                std::getline(std::cin, digits);
            }
            currentAMPCode = digits;
        }
        break;

        case 'g':
        {
            // Enter model
            string model = "";
            std::cout << "Enter the model (return for empty string): ";
            std::getline(std::cin, model);

            if(!model.empty())
            {
                currentModel = model;
            }
            else
            {
                currentModel = "";
            }
        }
        break;

        case 'h':
        {
            // Cycle through level values
            beepLevelIdx++;
            if (beepLevelIdx >= beepLevels.size())
            {
                beepLevelIdx = 0;
            }
            beepLevel = beepLevels[beepLevelIdx];
        }
        break;
    }
}

/* This section is related to the event handler implementation for RemoteControl Plugin Events. */

namespace Handlers {
    /* Event Handlers */
    static void onStatus(const Core::JSON::String& parameters) {
        std::string message;
        parameters.ToString(message);
        removeJsonQuotes(message);
        std::cout << "\n[RemoteControlEvt] " << __FUNCTION__ << ": " << message << std::endl;
    }
}

int main(int argc, char** argv)
{
    int retStatus = -1;
    JSONRPC::LinkType<Core::JSON::IElement> * remoteObject = NULL;

    int choice;
    string cmd;
    uint32_t ret;
    string lastCmd;

    Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(SERVER_DETAILS)));

    // Security Token
    std::cout << "Retrieving security token" << std::endl;
    std::string sToken;

    FILE *pSecurity = popen("/usr/bin/WPEFrameworkSecurityUtility", "r");
    if(pSecurity) {
        JsonObject pSecurityJson;
        std::string pSecurityOutput;
        int         pSecurityOutputTrimIndex;
        std::array<char, 256> pSecurityBuffer;

        while(fgets(pSecurityBuffer.data(), 256, pSecurity) != NULL) {
            pSecurityOutput += pSecurityBuffer.data();
        }
        pclose(pSecurity);

        pSecurityOutputTrimIndex = (int)pSecurityOutput.find('{');
        if(pSecurityOutputTrimIndex == (int)std::string::npos) {
            std::cout << "Security Utility returned unexpected output" << std::endl;
        } else {
            if(pSecurityOutputTrimIndex > 0) {
                 std::cout << "Trimming output from Security Utility" << std::endl;
                 pSecurityOutput = pSecurityOutput.substr(pSecurityOutputTrimIndex);
            }
            pSecurityJson.FromString(pSecurityOutput);
            if(pSecurityJson["success"].Boolean() == true) {
                std::cout << "Security Token retrieved successfully!" << std::endl;
                sToken = "token=" + pSecurityJson["token"].String();
            } else {
                std::cout << "Security Token retrieval failed!" << std::endl;
            }
        }
    } else {
        std::cout << "Failed to open security utility" << std::endl;
    }
    // End Security Token

    std::cout << "Using callsign: " << SYSSRV_CALLSIGN << std::endl;

    if (NULL == remoteObject) {
        remoteObject = new JSONRPC::LinkType<Core::JSON::IElement>(_T(SYSSRV_CALLSIGN), _T(""), false, sToken);
        if (NULL == remoteObject) {
            std::cout << "JSONRPC::Client initialization failed" << std::endl;

        } else {

            {
                // Create a controller client
                static auto& controllerClient = *new WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>("", "", false, sToken);
                // In case the plugin isn't activated already, try to start it, BEFORE registering for the events!
                string strres;
                JsonObject params;
                params["callsign"] = SYSSRV_CALLSIGN;
                JsonObject result;
                ret = controllerClient.Invoke<JsonObject, JsonObject>(2000, "activate", params, result);
                result.ToString(strres);
                std::cout<<"\nstartup result : "<< strres <<"\n";
            }

            /* Register handlers for Event reception. */
            std::cout << "\nSubscribing to event handlers\n" << std::endl;
            if (remoteObject->Subscribe<Core::JSON::String>(1000, _T("onStatus"),
                        &Handlers::onStatus) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onStatus" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onStatus" << std::endl;
            }

            /* API Validation Logic. */
            while (true) {
                while (cmd.empty())
                {
                    showMenu();
                    std::getline(std::cin, cmd);
                    lastCmd = cmd;
                }
                if ((cmd[0] >= '0') && (cmd[0] <= '9'))
                {
                    choice = stoi(cmd);
                }
                else if ((cmd[0] >= 'a') && (cmd[0] <= 'h'))
                {
                    handleParams(cmd);
                    cmd.clear();
                    continue;
                }
                else return 0;

                {
                    JsonObject result;  // The Clear method can leave crud.  A new instance off the stack works better.

                    switch (choice) {
                        case 0:
                            {
                                JsonObject params;
                                string res;
                                ret = remoteObject->Invoke<JsonObject, JsonObject>(INVOKE_TIMEOUT,
                                                    _T("getQuirks"), params, result);
                                std::cout<<"RemoteControl Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"RemoteControl getQuirks call - Success!\n";
                                } else {
                                    std::cout<<"RemoteControl getQuirks call - failed!\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;

                        case 1:
                            {
                                JsonObject params;
                                string res;
                                ret = remoteObject->Invoke<JsonObject, JsonObject>(INVOKE_TIMEOUT,
                                                    _T("getApiVersionNumber"), params, result);
                                std::cout<<"RemoteControl Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"RemoteControl getApiVersionNumber call - Success!\n";
                                } else {
                                    std::cout<<"RemoteControl getApiVersionNumber call - failed!\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;

                        case 2:
                            {
                                JsonObject params;
                                string res;
                                params["netType"] = netType;

                                if (timeout > 0) {
                                    params["timeout"] = timeout;
                                }

                                ret = remoteObject->Invoke<JsonObject, JsonObject>(INVOKE_TIMEOUT,
                                                    _T("startPairing"), params, result);
                                std::cout<<"RemoteControl Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"\nRemoteControl startPairing call success\n";
                                } else {
                                    std::cout<<"\nRemoteControl startPairing call failure\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;

                        case 3:
                            {
                                JsonObject params;
                                string res;
                                params["netType"] = netType;
                                ret = remoteObject->Invoke<JsonObject, JsonObject>(INVOKE_TIMEOUT,
                                                    _T("getNetStatus"), params, result);
                                std::cout<<"RemoteControl Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                formatForDisplay(res);
                                formatColumns(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"RemoteControl getNetStatus call - Success!\n";
                                } else {
                                    std::cout<<"RemoteControl getNetStatus call - failed!\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;

                        case 4:
                            {
                                JsonObject params;
                                string res;

                                params["netType"] = netType;
                                params["avDevType"] = avDevType;

                                std::cout<<"\nEnter at least part of a manufacturer name : ";
                                std::cin>> manufacturer;
                                params["manufacturer"] = manufacturer;


                                ret = remoteObject->Invoke<JsonObject, JsonObject>(INVOKE_TIMEOUT,
                                                    _T("getIRDBManufacturers"), params, result);
                                std::cout<<"RemoteControl Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"RemoteControl getIRDBManufacturers call - Success!\n";
                                } else {
                                    std::cout<<"RemoteControl getIRDBManufacturers call - failed!\n";
                                }
                                std::cout<<"result : "<< res <<"\n";
                            }
                            break;

                        case 5:
                            {
                                JsonObject params;
                                string     model;
                                string     res;

                                params["netType"] = netType;
                                params["avDevType"] = avDevType;

                                std::cout<<"\nEnter the full manufacturer name : ";
                                std::cin>> manufacturer;
                                params["manufacturer"] = manufacturer;

                                std::cout<<"\nEnter at least part of a model name : ";
                                std::cin>> model;
                                params["model"] = model;


                                ret = remoteObject->Invoke<JsonObject, JsonObject>(INVOKE_TIMEOUT,
                                                    _T("getIRDBModels"), params, result);
                                std::cout<<"RemoteControl Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"RemoteControl getIRDBModels call - Success!\n";
                                } else {
                                    std::cout<<"RemoteControl getIRDBModels call - failed!\n";
                                }
                                std::cout<<"result : "<< res <<"\n";
                            }
                            break;

                        case 6:
                            {
                                JsonObject params;
                                string     model;
                                string     res;

                                params["netType"] = netType;
                                params["avDevType"] = avDevType;

                                std::cout<<"\nEnter the full manufacturer name : ";
                                std::cin>> manufacturer;
                                params["manufacturer"] = manufacturer;

                                if(!currentModel.empty())
                                {
                                    params["model"] = currentModel;
                                }


                                ret = remoteObject->Invoke<JsonObject, JsonObject>(INVOKE_TIMEOUT,
                                                    _T("getIRCodesByNames"), params, result);
                                std::cout<<"RemoteControl Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"RemoteControl getIRCodesByNames call - Success!\n";
                                } else {
                                    std::cout<<"RemoteControl getIRCodesByNames call - failed!\n";
                                }
                                std::cout<<"result : "<< res <<"\n";
                            }
                            break;

                        case 7:
                            {
                                JsonObject params;
                                string res;
                                params["netType"] = netType;
                                params["remoteId"] = remoteId;
                                params["avDevType"] = avDevType;

                                if(avDevType=="TV")
                                {
                                    params["code"] = currentTVCode;
                                }
                                else
                                {
                                    params["code"] = currentAMPCode;
                                }

                                ret = remoteObject->Invoke<JsonObject, JsonObject>(INVOKE_TIMEOUT,
                                                    _T("setIRCode"), params, result);
                                std::cout<<"RemoteControl Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                formatForDisplay(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"RemoteControl setIRCode call - Success!\n";
                                } else {
                                    std::cout<<"RemoteControl setIRCode call - failed!\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;

                        case 8:
                            {
                                JsonObject params;
                                string res;
                                params["netType"] = netType;
                                params["remoteId"] = remoteId;

                                ret = remoteObject->Invoke<JsonObject, JsonObject>(INVOKE_TIMEOUT,
                                                    _T("clearIRCodes"), params, result);
                                std::cout<<"RemoteControl Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                formatForDisplay(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"RemoteControl clearIRCodes call - Success!\n";
                                } else {
                                    std::cout<<"RemoteControl clearIRCodes call - failed!\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;

                            case 9:
                            {
                                JsonObject params;
                                string     model;
                                string     res;

                                params["netType"] = netType;

                                ret = remoteObject->Invoke<JsonObject, JsonObject>(INVOKE_TIMEOUT,
                                                    _T("getIRCodesByAutoLookup"), params, result);
                                std::cout<<"RemoteControl Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"RemoteControl getIRCodesByAutoLookup call - Success!\n";
                                } else {
                                    std::cout<<"RemoteControl getIRCodesByAutoLookup call - failed!\n";
                                }
                                std::cout<<"result : "<< res <<"\n";
                            }
                            break;

                        case 10:
                            {
                                JsonObject params;
                                string res;
                                params["netType"] = netType;
                                ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                                    _T("getLastKeypressSource"), params, result);
                                std::cout<<"RemoteControl Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                formatForDisplay(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"RemoteControl getLastKeypressSource call - Success!\n";
                                } else {
                                    std::cout<<"RemoteControl getLastKeypressSource call - failed!\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;

                        case 11:
                            {
                                JsonObject params;
                                string res;
                                params["netType"] = netType;

                                ret = remoteObject->Invoke<JsonObject, JsonObject>(INVOKE_TIMEOUT,
                                                    _T("initializeIRDB"), params, result);
                                std::cout<<"RemoteControl Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                formatForDisplay(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"RemoteControl initializeIRDB call - Success!\n";
                                } else {
                                    std::cout<<"RemoteControl initializeIRDB call - failed!\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;         
                            
                        case 12:
                            {
                                JsonObject params;
                                string res;
                                params["netType"] = netType;
                                params["level"] = beepLevel;

                                ret = remoteObject->Invoke<JsonObject, JsonObject>(INVOKE_TIMEOUT,
                                                    _T("findMyRemote"), params, result);
                                std::cout<<"RemoteControl Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"RemoteControl findMyRemote call - Success!\n";
                                } else {
                                    std::cout<<"RemoteControl findMyRemote call - failed!\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;
                            
                        default:
                            std::cout<<"Entry not recognized!\n";
                            break;
                    }
                }

                std::cout<<"\n\nTo continue press ENTER;  Press 'r' to repeat command; To quit press any other key --> ";
                cmd.clear();

                std::getline(std::cin, cmd);
                if (cmd.empty())
                {
                    cmd.clear();
                    lastCmd.clear();
                }
                else if ((cmd[0] == 'r') || (cmd[0] == 'R'))
                    cmd = lastCmd;
                else if ((cmd[0] >= '0') && (cmd[0] <= '9'))
                {
                    choice = stoi(cmd);
                    lastCmd = cmd;
                }
                else if ((cmd[0] >= 'a') && (cmd[0] <= 'h'))
                {
                }
                else
                    break;

            }
        }
    }

    return retStatus;
}
