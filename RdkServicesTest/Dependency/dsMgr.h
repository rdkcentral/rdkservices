/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
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



/**
* @defgroup devicesettings
* @{
* @defgroup rpc
* @{
**/

#ifndef RPDSMGR_H_
#define RPDSMGR_H_
#include "libIARM.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IARM_BUS_DSMGR_NAME             "DSMgr"

IARM_Result_t dsMgr_init();
IARM_Result_t dsMgr_term();

typedef enum _dsHDRStandard_t {
    dsHDRSTANDARD_NONE = 0x0,
    dsHDRSTANDARD_HDR10 = 0x01,
    dsHDRSTANDARD_HLG = 0x02,
    dsHDRSTANDARD_DolbyVision = 0x04,
    dsHDRSTANDARD_TechnicolorPrime = 0x08,
    dsHDRSTANDARD_Invalid = 0x80,
} dsHDRStandard_t;


/*! Events published from DS Mananger */
typedef enum _DSMgr_EventId_t {
        IARM_BUS_DSMGR_EVENT_RES_PRECHANGE=0,              /*!< Resolution Pre Change Event  */
        IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE,               /*!< Resolution Post Change Event  */
        IARM_BUS_DSMGR_EVENT_ZOOM_SETTINGS,                                /*!< Zoom Settings Events */
        IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG,                 /*!< HDMI HPD DETECT Change Event  */
        IARM_BUS_DSMGR_EVENT_AUDIO_MODE,                   /*!< HDMI HPD DETECT Change Event  */
        IARM_BUS_DSMGR_EVENT_HDCP_STATUS,                  /*!< HDMI HDCP status */
        IARM_BUS_DSMGR_EVENT_RX_SENSE,                     /*!< HDMI Rx Sense status */
        IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG,              /*!< HDMI IN HPD change event */
        IARM_BUS_DSMGR_EVENT_HDMI_IN_SIGNAL_STATUS,    /*!< HDMI IN signal status change event */
        IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS,          /*!< HDMI IN status change event */
        IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE,          /*!< HDMI IN video mode update event */
        IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_HOTPLUG,         /*!< COMPOSITE IN HPD change event */
        IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_SIGNAL_STATUS,    /*!< COMPOSITE IN signal status change event */
        IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_STATUS,          /*!< COMPOSITE IN status change event */
        IARM_BUS_DSMGR_EVENT_TIME_FORMAT_CHANGE,           /*!< Clock Time Format Change Event*/
        IARM_BUS_DSMGR_EVENT_AUDIO_LEVEL_CHANGED,           /*!< Audio Level Change Event*/
        IARM_BUS_DSMGR_EVENT_AUDIO_OUT_HOTPLUG,            /*!< AUDIO OUT HPD change event */
        IARM_BUS_DSMGR_EVENT_AUDIO_FORMAT_UPDATE,  /*!< Audio Format change event */
        IARM_BUS_DSMGR_EVENT_VIDEO_FORMAT_UPDATE,  /*!< Video Format change event */
        IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_PRECHANGE,   /*!< Frame rate pre change */
        IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_POSTCHANGE,     /*!< Frame rate post change */
        IARM_BUS_DSMGR_EVENT_AUDIO_PORT_STATE,  /*!< Audio Port Init State */
        IARM_BUS_DSMGR_EVENT_MAX,                                              /*!< Max Event  */
} IARM_Bus_DSMgr_EventId_t;


#ifdef __cplusplus
}
#endif
#endif /* RPDSMGR_H_ */

/** @} */
/** @} */
                       
