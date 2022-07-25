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

#ifndef ControlSettingsTV_H
#define ControlSettingsTV_H

#include "string.h"

#include "tvTypes.h"
#include "tvLog.h"
#include "tvSettings.h"
#include <pthread.h>
#include "Module.h"
#include "tvError.h"
#include "tvTypes.h"
#include "tr181api.h"
#include "AbstractPlugin.h"
#include "utils.h"
#include "ControlSettingsCommon.h"

namespace WPEFramework {
namespace Plugin {

class ControlSettingsTV : public AbstractPlugin {
    private:
        ControlSettingsTV(const ControlSettingsTV&) = delete;
        ControlSettingsTV& operator=(const ControlSettingsTV&) = delete;
    public:
	DECLARE_JSON_RPC_METHOD(getBacklight)
        DECLARE_JSON_RPC_METHOD(setBacklight)
        DECLARE_JSON_RPC_METHOD(resetBacklight)
        DECLARE_JSON_RPC_METHOD(setBrightness)
        DECLARE_JSON_RPC_METHOD(getBrightness )
        DECLARE_JSON_RPC_METHOD(resetBrightness )
        DECLARE_JSON_RPC_METHOD(setContrast )
        DECLARE_JSON_RPC_METHOD(getContrast )
        DECLARE_JSON_RPC_METHOD(resetContrast )
        DECLARE_JSON_RPC_METHOD(setSharpness )
        DECLARE_JSON_RPC_METHOD(getSharpness )
        DECLARE_JSON_RPC_METHOD(resetSharpness )
        DECLARE_JSON_RPC_METHOD(setSaturation )
        DECLARE_JSON_RPC_METHOD(getSaturation )
        DECLARE_JSON_RPC_METHOD(resetSaturation )
        DECLARE_JSON_RPC_METHOD(setHue )
        DECLARE_JSON_RPC_METHOD(getHue )
        DECLARE_JSON_RPC_METHOD(resetHue )
        DECLARE_JSON_RPC_METHOD(setColorTemperature )
        DECLARE_JSON_RPC_METHOD(getColorTemperature )
        DECLARE_JSON_RPC_METHOD(resetColorTemperature )
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
        DECLARE_JSON_RPC_METHOD(getAutoBacklightControl )
        DECLARE_JSON_RPC_METHOD(setAutoBacklightControl )
        DECLARE_JSON_RPC_METHOD(resetAutoBacklightControl )


    public:
        ControlSettingsTV();
        ~ControlSettingsTV();
        ControlSettingsTV *instance;
        void Initialize();
        void Deinitialize();
	int getCurrentPictureMode(char *picMode);
        tvError_t UpdatePQParamToLocalCache(const char* forParam, int source, int pqmode, int format, int value,bool setNotDelete);
	int UpdatePQParamsToCache( const char *action, const char *tr181ParamName, const char *pqmode, const char *source, const char *format, tvPQParameterIndex_t pqParamIndex, int params[] );
        uint32_t generateStorageIdentifier(std::string &key,const char * forParam,int contentFormat, int pqmode, int source);
        int GetSaveConfig(const char *pqmode, const char *source, const char *format,std::vector<int> &sources,std::vector<int> &picturemodes, std::vector<int> &formats);
	void GetParamIndex(string source,string pqmode,string format,int& sourceIndex,int& pqmodeIndex,int& formatIndex);
        int getContentFormatIndex(tvVideoHDRFormat_t formatToConvert);

    protected:
        int numberModesSupported;
        bool appUsesGlobalBackLightFactor;
        char rfc_caller_id[RFC_BUFF_MAX];
        int pic_mode_index[PIC_MODES_SUPPORTED_MAX];
        int numberSourcesSupported;
        int source_index[SOURCES_SUPPORTED_MAX];
        static void dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
        bool isBacklightUsingGlobalBacklightFactor(void);
        int GetLocalparam(const char * forParam,int formatIndex,int pqIndex,int sourceIndex,int &value,bool cms=false,int tunnel_type=0);
        void GetColorTempStringFromEnum(int value, std::string &toStore);
        tvError_t saveBacklightToLocalStoreForGBF(const char* key, const char* value);
        int GetLastSetBacklightForGBF(int &backlight);
        tvDataComponentColor_t GetComponentColorEnum(std::string colorName);
        int GetDimmingModeIndex(const char* mode);
        void GetDimmingModeStringFromEnum(int value, std::string &toStore);
};

}//namespace Plugin
}//namespace WPEFramework
#endif
