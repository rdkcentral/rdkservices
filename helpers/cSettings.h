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

#include <string>
#include <stdlib.h>
#include <plugins/plugins.h>

using namespace std;

class cSettings {
    std::string filename;
    JsonObject data;
    public:
    /***
     * @brief    : Constructor.
     * @return   : nil.
     */
    cSettings(std::string file);

    /***
     * @brief    : Destructor.
     * @return   : nil.
     */
    ~cSettings();

    /***
     * @brief        : Get value of given key.
     * @param1[in]   : <string> key
     * @return       : <JsonValue> the value to the corresponding key
     */
    JsonValue getValue(std::string key);

    /***
     * @brief        : Set value of given key.
     * @param1[in]   : <string> key
     * @param2[in]   : <string> value
     * @return       : <bool> True if setvalue successfull, else False
     */
    bool setValue(std::string key,std::string value);

    /***
     * @brief        : Set value of given key.
     * @param1[in]   : <string> key
     * @param2[in]   : <int> value
     * @return       : <bool> True if setvalue successfull, else False
     */
    bool setValue(std::string key,int value);

    /***
     * @brief        : Set value of given key.
     * @param1[in]   : <string> key
     * @param2[in]   : <bool> value
     * @return       : <bool> True if setvalue successfull, else False
     */
    bool setValue(std::string key,bool value);

    /***
     * @brief        : Check if a particular key is set.
     * @param1[in]   : <string> key
     * @return       : <bool> True if key is already set, else False
     */
    bool contains(std::string key);

    /***
     * @brief        : Remove a particular key-value pair.
     * @param1[in]   : <string> key
     * @return       : <bool> True if key is key-value pair removed, else False
     */
    bool remove(std::string key);

    /***
     * @brief    : Update new inserts into the json object onto file.
     * @return   : <bool> False if timer thread couldn't be started.
     */
    bool writeToFile();

    /***
     * @brief    : Initialise the jsonobject from a given conf file.
     * @return   : <bool> False if file couldn't be accessed, else True.
     */
    bool readFromFile();
};

