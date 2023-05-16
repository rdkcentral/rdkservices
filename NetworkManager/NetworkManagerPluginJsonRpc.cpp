/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2022 RDK Management
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

#include "NetworkManagerPlugin.h"

#define NETWORKMANAGER_METHOD_GREETER "greeter"
#define NETWORKMANAGER_METHOD_ECHO "echo"
#define NETWORKMANAGER_METHOD_ISCONNECTEDTOINTERNET "isConnectedToInternet"
#define NETWORKMANAGER_METHOD_GETIPSETTINGS "getIPSettings"

namespace WPEFramework
{
    namespace Plugin
    {
        using namespace JsonData::NetworkManagerPlugin;

        /**
         * Hook up all our JSON RPC methods
         *
         * Each method definition comprises of:
         *  * Input parameters
         *  * Output parameters
         *  * Method name
         *  * Function that implements that method
         */
        void NetworkManagerPlugin::RegisterAllMethods()
        {
            JSONRPC::Register<GreeterParamsData, GreeterResultData>(_T(NETWORKMANAGER_METHOD_GREETER), &NetworkManagerPlugin::Greeter, this);
            JSONRPC::Register<EchoParamsData, EchoResultData>(_T(NETWORKMANAGER_METHOD_ECHO), &NetworkManagerPlugin::Echo, this);
            JSONRPC::Register<EchoParamsData, EchoResultData>(_T(NETWORKMANAGER_METHOD_ISCONNECTEDTOINTERNET), &NetworkManagerPlugin::isConnectedToInternet, this);
            JSONRPC::Register<GetIPSettingsParamsData,GetIPSettingsResultData >(_T(NETWORKMANAGER_METHOD_GETIPSETTINGS), &NetworkManagerPlugin::getIPSettings, this);
//            JSONRPC::Register<JsonObjectParamsData, JsonObjectResultData>(_T(NETWORKMANAGER_METHOD_GETIPSETTINGS), &NetworkManagerPlugin::getIPSettings, this);
        }

        /**
         * Unregister all our JSON-RPC methods
         */
        void NetworkManagerPlugin::UnregisterAllMethods()
        {
            JSONRPC::Unregister(_T(NETWORKMANAGER_METHOD_GREETER));
            JSONRPC::Unregister(_T(NETWORKMANAGER_METHOD_ECHO));
        }

        /**
         * JSON-RPC wrapper around the Greeter() method the plugin implements
         *
         * Do no actual work here, this should just deal with validating the incoming JSON, then
         * call out to the actual implementation over COM-RPC, then build the response JSON
         *
         */
        uint32_t NetworkManagerPlugin::Greeter(const GreeterParamsData &params, GreeterResultData &response)
        {
            uint32_t errorCode = Core::ERROR_NONE;

            // For debugging
            std::string paramsString;
            params.ToString(paramsString);
            TRACE(Trace::Information, (_T("Incoming JSON-RPC request for greeter method with input params %s"), paramsString.c_str()));

            // Validate incoming JSON data
            if (!params.Message.IsSet() || params.Message.Value().empty())
            {
                TRACE(Trace::Error, (_T("Message cannot be null")));
                return Core::ERROR_BAD_REQUEST;
            }

            // Invoke the actual method to do the work
            std::string result;
            errorCode = _networkManagerPlugin->Greet(params.Message.Value(), result);

            // Build the response JSON object
            if (errorCode == Core::ERROR_NONE)
            {
                response.Success = true;
                response.Greeting = result;
            }
            else
            {
                response.Success = false;
            }

            return errorCode;
        }


        /**
         * JSON-RPC wrapper around the Echo() method the plugin implements
         *
         */
        uint32_t NetworkManagerPlugin::Echo(const  EchoParamsData &params, EchoResultData &response)
        {
            uint32_t errorCode = Core::ERROR_NONE;

            // Validate incoming JSON data
            if (!params.Data.IsSet() || params.Data.Value().empty())
            {
                TRACE(Trace::Error, (_T("Message cannot be null")));
                return Core::ERROR_BAD_REQUEST;
            }

            // Invoke the actual method to do the work
            std::string result;
            errorCode = _networkManagerPlugin->Echo(params.Data.Value(), result);

            // Build the response JSON object
            if (errorCode == Core::ERROR_NONE)
            {
                response.Success = true;
                response.Data = result;
            }
            else
            {
                response.Success = false;
            }

            return errorCode;
        }

        /**
         * JSON-RPC wrapper around the Echo() method the plugin implements
         */
        uint32_t NetworkManagerPlugin::isConnectedToInternet(const  EchoParamsData &params, EchoResultData &response)
        {
            uint32_t errorCode = Core::ERROR_NONE;

            // Validate incoming JSON data
            if (!params.Data.IsSet() || params.Data.Value().empty())
            {
                TRACE(Trace::Error, (_T("Message cannot be null")));
                return Core::ERROR_BAD_REQUEST;
            }

            // Invoke the actual method to do the work
            std::string result;
            errorCode = _networkManagerPlugin->isConnectedToInternet(params.Data.Value(), result);

            // Build the response JSON object
            if (errorCode == Core::ERROR_NONE)
            {
                response.Success = true;
                response.Data = result;
            }
            else
            {
                response.Success = false;
            }

            return errorCode;
        }

        uint32_t NetworkManagerPlugin::getIPSettings(const GetIPSettingsParamsData& parameters, GetIPSettingsResultData& response)
        {
            uint32_t errorCode = Core::ERROR_NONE;

            // Invoke the actual method to do the work
            //std::string result;
            string params, result;
            parameters.ToString(params);
            errorCode = _networkManagerPlugin->getIPSettings(params, result);

/*            // Build the response JSON object
            if (errorCode == Core::ERROR_NONE)
            {
                response.Success = true;
                response.Data = result;
            }
            else
            {
                response.Success = false;
            }
*/
            return errorCode;
        }


    }
}
