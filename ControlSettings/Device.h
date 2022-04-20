#ifndef DEVICE_H
#define DEVICE_H

#include "string.h"
#include <pthread.h>
#include "Module.h"
#include "utils.h"
#include "tvTypes.h"
#include "tvError.h"
#include "tvLog.h"
#include "tvSettings.h"

#define returnResponse(return_status, error_log) \
    {response["success"] = return_status; \
    if(!return_status) \
        response["error_message"] = _T(error_log); \
    PLUGIN_Unlock(tvLock); \
    return (Core::ERROR_NONE);}

#define returnIfParamNotFound(param)\
    if(param.empty())\
    {\
        LOGERR("missing parameter %s\n",#param);\
        returnResponse(false,"missing parameter");\
    }
#define PLUGIN_Lock(lock) pthread_mutex_lock(&lock)
#define PLUGIN_Unlock(lock) pthread_mutex_unlock(&lock)

namespace WPEFramework {
namespace Plugin {

class Device  {
    private:
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;

    public:
        Device();
        ~Device();
        void getAspectRatio();
        void setAspectRatio();
	virtual void getBacklight();
        virtual void setBacklight();

    public:
       virtual void Initialize();
       virtual void DeInitialize();

    protected:

       std::string getErrorString (tvError_t eReturn);
};
}
}
#endif
