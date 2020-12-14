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

#include "EasterEgg.h"
#include <string>
#include <iostream>
#include <mutex>
#include <thread>

const short WPEFramework::Plugin::EasterEgg::API_VERSION_NUMBER_MAJOR = 1;
const short WPEFramework::Plugin::EasterEgg::API_VERSION_NUMBER_MINOR = 0;
const string WPEFramework::Plugin::EasterEgg::SERVICE_NAME = "org.rdk.EasterEgg";
//methods
const string WPEFramework::Plugin::EasterEgg::EASTEREGG_METHOD_LAUNCH_FACTORY_APP = "launchFactoryApp";
const string WPEFramework::Plugin::EasterEgg::EASTEREGG_METHOD_LAUNCH_RESIDENT_APP = "launchResidentApp";
const string WPEFramework::Plugin::EasterEgg::EASTEREGG_METHOD_TOGGLE_FACTORY_APP = "toggleFactoryApp";
const string WPEFramework::Plugin::EasterEgg::EASTEREGG_METHOD_LAUNCH_FACTORY_APP_SHORTCUT = "launchFactoryAppShortcut";

using namespace std;

#define EASTEREGG_THUNDER_TIMEOUT 20000

namespace WPEFramework {
    namespace Plugin {

        SERVICE_REGISTRATION(EasterEgg, 1, 0);

        EasterEgg* EasterEgg::_instance = nullptr;

        EasterEgg::EasterEgg()
                : AbstractPlugin(), mFactoryAppRunning(false)
        {
            LOGINFO("ctor");
            EasterEgg::_instance = this;

            registerMethod(EASTEREGG_METHOD_LAUNCH_FACTORY_APP, &EasterEgg::launchFactoryAppWrapper, this);
            registerMethod(EASTEREGG_METHOD_LAUNCH_RESIDENT_APP, &EasterEgg::launchResidentAppWrapper, this);
            registerMethod(EASTEREGG_METHOD_TOGGLE_FACTORY_APP, &EasterEgg::toggleFactoryAppWrapper, this);
            registerMethod(EASTEREGG_METHOD_LAUNCH_FACTORY_APP_SHORTCUT, &EasterEgg::launchFactoryAppShortcutWrapper, this);
        }

        EasterEgg::~EasterEgg()
        {
            LOGINFO("dtor");
            EasterEgg::_instance = nullptr;
        }

        string EasterEgg::Information() const
        {
            return(string("{\"service\": \"") + SERVICE_NAME + string("\"}"));
        }

        const string EasterEgg::Initialize(PluginHost::IShell* service )
        {
            LOGINFO();

            std::cout << "initializing\n";
            //Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));
            //mRdkShellPlugin = make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>>("org.rdk.RDKShell.1", "");
            return "";
        }

        void EasterEgg::Deinitialize(PluginHost::IShell* service)
        {
            LOGINFO();

            mRdkShellPlugin = nullptr;
        }

