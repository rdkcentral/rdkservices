#include <gtest/gtest.h>
#include <mntent.h>
#include <fstream>
#include "IarmBusMock.h"
#include "ServiceMock.h"
#include "FactoriesImplementation.h"
#include "UsbAccess.h"

#include "UdevMock.h"
#include "WrapsMock.h"

using ::testing::NiceMock;
using namespace WPEFramework;

class UsbAccessTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::UsbAccess> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
	Core::JSONRPC::Message message;
    string response;
    NiceMock<UdevImplMock> udevImplMock;
    NiceMock<WrapsImplMock> wrapsImplMock;
	NiceMock<IarmBusImplMock> iarmBusImplMock;

    UsbAccessTest()
        : plugin(Core::ProxyType<Plugin::UsbAccess>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
		/*In order to ensure consistent behavior across all test APIs,
         the "/etc/device.properties" file is opened only once*/
        std::ofstream file("/etc/device.properties");
        file << "MODEL_NUM=HSTP11MWR";
        file.close();
        Udev::getInstance().impl = &udevImplMock;
        Wraps::getInstance().impl = &wrapsImplMock;
		IarmBus::getInstance().impl = &iarmBusImplMock;
    }
    virtual ~UsbAccessTest() override
    {
        Wraps::getInstance().impl = nullptr;
        Udev::getInstance().impl = nullptr;
		IarmBus::getInstance().impl = nullptr;
    }
};

class UsbAccessEventTest : public UsbAccessTest {
protected:
    NiceMock<ServiceMock> service;
    Core::JSONRPC::Message message;
    NiceMock<FactoriesImplementation> factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    UsbAccessEventTest()
        : UsbAccessTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }

    virtual ~UsbAccessEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();

        PluginHost::IFactories::Assign(nullptr);
    }
};

class UsbAccessEventIarmTest : public UsbAccessEventTest {
protected:
    IARM_EventHandler_t eventHandler;

    UsbAccessEventIarmTest()
        : UsbAccessEventTest()
    {
        ON_CALL(iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    if ((string(IARM_BUS_SYSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_SYSMGR_EVENT_USB_MOUNT_CHANGED)) {
                        eventHandler = handler;
                    }
                    return IARM_RESULT_SUCCESS;
                }));
        EXPECT_EQ(string(""), plugin->Initialize(&service));
    }

    virtual ~UsbAccessEventIarmTest() override
    {
        plugin->Deinitialize(&service);
    }

    virtual void SetUp()
    {
        ASSERT_TRUE(eventHandler != nullptr);
    }
};

TEST_F(UsbAccessTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getFileList")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("createLink")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("clearLink")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getLinks")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getAvailableFirmwareFiles")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getMounted")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("updateFirmware")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("ArchiveLogs")));
}
extern "C" FILE* __real_popen(const char* command, const char* type);

TEST_F(UsbAccessTest, UpdateFirmware)
{
    Udev::getInstance().impl = &udevImplMock;
    Wraps::getInstance().impl = &wrapsImplMock;

    EXPECT_CALL(wrapsImplMock, system(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const char* command) {
                /*Since we have added the "etc/device.properties" file in the test fixture, it can be opened,
                and the model number is now available. Therefore, we need to change the input parameter for this test API as well as the corresponding string in the system command to match the model number.*/
                EXPECT_EQ(string(command), string(_T("/lib/rdk/userInitiatedFWDnld.sh usb '/tmp;reboot;' 'HSTP11MWR.bin' 0 >> /opt/logs/swupdate.log &")));
                return 0;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("updateFirmware"), _T("{\"fileName\":\"/tmp;reboot;/HSTP11MWR.bin\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("updateFirmware"), _T("{\"fileName\":\"/tmp\';reboot;/my.bin\"}"), response));
}

/*******************************************************************************************************************
 * Test function for :getFileList
 * getFileList :
 *                Gets a list of files and folders from the specified directory or path.
 *
 *                @return Response object containing the file list
 * Use case coverage:
 *                @Success :3
 *                @Failure :5
 ********************************************************************************************************************/

/**
 * @brief : getFileList when path parameter is missing in the request parameter
 *          Check if  path parameter is missing from the parameters JSON object;
 *          then  getFileList shall be failed and return Erorr code: ERROR_BAD_REQUEST
 *
 * @param[in]   :  Invalid parameters without the "path" label
 * @return      :  error code: ERROR_BAD_REQUEST
 */
TEST_F(UsbAccessTest, getFileListFailure_BadRequest)
{
    EXPECT_EQ(Core::ERROR_BAD_REQUEST, handler.Invoke(connection, _T("getFileList"), _T("{\"MissingPathParam\":\"/run/media/sda1/logs/PreviousLogs\"}"), response));
}

/**
 * @brief : getFileList when the device list is empty;
 *          Check if the device list is empty after retrieving mounted paths;
 *          then  getFileList shall be failed
 *
 * @param[in]   :  Valid parameters with the "path" label
 * @return      :  error code: ERROR_GENERAL
 */
TEST_F(UsbAccessTest, getFileListFailure_whenDeviceListEmpty)
{
    Udev::getInstance().impl = &udevImplMock;
    Wraps::getInstance().impl = &wrapsImplMock;


    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(nullptr));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;
        entry1.mnt_fsname = const_cast<char*>("/dev/sda1");
        entry1.mnt_dir = const_cast<char*>("/run/media/sda1/logs/PreviousLogs");

        static int callCount = 0;
        if (callCount == 0) {
           callCount++;
           return &entry1;
        } else {
           return static_cast<struct mntent*>(NULL);
        }
    }));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getFileList"), _T("{\"path\":\"/run/media/sda1/logs/PreviousLogs\"}"), response));
}

/**
 * @brief : getFileList when setmntent returns nullptr
 *          Check if setmntent fails to open the mounts file
 *          then  getFileList shall be failed
 *
 * @param[in]   :  Valid parameters with the "path" label
 * @return      :  error code: ERROR_GENERAL
 */
TEST_F(UsbAccessTest, getFileListFailure_when_setmntentNull)
{
    Udev::getInstance().impl = &udevImplMock;
    Wraps::getInstance().impl = &wrapsImplMock;

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));
    
    EXPECT_CALL(wrapsImplMock, setmntent(::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const char* command, const char* type) {
                EXPECT_EQ(string(command), string(_T("/proc/mounts")));
                return nullptr;
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getFileList"), _T("{\"path\":\"/run/media/sda1/logs/PreviousLogs\"}"), response));
}

/**
 * @brief : getFileList when getmntent returns nullptr
 *          Check if  getmntent fails to retrieve mount entries
 *          then  getFileList shall be failed
 *
 * @param[in]   :  Valid parameters with the "path" label
 * @return      :  error code: ERROR_GENERAL
 */
TEST_F(UsbAccessTest, getFileListFailure_when_getmntentNull)
{
    Udev::getInstance().impl = &udevImplMock;
    Wraps::getInstance().impl = &wrapsImplMock;

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
             return static_cast<struct mntent*>(NULL);
     }));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getFileList"), _T("{\"path\":\"/run/media/sda1/logs/PreviousLogs\"}"), response));
}

