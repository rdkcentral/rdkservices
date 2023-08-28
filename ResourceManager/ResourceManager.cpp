/**
 * * If not stated otherwise in this file or this component's LICENSE
 * * file the following copyright and licenses apply:
 * *
 * * Copyright 2020 RDK Management
 * *
 * * Licensed under the Apache License, Version 2.0 (the "License");
 * * you may not use this file except in compliance with the License.
 * * You may obtain a copy of the License at
 * *
 * * http://www.apache.org/licenses/LICENSE-2.0
 * *
 * * Unless required by applicable law or agreed to in writing, software
 * * distributed under the License is distributed on an "AS IS" BASIS,
 * * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * * See the License for the specific language governing permissions and
 * * limitations under the License.
 * **/

#include "ResourceManager.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

static std::string sThunderSecurityToken;
//methods
const string WPEFramework::Plugin::ResourceManager::RESOURCE_MANAGER_METHOD_SET_AV_BLOCKED = "setAVBlocked";
const string WPEFramework::Plugin::ResourceManager::RESOURCE_MANAGER_METHOD_GET_BLOCKED_AV_APPLICATIONS = "getBlockedAVApplications";
const string WPEFramework::Plugin::ResourceManager::RESOURCE_MANAGER_METHOD_RESERVE_TTS_RESOURCE = "reserveTTSResource";

namespace WPEFramework {
    namespace {