        std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> > EasterEgg::getThunderControllerClient(std::string callsign)
        {
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));
            std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> > thunderClient = make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> >(callsign.c_str(), "");
            return thunderClient;
        }

        std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> EasterEgg::getRDKShellPlugin()
        {
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));
            return make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>>("org.rdk.RDKShell.1", "");
        }

        void EasterEgg::killAllApps()
        {
            auto rdkshellPlugin = getRDKShellPlugin();
            if (!rdkshellPlugin)
            {
                return;
            }

            uint32_t status;
            JsonObject getStateResult;
            JsonObject param;

            status = rdkshellPlugin->Invoke<JsonObject, JsonObject>(EASTEREGG_THUNDER_TIMEOUT, "getState", param, getStateResult);
            if (status > 0)
            {
                std::cout << "failed to get state " << std::endl;
            }
            
            // If success is false, the container isn't running so nothing to do
            if (getStateResult["success"].Boolean())
            {
                const JsonArray stateList = getStateResult.HasLabel("state")?getStateResult["state"].Array():JsonArray();
                for (int i=0; i<stateList.Length(); i++)
                {
                    const JsonObject& stateInfo = stateList[i].Object();
                    if (stateInfo.HasLabel("callsign"))
                    {
                       JsonObject destroyRequest, destroyResponse;
                       destroyRequest["callsign"] = stateInfo["callsign"].String();
                       std::cout << "destroying " << stateInfo["callsign"].String().c_str() << std::endl;
                       status = rdkshellPlugin->Invoke<JsonObject, JsonObject>(EASTEREGG_THUNDER_TIMEOUT, "destroy", destroyRequest, destroyResponse);
                       if (status > 0)
                       {
                           std::cout << "failed to destroy client " << destroyRequest["callsign"].String() << std::endl;
                       }
                    }
                }
            }
        }

        uint32_t EasterEgg::launchFactoryAppWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            if (parameters.HasLabel("startup"))
            {
                bool startup = parameters["startup"].Boolean();
                if (startup)
                {
                    JsonObject joAgingParams;
                    JsonObject joAgingResult;
                    joAgingParams.Set("namespace","FactoryTest");
                    joAgingParams.Set("key","AgingState");
                    std::string agingGetInvoke = "org.rdk.PersistentStore.1.getValue";

                    std::cout << "attempting to check aging flag \n";
                    uint32_t status = getThunderControllerClient()->Invoke(EASTEREGG_THUNDER_TIMEOUT, agingGetInvoke.c_str(), joAgingParams, joAgingResult);
                    std::cout << "get status: " << status << std::endl;

                    if (status > 0)
                    {
                        response["message"] = " unable to check aging flag";
                        returnResponse(false);
                    }

                    if (!joAgingResult.HasLabel("value"))
                    {
                        response["message"] = " aging value not found";
                        returnResponse(false);
                    }

                    const std::string valueString = joAgingResult["value"].String();
                    if (valueString != "true")
                    {
                        std::cout << "aging value is " << valueString << std::endl;
                        response["message"] = " aging is not set for startup";
                        returnResponse(false);
                    }
                }
            }

            mFactoryAppRunning = true;
            bool ret = true;

            killAllApps();
            auto rdkshellPlugin = getRDKShellPlugin();
            JsonObject destroyRequest, destroyResponse;
            destroyRequest["callsign"] = "ResidentApp";
            uint32_t status = rdkshellPlugin->Invoke<JsonObject, JsonObject>(EASTEREGG_THUNDER_TIMEOUT, "destroy", destroyRequest, destroyResponse);

            if (!rdkshellPlugin)
            {
                response["message"] = "rdkshell plugin initialization failed";
                returnResponse(false);
            }

            char* factoryAppUrl = getenv("RDKSHELL_FACTORY_APP_URL");
            if (NULL != factoryAppUrl)
            {
                JsonObject launchRequest, launchResponse;
                launchRequest["callsign"] = "factoryapp";
                launchRequest["type"] = "LightningApp";
                launchRequest["uri"] = std::string(factoryAppUrl);
                launchRequest["focused"] = true;
                std::cout << "launching " << launchRequest["callsign"].String().c_str() << std::endl;
                status = rdkshellPlugin->Invoke<JsonObject, JsonObject>(EASTEREGG_THUNDER_TIMEOUT, "launch", launchRequest, launchResponse);
                bool launchFactoryResult = launchResponse.HasLabel("success")?launchResponse["success"].Boolean():false;
                if ((status > 0) || (false == launchFactoryResult))
                {
                    std::cout << "Launching factory application failed " << std::endl;
                    response["message"] = "launch factory app failed";
                    ret = false;
                }
                else
                {
                    std::cout << "Launching factory application succeeded " << std::endl;
                }
            }
            else
            {
                std::cout << "factory app url is empty " << std::endl;
                response["message"] = "factory application url is empty";
                ret = false;
            }
            returnResponse(ret);
        }

        uint32_t EasterEgg::launchFactoryAppShortcutWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            JsonObject joToFacParams;
            JsonObject joToFacResult;
            joToFacParams.Set("namespace","FactoryTest");
            joToFacParams.Set("key","ToFacFlag");
            std::string toFacGetInvoke = "org.rdk.PersistentStore.1.getValue";

            std::cout << "attempting to check flag \n";
            uint32_t status = getThunderControllerClient()->Invoke(EASTEREGG_THUNDER_TIMEOUT, toFacGetInvoke.c_str(), joToFacParams, joToFacResult);
            std::cout << "get status: " << status << std::endl;

            if (status > 0)
            {
                response["message"] = " unable to check toFac flag";
                returnResponse(false);
            }

            if (!joToFacResult.HasLabel("value"))
            {
                response["message"] = " toFac value not found";
                returnResponse(false);
            }

            const std::string valueString = joToFacResult["value"].String();
            if (valueString != "M" && valueString != "m")
            {
                std::cout << "toFac value is " << valueString << std::endl;
                response["message"] = " toFac not in the correct mode";
                returnResponse(false);
            }

            return launchFactoryAppWrapper(parameters, response);
        }

        uint32_t EasterEgg::launchResidentAppWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            mFactoryAppRunning = false;
            killAllApps();
            bool ret = true;
            std::string callsign("ResidentApp");
            JsonObject activateParams;
            activateParams.Set("callsign",callsign.c_str());
            JsonObject activateResult;
            int32_t status = getThunderControllerClient()->Invoke(3500, "activate", activateParams, activateResult);

            std::cout << "activate resident app status: " << status << std::endl;
            if (status > 0)
            {
                std::cout << "trying status one more time...\n";
                status = getThunderControllerClient()->Invoke(3500, "activate", activateParams, activateResult);
                std::cout << "activate resident app status: " << status << std::endl;
                if (status > 0)
                {
                    response["message"] = "resident app launch failed";
                    ret = false;
                }
                else
                {
                    ret = true;
                }
            }
            returnResponse(ret);
        }

        uint32_t EasterEgg::toggleFactoryAppWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool ret = true;
            if (mFactoryAppRunning)
            {
                launchResidentAppWrapper(parameters, response);
            }
            else
            {
                launchFactoryAppWrapper(parameters, response);
            }
            returnResponse(ret);
        }
    } // namespace Plugin
} // namespace WPEFramework
