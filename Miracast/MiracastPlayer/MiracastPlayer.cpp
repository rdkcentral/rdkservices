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
#include "MiracastPlayer.h"
#include <UtilsJsonRpc.h>
#include "UtilsIarm.h"

const short WPEFramework::Plugin::MiracastPlayer::API_VERSION_NUMBER_MAJOR = 1;
const short WPEFramework::Plugin::MiracastPlayer::API_VERSION_NUMBER_MINOR = 0;
const string WPEFramework::Plugin::MiracastPlayer::SERVICE_NAME = "org.rdk.MiracastPlayer";
using namespace std;

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

#define SERVER_DETAILS "127.0.0.1:9998"
#define SYSTEM_CALLSIGN "org.rdk.System"
#define SYSTEM_CALLSIGN_VER SYSTEM_CALLSIGN ".1"
#define SECURITY_TOKEN_LEN_MAX 1024
#define THUNDER_RPC_TIMEOUT 2000

/*Methods*/
const string WPEFramework::Plugin::MiracastPlayer::METHOD_MIRACAST_PLAYER_PLAY_REQUEST = "playRequest";
const string WPEFramework::Plugin::MiracastPlayer::METHOD_MIRACAST_PLAYER_STOP_REQUEST = "stopRequest";
const string WPEFramework::Plugin::MiracastPlayer::METHOD_MIRACAST_PLAYER_SET_PLAYER_STATE = "setPlayerState";
const string WPEFramework::Plugin::MiracastPlayer::METHOD_MIRACAST_PLAYER_SET_VIDEO_RECTANGLE = "setVideoRectangle";
const string WPEFramework::Plugin::MiracastPlayer::METHOD_MIRACAST_SET_VIDEO_FORMATS = "setVideoFormats";
const string WPEFramework::Plugin::MiracastPlayer::METHOD_MIRACAST_SET_AUDIO_FORMATS = "setAudioFormats";
const string WPEFramework::Plugin::MiracastPlayer::METHOD_MIRACAST_SET_RTSP_WAITTIMEOUT = "setRTSPWaitTimeOut";
const string WPEFramework::Plugin::MiracastPlayer::METHOD_MIRACAST_PLAYER_SET_LOG_LEVEL = "setLogging";

#ifdef ENABLE_MIRACAST_PLAYER_TEST_NOTIFIER
const string WPEFramework::Plugin::MiracastPlayer::METHOD_MIRACAST_TEST_NOTIFIER = "testNotifier";
#endif

#define EVT_ON_STATE_CHANGE "onStateChange"

namespace WPEFramework
{
	namespace
	{
		static Plugin::Metadata<Plugin::MiracastPlayer> metadata(
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
		SERVICE_REGISTRATION(MiracastPlayer, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

		MiracastPlayer *MiracastPlayer::_instance = nullptr;
		MiracastRTSPMsg *MiracastPlayer::m_miracast_rtsp_obj = nullptr;

		MiracastPlayer::MiracastPlayer()
			: PluginHost::JSONRPC(),
			  m_isServiceInitialized(false),
			  m_isServiceEnabled(false)
		{
			LOGINFO("Entering..!!!");
			MiracastPlayer::_instance = this;
			MIRACAST::logger_init("MiracastPlayer");
			Register(METHOD_MIRACAST_PLAYER_PLAY_REQUEST, &MiracastPlayer::playRequest, this);
			Register(METHOD_MIRACAST_PLAYER_STOP_REQUEST, &MiracastPlayer::stopRequest, this);
			Register(METHOD_MIRACAST_PLAYER_SET_PLAYER_STATE, &MiracastPlayer::setPlayerState, this);
			Register(METHOD_MIRACAST_PLAYER_SET_VIDEO_RECTANGLE, &MiracastPlayer::setVideoRectangle, this);
			Register(METHOD_MIRACAST_SET_VIDEO_FORMATS, &MiracastPlayer::setVideoFormats, this);
			Register(METHOD_MIRACAST_SET_AUDIO_FORMATS, &MiracastPlayer::setAudioFormats, this);
			Register(METHOD_MIRACAST_SET_RTSP_WAITTIMEOUT, &MiracastPlayer::setRTSPWaitTimeout, this);
			Register(METHOD_MIRACAST_PLAYER_SET_LOG_LEVEL, &MiracastPlayer::setLogging, this);

#ifdef ENABLE_MIRACAST_PLAYER_TEST_NOTIFIER
			Register(METHOD_MIRACAST_TEST_NOTIFIER, &MiracastPlayer::testNotifier, this);
			m_isTestNotifierEnabled = false;
#endif /* ENABLE_MIRACAST_SERVICE_TEST_NOTIFIER */
			LOGINFO("Exiting..!!!");
		}

		MiracastPlayer::~MiracastPlayer()
		{
			LOGINFO("Entering..!!!");
			if (nullptr != m_SystemPluginObj)
			{
				delete m_SystemPluginObj;
				m_SystemPluginObj = nullptr;
			}
			MIRACAST::logger_deinit();
			LOGINFO("Exiting..!!!");
		}

		// Thunder plugins communication
		void MiracastPlayer::getSystemPlugin()
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
						MIRACASTLOG_ERROR("failed to get security token\n");
					}
					security->Release();
				}
				else
				{
					MIRACASTLOG_ERROR("No security agent\n");
				}

