#pragma once

#include "Module.h"

#include "libIARM.h"

#include <thread>

namespace WPEFramework {
namespace Plugin {

    class UsbAccess :  public PluginHost::IPlugin, public PluginHost::JSONRPC {
    public:
        UsbAccess();
        virtual ~UsbAccess();
        virtual const string Initialize(PluginHost::IShell* service) override;
        virtual void Deinitialize(PluginHost::IShell* service) override;
        virtual string Information() const override;

        BEGIN_INTERFACE_MAP(UsbAccess)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        END_INTERFACE_MAP

    public/*members*/:
        static UsbAccess* _instance;

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
        static const string METHOD_ARCHIVE_LOGS;
        //events
        static const string EVT_ON_USB_MOUNT_CHANGED;
        static const string EVT_ON_ARCHIVE_LOGS;
        //other
        static const string LINK_URL_HTTP;
        static const string LINK_PATH;
        static const string REGEX_BIN;
        static const string REGEX_FILE;
        static const string PATH_DEVICE_PROPERTIES;
        static const std::list<string> ADDITIONAL_FW_PATHS;
        static const string ARCHIVE_LOGS_SCRIPT;
        enum ArchiveLogsError
        {
            ScriptError = -1,
            None,
            Locked,
            NoUSB,
            WritingError,
        };
        typedef std::map<ArchiveLogsError, string> ArchiveLogsErrorMap;
        static const ArchiveLogsErrorMap ARCHIVE_LOGS_ERRORS;

    private/*registered methods (wrappers)*/:

        //methods ("parameters" here is "params" from the curl request)
        uint32_t getFileListWrapper(const JsonObject& parameters, JsonObject& response);
        uint32_t createLinkWrapper(const JsonObject& parameters, JsonObject& response);
        uint32_t clearLinkWrapper(const JsonObject& parameters, JsonObject& response);
        uint32_t getAvailableFirmwareFilesWrapper(const JsonObject& parameters, JsonObject& response);
        uint32_t getMountedWrapper(const JsonObject& parameters, JsonObject& response);
        uint32_t updateFirmware(const JsonObject& parameters, JsonObject& response);
        uint32_t archiveLogs(const JsonObject& parameters, JsonObject& response);

    private/*iarm*/:
        void InitializeIARM();
        void DeinitializeIARM();
        static void eventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
        void iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
        void onUSBMountChanged(bool mounted, const string& device);

    private/*internal methods*/:
        UsbAccess(const UsbAccess&) = delete;
        UsbAccess& operator=(const UsbAccess&) = delete;

        struct FileEnt
        {
            char fileType; // 'f' or 'd'
            string filename;
        };
        typedef std::list<FileEnt> FileList;

        static bool getFileList(const string& path, FileList& files, const string& fileRegex, bool includeFolders);
        static bool getMounted(std::list<string>& paths);

        void archiveLogsInternal();
        void onArchiveLogs(ArchiveLogsError error);
        std::thread archiveLogsThread;
    };

} // namespace Plugin
}// namespace WPEFramework
