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

#ifndef CONTROLSETTINGS_H
#define CONTROLSETTINGS_H

#include <string>
#include "libIARM.h"
#include "libIBusDaemon.h"
#include "libIBus.h"
#include "iarmUtil.h"

#include "dsError.h"
#include "dsMgr.h"
#include "hdmiIn.hpp"

#include "bl_table.h"
#include "als_bl_iniparser.h"
#include <iostream>

#include "tvTypes.h"
#include "tvLog.h"
#include "tvSettings.h"
#include <pthread.h>
#include "Module.h"
#include "utils.h"
#include "tvTypes.h"
#include "tvError.h"

#include "tr181api.h"
#include <sys/stat.h>
#include <vector>
#include "ControlSettingsTV.h"
#include "ControlSettingsSTB.h"

namespace WPEFramework {
namespace Plugin {

    class ControlSettings : public DEVICE_TYPE {

    private:
        ControlSettings(const ControlSettings&) = delete;
        ControlSettings& operator=(const ControlSettings&) = delete;

	DECLARE_JSON_RPC_METHOD(getAspectRatio)
        DECLARE_JSON_RPC_METHOD(setAspectRatio)
        DECLARE_JSON_RPC_METHOD(getVideoFormat)
        DECLARE_JSON_RPC_METHOD(getVideoFrameRate)
        DECLARE_JSON_RPC_METHOD(getVideoResolution)

   public:
        ControlSettings();
        ~ControlSettings();
        static ControlSettings* _instance;
	void NotifyVideoFormatChange(tvVideoHDRFormat_t format);
        void NotifyVideoResolutionChange(tvResolutionParam_t resolution);
        void NotifyVideoFrameRateChange(tvVideoFrameRate_t frameRate);

    private:
        uint8_t _skipURL;
        int m_currentHdmiInResoluton;
        int m_videoZoomMode;
        bool m_isDisabledHdmiIn4KZoom;
	void InitializeIARM();
        void DeinitializeIARM();
        bool isIARMConnected();
        bool IARMinit();
        static void dsHdmiVideoModeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
        static void dsHdmiStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);


    public:
        //   IPlugin methods
        // -------------------------------------------------------------------------------------------------------
        const std::string Initialize(PluginHost::IShell* service);
        void Deinitialize(PluginHost::IShell* service);
   };
}
}
#endif
