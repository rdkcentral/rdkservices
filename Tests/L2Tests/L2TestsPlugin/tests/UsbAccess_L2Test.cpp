#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "L2Tests.h"
#include "L2TestsMock.h"
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <list>

#define JSON_TIMEOUT   (1000)
#define TEST_LOG(x, ...) fprintf(stderr, "\033[1;32m[%s:%d](%s)<PID:%d><TID:%d>" x "\n\033[0m", __FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), ##__VA_ARGS__); fflush(stderr);
#define USBACCESS_CALLSIGN  _T("org.rdk.UsbAccess.1")
#define USBACCESSL2TEST_CALLSIGN _T("L2tests.1")

using ::testing::NiceMock;
using namespace WPEFramework;
using testing::StrictMock;

typedef enum : uint32_t {
    UsbAccess_onUSBMountChanged = 0x00000001,
    UsbAccess_onArchieveLogs = 0x00000002,
    UsbAccess_StateInvalid = 0x00000000

}UsbAccessL2test_async_events_t;

/**
 * @brief Compare two request status objects
 *
 * @param[in] data Expected value
 * @return true if the argument and data match, false otherwise
 */

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

/**
 * @brief Internal test mock class
 *
 * Note that this is for internal test use only and doesn't mock any actual
 * concrete interface.
 */

class AsyncHandlerMock_USB
{
    public:
		AsyncHandlerMock_USB()
        {
        }
	    MOCK_METHOD(void, onUSBMountChanged, (const JsonObject &message));   
        MOCK_METHOD(void, onArchiveLogs,     (const JsonObject &message));    
};

/* UsbAccess L2 test class declaration */

class UsbAccess_L2test : public L2TestMocks {
    protected:
        Core::JSONRPC::Message message;
        string response;
        IARM_EventHandler_t iarmEventHandler = nullptr;
        virtual ~UsbAccess_L2test() override;
    
    public:
        UsbAccess_L2test();
        void onUSBMountChanged(const JsonObject &message);
        void onArchiveLogs(const JsonObject &message);
        std::string verify_getFileListContents(string *str1);

        uint32_t WaitForRequestStatus(uint32_t timeout_ms, UsbAccessL2test_async_events_t expected_status);

    private:
        /** @brief Mutex */
        std::mutex m_mutex;
        
        /** @brief Condition variable */
        std::condition_variable m_condition_variable;

        /** @brief Event signalled flag */
        uint32_t m_event_signalled;
};

/**
 * @brief Constructor for UsbAccess L2 test class
 */

UsbAccess_L2test:: UsbAccess_L2test():L2TestMocks()
{
    TEST_LOG("UsbAccess_L2test constructor");
    uint32_t status = Core::ERROR_GENERAL;
    Core::JSONRPC::Message message;

        ON_CALL(*p_iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    if ((string(IARM_BUS_SYSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_SYSMGR_EVENT_USB_MOUNT_CHANGED)) {
                        
                        iarmEventHandler = handler;
                    }
                    return IARM_RESULT_SUCCESS;
                }));
    
    status = ActivateService("org.rdk.UsbAccess");
    EXPECT_EQ(Core::ERROR_NONE, status);

    std::ofstream file("/etc/device.properties"); 
    file << "MODEL_NUM=HSTP11MWR";
    file.close();
}

/**
 * @brief Destructor for UsbAccess L2 test class
 */

UsbAccess_L2test:: ~UsbAccess_L2test(){

    TEST_LOG("UsbAccess_L2test Destructor");
    uint32_t status = Core::ERROR_GENERAL;
    
    status = DeactivateService("org.rdk.UsbAccess");
    EXPECT_EQ(Core::ERROR_NONE, status);
}

/**
 * @brief waits for various status change on asynchronous calls
 *
 * @param[in] timeout_ms timeout for waiting
 */

uint32_t UsbAccess_L2test::WaitForRequestStatus(uint32_t timeout_ms, UsbAccessL2test_async_events_t expected_status)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto now = std::chrono::system_clock::now();
    std::chrono::milliseconds timeout(timeout_ms);
    uint32_t signalled = UsbAccess_StateInvalid;

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

