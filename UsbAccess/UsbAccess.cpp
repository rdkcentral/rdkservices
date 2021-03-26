#include "UsbAccess.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <mntent.h>
#include <regex>

const short WPEFramework::Plugin::UsbAccess::API_VERSION_NUMBER_MAJOR = 1;
const short WPEFramework::Plugin::UsbAccess::API_VERSION_NUMBER_MINOR = 0;
const string WPEFramework::Plugin::UsbAccess::SERVICE_NAME = "org.rdk.UsbAccess";
const string WPEFramework::Plugin::UsbAccess::METHOD_GET_FILE_LIST = "getFileList";
const string WPEFramework::Plugin::UsbAccess::METHOD_CREATE_LINK = "createLink";
const string WPEFramework::Plugin::UsbAccess::METHOD_CLEAR_LINK = "clearLink";
const string WPEFramework::Plugin::UsbAccess::LINK_URL_HTTP = "http://localhost:50050/usbdrive";
const string WPEFramework::Plugin::UsbAccess::LINK_PATH = "/tmp/usbdrive";


using namespace std;

namespace WPEFramework {
    namespace Plugin {

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
            UsbAccess::_instance = nullptr;
        }

        const string UsbAccess::Initialize(PluginHost::IShell* /* service */)
        {
            return "";
        }

        void UsbAccess::Deinitialize(PluginHost::IShell* /* service */)
        {
        }

        string UsbAccess::Information() const
        {
            return(string("{\"service\": \"") + SERVICE_NAME + string("\"}"));
        }

        // Registered methods (wrappers) begin
        uint32_t UsbAccess::getFileListWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success = false;

            string dir;
            if (!getMountPath(dir))
            {
                LOGERR("mount path not found");
                response["error"] = "not found";
            }
            else
            {
                if (parameters.HasLabel("path"))
                {
                    string path = parameters["path"].String();
                    if (!path.empty())
                    {
                        if (path[0] != '/')
                            dir += "/";
                        dir += path;
                    }
                }

                struct stat statbuf;
                if (stat(dir.c_str(), &statbuf) != 0)
                {
                    LOGERR("path '%s' not found", dir.c_str());
                    response["error"] = "not found";
                }
                else if (!S_ISDIR(statbuf.st_mode))
                {
                    LOGERR("path '%s' isn't dir", dir.c_str());
                    response["error"] = "isn't dir";
                }
                else
                {
                    LOGINFO("path '%s' found and is dir", dir.c_str());

                    FileList files;
                    if (!getFileList(dir, files))
                    {
                        LOGERR("could not open");
                        response["error"] = "could not open";
                    }
                    else
                    {
                        JsonArray contents;
                        for (auto it = files.begin(); it != files.end(); it++)
                        {
                            LOGINFO("%s : %s", it->first.c_str(), it->second.c_str());

                            JsonObject ent;
                            ent["name"] = it->first.c_str();
                            ent["t"] = it->second.c_str();
                            contents.Add(ent);
                        }

                        response["contents"] = contents;
                        success = true;
                    }
                }
            }

            returnResponse(success);
        }

        uint32_t UsbAccess::createLinkWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success = false;

            string linkPath = LINK_PATH;
            struct stat statbuf;
            int rc = stat(linkPath.c_str(), &statbuf);
            if (rc == 0)
            {
                LOGERR("file exists");
                response["error"] = "file exists";
            }
            else
            {
                string dir;
                if (!getMountPath(dir))
                {
                    LOGERR("mount path not found");
                    response["error"] = "not found";
                }
                else
                {
                    rc = symlink(dir.c_str(), linkPath.c_str());
                    if (0 == rc)
                    {
                        LOGINFO("symlink %s created", linkPath.c_str());
                        response["baseURL"] = LINK_URL_HTTP;
                        success = true;
                    }
                    else
                    {
                        LOGERR("error %d", rc);
                        response["error"] = "could not create symlink";
                    }
                }
            }

            returnResponse(success);
        }

        uint32_t UsbAccess::clearLinkWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success = false;

            string linkPath = LINK_PATH;
            int rc = remove(linkPath.c_str());
            if (0 == rc)
            {
                LOGINFO("symlink %s removed", linkPath.c_str());
                success = true;
            }
            else
            {
                LOGERR("error %d", rc);
                response["error"] = "could not remove symlink";
            }

            returnResponse(success);
        }

        bool UsbAccess::getFileList(const string& dir, FileList& files) const
        {
            bool success = false;

            files.clear();

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
                                "([\\w-]*)\\.(png|jpg|jpeg|tiff|tif|bmp|mp4|mov|avi|mp3|wav|m4a|flac|mp4|aac|wma|txt)",
                                std::regex_constants::icase)) == true)
                            files.emplace_back(dp->d_name, "f");
                        else
                            LOGWARN("unsupported file name: '%s'", dp->d_name);
                    }
                }
                closedir(dirp);

                success = true;
            }

            return success;
        }

        bool UsbAccess::getMountPath(string& dir) const
        {
            // get the mount path of the first available partition of the first available USB device

            bool success = false;

            dir.clear();
            string path = "/sys/block/";

            // 1. get the first available USB device
            list<string> usbDevices;

            DIR* dirp = opendir(path.c_str());
            if (dirp != nullptr)
            {
                struct dirent* dp;
                while ((dp = readdir(dirp)) != nullptr)
                {
                    if (string(dp->d_name).rfind("sd", 0) == 0)
                        usbDevices.emplace_back(dp->d_name);
                }
                closedir(dirp);
            }

            usbDevices.sort();
            string usbDevice;

            for (auto it = usbDevices.begin(); it != usbDevices.end(); it++)
            {
                string rPath = path + *it + "/removable";
                FILE* aFile = fopen(rPath.c_str(), "r");
                if (aFile != nullptr)
                {
                    char isRemovable;
                    if ((fread(&isRemovable, 1, 1, aFile) == 1) && (isRemovable == '1'))
                    {
                        usbDevice = *it;
                        break;
                    }
                    fclose(aFile);
                }
            }

            // 2. get the first available partition
            list<string> partitions;

            if (!usbDevice.empty())
            {
                LOGINFO("usb device: %s", usbDevice.c_str());

                path += usbDevice;
                dirp = opendir(path.c_str());
                if (dirp != nullptr)
                {
                    struct dirent* dp;
                    while ((dp = readdir(dirp)) != nullptr)
                    {
                        if (dp->d_type == DT_DIR && string(dp->d_name).rfind(usbDevice.c_str(), 0) == 0)
                            partitions.emplace_back(dp->d_name);
                    }
                    closedir(dirp);
                }
            }

            partitions.sort();
            string partition = partitions.empty() ? usbDevice : partitions.front();

            // 3. get the mount path
            if (!partition.empty())
            {
                LOGINFO("usb device partition: %s", partition.c_str());

                path = "/dev/" + partition;

                struct mntent* ent;
                FILE* aFile = setmntent("/proc/mounts", "r");
                if (aFile != nullptr)
                {
                    while (nullptr != (ent = getmntent(aFile)))
                    {
                        if (string(ent->mnt_fsname) == path)
                        {
                            dir = ent->mnt_dir;
                            break;
                        }
                    }
                    endmntent(aFile);
                }
            }

            // 4. ensure folder exists
            if (!dir.empty())
            {
                LOGINFO("usb device partition mount: %s", dir.c_str());

                struct stat statbuf;
                if (stat(dir.c_str(), &statbuf) == 0)
                {
                    success = true;
                }
            }

            return success;
        }
    } // namespace Plugin
} // namespace WPEFramework
