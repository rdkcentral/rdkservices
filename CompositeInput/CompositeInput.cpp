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
#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"

#include "compositeIn.hpp"
#include "exception.hpp"
#include "dsUtl.h"
#include "dsError.h"
#include "dsMgr.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 1
#define API_VERSION_NUMBER_PATCH 5

#define COMPOSITE_HOT_PLUG_EVENT_CONNECTED 1
#define COMPOSITE_HOT_PLUG_EVENT_DISCONNECTED 0

#define COMPOSITEINPUT_METHOD_GET_COMPOSITE_INPUT_DEVICES "getCompositeInputDevices"
#define COMPOSITEINPUT_METHOD_START_COMPOSITE_INPUT "startCompositeInput"
#define COMPOSITEINPUT_METHOD_STOP_COMPOSITE_INPUT "stopCompositeInput"
#define COMPOSITEINPUT_METHOD_SCALE_COMPOSITE_INPUT "setVideoRectangle"

#define COMPOSITEINPUT_EVENT_ON_DEVICES_CHANGED "onDevicesChanged"
#define COMPOSITEINPUT_EVENT_ON_SIGNAL_CHANGED "onSignalChanged"
#define COMPOSITEINPUT_EVENT_ON_STATUS_CHANGED "onInputStatusChanged"
#define COMPOSITEINPUT_EVENT_ON_VIDEO_MODE_UPDATED "videoStreamInfoUpdate"

namespace WPEFramework
{
    namespace {

        static Plugin::Metadata<Plugin::CompositeInput> metadata(
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
    
    namespace Plugin
    {
        SERVICE_REGISTRATION(CompositeInput, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        CompositeInput* CompositeInput::_instance = nullptr;

        CompositeInput::CompositeInput()
        : PluginHost::JSONRPC()
        {
            CompositeInput::_instance = this;

            //InitializeIARM();

            Register(COMPOSITEINPUT_METHOD_GET_COMPOSITE_INPUT_DEVICES, &CompositeInput::getCompositeInputDevicesWrapper, this);
            Register(COMPOSITEINPUT_METHOD_START_COMPOSITE_INPUT, &CompositeInput::startCompositeInput, this);
            Register(COMPOSITEINPUT_METHOD_STOP_COMPOSITE_INPUT, &CompositeInput::stopCompositeInput, this);
            Register(COMPOSITEINPUT_METHOD_SCALE_COMPOSITE_INPUT, &CompositeInput::setVideoRectangleWrapper, this);
        }

        CompositeInput::~CompositeInput()
        {
        }
		const string CompositeInput::Initialize(PluginHost::IShell * /* service */)
        {
            CompositeInput::_instance = this;
            InitializeIARM();
            return (string());
        }

        void CompositeInput::Deinitialize(PluginHost::IShell* /* service */)
        {
            CompositeInput::_instance = nullptr;

            DeinitializeIARM();
        }

        void CompositeInput::InitializeIARM()
        {
            if (Utils::IARM::init())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_HOTPLUG, dsCompositeEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_SIGNAL_STATUS, dsCompositeSignalStatusEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_STATUS, dsCompositeStatusEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_VIDEO_MODE_UPDATE,dsCompositeVideoModeEventHandler) );
            }
        }

