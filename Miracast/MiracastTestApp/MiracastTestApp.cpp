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
#include "rdk/iarmmgrs-hal/pwrMgr.h"
#include "MiracastTestApp.h"
#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"

const short WPEFramework::Plugin::MiracastTestApp::API_VERSION_NUMBER_MAJOR = 1;
const short WPEFramework::Plugin::MiracastTestApp::API_VERSION_NUMBER_MINOR = 0;
const string WPEFramework::Plugin::MiracastTestApp::SERVICE_NAME = "org.rdk.MiracastTestApp";
const string WPEFramework::Plugin::MiracastTestApp::METHOD_MIRACAST_SET_ENABLE = "setEnable";
const string WPEFramework::Plugin::MiracastTestApp::METHOD_MIRACAST_GET_ENABLE = "getEnable";
const string WPEFramework::Plugin::MiracastTestApp::METHOD_MIRACAST_CLIENT_CONNECT = "acceptClientConnection";
const string WPEFramework::Plugin::MiracastTestApp::METHOD_MIRACAST_STOP_CLIENT_CONNECT = "stopClientConnection";
const string WPEFramework::Plugin::MiracastTestApp::METHOD_MIRACAST_SET_VIDEO_FORMATS = "setVideoFormats";
const string WPEFramework::Plugin::MiracastTestApp::METHOD_MIRACAST_SET_AUDIO_FORMATS = "setAudioFormats";
#ifdef ENABLE_TEST_NOTIFIER
const string WPEFramework::Plugin::MiracastTestApp::METHOD_MIRACAST_TEST_NOTIFIER = "testNotifier";
#endif
using namespace std;

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

#define SERVER_DETAILS  "127.0.0.1:9998"
#define SYSTEM_CALLSIGN "org.rdk.System"
#define SYSTEM_CALLSIGN_VER SYSTEM_CALLSIGN".1"
#define SECURITY_TOKEN_LEN_MAX 1024
#define THUNDER_RPC_TIMEOUT 2000

#define EVT_ON_CLIENT_CONNECTION_REQUEST "onClientConnectionRequest"
#define EVT_ON_CLIENT_STOP_REQUEST "onClientStopRequest"
#define EVT_ON_CLIENT_CONNECTION_STARTED "onClientConnectionStarted"
#define EVT_ON_CLIENT_CONNECTION_ERROR "onClientConnectionError"

