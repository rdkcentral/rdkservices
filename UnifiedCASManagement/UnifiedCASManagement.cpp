
#include "UnifiedCASManagement.h"

namespace WPEFramework {

    namespace Plugin {

        UnifiedCASManagement* UnifiedCASManagement::_instance;

        UnifiedCASManagement::UnifiedCASManagement() {
            _instance = this;
            m_RTPlayer = std::make_shared<RTPlayer>(this);
	    m_sessionId = UNDEFINED_SESSION_ID;
            RegisterAll();
        }

        UnifiedCASManagement::~UnifiedCASManagement() {
            UnregisterAll();
            UnifiedCASManagement::_instance = nullptr;
        }
    } // namespace
} // namespace