void UsbAccess_L2test::onArchiveLogs(const JsonObject &message)
{
    TEST_LOG("onArchiveLogs event triggered ******\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    std::string str;
    message.ToString(str);

    TEST_LOG("onArchiveLogs received: %s\n", str.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= UsbAccess_onArchieveLogs;
    m_condition_variable.notify_one();
}

/* Function to verify the contents of getFileList method in the FileList Test case */
std::string UsbAccess_L2test::verify_getFileListContents(string *str1)
{
    string str2 = *str1;

    /* Verify the contents of getFileList in the below list as set in L2-tests.yml */
    std::list<string> search_list = {
        "{\"name\":\".\",\"t\":\"d\"}", 
        "{\"name\":\"..\",\"t\":\"d\"}",
        "{\"name\":\"PreviousLogs\",\"t\":\"d\"}",
        "{\"name\":\"test.png\",\"t\":\"f\"}",
        "{\"name\":\"test.txt\",\"t\":\"f\"}"
    };

    for (auto const& i : search_list) {
        int pos = str2.find(i);
        if(pos >= 0)
            str2.erase(pos, i.length());
        pos = str2.find(",,");
        if(pos >= 0)
            str2.erase(pos, 2);
    }
    /* str2 will contain only “[]”  if the contents of getFileList are there in the above search_list */
    return str2;
}

/********************************************************
************Test case Details **************************
** 1. Triggered getFileList Method
** 2. Verify the response of getFileList Method
** 3. Verify the event onUSBMountChanged getting triggered
*******************************************************/

TEST_F(UsbAccess_L2test, FileList)
{  
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(USBACCESS_CALLSIGN, USBACCESSL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_USB> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    std::string message;
    JsonObject expected_status;
    uint32_t signalled = UsbAccess_StateInvalid;

    EXPECT_CALL(*p_udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));

    EXPECT_CALL(*p_udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(*p_udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(*p_udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(*p_udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));
        
    /* getFileList returns null as getmntent() system call used by getMounted API which is being invoked internally in the getFileList doesn't return any mounted path*/
    status = InvokeServiceMethod("org.rdk.UsbAccess.1", "getFileList", params, result);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());

    EXPECT_CALL(*p_wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1");
        entry1.mnt_dir = const_cast<char*>("/run/media/sda1/logs");

        static int callCount = 0;
        if (callCount == 0) {
           callCount++;
           return &entry1;
        } else {
           return static_cast<struct mntent*>(NULL);
        }
    }));
       
    /*
    * getFileList returns mounted path for which the contents are returned
    * Contents include a list of files and directories on the mounted path 
    * Files with the extension mentioned in REGEX_FILE are only returned
    * REGEX_FILE = "^[^,`~?!$@#%^&*+={}\\[\\]<>]+\\.(png|jpg|jpeg|tiff|tif|bmp|mp4|mov|avi|mp3|wav|m4a|flac|aac|wma|txt|bin|enc|ts)$"
    */

    status = InvokeServiceMethod("org.rdk.UsbAccess.1", "getFileList", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());

    string getFileListContents_path1  = result["contents"].String().c_str();   
    string output1 = verify_getFileListContents(&getFileListContents_path1);
    EXPECT_STREQ("/run/media/sda1/logs", result["path"].String().c_str());
    EXPECT_STREQ("[]", output1.c_str());

    std::cout << "output1  = " << output1 << "\n";

    /* Subscribe to the "onUSBMountChanged" event */
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                   _T("onUSBMountChanged"),
                                   &AsyncHandlerMock_USB::onUSBMountChanged,
                                   &async_handler);
    EXPECT_EQ(Core::ERROR_NONE, status);


    message = "{\"mounted\":true,\"device\":\"\\/dev\\/sda1\"}";
    expected_status.FromString(message);
    EXPECT_CALL(async_handler, onUSBMountChanged(MatchRequestStatus(expected_status)))
        .WillOnce(Invoke(this, &UsbAccess_L2test::onUSBMountChanged));
       
    IARM_Bus_SYSMgr_EventData_t usbEventData;
    usbEventData.data.usbMountData.mounted= true;
    strncpy(usbEventData.data.usbMountData.dir, "/dev/sda1", sizeof(usbEventData.data.usbMountData.dir));
    iarmEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_USB_MOUNT_CHANGED, &usbEventData, 1);

    EXPECT_CALL(*p_udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));

    EXPECT_CALL(*p_udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda2"));

    EXPECT_CALL(*p_udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(*p_udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(*p_udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda2"));   

    EXPECT_CALL(*p_wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry2;

        entry2.mnt_fsname = const_cast<char*>("/dev/sda2");
        entry2.mnt_dir = const_cast<char*>("/run/media/sda2/logs");

        static int callCount = 0;
        if (callCount == 0) {
           callCount++;
           return &entry2;
        } else {
           return static_cast<struct mntent*>(NULL);
        }
    }));

    /*
    *  getFileList returns a mounted path which is different than the one which is verified above 
    *  Contents include a list of files and directories on the mounted path
    *  Files with the extension mentioned in REGEX_FILE are only returned
    *  REGEX_FILE = "^[^,`~?!$@#%^&*+={}\\[\\]<>]+\\.(png|jpg|jpeg|tiff|tif|bmp|mp4|mov|avi|mp3|wav|m4a|flac|aac|wma|txt|bin|enc|ts)$"
    */

    status = InvokeServiceMethod("org.rdk.UsbAccess.1", "getFileList", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());

    string getFileListContents_path2  = result["contents"].String().c_str();   
    string output2 = verify_getFileListContents(&getFileListContents_path2);
    EXPECT_STREQ("/run/media/sda2/logs", result["path"].String().c_str());
    EXPECT_STREQ("[]", output2.c_str());

    message = "{\"mounted\":true,\"device\":\"\\/dev\\/sda2\"}";
    expected_status.FromString(message);
    EXPECT_CALL(async_handler, onUSBMountChanged(MatchRequestStatus(expected_status)))
        .WillOnce(Invoke(this, &UsbAccess_L2test::onUSBMountChanged));
       
    IARM_Bus_SYSMgr_EventData_t usbEventData1;
    usbEventData1.data.usbMountData.mounted= true;
    strncpy(usbEventData1.data.usbMountData.dir, "/dev/sda2", sizeof(usbEventData1.data.usbMountData.dir));
    iarmEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_USB_MOUNT_CHANGED, &usbEventData1, 1);
 
    signalled = WaitForRequestStatus(JSON_TIMEOUT,UsbAccess_onUSBMountChanged);
    EXPECT_TRUE(signalled & UsbAccess_onUSBMountChanged);

    /* Unregister for event */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onUSBMountChanged"));
    
}

