#include "ConnectionManager.h"
#include "RTPlayerNotify.h"

namespace WPEFramework
{
    ConnectionManager::ConnectionManager(WPEFramework::RTPlayerNotify* _playerInterface):m_stateMutex(), m_connectionMutex(), m_running(false), m_playerInterface(_playerInterface)
    {
        setConnectionState(ConnectionState::STATE_REMOTE_INVALID);
        startConnectionMonitor();
    }
    ConnectionManager::~ConnectionManager()
    {
        LOGDBG("ConnectionManager start of Deinitialize");
        if(connectionMonitorThread->joinable())
        {
            connectionMonitorThread->join();
        }
        rtError rc = rtRemoteShutdown(m_rtRemoteEnv, true);
        ASSERT(rc == RT_OK);
        setConnectionState(ConnectionState::STATE_REMOTE_SERVER_SHUTDOWN);
        LOGDBG("ConnectionManager end of Deinitialize");
        return;
    }

    void ConnectionManager::eventProcessorMain()
    {
        while(!stopRunning())
        {
            rtRemoteProcessSingleItem();
            /* Need to adjust the sleep duration depending upon
             * onProgress reporting frequency
             */
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
        return;
    }

    void ConnectionManager::registerForEvents()
    {
        LOGINFO(" registering for events ");
        m_rtObject.send("on","onMediaOpened",new rtFunctionCallback(eventHandler, m_playerInterface));
        m_rtObject.send("on","onProgress",new rtFunctionCallback(eventHandler, m_playerInterface));
        m_rtObject.send("on","onPlaying",new rtFunctionCallback(eventHandler, m_playerInterface));
        m_rtObject.send("on","onPaused",new rtFunctionCallback(eventHandler, m_playerInterface));
        m_rtObject.send("on","onComplete",new rtFunctionCallback(eventHandler, m_playerInterface));
        m_rtObject.send("on","onStatus",new rtFunctionCallback(eventHandler, m_playerInterface));
        m_rtObject.send("on","onWarning",new rtFunctionCallback(eventHandler, m_playerInterface));
        m_rtObject.send("on","onError",new rtFunctionCallback(eventHandler, m_playerInterface));
        m_rtObject.send("on","onSpeedChange",new rtFunctionCallback(eventHandler, m_playerInterface));
        m_rtObject.send("on","onClosed",new rtFunctionCallback(eventHandler, m_playerInterface));
        m_rtObject.send("on","onPlayerInitialized",new rtFunctionCallback(eventHandler, m_playerInterface));
        m_rtObject.send("on","onBuffering",new rtFunctionCallback(eventHandler, m_playerInterface));
        m_rtObject.send("on","onAcquiringLicense",new rtFunctionCallback(eventHandler, m_playerInterface));
        m_rtObject.send("on","onDRMMetadata",new rtFunctionCallback(eventHandler, m_playerInterface));
        m_rtObject.send("on","onBufferWarning",new rtFunctionCallback(eventHandler, m_playerInterface));
        m_rtObject.send("on","onPlaybackSpeedsChanged",new rtFunctionCallback(eventHandler, m_playerInterface));
        m_rtObject.send("on","onAdditionalAuthRequired",new rtFunctionCallback(eventHandler, m_playerInterface));
        m_rtObject.send("on","onCASData",new rtFunctionCallback(eventHandler, m_playerInterface));
        setConnectionState(ConnectionState::STATE_REMOTE_TARGET_EVENT_REGISTERED);
        eventProcessorThread.reset(new std::thread(
            [this]()mutable
            {
                eventProcessorMain();
            }));
    }

    rtObjectRef& ConnectionManager::getPlayerObject() noexcept(false)
    {
        LOGDBG("ConnectionManager acquiring lock via unique lock");
        std::unique_lock<std::timed_mutex> ul(m_connectionMutex, std::defer_lock);
        if(ul.try_lock_for(std::chrono::seconds(1)))
        {

            LOGDBG("ConnectionManager acquired connection monitor lock");
            if(getConnectionState() != ConnectionState::STATE_REMOTE_TARGET_EVENT_REGISTERED)
            {
                throw std::domain_error("RTObject not initialized");
            }
            LOGDBG("ConnectionManager released connection monitor lock");
        }
        else
        {
            LOGERR("ConnectionManager unable to acquire connection monitor lock");
            throw std::domain_error("RTObject not initialized");
        }
        return m_rtObject;
    }

    void ConnectionManager::setConnectionState(ConnectionState state)
    {
        std::unique_lock<std::mutex> lk(m_stateMutex);
        m_connectionState = state;
        lk.unlock();
        m_connectionCondtionVar.notify_one();
    }

    void ConnectionManager::startConnectionMonitor()
    {
        setRunningState(true);
        connectionMonitorThread.reset( new std::thread([this]()mutable{connectionMonitorMain();}));
    }

    void ConnectionManager::ensureSuccess(classFnPtr stateHandlerFunction)
    {
        rtError rc = RT_OBJECT_NOT_INITIALIZED;
        while(rc != RT_OK)
        {
            auto stateFunc = std::mem_fn(stateHandlerFunction);
            rc = stateFunc(this);
            if(rc != RT_OK)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }
        return;
    }

    rtError ConnectionManager::processInvalidState()
    {
        rtError rc = RT_OBJECT_NOT_INITIALIZED;
        m_rtRemoteEnv = rtEnvironmentGetGlobal();
        rc = rtRemoteInit(m_rtRemoteEnv);
        return rc;
    }

    rtError ConnectionManager::processInitializedState()
    {
        rtError rc = RT_OBJECT_NOT_INITIALIZED;
        const char* objectName = getenv("PX_WAYLAND_CLIENT_REMOTE_OBJECT_NAME");
        if (!objectName) {
            // Use rdkmediaplayer as the name if the environment variable is not set
            objectName = "rdkmediaplayer";
        }
        SYSLOG(Logging::Notification, (_T("Locating object %s"), objectName));
        LOGDBG("ConnectionManager trying to connect to to rdkmediaplayer");
        rc = rtRemoteLocateObject(objectName, m_rtObject, 3000, &ConnectionManager::remoteObjectDisconnected, this);
        LOGDBG("ConnectionManager locate object rc:%d %d", rc, RT_OK);
        return rc;
    }

    rtError ConnectionManager::processConnectedState()
    {
        rtError rc = RT_OK;
        registerForEvents();
        return rc;
    }

    void ConnectionManager::restartMonitorThread()
    {
        if(connectionMonitorThread->joinable())
        {
            LOGDBG("ConnectionManager Waiting to join connectionMonitor Thread");
            connectionMonitorThread->join();
            LOGDBG("ConnectionManager joined connectionMonitor Thread");
        }
        if(eventProcessorThread->joinable())
        {
            LOGDBG("EventProcessor Waiting to join connectionMonitor Thread");
            eventProcessorThread->join();
            LOGDBG("EventProcessor joined connectionMonitor Thread");
        }

        LOGINFO("ConnectionManager starting new connectionMonitor Thread");
        startConnectionMonitor();
        LOGDBG("ConnectionManager started new connectionMonitor Thread");
        setConnectionState(ConnectionState::STATE_REMOTE_INVALID);
        LOGDBG("ConnectionManager End of restartMonitorThread");
    }

    void ConnectionManager::remoteObjectDisconnected(void* argp)
    {
        ConnectionManager* connectionManager = static_cast<ConnectionManager*>(argp);
        LOGINFO("ConnectionManager Remote object got disconnected");
        connectionManager->setConnectionState(ConnectionState::STATE_REMOTE_TARGET_DISCONNECTED);
        LOGDBG("connectionManager setting connection status as target disconnected");
        connectionManager->restartMonitorThread();
        LOGDBG("connectionManager end of disconnect callback");
        return;
    }


    void ConnectionManager::connectionMonitorMain()
    {
        std::unique_lock<std::timed_mutex> ul(m_connectionMutex);
        rtError rc = RT_OK;
        while(!stopRunning())
        {
            ConnectionState connState = ConnectionState::STATE_REMOTE_MAX;
            LOGDBG("About to wait on condition variable");
            bool perform_action = m_connectionCondtionVar.wait_for(ul, std::chrono::seconds(1), [this, &connState]() {
                connState = getConnectionState();
                LOGDBG("current connection state:%d", std::underlying_type<ConnectionState>::type (connState));
                return (connState != ConnectionState::STATE_REMOTE_MAX && connState != ConnectionState::STATE_REMOTE_TARGET_EVENT_REGISTERED);
            });
            if (perform_action)
            {
                LOGDBG("Current rdkmediaplayer connection state: %d", std::underlying_type<ConnectionState>::type (connState));
                switch(connState)
                {
                    case ConnectionState::STATE_REMOTE_INVALID:
                        LOGINFO("ConnectionManager Setting up rtEnvironment");
                        ensureSuccess(&ConnectionManager::processInvalidState);
                        setConnectionState(ConnectionState::STATE_REMOTE_SERVER_INITIALIZED);
                        break;
                    case ConnectionState::STATE_REMOTE_SERVER_INITIALIZED:
                        LOGINFO("ConnectionManager Connecting with rdkmediaplayer");
                        ensureSuccess(&ConnectionManager::processInitializedState);
                        setConnectionState(ConnectionState::STATE_REMOTE_TARGET_ACQUIRED);
                        break;
                    case ConnectionState::STATE_REMOTE_TARGET_ACQUIRED:
                        LOGINFO("ConnectionManager Registering Events");
                        ensureSuccess(&ConnectionManager::processConnectedState);
                        setConnectionState(ConnectionState::STATE_REMOTE_TARGET_EVENT_REGISTERED);
                        break;
                    case ConnectionState::STATE_REMOTE_TARGET_DISCONNECTED:
                        LOGINFO("ConnectionManager reset current connection");
                    case ConnectionState::STATE_REMOTE_SERVER_SHUTDOWN:
                        setRunningState(false);
                        break;
                }
            }
            else
            {
                LOGDBG("Current rdkmediaplayer connection state: %d ConditionVariable timedout!!", std::underlying_type<ConnectionState>::type (connState));
            }
        }
        LOGDBG("ConnectionManager End of ConnectionMonitorMain");
        return;
    }

    rtError ConnectionManager::eventHandler(int argc, rtValue const* argv, rtValue* result, void* argp)
    {
        RTPlayerNotify* playerInterface = static_cast<RTPlayerNotify*> (argp);
        playerInterface->onEvent(argc, argv, result);
        return RT_OK;
    }
}
