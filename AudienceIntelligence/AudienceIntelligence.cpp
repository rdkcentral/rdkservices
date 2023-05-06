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
#include <cjson/cJSON.h>
#include "AudienceIntelligence.h"

#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"

#include <string>
#include <memory>
#include <iostream>
#include <mutex>
#include <thread>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <syscall.h>

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

bool ACRModeEnabled = true;
bool LARModeEnabled = true;
JsonArray acrevents_arr;

using namespace std;
namespace WPEFramework
{
    namespace Plugin
    {

	SERVICE_REGISTRATION(AudienceIntelligence, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);
	AudienceIntelligence* AudienceIntelligence::_instance = nullptr;
	std::vector<string> registeredEvtListeners;

	AudienceIntelligence::AudienceIntelligence()
        : PluginHost::JSONRPC(),
	_acrEventListener(nullptr)
        {
	    LOGINFO("ctor");

            Register("getLogLevel", &AudienceIntelligence::getLogLevelWrapper, this);
            Register("setLogLevel", &AudienceIntelligence::setLogLevelWrapper, this);
            Register("enableLAR", &AudienceIntelligence::enableLAR, this);
            Register("enableACR", &AudienceIntelligence::enableACR, this);
            Register("setACRFrequency", &AudienceIntelligence::setACRFrequency, this);

	    Register("registerListeners",&AudienceIntelligence::registerListeners, this);  //Register ACRLAR Events
	    Register("unregisterListeners",&AudienceIntelligence::unregisterListeners, this);
        }

	const string AudienceIntelligence::Initialize(PluginHost::IShell* service)
        {
	    LOGWARN("Initlaizing AudienceIntelligence");
            AudienceIntelligence::_instance = this;
	    _acrClient        = ACRClient::getInstance();
            _acrEventListener = new AudienceIntelligenceListener(this);
            InitializeIARM();
            return (string());
        }

	void AudienceIntelligence::Deinitialize(PluginHost::IShell * /* service */)
        {
	    LOGWARN("DeInitlaizing AudienceIntelligence");
            DeinitializeIARM();
            AudienceIntelligence::_instance = nullptr;
        }

        string AudienceIntelligence::Information() const
        {
            return (string());
        }

	void AudienceIntelligence::InitializeIARM()
        {
           LOGWARN("AudienceIntelligence::InitializeIARM");
	   if(Utils::IARM::init())
            {
       //             IARM_Result_t res;
            }
        }

       void AudienceIntelligence::DeinitializeIARM()
        {
           LOGWARN("AudienceIntelligence::DeinitializeIARM");
	   if(Utils::IARM::isConnected())
            {
            //        IARM_Result_t res;
            }

        }

	// Registered methods begin
        uint32_t AudienceIntelligence::getLogLevelWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
	    bool result = true;
            std::string logLevel = "INFO";
            //todo
            if (false == result) {
                response["message"] = "failed to get log level";
            }
            else {
                response["level"] = logLevel;
            }
	    returnResponse(result);
        }
        
