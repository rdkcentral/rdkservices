#include "UsbAccess.h"

#include <unistd.h>
#include <mntent.h>
#include <regex>
#include <libudev.h>
#include <algorithm>
#include <mutex>
#include <fstream>

#include "libIBus.h"
#include "sysMgr.h"

#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 2
const string WPEFramework::Plugin::UsbAccess::SERVICE_NAME = "org.rdk.UsbAccess";
const string WPEFramework::Plugin::UsbAccess::METHOD_GET_FILE_LIST = "getFileList";
const string WPEFramework::Plugin::UsbAccess::METHOD_CREATE_LINK = "createLink";
const string WPEFramework::Plugin::UsbAccess::METHOD_CLEAR_LINK = "clearLink";
const string WPEFramework::Plugin::UsbAccess::METHOD_GET_AVAILABLE_FIRMWARE_FILES = "getAvailableFirmwareFiles";
const string WPEFramework::Plugin::UsbAccess::METHOD_GET_MOUNTED = "getMounted";
const string WPEFramework::Plugin::UsbAccess::METHOD_UPDATE_FIRMWARE = "updateFirmware";
const string WPEFramework::Plugin::UsbAccess::METHOD_ARCHIVE_LOGS = "ArchiveLogs";
const string WPEFramework::Plugin::UsbAccess::LINK_URL_HTTP = "http://localhost:50050/usbdrive";
const string WPEFramework::Plugin::UsbAccess::LINK_PATH = "/tmp/usbdrive";
const string WPEFramework::Plugin::UsbAccess::EVT_ON_USB_MOUNT_CHANGED = "onUSBMountChanged";
const string WPEFramework::Plugin::UsbAccess::EVT_ON_ARCHIVE_LOGS = "onArchiveLogs";
const string WPEFramework::Plugin::UsbAccess::REGEX_BIN = "[\\w-]*\\.{0,1}[\\w-]*\\.bin";
const string WPEFramework::Plugin::UsbAccess::REGEX_FILE =
        "[\\w-]*\\.{0,1}[\\w ()-]*\\.(png|jpg|jpeg|tiff|tif|bmp|mp4|mov|avi|mp3|wav|m4a|flac|mp4|aac|wma|txt|bin|enc|ts)";
const string WPEFramework::Plugin::UsbAccess::PATH_DEVICE_PROPERTIES = "/etc/device.properties";
const std::list<string> WPEFramework::Plugin::UsbAccess::ADDITIONAL_FW_PATHS {"UsbTestFWUpdate", "UsbProdFWUpdate"};
const string WPEFramework::Plugin::UsbAccess::ARCHIVE_LOGS_SCRIPT = "/lib/rdk/usbLogUpload.sh";
const WPEFramework::Plugin::UsbAccess::ArchiveLogsErrorMap WPEFramework::Plugin::UsbAccess::ARCHIVE_LOGS_ERRORS = {
        {ScriptError,  "script error"},
        {None,         "none"},
        {Locked,       "Locked"},
        {NoUSB,        "No USB"},
        {WritingError, "Writing Error"}
};

// TODO: remove this
#define registerMethod(...) for (uint8_t i = 1; GetHandler(i); i++) GetHandler(i)->Register<JsonObject, JsonObject>(__VA_ARGS__)

namespace WPEFramework {

namespace {

