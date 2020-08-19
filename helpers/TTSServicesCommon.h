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

#pragma once
#include "TTSCommon.h"
#include "utils.h"

#define API_VERSION_NUMBER 1
namespace WPEFramework {
    namespace Plugin {
        /* @brief         : Used to log TTS Response from functions.. 
        * @param[in]     : "X" - TTS_Error returned by the function
        * 
        * @param[out]    : "response" - The json object, which is return by the Api.
        *                   Sets the value of response["TTS_Status"].
        */
        void logResponse(TTS::TTS_Error X, JsonObject& response)
        {   
            response["TTS_Status"] = static_cast<uint32_t>(X);
		switch (X){
                    case (TTS::TTS_OK):
                                LOGINFO("%s api operational success with TTS_OK code = %d", __func__, X);
                                break;
                    case (TTS::TTS_FAIL):
                                LOGINFO("%s api failed with TTS_FAIL error code = %d", __func__, X);
                                break;
                    case (TTS::TTS_NOT_ENABLED):
                                LOGINFO("%s api failed with TTS_NOT_ENABLED error code = %d", __func__, X);
                                break;
                    case (TTS::TTS_INVALID_CONFIGURATION):
                                LOGINFO("%s api failed with TTS_INVALID_CONFIGURATION error code = %d", __func__, X);
                                break;
                    default:
                                LOGINFO("%s api failed with unknow error code = %d", __func__, X);
                                response["TTS_Status"] = static_cast<uint32_t>(TTS::TTS_FAIL);
            }
        }
    }//namespace Plugin
}//namespace WPEFramework


