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

#pragma once

#include "Module.h"
#include "acrclient.h"
namespace WPEFramework {

    namespace Plugin {
	const string ACR_EVENTS = "com.comcast.acr_events";
	const string LAR_EVENTS = "com.comcast.lar_events";
	const string ACRLAR_EVENTS = "AcrLarEvents";
	
	class AudienceIntelligence;
        class AudienceIntelligenceListener : public ACRClientEventListener{
         public:
                AudienceIntelligenceListener(AudienceIntelligence* audintelligence);
                ~AudienceIntelligenceListener();
                virtual void onCLDSignatureEvent(const std::string& event,uint64_t epochts,unsigned int is_interlaced,unsigned int frame_rate,unsigned int pic_width,unsigned int pic_height);
		void onLARATSCMediaPlayEvent();
		//TODO add method to receive LAR Events
         private:
                AudienceIntelligence& maudintelligence;
         };


        class AudienceIntelligence : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        private:

            // We do not allow this plugin to be copied !!
            AudienceIntelligence(const AudienceIntelligence&) = delete;
            AudienceIntelligence& operator=(const AudienceIntelligence&) = delete;

            //Begin methods
            uint32_t getLogLevelWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setLogLevelWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t enableLAR(const JsonObject& parameters, JsonObject& response);
            uint32_t enableACR(const JsonObject& parameters, JsonObject& response);
            uint32_t setACRFrequency(const JsonObject& parameters, JsonObject& response);
        	
	    uint32_t registerListeners(const JsonObject& parameters, JsonObject& response);
            uint32_t unregisterListeners(const JsonObject& parameters, JsonObject& response);

	    //End methods

            //Begin events
	    //End events

	protected:
            void InitializeIARM();
            void DeinitializeIARM();

        public:
            AudienceIntelligence();
            virtual ~AudienceIntelligence();

            void notify(const std::string& event, const JsonObject& parameters); 
            AudienceIntelligenceListener *_acrEventListener;
            ACRClient *_acrClient;
 
	    static AudienceIntelligence* _instance;
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override;

            BEGIN_INTERFACE_MAP(AudienceIntelligence)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

        };
	} // namespace Plugin
} // namespace WPEFramework
