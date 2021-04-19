#include "UsbAccess.h"

#include <unistd.h>
#include <mntent.h>
#include <regex>
<<<<<<< HEAD
#include <libudev.h>
#include <algorithm>
=======

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
#include "libIARM.h"
#include "libIBus.h"
#include "sysMgr.h"
#endif /* USE_IARMBUS || USE_IARM_BUS */
>>>>>>> upstream/sprint/2102

const short WPEFramework::Plugin::UsbAccess::API_VERSION_NUMBER_MAJOR = 2;
const short WPEFramework::Plugin::UsbAccess::API_VERSION_NUMBER_MINOR = 0;
const string WPEFramework::Plugin::UsbAccess::SERVICE_NAME = "org.rdk.UsbAccess";
const string WPEFramework::Plugin::UsbAccess::METHOD_GET_FILE_LIST = "getFileList";
const string WPEFramework::Plugin::UsbAccess::METHOD_CREATE_LINK = "createLink";
const string WPEFramework::Plugin::UsbAccess::METHOD_CLEAR_LINK = "clearLink";
const string WPEFramework::Plugin::UsbAccess::METHOD_GET_AVAILABLE_FIRMWARE_FILES = "getAvailableFirmwareFiles";
const string WPEFramework::Plugin::UsbAccess::METHOD_GET_MOUNTED = "getMounted";
const string WPEFramework::Plugin::UsbAccess::METHOD_UPDATE_FIRMWARE = "updateFirmware";
const string WPEFramework::Plugin::UsbAccess::LINK_URL_HTTP = "http://localhost:50050/usbdrive";
const string WPEFramework::Plugin::UsbAccess::LINK_PATH = "/tmp/usbdrive";
<<<<<<< HEAD

namespace WPEFramework {
namespace Plugin {

    namespace {
        string joinPaths(const string& path1, const string& path2) {
            string result = path1;
            if (!result.empty() && !path2.empty() && *path2.begin() != '/' && *result.rbegin() != '/')
                result.append(1, '/');
            result.append(path2);
            return result;
        }

        bool createLink(const string& from, const string& to) {
            return (0 == symlink(from.c_str(), to.c_str()));
        }

        bool clearLink(const string& to) {
            return (0 == remove(to.c_str()));
        }

        int runScript(const char *command) {
            int result = -1;
            FILE *pipe = nullptr;
            if ((pipe = popen(command, "r"))) {
                result = pclose(pipe);
            }
            return result;
        }
    }

    SERVICE_REGISTRATION(UsbAccess, UsbAccess::API_VERSION_NUMBER_MAJOR, UsbAccess::API_VERSION_NUMBER_MINOR);

    UsbAccess::UsbAccess()
    : AbstractPlugin(UsbAccess::API_VERSION_NUMBER_MAJOR)
    {
        registerMethod(METHOD_GET_FILE_LIST, &UsbAccess::getFileListWrapper, this);
        registerMethod(METHOD_CREATE_LINK, &UsbAccess::createLinkWrapper, this);
        registerMethod(METHOD_CLEAR_LINK, &UsbAccess::clearLinkWrapper, this);
        registerMethod(METHOD_GET_AVAILABLE_FIRMWARE_FILES, &UsbAccess::getAvailableFirmwareFilesWrapper, this, {2});
        registerMethod(METHOD_GET_MOUNTED, &UsbAccess::getMountedWrapper, this, {2});
        registerMethod(METHOD_UPDATE_FIRMWARE, &UsbAccess::updateFirmware, this, {2});
    }

    UsbAccess::~UsbAccess()
    {
    }

    const string UsbAccess::Initialize(PluginHost::IShell * /* service */)
    {
        return "";
    }

    void UsbAccess::Deinitialize(PluginHost::IShell * /* service */)
    {
    }

    string UsbAccess::Information() const
    {
        return (string("{\"service\": \"") + SERVICE_NAME + string("\"}"));
    }

