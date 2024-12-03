/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
 *
 * Copyright 2023 RDK Management
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

#include <algorithm>
#include <regex>
#if defined(SECURITY_TOKEN_ENABLED) && ((SECURITY_TOKEN_ENABLED == 0) || (SECURITY_TOKEN_ENABLED == false))
#define GetSecurityToken(a, b) 0
#define GetToken(a, b, c) 0
#else
#include <WPEFramework/securityagent/securityagent.h>
#include <WPEFramework/securityagent/SecurityTokenUtil.h>
#endif
#include "MiracastService.h"
#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"
#include <fstream>

#define MIRACAST_DEVICE_PROPERTIES_FILE          "/etc/device.properties"

const short WPEFramework::Plugin::MiracastService::API_VERSION_NUMBER_MAJOR = 1;
const short WPEFramework::Plugin::MiracastService::API_VERSION_NUMBER_MINOR = 0;

const string WPEFramework::Plugin::MiracastService::SERVICE_NAME = "org.rdk.MiracastService";

const string WPEFramework::Plugin::MiracastService::METHOD_MIRACAST_SET_ENABLE = "setEnable";
const string WPEFramework::Plugin::MiracastService::METHOD_MIRACAST_GET_ENABLE = "getEnable";
const string WPEFramework::Plugin::MiracastService::METHOD_MIRACAST_SET_P2P_BACKEND_DISCOVERY = "setP2PBackendDiscovery";
const string WPEFramework::Plugin::MiracastService::METHOD_MIRACAST_CLIENT_CONNECT = "acceptClientConnection";
const string WPEFramework::Plugin::MiracastService::METHOD_MIRACAST_STOP_CLIENT_CONNECT = "stopClientConnection";
const string WPEFramework::Plugin::MiracastService::METHOD_MIRACAST_SET_UPDATE_PLAYER_STATE = "updatePlayerState";
const string WPEFramework::Plugin::MiracastService::METHOD_MIRACAST_SET_LOG_LEVEL = "setLogging";

#ifdef UNIT_TESTING
const string WPEFramework::Plugin::MiracastService::METHOD_MIRACAST_GET_STATUS = "getStatus";
const string WPEFramework::Plugin::MiracastService::METHOD_MIRACAST_SET_POWERSTATE = "setPowerState";
const string WPEFramework::Plugin::MiracastService::METHOD_MIRACAST_SET_WIFISTATE = "setWiFiState";
#endif /*UNIT_TESTING*/

#ifdef ENABLE_MIRACAST_SERVICE_TEST_NOTIFIER
const string WPEFramework::Plugin::MiracastService::METHOD_MIRACAST_TEST_NOTIFIER = "testNotifier";
#endif

using namespace std;

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 10

#define SERVER_DETAILS "127.0.0.1:9998"
#define SYSTEM_CALLSIGN "org.rdk.System"
#define SYSTEM_CALLSIGN_VER SYSTEM_CALLSIGN ".1"
#define WIFI_CALLSIGN "org.rdk.Wifi"
#define WIFI_CALLSIGN_VER WIFI_CALLSIGN ".1"
#define SECURITY_TOKEN_LEN_MAX 1024
#define THUNDER_RPC_TIMEOUT 2000

#define EVT_ON_CLIENT_CONNECTION_REQUEST       "onClientConnectionRequest"
#define EVT_ON_CLIENT_CONNECTION_ERROR         "onClientConnectionError"
#define EVT_ON_LAUNCH_REQUEST                  "onLaunchRequest"

static IARM_Bus_PWRMgr_PowerState_t m_powerState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY;
static bool m_IsTransitionFromDeepSleep = false;
static bool m_IsWiFiConnectingState = false;

namespace WPEFramework
{
	namespace
	{
		static Plugin::Metadata<Plugin::MiracastService> metadata(
			// Version (Major, Minor, Patch)
			API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
			// Preconditions
			{},
			// Terminations
			{},
			// Controls
			{});
	}

	namespace Plugin
	{
		SERVICE_REGISTRATION(MiracastService, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

		MiracastService *MiracastService::_instance = nullptr;
		MiracastController *MiracastService::m_miracast_ctrler_obj = nullptr;

		MiracastService::MiracastService()
			: PluginHost::JSONRPC(),
			  m_isServiceInitialized(false),
			  m_isServiceEnabled(false),
			  m_eService_state(MIRACAST_SERVICE_STATE_IDLE)
		{
			LOGINFO("Entering..!!!");
			MiracastService::_instance = this;
			MIRACAST::logger_init("MiracastService");

			Register(METHOD_MIRACAST_SET_ENABLE, &MiracastService::setEnableWrapper, this);
			Register(METHOD_MIRACAST_GET_ENABLE, &MiracastService::getEnable, this);
			Register(METHOD_MIRACAST_SET_P2P_BACKEND_DISCOVERY, &MiracastService::setP2PBackendDiscovery, this);
			Register(METHOD_MIRACAST_CLIENT_CONNECT, &MiracastService::acceptClientConnection, this);
			Register(METHOD_MIRACAST_STOP_CLIENT_CONNECT, &MiracastService::stopClientConnection, this);
			Register(METHOD_MIRACAST_SET_UPDATE_PLAYER_STATE, &MiracastService::updatePlayerState, this);
			Register(METHOD_MIRACAST_SET_LOG_LEVEL, &MiracastService::setLogging, this);
		#ifdef UNIT_TESTING
			Register(METHOD_MIRACAST_GET_STATUS, &MiracastService::getStatus, this);
			Register(METHOD_MIRACAST_SET_POWERSTATE, &MiracastService::setPowerStateWrapper, this);
			Register(METHOD_MIRACAST_SET_WIFISTATE, &MiracastService::setWiFiStateWrapper, this);
		#endif /*UNIT_TESTING*/

#ifdef ENABLE_MIRACAST_SERVICE_TEST_NOTIFIER
			m_isTestNotifierEnabled = false;
			Register(METHOD_MIRACAST_TEST_NOTIFIER, &MiracastService::testNotifier, this);
#endif /* ENABLE_MIRACAST_SERVICE_TEST_NOTIFIER */
			LOGINFO("Exiting..!!!");
		}

		MiracastService::~MiracastService()
		{
			LOGINFO("Entering..!!!");
			if (nullptr != m_SystemPluginObj)
			{
				delete m_SystemPluginObj;
				m_SystemPluginObj = nullptr;
			}
			if (nullptr != m_WiFiPluginObj)
			{
				delete m_WiFiPluginObj;
				m_WiFiPluginObj = nullptr;
			}
			if (m_FriendlyNameMonitorTimerID)
			{
				g_source_remove(m_FriendlyNameMonitorTimerID);
				m_FriendlyNameMonitorTimerID = 0;
			}
			remove_wifi_connection_state_timer();
			remove_miracast_connection_timer();
			Unregister(METHOD_MIRACAST_SET_ENABLE);
			Unregister(METHOD_MIRACAST_GET_ENABLE);
			Unregister(METHOD_MIRACAST_STOP_CLIENT_CONNECT);
			Unregister(METHOD_MIRACAST_CLIENT_CONNECT);
			Unregister(METHOD_MIRACAST_SET_UPDATE_PLAYER_STATE);
			Unregister(METHOD_MIRACAST_SET_LOG_LEVEL);
			MIRACAST::logger_deinit();
			LOGINFO("Exiting..!!!");
		}

		const void MiracastService::InitializeIARM()
		{
			if (Utils::IARM::init())
			{
				IARM_Result_t res;
				IARM_Bus_PWRMgr_GetPowerState_Param_t param;

				IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME,IARM_BUS_PWRMGR_EVENT_MODECHANGED, pwrMgrModeChangeEventHandler) );
				// get power state:
				res = IARM_Bus_Call(IARM_BUS_PWRMGR_NAME,
									IARM_BUS_PWRMGR_API_GetPowerState,
									(void *)&param,
									sizeof(param));
				if(IARM_RESULT_SUCCESS == res )
				{
					LOGINFO("MiracastService::Current state is IARM: (%d) \n",param.curState);
					setPowerState(param.curState);
				}
			}
		}