/**
 * @brief : getFileList when device node is not found
 *          Check if the device node is not found in the devnodes list
 *          then  getFileList shall be failed
 *
 * @param[in]   :  Valid parameters with the "path" label
 * @return      :  error code: ERROR_GENERAL
 */
TEST_F(UsbAccessTest, getFileListFailure_when_devNodNotFound)
{
    Udev::getInstance().impl = &udevImplMock;
    Wraps::getInstance().impl = &wrapsImplMock;

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;
        static struct mntent entry2;

        //To simulate the behavior where the device node is not found in the devnodes list;
        //set a different name for entry1.mnt_fsname
        entry1.mnt_fsname = const_cast<char*>("/dev/sda2");
        entry1.mnt_dir = const_cast<char*>("/run/media/sda1/logs/PreviousLogs");

        entry2.mnt_fsname = const_cast<char*>("/dev/sdb1");
        entry2.mnt_dir = const_cast<char*>("/run/media/sdb1/logs/");
        static int callCount = 0;
        if (callCount == 0) {
           callCount++;
           return &entry1;
        } else if (callCount == 1) {
           callCount++;
           return &entry2;
        } else {
           return static_cast<struct mntent*>(NULL);
        }
    }));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getFileList"), _T("{\"path\":\"/run/media/sda1/logs/PreviousLogs\"}"), response));
}

/**
 * @brief : getFileList when absolute path is found
 *          Check if an absolute path matching the path parameter is found in the mounted paths
 *          then  getFileList shall be succeeded and retrieves the list of files from the absolute path.
 *
 * @param[in]   :  Valid parameters with the "path" label
 * @return      :  response object containing the list of retrieved files and success status as true
 */
TEST_F(UsbAccessTest, getFileListSuccess_whenAbsPathFound)
{
    Udev::getInstance().impl = &udevImplMock;
    Wraps::getInstance().impl = &wrapsImplMock;
   
    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;
        static struct mntent entry2;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1"); // Set the value for mnt_fsname
        entry1.mnt_dir = const_cast<char*>("/run/media/sda1/logs/");    // Set the value for mnt_dir

        entry2.mnt_fsname = const_cast<char*>("/dev/sdb1"); // Set the value for mnt_fsname
        entry2.mnt_dir = const_cast<char*>("/run/media/sdb1/logs");    // Set the value for mnt_dir

        static int callCount = 0;
        if (callCount == 0) {
            callCount++;
            return &entry1;
        } else if (callCount == 1) {
            callCount++;
            return &entry2;
        } else {
            return static_cast<struct mntent*>(NULL);
        }
    }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFileList"), _T("{\"path\":\"/run/media/sda1/logs/PreviousLogs\"}"), response));
    EXPECT_EQ(response, string("{\"path\":\"\\/run\\/media\\/sda1\\/logs\\/PreviousLogs\",\"contents\":[{\"name\":\".\",\"t\":\"d\"},{\"name\":\"..\",\"t\":\"d\"},{\"name\":\"logFile.txt\",\"t\":\"f\"}],\"success\":true}"));
}

/**
 * @brief : getFileList when no path given in path param
 *          Check if no path is given in path param
 *          then construct an absolute path [will be the first retrieved mount path]
 *          and getFileList shall be succeeded and retrieves the list of files from the constructed
 *          absolute path.
 *
 * @param[in]   :  Empty string with the "path" label
 * @return      :  response object containing the list of retrieved files and success status as true
 */
TEST_F(UsbAccessTest, getFileListSuccess_withoutPath)
{
    Udev::getInstance().impl = &udevImplMock;
    Wraps::getInstance().impl = &wrapsImplMock;

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));

    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;
        static struct mntent entry2;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1"); // Set the value for mnt_fsname
        entry1.mnt_dir = const_cast<char*>("/run/media/sda1/logs");    // Set the value for mnt_dir

        entry2.mnt_fsname = const_cast<char*>("/dev/sdb1"); // Set the value for mnt_fsname
        entry2.mnt_dir = const_cast<char*>("/run/media/sdb1/logs/");    // Set the value for mnt_dir

        static int callCount = 0;
        if (callCount == 0) {
            callCount++;
            return &entry1;
        } else if (callCount == 1) {
            callCount++;
            return &entry2;
        } else {
            return static_cast<struct mntent*>(NULL);
        }
    }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFileList"), _T("{\"path\":}"), response));
    EXPECT_EQ(response, string("{\"path\":\"\\/run\\/media\\/sda1\\/logs\",\"contents\":[{\"name\":\".\",\"t\":\"d\"},{\"name\":\"..\",\"t\":\"d\"},{\"name\":\"PreviousLogs\",\"t\":\"d\"}],\"success\":true}"));
}

/**
 * @brief : getFileList when absolute path is created from a relative path param
 *          Check if an absolute path matching the path parameter is NOT found in the mounted paths
 *          then construct an absolute path [by combining the first mounted path with the pathParam]
 *          assumig that the pathParam is a relative path within the USB device.
 *          and getFileList shall be succeeded and retrieves the list of files from the constructed
 *          absolute path.
 *
 * @param[in]   :  Valid parameters with the "path" label
 * @return      :  response object containing the list of retrieved files and success status as true
 */
TEST_F(UsbAccessTest, getFileListSuccess_withRelativePathParam)
{
    Udev::getInstance().impl = &udevImplMock;
    Wraps::getInstance().impl = &wrapsImplMock;

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;
        static struct mntent entry2;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1");
        //To simulate the behaviour where the absPath is not match with the mount path,
        //set a differnt mount path here
        entry1.mnt_dir = const_cast<char*>("/run/media");

        entry2.mnt_fsname = const_cast<char*>("/dev/sdb1");
        entry2.mnt_dir = const_cast<char*>("/run/media/sdb1/logs/");

        static int callCount = 0;
        if (callCount == 0) {
            callCount++;
            return &entry1;
        } else if (callCount == 1) {
            callCount++;
            return &entry2;
        } else {
            return static_cast<struct mntent*>(NULL);
        }
    }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFileList"), _T("{\"path\":\"sda1/logs/PreviousLogs\"}"), response));
    EXPECT_EQ(response, string("{\"path\":\"\\/run\\/media\\/sda1\\/logs\\/PreviousLogs\",\"contents\":[{\"name\":\".\",\"t\":\"d\"},{\"name\":\"..\",\"t\":\"d\"},{\"name\":\"logFile.txt\",\"t\":\"f\"}],\"success\":true}"));
}
 /*Test cases for getFileList ends here*/

/*******************************************************************************************************************
 * Test function for :getMounted
 * getMounted : Returns a list of mounted USB devices.
 *                
 *
 *                @return Response object containing the list of mounted USB devices
 * Use case coverage:
 *                @Success :4
 *                @Failure :1
 ********************************************************************************************************************/


