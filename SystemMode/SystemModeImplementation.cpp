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
	std::cout<<"RamTesting SystemModeImplementation constru "<<std::endl;
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
    	    
    SystemModesCurrentStateMap[DEVICE_OPTIMIZE]  = Exchange::ISystemMode::State::VIDEO ;


    deviceOptimizeStateMap[VIDEO] ="VIDEO";
    deviceOptimizeStateMap[GAME] ="GAME";


    // Check if the file exists
    std::ifstream infile(SYSTEM_MODE_FILE);
    if (!infile.good()) {
	    // File doesn't exist, so create it
	    std::ofstream outfile(SYSTEM_MODE_FILE);
	    if (outfile) {
		    LOGINFO("File created successfully: %s\n", SYSTEM_MODE_FILE);
		    updateSystemModeFile("DEVICE_OPTIMIZE" ,"currentstate" , "VIDEO" , "add");
	    } else {
		    LOGERR("Error creating file: %s\n", SYSTEM_MODE_FILE);
	    }
    } else {
	    LOGINFO("File already exists: %s\n", SYSTEM_MODE_FILE);
	    for (int i =1 ;i <=SYSTEM_MODE_COUNT ; i++ )
	    {
		std::string value = "";   
		std::string systemMode = SystemModeMap[static_cast<Exchange::ISystemMode::SystemMode>(i)] ;
		getSystemModePropertyValue(systemMode, "callsign", value);	
		if (value != "")
		{
			updateSystemModeFile( systemMode, "callsign", "","deleteall") ;
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

   ASSERT (nullptr == SystemModeImpl);

   if (SystemModeImpl != nullptr)
   {
      SystemModeImpl_instance = SystemModeImpl;
   }

   return(SystemModeImpl_instance);
}

SystemModeImplementation::~SystemModeImplementation()
{ 
   std::cout<<"RamTesting SystemModeImplementation distru "<<std::endl;
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

}

/**
 * Register a notification callback
 */
/*
uint32_t SystemModeImplementation::Register(Exchange::ISystemMode::INotification *notification)
{
	std::cout<<"RamTesting SystemModeImplementation Register "<<std::endl;
    ASSERT (nullptr != notification);

    _adminLock.Lock();

    // Make sure we can't register the same notification callback multiple times
    if (std::find(_systemModeNotification.begin(), _systemModeNotification.end(), notification) == _systemModeNotification.end())
    {
        LOGINFO("Register notification");
        _systemModeNotification.push_back(notification);
        notification->AddRef();
    }

    _adminLock.Unlock();

    return Core::ERROR_NONE;
}
*/
/**
 * Unregister a notification callback
 */
/*
uint32_t SystemModeImplementation::Unregister(Exchange::ISystemMode::INotification *notification )
{
	std::cout<<"RamTesting SystemModeImplementation Unregister "<<std::endl;
    uint32_t status = Core::ERROR_GENERAL;

    ASSERT (nullptr != notification);

    _adminLock.Lock();

    // Make sure we can't unregister the same notification callback multiple times
    auto itr = std::find(_systemModeNotification.begin(), _systemModeNotification.end(), notification);
    if (itr != _systemModeNotification.end())
    {
        (*itr)->Release();
        LOGINFO("Unregister notification");
        _systemModeNotification.erase(itr);
        status = Core::ERROR_NONE;
    }
    else
    {
        LOGERR("notification not found");
    }

    _adminLock.Unlock();

    return status;
}
*/
Core::hresult SystemModeImplementation::RequestState(const SystemMode p_systemMode, const State p_state) 
//Core::hresult SystemModeImplementation::RequestState(const string psystemMode, const string pstate) 
{
	std::cout<<"RamTesting SystemModeImplementation::RequestState p_systemMode " << DEVICE_OPTIMIZE << " p_state : " << VIDEO <<std::endl;
	std::cout<<"RamTesting SystemModeImplementation::RequestState p_systemMode original" <<p_systemMode << " p_state : " << p_state<<std::endl;
	std::cout<<"RamTesting SystemModeImplementation::RequestState p_systemMode " << DEVICE_OPTIMIZE << " p_state : " << VIDEO <<std::endl;
	std::cout<<"RamTesting SystemModeImplementation::RequestState p_systemMode original" <<p_systemMode << " p_state : " << p_state<<std::endl;
	SystemMode systemMode = DEVICE_OPTIMIZE;
	State state 	   = GAME;

	auto SystemModeMapIterator = SystemModeMap.find(systemMode);
	Core::hresult result = Core::ERROR_NONE;

	if(SystemModeMapIterator != SystemModeMap.end())	
	{
		std::string systemMode_str = SystemModeMapIterator->second;
		switch (systemMode) {
			case DEVICE_OPTIMIZE:{

						     auto deviceOptimizeStateMapIterator = deviceOptimizeStateMap.find(state);
						     if (deviceOptimizeStateMapIterator != deviceOptimizeStateMap.end())
						     {
							     std::string new_state = deviceOptimizeStateMapIterator->second;
							     uint32_t temp = SystemModesCurrentStateMap[systemMode];
							     std::string old_state = deviceOptimizeStateMap[static_cast<WPEFramework::Exchange::ISystemMode::State>(temp)];
							     for (auto it = _clients.begin(); it != _clients.end(); ++it) {
								     if (it->second) {  // Check if the pointer is not null
									     std::cout<<"RamTesting SystemModeImplementation::RequestState it systemMode: " <<systemMode<<"state : "<<state <<std::endl;
									     it->second->Request(new_state);  // Call Request() on the object
									     SystemModesCurrentStateMap[systemMode]  = state;
									     updateSystemModeFile(systemMode_str,"currentstate",new_state,"add");
									     LOGINFO("SystemMode  state change from %s to new %s" ,old_state.c_str(),new_state.c_str());
									     stateRequested =true;
								     }
							     }
							     result = Core::ERROR_NONE;
						     }
						     else
						     {
							     LOGERR("Invalid state %d for systemMode %d" ,state,systemMode);
							     result = Core::ERROR_GENERAL;
						     }
						     break;
					     }
			default:
					     {
						     LOGERR("Invalid systemMode %d",systemMode);
						     result = Core::ERROR_GENERAL;
						     break;
					     }

		}
	}
	else
	{
		LOGERR("Invalid systemMode %d",systemMode);
		result = Core::ERROR_GENERAL;
	}
	return result;
}
Core::hresult SystemModeImplementation::GetState(const SystemMode psystemMode, State &state )const 
{
	const SystemMode systemMode = DEVICE_OPTIMIZE;
	std::cout<<"RamTesting SystemModeImplementation::getState psystemMode :"<< psystemMode <<std::endl;

	Core::hresult result = Core::ERROR_NONE;
	auto SystemModeMapIterator = SystemModeMap.find(systemMode);
	if(SystemModeMapIterator != SystemModeMap.end())	
	{
		std::string value = "";
		std::string systemMode_str = SystemModeMapIterator->second;		
		getSystemModePropertyValue(systemMode_str ,"currentstate" , value);
		switch (systemMode) {
			case DEVICE_OPTIMIZE:{

						     // Iterate over the map to find the corresponding enum value
						     for (const auto& pair : deviceOptimizeStateMap) {
							     if (pair.second == value) {
								     state = pair.first;
								     break;
							     }
						     }
						     result = Core::ERROR_NONE;
						     break;
					     }
			default:
					     {
						     LOGERR("Invalid systemMode %d",systemMode);
						     result = Core::ERROR_GENERAL;
						     break;
					     }
		}
	}
	else
	{
		LOGERR("Invalid systemMode %d",systemMode);
		return Core::ERROR_GENERAL;
	}
	return result;

}

uint32_t SystemModeImplementation::ClientActivated(const string& callsign , const string& __SystemMode)
{
	std::cout<<"RamTesting SystemModeImplementation::ClientActivated callsign :"<<callsign <<std::endl;
	SystemMode pSystemMode = DEVICE_OPTIMIZE;

	if (callsign != "")
	{
		_controller = nullptr;
		std::cout<<"RamTesting SystemModeImplementation::ClientActivated callsign :"<<callsign <<std::endl;
		_controller = _communicatorClient->Open<PluginHost::IShell>(_T(callsign), ~0, 3000);

		if (_controller)
		{
			switch (pSystemMode) {
				case DEVICE_OPTIMIZE:{

							     Exchange::IDeviceOptimizeStateActivator  *deviceOptimizeStateActivator (_controller->QueryInterface<Exchange::IDeviceOptimizeStateActivator>());

							     if (deviceOptimizeStateActivator != nullptr) {
								     _adminLock.Lock();

								     std::map<const string, Exchange::IDeviceOptimizeStateActivator*>::iterator index(_clients.find(callsign));

								     if (index == _clients.end()) {
									     _clients.insert({callsign,deviceOptimizeStateActivator});
									     updateSystemModeFile( SystemModeMap[pSystemMode], "callsign", callsign,"add") ;
									     TRACE(Trace::Information, (_T("%s plugin is add to deviceOptimizeStateActivator map"), callsign.c_str()));

									     //If For Ex The plugins P1,P2,P3 who implement IDeviceOptimizeStateActivator . P1 ,P2 only activated . P3 is not in activated state .If org.rdk.SystemMode.RequestState (DeviceOptimize,GAME) is called then SystemMode trigger  P1.Request() and P2.Request() . After 5 min if P3 come to activate state , then SystemMode need to trigger P3. Request()i
									     if(stateRequested) 
									     {
										     State state ;
										     if(GetState(pSystemMode, state ) == Core::ERROR_NONE)
										     {
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
	SystemMode pSystemMode = DEVICE_OPTIMIZE;
	std::cout<<"RamTesting SystemModeImplementation::ClientDeactivated"<<std::endl;
	_adminLock.Lock();
	switch (pSystemMode) {
		case DEVICE_OPTIMIZE:
			{
				std::map<const string, Exchange::IDeviceOptimizeStateActivator*>::iterator index(_clients.find(callsign));

				if (index != _clients.end()) { // Remove from the list, if it is already there
					Exchange::IDeviceOptimizeStateActivator *temp = index->second;	
					temp->Release();				
					_clients.erase(index);
					updateSystemModeFile( SystemModeMap[pSystemMode], "callsign", callsign,"delete") ;
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

void SystemModeImplementation::updateSystemModeFile(const std::string& systemMode, const std::string& property, const std::string& value, const std::string& action) {

    if (systemMode.empty() || property.empty()) {
        LOGINFO("Error: systemMode or property is empty. systemMode: %s property: %s", systemMode.c_str(), property.c_str());
        return;
    }

    if (action != "add" && action != "delete" && action != "deleteall") {
        LOGINFO("Error: Invalid action. Action must be 'add', 'delete', or 'deleteall'.");
        return;
    }

    std::ifstream infile(SYSTEM_MODE_FILE);
    std::string line;
    std::stringstream buffer;
    bool propertyFound = false;
    std::string searchKey = systemMode + "_" + property;

    // Read the file content and process it line by line
    if (infile.is_open()) {
        while (std::getline(infile, line)) {
            // If the line starts with the searchKey
            if (line.find(searchKey) == 0) {
                propertyFound = true;
                if (action == "deleteall" && value.empty()) {
                    // Skip adding this line to the buffer, effectively removing it
                    continue;
                } else if (property == "currentstate") {
                    if (action == "add") {
                        // Replace the value for currentstate
                        line = searchKey + "=" + value;
                    } else if (action == "delete") {
                        // To delete a currentstate, we might want to clear or remove the line
                        line.clear(); // This effectively removes the line
                    }
                } else if (property == "callsign") {
                    if (action == "add") {
                        // Append the value to the callsign, ensuring no duplicate entries
                        if (line.find(value) == std::string::npos) {
                            line += value + "|";
                        }
                    } else if (action == "delete") {
                        // Remove the value from the callsign
                        size_t pos = line.find(value);
                        if (pos != std::string::npos) {
                            line.erase(pos, value.length() + 1); // +1 to remove the trailing '|'
                        }
                    }
                }
            }
            if (!line.empty()) {
                buffer << line << std::endl;
            }
        }
        infile.close();
    }

    // If the property wasn't found and the action is "add", add it to the file
    if (!propertyFound && action == "add") {
        if (property == "currentstate") {
            buffer << searchKey + "=" + value << std::endl;
        } else if (property == "callsign") {
            buffer << searchKey + "=" + value + "|" << std::endl;
        }
    }

    // Write the modified content back to the file
    std::ofstream outfile(SYSTEM_MODE_FILE);
    if (outfile.is_open()) {
        outfile << buffer.str();
        outfile.close();
        LOGINFO("Updated file %s successfully.", SYSTEM_MODE_FILE);
    } else {
        LOGINFO("Failed to open file %s for writing.", SYSTEM_MODE_FILE);
    }
}



bool SystemModeImplementation::getSystemModePropertyValue(const std::string& systemMode, const std::string& property, std::string& value) const
{
    if (systemMode.empty() || property.empty() ) {
        LOGINFO("Error: systemMode or property is empty. systemMode: %s property: %s ",systemMode.c_str(),property.c_str());
        return false;
    }
 
    std::ifstream infile(SYSTEM_MODE_FILE);
    std::string line;
    std::string searchKey = systemMode + "_" + property;

    if (!infile.is_open()) {
        std::cerr << "Failed to open file: " << SYSTEM_MODE_FILE << std::endl;
        return false;
    }

    while (std::getline(infile, line)) {
        // Check if the line starts with the search key
        if (line.find(searchKey) == 0) {
            // Extract the value after the '=' character
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                value = line.substr(pos + 1);
                infile.close();
                return true;
            }
        }
    }

    infile.close();
    return false;
}

} // namespace Plugin
} // namespace WPEFramework
