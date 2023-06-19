#include <gtest/gtest.h>
#include <mntent.h>

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
    string response;

    UsbAccessTest()
        : plugin(Core::ProxyType<Plugin::UsbAccess>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
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

TEST_F(UsbAccessTest, UpdateFirmware)
{
    UdevImplMock udevImplMock;
    WrapsImplMock wrapsImplMock;

    Udev::getInstance().impl = &udevImplMock;
    Wraps::getInstance().impl = &wrapsImplMock;

    EXPECT_CALL(wrapsImplMock, system(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const char* command) {
                EXPECT_EQ(string(command), string(_T("/lib/rdk/userInitiatedFWDnld.sh usb '/tmp;reboot;' 'my.bin' 0 >> /opt/logs/swupdate.log &")));

                return 0;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("updateFirmware"), _T("{\"fileName\":\"/tmp;reboot;/my.bin\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("updateFirmware"), _T("{\"fileName\":\"/tmp\';reboot;/my.bin\"}"), response));

    Udev::getInstance().impl = nullptr;
    Wraps::getInstance().impl = nullptr;
}

/*******************************************************************************************************************
 * Test function for :getFileList
 * getDeviceInfo :
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
    NiceMock<UdevImplMock> udevImplMock;
    NiceMock<WrapsImplMock> wrapsImplMock;

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

    Udev::getInstance().impl = nullptr;
    Wraps::getInstance().impl = nullptr;
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
    NiceMock<UdevImplMock> udevImplMock;
    NiceMock<WrapsImplMock> wrapsImplMock;

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

    Udev::getInstance().impl = nullptr;
    Wraps::getInstance().impl = nullptr;
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
    NiceMock<UdevImplMock> udevImplMock;
    NiceMock<WrapsImplMock> wrapsImplMock;

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

    Udev::getInstance().impl = nullptr;
    Wraps::getInstance().impl = nullptr;
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
    NiceMock<UdevImplMock> udevImplMock;
    NiceMock<WrapsImplMock> wrapsImplMock;

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

    Udev::getInstance().impl = nullptr;
    Wraps::getInstance().impl = nullptr;
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
    NiceMock<UdevImplMock> udevImplMock;
    NiceMock<WrapsImplMock> wrapsImplMock;

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
    EXPECT_EQ(response, string("{\"path\":\"\\/run\\/media\\/sda1\\/logs\\/PreviousLogs\",\"contents\":[{\"name\":\"logFile.txt\",\"t\":\"f\"},{\"name\":\"..\",\"t\":\"d\"},{\"name\":\".\",\"t\":\"d\"}],\"success\":true}"));
    Udev::getInstance().impl = nullptr;
    Wraps::getInstance().impl = nullptr;
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
    NiceMock<UdevImplMock> udevImplMock;
    NiceMock<WrapsImplMock> wrapsImplMock;

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
EXPECT_EQ(response, string("{\"path\":\"\\/run\\/media\\/sda1\\/logs\",\"contents\":[{\"name\":\"..\",\"t\":\"d\"},{\"name\":\"PreviousLogs\",\"t\":\"d\"},{\"name\":\".\",\"t\":\"d\"}],\"success\":true}"));

    Udev::getInstance().impl = nullptr;
    Wraps::getInstance().impl = nullptr;
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
    NiceMock<UdevImplMock> udevImplMock;
    NiceMock<WrapsImplMock> wrapsImplMock;

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
    EXPECT_EQ(response, string("{\"path\":\"\\/run\\/media\\/sda1\\/logs\\/PreviousLogs\",\"contents\":[{\"name\":\"logFile.txt\",\"t\":\"f\"},{\"name\":\"..\",\"t\":\"d\"},{\"name\":\".\",\"t\":\"d\"}],\"success\":true}"));

    Udev::getInstance().impl = nullptr;
    Wraps::getInstance().impl = nullptr;
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
    NiceMock<UdevImplMock> udevImplMock;
    NiceMock<WrapsImplMock> wrapsImplMock;

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

    Udev::getInstance().impl = nullptr;
    Wraps::getInstance().impl = nullptr;
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
    NiceMock<UdevImplMock> udevImplMock;
    NiceMock<WrapsImplMock> wrapsImplMock;

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
    Udev::getInstance().impl = nullptr;
    Wraps::getInstance().impl = nullptr;
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
    NiceMock<UdevImplMock> udevImplMock;
    NiceMock<WrapsImplMock> wrapsImplMock;

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

    Udev::getInstance().impl = nullptr;
    Wraps::getInstance().impl = nullptr;
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
    NiceMock<UdevImplMock> udevImplMock;
    NiceMock<WrapsImplMock> wrapsImplMock;

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

    Udev::getInstance().impl = nullptr;
    Wraps::getInstance().impl = nullptr;
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
    NiceMock<UdevImplMock> udevImplMock;
    NiceMock<WrapsImplMock> wrapsImplMock;

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
    Udev::getInstance().impl = nullptr;
    Wraps::getInstance().impl = nullptr;
}
/*Test cases for getMounted ends here*/