				string query = "token=" + token;
				Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(SERVER_DETAILS)));
				m_SystemPluginObj = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(_T(SYSTEM_CALLSIGN_VER), (_T("MiracastPlayer")), false, query);
				if (nullptr == m_SystemPluginObj)
				{
					MIRACASTLOG_ERROR("JSONRPC: %s: initialization failed", SYSTEM_CALLSIGN_VER);
				}
				else
				{
					MIRACASTLOG_INFO("JSONRPC: %s: initialization ok", SYSTEM_CALLSIGN_VER);
				}
			}
			MIRACASTLOG_INFO("Exiting..!!!");
		}

		const string MiracastPlayer::Initialize(PluginHost::IShell *service)
		{
			string msg;
			MIRACASTLOG_INFO("Entering..!!!");
			if (0 == access("/opt/miracast_disable", F_OK))
			{
				msg = "'/opt/miracast_disable' flag available";
				return msg;
			}
			if (!m_isServiceInitialized)
			{
				MiracastError ret_code = MIRACAST_OK;
				m_miracast_rtsp_obj = MiracastRTSPMsg::getInstance(ret_code, this);
				if (nullptr != m_miracast_rtsp_obj)
				{
					m_CurrentService = service;
					m_GstPlayer = MiracastGstPlayer::getInstance();
					getSystemPlugin();
					m_isServiceInitialized = true;
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
						msg = "Unknown Error:Failed to obtain MiracastRTSPMsg Object";
					}
					break;
					}
				}
			}

			// On success return empty, to indicate there is no error text.
			return msg;
		}

		void MiracastPlayer::Deinitialize(PluginHost::IShell * /* service */)
		{
			MiracastPlayer::_instance = nullptr;
			MIRACASTLOG_INFO("Entering..!!!");

			if (m_isServiceInitialized)
			{
				MiracastRTSPMsg::destroyInstance();
				m_CurrentService = nullptr;
				m_miracast_rtsp_obj = nullptr;
				m_isServiceInitialized = false;
				m_isServiceEnabled = false;
				MIRACASTLOG_INFO("Done..!!!");
			}
			MIRACASTLOG_INFO("Exiting..!!!");
		}

		string MiracastPlayer::Information() const
		{
			return (string("{\"service\": \"") + SERVICE_NAME + string("\"}"));
		}

		uint32_t MiracastPlayer::playRequest(const JsonObject &parameters, JsonObject &response)
		{
			RTSP_HLDR_MSGQ_STRUCT rtsp_hldr_msgq_data = {0};
			bool success = false;
			MIRACASTLOG_INFO("Entering..!!!");

			if(parameters.HasLabel("device_parameters")) {
				JsonObject device_parameters;
				std::string	source_dev_ip = "",
							source_dev_mac = "",
							source_dev_name = "",
							sink_dev_ip = "";

				device_parameters = parameters["device_parameters"].Object();

				source_dev_ip = device_parameters["source_dev_ip"].String();
				source_dev_mac = device_parameters["source_dev_mac"].String();
				source_dev_name = device_parameters["source_dev_name"].String();
				sink_dev_ip = device_parameters["sink_dev_ip"].String();

				strncpy( rtsp_hldr_msgq_data.source_dev_ip, source_dev_ip.c_str() , sizeof(rtsp_hldr_msgq_data.source_dev_ip));
				strncpy( rtsp_hldr_msgq_data.source_dev_mac, source_dev_mac.c_str() , sizeof(rtsp_hldr_msgq_data.source_dev_mac));
				strncpy( rtsp_hldr_msgq_data.source_dev_name, source_dev_name.c_str() , sizeof(rtsp_hldr_msgq_data.source_dev_name));
				strncpy( rtsp_hldr_msgq_data.sink_dev_ip, sink_dev_ip.c_str() , sizeof(rtsp_hldr_msgq_data.sink_dev_ip));

				rtsp_hldr_msgq_data.state = RTSP_START_RECEIVE_MSGS;
				success = true;
			}

			if(parameters.HasLabel("video_rectangle")) {
				JsonObject video_rectangle;
				unsigned int startX = 0,
							 startY = 0,
							 width = 0,
							 height = 0;

				video_rectangle = parameters["video_rectangle"].Object();

				startX = video_rectangle["X"].Number();
				startY = video_rectangle["Y"].Number();
				width = video_rectangle["W"].Number();
				height = video_rectangle["H"].Number();

				if (( 0 < width ) && ( 0 < height ))
				{
					m_video_sink_rect.startX = startX;
					m_video_sink_rect.startY = startY;
					m_video_sink_rect.width = width;
					m_video_sink_rect.height = height;

					rtsp_hldr_msgq_data.videorect = m_video_sink_rect;
					m_miracast_rtsp_obj->send_msgto_rtsp_msg_hdler_thread(rtsp_hldr_msgq_data);
				}
				else{
					success = false;
				}
			}

			MIRACASTLOG_INFO("Exiting..!!!");
			returnResponse(success);
		}

		uint32_t MiracastPlayer::stopRequest(const JsonObject &parameters, JsonObject &response)
		{
			RTSP_HLDR_MSGQ_STRUCT rtsp_hldr_msgq_data = {0};
			int64_t json_parsed_value;
			eM_PLAYER_STOP_REASON_CODE	stop_reason_code;
			bool success = true;

			MIRACASTLOG_INFO("Entering..!!!");

			returnIfNumberParamNotFound(parameters, "reason_code");

			getNumberParameter("reason_code", json_parsed_value);
			stop_reason_code = static_cast<eM_PLAYER_STOP_REASON_CODE>(json_parsed_value);

			if ( MIRACAST_PLAYER_APP_REQ_TO_STOP_ON_EXIT == stop_reason_code )
			{
				rtsp_hldr_msgq_data.stop_reason_code = MIRACAST_PLAYER_APP_REQ_TO_STOP_ON_EXIT;
			}
			else if ( MIRACAST_PLAYER_APP_REQ_TO_STOP_ON_NEW_CONNECTION == stop_reason_code )
			{
				rtsp_hldr_msgq_data.stop_reason_code = MIRACAST_PLAYER_APP_REQ_TO_STOP_ON_NEW_CONNECTION;
			}
			else
			{
				success = false;
				MIRACASTLOG_ERROR("!!! UNKNOWN STOP REASON CODE RECEIVED[%#04X] !!!",stop_reason_code);
				response["message"] = "UNKNOWN STOP REASON CODE RECEIVED";
			}

			if ( success )
			{
				rtsp_hldr_msgq_data.state = RTSP_TEARDOWN_FROM_SINK2SRC;
				m_miracast_rtsp_obj->send_msgto_rtsp_msg_hdler_thread(rtsp_hldr_msgq_data);
			}

			MIRACASTLOG_INFO("Exiting..!!!");
			returnResponse(success);
		}

		uint32_t MiracastPlayer::setPlayerState(const JsonObject &parameters, JsonObject &response)
		{
			string player_state;
			bool success = false;
			MIRACASTLOG_INFO("Entering..!!!");
			if(parameters.HasLabel("state"))
			{
				RTSP_HLDR_MSGQ_STRUCT rtsp_hldr_msgq_data = {0};
				getStringParameter("state", player_state);
				success = true;
				if (player_state == "STOP" || player_state == "stop")
				{
					rtsp_hldr_msgq_data.state = RTSP_TEARDOWN_FROM_SINK2SRC;
				}
				else if (player_state == "PLAY" || player_state == "play")
				{
					rtsp_hldr_msgq_data.state = RTSP_PLAY_FROM_SINK2SRC;
				}
				else if (player_state == "PAUSE" || player_state == "pause")
				{
					rtsp_hldr_msgq_data.state = RTSP_PAUSE_FROM_SINK2SRC;
				}
				else
				{
					MIRACASTLOG_ERROR("Invalid Player state[%s]",player_state.c_str());
					success = false;
				}

				if (success)
				{
					m_miracast_rtsp_obj->send_msgto_rtsp_msg_hdler_thread(rtsp_hldr_msgq_data);
				}
			}
			MIRACASTLOG_INFO("Exiting..!!!");
			returnResponse(success);
		}

		uint32_t MiracastPlayer::setVideoRectangle(const JsonObject &parameters, JsonObject &response)
		{
			RTSP_HLDR_MSGQ_STRUCT rtsp_hldr_msgq_data = {0};
			bool success = false;
			MIRACASTLOG_INFO("Entering..!!!");

			returnIfParamNotFound(parameters, "X");
			returnIfParamNotFound(parameters, "Y");
			returnIfParamNotFound(parameters, "W");
			returnIfParamNotFound(parameters, "H");

			int startX = 0,
				startY = 0,
				width = 0,
				height = 0;

			startX = parameters["X"].Number();
			startY = parameters["Y"].Number();
			width = parameters["W"].Number();
			height = parameters["H"].Number();

			if (( 0 < width ) && ( 0 < height ) &&
				(( startX != m_video_sink_rect.startX ) ||
				( startY != m_video_sink_rect.startY ) ||
				( width != m_video_sink_rect.width ) ||
				( height != m_video_sink_rect.height )))
			{
				m_video_sink_rect.startX = startX;
				m_video_sink_rect.startY = startY;
				m_video_sink_rect.width = width;
				m_video_sink_rect.height = height;

				rtsp_hldr_msgq_data.videorect = m_video_sink_rect;
				rtsp_hldr_msgq_data.state = RTSP_UPDATE_VIDEO_RECT;
				m_miracast_rtsp_obj->send_msgto_rtsp_msg_hdler_thread(rtsp_hldr_msgq_data);
				success = true;
			}

			MIRACASTLOG_INFO("Exiting..!!!");
			returnResponse(success);
		}

		uint32_t MiracastPlayer::setRTSPWaitTimeout(const JsonObject &parameters, JsonObject &response)
		{
			bool success = false;
			unsigned int request_time = 0,
						 response_time = 0;

			MIRACASTLOG_INFO("Entering..!!!");

			returnIfParamNotFound(parameters, "Request");
			returnIfParamNotFound(parameters, "Response");

			request_time = parameters["Request"].Number();
			response_time = parameters["Response"].Number();

			success = m_miracast_rtsp_obj->set_WFDRequestResponseTimeout( request_time , response_time );

			MIRACASTLOG_INFO("Exiting..!!!");
			returnResponse(success);
		}

		uint32_t MiracastPlayer::setLogging(const JsonObject &parameters, JsonObject &response)
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

		/**
		 * @brief This method used to set the videoformats for Miracast.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		uint32_t MiracastPlayer::setVideoFormats(const JsonObject &parameters, JsonObject &response)
		{
			JsonArray h264_codecs;
			RTSP_WFD_VIDEO_FMT_STRUCT st_video_fmt = {0};
			bool success = false;

			MIRACASTLOG_INFO("Entering..!!!");

			returnIfParamNotFound(parameters, "native");
			returnIfBooleanParamNotFound(parameters, "display_mode_supported");
			returnIfParamNotFound(parameters, "h264_codecs");

			h264_codecs = parameters["h264_codecs"].Array();
			if (0 == h264_codecs.Length())
			{
				MIRACASTLOG_WARNING("Got empty list of h264_codecs");
				returnResponse(false);
			}
			getNumberParameter("native", st_video_fmt.native);
			getBoolParameter("display_mode_supported", st_video_fmt.preferred_display_mode_supported);

			JsonArray::Iterator index(h264_codecs.Elements());
			int64_t json_parsed_value;

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

					getNumberParameterObject(codecs, "profile", json_parsed_value);
					st_video_fmt.st_h264_codecs.profile = static_cast<uint8_t>(json_parsed_value);

					getNumberParameterObject(codecs, "level", json_parsed_value);
					st_video_fmt.st_h264_codecs.level = static_cast<uint8_t>(json_parsed_value);

					getNumberParameterObject(codecs, "cea_mask", json_parsed_value);
					st_video_fmt.st_h264_codecs.cea_mask = static_cast<RTSP_CEA_RESOLUTIONS>(json_parsed_value);

					getNumberParameterObject(codecs, "vesa_mask", json_parsed_value);
					st_video_fmt.st_h264_codecs.vesa_mask = static_cast<RTSP_VESA_RESOLUTIONS>(json_parsed_value);

					getNumberParameterObject(codecs, "hh_mask", json_parsed_value);
					st_video_fmt.st_h264_codecs.hh_mask = static_cast<RTSP_HH_RESOLUTIONS>(json_parsed_value);

					getNumberParameterObject(codecs, "latency", json_parsed_value);
					st_video_fmt.st_h264_codecs.latency = static_cast<uint8_t>(json_parsed_value);

					getNumberParameterObject(codecs, "min_slice", json_parsed_value);
					st_video_fmt.st_h264_codecs.min_slice = static_cast<uint8_t>(json_parsed_value);

					getNumberParameterObject(codecs, "slice_encode", json_parsed_value);
					st_video_fmt.st_h264_codecs.slice_encode = static_cast<uint8_t>(json_parsed_value);

					if (codecs.HasLabel("video_frame_skip_support"))
					{
						bool video_frame_skip_support;
						video_frame_skip_support = codecs["video_frame_skip_support"].Boolean();
						st_video_fmt.st_h264_codecs.video_frame_skip_support = video_frame_skip_support;
					}

					if (codecs.HasLabel("max_skip_intervals"))
					{
						uint8_t max_skip_intervals;
						getNumberParameterObject(codecs, "max_skip_intervals", max_skip_intervals);
						st_video_fmt.st_h264_codecs.max_skip_intervals = max_skip_intervals;
					}

					if (codecs.HasLabel("video_frame_rate_change_support"))
					{
						bool video_frame_rate_change_support;
						video_frame_rate_change_support = codecs["video_frame_rate_change_support"].Boolean();
						st_video_fmt.st_h264_codecs.video_frame_rate_change_support = video_frame_rate_change_support;
					}
				}
				else
					MIRACASTLOG_WARNING("Unexpected variant type");
			}
			success = m_miracast_rtsp_obj->set_WFDVideoFormat(st_video_fmt);

			MIRACASTLOG_INFO("Exiting..!!!");
			returnResponse(success);
		}

		/**
		 * @brief This method used to set the audioformats for Miracast.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		uint32_t MiracastPlayer::setAudioFormats(const JsonObject &parameters, JsonObject &response)
		{
			JsonArray audio_codecs;
			RTSP_WFD_AUDIO_FMT_STRUCT st_audio_fmt = {};
			bool success = false;

			MIRACASTLOG_INFO("Entering..!!!");

			returnIfParamNotFound(parameters, "audio_codecs");

			audio_codecs = parameters["audio_codecs"].Array();
			if (0 == audio_codecs.Length())
			{
				MIRACASTLOG_WARNING("Got empty list of audio_codecs");
				returnResponse(false);
			}

			JsonArray::Iterator index(audio_codecs.Elements());
			int64_t json_parsed_value;

			while (index.Next() == true)
			{
				if (Core::JSON::Variant::type::OBJECT == index.Current().Content())
				{
					JsonObject codecs = index.Current().Object();

					returnIfParamNotFound(codecs, "audio_format");
					returnIfParamNotFound(codecs, "modes");
					returnIfParamNotFound(codecs, "latency");

					getNumberParameterObject(codecs, "audio_format", json_parsed_value);
					st_audio_fmt.audio_format = static_cast<RTSP_AUDIO_FORMATS>(json_parsed_value);

					getNumberParameterObject(codecs, "modes", json_parsed_value);
					st_audio_fmt.modes = static_cast<uint32_t>(json_parsed_value);

					getNumberParameterObject(codecs, "latency", json_parsed_value);
					st_audio_fmt.latency = static_cast<uint8_t>(json_parsed_value);
				}
				else
					MIRACASTLOG_WARNING("Unexpected variant type");
			}
			success = m_miracast_rtsp_obj->set_WFDAudioCodecs(st_audio_fmt);

			MIRACASTLOG_INFO("Exiting..!!!");
			returnResponse(success);
		}

#ifdef ENABLE_MIRACAST_PLAYER_TEST_NOTIFIER
		/**
		 * @brief This method used to stop the client connection.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		uint32_t MiracastPlayer::testNotifier(const JsonObject &parameters, JsonObject &response)
		{
			MIRACAST_PLAYER_TEST_NOTIFIER_MSGQ_ST stMsgQ = {0};
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
						if ( MIRACAST_OK == m_miracast_rtsp_obj->create_TestNotifier())
						{
							m_isTestNotifierEnabled = true;
							success = true;
						}
						else
						{
							MIRACASTLOG_ERROR("Failed to enable TestNotifier");
							response["message"] = "Failed to enable TestNotifier";
						}
					}
					else if (status == "DISABLED" || status == "disabled")
					{
						MIRACASTLOG_ERROR("TestNotifier not yet enabled. Unable to Disable it");
						response["message"] = "TestNotifier not yet enabled. Unable to Disable";
					}
				}
				else
				{
					MIRACASTLOG_ERROR("TestNotifier not yet enabled");
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
						m_miracast_rtsp_obj->destroy_TestNotifier();
						success = true;
						m_isTestNotifierEnabled = false;
					}
					else if (status == "ENABLED" || status == "enabled")
					{
						MIRACASTLOG_ERROR("TestNotifier already enabled");
						response["message"] = "TestNotifier already enabled";
						success = false;
					}
					else
					{
						MIRACASTLOG_ERROR("Invalid status");
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
					MIRACASTLOG_ERROR("Invalid MAC/Name has passed");
					response["message"] = "Invalid MAC/Name has passed";
				}
				else
				{
					strcpy( stMsgQ.src_dev_name, client_name.c_str());
					strcpy( stMsgQ.src_dev_mac_addr, client_mac.c_str());

					MIRACASTLOG_INFO("Given 'NAME, MAC and state' are[%s-%s-%s]",
							client_name.c_str(),
							client_mac.c_str(),
							state.c_str());

					success = true;

					if (state == "STATE_CHANGE" || state == "state_change")
					{
						int64_t json_parsed_value;
						eMIRA_PLAYER_STATES player_state;
						eM_PLAYER_REASON_CODE reason_code;

						returnIfNumberParamNotFound(parameters, "player_state");
						returnIfNumberParamNotFound(parameters, "reason_code");

						getNumberParameter("player_state", json_parsed_value);
						player_state = static_cast<eMIRA_PLAYER_STATES>(json_parsed_value);

						getNumberParameter("reason_code", json_parsed_value);
						reason_code = static_cast<eM_PLAYER_REASON_CODE>(json_parsed_value);

						MIRACASTLOG_INFO("Given 'player_state and reason_code' are[%#04X--%#04X]",
								player_state,
								reason_code);

						if (( MIRACAST_PLAYER_REASON_CODE_MAX_ERROR > reason_code ) &&
							( MIRACAST_PLAYER_REASON_CODE_SUCCESS <= reason_code ) &&
							( MIRACAST_PLAYER_STATE_PAUSED > player_state ) &&
							( MIRACAST_PLAYER_STATE_IDLE <= player_state ))
						{
							stMsgQ.state = MIRACAST_PLAYER_TEST_NOTIFIER_STATE_CHANGED;
							stMsgQ.player_state = player_state;
							stMsgQ.reason_code = reason_code;
						}
						else
						{
							success = false;
							MIRACASTLOG_ERROR("Invalid playerstate/reason_code passed");
							response["message"] = "Invalid playerstate/reason_code passed";
						}
					}
					else
					{
						success = false;
						MIRACASTLOG_ERROR("Invalid state passed");
						response["message"] = "Invalid state passed";
					}

					if (success)
					{
						m_miracast_rtsp_obj->send_msgto_test_notifier_thread(stMsgQ);
					}
				}
			}

			MIRACASTLOG_INFO("Exiting..!!!");

			returnResponse(success);
		}
#endif/*ENABLE_MIRACAST_PLAYER_TEST_NOTIFIER*/

		void MiracastPlayer::onStateChange(string client_mac, string client_name, eMIRA_PLAYER_STATES player_state, eM_PLAYER_REASON_CODE reason_code)
		{
			MIRACASTLOG_INFO("Entering..!!!");

			JsonObject params;
			params["mac"] = client_mac;
			params["name"] = client_name;
			params["state"] = stateDescription(player_state);
			params["reason_code"] = std::to_string(reason_code);
			params["reason"] = reasonDescription(reason_code);

			if (0 == access("/opt/miracast_autoconnect", F_OK))
			{
				std::string system_command = "";
				system_command = "curl -H \"Authorization: Bearer `WPEFrameworkSecurityUtility | cut -d '\"' -f 4`\"";
				system_command.append(" --header \"Content-Type: application/json\" --request POST --data '{\"jsonrpc\":\"2.0\", \"id\":3,\"method\":\"org.rdk.MiracastService.1.updatePlayerState\", \"params\":{");
				system_command.append("\"mac\": \"");
				system_command.append(client_mac);
				system_command.append("\",");
				system_command.append("\"state\": \"");
				system_command.append(stateDescription(player_state));
				system_command.append(",");
				system_command.append("\"reason_code\": ");
				system_command.append(std::to_string(reason_code));
				system_command.append("}}' http://127.0.0.1:9998/jsonrpc\n");

				MIRACASTLOG_INFO("System Command [%s]\n",system_command.c_str());
				system( system_command.c_str());
			}
			else
			{
				sendNotify(EVT_ON_STATE_CHANGE, params);
			}
			MIRACASTLOG_INFO("Exiting..!!!");
		}

		std::string MiracastPlayer::stateDescription(eMIRA_PLAYER_STATES e)
		{
			switch (e)
			{
				case MIRACAST_PLAYER_STATE_IDLE:
					return "IDLE";
				case MIRACAST_PLAYER_STATE_INITIATED:
					return "INITIATED";
				case MIRACAST_PLAYER_STATE_INPROGRESS:
					return "INPROGRESS";
				case MIRACAST_PLAYER_STATE_PLAYING:
					return "PLAYING";
				case MIRACAST_PLAYER_STATE_STOPPED:
				case MIRACAST_PLAYER_STATE_SELF_ABORT:
					return "STOPPED";
				default:
					return "Unimplemented state";
			}
		}

		std::string MiracastPlayer::reasonDescription(eM_PLAYER_REASON_CODE e)
		{
			switch (e)
			{
				case MIRACAST_PLAYER_REASON_CODE_SUCCESS:
					return "SUCCESS";
				case MIRACAST_PLAYER_REASON_CODE_APP_REQ_TO_STOP:
					return "APP REQUESTED TO STOP.";
				case MIRACAST_PLAYER_REASON_CODE_SRC_DEV_REQ_TO_STOP:
					return "SRC DEVICE REQUESTED TO STOP.";
				case MIRACAST_PLAYER_REASON_CODE_RTSP_ERROR:
					return "RTSP Failure.";
				case MIRACAST_PLAYER_REASON_CODE_RTSP_TIMEOUT:
					return "RTSP Timeout.";
				case MIRACAST_PLAYER_REASON_CODE_RTSP_METHOD_NOT_SUPPORTED:
					return "RTSP Method Not Supported.";
				case MIRACAST_PLAYER_REASON_CODE_GST_ERROR:
					return "GStreamer Failure.";
				case MIRACAST_PLAYER_REASON_CODE_INT_FAILURE:
					return "Internal Failure.";
				case MIRACAST_PLAYER_REASON_CODE_NEW_SRC_DEV_CONNECT_REQ:
					return "APP REQ TO STOP FOR NEW CONNECTION.";
				default:
					return "Unimplemented item.";
			}
		}

	} // namespace Plugin
} // namespace WPEFramework
