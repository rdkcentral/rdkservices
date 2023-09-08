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

#ifndef AVOutputTV_H
#define AVOutputTV_H

#include "string.h"

#include "tvTypes.h"
#include "tvLog.h"
#include "tvSettings.h"
#include <pthread.h>
#include "Module.h"
#include "tvError.h"
#include "tvTypes.h"
#include "tr181api.h"
#include "AVOutputCommon.h"
#include "libIARM.h"
#include "libIBusDaemon.h"
#include "libIBus.h"
#include "iarmUtil.h"
#include "UtilsLogging.h"
#include "UtilsJsonRpc.h"
#include "dsError.h"
#include "dsMgr.h"
#include "hdmiIn.hpp"

namespace WPEFramework {
namespace Plugin {

class AVOutputTV : public PluginHost::IPlugin, public PluginHost::JSONRPC {
    private:
        AVOutputTV(const AVOutputTV&) = delete;
        AVOutputTV& operator=(const AVOutputTV&) = delete;
    public:
	DECLARE_JSON_RPC_METHOD(getBacklight)
        DECLARE_JSON_RPC_METHOD(setBacklight)
        DECLARE_JSON_RPC_METHOD(resetBacklight)
        DECLARE_JSON_RPC_METHOD(getBacklightCaps)
        DECLARE_JSON_RPC_METHOD(setBrightness)
        DECLARE_JSON_RPC_METHOD(getBrightness )
        DECLARE_JSON_RPC_METHOD(resetBrightness )
	DECLARE_JSON_RPC_METHOD(getBrightnessCaps)
        DECLARE_JSON_RPC_METHOD(setContrast )
        DECLARE_JSON_RPC_METHOD(getContrast )
        DECLARE_JSON_RPC_METHOD(resetContrast )
	DECLARE_JSON_RPC_METHOD(getContrastCaps)
        DECLARE_JSON_RPC_METHOD(setSharpness )
        DECLARE_JSON_RPC_METHOD(getSharpness )
        DECLARE_JSON_RPC_METHOD(resetSharpness )
	DECLARE_JSON_RPC_METHOD(getSharpnessCaps)
        DECLARE_JSON_RPC_METHOD(setSaturation )
        DECLARE_JSON_RPC_METHOD(getSaturation )
        DECLARE_JSON_RPC_METHOD(resetSaturation )
	DECLARE_JSON_RPC_METHOD(getSaturationCaps)
        DECLARE_JSON_RPC_METHOD(setHue )
        DECLARE_JSON_RPC_METHOD(getHue )
        DECLARE_JSON_RPC_METHOD(resetHue )
	DECLARE_JSON_RPC_METHOD(getHueCaps)
        DECLARE_JSON_RPC_METHOD(setColorTemperature )
        DECLARE_JSON_RPC_METHOD(getColorTemperature )
        DECLARE_JSON_RPC_METHOD(resetColorTemperature )
	DECLARE_JSON_RPC_METHOD(getColorTemperatureCaps)
	DECLARE_JSON_RPC_METHOD(getComponentCaps )
        DECLARE_JSON_RPC_METHOD(getComponentSaturation )
        DECLARE_JSON_RPC_METHOD(setComponentSaturation )
        DECLARE_JSON_RPC_METHOD(resetComponentSaturation )
        DECLARE_JSON_RPC_METHOD(getComponentHue )
        DECLARE_JSON_RPC_METHOD(setComponentHue )
        DECLARE_JSON_RPC_METHOD(resetComponentHue )
        DECLARE_JSON_RPC_METHOD(getComponentLuma )
        DECLARE_JSON_RPC_METHOD(setComponentLuma )
        DECLARE_JSON_RPC_METHOD(resetComponentLuma )
        DECLARE_JSON_RPC_METHOD(getBacklightDimmingMode )
        DECLARE_JSON_RPC_METHOD(setBacklightDimmingMode )
        DECLARE_JSON_RPC_METHOD(resetBacklightDimmingMode )
	DECLARE_JSON_RPC_METHOD(getDimmingModeCaps )
        DECLARE_JSON_RPC_METHOD(getAutoBacklightControl )
        DECLARE_JSON_RPC_METHOD(setAutoBacklightControl )
        DECLARE_JSON_RPC_METHOD(resetAutoBacklightControl )
	DECLARE_JSON_RPC_METHOD(getAutoBacklightControlCaps )
        DECLARE_JSON_RPC_METHOD(getSupportedDolbyVisionModes )
        DECLARE_JSON_RPC_METHOD(getDolbyVisionMode )
        DECLARE_JSON_RPC_METHOD(setDolbyVisionMode )
        DECLARE_JSON_RPC_METHOD(resetDolbyVisionMode )
        DECLARE_JSON_RPC_METHOD(getDolbyVisionModeCaps )
        DECLARE_JSON_RPC_METHOD(getSupportedHDR10Modes )
        DECLARE_JSON_RPC_METHOD(getHDR10Mode )
        DECLARE_JSON_RPC_METHOD(setHDR10Mode )
        DECLARE_JSON_RPC_METHOD(resetHDR10Mode )
        DECLARE_JSON_RPC_METHOD(getHDR10ModeCaps )
        DECLARE_JSON_RPC_METHOD(getSupportedHLGModes )
        DECLARE_JSON_RPC_METHOD(getHLGMode )
        DECLARE_JSON_RPC_METHOD(setHLGMode )
        DECLARE_JSON_RPC_METHOD(resetHLGMode )
        DECLARE_JSON_RPC_METHOD(getHLGModeCaps )
        DECLARE_JSON_RPC_METHOD(getWBInfo )
        DECLARE_JSON_RPC_METHOD(getWBCtrl )
        DECLARE_JSON_RPC_METHOD(setWBCtrl )
        DECLARE_JSON_RPC_METHOD(resetWBCtrl )
        DECLARE_JSON_RPC_METHOD(getWBCaps )

