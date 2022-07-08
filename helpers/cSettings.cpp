/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/


#include <iostream>
#include <fstream>
#include "cSettings.h"
#include "utils.h"

/***
 * @brief    : Constructor.
 * @return  : nil.
 */
cSettings::cSettings(std::string file)
{
    filename = file;
    if (!readFromFile()) {
        /* File not present; create a new one assuming a fresh partition. */
        std::fstream fs;
        fs.open(filename.c_str(), std::fstream::in|std::fstream::out|std::fstream::app);
        if (!fs.is_open()) {
            std::cout << "Error:[ctor cSettings] unable to open configuration file." << std::endl;
        } else {
            fs << flush;
            fs.close();
        }
    }
}

/***
 * @brief    : Destructor.
 * @return  : nil.
 */
cSettings::~cSettings()
{
    ;
}

/***
 * @brief    : Initialise the jsonobject from a given conf file.
 * @return  : <bool> False if file couldn't be accessed, else True.
 */
bool cSettings::readFromFile()
{
    bool retStatus = false;
    std::string content;
    if (!Utils::fileExists(filename.c_str())) {
        return retStatus;
    }
    fstream ifile(filename,ios::in);
    if (ifile) {
        while (!ifile.eof()) {
            std::getline(ifile,content);
            size_t pos = content.find_last_of("=");
            if (std::string::npos != pos) {
                data[(content.substr(0, pos).c_str())] = content.substr(pos+1,std::string::npos);
            }
            retStatus = true;
        }
    } else {
        //Do nothing.
    }
    return retStatus;
}

/***
 * @brief    : Update new inserts into the json object onto file.
 * @return  : <bool> False if timer thread couldn't be started.
 */
bool cSettings::writeToFile()
{
    bool status = false;

    if (Utils::fileExists(filename.c_str())) {
        ofstream ofile;
        ofile.open(filename.c_str(),ios::out);
        if (ofile) {
            JsonObject::Iterator iterator = data.Variants();
            while (iterator.Next()) {
                if (!data[iterator.Label()].String().empty()) {
                    ofile << iterator.Label() << "=" << data[iterator.Label()].String() << endl;
                } else {
                    continue;
                }
            }
            status = true;
            ofile.close();
        } else {
            status = false;
        }
    }
    return status;
}

/***
 * @brief        : Get value of given key.
 * @param1[in]  : <string> key
 * @return     : <JsonValue> the value to the corresponding key
 */
JsonValue cSettings::getValue(std::string key)
{
    return data.Get(key.c_str());
}

/***
 * @brief        : Set value of given key.
 * @param1[in]  : <string> key
 * @param2[in] : <string> value
 * @return        : <bool> True if setvalue successfull, else False
 */
bool cSettings::setValue(std::string key,std::string value)
{
    data[key.c_str()] = value;
    return writeToFile();
}

/***
 * @brief        : Set value of given key.
 * @param1[in]  : <string> key
 * @param2[in] : <int> value
 * @return        : <bool> True if setvalue successfull, else False
 */
bool cSettings::setValue(std::string key,int value)
{
    data[key.c_str()] = value;
    return writeToFile();
}

/***
 * @brief        : Set value of given key.
 * @param1[in]  : <string> key
 * @param2[in] : <bool> value
 * @return        : <bool> True if setvalue successfull, else False
 */
bool cSettings::setValue(std::string key,bool value)
{
    data[key.c_str()] = value;
    return writeToFile();
}

/***
 * @brief        : Check if a particular key is set.
 * @param1[in]  : <string> key
 * @return     : <bool> True if key is already set, else False
 */
bool cSettings::contains(std::string key)
{
    bool resp = false;
    if (data.HasLabel(key.c_str())) {
        if (data[key.c_str()].String().empty()) {
            resp = false;
        } else {
            resp = true;
        }
    } else {
        resp = false;
    }
    return resp;
}

/***
 * @brief        : Remove a particular key-value pair.
 * @param1[in]  : <string> key
 * @return     : <bool> True if key is key-value pair removed, else False
 */
bool cSettings::remove(std::string key)
{
    bool status = false;
    /*
     * Noticed that there is an error with the Remove function.
     * work around is to assign a null value to the key and handle it
     * accordingly.
     */
    data[key.c_str()] = "";
    data.Remove(key.c_str());
    if (!contains(key)) {
        if (writeToFile()) {
            status = true;
        } else {
            status = false;
        }
    }
    else {
        status = false;
    }
    return status;
}

