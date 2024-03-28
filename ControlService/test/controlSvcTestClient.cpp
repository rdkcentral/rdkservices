/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/

/**
 * @file controlSvcTestClient.cpp
 * @brief Thunder Plugin based Implementation of CPP Test Client for ControlService service API's.
 * @reference RDK-25852.
 */

#include <iostream>
#include <bits/stdc++.h>
#include <string>

#include "Module.h"

#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

#define SYSSRV_CALLSIGN "org.rdk.ControlService.1"
#define SERVER_DETAILS  "127.0.0.1:9998"

using namespace std;
using namespace WPEFramework;

/* Declare module name */
MODULE_NAME_DECLARATION(BUILD_REFERENCE)

/* This section can be used for API validation logic. */
void showMenu()
{
    std::cout<<"\n\nControlService API Methods: \n";
    std::cout<<"0.getQuirks\n";
    std::cout<<"1.getApiVersionNumber\n";
    std::cout<<"2.getAllRemoteData\n";
    std::cout<<"3.getSingleRemoteData\n";
    std::cout<<"4.getLastPairedRemoteData\n";
    std::cout<<"5.getLastKeypressSource\n";
    std::cout<<"6.setValues\n";
    std::cout<<"7.getValues\n";
    std::cout<<"8.startPairingMode\n";
    std::cout<<"9.endPairingMode\n";
    std::cout<<"10.canFindMyRemote\n";
    std::cout<<"11.findLastUsedRemote\n";
    std::cout<<"12.checkRf4ceChipConnectivity\n";
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

/***
 * @brief  : compare two C string case insensitively
 * @param1[in] : c string 1
 * @param2[in] : c string 2
 * @return : <bool>; 0 if strings are same, some number if strings are different
 */
int strcicmp(char const *a, char const *b)
{
    int d = -1;
    for (;; a++, b++) {
        d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a) {
            return d;
        }
    }
    return -1;
}

/* This section is related to the event handler implementation for ControlService Plugin Events. */

namespace Handlers {
    /* Event Handlers */
    static void onControl(const Core::JSON::String& parameters) {
        std::string message;
        parameters.ToString(message);
        std::cout << "\n[ControlSvcEvt] " << __FUNCTION__ << ": " << message << std::endl;
    }
    static void onXRPairingStart(const Core::JSON::String& parameters) {
        std::string message;
        parameters.ToString(message);
        std::cout << "\n[ControlSvcEvt] " << __FUNCTION__ << ": " << message << std::endl;
    }
    static void onXRValidationUpdate(const Core::JSON::String& parameters) {
        std::string message;
        parameters.ToString(message);
        std::cout << "\n[ControlSvcEvt] " << __FUNCTION__ << ": " << message << std::endl;
    }
    static void onXRValidationComplete(const Core::JSON::String& parameters) {
        std::string message;
        parameters.ToString(message);
        std::cout << "\n[ControlSvcEvt] " << __FUNCTION__ << ": " << message << std::endl;
    }
    static void onXRConfigurationComplete(const Core::JSON::String& parameters) {
        std::string message;
        parameters.ToString(message);
        std::cout << "\n[ControlSvcEvt] " << __FUNCTION__ << ": " << message << std::endl;
    }
}