		void MiracastService::DeinitializeIARM()
		{
			if (Utils::IARM::isConnected())
			{
				IARM_Result_t res;
				IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_PWRMGR_NAME,IARM_BUS_PWRMGR_EVENT_MODECHANGED, pwrMgrModeChangeEventHandler) );
			}
		}

		void MiracastService::pwrMgrModeChangeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
		{
			if (nullptr == _instance)
			{
				LOGERR("#### MCAST-TRIAGE-NOK-PWR Miracast Service not enabled yet ####");
				return;
			}

			if ((0 == strcmp(owner, IARM_BUS_PWRMGR_NAME)) && (IARM_BUS_PWRMGR_EVENT_MODECHANGED == eventId ))
			{
				IARM_Bus_PWRMgr_EventData_t *param = (IARM_Bus_PWRMgr_EventData_t *)data;

				lock_guard<mutex> lck(_instance->m_DiscoveryStateMutex);
				_instance->setPowerState(param->data.state.newState);
			}
		}

		// Thunder plugins communication
		void MiracastService::getThunderPlugins()
		{
			MIRACASTLOG_INFO("Entering..!!!");

			if (nullptr == m_SystemPluginObj)
			{
				string token;
				// TODO: use interfaces and remove token
				auto security = m_CurrentService->QueryInterfaceByCallsign<PluginHost::IAuthenticate>("SecurityAgent");
				if (nullptr != security)
				{
					string payload = "http://localhost";
					if (security->CreateToken(static_cast<uint16_t>(payload.length()),
											  reinterpret_cast<const uint8_t *>(payload.c_str()),
											  token) == Core::ERROR_NONE)
					{
						MIRACASTLOG_INFO("got security token\n");
					}
					else
					{
						LOGERR("failed to get security token\n");
					}
					security->Release();
				}
				else
				{
					LOGERR("No security agent\n");
				}

				string query = "token=" + token;
				Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(SERVER_DETAILS)));
				m_SystemPluginObj = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(_T(SYSTEM_CALLSIGN_VER), (_T("MiracastService")), false, query);
				if (nullptr == m_SystemPluginObj)
				{
					LOGERR("JSONRPC: %s: initialization failed", SYSTEM_CALLSIGN_VER);
				}
				else
				{
					MIRACASTLOG_INFO("JSONRPC: %s: initialization ok", SYSTEM_CALLSIGN_VER);
				}

				m_WiFiPluginObj = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(_T(WIFI_CALLSIGN_VER), (_T("MiracastService")), false, query);
				if (nullptr == m_WiFiPluginObj)
				{
					LOGERR("JSONRPC: %s: initialization failed", WIFI_CALLSIGN_VER);
				}
				else
				{
					MIRACASTLOG_INFO("JSONRPC: %s: initialization ok", WIFI_CALLSIGN_VER);
				}
			}
			MIRACASTLOG_INFO("Exiting..!!!");
		}

		const string MiracastService::Initialize(PluginHost::IShell *service)
		{
			string	msg,
					p2p_ctrl_iface = "";
			MIRACASTLOG_INFO("Entering..!!!");

			if (!(envGetValue("WIFI_P2P_CTRL_INTERFACE", p2p_ctrl_iface)))
			{
				return "WIFI_P2P_CTRL_INTERFACE not configured in device properties file";
			}

			if (!m_isServiceInitialized)
			{
				MiracastError ret_code = MIRACAST_OK;
		
				m_miracast_ctrler_obj = MiracastController::getInstance(ret_code, this,p2p_ctrl_iface);
				if (nullptr != m_miracast_ctrler_obj)
				{
					InitializeIARM();
					m_CurrentService = service;
					getThunderPlugins();
					// subscribe for event
					if (nullptr != m_SystemPluginObj)
					{
						m_SystemPluginObj->Subscribe<JsonObject>(1000, "onFriendlyNameChanged", &MiracastService::onFriendlyNameUpdateHandler, this);
					}
					if (nullptr != m_WiFiPluginObj)
					{
						m_WiFiPluginObj->Subscribe<JsonObject>(1000, "onWIFIStateChanged", &MiracastService::onWIFIStateChangedHandler, this);
					}

					if ( false == updateSystemFriendlyName())
					{
						m_FriendlyNameMonitorTimerID = g_timeout_add(2000, MiracastService::monitor_friendly_name_timercallback, this);
						MIRACASTLOG_WARNING("Unable to get friendlyName, requires polling [%u]...",m_FriendlyNameMonitorTimerID);
					}
					else
					{
						MIRACASTLOG_INFO("friendlyName updated properly...");
					}
					m_isServiceInitialized = true;
					m_miracast_ctrler_obj->m_ePlayer_state = MIRACAST_PLAYER_STATE_IDLE;
				}
				else
				{
					switch (ret_code)
					{
					case MIRACAST_INVALID_P2P_CTRL_IFACE:
					{
						msg = "Invalid P2P Ctrl iface configured";
					}
					break;
					case MIRACAST_CONTROLLER_INIT_FAILED:
					{
						msg = "Controller Init Failed";
					}
					break;
					case MIRACAST_P2P_INIT_FAILED:
					{
						msg = "P2P Init Failed";
					}
					break;
					case MIRACAST_RTSP_INIT_FAILED:
					{
						msg = "RTSP msg handler Init Failed";
					}
					break;
					default:
					{
						msg = "Unknown Error:Failed to obtain MiracastController Object";
					}
					break;
					}
				}
			}
			// On success return empty, to indicate there is no error text.
			return msg;
		}

		void MiracastService::Deinitialize(PluginHost::IShell * /* service */)
		{
			MiracastService::_instance = nullptr;
			MIRACASTLOG_INFO("Entering..!!!");

			if (m_isServiceInitialized)
			{
				MiracastController::destroyInstance();
				DeinitializeIARM();
				m_CurrentService = nullptr;
				m_miracast_ctrler_obj = nullptr;
				m_isServiceInitialized = false;
				m_isServiceEnabled = false;
				MIRACASTLOG_INFO("Done..!!!");
			}
			MIRACASTLOG_INFO("Exiting..!!!");
		}

		string MiracastService::Information() const
		{
			return (string("{\"service\": \"") + SERVICE_NAME + string("\"}"));
		}

		/**
		 * @brief This method used to Enable/Disable the Miracast/WFD Discovery.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		uint32_t MiracastService::setEnableWrapper(const JsonObject &parameters, JsonObject &response)
		{
			bool success = false;
			bool is_enabled = true;

			MIRACASTLOG_INFO("Entering..!!!");
			if (parameters.HasLabel("enabled"))
			{
				getBoolParameter("enabled", is_enabled);

				lock_guard<mutex> lck(m_DiscoveryStateMutex);
				eMIRA_SERVICE_STATES current_state = getCurrentServiceState();
				if (true == is_enabled)
				{
					if (!m_isServiceEnabled)
					{
						lock_guard<recursive_mutex> lock(m_EventMutex);
						if (m_IsTransitionFromDeepSleep)
						{
							LOGINFO("#### MCAST-TRIAGE-OK Enable Miracast discovery Async");
							m_miracast_ctrler_obj->restart_discoveryAsync();
							m_IsTransitionFromDeepSleep = false;
						}
						else
						{
							setEnable(true);
						}
						m_isServiceEnabled = true;
						response["message"] = "Successfully enabled the WFD Discovery";
						success = true;
					}
					else
					{
						response["message"] = "WFD Discovery already enabled.";
					}
				}
				else
				{
					if ( MIRACAST_SERVICE_STATE_PLAYER_LAUNCHED == current_state )
					{
						response["message"] = "Failed as MiracastPlayer already Launched";
					}
					else if (m_isServiceEnabled)
					{
						lock_guard<recursive_mutex> lock(m_EventMutex);
						if (!m_IsTransitionFromDeepSleep)
						{
							if ( MIRACAST_SERVICE_STATE_RESTARTING_SESSION == current_state )
							{
								m_miracast_ctrler_obj->stop_discoveryAsync();
							}
							else
							{
								setEnable(false);
							}
						}
						else
						{
							LOGINFO("#### MCAST-TRIAGE-OK Skipping Disable discovery as done by PwrMgr");
						}
						m_isServiceEnabled = false;
						remove_wifi_connection_state_timer();
						remove_miracast_connection_timer();
						response["message"] = "Successfully disabled the WFD Discovery";
						success = true;
					}
					else
					{
						response["message"] = "WFD Discovery already disabled.";
					}
				}
			}
			else
			{
				response["message"] = "Invalid parameter passed";
			}

			returnResponse(success);
		}

		/**
		 * @brief This method used to get the current state of Miracast/WFD Discovery.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		uint32_t MiracastService::getEnable(const JsonObject &parameters, JsonObject &response)
		{
			MIRACASTLOG_INFO("Entering..!!!");
			response["enabled"] = m_isServiceEnabled;
			returnResponse(true);
		}

		/**
		 * @brief This method used to Enable/Disable the Miracast/WFD Discovery.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		void MiracastService::setEnable(bool isEnabled)
		{
			MIRACASTLOG_INFO("Entering..!!!");
			if ( nullptr != m_miracast_ctrler_obj )
			{
				m_miracast_ctrler_obj->set_enable(isEnabled);
			}
			MIRACASTLOG_INFO("Exiting..!!!");
		}

		/**
		 * @brief This method used to Enable/Disable the Miracast P2P Backend Discovery.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		uint32_t MiracastService::setP2PBackendDiscovery(const JsonObject &parameters, JsonObject &response)
		{
			bool success = false;
			bool is_enabled = true;

			MIRACASTLOG_INFO("Entering..!!!");
			if (parameters.HasLabel("enabled"))
			{
				getBoolParameter("enabled", is_enabled);
				m_miracast_ctrler_obj->setP2PBackendDiscovery(is_enabled);
				success = true;
			}
			else
			{
				response["message"] = "Invalid parameter passed";
			}
			returnResponse(success);
		}

		/**
		 * @brief This method used to Get the value of the given key from the environment (device properties file).
		 *
		 * @param: key and value reference pointer.
		 * @return Returns the true or false.
		 */
		bool MiracastService::envGetValue(const char *key, std::string &value)
		{
			std::ifstream fs(MIRACAST_DEVICE_PROPERTIES_FILE, std::ifstream::in);
			std::string::size_type delimpos;
			std::string line;
			if (!fs.fail())
			{
				while (std::getline(fs, line))
				{
					if (!line.empty() && ((delimpos = line.find('=')) > 0))
					{
						std::string itemKey = line.substr(0, delimpos);
						if (itemKey == key)
						{
							value = line.substr(delimpos + 1, std::string::npos);
							return true;
						}
					}
				}
			}
			return false;
		}

		/**
		 * @brief This method used to accept or reject the WFD connection request.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		uint32_t MiracastService::acceptClientConnection(const JsonObject &parameters, JsonObject &response)
		{
			bool success = false;
			std::string requestedStatus = "";

			MIRACASTLOG_INFO("Entering..!!!");
			if (parameters.HasLabel("requestStatus"))
			{
				requestedStatus = parameters["requestStatus"].String();
				if (("Accept" == requestedStatus) || ("Reject" == requestedStatus))
				{
					lock_guard<recursive_mutex> lock(m_EventMutex);
					eMIRA_SERVICE_STATES current_state = getCurrentServiceState();
					success = true;

					remove_miracast_connection_timer();

					if ( MIRACAST_SERVICE_STATE_DIRECT_LAUCH_WITH_CONNECTING == current_state)
					{
						if ("Accept" == requestedStatus)
						{
							MIRACASTLOG_INFO("#### Notifying Launch Request ####");
							m_miracast_ctrler_obj->switch_launch_request_context(m_src_dev_ip, m_src_dev_mac, m_src_dev_name, m_sink_dev_ip );
							changeServiceState(MIRACAST_SERVICE_STATE_CONNECTION_ACCEPTED);
						}
						else
						{
							changeServiceState(MIRACAST_SERVICE_STATE_CONNECTION_REJECTED);
							m_miracast_ctrler_obj->restart_session_discovery(m_src_dev_mac);
							m_miracast_ctrler_obj->m_ePlayer_state = MIRACAST_PLAYER_STATE_IDLE;
							changeServiceState(MIRACAST_SERVICE_STATE_RESTARTING_SESSION);
							MIRACASTLOG_INFO("#### Refreshing the Session ####");
						}
						m_src_dev_ip.clear();
						m_src_dev_mac.clear();
						m_src_dev_name.clear();
						m_sink_dev_ip.clear();
					}
					else
					{
						if ( MIRACAST_SERVICE_STATE_CONNECTING == current_state )
						{
							m_miracast_ctrler_obj->accept_client_connection(requestedStatus);
							if ("Accept" == requestedStatus)
							{
								changeServiceState(MIRACAST_SERVICE_STATE_CONNECTION_ACCEPTED);
							}
							else
							{
								changeServiceState(MIRACAST_SERVICE_STATE_CONNECTION_REJECTED);
							}
						}
						else
						{
							MIRACASTLOG_INFO("Ignoring '%s' as Session already Refreshed and Current State[%#08X]",requestedStatus.c_str(),current_state);
						}
					}
				}
				else
				{
					response["message"] = "Supported 'requestStatus' parameter values are Accept or Reject";
					LOGERR("Unsupported param passed [%s]", requestedStatus.c_str());
				}
			}
			else
			{
				LOGERR("Invalid parameter passed");
				response["message"] = "Invalid parameter passed";
			}

			returnResponse(success);
		}

		uint32_t MiracastService::stopClientConnection(const JsonObject &parameters, JsonObject &response)
		{
			bool success = false;
			MIRACASTLOG_INFO("Entering..!!!");

			returnIfStringParamNotFound(parameters, "name");
			returnIfStringParamNotFound(parameters, "mac");

			lock_guard<recursive_mutex> lock(m_EventMutex);
			eMIRA_SERVICE_STATES current_state = getCurrentServiceState();

			if ( MIRACAST_SERVICE_STATE_CONNECTION_ACCEPTED != current_state )
			{
				MIRACASTLOG_WARNING("stopClientConnection Already Received..!!!");
				response["message"] = "stopClientConnection Already Received";
			}
			else
			{
				std::string name,mac;

				getStringParameter("name", name);
				getStringParameter("mac", mac);

				if ((( 0 == name.compare(m_miracast_ctrler_obj->get_WFDSourceName())) &&
					( 0 == mac.compare(m_miracast_ctrler_obj->get_WFDSourceMACAddress())))||
					(( 0 == name.compare(m_miracast_ctrler_obj->get_NewSourceName())) &&
					( 0 == mac.compare(m_miracast_ctrler_obj->get_NewSourceMACAddress()))))
				{
					std::string cached_mac_address = "";
					if ( 0 == mac.compare(m_miracast_ctrler_obj->get_NewSourceMACAddress()))
					{
						cached_mac_address = mac;
					}

					if ( MIRACAST_SERVICE_STATE_PLAYER_LAUNCHED != current_state )
					{
						changeServiceState(MIRACAST_SERVICE_STATE_APP_REQ_TO_ABORT_CONNECTION);
						m_miracast_ctrler_obj->restart_session_discovery(cached_mac_address);
						changeServiceState(MIRACAST_SERVICE_STATE_RESTARTING_SESSION);
						success = true;
					}
					else
					{
						response["message"] = "stopClientConnection received after Launch";
						LOGERR("stopClientConnection received after Launch..!!!");
					}
				}
				else
				{
					response["message"] = "Invalid MAC and Name";
					LOGERR("Invalid MAC and Name[%s][%s]..!!!",mac.c_str(),name.c_str());
				}
			}
			MIRACASTLOG_INFO("Exiting..!!!");
			returnResponse(success);
		}

		/**
		 * @brief This method used to update the Player State for MiracastService Plugin.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		uint32_t MiracastService::updatePlayerState(const JsonObject &parameters, JsonObject &response)
		{
			string	mac;
			string	player_state;
			bool	success = true,
					restart_discovery_needed = false;

			MIRACASTLOG_INFO("Entering..!!!");

			returnIfStringParamNotFound(parameters, "mac");
			returnIfStringParamNotFound(parameters, "state");
			lock_guard<recursive_mutex> lock(m_EventMutex);
			if (parameters.HasLabel("mac"))
			{
				getStringParameter("mac", mac);
			}

			if (parameters.HasLabel("state"))
			{
				getStringParameter("state", player_state);
				if (player_state == "PLAYING" || player_state == "playing")
				{
					m_miracast_ctrler_obj->m_ePlayer_state = MIRACAST_PLAYER_STATE_PLAYING;
				}
				else if (player_state == "STOPPED" || player_state == "stopped")
				{
					int64_t json_parsed_value;
					eM_PLAYER_REASON_CODE stop_reason_code;

					getNumberParameter("reason_code", json_parsed_value);
					stop_reason_code = static_cast<eM_PLAYER_REASON_CODE>(json_parsed_value);

					if ( MIRACAST_PLAYER_REASON_CODE_NEW_SRC_DEV_CONNECT_REQ == stop_reason_code )
					{
						MIRACASTLOG_INFO("!!! STOPPED RECEIVED FOR NEW CONECTION !!!");
						m_miracast_ctrler_obj->flush_current_session();
					}
					else
					{
						restart_discovery_needed = true;
						if ( MIRACAST_PLAYER_REASON_CODE_APP_REQ_TO_STOP == stop_reason_code )
						{
							MIRACASTLOG_INFO("!!! STOPPED RECEIVED FOR ON EXIT !!!");
						}
						else if ( MIRACAST_PLAYER_REASON_CODE_SRC_DEV_REQ_TO_STOP == stop_reason_code )
						{
							MIRACASTLOG_INFO("!!! SRC DEV TEARDOWN THE CONNECTION !!!");
						}
						else
						{
							MIRACASTLOG_ERROR("!!! STOPPED RECEIVED FOR REASON[%#04X] !!!",stop_reason_code);
						}
					}
					m_miracast_ctrler_obj->m_ePlayer_state = MIRACAST_PLAYER_STATE_STOPPED;
				}
				else if (player_state == "INITIATED" || player_state == "initiated")
				{
					m_miracast_ctrler_obj->m_ePlayer_state = MIRACAST_PLAYER_STATE_INITIATED;
				}
				else if (player_state == "INPROGRESS" || player_state == "inprogress")
				{
					m_miracast_ctrler_obj->m_ePlayer_state = MIRACAST_PLAYER_STATE_INPROGRESS;
				}
				else
				{
					m_miracast_ctrler_obj->m_ePlayer_state = MIRACAST_PLAYER_STATE_IDLE;
				}
			}

			if ( m_isServiceEnabled && restart_discovery_needed )
			{
				// It will restart the discovering
				m_miracast_ctrler_obj->restart_session_discovery(mac);
				changeServiceState(MIRACAST_SERVICE_STATE_RESTARTING_SESSION);
			}

			MIRACASTLOG_INFO("Player State set to [%s (%d)] for Source device [%s].", player_state.c_str(), (int)m_miracast_ctrler_obj->m_ePlayer_state, mac.c_str());
			// @TODO: Need to check what to do next?

			MIRACASTLOG_INFO("Exiting..!!!");
			returnResponse(success);
		}

		uint32_t MiracastService::setLogging(const JsonObject &parameters, JsonObject &response)
		{
			std::string log_level = "";
			bool success = false;

			MIRACASTLOG_INFO("Entering..!!!");

			if (parameters.HasLabel("separate_logger"))
			{
				JsonObject separate_logger;
				separate_logger = parameters["separate_logger"].Object();

				if (separate_logger.HasLabel("status"))
				{
					std::string status = "";
					status = separate_logger["status"].String();

					success = true;

					if (status == "ENABLE" || status == "enable")
					{
						std::string logfilename = "";
						logfilename = separate_logger["logfilename"].String();
						MIRACAST::enable_separate_logger(logfilename);
					}
					else if (status == "DISABLE" || status == "disable")
					{
						MIRACAST::disable_separate_logger();
					}
					else
					{
						success = false;
					}
				}
			}

			if (parameters.HasLabel("level"))
			{
				LogLevel level = FATAL_LEVEL;
				getStringParameter("level", log_level);
				success = true;
				if (log_level == "FATAL" || log_level == "fatal")
				{
					level = FATAL_LEVEL;
				}
				else if (log_level == "ERROR" || log_level == "error")
				{
					level = ERROR_LEVEL;
				}
				else if (log_level == "WARNING" || log_level == "warning")
				{
					level = WARNING_LEVEL;
				}
				else if (log_level == "INFO" || log_level == "info")
				{
					level = INFO_LEVEL;
				}
				else if (log_level == "VERBOSE" || log_level == "verbose")
				{
					level = VERBOSE_LEVEL;
				}
				else if (log_level == "TRACE" || log_level == "trace")
				{
					level = TRACE_LEVEL;
				}
				else
				{
					success = false;
				}

				if (success)
				{
					set_loglevel(level);
				}
			}
			MIRACASTLOG_INFO("Exiting..!!!");
			returnResponse(success);
		}

#ifdef ENABLE_MIRACAST_SERVICE_TEST_NOTIFIER
		/**
		 * @brief This method used to stop the client connection.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		uint32_t MiracastService::testNotifier(const JsonObject &parameters, JsonObject &response)
		{
			MIRACAST_SERVICE_TEST_NOTIFIER_MSGQ_ST stMsgQ = {0};
			string client_mac,client_name,state;
			string status;
			bool success = false;

			MIRACASTLOG_INFO("Entering..!!!");

			if ( false == m_isTestNotifierEnabled )
			{
				if (parameters.HasLabel("setStatus"))
				{
					getStringParameter("setStatus", status);
					if (status == "ENABLED" || status == "enabled")
					{
						if ( MIRACAST_OK == m_miracast_ctrler_obj->create_TestNotifier())
						{
							m_isTestNotifierEnabled = true;
							success = true;
						}
						else
						{
							LOGERR("Failed to enable TestNotifier");
							response["message"] = "Failed to enable TestNotifier";
						}
					}
					else if (status == "DISABLED" || status == "disabled")
					{
						LOGERR("TestNotifier not yet enabled. Unable to Disable it");
						response["message"] = "TestNotifier not yet enabled. Unable to Disable";
					}
				}
				else
				{
					LOGERR("TestNotifier not yet enabled");
					response["message"] = "TestNotifier not yet enabled";
				}
			}
			else
			{
				if (parameters.HasLabel("setStatus"))
				{
					getStringParameter("setStatus", status);
					if (status == "DISABLED" || status == "disabled")
					{
						m_miracast_ctrler_obj->destroy_TestNotifier();
						success = true;
						m_isTestNotifierEnabled = false;
					}
					else if (status == "ENABLED" || status == "enabled")
					{
						LOGERR("TestNotifier already enabled");
						response["message"] = "TestNotifier already enabled";
						success = false;
					}
					else
					{
						LOGERR("Invalid status");
						response["message"] = "Invalid status";
					}
					return success;
				}

				returnIfStringParamNotFound(parameters, "state");
				returnIfStringParamNotFound(parameters, "mac");
				returnIfStringParamNotFound(parameters, "name");

				if (parameters.HasLabel("state"))
				{
					getStringParameter("state", state);
				}

				if (parameters.HasLabel("mac"))
				{
					getStringParameter("mac", client_mac);
				}

				if (parameters.HasLabel("name"))
				{
					getStringParameter("name", client_name);
				}

				if (client_mac.empty()||client_name.empty())
				{
					LOGERR("Invalid MAC/Name has passed");
					response["message"] = "Invalid MAC/Name has passed";
				}
				else
				{
					strncpy( stMsgQ.src_dev_name, client_name.c_str(), sizeof(stMsgQ.src_dev_name));
					stMsgQ.src_dev_name[sizeof(stMsgQ.src_dev_name) - 1] = '\0';
					strncpy( stMsgQ.src_dev_mac_addr, client_mac.c_str(), sizeof(stMsgQ.src_dev_mac_addr));
					stMsgQ.src_dev_mac_addr[sizeof(stMsgQ.src_dev_mac_addr) - 1] = '\0';

					MIRACASTLOG_INFO("Given [NAME-MAC-state] are[%s-%s-%s]",
							client_name.c_str(),
							client_mac.c_str(),
							state.c_str());

					success = true;

					if (state == "CONNECT_REQUEST" || state == "connect_request")
					{
						stMsgQ.state = MIRACAST_SERVICE_TEST_NOTIFIER_CLIENT_CONNECTION_REQUESTED;
					}
					else if (state == "CONNECT_ERROR" || state == "connect_error")
					{
						int64_t json_parsed_value;
						eMIRACAST_SERVICE_ERR_CODE error_code;

						returnIfNumberParamNotFound(parameters, "error_code");

						getNumberParameter("error_code", json_parsed_value);
						error_code = static_cast<eMIRACAST_SERVICE_ERR_CODE>(json_parsed_value);

						if (( MIRACAST_SERVICE_ERR_CODE_MAX_ERROR > error_code ) &&
							( MIRACAST_SERVICE_ERR_CODE_SUCCESS <= error_code ))
						{
							stMsgQ.state = MIRACAST_SERVICE_TEST_NOTIFIER_CLIENT_CONNECTION_ERROR;
							stMsgQ.error_code = error_code;
						}
						else
						{
							success = false;
							LOGERR("Invalid error_code passed");
							response["message"] = "Invalid error_code passed";
						}
					}
					else if (state == "LAUNCH" || state == "launch")
					{
						string source_dev_ip,sink_dev_ip;
						stMsgQ.state = MIRACAST_SERVICE_TEST_NOTIFIER_LAUNCH_REQUESTED;

						returnIfStringParamNotFound(parameters, "source_dev_ip");
						returnIfStringParamNotFound(parameters, "sink_dev_ip");

						if (parameters.HasLabel("source_dev_ip"))
						{
							getStringParameter("source_dev_ip", source_dev_ip);
						}

						if (parameters.HasLabel("sink_dev_ip"))
						{
							getStringParameter("sink_dev_ip", sink_dev_ip);
						}

						if (source_dev_ip.empty()||sink_dev_ip.empty())
						{
							LOGERR("Invalid source_dev_ip/sink_dev_ip has passed");
							response["message"] = "Invalid source_dev_ip/sink_dev_ip has passed";
							success = false;
						}
						else
						{
							strncpy( stMsgQ.src_dev_ip_addr, source_dev_ip.c_str(), sizeof(stMsgQ.src_dev_ip_addr));
							stMsgQ.src_dev_ip_addr[sizeof(stMsgQ.src_dev_ip_addr) - 1] = '\0';
							strncpy( stMsgQ.sink_ip_addr, sink_dev_ip.c_str(), sizeof(stMsgQ.sink_ip_addr));
							stMsgQ.sink_ip_addr[sizeof(stMsgQ.sink_ip_addr) - 1] = '\0';

							MIRACASTLOG_INFO("Given [Src-Sink-IP] are [%s-%s]",
									source_dev_ip.c_str(),
									sink_dev_ip.c_str());
						}
					}
					else
					{
						success = false;
						LOGERR("Invalid state passed");
						response["message"] = "Invalid state passed";
					}
					if (success)
					{
						m_miracast_ctrler_obj->send_msgto_test_notifier_thread(stMsgQ);
					}
				}
			}

			MIRACASTLOG_INFO("Exiting..!!!");

			returnResponse(success);
		}
#endif/*ENABLE_MIRACAST_SERVICE_TEST_NOTIFIER*/

		void MiracastService::onMiracastServiceClientConnectionRequest(string client_mac, string client_name)
		{
			std::string requestStatus = "Accept";
			bool is_another_connect_request = false;
			MIRACASTLOG_INFO("Entering..!!!");

			lock_guard<recursive_mutex> lock(m_EventMutex);
			eMIRA_SERVICE_STATES current_state = getCurrentServiceState();

			if ( MIRACAST_SERVICE_STATE_PLAYER_LAUNCHED == current_state )
			{
				is_another_connect_request = true;
				MIRACASTLOG_WARNING("Another Connect Request received while casting");
			}
			if ((MIRACAST_SERVICE_STATE_DIRECT_LAUCH_REQUESTED != current_state) &&
				((0 == access("/opt/miracast_autoconnect", F_OK))||
				 (0 == access("/opt/miracast_direct_request", F_OK))))
			{
				char commandBuffer[768] = {0};

				if ( is_another_connect_request )
				{
					MIRACASTLOG_INFO("!!! NEED TO STOP ONGOING SESSION !!!");
					strncpy(commandBuffer,"curl -H \"Authorization: Bearer `WPEFrameworkSecurityUtility | cut -d '\"' -f 4`\" --header \"Content-Type: application/json\" --request POST --data '{\"jsonrpc\":\"2.0\", \"id\":3,\"method\":\"org.rdk.MiracastPlayer.1.stopRequest\", \"params\":{\"reason\": \"NEW_CONNECTION\"}}' http://127.0.0.1:9998/jsonrpc",sizeof(commandBuffer));
					commandBuffer[sizeof(commandBuffer) - 1] = '\0';
					MIRACASTLOG_INFO("Stopping old Session by [%s]",commandBuffer);
					MiracastCommon::execute_SystemCommand(commandBuffer);
					sleep(1);
				}
				changeServiceState(MIRACAST_SERVICE_STATE_CONNECTING);
				memset(commandBuffer,0x00,sizeof(commandBuffer));
				snprintf( commandBuffer,
						sizeof(commandBuffer),
						"curl -H \"Authorization: Bearer `WPEFrameworkSecurityUtility | cut -d '\"' -f 4`\" --header \"Content-Type: application/json\" --request POST --data '{\"jsonrpc\":\"2.0\", \"id\":3,\"method\":\"org.rdk.MiracastService.1.acceptClientConnection\", \"params\":{\"requestStatus\": \"%s\"}}' http://127.0.0.1:9998/jsonrpc",
						requestStatus.c_str());
				MIRACASTLOG_INFO("AutoConnecting [%s - %s] by [%s]",client_name.c_str(),client_mac.c_str(),commandBuffer);
				MiracastCommon::execute_SystemCommand(commandBuffer);
				changeServiceState(MIRACAST_SERVICE_STATE_CONNECTION_ACCEPTED);
			}
			else
			{
				JsonObject params;
				params["mac"] = client_mac;
				params["name"] = client_name;
				sendNotify(EVT_ON_CLIENT_CONNECTION_REQUEST, params);
				m_src_dev_mac = client_mac;

				if (MIRACAST_SERVICE_STATE_DIRECT_LAUCH_REQUESTED == current_state)
				{
					changeServiceState(MIRACAST_SERVICE_STATE_DIRECT_LAUCH_WITH_CONNECTING);
				}
				else
				{
					changeServiceState(MIRACAST_SERVICE_STATE_CONNECTING);
				}
				m_MiracastConnectionMonitorTimerID = g_timeout_add(40000, MiracastService::monitor_miracast_connection_timercallback, this);
				MIRACASTLOG_INFO("Timer created to Monitor Miracast Connection Status [%u]",m_MiracastConnectionMonitorTimerID);
			}
		}

		void MiracastService::onMiracastServiceClientConnectionError(string client_mac, string client_name , eMIRACAST_SERVICE_ERR_CODE error_code )
		{
			MIRACASTLOG_INFO("Entering..!!!");

			lock_guard<recursive_mutex> lock(m_EventMutex);
			eMIRA_SERVICE_STATES current_state = getCurrentServiceState();

			if ( MIRACAST_SERVICE_STATE_CONNECTION_ACCEPTED != current_state )
			{
				MIRACASTLOG_INFO("Session already refreshed, So no need to report Error. Current state [%#08X]",current_state);
			}
			else
			{
				JsonObject params;
				params["mac"] = client_mac;
				params["name"] = client_name;
				params["error_code"] = std::to_string(error_code);
				params["reason"] = reasonDescription(error_code);
				sendNotify(EVT_ON_CLIENT_CONNECTION_ERROR, params);
			}
			MIRACASTLOG_INFO("Exiting..!!!");
		}

		std::string MiracastService::reasonDescription(eMIRACAST_SERVICE_ERR_CODE e)
		{
			switch (e)
			{
				case MIRACAST_SERVICE_ERR_CODE_SUCCESS:
					return "SUCCESS";
				case MIRACAST_SERVICE_ERR_CODE_P2P_GROUP_NEGO_ERROR:
					return "P2P GROUP NEGOTIATION FAILURE.";
				case MIRACAST_SERVICE_ERR_CODE_P2P_GROUP_FORMATION_ERROR:
					return "P2P GROUP FORMATION FAILURE.";
				case MIRACAST_SERVICE_ERR_CODE_GENERIC_FAILURE:
					return "P2P GENERIC FAILURE.";
				case MIRACAST_SERVICE_ERR_CODE_P2P_CONNECT_ERROR:
					return "P2P CONNECT FAILURE.";
				default:
					return "Unimplemented item";
			}
		}

		bool MiracastService::updateSystemFriendlyName()
		{
			JsonObject params, Result;
			bool return_value = false;
			MIRACASTLOG_INFO("Entering..!!!");

			getThunderPlugins();

			if (nullptr == m_SystemPluginObj)
			{
				LOGERR("m_SystemPluginObj not yet instantiated");
				return false;
			}

			uint32_t ret = m_SystemPluginObj->Invoke<JsonObject, JsonObject>(THUNDER_RPC_TIMEOUT, _T("getFriendlyName"), params, Result);

			if (Core::ERROR_NONE == ret)
			{
				if (Result["success"].Boolean())
				{
					std::string friendlyName = "";
					friendlyName = Result["friendlyName"].String();
					m_miracast_ctrler_obj->set_FriendlyName(friendlyName,m_isServiceEnabled);
					MIRACASTLOG_INFO("Miracast FriendlyName=%s", friendlyName.c_str());
					return_value = true;
				}
				else
				{
					ret = Core::ERROR_GENERAL;
					LOGERR("updateSystemFriendlyName call failed");
				}
			}
			else
			{
				LOGERR("updateSystemFriendlyName call failed E[%u]", ret);
			}
			return return_value;
		}

		void MiracastService::onFriendlyNameUpdateHandler(const JsonObject &parameters)
		{
			string message;
			string value;
			parameters.ToString(message);
			MIRACASTLOG_INFO("[Friendly Name Event], [%s]", message.c_str());

			if (parameters.HasLabel("friendlyName"))
			{
				value = parameters["friendlyName"].String();
				m_miracast_ctrler_obj->set_FriendlyName(value, m_isServiceEnabled);
				MIRACASTLOG_INFO("Miracast FriendlyName=%s", value.c_str());
			}
		}

		gboolean MiracastService::monitor_friendly_name_timercallback(gpointer userdata)
		{
			gboolean timer_retry_state = G_SOURCE_CONTINUE;
			MIRACASTLOG_TRACE("Entering..!!!");
			MiracastService *self = (MiracastService *)userdata;
			MIRACASTLOG_INFO("TimerCallback Triggered for updating friendlyName...");
			if ( true == self->updateSystemFriendlyName() )
			{
				MIRACASTLOG_INFO("friendlyName updated properly, No polling required...");
				timer_retry_state = G_SOURCE_REMOVE;
			}
			else
			{
				MIRACASTLOG_WARNING("Unable to get friendlyName, still requires polling...");
			}
			MIRACASTLOG_TRACE("Exiting..!!!");
			return timer_retry_state;
		}

		void MiracastService::setWiFiState(DEVICE_WIFI_STATES wifiState)
		{
			MIRACASTLOG_INFO("Miracast WiFi State=%#08X", wifiState);
			lock_guard<mutex> lck(m_DiscoveryStateMutex);
			if (m_isServiceEnabled)
			{
				switch(wifiState)
				{
					case DEVICE_WIFI_STATE_CONNECTING:
					{
						MIRACASTLOG_INFO("#### MCAST-TRIAGE-OK-WIFI DEVICE_WIFI_STATE [CONNECTING] ####");
						{lock_guard<recursive_mutex> lock(m_EventMutex);
							setEnable(false);
						}
						remove_wifi_connection_state_timer();
						m_IsWiFiConnectingState = true;
						m_WiFiConnectedStateMonitorTimerID = g_timeout_add(30000, MiracastService::monitor_wifi_connection_state_timercallback, this);
						MIRACASTLOG_INFO("Timer created to Monitor WiFi Connection Status [%u]",m_WiFiConnectedStateMonitorTimerID);
					}
					break;
					case DEVICE_WIFI_STATE_CONNECTED:
					case DEVICE_WIFI_STATE_FAILED:
					{
						MIRACASTLOG_INFO("#### MCAST-TRIAGE-OK-WIFI DEVICE_WIFI_STATE [%s] ####",( DEVICE_WIFI_STATE_CONNECTED == wifiState ) ? "CONNECTED" : "FAILED");
						if (m_IsWiFiConnectingState)
						{
							{lock_guard<recursive_mutex> lock(m_EventMutex);
								setEnable(true);
							}
							m_IsWiFiConnectingState = false;
						}
						remove_wifi_connection_state_timer();
					}
					break;
					default:
					{
						/* NOP */
					}
					break;
				}
			}
		}

	#ifdef UNIT_TESTING
		/**
		 * @brief This method used to get the current status of Miracast.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		uint32_t MiracastService::getStatus(const JsonObject &parameters, JsonObject &response)
		{
			MIRACASTLOG_INFO("Entering..!!!");
			lock_guard<mutex> lck(m_DiscoveryStateMutex);
			lock_guard<recursive_mutex> lock(m_EventMutex);
			response["enabled"] = m_isServiceEnabled;
			response["state"] = std::to_string(getCurrentServiceState());
			response["powerState"] = getPowerStateString(getCurrentPowerState());
			response["DeepSleepTransition"] = m_IsTransitionFromDeepSleep;
			response["wifiState"] = m_IsWiFiConnectingState;
			returnResponse(true);
		}

		/**
		 * @brief This method used to get the set Power status to Miracast.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		uint32_t MiracastService::setPowerStateWrapper(const JsonObject &parameters, JsonObject &response)
		{
			string message;
			uint32_t powerState;
			MIRACASTLOG_INFO("Entering..!!!");
			parameters.ToString(message);
			MIRACASTLOG_INFO("[Power State Changed Event], [%s]", message.c_str());

			if (parameters.HasLabel("state"))
			{
				powerState = parameters["state"].Number();
				setPowerState(static_cast<IARM_Bus_PWRMgr_PowerState_t>(powerState));
			}
			returnResponse(true);
		}

		/**
		 * @brief This method used to get the set WiFi status to Miracast.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		uint32_t MiracastService::setWiFiStateWrapper(const JsonObject &parameters, JsonObject &response)
		{
			string message;
			uint32_t wifiState;
			MIRACASTLOG_INFO("Entering..!!!");
			parameters.ToString(message);
			MIRACASTLOG_INFO("[WiFi State Changed Event], [%s]", message.c_str());

			if (parameters.HasLabel("state"))
			{
				wifiState = parameters["state"].Number();
				DEVICE_WIFI_STATES temp = static_cast<DEVICE_WIFI_STATES>(wifiState);
				MIRACASTLOG_INFO("[WiFi State Changed Event], [%#08X][%#08X]", wifiState,temp);
				setWiFiState(static_cast<DEVICE_WIFI_STATES>(wifiState));
			}
			returnResponse(true);
		}
	#endif /*UNIT_TESTING*/

		void MiracastService::onWIFIStateChangedHandler(const JsonObject &parameters)
		{
			string message;
			uint32_t wifiState;
			parameters.ToString(message);
			MIRACASTLOG_INFO("[WiFi State Changed Event], [%s]", message.c_str());

			if (parameters.HasLabel("state"))
			{
				wifiState = parameters["state"].Number();
				setWiFiState(static_cast<DEVICE_WIFI_STATES>(wifiState));
			}
		}

		gboolean MiracastService::monitor_wifi_connection_state_timercallback(gpointer userdata)
		{
			MIRACASTLOG_TRACE("Entering..!!!");
			MiracastService *self = (MiracastService *)userdata;
			MIRACASTLOG_INFO("TimerCallback Triggered for Monitor WiFi Connection Status...");
			{lock_guard<mutex> lck(self->m_DiscoveryStateMutex);
				MIRACASTLOG_INFO("#### MCAST-TRIAGE-OK-WIFI Discovery[%u] WiFiConnectingState[%u] ####",
									self->m_isServiceEnabled,m_IsWiFiConnectingState);
				if (self->m_isServiceEnabled && m_IsWiFiConnectingState)
				{
					{lock_guard<recursive_mutex> lock(self->m_EventMutex);
						self->setEnable(true);
					}
				}
				m_IsWiFiConnectingState = false;
			}
			MIRACASTLOG_TRACE("Exiting..!!!");
			return G_SOURCE_REMOVE;
		}

		gboolean MiracastService::monitor_miracast_connection_timercallback(gpointer userdata)
		{
			MiracastService *self = (MiracastService *)userdata;
			MIRACASTLOG_TRACE("Entering..!!!");
			lock_guard<recursive_mutex> lock(self->m_EventMutex);
			MIRACASTLOG_INFO("TimerCallback Triggered for Monitor Miracast Connection Expired and Restarting Session...");
			if (self->m_isServiceEnabled)
			{
				self->m_miracast_ctrler_obj->restart_session_discovery(self->m_src_dev_mac);
				self->m_src_dev_mac.clear();
				self->changeServiceState(MIRACAST_SERVICE_STATE_RESTARTING_SESSION);
			}
			MIRACASTLOG_TRACE("Exiting..!!!");
			return G_SOURCE_REMOVE;
		}

		void MiracastService::remove_wifi_connection_state_timer(void)
		{
			MIRACASTLOG_TRACE("Entering..!!!");
			if (m_WiFiConnectedStateMonitorTimerID)
			{
				MIRACASTLOG_INFO("Removing WiFi Connection Status Monitor Timer");
				g_source_remove(m_WiFiConnectedStateMonitorTimerID);
				m_WiFiConnectedStateMonitorTimerID = 0;
			}
			m_IsWiFiConnectingState = false;
			MIRACASTLOG_TRACE("Exiting..!!!");
		}

		void MiracastService::remove_miracast_connection_timer(void)
		{
			MIRACASTLOG_TRACE("Entering..!!!");
			if (m_MiracastConnectionMonitorTimerID)
			{
				MIRACASTLOG_INFO("Removing Miracast Connection Status Monitor Timer");
				g_source_remove(m_MiracastConnectionMonitorTimerID);
				m_MiracastConnectionMonitorTimerID = 0;
			}
			MIRACASTLOG_TRACE("Exiting..!!!");
		}

		void MiracastService::onMiracastServiceLaunchRequest(string src_dev_ip, string src_dev_mac, string src_dev_name, string sink_dev_ip, bool is_connect_req_reported )
		{
			lock_guard<recursive_mutex> lock(m_EventMutex);
			eMIRA_SERVICE_STATES current_state = getCurrentServiceState();
			MIRACASTLOG_INFO("Entering[%u]..!!!",is_connect_req_reported);

			if ( !is_connect_req_reported )
			{
				changeServiceState(MIRACAST_SERVICE_STATE_DIRECT_LAUCH_REQUESTED);
				m_src_dev_ip = src_dev_ip;
				m_src_dev_mac = src_dev_mac;
				m_src_dev_name = src_dev_name;
				m_sink_dev_ip = sink_dev_ip;
				MIRACASTLOG_INFO("Direct Launch request has received. So need to notify connect Request");
				onMiracastServiceClientConnectionRequest( src_dev_mac, src_dev_name );
			}
			else if ( MIRACAST_SERVICE_STATE_CONNECTION_ACCEPTED != current_state )
			{
				MIRACASTLOG_INFO("Session already refreshed, So no need to notify Launch Request. Current state [%#08X]",current_state);
				//m_miracast_ctrler_obj->restart_session_discovery();
			}
			else
			{
				JsonObject params;
				JsonObject device_params;
				device_params["source_dev_ip"] = src_dev_ip;
				device_params["source_dev_mac"] = src_dev_mac;
				device_params["source_dev_name"] = src_dev_name;
				device_params["sink_dev_ip"] = sink_dev_ip;
				params["device_parameters"] = device_params;

				if (0 == access("/opt/miracast_autoconnect", F_OK))
				{
					char commandBuffer[768] = {0};
					snprintf( commandBuffer,
							sizeof(commandBuffer),
							"curl -H \"Authorization: Bearer `WPEFrameworkSecurityUtility | cut -d '\"' -f 4`\" --header \"Content-Type: application/json\" --request POST --data '{\"jsonrpc\":\"2.0\", \"id\":3,\"method\":\"org.rdk.MiracastPlayer.1.playRequest\", \"params\":{\"device_parameters\": {\"source_dev_ip\": \"%s\",\"source_dev_mac\": \"%s\",\"source_dev_name\": \"%s\",\"sink_dev_ip\": \"%s\"},\"video_rectangle\": {\"X\": 0,\"Y\": 0,\"W\": 1280,\"H\": 720}}}' http://127.0.0.1:9998/jsonrpc",
							src_dev_ip.c_str(),
							src_dev_mac.c_str(),
							src_dev_name.c_str(),
							sink_dev_ip.c_str());
					MIRACASTLOG_INFO("System Command [%s]",commandBuffer);
					MiracastCommon::execute_SystemCommand(commandBuffer);
				}
				else
				{
					sendNotify(EVT_ON_LAUNCH_REQUEST, params);
				}
				changeServiceState(MIRACAST_SERVICE_STATE_PLAYER_LAUNCHED);
			}
		}

		void MiracastService::onStateChange(eMIRA_SERVICE_STATES state)
		{
			MIRACASTLOG_INFO("Entering state [%#08X]",state);
			lock_guard<recursive_mutex> lock(m_EventMutex);
			switch (state)
			{
				case MIRACAST_SERVICE_STATE_IDLE:
				case MIRACAST_SERVICE_STATE_DISCOVERABLE:
				{
					changeServiceState(state);
				}
				break;
				default:
				{

				}
				break;
			}
			MIRACASTLOG_INFO("Exiting...");
		}

		eMIRA_SERVICE_STATES MiracastService::getCurrentServiceState(void)
		{
			MIRACASTLOG_INFO("current state [%#08X]",m_eService_state);
			return m_eService_state;
		}

		void MiracastService::changeServiceState(eMIRA_SERVICE_STATES eService_state)
		{
			eMIRA_SERVICE_STATES old_state = m_eService_state,
								 new_state = eService_state;
			m_eService_state = eService_state;
			MIRACASTLOG_INFO("changing state [%#08X] -> [%#08X]",old_state,new_state);
		}

		std::string MiracastService::getPowerStateString(IARM_Bus_PWRMgr_PowerState_t pwrState)
		{
			std::string	pwrStateStr = "";

			switch (pwrState)
			{
				case IARM_BUS_PWRMGR_POWERSTATE_ON:
				{
					pwrStateStr = "ON";
				}
				break;
				case IARM_BUS_PWRMGR_POWERSTATE_OFF:
				{
					pwrStateStr = "OFF";
				}
				break;
				case IARM_BUS_PWRMGR_POWERSTATE_STANDBY:
				case IARM_BUS_PWRMGR_POWERSTATE_STANDBY_LIGHT_SLEEP:
				{
					pwrStateStr = "LIGHT_SLEEP";
				}
				break;
				case IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP:
				{
					pwrStateStr = "DEEP_SLEEP";
				}
				break;
				default:
				{
					pwrStateStr = "UNKNOWN";
				}
				break;
			}
			return pwrStateStr;
		}

		IARM_Bus_PWRMgr_PowerState_t MiracastService::getCurrentPowerState(void)
		{
			MIRACASTLOG_INFO("current power state [%s]",getPowerStateString(m_powerState).c_str());
			return m_powerState;
		}

		void MiracastService::setPowerState(IARM_Bus_PWRMgr_PowerState_t pwrState)
		{
			IARM_Bus_PWRMgr_PowerState_t old_pwr_state = m_powerState,
										 new_pwr_state = pwrState;
			m_powerState = pwrState;
			MIRACASTLOG_INFO("changing power state [%s] -> [%s]",
								getPowerStateString(old_pwr_state).c_str(),
								getPowerStateString(new_pwr_state).c_str());
			if (IARM_BUS_PWRMGR_POWERSTATE_ON == pwrState)
			{
				lock_guard<recursive_mutex> lock(_instance->m_EventMutex);
				if ((m_IsTransitionFromDeepSleep) && (_instance->m_isServiceEnabled))
				{
					LOGINFO("#### MCAST-TRIAGE-OK-PWR Enable Miracast discovery from PwrMgr [%d]",_instance->m_isServiceEnabled);
					_instance->m_miracast_ctrler_obj->restart_discoveryAsync();
					m_IsTransitionFromDeepSleep = false;
				}
				else if (!_instance->m_isServiceEnabled)
				{
					LOGINFO("#### MCAST-TRIAGE-OK-PWR Miracast discovery already Disabled [%d]. No need to enable it",_instance->m_isServiceEnabled);
				}
			}
			else if (IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP == pwrState)
			{
				lock_guard<recursive_mutex> lock(_instance->m_EventMutex);
				if ( _instance->m_isServiceEnabled )
				{
					LOGINFO("#### MCAST-TRIAGE-OK-PWR Miracast Discovery Disabled ####");
					_instance->setEnable(false);
				}
				else
				{
					LOGINFO("#### MCAST-TRIAGE-OK-PWR Miracast discovery already Disabled [%d]. No need to disable it",_instance->m_isServiceEnabled);
				}
				_instance->remove_wifi_connection_state_timer();
				_instance->remove_miracast_connection_timer();
				m_IsTransitionFromDeepSleep = true;
			}
		}
	} // namespace Plugin
} // namespace WPEFramework
