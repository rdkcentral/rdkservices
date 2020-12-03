#include "UsbAccess.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

const short WPEFramework::Plugin::UsbAccess::API_VERSION_NUMBER_MAJOR = 1;
const short WPEFramework::Plugin::UsbAccess::API_VERSION_NUMBER_MINOR = 0;
const string WPEFramework::Plugin::UsbAccess::SERVICE_NAME = "org.rdk.UsbAccess";
const string WPEFramework::Plugin::UsbAccess::METHOD_GET_FILE_LIST = "getFileList";
const string WPEFramework::Plugin::UsbAccess::METHOD_CREATE_LINK = "createLink";
const string WPEFramework::Plugin::UsbAccess::METHOD_CLEAR_LINK = "clearLink";
const string WPEFramework::Plugin::UsbAccess::USB_MOUNT_PATH = "/run/media/sda"; // platco
const string WPEFramework::Plugin::UsbAccess::LINK_URL_HTTP = "http://localhost:50050/usbdrive";
const string WPEFramework::Plugin::UsbAccess::LIGHTTPD_CONF_PATH = "/etc/lighttpd/lighttpd.conf";

using namespace std;

namespace WPEFramework {
    namespace Plugin {

        SERVICE_REGISTRATION(UsbAccess, UsbAccess::API_VERSION_NUMBER_MAJOR, UsbAccess::API_VERSION_NUMBER_MINOR);

        UsbAccess* UsbAccess::_instance = nullptr;

        UsbAccess::UsbAccess()
            : AbstractPlugin()
        {
            LOGINFO();
            UsbAccess::_instance = this;
            registerMethod(METHOD_GET_FILE_LIST, &UsbAccess::getFileListWrapper, this);
            registerMethod(METHOD_CREATE_LINK, &UsbAccess::createLinkWrapper, this);
            registerMethod(METHOD_CLEAR_LINK, &UsbAccess::clearLinkWrapper, this);
        }

        UsbAccess::~UsbAccess()
        {
            LOGINFO();
            UsbAccess::_instance = nullptr;
        }

        const string UsbAccess::Initialize(PluginHost::IShell* /* service */)
        {
            LOGINFO();
            return "";
        }

        void UsbAccess::Deinitialize(PluginHost::IShell* /* service */)
        {
            LOGINFO();
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

            string dir = USB_MOUNT_PATH;
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

                DIR* dirp = opendir(dir.c_str());
                if (dirp == nullptr)
                {
                    LOGERR("could not open");
                    response["error"] = "could not open";
                }
                else
                {
                    JsonArray contents;
                    struct dirent * dp;
                    while ((dp = readdir(dirp)) != nullptr)
                    {
                        const char* name = dp->d_name;
                        const char* t = dp->d_type == DT_DIR ? "d" : "f";

                        LOGINFO("%s : %s", name, t);

                        JsonObject ent;
                        ent["name"] = name;
                        ent["t"] = t;
                        contents.Add(ent);
                    }
                    closedir(dirp);
                    response["contents"] = contents;
                    success = true;
                }
            }

            returnResponse(success);
        }

        uint32_t UsbAccess::createLinkWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success = false;
            string linkPath = getLinkPath();
            int rc = symlink(USB_MOUNT_PATH.c_str(), linkPath.c_str());

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

            returnResponse(success);
        }

        uint32_t UsbAccess::clearLinkWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success = false;
            string linkPath = getLinkPath();
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

        string UsbAccess::getLinkPath() const
        {
            static string path;

            static bool initedOnce = false;
            if (!initedOnce)
            {
                FILE* f = fopen(LIGHTTPD_CONF_PATH.c_str(), "r");
                if (f != nullptr)
                {
                    std::vector<char> buf;
                    buf.resize(1024);

                    while (fgets(buf.data(), buf.size(), f))
                    {
                        if (strstr(buf.data(), "server.document-root") == buf.data())
                        {
                            std::string s(buf.data());

                            size_t begin = s.find_first_of('"');
                            size_t end = std::string::npos;
                            if (std::string::npos != begin)
                                end = s.find_first_of('"', begin + 2);

                            if (std::string::npos != begin && std::string::npos != end)
                            {
                                path = s.substr(begin + 1, end - begin - 1);
                                break;
                            }
                            else
                                LOGERR("Failed to parse line: ", s.c_str());
                        }
                    }
                    fclose(f);

                    if (path.empty())
                        LOGERR("Failed to parse conf %s", LIGHTTPD_CONF_PATH.c_str());
                    else
                    {
                        if (path[path.size() - 1] != '/')
                            path += "/";
                        path += "usbdrive";
                        LOGINFO("Link path is: %s", path.c_str());
                    }
                }
                else
                    LOGERR("Failed to open conf %s:%s", LIGHTTPD_CONF_PATH.c_str(), strerror(errno));

                initedOnce = true;
            }

            return path;
        }
    } // namespace Plugin
} // namespace WPEFramework