/**
 * @brief : getMounted when setmntent returns nullptr
 *          Check if setmntent fails to open the currently mounted file system
 *          then  getMounted shall be failed
 *
 * @param[in]   :  None
 * @return      :  error code: ERROR_GENERAL
 */
TEST_F(UsbAccessTest, getMountedFailure_when_setmntentNull)
{
    Udev::getInstance().impl = &udevImplMock;
    Wraps::getInstance().impl = &wrapsImplMock;

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, setmntent(::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const char* command, const char* type) {
                EXPECT_EQ(string(command), string(_T("/proc/mounts")));
                return nullptr;
            }));


    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getMounted"), _T("{}"), response));
}

/**
 * @brief : getMounted when getmntent returns nullptr
 *          Check if  getmntent fails to retrieve mount entries
 *          then  getMounted shall be succeded and returns empty path list 
 *
 * @param[in]   :  None
 * @return      :  {\"mounted\":[],\"success\":true}
 */
TEST_F(UsbAccessTest, getMountedSuccess_when_getmntentNull)
{
    Udev::getInstance().impl = &udevImplMock;
    Wraps::getInstance().impl = &wrapsImplMock;

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
             return static_cast<struct mntent*>(NULL);
     }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMounted"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"mounted\":[],\"success\":true}"));
}
/**
 * @brief : getMounted when the device list is empty;
 *          Check if the device list is empty;
 *          then  getMounted shall be succeded and returns empty path list
 *
 * @param[in]   :  None
 * @return      :  error code: ERROR_GENERAL
 */
TEST_F(UsbAccessTest, getMountedSuccess_whenDeviceListEmpty)
{
    Udev::getInstance().impl = &udevImplMock;
    Wraps::getInstance().impl = &wrapsImplMock;


    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(nullptr));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;
        entry1.mnt_fsname = const_cast<char*>("/dev/sda1");
        entry1.mnt_dir = const_cast<char*>("/run/media/sda1/logs/PreviousLogs");

        static int callCount = 0;
        if (callCount == 0) {
           callCount++;
           return &entry1;
        } else {
           return static_cast<struct mntent*>(NULL);
        }
    }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMounted"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"mounted\":[],\"success\":true}"));
}
/**
 * @brief : getMounted when device node is not found
 *          Check if the device node is not found in the devnodes list
 *          then  getMounted shall be succeded and returns empty path list
 *
 * @param[in]   :  None
 * @return      :  {\"mounted\":[],\"success\":true}
 */
TEST_F(UsbAccessTest, getMountedSuccess_when_devNodNotFound)
{
    Udev::getInstance().impl = &udevImplMock;
    Wraps::getInstance().impl = &wrapsImplMock;

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;
        static struct mntent entry2;

        //To simulate the behavior where the device node is not found in the devnodes list;
        //set a different name for entry1.mnt_fsname
        entry1.mnt_fsname = const_cast<char*>("/dev/sda2");
        entry1.mnt_dir = const_cast<char*>("/run/media/sda1/logs/PreviousLogs");

        entry2.mnt_fsname = const_cast<char*>("/dev/sdb1");
        entry2.mnt_dir = const_cast<char*>("/run/media/sdb1/logs/");
        static int callCount = 0;
        if (callCount == 0) {
           callCount++;
           return &entry1;
        } else if (callCount == 1) {
           callCount++;
           return &entry2;
        } else {
           return static_cast<struct mntent*>(NULL);
        }
    }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMounted"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"mounted\":[],\"success\":true}"));
}
/**
 * @brief : getMounted when the device node of the mounted file system is found in the devnodes list 
 *          Check if the device node of the mounted file system is found in the devnodes list
 *          then getMounted shall be succeeded and retrieves the the list of file paths
 *          where USB devices are mounted
 *
 * @param[in]   :  None
 * @return      :  response object containing the USB mount point paths
 */
TEST_F(UsbAccessTest, getMountedSuccess_withUSBMountPath)
{
    Udev::getInstance().impl = &udevImplMock;
    Wraps::getInstance().impl = &wrapsImplMock;

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));

    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;
        static struct mntent entry2;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1"); // Set the value for mnt_fsname
        entry1.mnt_dir = const_cast<char*>("/run/media/sda1/logs");    // Set the value for mnt_dir

        entry2.mnt_fsname = const_cast<char*>("/dev/sdb1"); // Set the value for mnt_fsname
        entry2.mnt_dir = const_cast<char*>("/run/media/sdb1/logs/");    // Set the value for mnt_dir

        static int callCount = 0;
        if (callCount == 0) {
            callCount++;
            return &entry1;
        } else if (callCount == 1) {
            callCount++;
            return &entry2;
        } else {
            return static_cast<struct mntent*>(NULL);
        }
    }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMounted"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"mounted\":[\"\\/run\\/media\\/sda1\\/logs\"],\"success\":true}"));
}
/*Test cases for getMounted ends here*/

/*******************************************************************************************************************
* Test function for :getAvailableFirmwareFiles
* getAvailableFirmwareFiles :
*                Gets a list of firmware files on a device.
*
*                @return list of firmware files and request succeeded.
* Use case coverage:
*                @Failure :1
*                @Success :5
********************************************************************************************************************/

/**
* @brief : getAvailableFirmwareFiles when setmntent returns nullptr,
*        Check if setmntent fails to open the mounts file,
*        then getAvailableFirmwareFiles shall be failed.
*
* @param[in]   :  NONE
* @return      :  {"availableFirmwareFiles": [],"success":false}
*/
TEST_F(UsbAccessTest, getAvailableFirmwareFilesFailed_whenSetmntentValueNull)
{
    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, setmntent(::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const char* command, const char* type) {
                EXPECT_EQ(string(command), string(_T("/proc/mounts")));
                return nullptr;
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getAvailableFirmwareFiles"), _T("{}"), response));
}

/**
* @brief : getAvailableFirmwareFiles when getmntent returns nullptr,
*        Check if  getmntent fails to retrieve mount entries
*        then  getAvailableFirmwareFiles shall be succeded and returns empty path list.
*
* @param[in]   :  NONE
* @return      :  {"availableFirmwareFiles": [],"success":true}
*/
TEST_F(UsbAccessTest, getAvailableFirmwareFilesSuccess_when_getmntentNull)
{
    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));

    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
             return static_cast<struct mntent*>(NULL);
     }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getAvailableFirmwareFiles"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"availableFirmwareFiles\":[],\"success\":true}"));
}

/**
 * @brief : getAvailableFirmwareFiles when the device list is empty,
 *        Check if the device list is empty,
 *        then  getAvailableFirmwareFiles shall be succeded and returns empty path list.
 *
 * @param[in]   : NONE
 * @return      : {"availableFirmwareFiles": [],"success":true}
 */
TEST_F(UsbAccessTest, getAvailableFirmwareFilesSuccess_whenDeviceListEmpty)
{
    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(nullptr));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;
        entry1.mnt_fsname = const_cast<char*>("/dev/sda1");
        entry1.mnt_dir = const_cast<char*>("/run/sda1");

        static int callCount = 0;
        if (callCount == 0) {
           callCount++;
           return &entry1;
        } else {
           return static_cast<struct mntent*>(NULL);
        }
    }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getAvailableFirmwareFiles"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"availableFirmwareFiles\":[],\"success\":true}"));
}