/********************************************************
************Test case Details **************************
** 1. Triggered getAvailableFirmwareFiles & updateFirmware Methods
** 2. Verify getAvailableFirmwareFiles & updateFirmware method getting triggered and verify their responses
*******************************************************/

TEST_F(UsbAccess_L2test, FirmwareUpdate)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(USBACCESS_CALLSIGN, USBACCESSL2TEST_CALLSIGN);
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;

    EXPECT_CALL(*p_udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    
    EXPECT_CALL(*p_udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(*p_udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(*p_udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(*p_udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(*p_wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1");
        entry1.mnt_dir = const_cast<char*>("/run/sda1/");

        static int callCount = 0;
        if (callCount == 0) {
           callCount++;
           return &entry1;
        } else {
           return static_cast<struct mntent*>(NULL);
        }
    }));
    

    status = InvokeServiceMethod("org.rdk.UsbAccess.1", "updateFirmware", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());

    EXPECT_CALL(*p_wrapsImplMock, v_secure_system(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const char* command, va_list args) {
                /*Since we have added the "etc/device.properties" file in the test fixture, it can be opened,
                and the model number is now available. Therefore, we need to change the input parameter for this test API as well as the corresponding string in the system command to match the model number.*/
                va_list args2;
                va_copy(args2,args);
                char strFmt[256];
                vsnprintf(strFmt, sizeof(strFmt), command, args2);
                EXPECT_EQ(string(strFmt), string(_T("/lib/rdk/userInitiatedFWDnld.sh usb '/tmp/reboot/' 'HSTP11MWR.bin' 0 >> /opt/logs/swupdate.log &")));
                return 0;
            }));

    /* Gets a list of firmware files on the device. These files should start with the PMI or model number for that device and end with '.bin'. Firmware files are scanned in the root directories */
    status = InvokeServiceMethod("org.rdk.UsbAccess.1", "getAvailableFirmwareFiles", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_STREQ("[\"\\/run\\/sda1\\/UsbTestFWUpdate\\/HSTP11MWR_3.11p5s1_VBN_sdy.bin\",\"\\/run\\/sda1\\/UsbProdFWUpdate\\/HSTP11MWR_4.11p5s1_VBN_sdy.bin\",\"\\/run\\/sda1\\/HSTP11MWR_5.11p5s1_VBN_sdy.bin\"]"
, result["availableFirmwareFiles"].String().c_str());

    /* Updates the firmware using the specified file retrieved from the 'getAvailableFirmwareFiles' method. The firmware file path to use for the update*/
    params["fileName"] = "/tmp/reboot/HSTP11MWR.bin";
    status = InvokeServiceMethod("org.rdk.UsbAccess.1", "updateFirmware", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());

}

/********************************************************
************Test case Details **************************
** 1. Triggered getLinks, clearLink & createLink Methods
** 2. Verify getLinks, clearLink & createLink method getting triggered and verify their responses
*******************************************************/

TEST_F(UsbAccess_L2test, SymLinkoperations)
{  
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(USBACCESS_CALLSIGN, USBACCESSL2TEST_CALLSIGN);
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    std::string message;
    JsonObject expected_status;
   
    status = InvokeServiceMethod("org.rdk.UsbAccess.1", "clearLink", params , result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());

    status = InvokeServiceMethod("org.rdk.UsbAccess.1", "createLink", params , result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);  
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());  


    EXPECT_CALL(*p_udevImplMock, udev_enumerate_get_list_entry(testing::_))
         .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));

    EXPECT_CALL(*p_udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(*p_udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(*p_udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(*p_udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(*p_wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1"); // Set the value for mnt_fsname
        entry1.mnt_dir = const_cast<char*>("/run/media/sda1/logs");    // Set the value for mnt_dir

        static int callCount = 0;
        if (callCount == 0) {
            callCount++;
            return &entry1;
        } else {
            return static_cast<struct mntent*>(NULL);
        }
    }));

    
    status = InvokeServiceMethod("org.rdk.UsbAccess.1", "getLinks", params , result);    
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());
    
    /* Creates a symbolic link to the root folder of the USB drive */
    status = InvokeServiceMethod("org.rdk.UsbAccess.1", "createLink", params , result);
    EXPECT_EQ(Core::ERROR_NONE, status); 
    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_STREQ("http://localhost:50050/usbdrive", result["baseURL"].String().c_str());
   
    /* Returns the list of links and associated baseURL paths */
    status = InvokeServiceMethod("org.rdk.UsbAccess.1", "getLinks", params , result);    
    EXPECT_EQ(Core::ERROR_NONE, status); 
    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_STREQ("[{\"path\":\"\\/run\\/media\\/sda1\\/logs\",\"baseURL\":\"http:\\/\\/localhost:50050\\/usbdrive\"}]", result["links"].String().c_str());
      
    /* Clears or removes the symbolic link created by the 'createLink' method */
    status = InvokeServiceMethod("org.rdk.UsbAccess.1", "clearLink", params , result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());

    status = InvokeServiceMethod("org.rdk.UsbAccess.1", "getLinks", params , result);    
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());

}

