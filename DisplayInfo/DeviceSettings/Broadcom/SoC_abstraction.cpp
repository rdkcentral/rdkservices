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

#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

#include "../../Module.h"
#include "UtilsLogging.h"

#define GRAPHICS_RESOLUTION_LOC 4
#define WIDTH 0
#define HEIGHT 1
#define PROC_BRCM_CORE_PATH "/proc/brcm/core"
#define PROC_BRCM_DISPLAY_PATH "/proc/brcm/display"

using namespace std;
using namespace WPEFramework;

/**
 * Sanitizeline
 *
 * @brief   : Remove extra white spaces in between words
 *
 * @param   : line to santize.
 * @return  : void
 */
void sanitizeLine(string& line)
{
    for(unsigned int i = 0; i < line.size(); i++)
    {
        while(line[i] == ' ' && line[i+1] == ' ')
        {
            line.erase(i,1);
        }
    }
}

/**
 * tokenize
 *
 * @brief   : split line into tokens using delimitter
 *
 * @param   : line
 * @param   : tokens array
 * @param   : delimitter
 * @return  : void
 */
void tokenize(string line, vector<string>& tokens, char delim)
{
    istringstream iss(line);
    string token;
    while (getline(iss, token, delim))
    {
        tokens.push_back(token);
    }
}

/**
 * parse_proc_brcm_core
 *
 * @brief   : parse profs to obtain GPU ram details
 *
 * @param   : columnHeader
 * @param   : bufferName
 * @return  : value
 */
string parse_proc_brcm_core(string columnHeader)
{
    ifstream procfile(PROC_BRCM_CORE_PATH);
    string line, value;
    vector<string> tokens;
    int column = 0;

    if (procfile.is_open())
    {
        while (getline(procfile,line))
        {
            if(line.find(columnHeader) != string::npos)
            {
                tokens.clear();
                sanitizeLine(line);
                tokenize(line, tokens, ' ');
                column = find(tokens.begin(), tokens.end(), columnHeader) - tokens.begin(); //found the column where the data can be found
            }
            if(line.find("GFX") != string::npos)
            {
                tokens.clear();
                sanitizeLine(line);
                tokenize(line, tokens, ' ');
                value = tokens.at(column); //fetch the value in the corresponding column
            }
        }
        procfile.close();
    }
    else LOGERR("could not open file %s", PROC_BRCM_CORE_PATH);
    return value;
}

/**
 * parse_proc_brcm_display
 *
 * @brief   : parse profs to obtain Graphics resolution details
 * @return  : value
 */
string parse_proc_brcm_display()
{
    ifstream procfile(PROC_BRCM_DISPLAY_PATH);
    string line, value;
    vector<string> tokens;
    if (procfile.is_open())
    {
        while (getline(procfile,line))
        {
            if(line.find("graphics") != string::npos)
            {
                tokens.clear();
                sanitizeLine(line);
                tokenize(line, tokens, ' ');
                if (tokens.size() >= GRAPHICS_RESOLUTION_LOC )
                    value = tokens.at(GRAPHICS_RESOLUTION_LOC);
            }
        }
        procfile.close();
    }
    else LOGERR("could not open file %s", PROC_BRCM_DISPLAY_PATH);
    return value;
}

uint64_t SoC_GetTotalGpuRam()
{
    string value = parse_proc_brcm_core("MB");
    uint64_t ret = 0;
    try
    {
        ret = stoi(value) * 1024 * 1024;
        LOGINFO("total GPU ram returned from proc = %s MB", value.c_str());
    }
    catch(...)
    {
        LOGERR("Unable to process Total Gpu ram");
    }
    return ret;
}

uint64_t SoC_GetFreeGpuRam()
{
    string value = parse_proc_brcm_core("used");
    uint64_t ret = 0;
    int usedPercentage = 0;
    try
    {
        usedPercentage = stoi(value);
        LOGINFO("percentage of GPU memory used = %s", value.c_str());
    }
    catch(...)
    {
        LOGERR("Unable to process Free Gpu ram");
    }

    ret = (100 - usedPercentage) * 0.01 * SoC_GetTotalGpuRam();
    return ret;
}

uint32_t SoC_GetGraphicsWidth()
{
    vector<string> resolution;
    uint32_t ret = 0;
    string value = parse_proc_brcm_display();
    LOGINFO("graphics plane dimensions returned from proc = %s" , value.c_str());
    tokenize(value, resolution, 'x'); // graphics resolution is in the format 1280x720
    if(resolution.size() > WIDTH)
    {
        try
        {
            ret = stoi(resolution.at(WIDTH));
            LOGINFO("graphics plane width = %d", ret);
        }
        catch(...)
        {
            LOGERR("Unable to process gfx plane width");
        }
    }

    return ret;
}

uint32_t SoC_GetGraphicsHeight()
{
    vector<string> resolution;
    uint32_t ret = 0;
    string value = parse_proc_brcm_display();
    LOGINFO("graphics plane dimensions returned from proc = %s" , value.c_str());
    tokenize(value, resolution, 'x'); // graphics resolution is in the format 1280x720
    if(resolution.size() > HEIGHT)
    {
        try
        {
            ret = stoi(resolution.at(HEIGHT));
            LOGINFO("graphics plane height = %d", ret);
        }
        catch(...)
        {
            LOGERR("Unable to process gfx plane height");
        }
    }
    return ret;
}