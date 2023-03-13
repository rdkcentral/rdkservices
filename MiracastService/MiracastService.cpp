/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
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
 **/

#include "MiracastService.h"
#include <algorithm>

#include "rdk/iarmmgrs-hal/pwrMgr.h"

#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"

const short WPEFramework::Plugin::MiracastService::API_VERSION_NUMBER_MAJOR = 1;
const short WPEFramework::Plugin::MiracastService::API_VERSION_NUMBER_MINOR = 0;
const string WPEFramework::Plugin::MiracastService::SERVICE_NAME = "org.rdk.MiracastService";
const string WPEFramework::Plugin::MiracastService::METHOD_MIRACAST_SET_ENABLE = "setEnable";
const string WPEFramework::Plugin::MiracastService::METHOD_MIRACAST_CLIENT_CONNECT_REQUEST = "acceptClientConnectionRequest";

using namespace std;

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework
{
	namespace {

		static Plugin::Metadata<Plugin::MiracastService> metadata(
				// Version (Major, Minor, Patch)
				API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
				// Preconditions
				{},
				// Terminations
				{},
				// Controls
				{}
				);
	}

	namespace Plugin
	{
		SERVICE_REGISTRATION(MiracastService, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

		MiracastService* MiracastService::_instance = nullptr;
		MiracastServiceImplementation* MiracastService::m_miracast_service_impl = nullptr;

		MiracastService::MiracastService()
			: PluginHost::JSONRPC(),
			m_isPlatInitialized (false)
		{
			LOGINFO("MiracastService::ctor");
			MiracastService::_instance = this;
			Register( METHOD_MIRACAST_SET_ENABLE , &MiracastService::setEnable, this);
			Register( METHOD_MIRACAST_CLIENT_CONNECT_REQUEST , &MiracastService::acceptClientConnectionRequest, this);
		}

		MiracastService::~MiracastService()
		{
			LOGINFO("MiracastService::~MiracastService");
		}

		const string MiracastService::Initialize(PluginHost::IShell* service )
		{
			LOGINFO("MiracastService::Initialize");
			if (!m_isPlatInitialized){
				mCurrentService = service;
				m_miracast_service_impl = MiracastServiceImplementation::create(nullptr);
				m_isPlatInitialized = true;
			}

			// On success return empty, to indicate there is no error text.
			return (string());
		}

		void MiracastService::Deinitialize(PluginHost::IShell* /* service */)
		{
			MiracastService::_instance = nullptr;
			LOGINFO("MiracastService::Deinitialize");
			if (m_isPlatInitialized){
				m_miracast_service_impl->StopApplication();
				MiracastServiceImplementation::Destroy(m_miracast_service_impl);
				mCurrentService = nullptr;
				m_miracast_service_impl = nullptr;
				m_isPlatInitialized = false;
			}
		}

		string MiracastService::Information() const
		{
			return(string("{\"service\": \"") + SERVICE_NAME + string("\"}"));
		}

		/**
		 * @brief This method used to Enable/Disable the Miracast Discovery.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		uint32_t MiracastService::setEnable(const JsonObject& parameters, JsonObject& response)
		{
			bool success = false;
			std::string is_enabled = "";

			LOGINFO("MiracastService::setEnable");
			if ( parameters.HasLabel("enabled")){
				is_enabled = parameters["enabled"].String();
				if (( "true" == is_enabled ) || ( "false" == is_enabled )){
					m_miracast_service_impl->setEnable( is_enabled );
					success = true;
				}
			}
			returnResponse(success);
		}

		/**
		 * @brief This method used to accept or reject the connection request.
		 *
		 * @param: None.
		 * @return Returns the success code of underlying method.
		 */
		uint32_t MiracastService::acceptClientConnectionRequest(const JsonObject& parameters, JsonObject& response)
		{
			bool success = false;
			std::string requestedStatus = "";

			LOGINFO("MiracastService::acceptClientConnectionRequest");
			if ( parameters.HasLabel("requestStatus")){
				requestedStatus = parameters[""].String();
				if (( "Accept" == requestedStatus ) || ( "Reject" == requestedStatus )){
					m_miracast_service_impl->acceptClientConnectionRequest( requestedStatus );
					success = true;
				}
			}

			returnResponse(success);
		}
	} // namespace Plugin
} // namespace WPEFramework
