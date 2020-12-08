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

#include "CompositeInput.h"
#include "utils.h"

#include "compositeIn.hpp"
#include "exception.hpp"
#include "dsUtl.h"
#include "dsError.h"
#include "dsMgr.h"

#define COMPOSITE_HOT_PLUG_EVENT_CONNECTED 0
#define COMPOSITE_HOT_PLUG_EVENT_DISCONNECTED 1

#define COMPOSITEINPUT_METHOD_GET_COMPOSITE_INPUT_DEVICES "getCompositeInputDevices"
#define COMPOSITEINPUT_METHOD_START_COMPOSITE_INPUT "startCompositeInput"
#define COMPOSITEINPUT_METHOD_STOP_COMPOSITE_INPUT "stopCompositeInput"
#define COMPOSITEINPUT_METHOD_SCALE_COMPOSITE_INPUT "setVideoRectangle"

#define COMPOSITEINPUT_EVENT_ON_DEVICES_CHANGED "onDevicesChanged"

namespace WPEFramework
{
    namespace Plugin
    {
        SERVICE_REGISTRATION(CompositeInput, 1, 0);

        CompositeInput* CompositeInput::_instance = nullptr;

        CompositeInput::CompositeInput()
        : AbstractPlugin()
        {
            LOGINFO();
            CompositeInput::_instance = this;

            InitializeIARM();

            registerMethod(COMPOSITEINPUT_METHOD_GET_COMPOSITE_INPUT_DEVICES, &CompositeInput::getCompositeInputDevicesWrapper, this);
            registerMethod(COMPOSITEINPUT_METHOD_START_COMPOSITE_INPUT, &CompositeInput::startCompositeInput, this);
            registerMethod(COMPOSITEINPUT_METHOD_STOP_COMPOSITE_INPUT, &CompositeInput::stopCompositeInput, this);
            registerMethod(COMPOSITEINPUT_METHOD_SCALE_COMPOSITE_INPUT, &CompositeInput::setVideoRectangleWrapper, this);
        }

        CompositeInput::~CompositeInput()
        {
            LOGINFO();
            CompositeInput::_instance = nullptr;

            DeinitializeIARM();
        }

        void CompositeInput::InitializeIARM()
        {
            LOGINFO();

            if (Utils::IARM::init())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_HOTPLUG, dsCompositeEventHandler) );
            }
        }

        void CompositeInput::DeinitializeIARM()
        {
            LOGINFO();

            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_HOTPLUG) );
            }
        }

        uint32_t CompositeInput::startCompositeInput(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();
            returnIfParamNotFound(parameters, "portId");

            string sPortId = parameters["portId"].String();
            int portId = 0;
            try {
                portId = stoi(sPortId);
            }catch (const device::Exception& err) {
                LOG_DEVICE_EXCEPTION1(sPortId);
                returnResponse(false);
            }

            bool success = true;
            try
            {
                device::CompositeInput::getInstance().selectPort(portId);
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(sPortId);
                success = false;
            }
            returnResponse(success);

        }

        uint32_t CompositeInput::stopCompositeInput(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            bool success = true;
            try
            {
                device::CompositeInput::getInstance().selectPort(-1);
            }
            catch (const device::Exception& err)
            {
                LOGWARN("CompositeInputService::stopCompositeInput Failed");
                success = false;
            }
            returnResponse(success);

        }

        uint32_t CompositeInput::setVideoRectangleWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            bool result = true;
            if (!parameters.HasLabel("x") && !parameters.HasLabel("y"))
            {
                result = false;
                response["message"] = "please specif coordinates (x,y)";
            }

            if (!parameters.HasLabel("w") && !parameters.HasLabel("h"))
            {
                result = false;
                response["message"] = "please specify window width and height (w,h)";
            }

            if (result)
            {
                int x = 0;
                int y = 0;
                int w = 0;
                int h = 0;

                try
		{
		       if (parameters.HasLabel("x"))
                       {
                           x = std::stoi(parameters["x"].String());
                       }
                       if (parameters.HasLabel("y"))
                       {
                           y = std::stoi(parameters["y"].String());
                       }
                       if (parameters.HasLabel("w"))
                       {
                           w = std::stoi(parameters["w"].String());
                       }
                       if (parameters.HasLabel("h"))
                       {
                           h = std::stoi(parameters["h"].String());
                       }
		}
                catch (...) {
		    LOGWARN("Invalid Arguments");
		    response["message"] = "Invalid Arguments";
		    returnResponse(false);
                }

                result = setVideoRectangle(x, y, w, h);
                if (false == result) {
                  LOGWARN("CompositeInputService::setVideoRectangle Failed");
                  response["message"] = "failed to set scale";
                }
            }

            returnResponse(result);

        }

        bool CompositeInput::setVideoRectangle(int x, int y, int width, int height)
        {
            bool ret = true;

            try
            {
                device::CompositeInput::getInstance().scaleVideo(x, y, width, height);
            }
            catch (const device::Exception& err)
            {
                ret = false;
            }

            return ret;
        }

        uint32_t CompositeInput::getCompositeInputDevicesWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            response["devices"] = getCompositeInputDevices();

            returnResponse(true);
        }

        JsonArray CompositeInput::getCompositeInputDevices()
        {
            JsonArray list;
            try
            {
                int num = device::CompositeInput::getInstance().getNumberOfInputs();
                if (num > 0) {
                    int i = 0;
                    for (i = 0; i < num; i++) {
                        //Input ID is aleays 0-indexed, continuous number starting 0
                        JsonObject hash;
                        hash["id"] = i;
                        std::stringstream locator;
                        locator << "cvbsin://localhost/deviceid/" << i;
                        hash["locator"] = locator.str();
                        hash["connected"] = device::CompositeInput::getInstance().isPortConnected(i) ? "true" : "false";
                        LOGWARN("CompositeInputService::getCompositeInputDevices id %d, locator=[%s], connected=[%s]", i, hash["locator"].String().c_str(), hash["connected"].String().c_str());
                        list.Add(hash);
                    }
                }
            }
            catch (const std::exception e)  {
                LOGWARN("CompositeInputService::getCompositeInputDevices Failed");
            }

            return list;
        }


        /**
         * @brief This function is used to translate COMPOSITE input hotplug to 
         * deviceChanged event.
         *
         * @param[in] input Number of input port integer.
         * @param[in] connection status of input port integer.
         */
        void CompositeInput::compositeInputHotplug( int input , int connect)
        {
            LOGWARN("compositeInputHotplug [%d, %d]", input, connect);

            JsonObject params;
            params["devices"] = getCompositeInputDevices();
            sendNotify(COMPOSITEINPUT_EVENT_ON_DEVICES_CHANGED, params);
        }

        void CompositeInput::dsCompositeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            LOGINFO();

            if(!CompositeInput::_instance)
                return;

            if (IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_HOTPLUG == eventId)
            {
                IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                int compositein_hotplug_port = eventData->data.composite_in_connect.port;
                int compositein_hotplug_conn = eventData->data.composite_in_connect.isPortConnected;
                LOGWARN("Received IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_HOTPLUG  event data:%d", compositein_hotplug_port);

                CompositeInput::_instance->compositeInputHotplug(compositein_hotplug_port, compositein_hotplug_conn ? COMPOSITE_HOT_PLUG_EVENT_CONNECTED : COMPOSITE_HOT_PLUG_EVENT_DISCONNECTED);
            }
        }

    } // namespace Plugin
} // namespace WPEFramework
