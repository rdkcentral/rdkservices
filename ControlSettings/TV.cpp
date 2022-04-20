#include <string>
#include "TV.h"

namespace WPEFramework {
namespace Plugin {

    TV::TV()
    {
    //Register Common API 
    }
    
    TV :: ~TV()
    {
    }

    void TV::Initialize()
    {
        LOGINFO();
	//Platform specific Init Sequence
    }

    void TV::DeInitialize()
    {
        LOGINFO();
    }

    void TV::getBacklight()
    {
        LOGINFO("Derived Entry : %s\n",__FUNCTION__);
        LOGINFO("Derived Exit  : %s\n",__FUNCTION__);
    }

    void TV::setBacklight()
    {
        LOGINFO("Derived Entry : %s\n",__FUNCTION__);
        LOGINFO("Derived Exit  : %s\n",__FUNCTION__);
    }

}//namespace Plugin
}//namespace WPEFramework
