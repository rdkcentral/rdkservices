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

#pragma once

#include <cjson/cJSON.h>
#include <string>
#include <map>
#include <mutex>
#include "utils.h"
#include "NetUtilsNetlink.h"

namespace WPEFramework {
    #define MAX_COMMAND_LENGTH      256
    #define MAX_OUTPUT_LENGTH       128

    namespace Plugin {
        class Network;

        class NetUtils {
        private:
            void _loadInterfaceDescriptions();

        public:
            NetUtils();
            virtual ~NetUtils();

            void InitialiseNetUtils();
            const std::string& getInterfaceDescription(const std::string name);

            static bool isIPV4(const std::string &address);
            static bool isIPV6(const std::string &address);
            static int execCmd(const char *command, std::string &output, bool *result = NULL, const char *outputfile = NULL);
            static bool getFile(const char *filepath, std::string &contents, bool deleteFile = false);

            static bool envGetValue(const char *key, std::string &value);

            static void getTmpFilename(const char *in, std::string &out);

        private:
            static unsigned int     m_counter;
            static std::mutex       m_counterProtect;
            std::map<std::string, std::string> interface_descriptions;
        };
    } // namespace Plugin
} // namespace WPEFramework
