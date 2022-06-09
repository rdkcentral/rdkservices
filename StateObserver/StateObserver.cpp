/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/

 /**
 * @file StateObserver.cpp
 * @brief Thunder Plugin based Implementation for StateObserver service API's (RDK-25848).
 */

/**
  @mainpage StateObserver

  */

#include <stdio.h>
#include <list>
#include <vector>
#include <string>
#include <algorithm>

#include "StateObserver.h"
#include "libIARM.h"
#include "libIBus.h"
#include "iarmUtil.h"
#include "sysMgr.h"

#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"

#define STATEOBSERVER_MAJOR_VERSION 1
#define STATEOBSERVER_MINOR_VERSION 0
#define DEBUG_INFO 0

namespace WPEFramework {

		namespace Plugin {
		/*
		 *Register StateObserver  module as wpeframework plugin
		 **/
		SERVICE_REGISTRATION(StateObserver, STATEOBSERVER_MAJOR_VERSION, STATEOBSERVER_MINOR_VERSION);

		StateObserver* StateObserver::_instance = nullptr;

		const string StateObserver::STATE_OBSERVER_PLUGIN_NAME = "com.comcast.stateObserver";

		const string StateObserver::EVT_STATE_OBSERVER_PROPERTY_CHANGED = "propertyChanged";

		std::vector<string> registeredPropertyNames;


		StateObserver::StateObserver()
		: PluginHost::JSONRPC()
		, m_apiVersionNumber((uint32_t)-1)
		{
			StateObserver::_instance = this;
			Register("getValues", &StateObserver::getValues, this);
			Register("registerListeners", &StateObserver::registerListeners, this);
			Register("unregisterListeners", &StateObserver::unregisterListeners, this);
			Register("setApiVersionNumber", &StateObserver::setApiVersionNumberWrapper, this);
			Register("getApiVersionNumber", &StateObserver::getApiVersionNumberWrapper, this);
			Register("getRegisteredPropertyNames", &StateObserver::getRegisteredPropertyNames, this);
			Register("getName", &StateObserver::getNameWrapper, this);
			setApiVersionNumber(1);
		}

		StateObserver::~StateObserver()
		{
		}

		const string StateObserver::Initialize(PluginHost::IShell* /* service */)
		{
			InitializeIARM();

			// On success return empty, to indicate there is no error text.
			return (string());
		}

		void StateObserver::Deinitialize(PluginHost::IShell* /* service */)
		{
			DeinitializeIARM();
			StateObserver::_instance = nullptr;
			//Unregister all the APIs
		}

