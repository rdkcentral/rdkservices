#pragma once

#include <mutex>
#include <libIBus.h>
#include <UtilsSynchro.hpp>
#include <map>
#include <string>
#include "UtilsLogging.h"

using namespace WPEFramework;

namespace Utils {

    namespace Synchro {

        // owner -> map( eventId -> real handler)
        using HandlerMapType = std::map<std::string, std::map<IARM_EventId_t, IARM_EventHandler_t>>;

        // maps evnt types to handlers, one per specific class
        template<class UsingClass>
        struct IarmHandlers {
            static HandlerMapType _registered_iarm_handlers;
        };

        template<class UsingClass>
        HandlerMapType IarmHandlers<UsingClass>::_registered_iarm_handlers;

        // we need separate handler per class, so that when we call IARM_Bus_RemoveEventHandler, we will not
        // remove _generic_iarm_handler registered by other classes/in-process plugins
        template<class UsingClass>
        static void _generic_iarm_handler(const char *owner, IARM_EventId_t eventId, void *data, size_t len) {
            auto& handlers_map = IarmHandlers<UsingClass>::_registered_iarm_handlers;
            isThreadUsingLockedApi = true;
            std::lock_guard<std::recursive_mutex> lock(ApiLocks<UsingClass>::mtx);
            LOGINFO("calling handler %s/%d with lock: %p\n", owner, eventId, &ApiLocks<UsingClass>::mtx);
            try {
                handlers_map[owner][eventId](owner, eventId, data, len);
            } catch (...) {
                isThreadUsingLockedApi = false;
                throw;
            }
            isThreadUsingLockedApi = false;
        }

        template<class UsingClass>
        static IARM_Result_t RegisterLockedIarmEventHandler(const char *ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
            auto generic_handler = _generic_iarm_handler<UsingClass>;
            auto& handlers_map = IarmHandlers<UsingClass>::_registered_iarm_handlers;

            std::lock_guard<std::recursive_mutex> lock(ApiLocks<UsingClass>::mtx);
            handlers_map[ownerName][eventId] = handler;
            return ::IARM_Bus_RegisterEventHandler(ownerName, eventId, generic_handler);
        }

        template<class UsingClass>
        static IARM_Result_t RemoveLockedEventHandler(const char *ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
            auto& handlers_map = IarmHandlers<UsingClass>::_registered_iarm_handlers;

            std::lock_guard<std::recursive_mutex> lock(ApiLocks<UsingClass>::mtx);
            if (handler != handlers_map[ownerName][eventId]) {
                LOGERR("class %s RemoveLockedEventHandler for ownerName: %s, event: %d passed handler: %p different than registered: %p\n", typeid(UsingClass).name(), ownerName, eventId, handler, handlers_map[ownerName][eventId]); fflush(stdout);
            }
            // still erase the event in any case
            handlers_map[ownerName].erase(eventId);
            return ::IARM_Bus_RemoveEventHandler(ownerName, eventId, _generic_iarm_handler<UsingClass>);
        }
    } // Synchro
} // Utils
