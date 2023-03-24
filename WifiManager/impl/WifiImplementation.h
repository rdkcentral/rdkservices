#include "UtilsIarm.h"
#include "libIBus.h"

namespace WPEFramework {
    namespace Plugin {
        namespace WifiImplementation {
            void init() {
                Utils::IARM::init();
            }

            void deinit() {}
        }
}}