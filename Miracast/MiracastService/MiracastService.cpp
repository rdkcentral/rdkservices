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

#ifdef ENABLE_MIRACAST_SERVICE_TEST_NOTIFIER
const string WPEFramework::Plugin::MiracastService::METHOD_MIRACAST_TEST_NOTIFIER = "testNotifier";
#endif

using namespace std;

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 4

#define SERVER_DETAILS "127.0.0.1:9998"
#define SYSTEM_CALLSIGN "org.rdk.System"
#define SYSTEM_CALLSIGN_VER SYSTEM_CALLSIGN ".1"
#define SECURITY_TOKEN_LEN_MAX 1024
#define THUNDER_RPC_TIMEOUT 2000

#define EVT_ON_CLIENT_CONNECTION_REQUEST       "onClientConnectionRequest"
#define EVT_ON_CLIENT_CONNECTION_ERROR         "onClientConnectionError"
#define EVT_ON_LAUNCH_REQUEST                  "onLaunchRequest"

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
			  m_isServiceEnabled(false)
		{
			LOGINFO("Entering..!!!");
			MiracastService::_instance = this;
			MIRACAST::logger_init("MiracastService");

			Register(METHOD_MIRACAST_SET_ENABLE, &MiracastService::setEnable, this);
			Register(METHOD_MIRACAST_GET_ENABLE, &MiracastService::getEnable, this);
			Register(METHOD_MIRACAST_SET_P2P_BACKEND_DISCOVERY, &MiracastService::setP2PBackendDiscovery, this);
			Register(METHOD_MIRACAST_CLIENT_CONNECT, &MiracastService::acceptClientConnection, this);
			Register(METHOD_MIRACAST_STOP_CLIENT_CONNECT, &MiracastService::stopClientConnection, this);
			Register(METHOD_MIRACAST_SET_UPDATE_PLAYER_STATE, &MiracastService::updatePlayerState, this);
			Register(METHOD_MIRACAST_SET_LOG_LEVEL, &MiracastService::setLogging, this);

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
			if (m_FriendlyNameMonitorTimerID)
			{
				g_source_remove(m_FriendlyNameMonitorTimerID);
				m_FriendlyNameMonitorTimerID = 0;
			}
			Unregister(METHOD_MIRACAST_SET_ENABLE);
			Unregister(METHOD_MIRACAST_GET_ENABLE);
			Unregister(METHOD_MIRACAST_STOP_CLIENT_CONNECT);
			Unregister(METHOD_MIRACAST_CLIENT_CONNECT);
			Unregister(METHOD_MIRACAST_SET_UPDATE_PLAYER_STATE);
			Unregister(METHOD_MIRACAST_SET_LOG_LEVEL);
			MIRACAST::logger_deinit();
			LOGINFO("Exiting..!!!");
		}

		// Thunder plugins communication
		void MiracastService::getSystemPlugin()
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
					m_CurrentService = service;
					getSystemPlugin();
					// subscribe for event
					m_SystemPluginObj->Subscribe<JsonObject>(1000, "onFriendlyNameChanged", &MiracastService::onFriendlyNameUpdateHandler, this);
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
		uint32_t MiracastService::setEnable(const JsonObject &parameters, JsonObject &response)
		{
			bool success = false;
			bool is_enabled = true;

			MIRACASTLOG_INFO("Entering..!!!");
			if (parameters.HasLabel("enabled"))
			{
				getBoolParameter("enabled", is_enabled);

				if (true == is_enabled)
				{
					if (!m_isServiceEnabled)
					{
						m_miracast_ctrler_obj->set_enable(is_enabled);
						success = true;
						m_isServiceEnabled = is_enabled;
						response["message"] = "Successfully enabled the WFD Discovery";
						m_eService_state = MIRACAST_SERVICE_STATE_DISCOVERABLE;
					}
					else
					{
						response["message"] = "WFD Discovery already enabled.";
					}
				}
				else
				{
					if (m_isServiceEnabled)
					{
						m_miracast_ctrler_obj->set_enable(is_enabled);
						success = true;
						m_isServiceEnabled = is_enabled;
						response["message"] = "Successfully disabled the WFD Discovery";
						m_eService_state = MIRACAST_SERVICE_STATE_IDLE;
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
					m_miracast_ctrler_obj->accept_client_connection(requestedStatus);
					success = true;
					m_eService_state = MIRACAST_SERVICE_STATE_CONNECTION_ACCEPTED;
				}
				else
				{
					response["message"] = "Supported 'requestStatus' parameter values are Accept or Reject";
					LOGERR("Unsupported param passed [%s].\n", requestedStatus.c_str());
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

			if ( MIRACAST_SERVICE_STATE_APP_REQ_TO_ABORT_CONNECTION == m_eService_state )
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

					if ( MIRACAST_SERVICE_STATE_PLAYER_LAUNCHED != m_eService_state )
					{
						m_eService_state = MIRACAST_SERVICE_STATE_APP_REQ_TO_ABORT_CONNECTION;
						m_miracast_ctrler_obj->restart_session_discovery(cached_mac_address);
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
					m_eService_state = MIRACAST_SERVICE_STATE_DISCOVERABLE;
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
				m_miracast_ctrler_obj->restart_session_discovery();
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
					status = separate_logger["separate_logger"].String();

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
					strcpy( stMsgQ.src_dev_name, client_name.c_str());
					strcpy( stMsgQ.src_dev_mac_addr, client_mac.c_str());

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
							strcpy( stMsgQ.src_dev_ip_addr, source_dev_ip.c_str());
							strcpy( stMsgQ.sink_ip_addr, sink_dev_ip.c_str());

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

			if ( MIRACAST_SERVICE_STATE_PLAYER_LAUNCHED == m_eService_state )
			{
				is_another_connect_request = true;
				MIRACASTLOG_WARNING("Another Connect Request received while casting\n");
			}

			if (0 == access("/opt/miracast_autoconnect", F_OK))
			{
				std::string system_command = "";

				if ( is_another_connect_request )
				{
					MIRACASTLOG_INFO("!!! NEED TO STOP ONGOING SESSION !!!\n");

					system_command = "curl -H \"Authorization: Bearer `WPEFrameworkSecurityUtility | cut -d '\"' -f 4`\"";
					system_command.append(" --header \"Content-Type: application/json\" --request POST --data '{\"jsonrpc\":\"2.0\", \"id\":3,\"method\":\"org.rdk.MiracastPlayer.1.stopRequest\", \"params\":{");
					system_command.append("\"reason\": \"NEW_CONNECTION\"");
					system_command.append("}}' http://127.0.0.1:9998/jsonrpc\n");
					MIRACASTLOG_INFO("Stopping old Session by [%s]\n",system_command.c_str());
					system( system_command.c_str());
					sleep(1);
				}
				system_command = "curl -H \"Authorization: Bearer `WPEFrameworkSecurityUtility | cut -d '\"' -f 4`\"";
				system_command.append(" --header \"Content-Type: application/json\" --request POST --data '{\"jsonrpc\":\"2.0\", \"id\":3,\"method\":\"org.rdk.MiracastService.1.acceptClientConnection\", \"params\":{");
				system_command.append("\"requestStatus\": \"");
				system_command.append(requestStatus);
				system_command.append("\"}}' http://127.0.0.1:9998/jsonrpc\n");
				MIRACASTLOG_INFO("AutoConnecting [%s - %s] by [%s]\n",
									client_name.c_str(),
									client_mac.c_str(),
									system_command.c_str());
				system( system_command.c_str());
            }
			else
			{
				JsonObject params;
				params["mac"] = client_mac;
				params["name"] = client_name;
				sendNotify(EVT_ON_CLIENT_CONNECTION_REQUEST, params);
			}
		}

		void MiracastService::onMiracastServiceClientConnectionError(string client_mac, string client_name , eMIRACAST_SERVICE_ERR_CODE error_code )
		{
			MIRACASTLOG_INFO("Entering..!!!");

			if ( MIRACAST_SERVICE_STATE_APP_REQ_TO_ABORT_CONNECTION != m_eService_state )
			{
				JsonObject params;
				params["mac"] = client_mac;
				params["name"] = client_name;
				params["error_code"] = std::to_string(error_code);
				params["reason"] = reasonDescription(error_code);
				sendNotify(EVT_ON_CLIENT_CONNECTION_ERROR, params);
				m_eService_state = MIRACAST_SERVICE_STATE_DISCOVERABLE;
			}
			else
			{
				MIRACASTLOG_INFO("APP_REQ_TO_ABORT_CONNECTION has requested. So no need to notify..!!!");
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

			getSystemPlugin();

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

		void MiracastService::onMiracastServiceLaunchRequest(string src_dev_ip, string src_dev_mac, string src_dev_name, string sink_dev_ip)
		{
			MIRACASTLOG_INFO("Entering..!!!");

			if ( MIRACAST_SERVICE_STATE_APP_REQ_TO_ABORT_CONNECTION == m_eService_state )
			{
				MIRACASTLOG_INFO("APP_REQ_TO_ABORT_CONNECTION has requested. So no need to notify Launch Request..!!!");
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
					std::string system_command = "";
					system_command = "curl -H \"Authorization: Bearer `WPEFrameworkSecurityUtility | cut -d '\"' -f 4`\"";
					
					system_command.append(" --header \"Content-Type: application/json\" --request POST --data '{\"jsonrpc\":\"2.0\", \"id\":3,\"method\":\"org.rdk.MiracastPlayer.1.playRequest\", \"params\":{");
					
					system_command.append("\"device_parameters\": {\n");

					system_command.append("\"source_dev_ip\": \"");
					system_command.append(src_dev_ip);
					system_command.append("\",\n");

					system_command.append("\"source_dev_mac\": \"");
					system_command.append(src_dev_mac);
					system_command.append("\",\n");

					system_command.append("\"source_dev_name\": \"");
					system_command.append(src_dev_name);
					system_command.append("\",\n");

					system_command.append("\"sink_dev_ip\": \"");
					system_command.append(sink_dev_ip);
					system_command.append("\"\n},\n");

					system_command.append("\"video_rectangle\": {\n");

					system_command.append("\"X\": ");
					system_command.append("0");
					system_command.append(",\n");

					system_command.append("\"Y\": ");
					system_command.append("0");
					system_command.append(",\n");

					system_command.append("\"W\": ");
					system_command.append("1920");
					system_command.append(",\n");

					system_command.append("\"H\": ");
					system_command.append("1080");

					system_command.append("}}}' http://127.0.0.1:9998/jsonrpc\n");

					MIRACASTLOG_INFO("System Command [%s]\n",system_command.c_str());
					system( system_command.c_str());
				}
				else
				{
					sendNotify(EVT_ON_LAUNCH_REQUEST, params);
					m_eService_state = MIRACAST_SERVICE_STATE_PLAYER_LAUNCHED;
				}
			}
		}
	} // namespace Plugin
} // namespace WPEFramework
