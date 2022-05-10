#pragma once

#define IARM_BUS_SYSMGR_NAME "SYSMgr"

typedef enum _SYSMgr_EventId_t {
    IARM_BUS_SYSMGR_EVENT_USB_MOUNT_CHANGED,
    IARM_BUS_SYSMGR_EVENT_MAX
} IARM_Bus_SYSMgr_EventId_t;

typedef struct _IARM_BUS_SYSMgr_EventData_t {
    union {
        struct _USB_MOUNT {
            int mounted;
            char device[128];
            char dir[256];
        } usbMountData;
    } data;
} IARM_Bus_SYSMgr_EventData_t;
