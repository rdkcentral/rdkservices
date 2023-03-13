/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
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
*/

#ifndef _MIRACAST_SERVICE_H_
#define _MIRACAST_SERVICE_H_

#include <miracastErrors.h>
#include <string>
#include <vector>

using namespace std;
using namespace MIRACAST;

enum
{
	Stop_Miracast_Service=1,
	Start_WiFi_Display,
	Stop_WiFi_Display,
	Accept_ConnectDevice_Request,
	Reject_ConnectDevice_Request
};

enum DEVICEROLE
{
    SOURCE = 0,
    PRIMARY_SINK,
    SECONDARY_SINK,
    DUAL_ROLE
};

typedef struct d_info
{
    string deviceMAC;
    string deviceType;
    string modelName;
    bool isCPSupported;
    enum DEVICEROLE deviceRole; 
}DeviceInfo;

class MiracastCallback
{
public:
    MiracastCallback() {}
    virtual ~MiracastCallback() {}
    virtual void onMiracastDisabled() = 0;
    virtual void onDeviceDiscovery(DeviceInfo* device) = 0;
    virtual void onDeviceLost(std::string MAC) = 0;
    virtual void onGroupRemoved(std::string reason) = 0;
    virtual void onProvisionReq(std::string authType, std::string MAC) = 0;
    virtual void onGoNegReq(std::string MAC) = 0;
    virtual void onGroupFormationSuccess() = 0;
    virtual void onGroupStarted() = 0;
    virtual void onConnected() = 0; 
    virtual void onStreamingStarted() = 0; 
    virtual void onSessionEstablished() = 0;
    virtual void onWFDTimeout() = 0;
    virtual void onDeviceOff() = 0;
    virtual void HDCPStatus(bool status) = 0;
    virtual void stopService() = 0;
};

class MiracastPrivate;

class MiracastServiceImplementation
{
	public:
		static MiracastServiceImplementation *create(MiracastCallback* Callback);
		static void Destroy( MiracastServiceImplementation* object );
		void setEnable( std::string is_enabled );
		void acceptClientConnectionRequest( std::string is_accepted );
		
		bool enableMiracast(bool flag = false);

		void StopApplication( void );
		//Global APIs
		MiracastError discoverDevices(); 
		MiracastError selectDevice();
		MiracastError connectDevice(std::string MAC);
		MiracastError startStreaming();

		//APIs to request for device/connection related details
		std::string getConnectedMAC();
		std::vector<DeviceInfo*> getAllPeers();
		bool getConnectionStatus();
		DeviceInfo* getDeviceDetails(std::string MAC);

		//APIs to disconnect
		bool stopStreaming();
		bool disconnectDevice();

	private:

		MiracastServiceImplementation(MiracastCallback* Callback);
		MiracastServiceImplementation();
		MiracastServiceImplementation(MiracastServiceImplementation&);
		~MiracastServiceImplementation();
		MiracastPrivate* m_impl;
};

#endif