        static Plugin::Metadata<Plugin::ResourceManager> metadata(
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

    namespace Plugin {
        SERVICE_REGISTRATION(ResourceManager, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        ResourceManager* ResourceManager::_instance = nullptr;
        ResourceManager::ResourceManager()
            : PluginHost::JSONRPC()
        {
            Register<JsonObject, JsonObject>(_T(RESOURCE_MANAGER_METHOD_SET_AV_BLOCKED), &ResourceManager::setAVBlockedWrapper, this);
            Register<JsonObject, JsonObject>(_T(RESOURCE_MANAGER_METHOD_GET_BLOCKED_AV_APPLICATIONS), &ResourceManager::getBlockedAVApplicationsWrapper, this);
            Register<JsonObject, JsonObject>(_T(RESOURCE_MANAGER_METHOD_RESERVE_TTS_RESOURCE), &ResourceManager::reserveTTSResourceWrapper, this);
            mEssRMgr = nullptr;

#ifdef ENABLE_ERM
            mEssRMgr = EssRMgrCreate();
            std::cout<<"EssRMgrCreate "<<((mEssRMgr != nullptr)?"succeeded":"failed")<<std::endl;
#else
            std::cout<<"ENABLE_ERM not defined"<<std::endl;
#endif
        }

        ResourceManager::~ResourceManager()
        {
            Unregister(_T(RESOURCE_MANAGER_METHOD_SET_AV_BLOCKED));
            Unregister(_T(RESOURCE_MANAGER_METHOD_GET_BLOCKED_AV_APPLICATIONS));
            Unregister(_T(RESOURCE_MANAGER_METHOD_RESERVE_TTS_RESOURCE));
#ifdef ENABLE_ERM
            EssRMgrDestroy(mEssRMgr);
#endif
        }

        const string ResourceManager::Initialize(PluginHost::IShell * service )
        {
            ResourceManager::_instance = this;

            mCurrentService = service;

            // TODO: use interfaces and remove token
            auto security = mCurrentService->QueryInterfaceByCallsign<PluginHost::IAuthenticate>("SecurityAgent");
            if (security != nullptr) {
                string payload = "http://localhost";
                string token;
                if (security->CreateToken(
                        static_cast<uint16_t>(payload.length()),
                        reinterpret_cast<const uint8_t*>(payload.c_str()),
                        token)
                    == Core::ERROR_NONE) {
                    sThunderSecurityToken = token;
                    std::cout << "Resourcemanager got security token" << std::endl;
                } else {
                    std::cout << "Resourcemanager failed to get security token" << std::endl;
                }
                security->Release();
            } else {
                std::cout << "Resourcemanager : No security agent" << std::endl;
            }


            return (string());
        }

        void ResourceManager::Deinitialize(PluginHost::IShell * /* service */)
        {
            ResourceManager::_instance = nullptr;
        }

        string ResourceManager::Information() const
        {
            return (string());
        }

        uint32_t ResourceManager::setAVBlockedWrapper(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFOMETHOD();

            bool status = true;
            if (nullptr != mEssRMgr)
            {
                if (parameters.HasLabel("appid") && parameters.HasLabel("blocked"))
                {
                    std::string app = parameters["appid"].String();
                    bool blockAV    = parameters["blocked"].Boolean();
                    std::cout<<"appid : "<< app << std::endl;
                    std::cout<<"blocked  : "<<std::boolalpha << blockAV << std::endl;

                    status = (status && setAVBlocked(app, blockAV));
                    std::cout<< "Resourcemanager : EssRMgrAddToBlackList returned : "<<std::boolalpha <<status<< std::endl;
                }
                else
                {
                    std::string jsonstr;
                    parameters.ToString(jsonstr);
                    std::cout<<"Resourcemanager : ERROR: appid and appid status required in "<< jsonstr << std::endl;
                }
            }
            else
            {
                response["message"] = "ERM not enabled";
            }
            returnResponse(status);
        }

        uint32_t ResourceManager::getBlockedAVApplicationsWrapper(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFOMETHOD();
            bool status = false;
            JsonArray appsList = JsonArray(); 

            if (nullptr != mEssRMgr)
            {
                std::vector<std::string> apps;
                status = getBlockedAVApplications(apps);
                if (true == status)
                {
                    std::string appCallSign;
                    for (std::vector<std::string>::iterator appsItr = apps.begin(); appsItr != apps.end(); appsItr++)
                    {
                       appsList.Add(*appsItr);
                       // appCallSign = *appsItr;
                       // std::string::size_type pos = appCallSign.find(',');
                       // if (pos != std::string::npos)
                       // {
                       //     appsList.Add(appCallSign.substr(0, pos));
                       // }
                    }
                }
            }
            else
            {
                response["message"] = "ERM not enabled";
            }
            response["clients"]=appsList;
            returnResponse(status);
        }

        uint32_t ResourceManager::reserveTTSResourceWrapper(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFOMETHOD();
            bool status = false;

	    if (parameters.HasLabel("appid")) 
	    {
		std::string app = parameters["appid"].String();
                std::cout<<"appid : "<< app << std::endl;

                status = reserveTTSResource(app);
            }
            else
            {
                std::string jsonstr;
                parameters.ToString(jsonstr);
                std::cout<<"ERROR: appid required in "<< jsonstr << std::endl;
            }

            returnResponse(status);
        }

        bool ResourceManager::setAVBlocked(const string& callsign, const bool blockAV)
        {
            bool status = true;
#ifdef ENABLE_ERM
            status = blockAV?EssRMgrAddToBlackList(mEssRMgr, callsign.c_str()):EssRMgrRemoveFromBlackList(mEssRMgr, callsign.c_str());
            std::cout<<"setAVBlocked call returning  "<<std::boolalpha << status << std::endl;
            if (true == status)
            {
                mAppsAVBlacklistStatus[callsign] = blockAV;
		std::cout<<"mAppsAVBlacklistStatus updated  "<<std::endl;
            }
#endif
            return status;
        }

        bool ResourceManager::getBlockedAVApplications(std::vector<std::string> &appsList)
        {
            bool status = false;
#ifdef ENABLE_ERM
           std::cout<<"iterating mAppsAVBlacklistStatus ..." << std::endl;
            std::map<std::string, bool>::iterator appsItr = mAppsAVBlacklistStatus.begin();
            for (;appsItr != mAppsAVBlacklistStatus.end(); appsItr++)
            {
                std::cout<<"app : " <<appsItr->first<< std::endl;
                std::cout<<"blocked  : "<<std::boolalpha << appsItr->second << std::endl;
                if (true == appsItr->second)
                {
                    appsList.push_back(appsItr->first);
                }
            }
            status = true;
#endif
            return status;
        }

        struct JSONRPCDirectLink
        {
        private:
          uint32_t mId { 0 };
          std::string mCallSign { };
          PluginHost::IDispatcher * dispatcher_ {nullptr};

          Core::ProxyType<Core::JSONRPC::Message> Message() const
          {
            return (Core::ProxyType<Core::JSONRPC::Message>(PluginHost::IFactories::Instance().JSONRPC()));
          }

          template <typename PARAMETERS>
          bool ToMessage(PARAMETERS& parameters, Core::ProxyType<Core::JSONRPC::Message>& message) const
          {
            return ToMessage((Core::JSON::IElement*)(&parameters), message);
          }
          bool ToMessage(Core::JSON::IElement* parameters, Core::ProxyType<Core::JSONRPC::Message>& message) const
          {
            if (!parameters->IsSet())
              return true;
            string values;
            if (!parameters->ToString(values))
            {
              std::cout << "Failed to convert params to string\n";
              return false;
            }
            if (values.empty() != true)
            {
              message->Parameters = values;
            }
            return true;
          }
          template <typename RESPONSE>
          bool FromMessage(RESPONSE& response, const Core::ProxyType<Core::JSONRPC::Message>& message, bool isResponseString=false) const
          {
            return FromMessage((Core::JSON::IElement*)(&response), message, isResponseString);
          }
          bool FromMessage(Core::JSON::IElement* response, const Core::ProxyType<Core::JSONRPC::Message>& message, bool isResponseString=false) const
          {
            Core::OptionalType<Core::JSON::Error> error;
            if ( !isResponseString && !response->FromString(message->Result.Value(), error) )
            {
              std::cout << "Failed to parse response!!! Error: '" <<  error.Value().Message() << "'\n";
              return false;
            }
            return true;
          }

        public:
          JSONRPCDirectLink(PluginHost::IShell* service, std::string callsign)
            : mCallSign(callsign)
          {
            if (service)
              dispatcher_ = service->QueryInterfaceByCallsign<PluginHost::IDispatcher>(mCallSign);
          }
      
          JSONRPCDirectLink(PluginHost::IShell* service)
            : JSONRPCDirectLink(service, "Controller")
          {
          }
          ~JSONRPCDirectLink()
          {
            if (dispatcher_)
              dispatcher_->Release();
          }

          template <typename PARAMETERS>
          uint32_t Get(const uint32_t waitTime, const string& method, PARAMETERS& respObject)
          {
            JsonObject empty;
            return Invoke(waitTime, method, empty, respObject);
          }

          template <typename PARAMETERS>
          uint32_t Set(const uint32_t waitTime, const string& method, const PARAMETERS& sendObject)
          {
            JsonObject empty;
            return Invoke(waitTime, method, sendObject, empty);
          }

          template <typename PARAMETERS, typename RESPONSE>
          uint32_t Invoke(const uint32_t waitTime, const string& method, const PARAMETERS& parameters, RESPONSE& response, bool isResponseString=false)
          {
            if (dispatcher_ == nullptr) {
              std::cout << "No JSON RPC dispatcher for " << mCallSign << '\n';
              return Core::ERROR_GENERAL;
            }

            auto message = Message();

            message->JSONRPC = Core::JSONRPC::Message::DefaultVersion;
            message->Id = Core::JSON::DecUInt32(++mId);
            message->Designator = Core::JSON::String(mCallSign + ".1." + method);

            ToMessage(parameters, message);

            const uint32_t channelId = ~0;
#ifndef USE_THUNDER_R4
            auto resp =  dispatcher_->Invoke(sThunderSecurityToken, channelId, *message);
#else
            Core::JSONRPC::Context context(channelId, message->Id.Value(), sThunderSecurityToken) ;
            auto resp = dispatcher_->Invoke(context, *message);
#endif /* USE_THUNDER_R4 */
            if (resp->Error.IsSet()) {
              std::cout << "Call failed: " << message->Designator.Value() << " error: " <<  resp->Error.Text.Value() << "\n";
              return resp->Error.Code;
            }

            if (!FromMessage(response, resp, isResponseString))
              return Core::ERROR_GENERAL;

            return Core::ERROR_NONE;
          }
        };

        bool ResourceManager::reserveTTSResource(const string& client)
        {
            uint32_t ret = Core::ERROR_NONE;
            bool status = false;

            JsonObject params;
            JsonObject result;
            JsonObject clientParam;
            JsonArray clientList = JsonArray(); 
            JsonArray accessList = JsonArray();

            clientList.Add(client);
            clientParam.Set("method", "speak");
            clientParam["apps"] = clientList;
            accessList.Add(clientParam);
            params["accesslist"] = accessList;

            std::string jsonstr;
            params.ToString(jsonstr);
            std::cout<<"Resourcemanager : about to call setACL : "<< jsonstr << std::endl;

            ret =  JSONRPCDirectLink(mCurrentService, "org.rdk.TextToSpeech").Invoke<JsonObject, JsonObject>(20000, "setACL", params, result);

            status = ((Core::ERROR_NONE == ret) && (result.HasLabel("success")) && (result["success"].Boolean()));

            result.ToString(jsonstr);
            std::cout<<"setACL response : "<< jsonstr << std::endl;
            std::cout<<"setACL status  : "<<std::boolalpha << status << std::endl;
            
            return (status);
        }

    } // namespace Plugin
} // namespace WPEFramework