int main(int argc, char** argv)
{
    JSONRPC::LinkType<Core::JSON::IElement> * remoteObject = NULL;

    int choice;
    uint32_t ret;
    string json;
    string cmd;
    string lastCmd;
    int remoteId = 255;
    int timeOutPeriod = 0;
    int pairingMode = 255;
    int restrictPairing;
    bool bNeedExtraLineRead = false;


    Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(SERVER_DETAILS)));

    // Security Token
    std::cout << "Retrieving security token" << std::endl;
    std::string sToken;

    FILE *pSecurity = popen("/usr/bin/WPEFrameworkSecurityUtility", "r");
    if(pSecurity) {
        JsonObject pSecurityJson;
        std::string pSecurityOutput;
        size_t pSecurityOutputTrimIndex;
        std::array<char, 256> pSecurityBuffer;

        while(fgets(pSecurityBuffer.data(), 256, pSecurity) != NULL) {
            pSecurityOutput += pSecurityBuffer.data();
        }
        pclose(pSecurity);

        pSecurityOutputTrimIndex = pSecurityOutput.find('{');
        if(pSecurityOutputTrimIndex == std::string::npos) {
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
            if (remoteObject->Subscribe<Core::JSON::String>(1000, _T("onControl"),
                        &Handlers::onControl) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onControl" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onControl" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::String>(1000, _T("onXRPairingStart"),
                        &Handlers::onXRPairingStart) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onXRPairingStart" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onXRPairingStart" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::String>(1000, _T("onXRValidationUpdate"),
                        &Handlers::onXRValidationUpdate) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onXRValidationUpdate" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onXRValidationUpdate" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::String>(1000, _T("onXRValidationComplete"),
                        &Handlers::onXRValidationComplete) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onXRValidationComplete" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onXRValidationComplete" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::String>(1000, _T("onXRConfigurationComplete"),
                        &Handlers::onXRConfigurationComplete) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onXRConfigurationComplete" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onXRConfigurationComplete" << std::endl;
            }

            /* API Validation Logic. */
            while (true) {
                json.clear();
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
                else return 0;

                {
                    JsonObject result;  // The Clear method can leave crud.  A new instance off the stack works better.

                    switch (choice) {
                        case 0:
                            {
                                JsonObject params;
                                string res;
                                ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                                    _T("getQuirks"), params, result);
                                std::cout<<"ControlService Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"ControlService getQuirks call - Success!\n";
                                } else {
                                    std::cout<<"ControlService getQuirks call - failed!\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;

                        case 1:
                            {
                                JsonObject params;
                                string res;
                                ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                                    _T("getApiVersionNumber"), params, result);
                                std::cout<<"ControlService Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"ControlService getApiVersionNumber call - Success!\n";
                                } else {
                                    std::cout<<"ControlService getApiVersionNumber call - failed!! \n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;

                        case 2:
                            {
                                JsonObject params;
                                string res;
                                ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                                    _T("getAllRemoteData"), params, result);
                                std::cout<<"ControlService Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                formatForDisplay(res);
                                formatColumns(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"ControlService getAllRemoteData call - Success!\n";
                                } else {
                                    std::cout<<"ControlService getAllRemoteData call - failed!\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;

                        case 3:
                            {
                                JsonObject params;
                                string res;

                                if(remoteId == 255)
                                {
                                std::cout<<"\nEnter a 'remoteId' parameter (e.g., 1 thru 9) : ";
                                std::cin>> remoteId;
                                    bNeedExtraLineRead = true;
                                }
                                params["remoteId"] = remoteId;

                                ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                                    _T("getSingleRemoteData"), params, result);
                                std::cout<<"ControlService Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                formatForDisplay(res);
                                formatColumns(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"ControlService getSingleRemoteData call - Success!\n";
                                } else {
                                    std::cout<<"ControlService getSingleRemoteData call - failed!\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;

                        case 4:
                            {
                                JsonObject params;
                                string res;
                                ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                                    _T("getLastPairedRemoteData"), params, result);
                                std::cout<<"ControlService Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                formatForDisplay(res);
                                formatColumns(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"ControlService getLastPairedRemoteData call - Success!\n";
                                } else {
                                    std::cout<<"ControlService getLastPairedRemoteData call - failed!\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;

                        case 5:
                            {
                                JsonObject params;
                                string res;
                                ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                                    _T("getLastKeypressSource"), params, result);
                                std::cout<<"ControlService Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                formatForDisplay(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"ControlService getLastKeypressSource call - Success!\n";
                                } else {
                                    std::cout<<"ControlService getLastKeypressSource call - failed!\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;

                        case 6:
                            {
                                JsonObject params;
                                string res;
                                int param_choice;

                                std::cout<<"\nsetValue parameters: \n";
                                std::cout<<"1. enableASB\n";
                                std::cout<<"2. enableOpenChime\n";
                                std::cout<<"3. enableCloseChime\n";
                                std::cout<<"4. enablePrivacyChime\n";
                                std::cout<<"5. conversationalMode\n";
                                std::cout<<"6. chimeVolume\n";
                                std::cout<<"7. irCommandRepeats\n\n";
                                std::cout<<"\nEnter a choice for parameter to set: ";

                                std::cin >> param_choice;
                                bNeedExtraLineRead = true;

                                std::cout<<"enter value ";

                                switch (param_choice) {
                                    case 1:
                                        {
                                            string strValue;
                                            bool value = false;
                                            std::cout<<"(true, false) --> ";
                                            std::cin>> strValue;
                                            if ("1" == strValue) {
                                                istringstream("1") >> value;
                                            }
                                            else if (!strcicmp("true", strValue.c_str())) {
                                                istringstream("true") >> std::boolalpha >> value;
                                            }
                                            params["enableASB"] = value;
                                        }
                                        break;

                                    case 2:
                                        {
                                            string strValue;
                                            bool value = false;
                                            std::cout<<"(true, false) --> ";
                                            std::cin>> strValue;
                                            if ("1" == strValue) {
                                                istringstream("1") >> value;
                                            }
                                            else if (!strcicmp("true", strValue.c_str())) {
                                                istringstream("true") >> std::boolalpha >> value;
                                            }
                                            params["enableOpenChime"] = value;
                                        }
                                        break;

                                    case 3:
                                        {
                                            string strValue;
                                            bool value = false;
                                            std::cout<<"(true, false) --> ";
                                            std::cin>> strValue;
                                            if ("1" == strValue) {
                                                istringstream("1") >> value;
                                            }
                                            else if (!strcicmp("true", strValue.c_str())) {
                                                istringstream("true") >> std::boolalpha >> value;
                                            }
                                            params["enableCloseChime"] = value;
                                        }
                                        break;

                                    case 4:
                                        {
                                            string strValue;
                                            bool value = false;
                                            std::cout<<"(true, false) --> ";
                                            std::cin>> strValue;
                                            if ("1" == strValue) {
                                                istringstream("1") >> value;
                                            }
                                            else if (!strcicmp("true", strValue.c_str())) {
                                                istringstream("true") >> std::boolalpha >> value;
                                            }
                                            params["enablePrivacyChime"] = value;
                                        }
                                        break;

                                    case 5:
                                        {
                                            int value;
                                            std::cout<<"(e.g., 0 thru 6 ) --> ";
                                            std::cin>> value;
                                            params["conversationalMode"] = value;
                                        }
                                        break;

                                    case 6:
                                        {
                                            int value;
                                            std::cout<<"(e.g., 0 thru 2 ) --> ";
                                            std::cin>> value;
                                            params["chimeVolume"] = value;
                                        }
                                        break;

                                    case 7:
                                        {
                                            int value;
                                            std::cout<<"(e.g., 1 thru 10 ) --> ";
                                            std::cin>> value;
                                            params["irCommandRepeats"] = value;
                                        }
                                        break;

                                    default:
                                        break;

                                }

                                params.ToString(json);
                                std::cout<<"\nsetValues request params: "<<json.c_str()<<"\n";

                                ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                                    _T("setValues"), params, result);
                                std::cout<<"ControlService Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"\nControlService setValues: success\n";
                                } else {
                                    std::cout<<"\nControlService setValues: failure\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;

                        case 7:
                            {
                                JsonObject params;
                                string res;
                                ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                                    _T("getValues"), params, result);
                                std::cout<<"ControlService Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                formatForDisplay(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"ControlService getValues call - Success!\n";
                                } else {
                                    std::cout<<"ControlService getValues call - failed!\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;

                        case 8:
                            {
                                JsonObject params;
                                string res;

                                if(pairingMode == 255)
                                {
                                std::cout<<"\nEnter a 'pairingMode' integer [BUTTON_BUTTON(0), SCREENBIND(1), ONE_PRESS_AUTO(2)] : ";
                                std::cin>> pairingMode;
                                    bNeedExtraLineRead = true;

                                std::cout<<"\nEnter a 'restrictPairing' integer [NO_RESTRICTIONS(0), VOICE_REMOTES_ONLY(1), VOICE_ASSISTANTS_ONLY(2)] : ";
                                std::cin>> restrictPairing;
                                }
                                params["pairingMode"] = pairingMode;
                                params["restrictPairing"] = restrictPairing;

                                ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                                    _T("startPairingMode"), params, result);
                                std::cout<<"ControlService Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"ControlService startPairingMode call - Success!\n";
                                } else {
                                    std::cout<<"ControlService startPairingMode call - failed!\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;

                        case 9:
                            {
                                JsonObject params;
                                string res;
                                ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                                    _T("endPairingMode"), params, result);
                                std::cout<<"ControlService Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"ControlService endPairingMode call - Success!\n";
                                } else {
                                    std::cout<<"ControlService endPairingMode call - failed!\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;

                        case 10:
                            {
                                JsonObject params;
                                string res;
                                ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                                    _T("canFindMyRemote"), params, result);
                                std::cout<<"ControlService Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"ControlService canFindMyRemote call - Success!\n";
                                } else {
                                    std::cout<<"ControlService canFindMyRemote call - failed!\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;

                        case 11:
                            {
                                JsonObject params;
                                string res;

                                if(timeOutPeriod == 0)
                                {
                                    std::cout<<"\nEnter a 'timeout period' [5 - 30] : ";
                                    std::cin>> timeOutPeriod;
                                    bNeedExtraLineRead = true;
                                }

                                params["timeOutPeriod"] = timeOutPeriod;

                                ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                                    _T("findLastUsedRemote"), params, result);
                                std::cout<<"ControlService Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"ControlService findLastUsedRemote call - Success!\n";
                                } else {
                                    std::cout<<"ControlService findLastUsedRemote call - failed!\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;

                        case 12:
                            {
                                JsonObject params;
                                string res;
                                ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                                    _T("checkRf4ceChipConnectivity"), params, result);
                                std::cout<<"ControlService Invoke ret : "<< ret <<"\n";
                                result.ToString(res);
                                if (result["success"].Boolean()) {
                                    std::cout<<"ControlService checkRf4ceChipConnectivity call - Success!\n";
                                } else {
                                    std::cout<<"ControlService checkRf4ceChipConnectivity call - failed!\n";
                                }
                                std::cout<<"result : "<<res<<"\n";
                            }
                            break;

                        default:
                            std::cout<<"Entry not recognized!\n";
                            break;

                    }
                }

                std::cout<<"\n\nTo continue press ENTER; To quit press any other key --> ";

                if(bNeedExtraLineRead)
                {
                if (std::cin.peek() == '\n')
                    {
                    std::getline(std::cin, cmd);
                    }
                    bNeedExtraLineRead = false;
                }
                else
                    cmd.clear();

                std::getline(std::cin, cmd);
                if (cmd.empty())
                {
                    remoteId = 255;
                    timeOutPeriod = 0;
                    pairingMode = 255;
                    cmd.clear();
                    lastCmd.clear();
                }
                else if ((cmd[0] == 'r') || (cmd[0] == 'R'))
                    cmd = lastCmd;
                else if ((cmd[0] >= '0') && (cmd[0] <= '9'))
                {
                    remoteId = 255;
                    timeOutPeriod = 0;
                    pairingMode = 255;
                    lastCmd = cmd;
                }
                else
                    break;
            }
        }
    }

    return 0;
}
