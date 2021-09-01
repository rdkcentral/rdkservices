/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
* Copyright 2021 Liberty Global Service B.V.
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

#include "DisplaySettings.h"
#include "utils.h"

namespace WPEFramework {
    namespace Plugin {
        // all LGI extensions are implemented here
        class LgiDisplaySettings : public DisplaySettings
        {
            public:
                LgiDisplaySettings();
                virtual ~LgiDisplaySettings()
                {
                }

                uint32_t setOutputFrameRatePreference(const JsonObject& parameters, JsonObject& response); // args: (videoDisplay, followContent)
                uint32_t setAudioProcessingHint(const JsonObject& parameters, JsonObject& response); // args:  (audioPort, audioMode, audioDelayMs)
                uint32_t getAudioOutputEncoding(const JsonObject& parameters, JsonObject& response); //args: (audioPort)
                uint32_t getFollowColorSpace(const JsonObject& parameters, JsonObject& response); // args: (videoDisplay)
                uint32_t setFollowColorSpace(const JsonObject& parameters, JsonObject& response); // args: (videoDisplay,followColorSpace)
                uint32_t getPreferredOutputColorSpace(const JsonObject& parameters, JsonObject& response); // args: (videoDisplay)
                uint32_t setPreferredOutputColorSpace(const JsonObject& parameters, JsonObject& response); // args: (videoDisplay,colorSpaces: array of colorspace string)
                uint32_t getHDRGfxColorSpace(const JsonObject& parameters, JsonObject& response); // args: (videoDisplay)
                uint32_t setHDRGfxColorSpace(const JsonObject& parameters, JsonObject& response); // args: (videoDisplay,y,cr,cb)

            protected:
                virtual bool getDefaultActiveInput() const
                {
                    return false;
                }

                virtual bool isFollowSoundModeSupported() const
                {
                    return true;
                }

                virtual uint32_t getBassEnhancer(const JsonObject& parameters, JsonObject& response);
        };
    } //Plugin
} //WPEFramework
