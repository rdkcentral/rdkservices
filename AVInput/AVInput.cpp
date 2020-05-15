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

#include "AVInput.h"
#include "hdmiIn.hpp"

const short WPEFramework::Plugin::AVInput::API_VERSION_NUMBER_MAJOR = 1;
const short WPEFramework::Plugin::AVInput::API_VERSION_NUMBER_MINOR = 0;
const string WPEFramework::Plugin::AVInput::SERVICE_NAME = "org.rdk.AVInput";
//methods
const string WPEFramework::Plugin::AVInput::METHOD_GET_API_VERSION_NUMBER = "getApiVersionNumber";
const string WPEFramework::Plugin::AVInput::AVINPUT_METHOD_NUMBER_OF_INPUTS = "numberOfInputs";
const string WPEFramework::Plugin::AVInput::AVINPUT_METHOD_CURRENT_VIDEO_MODE = "currentVideoMode";
const string WPEFramework::Plugin::AVInput::AVINPUT_METHOD_CONTENT_PROTECTED = "contentProtected";
//events
const string WPEFramework::Plugin::AVInput::AVINPUT_EVENT_ON_AV_INPUT_ACTIVE = "onAVInputActive";
const string WPEFramework::Plugin::AVInput::AVINPUT_EVENT_ON_AV_INPUT_INACTIVE = "onAVInputInactive";

#define SUBSCRIPTION_CALLSIGN "org.rdk.HdmiInput"
#define SUBSCRIPTION_CALLSIGN_VER SUBSCRIPTION_CALLSIGN".1"
#define SUBSCRIPTION_EVENT "onDevicesChanged"
#define SERVER_DETAILS  "127.0.0.1:9998"
#define WARMING_UP_TIME_IN_SECONDS 5
#define RECONNECTION_TIME_IN_MILLISECONDS 5500

using namespace std;

namespace WPEFramework {
    namespace Plugin {

        SERVICE_REGISTRATION(AVInput, 1, 0);

        AVInput* AVInput::_instance = nullptr;

        AVInput::AVInput()
            : AbstractPlugin()
            , m_apiVersionNumber(API_VERSION_NUMBER_MAJOR)
            , m_client(nullptr)
            , m_subscribed(false)
        {
            LOGINFO("ctor");
            AVInput::_instance = this;
            registerMethod(METHOD_GET_API_VERSION_NUMBER, &AVInput::getApiVersionNumber, this);
            registerMethod(AVINPUT_METHOD_NUMBER_OF_INPUTS, &AVInput::numberOfInputsWrapper, this);
            registerMethod(AVINPUT_METHOD_CURRENT_VIDEO_MODE, &AVInput::currentVideoModeWrapper, this);
            registerMethod(AVINPUT_METHOD_CONTENT_PROTECTED, &AVInput::contentProtectedWrapper, this);

            m_timer.connect(std::bind(&AVInput::onTimer, this));
        }

        AVInput::~AVInput()
        {
            LOGINFO("dtor");
            AVInput::_instance = nullptr;
        }

        const string AVInput::Initialize(PluginHost::IShell* /* service */)
        {
            LOGINFO();

            if(m_timer.isActive()) {
                m_timer.stop();
            }

            activatePlugin(SUBSCRIPTION_CALLSIGN);
            LOGINFO("Starting the timer");
            m_timer.start(RECONNECTION_TIME_IN_MILLISECONDS);
            return "";
        }

        void AVInput::Deinitialize(PluginHost::IShell* /* service */)
        {
            LOGINFO();
        }

        string AVInput::Information() const
        {
            return(string("{\"service\": \"") + SERVICE_NAME + string("\"}"));
        }

        // Registered methods begin
        uint32_t AVInput::getApiVersionNumber(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            UNUSED(parameters);
            response["version"] = m_apiVersionNumber;
            returnResponse(true);
        }

        uint32_t AVInput::numberOfInputsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool success;
            if (getActivatedPluginReady(SUBSCRIPTION_CALLSIGN))
            {
                response["numberOfInputs"] = numberOfInputs(&success);
                response["message"] = "Success";
            } else {
                success = false;
                response["message"] = string(SUBSCRIPTION_CALLSIGN) + " plugin is not ready";
            }
            returnResponse(success);
        }

