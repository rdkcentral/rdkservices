#include <string>
#include <list>
#include <memory>
#include <rbus.h>
#include "IOTConnector.h"
#define RBUS_COMPONENT_NAME "IOTControlPlugin"

namespace WPEFramework
{
    namespace iotbridge
    {

        static rbusError_t rbusHandleStatus = RBUS_ERROR_NOT_INITIALIZED;
        static rbusHandle_t rbusHandle;

        bool initializeIPC()
        {
            

            rbusHandleStatus = rbus_open(&rbusHandle, RBUS_COMPONENT_NAME);
            if (rbusHandleStatus != RBUS_ERROR_SUCCESS)
            {
                printf("consumer: rbus_open failed: %d\n", rbusHandleStatus);
                return false;
            }
            return true;
        }

        void unInitialize()
        {
            rbus_close(rbusHandle);
            rbusHandleStatus = RBUS_ERROR_NOT_INITIALIZED;
            rbusHandle = nullptr;
        }
        int getDeviceList(std::list<std::shared_ptr<IOTDevice> > &deviceList)
        {
            
            return 0;
        }
        int getDeviceProperties(std::shared_ptr<IOTDevice> iotDevice, std::list<std::string> &propList)
        {
            return 0;
        }
        int getDeviceProperty(std::shared_ptr<IOTDevice> iotDevice, const std::string &propertyName)
        {
            return 0;
        }

    }//namespace iotbridge
}//namespace wpeframework