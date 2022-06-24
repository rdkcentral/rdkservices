/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2022 Sky UK
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
#ifndef CONTROLSETTINGS_COMMON_H
#define CONTROLSETTINGS_COMMON_H

#define RFC_BUFF_MAX 100
#define VIDEO_DESCRIPTION_MAX (25)
#define VIDEO_DESCRIPTION_NAME_SIZE (25)

#define PIC_MODES_SUPPORTED_MAX   (15)
#define SOURCES_SUPPORTED_MAX   (15)

#define TVSETTINGS_RFC_CALLERID        "tvsettings"
#define TVSETTINGS_GENERIC_STRING_RFC_PARAM    "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings."
#define TVSETTINGS_PICTUREMODE_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.PictureMode"
#define TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.PictureModeString"
#define TVSETTINGS_BACKLIGHT_CONTROL_USE_GBF_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.UseGBFForBacklightControl"

#define STRING_PICMODE  "PicMode."
#define STRING_FORMAT  "Format."
#define STRING_DEFAULT  "Default"
#define STRING_SOURCE    "Source."



#define DECLARE_JSON_RPC_METHOD(method) \
uint32_t method(const JsonObject& parameters, JsonObject& response);

#define PLUGIN_Lock(lock) pthread_mutex_lock(&lock)
#define PLUGIN_Unlock(lock) pthread_mutex_unlock(&lock)

#define returnResponse(return_status, error_log) \
    {response["success"] = return_status; \
    if(!return_status) \
        response["error_message"] = _T(error_log); \
/*    PLUGIN_Unlock(Lock);*/ \
    return (Core::ERROR_NONE);}

#define returnIfParamNotFound(param)\
    if(param.empty())\
    {\
        LOGERR("missing parameter %s\n",#param);\
        returnResponse(false,"missing parameter");\
    }

static pthread_mutex_t Lock = PTHREAD_MUTEX_INITIALIZER;


static std::string getErrorString (tvError_t eReturn)
    {
        switch (eReturn)
        {
            case tvERROR_NONE:
                return "API SUCCESS";
            case tvERROR_GENERAL:
                return "API FAILED";
            case tvERROR_OPERATION_NOT_SUPPORTED:
                return "OPERATION NOT SUPPORTED ERROR";
            case tvERROR_INVALID_PARAM:
                return "INVALID PARAM ERROR";
            case tvERROR_INVALID_STATE:
                return "INVALID STATE ERROR";
        }
        return "UNKNOWN ERROR";
    }

#endif
