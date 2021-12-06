/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/
#include <string>
#include <thread>
#include <mutex>
#include <iostream>
#include <list>

#include <rtRemote.h>
#include <rtObject.h>
#include <rtError.h>
#include "RtNotifier.h"

#include "XCastSystemRemoteObject.h"

using namespace std;

typedef struct _RegAppLaunchParams {
    char *appName = NULL;
    char *query = NULL;
    char *payload = NULL;
}RegAppLaunchParams;

/**
 * This is the connector class for interacting with xdial client using rtRemote.
 */
class RtXcastConnector {
protected:
    RtXcastConnector():m_runEventThread(true){
        }
public:
    std::list<RegAppLaunchParams> m_appLaunchParamList;

    virtual ~RtXcastConnector();
    /**
     * Initialize rtRemote communication with rtDial server
     */
    bool initialize();
    
    /** Shutdown rtRemote connectivity */
    void shutdown();
    /**
     *The application state change function . This is invoked from application side.
     *   @param app - The application name
     *   @param state - The state of the application
     *   @param id - The application identifier
     *   @param error - The error string if the requested application is not available or due to other errors
     *   @return indicates whether state is properly communicated to rtdial server.
     */
    int applicationStateChanged( string app, string state, string id, string error);
    /**
     *This function will enable cast service by default.
     *@param friendlyname - friendlyname
     *@param enableService - Enable/Disable the SSDP discovery of Dial server
     */
    void enableCastService(string friendlyname,bool enableService = true);
    /**
     *This function will update friendly name.
     *@param friendlyname - friendlyname
     */
    void updateFriendlyName(string friendlyname);
    void registerApplications (string strApps);
    /**
     *Request the single instance of this class
     */
    static  RtXcastConnector * getInstance();
    /**
     *Call back function for rtConnection
     */
    int connectToRemoteService();
    bool IsDynamicAppListEnabled();
    bool getEntryFromAppLaunchParamList (const char* appName, RegAppLaunchParams* reqParam);
    
    void setService(RtNotifier * service){
        m_observer = service;
        m_xcast_system_remote_object->setService(service);
    }
private:
    //Internal methods
    //RT Connector class
    RtNotifier * m_observer;
    //Event Monitoring thread
    thread m_eventMtrThread;
    // Atomic lock
    mutex m_threadlock;
    // Boolean event thread exit condition
    bool m_runEventThread;

    XCastSystemRemoteObjectReferenceWrapper m_xcast_system_remote_object;

    // Member function to handle RT messages.
    void processRtMessages();
    void clearAppLaunchParamList ();
    bool IsAppEnabled(char* strAppName);

    // Class level contracts
    // Singleton instance
    static RtXcastConnector * _instance;
    // Thread main function
    static void threadRun(RtXcastConnector *rtCtx);

    static rtError onApplicationLaunchRequestCallback(int numArgs, const rtValue* args, rtValue* result, void* context);
    static rtError onApplicationHideRequestCallback(int numArgs, const rtValue* args, rtValue* result, void* context);
    static rtError onApplicationResumeRequestCallback(int numArgs, const rtValue* args, rtValue* result, void* context);
    static rtError onApplicationStateRequestCallback(int numArgs, const rtValue* args, rtValue* result, void* context);
    static rtError onApplicationStopRequestCallback(int numArgs, const rtValue* args, rtValue* result, void* context);
    static rtError onRtServiceByeCallback(int numArgs, const rtValue* args, rtValue* result, void* context);
    static void remoteDisconnectCallback(void * context);
};
