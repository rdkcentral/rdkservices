/* If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2023 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
*/

#ifndef L2TEST_CONTROLLER_H
#define L2TEST_CONTROLLER_H

using namespace WPEFramework;

class L2testController
{
    private:
        static L2testController* instancePtr;
        L2testController(){
           m_fp = NULL;
        }
        ~L2testController() {
        }
        /** @brief Pipe file to send commands to the WPEFramework process. */
        FILE *m_fp;

   public:
       // deleting copy constructor
         L2testController(const L2testController& obj) = delete;
         L2testController& operator=(const L2testController&) = delete;

        /**
         * @brief Getting the class instance
         *
         */
        static L2testController* getInstance();

        /**
         * @brief Spawn Thunder process in isolation
         *
         */
        bool StartThunder();

        /**
         * @brief Stops thunder
         *
         */
        void StopThunder();

        /**
         * @brief Perform L2 test, calls first method of L2Tests plugin
         *
         * @param[in] method Method name
         * @param[in] params Method parameters
         * @param[out] results Method results
         * @return Zero (Core::ERROR_NONE) on succes or another value on error
         */
       uint32_t PerformL2Tests(JsonObject &params, JsonObject &results);

};

#endif /* L2TEST_CONTROLLER_H */
