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
            //events
            static const string EVT_ON_USB_MOUNT_CHANGED;
            //other
            static const string USB_MOUNT_PATH;
            static const string LINK_PATH;
            static const string LINK_URL_HTTP;

        private/*registered methods (wrappers)*/:

            //methods ("parameters" here is "params" from the curl request)
            uint32_t getFileListWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t createLinkWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t clearLinkWrapper(const JsonObject& parameters, JsonObject& response);

        private/*internal methods*/:
            UsbAccess(const UsbAccess&) = delete;
            UsbAccess& operator=(const UsbAccess&) = delete;
        };
    } // namespace Plugin
} // namespace WPEFramework