/**
 * @brief : getAvailableFirmwareFiles when device node is not found,
 *        Check if the device node is not found in the devnodes list,
 *        then  getMounted shall be succeded and returns empty path list.
 *
 * @param[in]   :  NONE
 * @return      :  {"availableFirmwareFiles": [],"success":true}
 */
TEST_F(UsbAccessTest, getAvailableFirmwareFilesSuccess_when_devNodNotFound)
{
    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;
        static struct mntent entry2;

        entry1.mnt_fsname = const_cast<char*>("/dev/sdb1");
        entry1.mnt_dir = const_cast<char*>("/mnt/usb1");
        entry2.mnt_fsname = const_cast<char*>("/dev/sdb2");
        entry2.mnt_dir = const_cast<char*>("/mnt/usb2");
        static int callCount = 0;
        if (callCount == 0) {
           callCount++;
           return &entry1;
        }
        else if (callCount == 1) {
           callCount++;
           return &entry2;
        }
        else {
           return static_cast<struct mntent*>(NULL);
        }
    }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getAvailableFirmwareFiles"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"availableFirmwareFiles\":[],\"success\":true}"));
}

/**
* @brief : getAvailableFirmwareFiles when bin files are not present,
*        Check if .bin files are not present in mounted paths,
*        then getAvailableFirmwareFiles shall succeed and returns an empty list.
*
* @param[in]   :  NONE
* @return      :  {"availableFirmwareFiles": [],"success":true}
*/
TEST_F(UsbAccessTest, getAvailableFirmwareFilesSuccess_withoutBinfiles)
{
    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda2"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda2"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda2");
        entry1.mnt_dir = const_cast<char*>("/run/sda2");

        static int callCount = 0;
        if (callCount == 0) {
           callCount++;
           return &entry1;
        } else {
           return static_cast<struct mntent*>(NULL);
        }
    }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getAvailableFirmwareFiles"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"availableFirmwareFiles\":[],\"success\":true}"));
}

/**
* @brief : getAvailableFirmwareFiles when bin files are present,
*        Check if .bin files are present in mounted paths, additional paths(UsbTestFWUpdate, UsbProdFWUpdate),
*        then getAvailableFirmwareFiles shall succeed and returns all the file paths.
*
* @param[in]   :  NONE
* @return      :  returns The list of firmware files including the full path name
*/
TEST_F(UsbAccessTest, getAvailableFirmwareFilesSuccess_withBinFiles)
{
    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1");
        entry1.mnt_dir = const_cast<char*>("/run/sda1");

        static int callCount = 0;
        if (callCount == 0) {
           callCount++;
           return &entry1;
        } else {
           return static_cast<struct mntent*>(NULL);
        }
    }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getAvailableFirmwareFiles"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"availableFirmwareFiles\":[\"\\/run\\/sda1\\/UsbTestFWUpdate\\/HSTP11MWR_3.11p5s1_VBN_sdy.bin\",\"\\/run\\/sda1\\/UsbProdFWUpdate\\/HSTP11MWR_4.11p5s1_VBN_sdy.bin\",\"\\/run\\/sda1\\/HSTP11MWR_5.11p5s1_VBN_sdy.bin\"],\"success\":true}"));
}
/*Test cases for getAvailableFirmwareFiles ends here*/

/*******************************************************************************************************************
* Test function for :ArchiveLogs
* ArchiveLogs :
*             Compresses and uploads device logs into attached USB drive.
*             @return Whether the request succeeded.
*
*Event : onArchiveLogs
*             Triggered to archive the device logs and returns the status of the archive.
*
*                @return status of the archive.
* Use case coverage:
*                @Failure :7
*                @Success :2
********************************************************************************************************************/

/**
 * @brief : ArchiveLogs when path label is missing in the request parameter
 *          Check if  path label is missing from the parameters JSON object;
 *          then ArchiveLogs shall be failed and return Erorr code: ERROR_BAD_REQUEST
 *
 * @param[in]   :  Invalid parameters without the "path" label
 * @return      :  error code: ERROR_BAD_REQUEST
 */
TEST_F(UsbAccessEventTest, archiveLogsFailure_BadRequest)
{
    EXPECT_EQ(Core::ERROR_BAD_REQUEST, handler.Invoke(connection, _T("ArchiveLogs"), _T("{\"params\":{}}"), response));
}

