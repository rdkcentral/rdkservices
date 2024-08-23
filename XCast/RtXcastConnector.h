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

#include "RtNotifier.h"
#include "XCastCommon.h"
#include <xdial/gdialservicecommon.h>
#include <xdial/gdialservice.h>
using namespace std;


/**
 * This is the connector class for interacting with xdial client using rtRemote.
 */
class RtXcastConnector : public GDialNotifier
{
protected:
    //RtXcastConnector():m_runEventThread(true){}
    RtXcastConnector(){}
public:
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
    void registerApplications (std::vector<DynamicAppConfig*>& appConfigList);
    string  getProtocolVersion(void);
    /**
     *Request the single instance of this class
     */
    static  RtXcastConnector * getInstance();

    virtual void onApplicationLaunchRequest(string appName, string parameter) override;
    virtual void onApplicationLaunchRequestWithLaunchParam (string appName,string strPayLoad, string strQuery, string strAddDataUrl) override;
    virtual void onApplicationStopRequest(string appName, string appID) override;
    virtual void onApplicationHideRequest(string appName, string appID) override;
    virtual void onApplicationResumeRequest(string appName, string appID) override;
    virtual void onApplicationStateRequest(string appName, string appID) override;

    /**
     *Call back function for rtConnection
     */
    //int connectToRemoteService();
    
    void setService(RtNotifier * service){
        m_observer = service;
    }
private:
    //Internal methods
    //RT Connector class
    RtNotifier * m_observer;
    //Event Monitoring thread
    //thread m_eventMtrThread;
    // Atomic lock
    //mutex m_threadlock;
    // Boolean event thread exit condition
    //bool m_runEventThread;
    // Member function to handle RT messages.
    //void processRtMessages();
    bool IsAppEnabled(char* strAppName);

    // Class level contracts
    // Singleton instance
    static RtXcastConnector * _instance;
    // Thread main function
    //static void threadRun(RtXcastConnector *rtCtx);
};