        void CompositeInput::DeinitializeIARM()
        {
            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_HOTPLUG, dsCompositeEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_SIGNAL_STATUS, dsCompositeSignalStatusEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_STATUS, dsCompositeStatusEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_VIDEO_MODE_UPDATE, dsCompositeVideoModeEventHandler) );
            }
        }

        uint32_t CompositeInput::startCompositeInput(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            returnIfParamNotFound(parameters, "portId");

            string sPortId = parameters["portId"].String();
            int portId = 0;
            try {
                portId = stoi(sPortId);
            }catch (const std::exception& err) {
                LOGERR("Failed to  portId value..!");
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
            LOGINFOMETHOD();

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
            LOGINFOMETHOD();

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
            LOGINFOMETHOD();

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
            catch (const std::exception& e)  {
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

        /**
         * @brief This function is used to translate Composite input signal change
         * to onSignalChanged event.
         *
         * @param[in] port Composite In port id.
         * @param[in] signalStatus signal status of Composite In port.
         */
        void CompositeInput::compositeInputSignalChange( int port , int signalStatus)
        {
            LOGWARN("compositeInputSignalChange [%d, %d]", port, signalStatus);

            JsonObject params;
            params["id"] = port;
            std::stringstream locator;
            locator << "cvbsin://localhost/deviceid/" << port;
            params["locator"] = locator.str();

	    switch (signalStatus) {
		    case dsCOMP_IN_SIGNAL_STATUS_NOSIGNAL:
			    params["signalStatus"] = "noSignal";
			    break;

	            case dsCOMP_IN_SIGNAL_STATUS_UNSTABLE:
			    params["signalStatus"] = "unstableSignal";
			    break;

                    case dsCOMP_IN_SIGNAL_STATUS_NOTSUPPORTED:
                            params["signalStatus"] = "notSupportedSignal";
                            break;

                    case dsCOMP_IN_SIGNAL_STATUS_STABLE:
                            params["signalStatus"] = "stableSignal";
                            break;

	            default:
                            params["signalStatus"] = "none";
                            break;
            }

            sendNotify(COMPOSITEINPUT_EVENT_ON_SIGNAL_CHANGED, params);
        }

        /**
         * @brief This function is used to translate Composite input status change to
         * inputStatusChanged event.
         *
         * @param[in] port Composite In port id.
         * @param[bool] isPresented Composite In presentation started/stopped.
         */
        void CompositeInput::compositeInputStatusChange( int port , bool isPresented)
        {
            LOGWARN("compositeInputStatusChange [%d, %d]", port, isPresented);

            JsonObject params;
            params["id"] = port;
            std::stringstream locator;
            locator << "cvbsin://localhost/deviceid/" << port;
            params["locator"] = locator.str();

            if(isPresented) {
                params["status"] = "started";
            }
            else {
                params["status"] = "stopped";
            }

            sendNotify(COMPOSITEINPUT_EVENT_ON_STATUS_CHANGED, params);
        }
	          /**
         * @brief This function is used to translate Composite input video mode change to
         * videoStreamInfoUpdate event.
         *
         * @param[in] port Composite In port id.
         * @param[dsVideoPortResolution_t] video resolution data
         */
        void CompositeInput::compositeInputVideoModeUpdate( int port , dsVideoPortResolution_t resolution)
        {
            LOGWARN("compositeInputVideoModeUpdate [%d]", port);

            JsonObject params;
            params["id"] = port;
            std::stringstream locator;
            locator << "cvbsin://localhost/deviceid/" << port;
            params["locator"] = locator.str();

	        switch(resolution.pixelResolution) {
				case dsVIDEO_PIXELRES_720x480:
				params["width"] = 720;
				params["height"] = 480;
				break;
			case dsVIDEO_PIXELRES_720x576:
				params["width"] = 720;
				params["height"] = 576;
				break;
		   default:
				params["width"] = 720;
				params["height"] = 576;
				break;
		   }

			params["progressive"] = false;

            switch(resolution.frameRate) {
                    case dsVIDEO_FRAMERATE_24:
                        params["frameRateN"] = 24000;
                        params["frameRateD"] = 1000;
                        break;

                    case dsVIDEO_FRAMERATE_25:
                        params["frameRateN"] = 25000;
                        params["frameRateD"] = 1000;
                        break;

                    case dsVIDEO_FRAMERATE_30:
                        params["frameRateN"] = 30000;
                        params["frameRateD"] = 1000;
                        break;

                    case dsVIDEO_FRAMERATE_50:
                        params["frameRateN"] = 50000;
                        params["frameRateD"] = 1000;
                        break;

                    case dsVIDEO_FRAMERATE_60:
                        params["frameRateN"] = 60000;
                        params["frameRateD"] = 1000;
                        break;

                    case dsVIDEO_FRAMERATE_23dot98:
                        params["frameRateN"] = 24000;
                        params["frameRateD"] = 1001;
                        break;

                    case dsVIDEO_FRAMERATE_29dot97:
                        params["frameRateN"] = 30000;
                        params["frameRateD"] = 1001;
                        break;

                    case dsVIDEO_FRAMERATE_59dot94:
                        params["frameRateN"] = 60000;
                        params["frameRateD"] = 1001;
                        break;

                    default:
                        params["frameRateN"] = 60000;
                        params["frameRateD"] = 1000;
                        break;
            }

            sendNotify(COMPOSITEINPUT_EVENT_ON_VIDEO_MODE_UPDATED, params);
        }

        void CompositeInput::dsCompositeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
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

        void CompositeInput::dsCompositeSignalStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if(!CompositeInput::_instance)
                return;

            if (IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_SIGNAL_STATUS == eventId)
            {
                IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                int composite_in_port = eventData->data.composite_in_sig_status.port;
                int composite_in_signal_status = eventData->data.composite_in_sig_status.status;
                LOGWARN("Received IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_SIGNAL_STATUS  event  port: %d, signal status: %d", composite_in_port,composite_in_signal_status);

                CompositeInput::_instance->compositeInputSignalChange(composite_in_port, composite_in_signal_status);
            }
        }

        void CompositeInput::dsCompositeStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if(!CompositeInput::_instance)
                return;

            if (IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_STATUS == eventId)
            {
                IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                int composite_in_port = eventData->data.composite_in_status.port;
                bool composite_in_status = eventData->data.composite_in_status.isPresented;
                LOGWARN("Received IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_STATUS  event  port: %d, started: %d", composite_in_port,composite_in_status);

                CompositeInput::_instance->compositeInputStatusChange(composite_in_port, composite_in_status);
            }
        }
	 void CompositeInput::dsCompositeVideoModeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
 {
		if(!CompositeInput::_instance)
			return;

		if (IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_VIDEO_MODE_UPDATE == eventId)
		{
			IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
			int composite_in_port = eventData->data.composite_in_video_mode.port;
			dsVideoPortResolution_t resolution = {};
			resolution.pixelResolution =  eventData->data.composite_in_video_mode.resolution.pixelResolution;
			resolution.interlaced =  eventData->data.composite_in_video_mode.resolution.interlaced;
			resolution.frameRate =  eventData->data.composite_in_video_mode.resolution.frameRate;
			LOGWARN("Received IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_VIDEO_MODE_UPDATE  event  port: %d, pixelResolution: %d, interlaced : %d, frameRate: %d \n", composite_in_port,resolution.pixelResolution, resolution.interlaced, resolution.frameRate);

			CompositeInput::_instance->compositeInputVideoModeUpdate(composite_in_port, resolution);

		}
  }

    } // namespace Plugin
} // namespace WPEFramework