        uint32_t AVInput::currentVideoModeWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool success;
            if (getActivatedPluginReady(SUBSCRIPTION_CALLSIGN))
            {
                response["currentVideoMode"] = currentVideoMode();
                response["message"] = "Success";
            } else {
                success = false;
                response["message"] = string(SUBSCRIPTION_CALLSIGN) + " plugin is not ready";
            }
            returnResponse(success);
        }

        uint32_t AVInput::contentProtectedWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            response["isContentProtected"] = contentProtected();

            returnResponse(true);
        }
        // Registered methods begin

        // Events begin
        void AVInput::onAVInputActive(JsonObject& url)
        {
            LOGINFO();
            sendNotify(C_STR(AVINPUT_EVENT_ON_AV_INPUT_ACTIVE), url);
        }
        void AVInput::onAVInputInactive(JsonObject& url)
        {
            LOGINFO();
            sendNotify(C_STR(AVINPUT_EVENT_ON_AV_INPUT_INACTIVE), url);
        }
        // Events end

        // Internal methods begin
        int AVInput::numberOfInputs(bool *pSuccess)
        {
            int res = 0;
            LOGINFO("Invoking device::HdmiInput::getInstance().GetNumberOfInputs()");
            try
            {
                res = device::HdmiInput::getInstance().getNumberOfInputs();;
            }
            catch (...)
            {
                LOGERR("Exception caught");
                if (pSuccess) {
                    *pSuccess = false;
                }
            }
            if (pSuccess) {
                *pSuccess = true;
            }
            return res;
        }

        string AVInput::currentVideoMode(bool *pSuccess)
        {
            string res;
            LOGINFO("Invoking device::HdmiInput::getInstance().getCurrentVideoMode()");
            try
            {
                res  = device::HdmiInput::getInstance().getCurrentVideoMode();
            }
            catch (...)
            {
                LOGERR("Exception caught");
                if (pSuccess) {
                    *pSuccess = false;
                }
            }
            if (pSuccess) {
                *pSuccess = true;
            }

            return res;
        }

        bool AVInput::contentProtected()
        {
            // Ths is the way it's done in Service Manager
            // We can get the "content protected" attribute from HDCP, but does it cover the Hdmi In stream?
            return true;
        }

        // Thunder plugins communication
        std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> > AVInput::getThunderControllerClient()
        {
            // making function local static to be thread safe
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(SERVER_DETAILS)));
            static std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> > thunderClient = make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> >("", "");
            return thunderClient;
        }

        void AVInput::activatePlugin(const char* callSign)
        {
            JsonObject joParams;
            joParams.Set("callsign",callSign);
            JsonObject joResult;

            if(!isPluginActivated(callSign))
            {
                LOGINFO("Activating %s", callSign);
                // deactivatePlugin() would have "deactivate" as a command (plus, m_activatedPlugins should have an entry erased from it, see isPluginActivated())
                // setting wait Time to 2 seconds
                uint32_t status = getThunderControllerClient()->Invoke<JsonObject, JsonObject>(2000, "activate", joParams, joResult);
                string strParams;
                string strResult;
                joParams.ToString(strParams);
                joResult.ToString(strResult);
                LOGINFO("Called method %s, with params %s, status: %d, result: %s"
                        , "activate"
                        , C_STR(strParams)
                        , status
                        , C_STR(strResult));
                if (status == Core::ERROR_NONE)
                {
                    time_t endTime = time(NULL) + WARMING_UP_TIME_IN_SECONDS;
                    LOGINFO("Adding %s to the list of active plugins. Should be ready in about %d seconds", callSign, WARMING_UP_TIME_IN_SECONDS);
                    m_activatedPlugins[string(callSign)] = endTime;

                }
            }
        }

        bool AVInput::isPluginActivated(const char* callSign)
        {
            string method = "status@" + string(callSign);
            Core::JSON::ArrayType<PluginHost::MetaData::Service> joResult;
            getThunderControllerClient()->Get<Core::JSON::ArrayType<PluginHost::MetaData::Service> >(2000, method.c_str(),joResult);
            LOGINFO("Getting status for callSign %s, result: %s", callSign, joResult[0].JSONState.Data().c_str());
            bool pluginActivated = joResult[0].JSONState == PluginHost::IShell::ACTIVATED;
            if(!pluginActivated)
            {
                auto p = m_activatedPlugins.find(string(callSign));
                if (p != m_activatedPlugins.end())
                {
                    LOGWARN("Previoulsly active plugin %s appers to be deactivated, removing from the list", callSign);
                    m_activatedPlugins.erase(p);
                }
            }
            return pluginActivated;
        }

        bool AVInput::getActivatedPluginReady(const char* callSign)
        {
            bool res = false;

            auto p = m_activatedPlugins.find(string(callSign));
            if (p != m_activatedPlugins.end())
            {
                time_t endTime = p->second;
                time_t nowTime = time(NULL);
                time_t diffTime = endTime - nowTime;

                if (diffTime > 0) {
                    LOGINFO("Waiting about %ld second(s) for %s to warm up ", diffTime, callSign);
                    sleep(diffTime);
                    res = true;
                } else {
                    res = true;
                }
            } else {
                LOGERR("Plugin %s has not been activated yet. Call activatePlugin() first!", callSign);
            }
            return  res;
        }
        // Thunder plugins communication end

        // Event management
        // 1.
        uint32_t AVInput::subscribe(const char* callSignVer, const char* eventName)
        {
            uint32_t err = Core::ERROR_NONE;
            LOGINFO("Attempting to subscribe for event");
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(SERVER_DETAILS)));
            if (nullptr == m_client) {
                m_client = make_shared<WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>>(_T(callSignVer), (_T(callSignVer)));
                if (nullptr == m_client) {
                    LOGERR("JSONRPC: %s: client initialization failed", callSignVer);
                    err = Core::ERROR_UNAVAILABLE;
                } else {
                    /* Register handlers for Event reception. */
                    err =m_client->Subscribe<JsonObject>(1000, eventName
                            , &AVInput::onDevicesChangedEventHandler, this);
                    if ( err  == Core::ERROR_NONE) {
                        LOGINFO("Subscribed for %s", eventName);
                    } else {
                        LOGERR("Failed to subscribe for %s with code %d", eventName, err);
                    }
                }
            }
            return err;
        }

        // 2.
        void AVInput::onDevicesChangedEventHandler(const JsonObject& parameters) {
            JsonArray devices;
            string message;

            parameters.ToString(message);
            LOGINFO("[onDevicesChanged event], %s : %s", __FUNCTION__, C_STR(message));

            if (parameters.HasLabel("devices")) {
                devices = parameters["devices"].Array();

                for (int i = 0; i < devices.Length(); ++i)
                {
                    JsonObject device = devices[i].Object();
                    JsonObject url;
                    bool connected;
                    int id;

                    if (device.HasLabel("connected")) {
                        if(device["connected"].String() == "true") {
                            connected = true;
                        } else {
                            connected = false;
                        }
                    } else {
                        connected = false;
                        LOGERR("Field 'connected' could not be found in the event's payload. Assuming false");
                    }

                    if (device.HasLabel("id")) {
                        getNumberParameter("id", id);
                    } else {
                        id = 0;
                        LOGERR("Field 'id' could not be found in the event's payload. Assuming 0");
                    }

                    url["url"] = string("avin://input") + std::to_string(id);

                    if (connected) {
                        onAVInputActive(url);
                    } else {
                        onAVInputInactive(url);
                    }
                }
            } else {
                LOGERR("Field 'devices' could not be found in the event's payload.");
            }
        }

        // 3.
        void AVInput::onTimer()
        {
            std::lock_guard<std::mutex> guard(m_callMutex);
            LOGINFO();
            bool pluginActivated = isPluginActivated(SUBSCRIPTION_CALLSIGN);
            if(!m_subscribed) {
                if (pluginActivated && subscribe(SUBSCRIPTION_CALLSIGN_VER, SUBSCRIPTION_EVENT) == Core::ERROR_NONE)
                {
                    m_subscribed = true;
                    if (m_timer.isActive()) {
                        m_timer.stop();
                        LOGINFO("Timer stopped.");
                    }
                    LOGINFO("Subscription completed.");
                } else {
                    LOGERR("Could not subscribe this time, one more attempt in %d msec. Plugin is %s", RECONNECTION_TIME_IN_MILLISECONDS, pluginActivated ? "ACTIVE" : "BLOCKED");
                    if (!pluginActivated)
                    {
                        activatePlugin(SUBSCRIPTION_CALLSIGN);
                    }
                }
            } else {
                // Not supposed to be here
                LOGINFO("Already subscribed. Stopping the timer.");
                if (m_timer.isActive()) {
                    m_timer.stop();
                }
            }
        }
         // Event management end
         // Internal methods end
    } // namespace Plugin
} // namespace WPEFramework