/********************************************************
************Test case Details **************************
** 1. Subscribe for onArchiveLogs Events
** 2. Verify onArchiveLogs event triggered and verify the response
*******************************************************/

TEST_F(UsbAccess_L2test,UsbAccessArchiveLogs)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(USBACCESS_CALLSIGN,USBACCESSL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_USB> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    uint32_t signalled = UsbAccess_StateInvalid;
    std::string message;
    JsonObject expected_status;

    status = InvokeServiceMethod("org.rdk.UsbAccess.1", "ArchiveLogs", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());


    EXPECT_CALL(*p_udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));

    EXPECT_CALL(*p_udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(*p_udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(*p_udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(*p_udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(*p_wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1");
        entry1.mnt_dir = const_cast<char*>("/run/media/sda1");

        static int callCount = 0;
        if (callCount == 0) {
           callCount++;
           return &entry1;
        } else {
           return static_cast<struct mntent*>(NULL);
        }
    }));

    ON_CALL(*p_wrapsImplMock, v_secure_popen(::testing::_, ::testing::_, ::testing::_))
     .WillByDefault(::testing::Invoke(
        [&](const char *direction, const char *command, va_list args) -> FILE* {
            const char* valueToReturn = NULL;
            va_list args2;
            va_copy(args2, args);
            char strFmt[256];
            vsnprintf(strFmt, sizeof(strFmt), command, args2);
            va_end(args2);     
            if (strcmp(strFmt, "/lib/rdk/usbLogUpload.sh /run/media/sda1") == 0) {
                valueToReturn = "/run/media/sda1/5C3400F15492_Logs_12-05-22-10-41PM.tgz";
            }
            if (valueToReturn != NULL) {
                  char buffer[1024];
                  memset(buffer, 0, sizeof(buffer));
                  strcpy(buffer, valueToReturn);
                  FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                  return pipe;
            } else {
                 return nullptr;
            }
        }));

    /* errorCode and errorDescription should not be set */
    EXPECT_FALSE(result.HasLabel("errorCode"));
    EXPECT_FALSE(result.HasLabel("errorDescription"));


    /* Register for abortlog event. */
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                           _T("onArchiveLogs"),
                                           &AsyncHandlerMock_USB::onArchiveLogs,
                                           &async_handler);

    EXPECT_EQ(Core::ERROR_NONE, status);

    
    /* Compresses and uploads device logs into attached USB drive */
    params["path"] = "/run/media/sda1";
    params["error"] = "none";
    params["success"] = "true";
    status = InvokeServiceMethod("org.rdk.UsbAccess.1", "ArchiveLogs", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());

    /* Request status for onArchive. */

    message = "{\"error\":\"none\",\"success\":true,\"path\":\"\\/run\\/media\\/sda1\\/5C3400F15492_Logs_12-05-22-10-41PM.tgz\"}";
    expected_status.FromString(message);
    EXPECT_CALL(async_handler, onArchiveLogs(MatchRequestStatus(expected_status)))
        .WillOnce(Invoke(this, &UsbAccess_L2test::onArchiveLogs));


    signalled = WaitForRequestStatus(JSON_TIMEOUT,UsbAccess_onArchieveLogs);
    EXPECT_TRUE(signalled & UsbAccess_onArchieveLogs);

    /* Unregister for events. */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onArchiveLogs"));
}
