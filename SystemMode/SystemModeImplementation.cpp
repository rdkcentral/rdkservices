/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
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
*/

#include "SystemModeImplementation.h"
#include <sys/prctl.h>
#include "UtilsJsonRpc.h"
#include <mutex>
#include "tracing/Logging.h"
#include <fstream>
#include <sstream>


#define SYSTEMMODE_NAMESPACE "SystemMode"

namespace WPEFramework {
namespace Plugin {

SERVICE_REGISTRATION(SystemModeImplementation, 1, 0);

SystemModeImplementation::SystemModeImplementation()
: _adminLock()
, _engine(Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create())
, _communicatorClient(Core::ProxyType<RPC::CommunicatorClient>::Create(Core::NodeId("/tmp/communicator"), Core::ProxyType<Core::IIPCServer>(_engine)))
, _controller(nullptr)
,stateRequested(false)	
{
    LOGINFO("Create SystemModeImplementation Instance");

    SystemModeImplementation::instance(this);

     if (!_communicatorClient.IsValid())
     {
         LOGWARN("Invalid _communicatorClient\n");
    }
    else
    {

#if ((THUNDER_VERSION == 2) || ((THUNDER_VERSION == 4) && (THUNDER_VERSION_MINOR == 2)))
        _engine->Announcements(_communicatorClient->Announcement());
#endif

    }

    //set default value for each  SystemMode

    SystemModeMap[DEVICE_OPTIMIZE] = "DEVICE_OPTIMIZE";

    SystemModeInterfaceMap["DEVICE_OPTIMIZE"]= DEVICE_OPTIMIZE;

    deviceOptimizeStateMap[VIDEO] ="VIDEO";
    deviceOptimizeStateMap[GAME] ="GAME";


    // Check if the file exists
    std::ifstream infile(SYSTEM_MODE_FILE);
    if (!infile.good()) {
	    // File doesn't exist, so create it
	    std::ofstream outfile(SYSTEM_MODE_FILE);
	    if (outfile) {
		    LOGINFO("File created successfully: %s\n", SYSTEM_MODE_FILE);

		    //set default value for each  SystemMode
		    Utils::String::updateSystemModeFile("DEVICE_OPTIMIZE" ,"currentstate" , "VIDEO" , "add");
	    } else {
		    LOGERR("Error creating file: %s\n", SYSTEM_MODE_FILE);
	    }
    } else {
	    LOGINFO("File already exists: %s\n", SYSTEM_MODE_FILE);
	   
	    std::string currentstate ="";
	    Utils::String::getSystemModePropertyValue("DEVICE_OPTIMIZE" ,"currentstate",currentstate); 
	    if(currentstate == "")
	    {
		    //File already exists but currentstate not set then set default value
		    Utils::String::updateSystemModeFile("DEVICE_OPTIMIZE" ,"currentstate" , "VIDEO" , "add");
	    }

	    for (int i =1 ;i <=SYSTEM_MODE_COUNT ; i++ )
	    {
		std::string value = "";   
		std::string systemMode = SystemModeMap[static_cast<Exchange::ISystemMode::SystemMode>(i)] ;
		Utils::String::getSystemModePropertyValue(systemMode, "callsign", value);	
		if (value != "")
		{
			Utils::String::updateSystemModeFile( systemMode, "callsign", "","deleteall") ;
			std::vector<std::string> callSignList;
			Utils::String::split(callSignList, value , "|") ;
			for (const auto& token : callSignList) {

				ClientActivated(token , systemMode);
			}

		}
	    }
    }

}

SystemModeImplementation* SystemModeImplementation::instance(SystemModeImplementation *SystemModeImpl)
{
   static SystemModeImplementation *SystemModeImpl_instance = nullptr;

   ASSERT (nullptr != SystemModeImpl);

   if (SystemModeImpl != nullptr)
   {
      SystemModeImpl_instance = SystemModeImpl;
   }

   return(SystemModeImpl_instance);
}

SystemModeImplementation::~SystemModeImplementation()
{ 
    if (_controller)
    {
        _controller->Release();
	 _controller = nullptr;
    }

    LOGINFO("Disconnect from the COM-RPC socket\n");
    // Disconnect from the COM-RPC socket
    _communicatorClient->Close(RPC::CommunicationTimeOut);
    if (_communicatorClient.IsValid())
    {
        _communicatorClient.Release();
    }

    if(_engine.IsValid())
    {
        _engine.Release();
    }

    for (auto& entry : _clients) {
	    if (entry.second) {  // Check if the pointer is not null
		    entry.second->Release();
	    }
    }

}


Core::hresult SystemModeImplementation::RequestState(const SystemMode pSystemMode, const State pState ) 
{
	auto SystemModeMapIterator = SystemModeMap.find(pSystemMode);
	Core::hresult result = Core::ERROR_NONE;

	if(SystemModeMapIterator != SystemModeMap.end())	
	{
		std::string systemMode_str = SystemModeMapIterator->second;
		switch (pSystemMode) {
			case DEVICE_OPTIMIZE:{

						     auto deviceOptimizeStateMapIterator = deviceOptimizeStateMap.find(pState);

						     if (deviceOptimizeStateMapIterator != deviceOptimizeStateMap.end())
						     {

							     std::string new_state = deviceOptimizeStateMapIterator->second;
							     std::string old_state = "";
							     Utils::String::getSystemModePropertyValue(systemMode_str ,"currentstate" , old_state);
							     for (auto it = _clients.begin(); it != _clients.end(); ++it) {
								     if (it->second) {  // Check if the pointer is not null
									     it->second->Request(new_state);  // Call Request() on the object
								     }
							     }
							     Utils::String::updateSystemModeFile(systemMode_str,"currentstate",new_state,"add");
							     LOGINFO("SystemMode  state change from %s to new %s" ,old_state.c_str(),new_state.c_str());
							     stateRequested =true;
							     result = Core::ERROR_NONE;
						     }
						     else
						     {
							     LOGERR("Invalid state %d for systemMode %s" ,pState,systemMode_str.c_str());
							     result = Core::ERROR_GENERAL;
						     }
						     break;
					     }
			default:
					     {
						     LOGERR("Invalid systemMode %s",systemMode_str.c_str());
						     result = Core::ERROR_GENERAL;
						     break;
					     }

		}
	}
	else
	{
		LOGERR("Invalid systemMode %d",pSystemMode);
		result = Core::ERROR_GENERAL;
	}
	return result;
}

Core::hresult SystemModeImplementation::GetState(const SystemMode pSystemMode, GetStateResult& successResult)const 
{

	Core::hresult result = Core::ERROR_NONE;
	auto SystemModeMapIterator = SystemModeMap.find(pSystemMode);
	if(SystemModeMapIterator != SystemModeMap.end())	
	{
		std::string value = "";
		std::string systemMode_str = SystemModeMapIterator->second;		
		Utils::String::getSystemModePropertyValue(systemMode_str ,"currentstate" , value);
		switch (pSystemMode) {
			case DEVICE_OPTIMIZE:{

						     // Iterate over the map to find the corresponding enum value
						     for (const auto& pair : deviceOptimizeStateMap) {
							     if (pair.second == value) {
								     successResult.state = pair.first;
								     break;
							     }
						     }
						     result = Core::ERROR_NONE;
						     break;
					     }
			default:
					     {
						     LOGERR("Invalid systemMode %d",static_cast<uint32_t>(pSystemMode));
						     result = Core::ERROR_GENERAL;
						     break;
					     }
		}
	}
	else
	{
		LOGERR("Invalid systemMode %d",static_cast<uint32_t>(pSystemMode));
		return Core::ERROR_GENERAL;
	}
	return result;

}

uint32_t SystemModeImplementation::ClientActivated(const string& callsign , const string& systemMode)
{
	SystemMode pSystemMode ;
	auto it = SystemModeInterfaceMap.find(systemMode);

	if (it != SystemModeInterfaceMap.end()) {
		pSystemMode = it->second;
	} else {
		LOGERR("Invalid systemMode %s",systemMode.c_str());
		return 0;
	}

	if (callsign != "")
	{
		
		if (_controller)
                {
                        _controller->Release();
                        _controller = nullptr;
                }
		
		_controller = _communicatorClient->Open<PluginHost::IShell>(_T(callsign), ~0, 3000);

		if (_controller)
		{
			switch (pSystemMode) {
				case DEVICE_OPTIMIZE:{

							     Exchange::IDeviceOptimizeStateActivator  *deviceOptimizeStateActivator (_controller->QueryInterface<Exchange::IDeviceOptimizeStateActivator>());

							     if (deviceOptimizeStateActivator != nullptr) {
								     _adminLock.Lock();

								     std::map<const string, Exchange::IDeviceOptimizeStateActivator*>::iterator index(_clients.find(callsign));

								     {

									     //Insert _clients detail directly to map .even some junk value is there for callsign it will be replaced by new entry
									     _clients.insert({callsign,deviceOptimizeStateActivator});
									     Utils::String::updateSystemModeFile( SystemModeMap[pSystemMode], "callsign", callsign,"add") ;
									     TRACE(Trace::Information, (_T("%s plugin is add to deviceOptimizeStateActivator map"), callsign.c_str()));

									     //If For Ex The plugins P1,P2,P3 who implement IDeviceOptimizeStateActivator . P1 ,P2 only activated . P3 is not in activated state .If org.rdk.SystemMode.RequestState (DeviceOptimize,GAME) is called then SystemMode trigger  P1.Request() and P2.Request() . After 5 min if P3 come to activate state , then SystemMode need to trigger P3. Request()i
									     if(stateRequested) 
									     {
										     State state ;
										     GetStateResult successResult;
										     if(GetState(pSystemMode, successResult ) == Core::ERROR_NONE)
										     {
											     state = successResult.state;
											     std::string state_str = deviceOptimizeStateMap[state];
											     deviceOptimizeStateActivator->Request(state_str) ;
										     }
									     }
								     }

								     deviceOptimizeStateActivator->AddRef();

								     _adminLock.Unlock();

							     }
							     break;
						     }
				default:
						     {
							     LOGERR("Invalid systemMode %d",pSystemMode);
							     break;
						     }					     
			}

		}
	}
	return 0;	
}
uint32_t SystemModeImplementation::ClientDeactivated(const string& callsign ,const string& systemMode)
{
	SystemMode pSystemMode ;

	auto it = SystemModeInterfaceMap.find(systemMode);

	if (it != SystemModeInterfaceMap.end()) {
		pSystemMode = it->second;
	} else {
		LOGERR("Invalid systemMode %s",systemMode.c_str());
		return 0;
	}

	_adminLock.Lock();
	switch (pSystemMode) {
		case DEVICE_OPTIMIZE:
			{
				std::map<const string, Exchange::IDeviceOptimizeStateActivator*>::iterator index(_clients.find(callsign));

				if (index != _clients.end()) { // Remove from the list, if it is already there
					Exchange::IDeviceOptimizeStateActivator *temp = index->second;	
					temp->Release();				
					_clients.erase(index);
					Utils::String::updateSystemModeFile( SystemModeMap[pSystemMode], "callsign", callsign,"delete") ;
					TRACE(Trace::Information, (_T("%s plugin is removed from power control list"), callsign.c_str()));
				}
				break;
			}
		default:{
				LOGERR("Invalid systemMode %d",pSystemMode);
				break;
			}
	}
	_adminLock.Unlock();
	return 0;
}

} // namespace Plugin
} // namespace WPEFramework
