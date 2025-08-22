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

#include "cTimer.h"

/***
 * @brief : Constructor.
 * @return   : nil.
 */
cTimer::cTimer()
{
    clear = false;
    active = false;
    interval = 0;
}

/***
 * @brief : Destructor.
 * @return   : nil.
 */
cTimer::~cTimer()
{
    this->clear = true;
}

/***
Running this timer function as thread function
*/
void cTimer::timerFunction() {
    this->active = true;
    while (true) {
        if (this->clear) {
            this->active = false;
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        if (this->clear) {
            this->active = false;
            return;
        }
        this->callBack_function();
    }
}
/***
 * @brief : start timer thread.
 * @return   : <bool> False if timer thread couldn't be started.
 */
bool cTimer::start()
{
    if (interval <= 0 && callBack_function == NULL) {
        return false;
    }
    this->clear = false;
    timerThread = std::thread(&cTimer::timerFunction, this);
    return true;
}

/***
 * @brief : stop timer thread.
 * @return   : nil
 */
void cTimer::stop()
{
    this->clear = true;
}

bool cTimer::isActive()
{
    return this->active;
}

void cTimer::detach()
{
    timerThread.detach();
}

void cTimer::join()
{
    if (timerThread.joinable()) {
        timerThread.join();
    }
}

/***
 * @brief     : Set interval in which the given function should be invoked.
 * @param1[in]   : function which has to be invoked on timed intervals
 * @param2[in]  : timer interval val.
 * @return     : nil
 */
void cTimer::setInterval(void (*function)(), int val)
{
    this->callBack_function = function;
    this->interval = val;
}
