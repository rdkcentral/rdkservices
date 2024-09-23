/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
 */

#include "SystemMode.h"
#include "UtilsJsonRpc.h"
#include "UtilsString.h"

namespace WPEFramework {
namespace Plugin {

    void SystemMode::RegisterAll()
    {
        Register("RequestState", &SystemMode::RequestState, this);
        Register("GetState", &SystemMode::GetState, this);
    }
   
    uint32_t SystemMode::RequestState(const JsonObject& parameters, JsonObject& response)
    {
	    returnIfParamNotFound(parameters, "systemMode");
	    returnIfParamNotFound(parameters, "state");
	    bool status = false;
	    string strSystemMode = parameters["systemMode"].String();
	    string strState      = parameters["state"].String();
	    if (strSystemMode != "" && strState != "")
	    {
		    if(_systemMode) {
			    Exchange::ISystemMode::SystemMode  pSystemMode ;
			    Exchange::ISystemMode::State pState ;	    
			    auto it = StrToSystemModeMap.find(strSystemMode);
			    if (it != StrToSystemModeMap.end()) {
				    pSystemMode = it->second;
			    } else {
				    LOGERR("Invalid systemMode :%s ",strSystemMode.c_str());
				    JsonObject error;
				    error["message"] = "Invalid systemMode";
				    response["error"] = error;
				    status = false;
				    returnResponse(status);
			    }

			    auto it1 = StrToDeviceOptimizeStateMap.find(strState);
			    if (it1 != StrToDeviceOptimizeStateMap.end()) {
				    pState = it1->second;
			    } else {
				    pState = it1->second;
				    LOGERR("Invalid State :%s ",strState.c_str());
				    JsonObject error;
				    error["message"] = "Invalid state";
				    response["error"] = error;
				    status = false;
				    returnResponse(status);
			    }

			    if (Core::ERROR_NONE == _systemMode->RequestState(pSystemMode,pState) )
			    {
				    status = true ;
				    returnResponse(true);
			    }
			    else 
			    {
				    status = false ;
			    }
		    }
		    else
		    {
			    LOGERR("_systemMode is null ");
			    status = false;
		    }
	    }
	    else
	    {
		    LOGERR("systemMode/state is empty systemMode :%s ,state :%s ",strSystemMode.c_str(),strState.c_str());
		    JsonObject error;
		    error["message"] = "systemMode/state is empty";
		    response["error"] = error;
		    status = false;
	    }
	    returnResponse(status);
    }

    uint32_t SystemMode::GetState(const JsonObject& parameters, JsonObject& response)
    {
	    returnIfParamNotFound(parameters, "systemMode");
	    bool status = false;
	    string strSystemMode = parameters["systemMode"].String();
	    if (strSystemMode != "" )
	    {
		    if(_systemMode) {
			    Exchange::ISystemMode::SystemMode  pSystemMode ;
			    Exchange::ISystemMode::State pState ;			  
			    auto it = StrToSystemModeMap.find(strSystemMode);
			    if (it != StrToSystemModeMap.end()) {
				    pSystemMode = it->second;
			    } else {
				    LOGERR("Invalid systemMode :%s ",strSystemMode.c_str());
				    JsonObject error;
				    error["message"] = "Invalid systemMode";
				    response["error"] = error;
				    status = false;
				    returnResponse(status);
			    }

			    if (Core::ERROR_NONE == _systemMode->GetState(pSystemMode,pState) )
			    {
				    std::string keyForValue = "";

				    // Iterate through the map to find the key for the given value
				    for (const auto& pair : StrToDeviceOptimizeStateMap) {
					    if (pair.second == pState) {
						    keyForValue = pair.first;
						    break;
					    }
				    }

				    if (!keyForValue.empty()) {
					    LOGINFO("The key for the given value is: %s" ,keyForValue.c_str() ); 
					    response["state"] = keyForValue;
					    status = true ;
				    } else {
					    LOGERR ("Invalid State");
					    status = false;
				    }
			    }
			    else 
			    {
				    status = false ;
			    }
		    }
		    else
		    {
			    LOGERR("_systemMode is null ");
			    status = false;
		    }
	    }
	    else
	    {
		    LOGERR("systemMode is empty systemMode :%s ",strSystemMode.c_str());
		    JsonObject error;
		    error["message"] = "systemMode is empty";
		    response["error"] = error;
		    status = false;
	    }
	    returnResponse(status);
    }

} // namespace Plugin
} // namespace WPEFramework
