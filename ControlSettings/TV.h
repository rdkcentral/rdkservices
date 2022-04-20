#include "string.h"
#include "Device.h"

namespace WPEFramework {
namespace Plugin {

class TV : public Device {
    private:
        TV(const TV&) = delete;
        TV& operator=(const TV&) = delete;

    public:
        TV();
        ~TV();
        void getBacklight();
        void setBacklight();

    public:
       virtual void Initialize();
       virtual void DeInitialize();
       virtual std::string Information() const;
};

}//namespace Plugin
}//namespace WPEFramework