namespace WPEFramework
{
	namespace
	{
		static Plugin::Metadata<Plugin::MiracastTestApp> metadata(
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
		SERVICE_REGISTRATION(MiracastTestApp, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

		MiracastTestApp *MiracastTestApp::_instance = nullptr;
		MiracastController *MiracastTestApp::m_miracast_ctrler_obj = nullptr;

		MiracastTestApp::MiracastTestApp()
			: PluginHost::JSONRPC(),
			  m_isServiceInitialized(false),
			  m_isServiceEnabled(false)
		{
			LOGINFO("Entering..!!!");
			MiracastTestApp::_instance = this;

			Register(METHOD_MIRACAST_SET_ENABLE, &MiracastTestApp::setEnable, this);
			Register(METHOD_MIRACAST_GET_ENABLE, &MiracastTestApp::getEnable, this);
			Register(METHOD_MIRACAST_STOP_CLIENT_CONNECT, &MiracastTestApp::stopClientConnection, this);
			Register(METHOD_MIRACAST_CLIENT_CONNECT, &MiracastTestApp::acceptClientConnection, this);
			Register(METHOD_MIRACAST_SET_VIDEO_FORMATS, &MiracastTestApp::setVideoFormats, this);
			Register(METHOD_MIRACAST_SET_AUDIO_FORMATS, &MiracastTestApp::setAudioFormats, this);
#ifdef	ENABLE_TEST_NOTIFIER
			Register(METHOD_MIRACAST_TEST_NOTIFIER, &MiracastTestApp::testNotifier, this);
#endif
			MIRACAST::logger_init();
			LOGINFO("Exiting..!!!");
		}

		MiracastTestApp::~MiracastTestApp()
		{
			LOGINFO("Entering..!!!");
			if (nullptr != m_SystemPluginObj)
			{
				delete m_SystemPluginObj;
				m_SystemPluginObj = nullptr;
			}
			Unregister(METHOD_MIRACAST_SET_ENABLE);
			Unregister(METHOD_MIRACAST_GET_ENABLE);
			Unregister(METHOD_MIRACAST_STOP_CLIENT_CONNECT);
			Unregister(METHOD_MIRACAST_CLIENT_CONNECT);
			MIRACAST::logger_deinit();
			LOGINFO("Exiting..!!!");
		}

		// Thunder plugins communication
        void MiracastTestApp::getSystemPlugin()
        {
			LOGINFO("Entering..!!!");

            if(nullptr == m_SystemPluginObj)
            {
				string token;
                // TODO: use interfaces and remove token
                auto security = m_CurrentService->QueryInterfaceByCallsign<PluginHost::IAuthenticate>("SecurityAgent");
                if (nullptr != security)
				{
                    string payload = "http://localhost";
                    if (security->CreateToken( static_cast<uint16_t>(payload.length()),
											reinterpret_cast<const uint8_t*>(payload.c_str()),
                            				token) == Core::ERROR_NONE)
					{
						LOGINFO("got security token\n");
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
                m_SystemPluginObj = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(_T(SYSTEM_CALLSIGN_VER), (_T("MiracastTestApp")), false, query);
				if (nullptr == m_SystemPluginObj)
				{
					LOGERR("JSONRPC: %s: initialization failed", SYSTEM_CALLSIGN_VER);
				}
				else
				{
					LOGINFO("JSONRPC: %s: initialization ok", SYSTEM_CALLSIGN_VER);
				}
            }
			LOGINFO("Exiting..!!!");
        }

		const string MiracastTestApp::Initialize(PluginHost::IShell *service)
		{
			string msg;
			LOGINFO("Entering..!!!");
			if (!m_isServiceInitialized)
			{
				MiracastError ret_code = MIRACAST_OK;
				m_miracast_ctrler_obj = MiracastController::getInstance(ret_code,this);
				if ( nullptr != m_miracast_ctrler_obj ){
					m_CurrentService = service;
					getSystemPlugin();
					// subscribe for event
					m_SystemPluginObj->Subscribe<JsonObject>(1000, "onFriendlyNameChanged"
										, &MiracastTestApp::onFriendlyNameUpdateHandler, this);
					updateSystemFriendlyName();
					m_isServiceInitialized = true;
				}
				else{
					switch (ret_code){
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

		void MiracastTestApp::Deinitialize(PluginHost::IShell * /* service */)
		{
			MiracastTestApp::_instance = nullptr;
			LOGINFO("Entering..!!!");

			if (m_isServiceInitialized)
			{
				MiracastController::destroyInstance();
				m_CurrentService = nullptr;
				m_miracast_ctrler_obj = nullptr;
				m_isServiceInitialized = false;
				m_isServiceEnabled = false;
				LOGINFO("Done..!!!");
			}
			LOGINFO("Exiting..!!!");
		}

		string MiracastTestApp::Information() const
		{
			return (string("{\"service\": \"") + SERVICE_NAME + string("\"}"));
		}

		/**
		 * @brief This method used to Enable/Disable the Miracast/WFD Discovery.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		uint32_t MiracastTestApp::setEnable(const JsonObject &parameters, JsonObject &response)
		{
			bool success = false;
			bool is_enabled = true;

			LOGINFO("Entering..!!!");
			if (parameters.HasLabel("enabled"))
			{
				getBoolParameter("enabled", is_enabled);

				if ( true == is_enabled )
				{
					if (!m_isServiceEnabled)
					{
						m_miracast_ctrler_obj->set_enable(is_enabled);
						success = true;
						m_isServiceEnabled = is_enabled;
						response["message"] = "Successfully enabled the WFD Discovery";
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
		uint32_t MiracastTestApp::getEnable(const JsonObject &parameters, JsonObject &response)
		{
			LOGINFO("Entering..!!!");
			response["enabled"] = m_isServiceEnabled;
			returnResponse(true);
		}

		/**
		 * @brief This method used to accept or reject the WFD connection request.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		uint32_t MiracastTestApp::acceptClientConnection(const JsonObject &parameters, JsonObject &response)
		{
			bool success = false;
			std::string requestedStatus = "";

			LOGINFO("Entering..!!!");

			if (parameters.HasLabel("requestStatus"))
			{
				requestedStatus = parameters["requestStatus"].String();
				if (("Accept" == requestedStatus) || ("Reject" == requestedStatus))
				{
					m_miracast_ctrler_obj->accept_client_connection (requestedStatus);
					success = true;
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

		/**
		 * @brief This method used to stop the client connection.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		uint32_t MiracastTestApp::stopClientConnection(const JsonObject &parameters, JsonObject &response)
		{
			bool success = false;
			std::string mac_addr = "";

			LOGINFO("Entering..!!!");

			if (parameters.HasLabel("clientMac"))
			{
				mac_addr = parameters["clientMac"].String();
				const std::regex mac_regex("^([0-9a-f]{2}[:]){5}([0-9a-f]{2})$");

				if (true == std::regex_match(mac_addr, mac_regex))
				{
					if (true == m_miracast_ctrler_obj->stop_client_connection(mac_addr))
					{
						success = true;
						response["message"] = "Successfully Initiated the Stop WFD Client Connection";
						LOGINFO("Successfully Initiated the Stop WFD Client Connection");
					}
					else
					{
						LOGERR("MAC Address[%s] not connected yet",mac_addr.c_str());
						response["message"] = "MAC Address not connected yet.";
					}
				}
				else
				{
					LOGERR("Invalid MAC Address[%s] passed",mac_addr.c_str());
					response["message"] = "Invalid MAC Address";
				}
			}
			else
			{
				LOGERR("Invalid parameter passed");
				response["message"] = "Invalid parameter passed";
			}

			returnResponse(success);
		}

		/**
		 * @brief This method used to set the videoformats for Miracast.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		uint32_t MiracastTestApp::setVideoFormats(const JsonObject &parameters, JsonObject &response)
		{
			JsonArray h264_codecs;
			RTSP_WFD_VIDEO_FMT_STRUCT st_video_fmt = {0};
			bool success = false;

			LOGINFO("Entering..!!!");

			returnIfParamNotFound(parameters, "native");
			returnIfBooleanParamNotFound(parameters, "display_mode_supported");
			returnIfParamNotFound(parameters, "h264_codecs");

			h264_codecs = parameters["h264_codecs"].Array();
			if (0 == h264_codecs.Length())
            {
                LOGWARN("Got empty list of h264_codecs");
                returnResponse(false);
            }
			getNumberParameter("native", st_video_fmt.native);
			getBoolParameter("display_mode_supported", st_video_fmt.preferred_display_mode_supported);

			JsonArray::Iterator index(h264_codecs.Elements());

            while (index.Next() == true)
            {
                if (Core::JSON::Variant::type::OBJECT == index.Current().Content())
                {
                    JsonObject codecs = index.Current().Object();

					returnIfParamNotFound(codecs, "profile");
					returnIfParamNotFound(codecs, "level");
					returnIfParamNotFound(codecs, "cea_mask");
					returnIfParamNotFound(codecs, "vesa_mask");
					returnIfParamNotFound(codecs, "hh_mask");

					getNumberParameterObject(codecs,"profile",st_video_fmt.st_h264_codecs.profile);
					getNumberParameterObject(codecs,"level",st_video_fmt.st_h264_codecs.level);
					getNumberParameterObject(codecs,"cea_mask",st_video_fmt.st_h264_codecs.cea_mask);
					getNumberParameterObject(codecs,"vesa_mask",st_video_fmt.st_h264_codecs.vesa_mask);
					getNumberParameterObject(codecs,"hh_mask",st_video_fmt.st_h264_codecs.hh_mask);
					getNumberParameterObject(codecs,"latency",st_video_fmt.st_h264_codecs.latency);
					getNumberParameterObject(codecs,"min_slice",st_video_fmt.st_h264_codecs.min_slice);
					getNumberParameterObject(codecs,"slice_encode",st_video_fmt.st_h264_codecs.slice_encode);

					if (codecs.HasLabel("video_frame_skip_support")){
						bool video_frame_skip_support;
						video_frame_skip_support = codecs["video_frame_skip_support"].Boolean();
						st_video_fmt.st_h264_codecs.video_frame_skip_support = video_frame_skip_support;
					}

					if (codecs.HasLabel("max_skip_intervals")){
						uint8_t max_skip_intervals;
						getNumberParameterObject(codecs,"max_skip_intervals",max_skip_intervals);
						st_video_fmt.st_h264_codecs.max_skip_intervals = max_skip_intervals;
					}

					if (codecs.HasLabel("video_frame_rate_change_support")){
						bool video_frame_rate_change_support;
						video_frame_rate_change_support = codecs["video_frame_rate_change_support"].Boolean();
						st_video_fmt.st_h264_codecs.video_frame_rate_change_support = video_frame_rate_change_support;
					}
                }
                else
                    LOGWARN("Unexpected variant type");
            }
			success = m_miracast_ctrler_obj->set_WFDVideoFormat(st_video_fmt);

			LOGINFO("Exiting..!!!");
			returnResponse(success);
		}

		/**
		 * @brief This method used to set the audioformats for Miracast.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		uint32_t MiracastTestApp::setAudioFormats(const JsonObject &parameters, JsonObject &response)
		{
			JsonArray audio_codecs;
			RTSP_WFD_AUDIO_FMT_STRUCT st_audio_fmt = {0};
			bool success = false;

			LOGINFO("Entering..!!!");

			returnIfParamNotFound(parameters, "audio_codecs");

			audio_codecs = parameters["audio_codecs"].Array();
			if (0 == audio_codecs.Length())
            {
                LOGWARN("Got empty list of audio_codecs");
                returnResponse(false);
            }

			JsonArray::Iterator index(audio_codecs.Elements());

            while (index.Next() == true)
            {
                if (Core::JSON::Variant::type::OBJECT == index.Current().Content())
                {
                    JsonObject codecs = index.Current().Object();

					returnIfParamNotFound(codecs, "audio_format");
					returnIfParamNotFound(codecs, "modes");
					returnIfParamNotFound(codecs, "latency");

					getNumberParameterObject(codecs,"audio_format",st_audio_fmt.audio_format);
					getNumberParameterObject(codecs,"modes",st_audio_fmt.modes);
					getNumberParameterObject(codecs,"latency",st_audio_fmt.latency);
                }
                else
                    LOGWARN("Unexpected variant type");
            }
			success = m_miracast_ctrler_obj->set_WFDAudioCodecs(st_audio_fmt);

			LOGINFO("Exiting..!!!");
			returnResponse(success);
		}
#ifdef ENABLE_TEST_NOTIFIER
		/**
		 * @brief This method used to stop the client connection.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		uint32_t MiracastTestApp::testNotifier(const JsonObject &parameters, JsonObject &response)
		{
			bool success = false;
			uint32_t state;

			LOGINFO("Entering..!!!");

			if (parameters.HasLabel("state"))
			{
				getNumberParameter("state", state);

				if ((TEST_NOTIFIER_INVALID_STATE < state ) &&
					( TEST_NOTIFIER_SHUTDOWN > state )){
					m_miracast_ctrler_obj->send_msgto_test_notifier_thread(state);
					success = true;
				}
				else{
					LOGERR("Invalid state passed");
					response["message"] = "Invalid state passed";
				}
			}
			else
			{
				LOGERR("Invalid parameter passed");
				response["message"] = "Invalid parameter passed";
			}
			LOGINFO("Exiting..!!!");

			returnResponse(success);
		}
#endif/*ENABLE_TEST_NOTIFIER*/

		void MiracastTestApp::onMiracastTestAppClientConnectionRequest(string client_mac, string client_name)
		{
			LOGINFO("Entering..!!!");

			JsonObject params;
			params["clientMac"] = client_mac;
			params["clientName"] = client_name;
			sendNotify(EVT_ON_CLIENT_CONNECTION_REQUEST, params);
		}

		void MiracastTestApp::onMiracastTestAppClientStopRequest(string client_mac, string client_name)
		{
			LOGINFO("Entering..!!!");

			JsonObject params;
			params["clientMac"] = client_mac;
			params["clientName"] = client_name;
			sendNotify(EVT_ON_CLIENT_STOP_REQUEST, params);
		}

		void MiracastTestApp::onMiracastTestAppClientConnectionStarted(string client_mac, string client_name)
		{
			LOGINFO("Entering..!!!");

			JsonObject params;
			params["clientMac"] = client_mac;
			params["clientName"] = client_name;
			sendNotify(EVT_ON_CLIENT_CONNECTION_STARTED, params);
		}

		void MiracastTestApp::onMiracastTestAppClientConnectionError(string client_mac, string client_name)
		{
			LOGINFO("Entering..!!!");

			JsonObject params;
			params["clientMac"] = client_mac;
			params["clientName"] = client_name;
			sendNotify(EVT_ON_CLIENT_CONNECTION_ERROR, params);
		}

		int MiracastTestApp::updateSystemFriendlyName()
		{
			JsonObject params, Result;
			LOGINFO("Entering..!!!");

			if (nullptr == m_SystemPluginObj)
			{
				LOGERR("m_SystemPluginObj not yet instantiated");
				return Core::ERROR_GENERAL;
			}

			uint32_t ret = m_SystemPluginObj->Invoke<JsonObject, JsonObject>(THUNDER_RPC_TIMEOUT, _T("getFriendlyName"), params, Result);

			if (Core::ERROR_NONE == ret)
			{
				if (Result["success"].Boolean())
				{
					std::string friendlyName = "";
					friendlyName = Result["friendlyName"].String();
					m_miracast_ctrler_obj->set_FriendlyName(friendlyName);
					LOGINFO("Miracast FriendlyName=%s", friendlyName.c_str());
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
			return ret;
		}

		void MiracastTestApp::onFriendlyNameUpdateHandler(const JsonObject& parameters)
		{
			string message;
			string value;
			parameters.ToString(message);
			LOGINFO("[Friendly Name Event], [%s]", message.c_str());

			if (parameters.HasLabel("friendlyName")) {
				value = parameters["friendlyName"].String();
				m_miracast_ctrler_obj->set_FriendlyName(value,m_isServiceEnabled);
				LOGINFO("Miracast FriendlyName=%s", value.c_str());
			}
		}
	} // namespace Plugin
} // namespace WPEFramework