    static Plugin::Metadata<Plugin::UsbAccess> metadata(
        // Version (Major, Minor, Patch)
        API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
        // Preconditions
        {},
        // Terminations
        {},
        // Controls
        {}
    );
}

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
            int result = system(command);
            if (result != -1 && WIFEXITED(result)) {
                result = WEXITSTATUS(result);
            }
            return result;
        }

        string findProp(const char* filename, const char* prop) {
            string result;
            std::ifstream fs(filename, std::ifstream::in);
            std::string::size_type delimpos;
            std::string line;

            if (!fs.fail()) {
                while (std::getline(fs, line)) {
                    if (!line.empty() &&
                        ((delimpos = line.find('=')) > 0)) {
                        std::string itemKey = line.substr(0, delimpos);
                        if (itemKey.compare(prop) == 0) {
                            result = line.substr(delimpos + 1, std::string::npos);
                            break;
                        }
                    }
                }
            }
            return result;
        }

        string deviceSpecificRegexBin() {
            static string result;
            static std::once_flag flag;
            std::call_once(flag, [&]() {
                string model = findProp(UsbAccess::PATH_DEVICE_PROPERTIES.c_str(), "MODEL_NUM");
                result = (model + UsbAccess::REGEX_BIN);

                LOGINFO("bin file regex for device '%s' is '%s'", model.c_str(), result.c_str());
            });
            return result;
        }

        time_t fileModTime(const char* filename) {
            struct stat st;
            time_t mod_time = 0;
            if (stat(filename, &st) == 0)
                mod_time = st.st_mtime;
            return mod_time;
        }

        bool isAbsPath(const string& mountPath, const string& givenPath) {
            if(givenPath.rfind(mountPath, 0) == 0)
                return true;
            else
                return false;
        }

        //From usb number 2, it returns the next available number
        int nextAvailableNumber(const std::set<int>& numList) {
            int nNum = 2;
            for(auto const& it : numList) {
                if(it != nNum) {
                    break;
                }
                nNum++;
            }
            return nNum;
        }

        //Returns if the incomingPath already has existing link
        bool isSymlinkExists(string& availableLink, string& incomingPath)
        {
            bool bLinkExists = false;
            char targetPath[256];
            int len = readlink(availableLink.c_str(), targetPath, sizeof(targetPath));
            if(len == -1)
            {
            	LOGINFO("readlink error");
            }
            else
            {
                targetPath[len] = '\0';
                LOGINFO("target path : %s", targetPath);
                if(incomingPath.compare(string(targetPath)) == 0)
                {
                    LOGINFO("Symlink already exists");
                    bLinkExists = true;
                }
            }
            return bLinkExists;
        }
    }

    SERVICE_REGISTRATION(UsbAccess, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    UsbAccess* UsbAccess::_instance = nullptr;

    UsbAccess::UsbAccess()
    : PluginHost::JSONRPC()
    {
        UsbAccess::_instance = this;

        CreateHandler({2});
        registerMethod(_T("getFileList"), &UsbAccess::getFileListWrapper, this);
        registerMethod(_T("createLink"), &UsbAccess::createLinkWrapper, this);
        registerMethod(_T("clearLink"), &UsbAccess::clearLinkWrapper, this);
        registerMethod(_T("getAvailableFirmwareFiles"), &UsbAccess::getAvailableFirmwareFilesWrapper, this);
        registerMethod(_T("getMounted"), &UsbAccess::getMountedWrapper, this);
        registerMethod(_T("updateFirmware"), &UsbAccess::updateFirmware, this);
        registerMethod(_T("ArchiveLogs"), &UsbAccess::archiveLogs, this);
    }

    UsbAccess::~UsbAccess()
    {
        UsbAccess::_instance = nullptr;

        if (archiveLogsThread.joinable())
            archiveLogsThread.join();
    }

    const string UsbAccess::Initialize(PluginHost::IShell * /* service */)
    {
        InitializeIARM();
        return "";
    }

    void UsbAccess::Deinitialize(PluginHost::IShell * /* service */)
    {
        DeinitializeIARM();
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
        {
            string absPath;
            //Loop through all the paths to match for absolute path
            for(auto const& it : paths)
            {
                if(isAbsPath(it, pathParam))
                {
                    absPath = pathParam;
                    break;
                }
            }
            //If absolute path is not found, treat it as relative path
            if(absPath.empty())
            {
                absPath = joinPaths(*paths.begin(), pathParam);
            }
            LOGINFO("Srikanth - absPath2 = %s", absPath.c_str());
            result = getFileList(absPath, files, REGEX_FILE, true);
        }

        if (!result)
            response["error"] = "not found";
        else
        {
            JsonArray arr;
            for_each(files.begin(), files.end(), [&arr](const FileEnt& it)
            {
                JsonObject ent;
                ent["name"] = it.filename;
                ent["t"] = string(1, it.fileType);
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
        string baseURL = LINK_URL_HTTP;

		string pathParam;
		if (parameters.HasLabel("path"))
			pathParam = parameters["path"].String();

		bool bLinkExists = false;

        //Loop through all the existing IDs and set bLinkExists to true, if it already has the link
        for(auto itr : m_CreatedLinkIds)
        {
            string linkPath = LINK_PATH+std::to_string(itr);
			bLinkExists = isSymlinkExists(linkPath, pathParam);
			if(bLinkExists)
				break;
		}

        getMounted(paths);
        if (!paths.empty() && (!bLinkExists))
        {
            string pathParam;
            if (parameters.HasLabel("path"))
                pathParam = parameters["path"].String();

            //For the first usb drive, create link with out number suffix
            if( pathParam.empty() || ((*paths.begin()).compare(pathParam)) == 0 )
            {
                result = createLink(*paths.begin(), LINK_PATH);
            }
            else
            {
                //Loop through all the paths for exact match and create link
                for(auto const& it : paths)
                {
                    if(it.compare(pathParam) == 0)
                    {
                        int nUsbNum = nextAvailableNumber(m_CreatedLinkIds);
                        LOGINFO("USB Link num = %d", nUsbNum);
                        baseURL = baseURL + std::to_string(nUsbNum);    //Add number suffix
                        result = createLink(it, LINK_PATH+std::to_string(nUsbNum));
						if (result)
							m_CreatedLinkIds.insert(nUsbNum);
                        break;
                    }
                }
            }
        }

        if (result)
	        response["baseURL"] = baseURL;
        else
            response["error"] = "could not create symlink";

        returnResponse(result);
    }

    uint32_t UsbAccess::clearLinkWrapper(const JsonObject &parameters, JsonObject &response)
    {
        LOGINFOMETHOD();

        bool result = false;

        int nUsbNum = 1;
        string urlParam;
        if (parameters.HasLabel("baseURL"))
            urlParam = parameters["baseURL"].String();

        LOGINFO("Clearing path: %s", urlParam.c_str());
        if(urlParam.empty() || (urlParam.compare(LINK_URL_HTTP)) == 0)
        {
            result = clearLink(LINK_PATH);
        }
        else
        {
            //Validate incoming path
            std::smatch match;
            LOGINFO("Path param = %s", urlParam.c_str());
            if (regex_search(urlParam, match, std::regex("http://localhost:50050/usbdrive[0-9]+")) &&  match.size() >= 1) {
                nUsbNum = std::stoi(urlParam.substr(urlParam.find_last_not_of("0123456789") + 1));
                LOGINFO("USB num = %d", nUsbNum);
                result = clearLink(LINK_PATH+std::to_string(nUsbNum));
                if (result)
                    m_CreatedLinkIds.erase(nUsbNum);
            }
        }

        if (!result)
            response["error"] = "could not remove symlink";

        LOGINFO("nUsbNum = %d", nUsbNum);
        returnResponse(result);
    }

    uint32_t UsbAccess::getAvailableFirmwareFilesWrapper(const JsonObject &parameters, JsonObject &response)
    {
        LOGINFOMETHOD();

        bool result = false;

        std::list<string> paths;
        result = getMounted(paths);

        for (auto it = paths.begin(); it != paths.end(); ++it)
            for (auto jt = ADDITIONAL_FW_PATHS.begin(); jt != ADDITIONAL_FW_PATHS.end(); ++jt)
                paths.insert(it, joinPaths(*it, *jt));

        JsonArray arr;
        std::list<string> allFiles;
        for_each(paths.begin(), paths.end(), [&allFiles](const string& it)
        {
            FileList files;
            getFileList(it, files, deviceSpecificRegexBin(), false);
            for_each(files.begin(), files.end(), [&allFiles, &it](const FileEnt& jt) {
                allFiles.emplace_back(joinPaths(it, jt.filename));
            });
        });
        // sort list in ascending order based on time, with newest image being the last in the list.
        allFiles.sort([](const string &a, const string &b)
        {
            return std::difftime(fileModTime(a.c_str()), fileModTime(b.c_str())) < 0;
        });
        for_each(allFiles.begin(), allFiles.end(), [&arr](const string& it)
        {
            arr.Add(it);
        });
        response["availableFirmwareFiles"] = arr;

        returnResponse(result);
    }

    uint32_t UsbAccess::updateFirmware(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFOMETHOD();

        bool result = false;

        string fileName;
        if (parameters.HasLabel("fileName"))
            fileName = parameters["fileName"].String();

        string name = fileName.substr(fileName.find_last_of("/\\") + 1);
        string path = fileName.substr(0, fileName.find_last_of("/\\"));

        if (fileName.find('\'') == string::npos &&
            !name.empty() && !path.empty() &&
            std::regex_match(name, std::regex(deviceSpecificRegexBin(), std::regex_constants::icase)) == true)
        {
            char buff[1000];
            size_t n = sizeof(buff);
            int size = snprintf(buff, n,
                    "/lib/rdk/userInitiatedFWDnld.sh %s '%s' '%s' %d >> /opt/logs/swupdate.log &",
                    "usb",
                    path.c_str(),
                    name.c_str(),
                    0);
            if (size > 0 && (size_t)size < n)
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

        bool result = false;

        std::list<string> paths;
        result = getMounted(paths);

        JsonArray arr;
        for_each(paths.begin(), paths.end(), [&arr](const string& it)
        {
            arr.Add(it.c_str());
        });
        response["mounted"] = arr;

        returnResponse(result);
    }

    uint32_t UsbAccess::archiveLogs(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFOMETHOD();

        if (archiveLogsThread.joinable())
            archiveLogsThread.join();

        m_oArchiveParams = parameters;
        archiveLogsThread = std::thread(&UsbAccess::archiveLogsInternal, this);

        returnResponse(true);
    }

    void UsbAccess::archiveLogsInternal()
    {
        ArchiveLogsError error = ScriptError;

        LOGINFO("Inside achivelogs");
        string usbFilePath;
        std::list<string> paths;
        getMounted(paths);
        if (paths.empty())
            error = NoUSB;
        else
        {
            string pathParam;
            if (m_oArchiveParams.HasLabel("path"))
                pathParam = m_oArchiveParams["path"].String();

            string usbPath;
            //In case no path is provided, take the first from the mounted paths
            if(pathParam.empty())
            {
                usbPath = *paths.begin();
            }
            else
            {
                //Loop through all the paths for exact match and create link
                for(auto const& it : paths)
                {
                    if(it.compare(pathParam) == 0)
                    {
                        usbPath = pathParam;
                        break;
                    }
                }
            }

            error = NoUSB;
            LOGINFO("Test log");
            if(!usbPath.empty())
            {
                LOGINFO("usbPath = %s", usbPath.c_str());
                string script = (ARCHIVE_LOGS_SCRIPT + " " + usbPath);
                FILE* fp = popen(script.c_str(), "r");
                if (NULL != fp) {
                    char buf[256];
                    while(fgets(buf, sizeof(buf), fp) != NULL)
                    {
                        usbFilePath = buf; //Capture file path returned by the script
                        //Remove /n character at the end
                        usbFilePath.erase(std::remove(usbFilePath.begin(), usbFilePath.end(), '\n'), usbFilePath.end());
                        LOGINFO("buf = %s", usbFilePath.c_str());
                    }
                }
                else {
                    LOGERR("Cannot run command\n");
                    error = ScriptError;
                }

                int rc = pclose(fp);
                // int rc = runScript(script.c_str());
                LOGINFO("'%s' exit code: %d", script.c_str(), rc);
                error = static_cast<ArchiveLogsError>(rc);
            }
        }

        onArchiveLogs(error, usbFilePath);
    }

    void UsbAccess::onArchiveLogs(ArchiveLogsError error, const string& filePath)
    {
        JsonObject params;
        auto it = ARCHIVE_LOGS_ERRORS.find(error);
        if (it == ARCHIVE_LOGS_ERRORS.end())
            it = ARCHIVE_LOGS_ERRORS.find(ScriptError);
        params["error"] = it->second;
        params["success"] = (error == None);
        params["path"] = filePath;
        sendNotify(EVT_ON_ARCHIVE_LOGS.c_str(), params);
    }

    // iarm
    void UsbAccess::InitializeIARM()
    {
        if (Utils::IARM::init())
        {
            IARM_Result_t res;
            IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_USB_MOUNT_CHANGED, eventHandler));
        }
    }

    void UsbAccess::DeinitializeIARM()
    {
        if (Utils::IARM::isConnected())
        {
            IARM_Result_t res;
            IARM_CHECK(IARM_Bus_UnRegisterEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_USB_MOUNT_CHANGED));
        }
    }

    void UsbAccess::eventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        if (UsbAccess::_instance)
            UsbAccess::_instance->iarmEventHandler(owner, eventId, data, len);
        else
            LOGWARN("cannot handle IARM events without a UsbAccess plugin instance!");
    }

    void UsbAccess::iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        if (strcmp(owner, IARM_BUS_SYSMGR_NAME) != 0)
        {
            LOGERR("unexpected event: owner %s, eventId: %d, data: %p, size: %zu.", owner, (int)eventId, data, len);
            return;
        }
        if (data == nullptr || len == 0)
        {
            LOGERR("event with NO DATA: eventId: %d, data: %p, size: %zu.", (int)eventId, data, len);
            return;
        }

        switch (eventId)
        {
            case IARM_BUS_SYSMGR_EVENT_USB_MOUNT_CHANGED:
            {
                IARM_Bus_SYSMgr_EventData_t *eventData = (IARM_Bus_SYSMgr_EventData_t*)data;
                onUSBMountChanged((eventData->data.usbMountData.mounted == 1), eventData->data.usbMountData.dir);
                break;
            }
            default:
                LOGWARN("unexpected event: owner %s, eventId: %d, data: %p, size: %zu.", owner, (int)eventId, data, len);
                break;
        }
    }

    void UsbAccess::onUSBMountChanged(bool mounted, const string& device)
    {
        JsonObject params;
        params["mounted"] = mounted;
        params["device"] = device;
        sendNotify(EVT_ON_USB_MOUNT_CHANGED.c_str(), params);
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
                    if (((dp->d_type == DT_DIR) && includeFolders) ||
                        ((dp->d_type != DT_DIR) && (fileRegex.empty() ||
                            std::regex_match(dp->d_name, std::regex(fileRegex, std::regex_constants::icase)) == true)))
                        files.push_back(
                                {
                                    dp->d_type == DT_DIR ? 'd' : 'f',
                                    dp->d_name
                                });
                }
                closedir(dirp);

                result = true;
            }
        }

        return result;
    }

    bool UsbAccess::getMounted(std::list <std::string>& paths)
    {
        bool result = false;

        std::list<std::string> devnodes;

        struct udev *udev = udev_new();

        struct udev_enumerate *enumerate = udev_enumerate_new(udev);
        udev_enumerate_add_match_subsystem(enumerate, "block");
        udev_enumerate_scan_devices(enumerate);

        struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
        struct udev_list_entry *entry;

        // If the list is empty, or on failure, NULL is returned.
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
            result = true;

            while (nullptr != (ent = getmntent(file)))
                if (std::find(devnodes.begin(), devnodes.end(), string(ent->mnt_fsname)) != devnodes.end())
                    mapping.emplace(string(ent->mnt_fsname), string(ent->mnt_dir));

            endmntent(file);
        }

        for (auto const& x : mapping)
            paths.emplace_back(x.second);

        return result;
    }
} // namespace Plugin
} // namespace WPEFramework
