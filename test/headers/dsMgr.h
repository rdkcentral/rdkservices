#pragma once

#define IARM_BUS_DSMGR_NAME "DSMgr"

typedef enum _DSMgr_EventId_t {
    IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_PRECHANGE,
    IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_POSTCHANGE,
    IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG,
    IARM_BUS_DSMGR_EVENT_HDMI_IN_SIGNAL_STATUS,
    IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS,
    IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE,
    IARM_BUS_DSMGR_EVENT_HDMI_IN_ALLM_STATUS,
    IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_HOTPLUG,
    IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_SIGNAL_STATUS,
    IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_STATUS,
} IARM_Bus_DSMgr_EventId_t;

typedef enum _dsHDRStandard_t {
    dsHDRSTANDARD_NONE = 0x0,
} dsHDRStandard_t;

/*! DS Manager  Event Data */
typedef struct _DSMgr_EventData_t {
    union {
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

        struct _HDMI_IN_ALLM_MODE_DATA{
         /* Declare HDMI In ALLM Mode*/
            dsHdmiInPort_t port;
            bool allm_mode;
        }hdmi_in_allm_mode; /*HDMI in ALLM Mode change*/
    } data;
}IARM_Bus_DSMgr_EventData_t;
