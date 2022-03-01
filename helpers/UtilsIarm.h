#pragma once

#include "UtilsLogging.h"

#define IARM_CHECK(FUNC) { \
    if ((res = FUNC) != IARM_RESULT_SUCCESS) { \
        LOGINFO("IARM %s: %s", #FUNC, \
            res == IARM_RESULT_INVALID_PARAM ? "invalid param" : ( \
            res == IARM_RESULT_INVALID_STATE ? "invalid state" : ( \
            res == IARM_RESULT_IPCCORE_FAIL ? "ipcore fail" : ( \
            res == IARM_RESULT_OOM ? "oom" : "unknown")))); \
    } \
    else \
    { \
        LOGINFO("IARM %s: success", #FUNC); \
    } \
}

namespace Utils
{
struct IARM
{
    static bool init();
    static bool isConnected();

    static const char *NAME;
};

const char *IARM::NAME = "Thunder_Plugins";

bool IARM::isConnected()
{
    IARM_Result_t res;
    int isRegistered = 0;
    res = IARM_Bus_IsConnected(NAME, &isRegistered);
    LOGINFO("IARM_Bus_IsConnected: %d (%d)", res, isRegistered);

    return (isRegistered == 1);
}

bool IARM::init()
{
    IARM_Result_t res;
    bool result = false;

    if (isConnected()) {
        LOGINFO("IARM already connected");
        result = true;
    }
    else {
        res = IARM_Bus_Init(NAME);
        LOGINFO("IARM_Bus_Init: %d", res);
        if (res == IARM_RESULT_SUCCESS ||
            res == IARM_RESULT_INVALID_STATE /* already inited or connected */) {

            res = IARM_Bus_Connect();
            LOGINFO("IARM_Bus_Connect: %d", res);
            if (res == IARM_RESULT_SUCCESS ||
                res == IARM_RESULT_INVALID_STATE /* already connected or not inited */) {

                result = isConnected();
            }
            else {
                LOGERR("IARM_Bus_Connect failure: %d", res);
            }
        }
        else {
            LOGERR("IARM_Bus_Init failure: %d", res);
        }
    }

    return result;
}
}
