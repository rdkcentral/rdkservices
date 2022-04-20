#include <string>
#include "STB.h"

namespace WPEFramework {
namespace Plugin {

    STB::STB()
    {
    //Register Common API 
    }
    
    STB :: ~STB()
    {
    }

    void STB::Initialize()
    {
        LOGINFO();
        //Platform specific Init Sequence
    }

    void STB::DeInitialize()
    {
        LOGINFO();
    }

    std::string STB::Information() const
    {
        // No additional info to report.
        return (std::string());
    }

}//namespace Plugin
}//namespace WPEFramework
