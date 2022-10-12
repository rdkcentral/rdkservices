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

#pragma once
#ifndef RPDSMGR_H_
#define RPDSMGR_H_
#include "libIARM.h"
#include "dsTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IARM_BUS_DSMGR_NAME		"DSMgr"

IARM_Result_t dsMgr_init();
IARM_Result_t dsMgr_term();


/*! Events published from DS Mananger */
typedef enum _DSMgr_EventId_t {
	IARM_BUS_DSMGR_EVENT_RES_PRECHANGE=0,              /*!< Resolution Pre Change Event  */
	IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE,               /*!< Resolution Post Change Event  */
	IARM_BUS_DSMGR_EVENT_ZOOM_SETTINGS,				   /*!< Zoom Settings Events */
	IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG,                 /*!< HDMI HPD DETECT Change Event  */
	IARM_BUS_DSMGR_EVENT_AUDIO_MODE,                   /*!< HDMI HPD DETECT Change Event  */
	IARM_BUS_DSMGR_EVENT_HDCP_STATUS,                  /*!< HDMI HDCP status */
	IARM_BUS_DSMGR_EVENT_RX_SENSE,                     /*!< HDMI Rx Sense status */
	IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG,              /*!< HDMI IN HPD change event */
	IARM_BUS_DSMGR_EVENT_HDMI_IN_SIGNAL_STATUS,    /*!< HDMI IN signal status change event */
	IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS,          /*!< HDMI IN status change event */
	IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE,          /*!< HDMI IN video mode update event */
	IARM_BUS_DSMGR_EVENT_HDMI_IN_ALLM_STATUS,          /*!< HDMI IN ALLM mode update event */
	IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_HOTPLUG,         /*!< COMPOSITE IN HPD change event */
	IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_SIGNAL_STATUS,    /*!< COMPOSITE IN signal status change event */
	IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_STATUS,          /*!< COMPOSITE IN status change event */
	IARM_BUS_DSMGR_EVENT_TIME_FORMAT_CHANGE,           /*!< Clock Time Format Change Event*/
	IARM_BUS_DSMGR_EVENT_AUDIO_LEVEL_CHANGED,           /*!< Audio Level Change Event*/
	IARM_BUS_DSMGR_EVENT_AUDIO_OUT_HOTPLUG,            /*!< AUDIO OUT HPD change event */
	IARM_BUS_DSMGR_EVENT_AUDIO_FORMAT_UPDATE,  /*!< Audio Format change event */
    IARM_BUS_DSMGR_EVENT_AUDIO_PRIMARY_LANGUAGE_CHANGED,  /*!< Audio Primary Language change event */
    IARM_BUS_DSMGR_EVENT_AUDIO_SECONDARY_LANGUAGE_CHANGED,  /*!< Audio Secondary Language change event */
    IARM_BUS_DSMGR_EVENT_AUDIO_FADER_CONTROL_CHANGED,  /*!< Audio Fader Control change event */
    IARM_BUS_DSMGR_EVENT_AUDIO_ASSOCIATED_AUDIO_MIXING_CHANGED,  /*!< Audio Associated Audio Mixing change event */
	IARM_BUS_DSMGR_EVENT_VIDEO_FORMAT_UPDATE,  /*!< Video Format change event */
	IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_PRECHANGE,   /*!< Frame rate pre change */
	IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_POSTCHANGE,     /*!< Frame rate post change */
    IARM_BUS_DSMGR_EVENT_AUDIO_PORT_STATE,  /*!< Audio Port Init State */
    IARM_BUS_DSMGR_EVENT_SLEEP_MODE_CHANGED,           /*!< Sleep Mode Change Event*/
	IARM_BUS_DSMGR_EVENT_MAX,					       /*!< Max Event  */
} IARM_Bus_DSMgr_EventId_t;