	uint32_t AudienceIntelligence::setLogLevelWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
	    bool result = true;
            if (!parameters.HasLabel("level"))
            {
                result = false;
                response["message"] = "please specify log level (level = DEBUG/INFO/WARN/ERROR/FATAL)";
            }
            if (result)
            {
                std::string logLevel  = parameters["level"].String();
                //todo
                if (false == result) {
                    response["message"] = "failed to set log level";
                }
                else
                {
                    response["level"] = logLevel;
                }
            }
	    returnResponse(result);
	}
        
	uint32_t AudienceIntelligence::enableACR(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
	    bool result = true;

            if (!parameters.HasLabel("enable"))
            {
                result = false;
                response["message"] = "please specify enable parameter";
            }
            if (result)
            {
                ACRModeEnabled = parameters["enable"].Boolean();
                    if (ACRModeEnabled) {
                        response["message"] = "ACR feature enabled";
			if(_acrClient) {
                                _acrClient->enableAudienceIntelligence(ACRModeEnabled);
                        }
		    }
                    else {
			 _acrClient->enableAudienceIntelligence(ACRModeEnabled);
                        response["message"] = "ACR feature disabled";
		    }
            }
	    returnResponse(result);
        }
        
	uint32_t AudienceIntelligence::enableLAR(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
	    bool result = true;

            if (!parameters.HasLabel("enable"))
            {
                result = false;
                response["message"] = "please specify enable parameter";
            }
            if (result)
            {
                LARModeEnabled = parameters["enable"].Boolean();
                    if (LARModeEnabled)
                        response["message"] = "LAR feature enabled";
                    else
                        response["message"] = "LAR feature disabled";
            }
	    returnResponse(result);
        }
        
	uint32_t AudienceIntelligence::setACRFrequency(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
		return 0;
        }


	uint32_t AudienceIntelligence::registerListeners(const JsonObject& parameters, JsonObject& response)
        {
                LOGINFOMETHOD();

                bool ret=false;
                string json;
                std::vector<string>::iterator it;
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
                        ret = true;
                        cJSON *elem;
                        int i;
                        vector<string> eventname;
                        JsonArray response_arr;
                        JsonObject acrlarevent;
			LOGINFO("Register arrsize : %d ",arrsize);
                        for(i=0;i<arrsize;i++)
                        {
                                elem = cJSON_GetArrayItem(items, i);
                                string evt_str =elem->valuestring;
                                eventname.push_back(evt_str);
                                LOGINFO("Register Event Listener %s ",evt_str.c_str());
                                acrlarevent["propertyName"] = evt_str;
                                response_arr.Add(acrlarevent);
                         }

                         for( it = eventname.begin(); it!= eventname.end(); ++it )
                         {

                                if (std::find(registeredEvtListeners.begin(), registeredEvtListeners.end(), *it) == registeredEvtListeners.end())
                                {
                                        LOGINFO("Event being added to listeners %s",it->c_str());
                                        registeredEvtListeners.push_back(*it);
                                }
			 }
                         _acrClient->RegisterEventListener(eventname,_acrEventListener);
			 cout << "ACR RegisterEventListener after call "<< endl;
                      	 response["properties"]=response_arr;
                         /*string json_str;
                         response.ToString(json_str);
                         LOGINFO("json array of properties is %s\n",json_str.c_str());*/

                }
                LOGTRACEMETHODFIN();
                cJSON_Delete(root);
                returnResponse(ret);
        }

	 uint32_t AudienceIntelligence::unregisterListeners(const JsonObject& parameters, JsonObject& response)
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
                        vector<string> acrlarevents;
                        for(int i=0;i<arrsize;i++)
                        {
                                elem = cJSON_GetArrayItem(items, i);
                                string prop_str =elem->valuestring;
                                acrlarevents.push_back(prop_str);
                        }
                        for( vector<string>::iterator it = acrlarevents.begin(); it!= acrlarevents.end(); ++it )
                        {
                                vector<string>::iterator itr=find(registeredEvtListeners.begin(), registeredEvtListeners.end(), *it);
                                if(itr!=registeredEvtListeners.end())
                                {
                                        LOGINFO("Event erased : %s",it->c_str());
                                        registeredEvtListeners.erase(itr);
                                }
                        }
                        _acrClient->UnRegisterEventListener(acrlarevents,_acrEventListener);
                }
                LOGTRACEMETHODFIN();
                cJSON_Delete(root);
                returnResponse(ret);
        }


 	void AudienceIntelligence::notify(const std::string& event, const JsonObject& parameters)
        {
                string property_name = parameters["propertyName"].String();
		LOGINFO(" event notification : %s  propertyname : %s \n",event.c_str(),property_name.c_str());
                if(std::find(registeredEvtListeners.begin(), registeredEvtListeners.end(), property_name) != registeredEvtListeners.end())
                {

			 LOGINFO(" Property registered notifying the events \n");
                        sendNotify(event.c_str(),parameters);
                }
	}

	AudienceIntelligence::~AudienceIntelligence()
	{
            Unregister("getLogLevel");
            Unregister("setLogLevel");
            Unregister("enableLAR");
            Unregister("enableACR");
            Unregister("setACRFrequency");
	    Unregister("registerListeners");
	    Unregister("unregisterListeners");
            delete _acrEventListener;
            _acrEventListener = nullptr;
            delete _acrClient;
            _acrClient = nullptr;

       }

	AudienceIntelligenceListener::AudienceIntelligenceListener(AudienceIntelligence* audintelligence)
                        : maudintelligence(*audintelligence)
        {
		LOGINFO("Constructor \n");
        }
        AudienceIntelligenceListener::~AudienceIntelligenceListener()
        {
		LOGINFO("Destructor \n");
        }


	void AudienceIntelligenceListener::onCLDSignatureEvent(const std::string& event,uint64_t epochts,unsigned int is_interlaced,unsigned int frame_rate,unsigned int pic_width,unsigned int pic_height)
        {
                LOGINFO("CLD event at Audience Intelligence Plugin :%s \n",event.c_str());

                JsonObject params;
		params.FromString(event);
		params["timestamp"] = epochts;
                acrevents_arr.Add(params);
		if(acrevents_arr.Length() == 4)
		{
                	//string json;
                	JsonObject acrevent;
			acrevent["propertyName"]=ACR_EVENTS;
			acrevent["acr_signatures"] = acrevents_arr;		
			acrevent["is_interlaced"] = is_interlaced;
			acrevent["frame_rate"] 	  = frame_rate;
			acrevent["pic_width"] 	  = pic_width;
			acrevent["pic_height"]    = pic_height;
			//acrevent.ToString(json);
                	//LOGINFO("NOTIFY json is %s\n",json.c_str());
                	maudintelligence.notify("onacrevent",acrevent);
			acrevents_arr.Clear();
			LOGINFO(" length of acrevents_arr now %d \n",acrevents_arr.Length());
		}
        }

	void AudienceIntelligenceListener::onLARATSCMediaPlayEvent()
	{
		LOGINFO("onLARATSCMediaPlayEvent \n");
		sendNotify("onLARMediaPlay",JsonObject());
	}



    } // namespace Plugin
} // namespace WPEFramework
 
