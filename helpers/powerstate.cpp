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

/*
##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2020 RDK Management
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
##########################################################################
*/

/**
* @defgroup servicemanager
* @{
* @defgroup src
* @{
**/


#include "powerstate.h"
#include <time.h>

#ifdef __cplusplus
extern "C" {
#include "libIBus.h"
#include "pwrMgr.h"
}
#endif

static CPowerState s_instance;

CPowerState* CPowerState::instance()
{
    return &s_instance;
}

bool CPowerState::start()
{
    return true;
}

bool CPowerState::stop()
{
    return true;
}

bool CPowerState::setPowerState(std::string powerState)
{
    IARM_Bus_PWRMgr_SetPowerState_Param_t param;
    //if (powerState.compare("standby", Qt::CaseInsensitive) == 0)
    if (powerState == "STANDBY")
    {
        param.newState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY;
    }
    //else if (powerState.compare("on", Qt::CaseInsensitive) == 0)
    else if (powerState == "ON")
    {
        param.newState = IARM_BUS_PWRMGR_POWERSTATE_ON;
    }
    //else if (powerState.compare("DEEP_SLEEP", Qt::CaseInsensitive) == 0)
    else if (powerState == "DEEP_SLEEP")
    {
        param.newState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP;
    }
    //else if (powerState.compare("LIGHT_SLEEP", Qt::CaseInsensitive) == 0)
    else if (powerState == "LIGHT_SLEEP")
    {
        /* Treat Light sleep as Standby */
        //param.newState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_LIGHT_SLEEP;
        param.newState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY;
    }
    else
    {
        /* powerState can be one of "ON" or "STANDBY". */
        return false;
    }

    IARM_Result_t res = IARM_Bus_Call(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_API_SetPowerState,
                  (void *)&param, sizeof(param));

    if(res == IARM_RESULT_SUCCESS)
        return true;
    else
        return false;
}

std::string CPowerState::getPowerState()
{
    std::string currentState = "UNKNOWN";
    IARM_Bus_PWRMgr_GetPowerState_Param_t param;
    IARM_Result_t res = IARM_Bus_Call(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_API_GetPowerState,
            (void *)&param, sizeof(param));

    if (res == IARM_RESULT_SUCCESS) {
        if (param.curState == IARM_BUS_PWRMGR_POWERSTATE_ON)
        currentState = "ON";
        else if ((param.curState == IARM_BUS_PWRMGR_POWERSTATE_STANDBY) || 
                 (param.curState == IARM_BUS_PWRMGR_POWERSTATE_STANDBY_LIGHT_SLEEP) ||
                 (param.curState == IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP) )
        currentState = "STANDBY";
    }

    return currentState;
}

/** @} */
/** @} */