        DECLARE_JSON_RPC_METHOD(getBacklight2 )
        DECLARE_JSON_RPC_METHOD(getBrightness2 )
        DECLARE_JSON_RPC_METHOD(getContrast2 )
        DECLARE_JSON_RPC_METHOD(getSharpness2 )
        DECLARE_JSON_RPC_METHOD(getSaturation2 )
        DECLARE_JSON_RPC_METHOD(getHue2 )
        DECLARE_JSON_RPC_METHOD(getColorTemperature2 )
        DECLARE_JSON_RPC_METHOD(getSupportedPictureModes )
        DECLARE_JSON_RPC_METHOD(getPictureMode )
        DECLARE_JSON_RPC_METHOD(setPictureMode )
        DECLARE_JSON_RPC_METHOD(resetPictureMode )
        DECLARE_JSON_RPC_METHOD(enableWBMode )
        DECLARE_JSON_RPC_METHOD(setBacklightFade )

        DECLARE_JSON_RPC_METHOD(getAspectRatio2)
        DECLARE_JSON_RPC_METHOD(setAspectRatio2)
        DECLARE_JSON_RPC_METHOD(resetAspectRatio)
        DECLARE_JSON_RPC_METHOD(getAspectRatioCaps)
        DECLARE_JSON_RPC_METHOD(getVideoFormat)
        DECLARE_JSON_RPC_METHOD(getVideoFrameRate)
        DECLARE_JSON_RPC_METHOD(getVideoResolution)
	DECLARE_JSON_RPC_METHOD(getLowLatencyState)
        DECLARE_JSON_RPC_METHOD(setLowLatencyState)
        DECLARE_JSON_RPC_METHOD(resetLowLatencyState)
        DECLARE_JSON_RPC_METHOD(getLowLatencyStateCaps)

    public:
        int m_currentHdmiInResoluton;
        int m_videoZoomMode;
        bool m_isDisabledHdmiIn4KZoom;
        char rfc_caller_id[RFC_BUFF_MAX];
        AVOutputTV();
        ~AVOutputTV();
        static AVOutputTV *instance;
        void Initialize();
        void Deinitialize();

	static AVOutputTV* getInstance() { return instance; }
	tvError_t setAspectRatioZoomSettings(tvDisplayMode_t mode);
        tvError_t getUserSelectedAspectRatio (tvDisplayMode_t* mode);
        tvError_t setDefaultAspectRatio(std::string pqmode="all",std::string format="all",std::string source="all");
	static void dsHdmiVideoModeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
        static void dsHdmiStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
        static void dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
	tvContentFormatType_t ConvertFormatStringToTVContentFormat(const char *format);
};

}//namespace Plugin
}//namespace WPEFramework
#endif