/**
* @brief : onArchiveLogs event when setmntent returns nullptr,
*        Check if setmntent fails to open the mounts file,
*        then ArchiveLogs shall succeed and onArchiveLogs return Erorr code : No USB.
*
* @param[in]   :  "params": {"path":"run/sda1"}
* @return      :  "success":true
*/
TEST_F(UsbAccessEventTest, onArchiveLogsFailure_whenSetmntentValueNull)
{
    Core::Event onArchiveLogs(false, true);

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, setmntent(::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const char* command, const char* type) {
                EXPECT_EQ(string(command), string(_T("/proc/mounts")));
                return nullptr;
            }));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                std::string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.UsbAccess.onArchiveLogs\",\"params\":{\"error\":\"No USB\",\"success\":false,\"path\":\"\"}}");
                onArchiveLogs.SetEvent();
                return Core::ERROR_NONE;
          }));

    handler.Subscribe(0, _T("onArchiveLogs"), _T("org.rdk.UsbAccess"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("ArchiveLogs"), _T("{\"path\":\"/run/sda1\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onArchiveLogs.Lock());
    handler.Unsubscribe(0, _T("onArchiveLogs"), _T("org.rdk.UsbAccess"), message);
}

/**
* @brief : onArchiveLogs event when getmntent returns nullptr,
*        Check if getmntent fails to retrieve mount entries,
*        then ArchiveLogs shall succeed and onArchiveLogs return Erorr code : No USB.
*
* @param[in]   :  "params": {"path":"run/sda1"}
* @return      :  "success":true
*/
TEST_F(UsbAccessEventTest, onArchiveLogsFailure_whengetmntentNull)
{
    Core::Event onArchiveLogs(false, true);

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
             return static_cast<struct mntent*>(NULL);
     }));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                std::string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.UsbAccess.onArchiveLogs\",\"params\":{\"error\":\"No USB\",\"success\":false,\"path\":\"\"}}");
                onArchiveLogs.SetEvent();
                return Core::ERROR_NONE;
          }));

    handler.Subscribe(0, _T("onArchiveLogs"), _T("org.rdk.UsbAccess"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("ArchiveLogs"), _T("{\"path\":\"/run/sda1\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onArchiveLogs.Lock());
    handler.Unsubscribe(0, _T("onArchiveLogs"), _T("org.rdk.UsbAccess"), message);
}

/**
* @brief : onArchiveLogs event when device node is not found,
*        Check if the device node is not found in the devnodes list,
*        then ArchiveLogs shall succeed and onArchiveLogs return Erorr code : No USB.
*
* @param[in]   :  "params": {"path":"run/sda1"}
* @return      :  "success":true
*/
TEST_F(UsbAccessEventTest, onArchiveLogsFailure_when_devNodNotFound)
{
    Core::Event onArchiveLogs(false, true);

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda2");
        entry1.mnt_dir = const_cast<char*>("/run/sda1");

        static int callCount = 0;
        if (callCount == 0) {
           callCount++;
           return &entry1;
        } else {
           return static_cast<struct mntent*>(NULL);
        }
    }));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                std::string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.UsbAccess.onArchiveLogs\",\"params\":{\"error\":\"No USB\",\"success\":false,\"path\":\"\"}}");
                onArchiveLogs.SetEvent();
                return Core::ERROR_NONE;
          }));

    handler.Subscribe(0, _T("onArchiveLogs"), _T("org.rdk.UsbAccess"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("ArchiveLogs"), _T("{\"path\":\"/run/sda1\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onArchiveLogs.Lock());
    handler.Unsubscribe(0, _T("onArchiveLogs"), _T("org.rdk.UsbAccess"), message);
}

/**
* @brief : onArchiveLogs event when path Param was not found in Mountedpath,
*        Check if the path Param was not found in Mountedpath,
*        then ArchiveLogs shall succeed and onArchiveLogs return Erorr code : No USB.
*
* @param[in]   :  "params": {"path":"run/sda2"}
* @return      :  "success":true
*/
TEST_F(UsbAccessEventTest, onArchiveLogsFailure_when_pathParamNotFoundInMountedpath)
{
    Core::Event onArchiveLogs(false, true);

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1");
        entry1.mnt_dir = const_cast<char*>("/run/sda1");

        static int callCount = 0;
        if (callCount == 0) {
           callCount++;
           return &entry1;
        } else {
           return static_cast<struct mntent*>(NULL);
        }
    }));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                std::string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.UsbAccess.onArchiveLogs\",\"params\":{\"error\":\"No USB\",\"success\":false,\"path\":\"\"}}");
                onArchiveLogs.SetEvent();
                return Core::ERROR_NONE;
          }));

    handler.Subscribe(0, _T("onArchiveLogs"), _T("org.rdk.UsbAccess"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("ArchiveLogs"), _T("{\"path\":\"/run/sda2\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onArchiveLogs.Lock());
    handler.Unsubscribe(0, _T("onArchiveLogs"), _T("org.rdk.UsbAccess"), message);
}

/**
* @brief : onArchiveLogs event when file open is not Successful,
*        Check /lib/rdk/usbLogUpload.sh file open is not successful,
*        then ArchiveLogs shall succeed and onArchiveLogs return Erorr code : script error.
*
* @param[in]   :  "params": {"path":"run/sda1"}
* @return      :  "success":true
*/
TEST_F(UsbAccessEventTest, onArchiveLogsFailure_when_FileFailedtoOpen)
{
    Core::Event onArchiveLogs(false, true);

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1");
        entry1.mnt_dir = const_cast<char*>("/run/sda1");

        static int callCount = 0;
        if (callCount == 0) {
           callCount++;
           return &entry1;
        } else {
           return static_cast<struct mntent*>(NULL);
        }
    }));

    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const char* command, const char* type) {
                EXPECT_EQ(string(command), string(_T("/lib/rdk/usbLogUpload.sh /run/sda1")));
                return nullptr;
            }));

    ON_CALL(wrapsImplMock, pclose(::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](FILE* pipe){
                return -1;
            }));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.UsbAccess.onArchiveLogs\",\"params\":{\"error\":\"script error\",\"success\":false,\"path\":\"\"}}");
                onArchiveLogs.SetEvent();
                return Core::ERROR_NONE;
          }));

    handler.Subscribe(0, _T("onArchiveLogs"), _T("org.rdk.UsbAccess"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("ArchiveLogs"), _T("{\"path\":\"/run/sda1\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onArchiveLogs.Lock());
    handler.Unsubscribe(0, _T("onArchiveLogs"), _T("org.rdk.UsbAccess"), message);
}

/**
* @brief : onArchiveLogs when file termination is not successful,
*        Check /lib/rdk/usbLogUpload.sh file termination is not successful,
*        then ArchiveLogs shall succeed and onArchiveLogs return Erorr code : script error.
*
* @param[in]   :  "params": {"path":"run/sda1"}
* @return      :  "success":true
*/
TEST_F(UsbAccessEventTest, onArchiveLogsFailure_when_FileFailedtoTerminate)
{
    Core::Event onArchiveLogs(false, true);

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1");
        entry1.mnt_dir = const_cast<char*>("/run/sda1");

        static int callCount = 0;
        if (callCount == 0) {
           callCount++;
           return &entry1;
        } else {
           return static_cast<struct mntent*>(NULL);
        }
    }));

    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const char* command, const char* type) {
                EXPECT_EQ(string(command), string(_T("/lib/rdk/usbLogUpload.sh /run/sda1")));
                return __real_popen(command, type);
            }));

    ON_CALL(wrapsImplMock, pclose(::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](FILE* pipe){
                return -1;
            }));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.UsbAccess.onArchiveLogs\",\"params\":{\"error\":\"script error\",\"success\":false,\"path\":\"\"}}");
                onArchiveLogs.SetEvent();
                return Core::ERROR_NONE;
          }));

    handler.Subscribe(0, _T("onArchiveLogs"), _T("org.rdk.UsbAccess"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("ArchiveLogs"), _T("{\"path\":\"/run/sda1\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onArchiveLogs.Lock());
    handler.Unsubscribe(0, _T("onArchiveLogs"), _T("org.rdk.UsbAccess"), message);
}

/**
* @brief :onArchiveLogs event successful when path param is empty,
*        Check if any mounted paths are present, and archiving the logs into the first mounted path,
*        then onArchiveLogs shall succeed and return Erorr code : none.
*
* @param[in]   :  "params": {"path":""}
* @return      :  "success":true
*/

TEST_F(UsbAccessEventTest, archiveLogsSuccess_When_pathParamisEmpty)
{
    Core::Event onArchiveLogs(false, true);

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1");
        entry1.mnt_dir = const_cast<char*>("/run/sda1");

        static int callCount = 0;
        if (callCount == 0) {
           callCount++;
           return &entry1;
        } else {
           return static_cast<struct mntent*>(NULL);
        }
    }));

    ON_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
     .WillByDefault(::testing::Invoke(
        [&](const char* command, const char* type) -> FILE* {
            const char* valueToReturn = NULL;
            if (strcmp(command, "/lib/rdk/usbLogUpload.sh /run/sda1") == 0) {
                valueToReturn = "/run/sda1/5C3400F15492_Logs_12-05-22-10-41PM.tgz";
            }
            if (valueToReturn != NULL) {
                  char buffer[1024];
                  memset(buffer, 0, sizeof(buffer));
                  strncpy(buffer, valueToReturn, sizeof(buffer) - 1);
                  FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                  return pipe;
            } else {
                 return nullptr;
            }
        }));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                std::string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.UsbAccess.onArchiveLogs\",\"params\":{\"error\":\"none\",\"success\":true,\"path\":\"\\/run\\/sda1\\/5C3400F15492_Logs_12-05-22-10-41PM.tgz\"}}");
                onArchiveLogs.SetEvent();
                return Core::ERROR_NONE;
          }));


    handler.Subscribe(0, _T("onArchiveLogs"), _T("org.rdk.UsbAccess"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("ArchiveLogs"), _T("{\"path\":\"\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onArchiveLogs.Lock());
    handler.Unsubscribe(0, _T("onArchiveLogs"), _T("org.rdk.UsbAccess"), message);
}

/**
* @brief : onArchiveLogs event successful when valid path is passed,
*        Check if valid path is passed, and /lib/rdk/usbLogUpload.sh file is terminating,
*        then onArchiveLogs shall succeed and return Erorr code : none.
*
* @param[in]   :  "params": {"path":"run/sda1"}
* @return      :  "success":true
*/
TEST_F(UsbAccessEventTest, archiveLogsSuccess_onValidPath)
{
    Core::Event onArchiveLogs(false, true);

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1");
        entry1.mnt_dir = const_cast<char*>("/run/sda1");

        static int callCount = 0;
        if (callCount == 0) {
           callCount++;
           return &entry1;
        } else {
           return static_cast<struct mntent*>(NULL);
        }
    }));

    ON_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
     .WillByDefault(::testing::Invoke(
        [&](const char* command, const char* type) -> FILE* {
            const char* valueToReturn = NULL;
            if (strcmp(command, "/lib/rdk/usbLogUpload.sh /run/sda1") == 0) {
                valueToReturn = "/run/sda1/5C3400F15492_Logs_12-05-22-10-41PM.tgz";
            }
            if (valueToReturn != NULL) {
                  char buffer[1024];
                  memset(buffer, 0, sizeof(buffer));
                  strncpy(buffer, valueToReturn, sizeof(buffer) - 1);
                  FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                  return pipe;
            } else {
                 return nullptr;
            }
        }));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.UsbAccess.onArchiveLogs\",\"params\":{\"error\":\"none\",\"success\":true,\"path\":\"\\/run\\/sda1\\/5C3400F15492_Logs_12-05-22-10-41PM.tgz\"}}");
                onArchiveLogs.SetEvent();
                return Core::ERROR_NONE;
          }));

    handler.Subscribe(0, _T("onArchiveLogs"), _T("org.rdk.UsbAccess"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("ArchiveLogs"), _T("{\"path\":\"/run/sda1\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onArchiveLogs.Lock());
    handler.Unsubscribe(0, _T("onArchiveLogs"), _T("org.rdk.UsbAccess"), message);
}
/*Test cases for ArchiveLogs ends here*/

/*********************************************************************************************************
 * Test function for :createLink
 * createLink :Creates a symbolic link to the root folder of the USB drive.
 * If called, and a link already exists, then it errors out.
 *
 *
 *                @return The URL of the web server that points to this location
 * Use case coverage:
 *                @Success :3
 *                @Failure :4
 *******************************************************************************************************/


/**
 * @brief : createLink when send Bad request
 *          Check if  path parameter is missing from the parameters JSON object;
 *          then  createLink shall be failed and return Erorr code: ERROR_BAD_REQUEST
 *
 * @param[in]   :  Invalid parameters without the "path" label
 * @return      :  error code: ERROR_BAD_REQUEST
 */
TEST_F(UsbAccessTest, createLinkFailed_BadRequest)
{
    EXPECT_EQ(Core::ERROR_BAD_REQUEST, handler.Invoke(connection, _T("createLink"), _T("{\"MissingPathParam\":\"/run/media/sda1/logs/PreviousLogs\"}"), response));
}

/**
 * @brief : createLink When a symlink already exists for the given pathParam:
 *          Check if a symlink already exists for the given pathParam:;
 *          then  createLink shall be failed
 *
 * @param[in]   :  Any valid path parameter for eg: {\"path\":\"/run/media/sda1\"}
 * @return      :  error code: ERROR_GENERAL
 */
TEST_F(UsbAccessTest, createLinkFailed_when_LinkExistInthePathParam)
{
   //To simulate a symbolicLink already exist in the pathParam
    symlink("/run/media/sda1", "/tmp/usbdrive");

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));

    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1"); // Set the value for mnt_fsname
        entry1.mnt_dir = const_cast<char*>("/run/media/sda1");    // Set the value for mnt_dir

        static int callCount = 0;
        if (callCount == 0) {
            callCount++;
            return &entry1;
        } else {
            return static_cast<struct mntent*>(NULL);
        }
    }));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("createLink"), _T("{\"path\":\"/run/media/sda1\"}"), response));
    //Remove the symbolicLink created
    std::remove("/tmp/usbdrive");
}

