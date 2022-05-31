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

 /**
 * @file ContinueWatching.h
 * @brief Thunder Plugin based Implementation for ContinueWatching service API's (RDK-25833).
 */

 /**
  @mainpage ContinueWatching

  <b>ContinueWatching</b> The Continue Watching Service will provide
  *  a method for applications on the STB to store a token for
  * retrieval by XRE.Applications like netflix will use this to store a token
  * that XRE will retrieve and use the data to gather data from the
  * OTT provider that can be used to populate a continue watching panel in the UI.
  * This service will be enabled/disabled using an TR181 parameter.
  */

#pragma once

#ifndef CONTINUEWATCHING_H
#define CONTINUEWATCHING_H

#include <string.h>
#include <mutex>
#include "Module.h"
#if !defined(DISABLE_SECAPI)
#include "sec_security_datatype.h"
#endif

#define CW_LOCAL_FILE  "/opt/continuewatching.json"
#define NETFLIX_CONTINUEWATCHING_APP_NAME  "netflix"

namespace WPEFramework {

	namespace Plugin {

		/**
		* @brief WPEFramework class declaration for ContinueWatching
		**/

		// This is a server for a JSONRPC communication channel.
		// For a plugin to be capable to handle JSONRPC, inherit from PluginHost::JSONRPC.
		// By inheriting from this class, the plugin realizes the interface PluginHost::IDispatcher.
		// This realization of this interface implements, by default, the following methods on this plugin
		// - exists
		// - register
		// - unregister
		// Any other methood to be handled by this plugin  can be added can be added by using the
		// templated methods Register on the PluginHost::JSONRPC class.
		// As the registration/unregistration of notifications is realized by the class PluginHost::JSONRPC,
		// this class exposes a public method called, Notify(), using this methods, all subscribed clients
		// will receive a JSONRPC message as a notification, in case this method is called.
	        class ContinueWatching : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        	private:
			// We do not allow this plugin to be copied !!
			ContinueWatching(const ContinueWatching&) = delete;
			ContinueWatching& operator=(const ContinueWatching&) = delete;
			uint32_t getApplicationToken(const JsonObject& parameters, JsonObject& response);
			uint32_t setApplicationToken(const JsonObject& parameters, JsonObject& response);
			uint32_t deleteApplicationToken(const JsonObject& parameters, JsonObject& response);
			std::string getAppToken(std::string strApplicationName);
			bool setAppToken(std::string strApplicationName, std::string token);
			bool deleteAppToken(std::string strApplicationName);
	       private:
			std::mutex m_mutex;
        	public:
			ContinueWatching();
			virtual ~ContinueWatching();
            virtual const string Initialize(PluginHost::IShell* shell) override { return {}; }
                        virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override { return {}; }
            BEGIN_INTERFACE_MAP(ContinueWatching)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP
        	private:
			uint32_t getApiVersionNumber();
			void setApiVersionNumber(uint32_t apiVersionNumber);
        	public:
			static ContinueWatching* _instance;
		private:
			uint32_t m_apiVersionNumber;
	        };

		/**
		* @brief Class declaration for ContinueWatching Implementation
		**/
		class ContinueWatchingImpl
		{
		public:
   			ContinueWatchingImpl();
			virtual ~ContinueWatchingImpl();
			virtual std::string getApplicationToken() = 0;
			virtual bool setApplicationToken(std::string token) = 0;
			virtual bool deleteApplicationToken() = 0;
			bool encryptData(uint8_t* clearData, int clearDataLength, uint8_t* protectedData, int protectedDataLength, int &bytesWritten);
			bool decryptData(uint8_t* protectedData, int protectedDataLength, uint8_t* clearData, int clearDataLength, int &bytesWritten);
			std::string sha256(const std::string str);
			bool writeToJson(std::string protectedData);
			std::string readFromJson();
			bool deleteToken();

		protected:
		    	bool checkTR181(const std::string& feature);
	    		bool tr181FeatureEnabled();

	    		std::string mStrApplicationName;
			#if !defined(DISABLE_SECAPI)
    			SEC_OBJECTID mSecObjectId;
			#endif
		};

		/**
		* @brief Class declaration for Netflix's Implementation of ContinueWatching
		**/
		class NetflixContinueWatchingImpl : public ContinueWatchingImpl
		{
		public:
			NetflixContinueWatchingImpl();
			virtual ~NetflixContinueWatchingImpl();
			std::string getApplicationToken();
			bool setApplicationToken(std::string token);
			bool deleteApplicationToken();
		};

		class ContinueWatchingImplFactory
		{
		public:
			ContinueWatchingImplFactory();
			virtual ~ContinueWatchingImplFactory();
			ContinueWatchingImpl* createContinueWatchingImpl(std::string strApplicationName);
		};
	} // namespace Plugin
} // namespace WPEFramework

#endif // CONTINUEWATCHING_H
