#ifndef __CONNECTION_MANAGER__
#define __CONNECTION_MANAGER__

#include <string>
#include <list>
#include <rtRemote.h>
#include <mutex>
#include <condition_variable>
#include "Module.h"
#include "utils.h"
#include "RTPlayerNotify.h"

namespace WPEFramework
{
    class RTPlayer;
    class ConnectionManager
    {
        public:
            typedef rtError (ConnectionManager::*classFnPtr)();
            enum class ConnectionState: uint32_t
            {
                STATE_REMOTE_INVALID = 0,
                STATE_REMOTE_SERVER_INITIALIZED = 1,
                STATE_REMOTE_TARGET_ACQUIRED = 2,
                STATE_REMOTE_TARGET_EVENT_REGISTERED = 3,
                STATE_REMOTE_TARGET_DISCONNECTED = 4,
                STATE_REMOTE_SERVER_SHUTDOWN = 5,
                STATE_REMOTE_MAX
            };
            ConnectionManager() = delete;
            explicit ConnectionManager(WPEFramework::RTPlayerNotify* playerInterface);
            ~ConnectionManager();
            void restartMonitorThread();
            ConnectionState getConnectionState() const
            {
                std::lock_guard<std::mutex> lk(m_stateMutex);
                return m_connectionState;
            }
            void setConnectionState(ConnectionState state);

            void startConnectionMonitor();
            void connectionMonitorMain();
            rtObjectRef& getPlayerObject() noexcept(false);
            static rtError eventHandler(int argc, rtValue const* argv, rtValue* result, void* argp);
            bool stopRunning() const
            {
                std::lock_guard<std::mutex> lk(m_runningMutex);
                return !m_running;
            }

            void setRunningState(const bool& running)
            {
                std::lock_guard<std::mutex> lk(m_runningMutex);
                m_running = running;
                return;
            }

        private:
            rtRemoteEnvironment* m_rtRemoteEnv;
            rtObjectRef m_rtObject;
            /*
            * These mutxes are made mutable because they are used in
            * const getter functions. But those const function have
            * to change the lock state of these mutexes hence making
            * them mutable.
            */

            /*
            * m_stateMutex is protecting setter and getter for connectionState
            * m_connectionMutex is protecting the polling and session validation.
            *
            */
            mutable std::mutex m_stateMutex;
            mutable std::timed_mutex m_connectionMutex;
            std::condition_variable_any m_connectionCondtionVar;
            mutable std::mutex m_runningMutex;

            /*
            * We are having this as a pointer because we dont want
            * the thread to start immediately after the constructor.
            * Having it as a pointer gives as the option to start
            * the thread after the object is completely constructed.
            */
            std::unique_ptr<std::thread> connectionMonitorThread;
            std::unique_ptr<std::thread> eventProcessorThread;
            ConnectionState m_connectionState;
            bool m_running;
            WPEFramework::RTPlayerNotify* m_playerInterface;
        private:
            void registerForEvents();
            static void remoteObjectDisconnected(void* argp);
            void ensureSuccess(classFnPtr stateHandlerFunction);
            rtError processInvalidState();
            rtError processInitializedState();
            rtError processConnectedState();
            void eventProcessorMain();
    };
}
#endif
