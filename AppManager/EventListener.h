#ifndef APPMANAGER_EVENTLISTENER
#define APPMANAGER_EVENTLISTENER
#include "Module.h"
#include <interfaces/IAppManager.h>

class EventListener
{
    public:
        virtual void onAppStateChanged(std::string client, WPEFramework::Exchange::IAppManager::LifecycleState newState, WPEFramework::Exchange::IAppManager::LifecycleState oldState) = 0;
};
#endif
