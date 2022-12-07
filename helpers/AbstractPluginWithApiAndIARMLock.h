#pragma once

#include <mutex>
#include <libIBus.h>
#include <AbstractPluginWithApiLock.h>
#include <map>
#include <string>
#include "UtilsLogging.h"

namespace WPEFramework {

    namespace Plugin {

        static std::map<std::string, std::map<IARM_EventId_t, IARM_EventHandler_t>> _registered_iarm_handlers;

        static void _generic_iarm_handler(const char *owner, IARM_EventId_t eventId, void *data, size_t len) {
            std::lock_guard<std::mutex> lock(AbstractPluginWithApiLock::getApiLock());
            // we can't be in IARM handler thread & API request thread at the same time, so
            // it's safe to use AbstractPluginWithApiLock's isThreadUsingLockedApi here
            isThreadUsingLockedApi = true;
            try {
                if (_registered_iarm_handlers[owner].count(eventId)) {
                    LOGINFO("handling IARM handler under lock: %s/%d len:%d\n",owner, eventId, len);
                    _registered_iarm_handlers[owner][eventId](owner, eventId, data, len);
                } else {
                    LOGERR("missing handler for %s / %d", owner, eventId);
                }
            } catch (...) {
                isThreadUsingLockedApi = false;
                throw;
            }
            isThreadUsingLockedApi = false;
        }
        /*
            provides overloaded versions of IARM_Bus_RegisterEventHandler (and IARM_Bus_UnRegisterEventHandler)
            that take locks when executing
        */
        class AbstractPluginWithApiAndIARMLock : public AbstractPluginWithApiLock {
        public:

            // we are providing IARM_Bus_RegisterEventHandler as new static member (libiarm provides standalone function)
            static IARM_Result_t IARM_Bus_RegisterEventHandler(const char *ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                if (_registered_iarm_handlers[ownerName].count(eventId)) {
                    LOGERR("double registration for %s / %d ; previous handler will be overriden", ownerName, eventId);
                }
                _registered_iarm_handlers[ownerName][eventId] = handler;
                auto ret = ::IARM_Bus_RegisterEventHandler(ownerName, eventId, _generic_iarm_handler);
                if (IARM_RESULT_SUCCESS != ret) {
                    _registered_iarm_handlers[ownerName].erase(eventId);
                }
                return ret;
            }

            // we are providing IARM_Bus_UnRegisterEventHandler as new static member (libiarm provides standalone function)
            static IARM_Result_t IARM_Bus_UnRegisterEventHandler(const char *ownerName, IARM_EventId_t eventId) {
                _registered_iarm_handlers[ownerName].erase(eventId);
                return ::IARM_Bus_UnRegisterEventHandler(ownerName, eventId);
            }
        };
    } // Plugin
} // WPEFramework
