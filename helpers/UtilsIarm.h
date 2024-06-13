#pragma once

#include "UtilsLogging.h"

#include "libIBus.h"
#include <unistd.h>

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

namespace Utils {
struct IARM {
    static bool init()
    {
        IARM_Result_t res;
        bool result = false;

        if (isConnected()) {
            LOGINFO("IARM already connected");
            result = true;
        } else {
            unsigned int retryCount = 0;
            do
            {
                res = IARM_Bus_Init(NAME);
                LOGINFO("IARM_Bus_Init: %d", res);
                if (res == IARM_RESULT_SUCCESS || res == IARM_RESULT_INVALID_STATE /* already inited or connected */) {
                    res = IARM_Bus_Connect();
                    LOGINFO("IARM_Bus_Connect: %d", res);
                    if (res == IARM_RESULT_SUCCESS || res == IARM_RESULT_INVALID_STATE /* already connected or not inited */) {
                        result = isConnected();
                        LOGERR("ARM_Bus_Connect result: %d res: %d retryCount :%d ",result, res, retryCount);
                    } else {
                        LOGERR("IARM_Bus_Connect failure:result :%d res: %d retryCount :%d ",result, res, retryCount);
                    }
                } else {
                    LOGERR("IARM_Bus_Init failure: result :%d res: %d retryCount :%d",result, res,retryCount);
                }

                if(result == false) usleep(100000);

            }while((result == false) && (retryCount++ < 20));
        }

        return result;
    }

    static bool isConnected()
    {
        IARM_Result_t res;
        int isRegistered = 0;
        res = IARM_Bus_IsConnected(NAME, &isRegistered);
        LOGINFO("IARM_Bus_IsConnected: res:%d  isRegistered (%d)", res, isRegistered);

        return (isRegistered == 1);
    }

    static constexpr const char* NAME = "Thunder_Plugins";
};
}
