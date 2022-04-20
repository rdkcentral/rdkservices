#include "string.h"
#include "Device.h"

namespace WPEFramework {
namespace Plugin {

class STB : public Device {
    private:
        STB(const STB&) = delete;
        STB& operator=(const STB&) = delete;

    public:
	STB();
	~STB();

    public:
       virtual void Initialize();
       virtual void DeInitialize();
       virtual std::string Information() const;
};

}//namespace Plugin
}//namespace WPEFramework
