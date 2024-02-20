#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "L2Tests.h"
#include "L2TestsMock.h"
#include <mntent.h>
#include <fstream>
#include "UdevMock.h"
#include "WrapsMock.h"
#include "secure_wrappermock.h"
#include <mutex>
#include <condition_variable>
#include <fstream>
#include "readprocMockInterface.h"
#include <string>
#include <functional>

#define JSON_TIMEOUT   (1000)
#define TEST_LOG(x, ...) fprintf(stderr, "\033[1;32m[%s:%d](%s)<PID:%d><TID:%d>" x "\n\033[0m", __FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), ##__VA_ARGS__); fflush(stderr);
#define USBACCESS_CALLSIGN  _T("org.rdk.UsbAccess.1")
#define USBACCESSL2TEST_CALLSIGN _T("L2tests.1")

using ::testing::NiceMock;
using namespace WPEFramework;
using testing::StrictMock;

typedef enum : uint32_t {
    UsbAccess_onUSBMountChanged = 0x00000001,
    UsbAccess_Stateinvalid = 0x00000000

}UsbAccessL2test_async_events_t;

MATCHER_P(MatchRequestStatus, data, "")
{
    bool match = true;
    std::string expected;
    std::string actual;
    data.ToString(expected);
    arg.ToString(actual);
    TEST_LOG(" rec = %s, arg = %s",expected.c_str(),actual.c_str());
    EXPECT_STREQ(expected.c_str(),actual.c_str());
    return match;
}

class AsyncHandlerMock_USB
{
    public:
		AsyncHandlerMock_USB()
        {
        }
	    MOCK_METHOD(void, onUSBMountChanged, (const JsonObject &message));        
};

class UsbAccess_L2test : public L2TestMocks {
    protected:
        Core::JSONRPC::Message message;
        string response;
        NiceMock<UdevImplMock> udevImplMock;
        virtual ~UsbAccess_L2test() override;
    
    public:
        UsbAccess_L2test();
        void onUSBMountChanged(const JsonObject &message);
        uint32_t WaitForRequestStatus(uint32_t timeout_ms, UsbAccessL2test_async_events_t expected_status);

    private:
        /** @brief Mutex */
        std::mutex m_mutex;
        
        /** @brief Condition variable */
        std::condition_variable m_condition_variable;

        /** @brief Event signalled flag */
        uint32_t m_event_signalled;
};

UsbAccess_L2test:: UsbAccess_L2test():L2TestMocks()
{
    TEST_LOG("UsbAccess_L2test constructor");
    uint32_t status = Core::ERROR_GENERAL;
    Core::JSONRPC::Message message;
    
    status = ActivateService("org.rdk.UsbAccess");
    EXPECT_EQ(Core::ERROR_NONE, status);
}
UsbAccess_L2test:: ~UsbAccess_L2test(){
    TEST_LOG("UsbAccess_L2test Destructor");
    uint32_t status = Core::ERROR_GENERAL;
    
    status = DeactivateService("org.rdk.UsbAccess");
    EXPECT_EQ(Core::ERROR_NONE, status);
}

uint32_t UsbAccess_L2test::WaitForRequestStatus(uint32_t timeout_ms, UsbAccessL2test_async_events_t expected_status)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto now = std::chrono::system_clock::now();
    std::chrono::milliseconds timeout(timeout_ms);
    uint32_t signalled = UsbAccess_Stateinvalid;

   while (!(expected_status & m_event_signalled))
   {
      if (m_condition_variable.wait_until(lock, now + timeout) == std::cv_status::timeout)
      {
         TEST_LOG("Timeout waiting for request status event");
         break;
      }
   }

    signalled = m_event_signalled;

    
    return signalled;
}

void UsbAccess_L2test::onUSBMountChanged(const JsonObject &message)
{
    TEST_LOG("onUSBMountChanged triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    std::string str;
    message.ToString(str);
    
    /* Notify the requester thread. */

    m_event_signalled |= UsbAccess_onUSBMountChanged;
    m_condition_variable.notify_one();
}


TEST_F(UsbAccess_L2test, getMountedSuccess)
{  
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(USBACCESS_CALLSIGN, USBACCESSL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_USB> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject res;
    std::string message;
    JsonObject expected_status;
    uint32_t signalled = UsbAccess_Stateinvalid;

    /*const char *command="sudo mknod /tmp/usb_l2 b 8 1";
    const char *command1="sudo mount /tmp/usb_l2";
    const char *command2="sudo umount /tmp/usb_l2";

    int result= system(command);
    int result1= system(command1);
    int result2= system(command2);
    if(result != -1){
    	std::cout<< "mknod success" <<std:: endl; 
    }
    if(result1 != -1){
    	std::cout<< "mount success" <<std:: endl; 
    }
    if(result2 != -1){
    	std::cout<< "device node USB success" <<std:: endl; 
    }

    std::cout<< system("ls -l /tmp/usb_l2") <<std:: endl; 
    std::cout<< system("ls -lrt /tmp/") <<std:: endl; */

    const char *command = "sudo dd if=/dev/zero of=/loopfile bs=1024 count=51200 ";

    const char *command1 = "losetup -f | xargs -I {} sudo losetup {} /loopfile | xargs -I {} sudo losetup {} | xargs -I {} sudo mkfs -t ext4 -v {} | sudo mkdir /tmp/USB8_E_L2 | xargs -I {} sudo mount -t ext4 {} /tmp/USB8_E_L2 | xargs -I {} sudo df -h {} | xargs -I {} sudo tune2fs -l {}";

    int result1= system(command);
    int result2= system(command1);
    if(result1 != -1){
    	std::cout<< "Create usb success" <<std:: endl; 
    }
    if(result2 != -1){
    	std::cout<< "create loop device with large file" <<std:: endl; 
    }
    std::cout<< system("ls -l /tmp/USB8_E_L2") <<std:: endl; 
    std::cout<< system("ls -lrt /tmp/") <<std:: endl; 


        // Subscribe to the "onUSBMountChanged" event.
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                   _T("onUSBMountChanged"),
                                   &AsyncHandlerMock_USB::onUSBMountChanged,
                                   &async_handler);

    EXPECT_EQ(Core::ERROR_NONE, status);

	message = "{\"mounted\":\"mounted\",\"device\":\"device\"}";
    expected_status.FromString(message);
    EXPECT_CALL(async_handler, onUSBMountChanged(MatchRequestStatus(expected_status)))
        .WillOnce(Invoke(this, &UsbAccess_L2test::onUSBMountChanged));
	


    status = InvokeServiceMethod("org.rdk.UsbAccess.1", "getMounted", params, res);
    EXPECT_EQ(Core::ERROR_NONE, status);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UsbAccess_onUSBMountChanged);
    EXPECT_TRUE(signalled & UsbAccess_onUSBMountChanged);

    // Unregister for events.
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onUSBMountChanged"));
}

