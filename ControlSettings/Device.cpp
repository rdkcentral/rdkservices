#include <string>
#include "Device.h"


namespace WPEFramework {
namespace Plugin {

    Device::Device() 
    {
        
    }
    
    void Device::Initialize()
    {
        LOGINFO();
    }
    void Device::DeInitialize()
    {
        LOGINFO();
    }

    void Device::getAspectRatio()
    {
        LOGINFO("Base Entry : %s\n",__FUNCTION__);
	LOGINFO("Base Exit  : %s\n",__FUNCTION__);
    }

    void Device::setAspectRatio()
    {
        LOGINFO("Base Entry : %s\n",__FUNCTION__);
        LOGINFO("Base Exit  : %s\n",__FUNCTION__);
    }



}//namespace Plugin
}//namepsace WPEFramework
