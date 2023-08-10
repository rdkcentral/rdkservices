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

#include "NetworkManagerPluginImplementation.h"

#include "adapter/nm_adapter.h"
#include "adapter/iarm/iarm_adapter.h"
#include <random>
#include "UtilsJsonRpc.h"
#include "UtilsLOG_MILESTONE.h"
#include "UtilsUnused.h"
#include "UtilsgetRFCConfig.h"
#include "UtilsString.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;

namespace WPEC = WPEFramework::Core;
namespace WPEJ = WPEFramework::Core::JSON;

namespace WPEFramework
{
    namespace Plugin
    {
        SERVICE_REGISTRATION(NetworkManagerPluginImplementation, 1, 0);

        NetworkManagerPluginImplementation::NetworkManagerPluginImplementation()
            : _notificationCallbacks({}),
              _greetings({"Hello", "Goodbye", "Hi", "Bye", "Good Morning", "Good Afternoon"}) // The different greeting messages we can generate
        {
        }

        NetworkManagerPluginImplementation::~NetworkManagerPluginImplementation()
        {
        }

        /**
         * The actual method we want to implement from the interface
         *
         * If running out of process, this will run in the out-of-process part and return
         * the result over COM-RPC
         *
         * Generate a greeting
         *
         * @param[in] message   Who the greeting is for
         * @param[out] result   The generated greeting
         */
        uint32_t NetworkManagerPluginImplementation::Greet(const string &message, string &result /* @out */)
        {
            TRACE(Trace::Information, (_T("Generating greeting")));
            TRACE(Trace::Information, (_T("Running in process %d"), Core::ProcessInfo().Id()));

            // Pick a random greeting from the pre-determined list
            std::random_device dev;
            std::mt19937 rng(dev());
            std::uniform_int_distribution<std::mt19937::result_type> distribution(0, _greetings.size() - 1);

            std::string greeting = _greetings[distribution(rng)];

            // Build the final message
            result = greeting + ", " + message;

            // Send out a notification that we generated a greeting
            // Loop through all the registered callbacks and fire off the notification
            std::lock_guard<std::mutex> locker(_notificationMutex);
            TRACE(Trace::Information, (_T("We have %d callbacks to trigger"), _notificationCallbacks.size()));
            for (const auto callback : _notificationCallbacks)
            {
                callback->SomethingHappend(INetworkManagerPlugin::INotification::EXCITING_THING_HAPPENED);
            }

            // All good - return success
            return Core::ERROR_NONE;
        }


        /**
         * A simple echo method used for benchmarking. Returns the data it was given
         * without modification
         *
         * @param[in] message   Message to be echo'd
         * @param[out] result   Echo'd data
         */
        uint32_t NetworkManagerPluginImplementation::Echo(const string &message, string &result /* @out */)
        {
            // Just return exactly what we were sent
            result = message;

            // All good - return success
            return Core::ERROR_NONE;
        }


        uint32_t NetworkManagerPluginImplementation::isConnectedToInternet(const string &message, string &result /* @out */)
        {
            NetworkmanagerAdapter *ptr = new iarmAdapter;
            ptr->IsConnectedToInternet(message, result);

            result = message;
            // All good - return success
            return Core::ERROR_NONE;
        }

//      uint32_t NetworkManagerPluginImplementation::getIPSettings(const GetIpSettingsParamsData& parameters, JsonObject& response)
        uint32_t NetworkManagerPluginImplementation::getIPSettings(const string &message, string &result /* @out */)
        {
            //string params,result;
            JsonObject  response;
            JsonObject parameters;
            NetworkmanagerAdapter *ptr = new iarmAdapter;
            //parameters.FromString(message);
            //ptr->getIPSettings(params, result);
            //ptr->getIPSettings(parameters, response);
/*            WPEC::OptionalType<WPEJ::Error> error;
            if (!WPEJ::IElement::FromString(message, params, error)) {
                LOGERR("Failed to parse JSON document containing SSIDs. Due to: %s", WPEJ::ErrorDisplayMessage(error).c_str());
                return Core::ERROR_NONE;
            }*/
            //WPEFramework::Core::JSON::IElement error;
            parameters.FromString(message);

            ptr->getIPSettings(parameters, response);
            response.ToString(result);
            //result = message;
            // All good - return success
            return Core::ERROR_NONE;
        }

        /**
         * Register a notification callback
         */
        uint32_t NetworkManagerPluginImplementation::Register(INetworkManagerPlugin::INotification *notification)
        {
            std::lock_guard<std::mutex> locker(_notificationMutex);

            // Make sure we can't register the same notification callback multiple times
            if (std::find(_notificationCallbacks.begin(), _notificationCallbacks.end(), notification) == _notificationCallbacks.end())
            {
                TRACE(Trace::Information, (_T("Added a callback")));
                _notificationCallbacks.push_back(notification);
                notification->AddRef();
            }

            return Core::ERROR_NONE;
        }

        /**
         * Unregister a notification callback
         */
        uint32_t NetworkManagerPluginImplementation::Unregister(INetworkManagerPlugin::INotification *notification)
        {
            std::lock_guard<std::mutex> locker(_notificationMutex);

            // Make sure we can't register the same notification callback multiple times
            auto itr = std::find(_notificationCallbacks.begin(), _notificationCallbacks.end(), notification);
            if (itr != _notificationCallbacks.end())
            {
                (*itr)->Release();
                _notificationCallbacks.erase(itr);
            }

            return Core::ERROR_NONE;
        }
    }
}
