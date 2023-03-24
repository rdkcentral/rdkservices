#include "dbus/DBusClient.h"

namespace WPEFramework {
    namespace Plugin {
        namespace WifiImplementation {
            void init() {
                WifiManagerImpl::DBusClient::getInstance().run();
            }
            void deinit() {
                WifiManagerImpl::DBusClient::getInstance().stop();
            }
        }
}}