		void StateObserver::InitializeIARM()
		{
            if (Utils::IARM::init())
            {
                IARM_Result_t res;
			    IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE, onReportStateObserverEvents) );
            }
		}

		void StateObserver::DeinitializeIARM()
		{
            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE) );
            }
		}

		/**
		 * @brief This function is used to get the state observer plugin  name. The state observer
		 * plugin name is "com.comcast.stateObserver" .
		 *
		 * @return String of the state observer plugin  name.
 		 */
		string StateObserver::getName()
		{
			return StateObserver::STATE_OBSERVER_PLUGIN_NAME;
		}

		/**
		 * @brief This function is a wrapper function for getName function used to get the state observer
		 * plugin  name. The state observer plugin  name is "com.comcast.stateObserver".
		 *
		 * @param[out] String of the state observer plugin  name.
		 *
		 * @return Core::ERROR_NONE
		 */
		uint32_t StateObserver::getNameWrapper(const JsonObject& parameters, JsonObject& response)
		{
			response["Name"]= StateObserver::STATE_OBSERVER_PLUGIN_NAME;
			returnResponse(true);
		}

		uint32_t StateObserver::getRegisteredPropertyNames(const JsonObject &parameters, JsonObject &response)
		{
			JsonArray response_arr;
			for(const auto &propertyName: registeredPropertyNames)
			{
        		response_arr.Add(propertyName);
			}
			response["properties"] = response_arr;
			returnResponse(true);
		}

		/**
		 * @brief This function is used to get the version number of the state observer Plugin.
		 *
		 * @return version number of API integer value.
		 */
		unsigned int StateObserver::getApiVersionNumber()
		{
			return m_apiVersionNumber;
		}


		/**
		 * @brief This function is the wrapper  used to get the version number of the state observer
		 *  Plugin.
		 *
		 * param[out] The API Version Number.
		 *
		 * @return Core::ERROR_NONE
		 */
		uint32_t StateObserver::getApiVersionNumberWrapper(const JsonObject& parameters, JsonObject& response)
		{
			response["version"] = m_apiVersionNumber;
			returnResponse(true);
		}

		/**
		 * @brief This function is used to set the version number of the state observer Plugin.
		 *
		 * @param apiVersionNumber Integer variable of API version value.
		 */
		void StateObserver::setApiVersionNumber(unsigned int apiVersionNumber)
		{
			m_apiVersionNumber = apiVersionNumber;
		}

		/**
		 * @brief This function is the wrapper  used to get the version number of the state observer
		 *  Plugin.
		 *
		 * @param[in]  Api version number
		 *
		 * @return Core::ERROR_NONE
		 */
		uint32_t StateObserver::setApiVersionNumberWrapper(const JsonObject& parameters, JsonObject& response)
		{
			if (parameters.HasLabel("version"))
			{
				getNumberParameter("version", m_apiVersionNumber);
				returnResponse(true);
			}

			returnResponse(false);
		}


		/**
		 * @brief This function is used to get the values of the various device properties.
		 *
		 * param[in] Array of property names whose values needs to be fetched.
		 *
		 * param[out] state and error values of the properties
		 *
		 * @return Core::ERROR_NONE
		 *
		 *Request example: curl -d '{"jsonrpc":"2.0","id":"3","method": "StateObserver.1.getValues" ,"params":{"PropertyNames":["com.comcast.channel_map"]}}' http://127.0.0.1:9998/jsonrpc
		 *Response success:{"jsonrpc":"2.0","id":3,"result":{"properties":[{"propertyName":"com.comcast.channel_map","value":2}],"success":true}}
		 *Response failure:{"jsonrpc":"2.0","id":3,"result":{"success":false}}
		 */
		uint32_t StateObserver::getValues(const JsonObject& parameters, JsonObject& response)
		{
			LOGINFOMETHOD();
			bool ret=false;
			string json;
			parameters.ToString(json);
			LOGINFO("INPUT STR %s\n",json.c_str());
			cJSON *root;
			root=cJSON_Parse(json.c_str());
			if( root ==NULL)
			{
				LOGWARN("not able to parse json request \n");
				returnResponse(ret);
			}
			cJSON *items=cJSON_GetObjectItem(root,"PropertyNames");
			if( items == NULL)
			{
				LOGWARN("not able to fetch property names from request \n");
				cJSON_Delete(root);
				returnResponse(ret);
			}
			int arrsize=cJSON_GetArraySize(items);
			if(arrsize!=0)
			{
				ret=true;
				cJSON *elem;
				int i;
				std::vector<string> pname;
				for(i=0;i<arrsize;i++)
				{
				 	elem = cJSON_GetArrayItem(items, i);
					LOGINFO("prop array index %d  element %s\n",i,elem->valuestring);
					string prop_str =elem->valuestring;
					pname.push_back(prop_str);
				}
				getVal(pname,response);
			}
			LOGTRACEMETHODFIN();
			cJSON_Delete(root);
			returnResponse(ret);
		}


		/**
		 * @brief This function retrieves the values of the properties using IARM.
		 *
		 * param[in] pname vector of strings having the names of the properties whose value needs to be fetched.
		 *
		 * param[out] The state and error values of the properties.
		 *
		 */

		void StateObserver::getVal(std::vector<string> pname,JsonObject& response)
		{
			static bool checkForStandalone = true;
			static bool stbStandAloneMode = false;
			if (checkForStandalone)
			{
				char *env_var= getenv("SERVICE_MANAGER_STANDALONE_MODE");
				if ( env_var )
				{
					int v= atoi(env_var);
					LOGWARN("standalone mode value %d", v);
					if (v == 1)
					{
						stbStandAloneMode = true;
						LOGWARN("standalone mode enabled");
					}
				}
				checkForStandalone = false;
			}
			IARM_Bus_SYSMgr_GetSystemStates_Param_t param;
			IARM_Bus_Call(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_API_GetSystemStates, &param, sizeof(param));
			JsonArray response_arr;
			for( std::vector<string>::iterator it = pname.begin(); it!= pname.end(); ++it )
			{
				string err_str="none";
				JsonObject devProp;
				if(*it == SYSTEM_CHANNEL_MAP)
				{
					int channelMapState = param.channel_map.state;
					int channelMapError = param.channel_map.error;
					if (stbStandAloneMode)
					{
						LOGINFO("stand alone mode true\n");
						channelMapState = 2;
						channelMapError = 0;
					}
					devProp["propertyName"]=SYSTEM_CHANNEL_MAP;
					devProp["value"]=channelMapState;
					if(channelMapError == 1)
					{
						err_str="RDK-03005";
					}
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}

				else if(*it == SYSTEM_CARD_DISCONNECTED)
				{
					int systemCardState = param.disconnect_mgr_state.state;
					int systemCardError = param.disconnect_mgr_state.error;
					if (stbStandAloneMode)
					{
						systemCardState = 0;
						systemCardError = 0;
					}
					devProp["propertyName"]=SYSTEM_CARD_DISCONNECTED;
					devProp["value"]=systemCardState;
					if(systemCardError==1)
					{
						err_str = "RDK-03007";
					}
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it  == SYSTEM_TUNE_READY)
				{
					int tuneReadyState = param.TuneReadyStatus.state;
					if (stbStandAloneMode)
					{
						tuneReadyState = 1;
					}
					devProp["propertyName"]=SYSTEM_TUNE_READY;
					devProp["value"]=tuneReadyState;
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it  == SYSTEM_EXIT_OK)
				{
					devProp["propertyName"]=SYSTEM_EXIT_OK;
					devProp["value"]=param.exit_ok_key_sequence.state;
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it ==SYSTEM_CMAC )
				{
					devProp["propertyName"]=SYSTEM_CMAC ;
					devProp["value"]=param.cmac.state;
					if(param.cmac.error == 1)
					{
						err_str  = "RDK-03002";
					}
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it ==SYSTEM_MOTO_ENTITLEMENT )
				{
					devProp["propertyName"]=SYSTEM_MOTO_ENTITLEMENT;
					devProp["value"]=param.card_moto_entitlements.state;
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_DAC_INIT_TIMESTAMP)
				{
					devProp["propertyName"]=SYSTEM_DAC_INIT_TIMESTAMP;
					string dac_init_str(param.dac_init_timestamp.payload);
					devProp["value"]=dac_init_str;
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_CARD_SERIAL_NO )
				{
					string card_serial_str(param.card_serial_no.payload);
					devProp["propertyName"]=SYSTEM_CARD_SERIAL_NO;
					devProp["value"]=card_serial_str;
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_STB_SERIAL_NO )
				{
					string stb_string(param.stb_serial_no.payload);
					devProp["propertyName"]=SYSTEM_STB_SERIAL_NO;
					devProp["value"]=stb_string;
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it ==SYSTEM_ECM_MAC )
				{
					string ecm_mac_str(param.ecm_mac.payload);
					devProp["propertyName"]=SYSTEM_ECM_MAC;
					devProp["value"]=ecm_mac_str;
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_MOTO_HRV_RX)
				{
					devProp["propertyName"]=SYSTEM_MOTO_HRV_RX;
					devProp["value"]=param.card_moto_hrv_rx.state;
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_CARD_CISCO_STATUS )
				{
					LOGINFO("property SYSTEM_CARD_CISCO_STATUS \n");
					devProp["propertyName"]=SYSTEM_CARD_CISCO_STATUS;
					devProp["value"]=param.card_cisco_status.state;
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_VIDEO_PRESENTING)
				{
					devProp["propertyName"]=SYSTEM_VIDEO_PRESENTING;
					devProp["value"]=param.video_presenting.state;
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_HDMI_OUT )
				{
					devProp["propertyName"]=SYSTEM_HDMI_OUT;
					devProp["value"]=param.hdmi_out.state;
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_HDCP_ENABLED )
				{
					devProp["propertyName"]=SYSTEM_HDCP_ENABLED;
					devProp["value"]=param.hdcp_enabled.state;
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_HDMI_EDID_READ )
				{
					devProp["propertyName"]=SYSTEM_HDMI_EDID_READ;
					devProp["value"]=param.hdmi_edid_read.state;
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_FIRMWARE_DWNLD )
				{
					devProp["propertyName"]=SYSTEM_FIRMWARE_DWNLD;
					devProp["value"]=param.firmware_download.state;
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_TIME_SOURCE)
				{
					LOGWARN("%s PropertyName: %s Time source state: %d, time source error: %d",
						__FUNCTION__,
						SYSTEM_TIME_SOURCE.c_str(),
						param.time_source.state,
						param.time_source.error);

					devProp["propertyName"]=SYSTEM_TIME_SOURCE;
					devProp["value"]=param.time_source.state;
					if(param.time_source.error == 1)
					{
						err_str  = "RDK-03006";
					}
					devProp["error"]=err_str;

					std::string json;
					devProp.ToString(json);
					LOGWARN("%s devProp=%s", __FUNCTION__, json.c_str());
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_TIME_ZONE)
				{
					devProp["propertyName"]=SYSTEM_TIME_ZONE;
					devProp["value"]=param.time_zone_available.state;
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_CA_SYSTEM)
				{
					devProp["propertyName"]=SYSTEM_CA_SYSTEM;
					devProp["value"]=param.ca_system.state;
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_ESTB_IP)
				{
					devProp["propertyName"]=SYSTEM_ESTB_IP;
					devProp["value"]=param.estb_ip.state;
					if(param.estb_ip.error == 1)
					{
						err_str  = "RDK-03009";
					}
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_ECM_IP)
				{
					devProp["propertyName"]=SYSTEM_ECM_IP;
					devProp["value"]=param.ecm_ip.state;
					if(param.ecm_ip.error == 1)
					{
						err_str  = "RDK-03004";
					}
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_LAN_IP)
				{
					devProp["propertyName"]=SYSTEM_LAN_IP;
					devProp["value"]=param.lan_ip.state;
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_DOCSIS)
				{
					devProp["propertyName"]=SYSTEM_DOCSIS;
					devProp["value"]=param.docsis.state;
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_DSG_CA_TUNNEL)
				{
					devProp["propertyName"]=SYSTEM_DSG_CA_TUNNEL;
					devProp["value"]=param.dsg_ca_tunnel.state;
					if(param.dsg_ca_tunnel.error == 1)
					{
						err_str  = "RDK-03003";
					}
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_CABLE_CARD )
				{
					devProp["propertyName"]=SYSTEM_CABLE_CARD;
					devProp["value"]=param.cable_card.state;
					if(param.cable_card.error == 1)
					{
						err_str  = "RDK-03001";
					}
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_VOD_AD)
				{
					devProp["propertyName"]=SYSTEM_VOD_AD;
					devProp["value"]=param.vod_ad.state;
					devProp["error"]=err_str;
					response_arr.Add(devProp);
				}
				else if(*it == SYSTEM_IP_MODE)
				{
					int ipState=param.ip_mode.state;
					int ipError=param.ip_mode.error;
					devProp["propertyName"]=SYSTEM_IP_MODE;
					devProp["value"]=ipState;
					devProp["error"]=ipError;
					response_arr.Add(devProp);
				}

				else
				{
					LOGINFO("Invalid property Name\n");
					string res="Invalid property Name";
 					devProp["propertyName"] = *it;
					devProp["error"]=res;
					response_arr.Add(devProp);
				}

			}

			response["properties"]=response_arr;
			#if(DEBUG_INFO)
				string json_str;
				response.ToString(json_str);
				LOGINFO("json array of properties is %s\n",json_str.c_str());
			#endif
		}


		 /**
		 * @brief This function registers Listeners to properties.It adds the properties to a registered properties list.
		 *
		 * param[in] Names of the property.
		 *
		 * param[out] Returns the state and error values of the properties.
		 *
		 * @return Core::ERROR_NONE
		 *Request example: curl -d '{"jsonrpc":"2.0","id":"3","method": "StateObserver.1.registerListeners" ,"params":{"PropertyNames":["com.comcast.channel_map"]}}' http://127.0.0.1:9998/jsonrpc
		 *Response success:{"jsonrpc":"2.0","id":3,"result":{"properties":[{"propertyName":"com.comcast.channel_map","value":2}],"success":true}}
		 *Response failure:{"jsonrpc":"2.0","id":3,"result":{"success":false}}
		 */
		uint32_t StateObserver::registerListeners(const JsonObject& parameters, JsonObject& response)
		{
			LOGINFOMETHOD();
			bool ret=false;
			string json;
			parameters.ToString(json);
			cJSON *root;
			root=cJSON_Parse(json.c_str());
			if( root ==NULL)
			{
				LOGWARN("not able to parse json request \n");
				returnResponse(ret);
			}
			cJSON *items=cJSON_GetObjectItem(root,"PropertyNames");
			if( items == NULL)
			{
				LOGWARN("not able to fetch property names from request \n");
				cJSON_Delete(root);
				returnResponse(ret);
			}
			int arrsize=cJSON_GetArraySize(items);
			if(arrsize!=0)
			{
				ret=true;
				cJSON *elem;
				int i;
				std::vector<string> pname;
				for(i=0;i<arrsize;i++)
				{
					elem = cJSON_GetArrayItem(items, i);
					string prop_str =elem->valuestring;
					pname.push_back(prop_str);
				}

				for( std::vector<string>::iterator it = pname.begin(); it!= pname.end(); ++it )
				{
					if (std::find(registeredPropertyNames.begin(), registeredPropertyNames.end(), *it) == registeredPropertyNames.end())
					{
						LOGINFO("prop being added to listeners %s",it->c_str());
						registeredPropertyNames.push_back(*it);
					}
				}
				getVal(pname,response);
			}
			LOGTRACEMETHODFIN();
			cJSON_Delete(root);
			returnResponse(ret);
		}


		/**
		 * @brief This function unregisters Listeners to properties.It removes the properties from  a registered properties list.
		 *
		 * param[in] Names of the property.
		 *
		 * param[out] Returns the sucess true/false.
		 *
		 * @return Core::ERROR_NONE
		 *Request example: curl -d '{"jsonrpc":"2.0","id":"3","method": "StateObserver.1.unregisterListeners" ,"params":{"PropertyNames":["com.comcast.channel_map"]}}' http://127.0.0.1:9998/jsonrpc
		 *Response success:{"jsonrpc":"2.0","id":3,"result":{"success":true}}
		 *Response failure:{"jsonrpc":"2.0","id":3,"result":{"success":false}}
		 */
		uint32_t StateObserver::unregisterListeners(const JsonObject& parameters, JsonObject& response)
		{
			LOGINFOMETHOD();
			bool ret=false;
			string json;
			parameters.ToString(json);
			cJSON *root;
			root=cJSON_Parse(json.c_str());
			if( root ==NULL)
			{
				LOGWARN("not able to parse json request \n");
				returnResponse(ret);
			}
			cJSON *items=cJSON_GetObjectItem(root,"PropertyNames");
			if( items == NULL)
			{
				LOGWARN("not able to fetch property names from request \n");
				cJSON_Delete(root);
				returnResponse(ret);
			}
			int arrsize=cJSON_GetArraySize(items);
			if(arrsize!=0)
			{
				ret=true;
				cJSON *elem;
				int i;
				std::vector<string> pname;
				for(i=0;i<arrsize;i++)
				{
					elem = cJSON_GetArrayItem(items, i);
					string prop_str =elem->valuestring;
					pname.push_back(prop_str);
				}
				for( std::vector<string>::iterator it = pname.begin(); it!= pname.end(); ++it )
				{
					std::vector<string>::iterator itr=std::find(registeredPropertyNames.begin(), registeredPropertyNames.end(), *it);
					if(itr!=registeredPropertyNames.end())
					{
						//property found hence remove it
						LOGINFO("prop being removed %s",it->c_str());
						registeredPropertyNames.erase(itr);
					}
				}
			}
			LOGTRACEMETHODFIN();
			cJSON_Delete(root);
			returnResponse(ret);
		}

		 /**
		 * @brief This function is an event handler for property change event.
		 *
		 */
		void StateObserver::onReportStateObserverEvents(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
		{
			IARM_Bus_SYSMgr_GetSystemStates_Param_t systemStates;
			JsonObject params;
			int state=0;
			int error=0;

			/* Only handle state events */
			if (eventId != IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE)
			{
				LOGWARN(" No need handle other events..");
			}
			else
			{
				LOGINFO(" Property changed event received ");
				IARM_Bus_SYSMgr_EventData_t *sysEventData = (IARM_Bus_SYSMgr_EventData_t*)data;
				IARM_Bus_SYSMgr_SystemState_t stateId = sysEventData->data.systemStates.stateId;
				state = sysEventData->data.systemStates.state;
				error = sysEventData->data.systemStates.error;
				char* payload=sysEventData->data.systemStates.payload;
				#if(DEBUG_INFO)
					LOGINFO("stateId is %d state is %d error is %d \n",stateId,state,error);
					LOGINFO("payload is %s\n",payload);
				#endif
				switch(stateId)
				{

					case IARM_BUS_SYSMGR_SYSSTATE_TUNEREADY:
						systemStates.TuneReadyStatus.state = state;
						systemStates.TuneReadyStatus.error = error;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_TUNE_READY,state,error);
						break;

					case IARM_BUS_SYSMGR_SYSSTATE_CHANNELMAP:
						{
						systemStates.channel_map.state = state;
						systemStates.channel_map.error = error;
						memcpy( systemStates.channel_map.payload, payload, sizeof( systemStates.channel_map.payload ) );
						systemStates.channel_map.payload[ sizeof( systemStates.channel_map.payload ) - 1] = 0;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_CHANNEL_MAP,state,error);
						string payload_str(payload);
						params["payload"].FromString(payload_str);
						break;
						}

					case IARM_BUS_SYSMGR_SYSSTATE_DISCONNECTMGR:
						systemStates.disconnect_mgr_state.state = state;
						systemStates.disconnect_mgr_state.error = error;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_CARD_DISCONNECTED,state,error);
						break;


					case IARM_BUS_SYSMGR_SYSSTATE_EXIT_OK :
						systemStates.exit_ok_key_sequence.state = state;
						systemStates.exit_ok_key_sequence.error = error;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_EXIT_OK,state,error);
						break;

					case IARM_BUS_SYSMGR_SYSSTATE_CMAC :
						systemStates.cmac.state = state;
						systemStates.cmac.error = error;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_CMAC,state,error);
						break;

					case IARM_BUS_SYSMGR_SYSSTATE_MOTO_ENTITLEMENT :
						systemStates.card_moto_entitlements.state = state;
						systemStates.card_moto_entitlements.error = error;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_MOTO_ENTITLEMENT,state,error);
						break;

					case IARM_BUS_SYSMGR_SYSSTATE_MOTO_HRV_RX :
						systemStates.card_moto_hrv_rx.state = state;
						systemStates.card_moto_hrv_rx.error = error;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_MOTO_HRV_RX,state,error);
						break;

					case IARM_BUS_SYSMGR_SYSSTATE_DAC_INIT_TIMESTAMP :
						{
						systemStates.dac_init_timestamp.state = state;
						systemStates.dac_init_timestamp.error = error;
						strncpy(systemStates.dac_init_timestamp.payload,payload,strlen(payload));
						systemStates.dac_init_timestamp.payload[strlen(payload)]='\0';
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_DAC_INIT_TIMESTAMP,state,error);
						string payload_str(payload);
						params["payload"].FromString(payload_str);
						break;
						}

					case IARM_BUS_SYSMGR_SYSSTATE_CABLE_CARD_SERIAL_NO:
						{
						systemStates.card_serial_no.error =error;
						strncpy(systemStates.card_serial_no.payload,payload,strlen(payload));
						systemStates.card_serial_no.payload[strlen(payload)]='\0';
						params["propertyName"]=SYSTEM_CARD_SERIAL_NO;
						params["error"]=error;
						string payload_str(payload);
						params["payload"].FromString(payload_str);
						break;
						}

					 case IARM_BUS_SYSMGR_SYSSTATE_STB_SERIAL_NO:
						{
						systemStates.stb_serial_no.error =error;
						strncpy(systemStates.stb_serial_no.payload,payload,strlen(payload));
						systemStates.stb_serial_no.payload[strlen(payload)]='\0';
						params["propertyName"]=SYSTEM_STB_SERIAL_NO;
						params["error"]=error;
						string payload_str(payload);
						params["payload"].FromString(payload_str);
						break;
						}

					case IARM_BUS_SYSMGR_SYSSTATE_CARD_CISCO_STATUS :
						systemStates.card_cisco_status.state = state;
						systemStates.card_cisco_status.error = error;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_CARD_CISCO_STATUS,state,error);
						break;

					case IARM_BUS_SYSMGR_SYSSTATE_VIDEO_PRESENTING :
						systemStates.video_presenting.state = state;
						systemStates.video_presenting.error = error;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_VIDEO_PRESENTING,state,error);
						break;

					case IARM_BUS_SYSMGR_SYSSTATE_HDMI_OUT :
						systemStates.hdmi_out.state = state;
						systemStates.hdmi_out.error = error;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_HDMI_OUT,state,error);
						break;

					case IARM_BUS_SYSMGR_SYSSTATE_HDCP_ENABLED :
						systemStates.hdcp_enabled.state = state;
						systemStates.hdcp_enabled.error = error;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_HDCP_ENABLED,state,error);
						break;

					case IARM_BUS_SYSMGR_SYSSTATE_HDMI_EDID_READ :
						systemStates.hdmi_edid_read.state = state;
						systemStates.hdmi_edid_read.error = error;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_HDMI_EDID_READ,state,error);
						break;

					case IARM_BUS_SYSMGR_SYSSTATE_FIRMWARE_DWNLD :
						systemStates.firmware_download.state = state;
						systemStates.firmware_download.error = error;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_FIRMWARE_DWNLD,state,error);
						break;

					case IARM_BUS_SYSMGR_SYSSTATE_TIME_SOURCE :
						systemStates.time_source.state = state;
						systemStates.time_source.error = error;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_TIME_SOURCE,state,error);
						break;

					case IARM_BUS_SYSMGR_SYSSTATE_TIME_ZONE :
						{
						systemStates.time_zone_available.state = state;
						systemStates.time_zone_available.error = error;
						memcpy( systemStates.time_zone_available.payload, payload, sizeof( systemStates.time_zone_available.payload ) );
						systemStates.time_zone_available.payload[ sizeof( systemStates.time_zone_available.payload ) - 1] = 0;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_TIME_ZONE,state,error);
						string payload_str(payload);
						params["payload"] = payload_str;
						break;
						}

					case   IARM_BUS_SYSMGR_SYSSTATE_CA_SYSTEM :
						systemStates.ca_system.state = state;
						systemStates.ca_system.error = error;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_CA_SYSTEM,state,error);
						break;

					case   IARM_BUS_SYSMGR_SYSSTATE_ESTB_IP :
						systemStates.estb_ip.state = state;
						systemStates.estb_ip.error = error;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_ESTB_IP,state,error);
						break;

					case   IARM_BUS_SYSMGR_SYSSTATE_ECM_IP :
						systemStates.ecm_ip.state = state;
						systemStates.ecm_ip.error = error;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_ECM_IP,state,error);
						break;

					case   IARM_BUS_SYSMGR_SYSSTATE_LAN_IP :
						systemStates.lan_ip.state = state;
						systemStates.lan_ip.error = error;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_LAN_IP,state,error);
						break;

					case   IARM_BUS_SYSMGR_SYSSTATE_DOCSIS :
						systemStates.docsis.state = state;
						systemStates.docsis.error = error;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_DOCSIS,state,error);
						break;

					case   IARM_BUS_SYSMGR_SYSSTATE_DSG_CA_TUNNEL :
						systemStates.dsg_ca_tunnel.state = state;
						systemStates.dsg_ca_tunnel.error = error;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_DSG_CA_TUNNEL,state,error);
						break;

					case   IARM_BUS_SYSMGR_SYSSTATE_CABLE_CARD :
						systemStates.cable_card.state = state;
						systemStates.cable_card.error = error;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_CABLE_CARD,state,error);
						break;

					case   IARM_BUS_SYSMGR_SYSSTATE_VOD_AD :
						{
						systemStates.vod_ad.state = state;
						systemStates.vod_ad.error = error;
						memcpy( systemStates.vod_ad.payload, payload, sizeof( systemStates.vod_ad.payload ) );
						systemStates.vod_ad.payload[ sizeof( systemStates.vod_ad.payload ) -1 ] =0;
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_VOD_AD,state,error);
						string payload_str(payload);
						params["payload"].FromString(payload_str);
						break;
						}

					case IARM_BUS_SYSMGR_SYSSTATE_ECM_MAC:
						{
						systemStates.ecm_mac.error =error;
						strncpy(systemStates.ecm_mac.payload,payload,strlen(payload));
						systemStates.ecm_mac.payload[strlen(payload)]='\0';
						params["propertyName"]=SYSTEM_ECM_MAC;
						params["error"]=error;
						string payload_str(payload);
						params["payload"].FromString(payload_str);
						break;
						}

					case   IARM_BUS_SYSMGR_SYSSTATE_IP_MODE:
						{
						systemStates.ip_mode.state=state;
						systemStates.ip_mode.error =error;
						strncpy(systemStates.ip_mode.payload,payload,strlen(payload));
						systemStates.ip_mode.payload[strlen(payload)]='\0';
						if(StateObserver::_instance)
							StateObserver::_instance->setProp(params,SYSTEM_IP_MODE,state,error);
						string payload_str(payload);
						params["payload"].FromString(payload_str);
						break;
						}
					default:
						break;
				}

				//notify the params
				if(StateObserver::_instance)
				{
					#if(DEBUG_INFO)
						LOGINFO("calling notify event\n");
						string json;
						params.ToString(json);
						LOGINFO("NOTIFY json is %s\n",json.c_str());
					#endif
					StateObserver::_instance->notify(EVT_STATE_OBSERVER_PROPERTY_CHANGED,params);
				}
			}
		}

		/**
		 * @brief This function sends a notification whenever a change in properties value occurs.
		 *
		 * param[in] Property change event name.
		 *
		 * param[out] Notification about change in property.
		 *
		 */
		void StateObserver::notify(string eventname, JsonObject& params)
		{
			string property_name=params["propertyName"].String();
			if(std::find(registeredPropertyNames.begin(), registeredPropertyNames.end(), property_name) != registeredPropertyNames.end())
			{
				LOGINFO("calling send notify\n");
				#if(DEBUG_INFO)
					string json_str;
					params.ToString(json_str);
					LOGINFO("send notify request  %s \n",json_str.c_str());
				#endif
				//property added in registeredProperty list hence call sendNotify
				sendNotify(eventname.c_str(), params);
			}
		}

		/**
		 * @brief This function sets the state and error values to the json object.
		 *
		 * param[in] propName Name of property.
		 * param[in] state  state of property.
		 * param[in] error  error of property.
		 *
		 * param[out] JsonObject Property's values in a JsonObject.
		 *
		 */
		void StateObserver::setProp(JsonObject& params,string propName,int value,int error)
		{
			string property(propName);
			params["propertyName"]=property;
			params["value"]=value;
			params["error"]=error;
			#if(DEBUG_INFO)
				string json;
				params.ToString(json);
				LOGINFO("setProp Constructed json is %s\n",json.c_str());
			#endif
		}

	} // namespace Plugin
} // namespace WPEFramework