/*! DS Manager  Event Data */
typedef struct _DSMgr_EventData_t {
    union {
        struct _RESOLUTION_DATA{
        /* Declare Event Data structure for Video resolution Event */
            int width;              /*!< Resolution Width  */
            int height;              /*!< Key code */ 
        } resn; /*Reolution data*/
		struct _DFC_DATA{
        	/* Declare Event Data structure for Zoom settings Event */
            int zoomsettings;
        }dfc; /*zoom data*/
		
		struct _AUDIOMODE_DATA {
        	/* Declare Event Data structure for Video resolution Event */
            int type;              /*!< device type  */
            int mode;              /*!< device mode */ 
        }Audioport; /*Audio mode  data*/

		struct _HDMI_HPD_DATA{
        	/* Declare HDMI HPD Data */
            int event;
        }hdmi_hpd; /*HDMI Hot Plug detect*/

		struct _HDMI_HDCP_DATA{
        	/* Declare HDMI DCP Data */
            int hdcpStatus;
        }hdmi_hdcp; /*HDMI HDCP Hot Plug detect*/

        struct _HDMI_RXSENSE_DATA{
            /* Declare HDMI Rx Sense status */
            int status;
        }hdmi_rxsense; /*HDMI Rx Sense Data*/
        
        struct _HDMI_IN_CONNECT_DATA
        {
            dsHdmiInPort_t port;
            bool           isPortConnected;
        }hdmi_in_connect;

        struct _HDMI_IN_STATUS_DATA{
         /* Declare HDMI Input status*/
            dsHdmiInPort_t port;
            bool           isPresented;
        }hdmi_in_status; /*HDMI in status change detect*/

        struct _HDMI_IN_SIG_STATUS_DATA{
         /* Declare HDMI In signal status*/
            dsHdmiInPort_t port;
            dsHdmiInSignalStatus_t status;
        }hdmi_in_sig_status; /*HDMI in signal change detect*/

        struct _HDMI_IN_VIDEO_MODE_DATA{
         /* Declare HDMI In signal status*/
            dsHdmiInPort_t port;
            dsVideoPortResolution_t resolution;
        }hdmi_in_video_mode; /*HDMI in video mode update*/

        struct _COMPOSITE_IN_CONNECT_DATA
        {
            dsCompositeInPort_t port;
            bool           isPortConnected;
        }composite_in_connect;

        struct _COMPOSITE_IN_STATUS_DATA{
         /* Declare Composite Input status*/
            dsCompositeInPort_t port;
            bool           isPresented;
        }composite_in_status; /*Composite in status change detect*/

        struct _COMPOSITE_IN_SIG_STATUS_DATA{
         /* Declare Composite In signal status*/
            dsCompositeInPort_t port;
            dsCompInSignalStatus_t status;
        }composite_in_sig_status; /*Composite in signal change detect*/

        struct _FPD_TIME_FORMAT
        {
            dsFPDTimeFormat_t  eTimeFormat;
        }FPDTimeFormat;

        struct _HDCP_PROTOCOL_DATA
        {
        	dsHdcpProtocolVersion_t  protocolVersion;
        }HDCPProtocolVersion;
        struct _SLEEP_MODE_DATA
        {
            dsSleepMode_t sleepMode;
        }sleepModeInfo;

        struct _AUDIO_LEVEL_DATA
        {
            int level;
        }AudioLevelInfo;

        struct _AUDIO_OUT_CONNECT_DATA
        {
            dsAudioPortType_t portType;
            unsigned int      uiPortNo;
            bool           isPortConnected;
        }audio_out_connect;

        struct _AUDIO_FORMAT_DATA
	{
	    dsAudioFormat_t audioFormat;
	}AudioFormatInfo;

        struct _LANGUAGE_DATA
        {
            char audioLanguage[MAX_LANGUAGE_LEN];
        }AudioLanguageInfo;

        struct _FADER_CONTROL_DATA
        {
            int mixerbalance;
        }FaderControlInfo;

        struct _ASSOCIATED_AUDIO_MIXING_DATA
        {
            bool mixing;
        }AssociatedAudioMixingInfo;

        struct _VIDEO_FORMAT_DATA
	{
	    dsHDRStandard_t videoFormat;
	}VideoFormatInfo;

        struct _AUDIO_PORTSTATE_DATA
        {
           dsAudioPortState_t audioPortState;
        }AudioPortStateInfo;

        struct _HDMI_IN_ALLM_MODE_DATA{
         /* Declare HDMI In ALLM Mode*/
            dsHdmiInPort_t port;
            bool allm_mode;
        }hdmi_in_allm_mode; /*HDMI in ALLM Mode change*/

    } data;
}IARM_Bus_DSMgr_EventData_t;

#ifdef __cplusplus
}
#endif

#endif /* RPDSMGR_H_ */


/** @} */
/** @} */
