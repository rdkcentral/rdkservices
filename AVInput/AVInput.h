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

#pragma once

#include <mutex>
#include "Module.h"
#include "utils.h"
#include "AbstractPlugin.h"
#include <websocket/websocket.h>
#include "tptimer.h"

namespace WPEFramework {

    namespace Plugin {

        class AVInput :  public AbstractPlugin {
        public:
            AVInput();
            virtual ~AVInput();
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override;

        public/*members*/:
            static AVInput* _instance;

        public /*constants*/:
            static const short API_VERSION_NUMBER_MAJOR;
            static const short API_VERSION_NUMBER_MINOR;
            static const string SERVICE_NAME;
            //methods
            static const string METHOD_GET_API_VERSION_NUMBER;
            static const string AVINPUT_METHOD_NUMBER_OF_INPUTS;
            static const string AVINPUT_METHOD_CURRENT_VIDEO_MODE;
            static const string AVINPUT_METHOD_CONTENT_PROTECTED;
            //events
            static const string AVINPUT_EVENT_ON_AV_INPUT_ACTIVE;
            static const string AVINPUT_EVENT_ON_AV_INPUT_INACTIVE;

        private/*registered methods*/:
            // Note: `JsonObject& parameters` corresponds to `params` in JSON RPC call
            //methods
            uint32_t getApiVersionNumber(const JsonObject& parameters, JsonObject& response);
            uint32_t numberOfInputsWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t currentVideoModeWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t contentProtectedWrapper(const JsonObject& parameters, JsonObject& response);
            //Events
            void onAVInputActive(JsonObject& url);
            void onAVInputInactive(JsonObject& url);
            void onDevicesChangedEventHandler(const JsonObject& parameters);

        private/*internal methods*/:
            AVInput(const AVInput&) = delete;
            AVInput& operator=(const AVInput&) = delete;

            std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> getThunderControllerClient();
            void activatePlugin(const char* callSign);
            bool isPluginActivated(const char* callSign);
            bool getActivatedPluginReady(const char* callSign);

            int numberOfInputs(bool* pSuccess = nullptr);
            string currentVideoMode(bool* pSuccess = nullptr);
            bool contentProtected();

            uint32_t subscribe(const char* callSignVer, const char* eventName);
            void onTimer();

        private/*members*/:
            uint32_t m_apiVersionNumber;
            std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> > m_client;
            TpTimer m_timer;
            // A plugin we're depending on may require some time to warm up after activation; readyTimeInTicks is an estimation of that moment
            std::map<std::string/*callSign*/, time_t/*readyTime*/> m_activatedPlugins;
            bool m_subscribed;
            std::mutex m_callMutex;
        };
    } // namespace Plugin
} // namespace WPEFramework
