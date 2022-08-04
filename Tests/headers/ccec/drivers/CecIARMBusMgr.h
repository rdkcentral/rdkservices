#pragma once

#define IARM_BUS_CECMGR_NAME                         "CECMgr"
#define IARM_BUS_CECMGR_MAX_DATA_LENGTH              62
#define IARM_BUS_CECMGR_API_Send                     "Send"

typedef struct _IARM_Bus_CECMgr_Send_Param_t {
	uint8_t length;
	uint8_t data[IARM_BUS_CECMGR_MAX_DATA_LENGTH + 1];
	uint8_t retVal;
}IARM_Bus_CECMgr_Send_Param_t;
