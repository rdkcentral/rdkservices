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

#include <thread>
#include <chrono>

using namespace std;

class cTimer{
    private:
        bool clear;
        int interval;
        void (*callBack_function)();
    public:
        /***
         * @brief    : Constructor.
         * @return   : nil.
         */
        cTimer();

        /***
         * @brief    : Destructor.
         * @return   : nil.
         */
        ~cTimer();

        /***
         * @brief    : start timer thread.
         * @return   : <bool> False if timer thread couldn't be started.
         */
        bool start();

        /***
         * @brief   : stop timer thread.
         * @return   : nil
         */
        void stop();

        /***
         * @brief        : Set interval in which the given function should be invoked.
         * @param1[in]   : function which has to be invoked on timed intervals
         * @param2[in]   : timer interval val.
         * @return       : nil
         */
        void setInterval(void (*function)(), int val);
};