/**
 * @brief : createLink request with empty param and symlink already exists for the first USB mount path.
 *          Check if no pathParam is passed ,then the first path in the paths list is used as pathParam
 *          But if symlink already exists for the this pathParam;then  createLink shall be failed
 *
 * @param[in]   :  No pathParam
 * @return      :  error code: ERROR_GENERAL
 */
TEST_F(UsbAccessTest, createLinkFailed_withEmptyParam_LinkExistIntheFirstUsbMountedPath)
{
    //To simulate symbolicLink already exist in the pathParam
    symlink("/run/media/sda1", "/tmp/usbdrive");

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));

    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1"); // Set the value for mnt_fsname
        entry1.mnt_dir = const_cast<char*>("/run/media/sda1");    // Set the value for mnt_dir

        static int callCount = 0;
        if (callCount == 0) {
            callCount++;
            return &entry1;
        } else {
            return static_cast<struct mntent*>(NULL);
        }
    }));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("createLink"), _T("{}"), response));
    //Remove the symbolicLink created
    std::remove("/tmp/usbdrive");
}

/**
 * @brief : createLink request with pathParam is not the first usb path and symlink already Not exit.
 *
 *          Check if symbolic Link not exist with the specific pathParam, then loop through the  stored m_CreatedLinkIds map
 *          for exact match, If no match found,the createLink shall be failed and cound not create synlink
 *          the baseURL is added to the response object
 *
 * @param[in]   :  pathParam different from first usb mounted path
 * @return      :  ERROR_GENERAL
 */
TEST_F(UsbAccessTest, createLinkFailed_with_m_CreatedLinkIds_bLink_NOT_Exists)
{
    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));

    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1"); // Set the value for mnt_fsname
        entry1.mnt_dir = const_cast<char*>("/run/media/sda1");    // Set the value for mnt_dir

        static int callCount = 0;
        if (callCount == 0) {
            callCount++;
            return &entry1;
        } else {
            return static_cast<struct mntent*>(NULL);
        }
    }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("createLink"), _T("{}"), response));
    std::remove("/tmp/usbdrive");

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1"); // Set the value for mnt_fsname
        entry1.mnt_dir = const_cast<char*>("/run/media/sda1");    // Set the value for mnt_dir

        static int callCount = 0;
        if (callCount == 0) {
            callCount++;
            return &entry1;
        } else {
            return static_cast<struct mntent*>(NULL);
        }
    }));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("createLink"), _T("{\"path\":\"/run/media/sdb1\"}"), response));
}