    // Registered methods (wrappers) begin
    uint32_t UsbAccess::getFileListWrapper(const JsonObject &parameters, JsonObject &response)
    {
        LOGINFOMETHOD();

        bool result = false;

        string pathParam;
        if (parameters.HasLabel("path"))
            pathParam = parameters["path"].String();

        FileList files;
        std::list<string> paths;
        getMounted(paths);
        if (!paths.empty())
            result = getFileList(joinPaths(*paths.begin(), pathParam), files,
                    "([\\w-]*)\\.(png|jpg|jpeg|tiff|tif|bmp|mp4|mov|avi|mp3|wav|m4a|flac|mp4|aac|wma|txt|bin|enc)",
                    true);

        if (!result)
            response["error"] = "not found";
        else
=======
const string WPEFramework::Plugin::UsbAccess::EVT_ON_USB_FIRMWARE_UPDATE = "onUSBFirmwareUpdate";

using namespace std;

namespace WPEFramework {
    namespace Plugin {

        namespace {
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
        void _usbFirmwareUpdateStateChanged(const char *, IARM_EventId_t eventId, void *data, size_t)
        {
            if (eventId != IARM_BUS_SYSMGR_EVENT_USB_FW_UPDATE) return;

            IARM_Bus_SYSMgr_EventData_t *sysEventData = (IARM_Bus_SYSMgr_EventData_t*)data;
            const char *status = &sysEventData->data.usbFirmwareUpdate.status[0];
            LOGWARN("IARMEvt: IARM_BUS_SYSMGR_EVENT_USB_FW_UPDATE = '%s'\n", status);

            if (UsbAccess::_instance) {
                UsbAccess::_instance->onUSBFirmwareUpdate(status);
            } else {
                LOGERR("UsbAccess::_instance is NULL.\n");
            }
        }
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */
        }

        SERVICE_REGISTRATION(UsbAccess, UsbAccess::API_VERSION_NUMBER_MAJOR, UsbAccess::API_VERSION_NUMBER_MINOR);

        UsbAccess* UsbAccess::_instance = nullptr;

        UsbAccess::UsbAccess()
            : AbstractPlugin()
        {
            UsbAccess::_instance = this;
            registerMethod(METHOD_GET_FILE_LIST, &UsbAccess::getFileListWrapper, this);
            registerMethod(METHOD_CREATE_LINK, &UsbAccess::createLinkWrapper, this);
            registerMethod(METHOD_CLEAR_LINK, &UsbAccess::clearLinkWrapper, this);
        }

        UsbAccess::~UsbAccess()
        {
        }

        const string UsbAccess::Initialize(PluginHost::IShell* /* service */)
        {
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
            InitializeIARM();
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */
            return "";
        }

        void UsbAccess::Deinitialize(PluginHost::IShell* /* service */)
        {
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
            DeinitializeIARM();
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */
            UsbAccess::_instance = nullptr;
        }

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
        void UsbAccess::InitializeIARM()
        {
            if (Utils::IARM::init())
            {
                IARM_Result_t res;
                IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_USB_FW_UPDATE, _usbFirmwareUpdateStateChanged));
            }
        }

        void UsbAccess::DeinitializeIARM()
        {
            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                IARM_CHECK(IARM_Bus_UnRegisterEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_USB_FW_UPDATE));
            }
        }
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */

        string UsbAccess::Information() const
>>>>>>> upstream/sprint/2102
        {
            JsonArray arr;
            for_each(files.begin(), files.end(), [&arr](const PathInfo& it)
            {
                JsonObject ent;
                ent["name"] = it.first.c_str();
                ent["t"] = it.second.c_str();
                arr.Add(ent);
            });
            response["contents"] = arr;
        }

        returnResponse(result);
    }

    uint32_t UsbAccess::createLinkWrapper(const JsonObject &parameters, JsonObject &response)
    {
        LOGINFOMETHOD();

        bool result = false;

        std::list<string> paths;
        getMounted(paths);
        if (!paths.empty())
            result = createLink(*paths.begin(), LINK_PATH);

        if (result)
            response["baseURL"] = LINK_URL_HTTP;
        else
            response["error"] = "could not create symlink";

        returnResponse(result);
    }

    uint32_t UsbAccess::clearLinkWrapper(const JsonObject &parameters, JsonObject &response)
    {
        LOGINFOMETHOD();

        bool result = false;

        result = clearLink(LINK_PATH);
        if (!result)
            response["error"] = "could not remove symlink";

        returnResponse(result);
    }

    uint32_t UsbAccess::getAvailableFirmwareFilesWrapper(const JsonObject &parameters, JsonObject &response)
    {
        LOGINFOMETHOD();

        bool result = true;

        std::list<string> paths;
        getMounted(paths);

        JsonArray arr;
        for_each(paths.begin(), paths.end(), [&arr](const string& it)
        {
            FileList files;
            getFileList(it, files, "([\\w-]*)\\.bin", false);
            for_each(files.begin(), files.end(), [&arr,&it](const PathInfo& jt)
            {
                arr.Add(joinPaths(it, jt.first));
            });
        });
        response["availableFirmwareFiles"] = arr;

        returnResponse(result);
    }

