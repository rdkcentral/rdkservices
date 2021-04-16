#pragma once

#include "Module.h"
#include "utils.h"
#include "AbstractPlugin.h"

namespace WPEFramework {
namespace Plugin {

    class UsbAccess :  public AbstractPlugin {
    public:
        UsbAccess();
        virtual ~UsbAccess();
        virtual const string Initialize(PluginHost::IShell* service) override;
        virtual void Deinitialize(PluginHost::IShell* service) override;
        virtual string Information() const override;

    public /*constants*/:
        static const short API_VERSION_NUMBER_MAJOR;
        static const short API_VERSION_NUMBER_MINOR;
        static const string SERVICE_NAME;
        //methods
        static const string METHOD_GET_FILE_LIST;
        static const string METHOD_CREATE_LINK;
        static const string METHOD_CLEAR_LINK;
        static const string METHOD_GET_AVAILABLE_FIRMWARE_FILES;
        static const string METHOD_GET_MOUNTED;
        static const string METHOD_UPDATE_FIRMWARE;
        //events
        //other
        static const string LINK_URL_HTTP;
        static const string LINK_PATH;

    private/*registered methods (wrappers)*/:

        //methods ("parameters" here is "params" from the curl request)
        uint32_t getFileListWrapper(const JsonObject& parameters, JsonObject& response);
        uint32_t createLinkWrapper(const JsonObject& parameters, JsonObject& response);
        uint32_t clearLinkWrapper(const JsonObject& parameters, JsonObject& response);
        uint32_t getAvailableFirmwareFilesWrapper(const JsonObject& parameters, JsonObject& response);
        uint32_t getMountedWrapper(const JsonObject& parameters, JsonObject& response);
        uint32_t updateFirmware(const JsonObject& parameters, JsonObject& response);

    private/*internal methods*/:
        UsbAccess(const UsbAccess&) = delete;
        UsbAccess& operator=(const UsbAccess&) = delete;

        typedef string FileType;
        typedef std::pair<string,FileType> PathInfo;
        typedef std::list<PathInfo> FileList;

        static bool getFileList(const string& path, FileList& files, const string& fileRegex, bool includeFolders);
        static bool getMounted(std::list<string>& paths);
    };

} // namespace Plugin
}// namespace WPEFramework