/**
 * @brief : createLink request with pathParam and symlink already Not exit
 *          Check if pathParam is passed ,and if symlink is not already exist
 *          in the specific pathParam;then  createLink shall be succeeded and
 *          the baseURL is added to the response object.
 *
 * @param[in]   :  Valid pathParam for eg:{\"path\":\"/run/media/sda1\"}
 * @return      :  baseURL as response
 */
TEST_F(UsbAccessTest, createLinkSuccess_whenReadLinkFailed_withPathParam)
{
    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));

    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1"); // Set the value for mnt_fsname
        entry1.mnt_dir = const_cast<char*>("/run/media/sda1");    // Set the value for mnt_dir

        static int callCount = 0;
        if (callCount == 0) {
            callCount++;
            return &entry1;
        } else {
            return static_cast<struct mntent*>(NULL);
        }
    }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("createLink"), _T("{\"path\":\"/run/media/sda1\"}"), response));
    EXPECT_EQ(response, string("{\"baseURL\":\"http:\\/\\/localhost:50050\\/usbdrive\",\"success\":true}"));
    //Remove the symbolicLink created
    std::remove("/tmp/usbdrive");
}

/**
 * @brief : createLink request without pathParam and symlink already Not exit
 *          Check if no pathParam is passed ,then the first path in the paths list is used as pathParam
 *          and if symlink is not already exist in this pathParam;
 *          then  createLink shall be succeeded and  the baseURL is added to the response object.
 *
 * @param[in]   :  No pathParam
 * @return      :  baseURL as response
 */
TEST_F(UsbAccessTest, createLinkSuccess_whenReadLinkFailed_withoutPathParam)
{
    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));

    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1"); // Set the value for mnt_fsname
        entry1.mnt_dir = const_cast<char*>("/run/media/sda1");    // Set the value for mnt_dir

        static int callCount = 0;
        if (callCount == 0) {
            callCount++;
            return &entry1;
        } else {
            return static_cast<struct mntent*>(NULL);
        }
    }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("createLink"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"baseURL\":\"http:\\/\\/localhost:50050\\/usbdrive\",\"success\":true}"));
    //Remove the symbolicLink created
    std::remove("/tmp/usbdrive");
}

/**
 * @brief : createLink request where link not exist with first USB drive and had to loop through stored m_CreatedLinkIds
 *          Check if symbolic Link not exist with the first USB drive, then loop through the  stored m_CreatedLinkIds map
 *          to see if it already has the link;If the Link is not already exist then  createLink shall be succeeded and
 *          the baseURL is added to the response object.
 *
 * @param[in]   :  Any valid input pathParam
 * @return      :  baseURL as response
 */
TEST_F(UsbAccessTest, createLinkSuccess_with_m_CreatedLinkIds_bLink_NOT_Exists_CreatedLinkForFor1stUsb)
{
    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));

    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1"); // Set the value for mnt_fsname
        entry1.mnt_dir = const_cast<char*>("/run/media/sda1");    // Set the value for mnt_dir

        static int callCount = 0;
        if (callCount == 0) {
            callCount++;
            return &entry1;
        } else {
            return static_cast<struct mntent*>(NULL);
        }
    }));
    /*As per this invoke call,created link for the first usb drive,
      then the link ID and path are added to the m_CreatedLinkIds map. */
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("createLink"), _T("{}"), response));

    /*Remove the above created symbolic link inorder to simulate the behavior as symlink NoT exists with first usbdrive
     *and need to loop through the  stored m_CreatedLinkIds map to see if it already has the link.*/
    std::remove("/tmp/usbdrive");

    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));
    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1"); // Set the value for mnt_fsname
        entry1.mnt_dir = const_cast<char*>("/run/media/sda1");    // Set the value for mnt_dir

        static int callCount = 0;
        if (callCount == 0) {
            callCount++;
            return &entry1;
        } else {
            return static_cast<struct mntent*>(NULL);
        }
    }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("createLink"), _T("{\"path\":\"/run/media/sda1\"}"), response));
    EXPECT_EQ(response, string("{\"baseURL\":\"http:\\/\\/localhost:50050\\/usbdrive\",\"success\":true}"));
    std::remove("/tmp/usbdrive");
}
/*Test cases for createLink ends here*/

/*********************************************************************************************************
 * Test function for :getLinks
 * getLinks :Returns a list of created links and the associated root folder of the USB drive.
 *
 *                @return Whether the request succeeded
 * Use case coverage:
 *                @Success :2
 *******************************************************************************************************/

/**
 * @brief : getLinks when there are no Links,
 *          Check if there is no symbolic link exists,
 *          then getLinks shall be succeded and returns empty links list.
 *
 * @param[in]   :  None
 * @return      :  {"links":[],"success":true}
 */
TEST_F(UsbAccessTest, getLinkSuccess_WhenNoSymLink)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getLinks"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"links\":[],\"success\":true}"));
}

/**
 * @brief : getLinks when symbolic links exist,
 *          Check if symbolic links are created and stored,
 *          then getLinks shall be succeded and returns a list of created links and the associated root folder of the USB drive.
 *
 * @param[in]   :  None
 * @return      :  {"links":["path":"run/media/sda1","baseURL":"http:/localhost:50050/usbdrive"],"success":true}
*/
TEST_F(UsbAccessTest, getLinkSuccess_whenSymLinkExist)
{
    EXPECT_CALL(udevImplMock, udev_enumerate_get_list_entry(testing::_))
        .WillOnce(testing::Return(reinterpret_cast<struct udev_list_entry*>(0x3)));

    EXPECT_CALL(udevImplMock, udev_list_entry_get_name(testing::_))
         .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(udevImplMock, udev_device_get_parent_with_subsystem_devtype(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(reinterpret_cast<struct udev_device*>(0x5)));

    EXPECT_CALL(udevImplMock, udev_device_get_devtype(testing::_))
        .WillRepeatedly(testing::Return("disk"));

    EXPECT_CALL(udevImplMock, udev_device_get_devnode(testing::_))
        .WillRepeatedly(testing::Return("/dev/sda1"));

    EXPECT_CALL(wrapsImplMock, getmntent(testing::_))
      .WillRepeatedly(::testing::Invoke(
       [&](FILE*) -> struct mntent* {
        static struct mntent entry1;

        entry1.mnt_fsname = const_cast<char*>("/dev/sda1"); // Set the value for mnt_fsname
        entry1.mnt_dir = const_cast<char*>("/run/media/sda1");    // Set the value for mnt_dir

        static int callCount = 0;
        if (callCount == 0) {
            callCount++;
            return &entry1;
        } else {
            return static_cast<struct mntent*>(NULL);
        }
    }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("createLink"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"baseURL\":\"http:\\/\\/localhost:50050\\/usbdrive\",\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getLinks"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"links\":[{\"path\":\"\\/run\\/media\\/sda1\",\"baseURL\":\"http:\\/\\/localhost:50050\\/usbdrive\"}],\"success\":true}"));
    //Remove the symbolicLink created
    std::remove("/tmp/usbdrive");

}
/*Test cases for getLinks ends here*/

/*********************************************************************************************************
 * Test function for :clearLink
 * clearLink :Clears or removes the symbolic link created by the createLink
 *
 *                @return Whether the request succeeded
 * Use case coverage:
 *                @Success :3
 *                @Failure :4
 *******************************************************************************************************/


/**
 * @brief : clearLink without baseURL
 *          Check if  input parameter baseURL is missing from the request parameters JSON object;
 *          then  clearLink shall be failed and return Erorr code: ERROR_BAD_REQUEST
 *
 * @param[in]   :  Invalid parameters without the "baseURL" label
 * @return      :  error code: ERROR_BAD_REQUEST
 */
TEST_F(UsbAccessTest, clearLinkFailed_onBadRequest)
{
    EXPECT_EQ(Core::ERROR_BAD_REQUEST, handler.Invoke(connection, _T("clearLink"), _T("{\"MissingBaseUrl\":\"http://localhost:50050/usbdrive\"}"), response));
}

/**
 * @brief : clearLink when input baseURL[LINK_URL_HTTP] not exists
 *          Check if  input parameter baseURL is the default URL [http://localhost:50050/usbdrive];and if this link does not exist;
 *          then  clearLink shall be failed
 *
 * @param[in]   :  valid baseURL
 * @return      :  error code: ERROR_GENERAL
 */
TEST_F(UsbAccessTest, clearLinkFailed_withDefaultInputBaseUrl_whenLinkNotExist)
{
    std::remove("/tmp/usbdrive");
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("clearLink"), _T("{\"baseURL\":\"http://localhost:50050/usbdrive\"}"), response));
}