<<<<<<< HEAD
    uint32_t UsbAccess::updateFirmware(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFOMETHOD();
=======
        void UsbAccess::onUSBFirmwareUpdate(const char *status)
        {
            JsonObject params;
            params["status"] = status;
            sendNotify(C_STR(EVT_ON_USB_FIRMWARE_UPDATE), params);
        }

        bool UsbAccess::getFileList(const string& dir, FileList& files) const
        {
            bool success = false;
>>>>>>> upstream/sprint/2102

        bool result = false;

<<<<<<< HEAD
        string fileName;
        if (parameters.HasLabel("fileName"))
            fileName = parameters["fileName"].String();
=======
            DIR* dirp = opendir(dir.c_str());
            if (dirp != nullptr)
            {
                struct dirent * dp;
                while ((dp = readdir(dirp)) != nullptr)
                {
                    if (dp->d_type == DT_DIR)
                        files.emplace_back(dp->d_name, "d");
                    else
                    {
                        if (std::regex_match(dp->d_name, std::regex(
                                "([\\w-]*)\\.(png|jpg|jpeg|tiff|tif|bmp|mp4|mov|avi|mp3|wav|m4a|flac|mp4|aac|wma|txt|bin|enc)",
                                std::regex_constants::icase)) == true)
                            files.emplace_back(dp->d_name, "f");
                        else
                            LOGWARN("unsupported file name: '%s'", dp->d_name);
                    }
                }
                closedir(dirp);
>>>>>>> upstream/sprint/2102

        string name = fileName.substr(fileName.find_last_of("/\\") + 1);
        string path = fileName.substr(0, fileName.find_last_of("/\\"));
        if (!name.empty() && !path.empty() &&
            std::regex_match(name, std::regex("([\\w-]*)\\.bin", std::regex_constants::icase)) == true)
        {
            char buff[1000];
            size_t n = sizeof(buff);
            int size = snprintf(buff, n,
                    "/lib/rdk/userInitiatedFWDnld.sh %s %s %s %d >> /opt/logs/swupdate.log &",
                    "usb",
                    path.c_str(),
                    name.c_str(),
                    0);
            if (size > 0 && size < n)
            {
                int rc = runScript(buff);
                LOGINFO("'%s' return code: %d", buff, rc);
                result = true;
            }
        }

        if (!result)
            response["error"] = "invalid filename";

        returnResponse(result);
    }

    uint32_t UsbAccess::getMountedWrapper(const JsonObject &parameters, JsonObject &response)
    {
        LOGINFOMETHOD();

        bool result = true;

        std::list<string> paths;
        getMounted(paths);

        JsonArray arr;
        for_each(paths.begin(), paths.end(), [&arr](const string& it)
        {
            arr.Add(it.c_str());
        });
        response["mounted"] = arr;

        returnResponse(result);
    }

    // internal methods
    bool UsbAccess::getFileList(const string& path, FileList& files, const string& fileRegex, bool includeFolders)
    {
        bool result = false;

        if (!path.empty())
        {
            DIR *dirp = opendir(path.c_str());
            if (dirp != nullptr)
            {
                files.clear();

                struct dirent *dp;
                while ((dp = readdir(dirp)) != nullptr)
                {
                    if (dp->d_type == DT_DIR)
                    {
                        if (includeFolders)
                            files.emplace_back(dp->d_name, "d");
                    }
                    else if (fileRegex.empty() ||
                            std::regex_match(dp->d_name, std::regex(fileRegex, std::regex_constants::icase)) == true)
                        files.emplace_back(dp->d_name, "f");
                }
                closedir(dirp);

                result = true;
            }
        }

        return result;
    }

    void UsbAccess::getMounted(std::list <std::string>& paths)
    {
        std::list<std::string> devnodes;

        struct udev *udev = udev_new();

        struct udev_enumerate *enumerate = udev_enumerate_new(udev);
        udev_enumerate_add_match_subsystem(enumerate, "block");
        udev_enumerate_scan_devices(enumerate);

        struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
        struct udev_list_entry *entry;

        if (devices)
        {
            udev_list_entry_foreach(entry, devices)
            {
                const char *path = udev_list_entry_get_name(entry);
                struct udev_device *dev = udev_device_new_from_syspath(udev, path);
                struct udev_device *usb = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
                if (usb != nullptr && std::regex_match(udev_device_get_devtype(dev), std::regex("(partition|disk)")))
                    devnodes.emplace_back(udev_device_get_devnode(dev));

                udev_device_unref(dev);
            }
        }

        udev_enumerate_unref(enumerate);
        udev_unref(udev);

        std::map<std::string,std::string> mapping;

        struct mntent *ent;
        FILE *file = setmntent("/proc/mounts", "r");
        if (file != nullptr)
        {
            while (nullptr != (ent = getmntent(file)))
                if (std::find(devnodes.begin(), devnodes.end(), string(ent->mnt_fsname)) != devnodes.end())
                    mapping.emplace(string(ent->mnt_fsname), string(ent->mnt_dir));

            endmntent(file);
        }

        for (auto const& x : mapping)
            paths.emplace_back(x.second);
    }
} // namespace Plugin
} // namespace WPEFramework
