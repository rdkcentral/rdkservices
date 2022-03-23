#pragma once

#define IARM_BUS_DSMGR_NAME "DSMgr"

typedef enum _DSMgr_EventId_t {
    IARM_BUS_DSMGR_EVENT_RES_PRECHANGE = 0, /*!< Resolution Pre Change Event */
    IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE, /*!< Resolution Post Change Event */
    IARM_BUS_DSMGR_EVENT_ZOOM_SETTINGS, /*!< Zoom Settings Events */
    IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG, /*!< HDMI HPD DETECT Change Event */
    IARM_BUS_DSMGR_EVENT_AUDIO_MODE, /*!< HDMI HPD DETECT Change Event */
    IARM_BUS_DSMGR_EVENT_HDCP_STATUS, /*!< HDMI HDCP status */
    IARM_BUS_DSMGR_EVENT_RX_SENSE, /*!< HDMI Rx Sense status */
    IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG, /*!< HDMI IN HPD change event */
    IARM_BUS_DSMGR_EVENT_HDMI_IN_SIGNAL_STATUS, /*!< HDMI IN signal status change event */
    IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS, /*!< HDMI IN status change event */
    IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE, /*!< HDMI IN video mode update event */
    IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_HOTPLUG, /*!< COMPOSITE IN HPD change event */
    IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_SIGNAL_STATUS, /*!< COMPOSITE IN signal status change event */
    IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_STATUS, /*!< COMPOSITE IN status change event */
    IARM_BUS_DSMGR_EVENT_TIME_FORMAT_CHANGE, /*!< Clock Time Format Change Event*/
    IARM_BUS_DSMGR_EVENT_AUDIO_LEVEL_CHANGED, /*!< Audio Level Change Event*/
    IARM_BUS_DSMGR_EVENT_AUDIO_OUT_HOTPLUG, /*!< AUDIO OUT HPD change event */
    IARM_BUS_DSMGR_EVENT_AUDIO_FORMAT_UPDATE, /*!< Audio Format change event */
    IARM_BUS_DSMGR_EVENT_AUDIO_PRIMARY_LANGUAGE_CHANGED, /*!< Audio Primary Language change event */
    IARM_BUS_DSMGR_EVENT_AUDIO_SECONDARY_LANGUAGE_CHANGED, /*!< Audio Secondary Language change event */
    IARM_BUS_DSMGR_EVENT_AUDIO_FADER_CONTROL_CHANGED, /*!< Audio Fader Control change event */
    IARM_BUS_DSMGR_EVENT_AUDIO_ASSOCIATED_AUDIO_MIXING_CHANGED, /*!< Audio Associated Audio Mixing change event */
    IARM_BUS_DSMGR_EVENT_VIDEO_FORMAT_UPDATE, /*!< Video Format change event */
    IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_PRECHANGE, /*!< Frame rate pre change */
    IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_POSTCHANGE, /*!< Frame rate post change */
    IARM_BUS_DSMGR_EVENT_AUDIO_PORT_STATE, /*!< Audio Port Init State */
    IARM_BUS_DSMGR_EVENT_MAX, /*!< Max Event */
} IARM_Bus_DSMgr_EventId_t;