/**
 * @brief : clearLink when input baseURL is other than [LINK_URL_HTTP] ,but this link does not exists
 *          Check if  input parameter baseURL is other than [http://localhost:50050/usbdrive];and if this link does not exist;
 *          then  clearLink shall be failed
 *
 * @param[in]   :  any valid baseURL other than http://localhost:50050/usbdrive
 * @return      :  error code: ERROR_GENERAL
 */
TEST_F(UsbAccessTest, clearLinkFailed_withNonDefaultInputBaseUrl_whenLinkNotExist)
{
    std::remove("/tmp/usbdrive123");
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("clearLink"), _T("{\"baseURL\":\"http://localhost:50050/usbdrive123\"}"), response));
}

/**
 *  @brief : clearLink with Invalid input baseURL.
 *           Check if  input parameter baseURL is invalid[ contains characters other than 0-9 digits];
 *           then clearLink shall be failed
 *
 *@param[in]   :  Invalid baseURL [contains characters other than 0-9 digits]
 *@return      :  error code: ERROR_GENERAL
 */
TEST_F(UsbAccessTest, clearLinkFailed_withInvalidInputBaseUrl)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("clearLink"), _T("{\"baseURL\":\"http://localhost:50050/usbdrive#_123\"}"), response));
}

/**
 * @brief : clearLink when input baseURL is empty
 *          Check if  input parameter baseURL is empty;
 *          then the default LINK PATH "/tmp/usbdrive" will be cleared
 *
 * @param[in]   :  any valid baseURL other than http://localhost:50050/usbdrive
 * @return      :  error code: ERROR_GENERAL
 */
TEST_F(UsbAccessTest, clearLinkSuccess_whenInputParamIsEmpty)
{
    symlink("/run/media/sda1", "/tmp/usbdrive");
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("clearLink"), _T("{\"baseURL\":}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

/**
 * @brief : clearLink when input baseURL is "http://localhost:50050/usbdrive"
 *          Check if  input parameter baseURL = http://localhost:50050/usbdrive;
 *          and if the LINK_PATH is presnt,then clearLink shall be succeeded and input baseURL will be cleared
 *
 * @param[in]   :  default baseURL http://localhost:50050/usbdrive
 * @return      :  error code: ERROR_GENERAL
 */
TEST_F(UsbAccessTest, clearLinkSuccess_withDefaultInputBaseUrl)
{
    symlink("/run/media/sda1", "/tmp/usbdrive");
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("clearLink"), _T("{\"baseURL\":\"http://localhost:50050/usbdrive\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

/**
 * @brief : clearLink with valid input baseURL is other than "http://localhost:50050/usbdrive".
 *          Check if  input parameter baseURL is Other than http://localhost:50050/usbdrive;
 *          and if the particular LINK_PATH is present,then clearLink shall be succeeded and input baseURL will be cleared
 *
 * @param[in]   :  default baseURL http://localhost:50050/usbdrive
 * @return      :  error code: ERROR_GENERAL
 */
TEST_F(UsbAccessTest, clearLinkSuccess_withNonDefaultInputBaseUrl)
{
    symlink("/run/media/sda1", "/tmp/usbdrive123");
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("clearLink"), _T("{\"baseURL\":\"http://localhost:50050/usbdrive123\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}
/*Test cases for clearLinks ends here*/

/*******************************************************************************************************************
*Test function for Event:onUSBMountChanged
*Event : onUSBMountChanged
*             Triggered when a USB drive is mounted or unmounted.
*
*                @return (i)mount status [true when the USB device is mounted or false when the USB device is unmounted]
*                        and (ii)the location where the device is mounted
* Use case coverage:
*                @Success :2
********************************************************************************************************************/

/**
 * @brief : onUSBMountChanged when a USB drive is mounted/unmounted
 *          Check onUSBMountChanged triggered successfully when a USB drive is mounted
 *          with mount status = true and the location where the device is mounted
 * @param[in]   :  This method takes no parameters.
 * @return      :  \"params\":{\"mounted\":true,\"device\":\"\\/dev\\/sda1\"}
 *
 */
TEST_F(UsbAccessEventIarmTest, onUSBMountChangedSuccess)
{
    Core::Event onUSBMountChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.UsbAccess.onUSBMountChanged\",\"params\":{\"mounted\":true,\"device\":\"\\/dev\\/sda1\"}}");

                onUSBMountChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onUSBMountChanged"), _T("org.rdk.UsbAccess"), message);

    IARM_Bus_SYSMgr_EventData_t usbEventData;
    usbEventData.data.usbMountData.mounted= 1;
    strncpy(usbEventData.data.usbMountData.dir, "/dev/sda1", sizeof(usbEventData.data.usbMountData.dir));
    usbEventData.data.usbMountData.dir[sizeof(usbEventData.data.usbMountData.dir) - 1] = '\0';
    eventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_USB_MOUNT_CHANGED, &usbEventData, 1);

    EXPECT_EQ(Core::ERROR_NONE, onUSBMountChanged.Lock());

    handler.Unsubscribe(0, _T("onUSBMountChanged"), _T("org.rdk.System"), message);
}
/*Test cases for onUSBMountChanged ends here*/