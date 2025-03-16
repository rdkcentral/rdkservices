/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
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
#include <fstream>
#include "Module.h"
#include "tptimer.h"
#include "XCastNotifier.h"
#include "XCastCommon.h"
#include <gdialservicecommon.h>
#include <gdialservice.h>
using namespace std;


/**
 * This is the Manager class for interacting with gdial library.
 */
class XCastManager : public GDialNotifier
{
protected:
    XCastManager(){}
public:
    virtual ~XCastManager();
    /**
     * Initialize gdialService to communication with gdial server
     */
    bool initialize(const std::string& gdial_interface_name, bool networkStandbyMode );
    void deinitialize();
    
    /** Shutdown gdialService connectivity */
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
    void setNetworkStandbyMode(bool nwStandbymode);

    int setManufacturerName( string manufacturer);
    string getManufacturerName(void);
    int setModelName( string model);
    string getModelName(void);

    /**
     *Request the single instance of this class
     */
    static  XCastManager * getInstance();

    virtual void onApplicationLaunchRequest(string appName, string parameter) override;
    virtual void onApplicationLaunchRequestWithLaunchParam (string appName,string strPayLoad, string strQuery, string strAddDataUrl) override;
    virtual void onApplicationStopRequest(string appName, string appID) override;
    virtual void onApplicationHideRequest(string appName, string appID) override;
    virtual void onApplicationResumeRequest(string appName, string appID) override;
    virtual void onApplicationStateRequest(string appName, string appID) override;
    virtual void onStopped(void) override;
    virtual void updatePowerState(string powerState) override;

    /**
     *Call back function for rtConnection
     */
    int isGDialStarted();
    
    void setService(XCastNotifier * service){
        m_observer = service;
    }
private:
    //Internal methods
    XCastNotifier * m_observer;
    bool IsAppEnabled(char* strAppName);
    void getWiFiInterface(std::string& WiFiInterfaceName);
    void getGDialInterfaceName(std::string& interfaceName);
    std::string getReceiverID(void);
    bool envGetValue(const char *key, std::string &value);

    // Class level contracts
    // Singleton instance
    static XCastManager * _instance;
    std::recursive_mutex m_mutexSync;
};
