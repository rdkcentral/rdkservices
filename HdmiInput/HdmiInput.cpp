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

#include "HdmiInput.h"
#include "utils.h"

#include "hdmiIn.hpp"
#include "exception.hpp"
#include "dsUtl.h"
#include "dsError.h"
#include "dsMgr.h"

#define HDMI_HOT_PLUG_EVENT_CONNECTED 0
#define HDMI_HOT_PLUG_EVENT_DISCONNECTED 1

#define HDMIINPUT_METHOD_GET_HDMI_INPUT_DEVICES "getHDMIInputDevices"
#define HDMIINPUT_METHOD_WRITE_EDID "writeEDID"
#define HDMIINPUT_METHOD_READ_EDID "readEDID"
#define HDMIINPUT_METHOD_START_HDMI_INPUT "startHdmiInput"
#define HDMIINPUT_METHOD_STOP_HDMI_INPUT "stopHdmiInput"
#define HDMIINPUT_METHOD_SCALE_HDMI_INPUT "setVideoRectangle"

#define HDMIINPUT_EVENT_ON_DEVICES_CHANGED "onDevicesChanged"
#define HDMIINPUT_EVENT_ON_SIGNAL_CHANGED "onSignalChanged"
#define HDMIINPUT_EVENT_ON_STATUS_CHANGED "onInputStatusChanged"

namespace WPEFramework
{
    namespace Plugin
    {
        SERVICE_REGISTRATION(HdmiInput, 1, 0);

        HdmiInput* HdmiInput::_instance = nullptr;

        HdmiInput::HdmiInput()
        : AbstractPlugin()
        {
            HdmiInput::_instance = this;

            InitializeIARM();

            registerMethod(HDMIINPUT_METHOD_GET_HDMI_INPUT_DEVICES, &HdmiInput::getHDMIInputDevicesWrapper, this);
            registerMethod(HDMIINPUT_METHOD_WRITE_EDID, &HdmiInput::writeEDIDWrapper, this);
            registerMethod(HDMIINPUT_METHOD_READ_EDID, &HdmiInput::readEDIDWrapper, this);
            registerMethod(HDMIINPUT_METHOD_START_HDMI_INPUT, &HdmiInput::startHdmiInput, this);
            registerMethod(HDMIINPUT_METHOD_STOP_HDMI_INPUT, &HdmiInput::stopHdmiInput, this);
            registerMethod(HDMIINPUT_METHOD_SCALE_HDMI_INPUT, &HdmiInput::setVideoRectangleWrapper, this);
        }

        HdmiInput::~HdmiInput()
        {
            HdmiInput::_instance = nullptr;

            DeinitializeIARM();
        }

        void HdmiInput::InitializeIARM()
        {
            if (Utils::IARM::init())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG, dsHdmiEventHandler) );
		IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_SIGNAL_STATUS, dsHdmiSignalStatusEventHandler) );
		IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS, dsHdmiStatusEventHandler) );
            }
        }

        void HdmiInput::DeinitializeIARM()
        {
            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG) );
		IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_SIGNAL_STATUS) );
		IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS) );
            }
        }

        uint32_t HdmiInput::startHdmiInput(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
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
                device::HdmiInput::getInstance().selectPort(portId);
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(sPortId);
                success = false;
            }
            returnResponse(success);

        }

        uint32_t HdmiInput::stopHdmiInput(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success = true;
            try
            {
                device::HdmiInput::getInstance().selectPort(-1);
            }
            catch (const device::Exception& err)
            {
                LOGWARN("HdmiInputService::stopHdmiInput Failed");
                success = false;
            }
            returnResponse(success);

        }

        uint32_t HdmiInput::setVideoRectangleWrapper(const JsonObject& parameters, JsonObject& response)
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
                  LOGWARN("HdmiInputService::setVideoRectangle Failed");
                  response["message"] = "failed to set scale";
                }
            }

            returnResponse(result);

        }

        bool HdmiInput::setVideoRectangle(int x, int y, int width, int height)
        {
            bool ret = true;

            try
            {
                device::HdmiInput::getInstance().scaleVideo(x, y, width, height);
            }
            catch (const device::Exception& err)
            {
                ret = false;
            }

            return ret;
        }

        uint32_t HdmiInput::getHDMIInputDevicesWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            response["devices"] = getHDMIInputDevices();

            returnResponse(true);
        }

        uint32_t HdmiInput::writeEDIDWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            int deviceId;
            std::string message;

            if (parameters.HasLabel("deviceId") && parameters.HasLabel("message"))
            {
                getNumberParameter("deviceId", deviceId);
                message = parameters["message"].String();
            }
            else
            {
                LOGWARN("Required parameters are not passed");
                returnResponse(false);
            }


            writeEDID(deviceId, message);
            returnResponse(true);

        }

        uint32_t HdmiInput::readEDIDWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            response["name"] = readEDID();

            returnResponse(true);
        }

        JsonArray HdmiInput::getHDMIInputDevices()
        {
            JsonArray list;
            try
            {
                int num = device::HdmiInput::getInstance().getNumberOfInputs();
                if (num > 0) {
                    int i = 0;
                    for (i = 0; i < num; i++) {
                        //Input ID is aleays 0-indexed, continuous number starting 0
                        JsonObject hash;
                        hash["id"] = i;
                        std::stringstream locator;
                        locator << "hdmiin://localhost/deviceid/" << i;
                        hash["locator"] = locator.str();
                        hash["connected"] = device::HdmiInput::getInstance().isPortConnected(i) ? "true" : "false";
                        LOGWARN("HdmiInputService::getHDMIInputDevices id %d, locator=[%s], connected=[%s]", i, hash["locator"].String().c_str(), hash["connected"].String().c_str());
                        list.Add(hash);
                    }
                }
            }
            catch (const std::exception e)  {
                LOGWARN("HdmiInputService::getHDMIInputDevices Failed");
            }

            return list;
        }

        void HdmiInput::writeEDID(int deviceId, std::string message)
        {

        }

        std::string HdmiInput::readEDID()
        {
            return "HdmiInputEDIDStub";
        }

        /**
         * @brief This function is used to translate HDMI input hotplug to 
         * deviceChanged event.
         *
         * @param[in] input Number of input port integer.
         * @param[in] connection status of input port integer.
         */
        void HdmiInput::hdmiInputHotplug( int input , int connect)
        {
            LOGWARN("hdmiInputHotplug [%d, %d]", input, connect);

            JsonObject params;
            params["devices"] = getHDMIInputDevices();
            sendNotify(HDMIINPUT_EVENT_ON_DEVICES_CHANGED, params);
        }

        /**
         * @brief This function is used to translate HDMI input signal change to
         * signalChanged event.
         *
         * @param[in] port HDMI In port id.
         * @param[in] signalStatus signal status of HDMI In port.
         */
        void HdmiInput::hdmiInputSignalChange( int port , int signalStatus)
        {
            LOGWARN("hdmiInputSignalStatus [%d, %d]", port, signalStatus);

            JsonObject params;
            params["id"] = port;
            std::stringstream locator;
            locator << "hdmiin://localhost/deviceid/" << port;
            params["locator"] = locator.str();

	    switch (signalStatus) {
		    case dsHDMI_IN_SIGNAL_STATUS_NOSIGNAL:
			    params["signalStatus"] = "noSignal";
			    break;

	            case dsHDMI_IN_SIGNAL_STATUS_UNSTABLE:
			    params["signalStatus"] = "unstableSignal";
			    break;

                    case dsHDMI_IN_SIGNAL_STATUS_NOTSUPPORTED:
                            params["signalStatus"] = "notSupportedSignal";
                            break;

                    case dsHDMI_IN_SIGNAL_STATUS_STABLE:
                            params["signalStatus"] = "stableSignal";
                            break;

	            default:
                            params["signalStatus"] = "none";
                            break;
            }

            sendNotify(HDMIINPUT_EVENT_ON_SIGNAL_CHANGED, params);
        }

        /**
         * @brief This function is used to translate HDMI input status change to
         * inputStatusChanged event.
         *
         * @param[in] port HDMI In port id.
         * @param[bool] isPresented HDMI In presentation started/stopped.
         */
        void HdmiInput::hdmiInputStatusChange( int port , bool isPresented)
        {
            LOGWARN("hdmiInputStatus [%d, %d]", port, isPresented);

            JsonObject params;
            params["id"] = port;
            std::stringstream locator;
            locator << "hdmiin://localhost/deviceid/" << port;
            params["locator"] = locator.str();

            if(isPresented) {
                params["status"] = "started";
            }
	    else {
                params["status"] = "stopped";
            }

            sendNotify(HDMIINPUT_EVENT_ON_STATUS_CHANGED, params);
        }
	
        void HdmiInput::dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if(!HdmiInput::_instance)
                return;

            if (IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG == eventId)
            {
                IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                int hdmiin_hotplug_port = eventData->data.hdmi_in_connect.port;
                int hdmiin_hotplug_conn = eventData->data.hdmi_in_connect.isPortConnected;
                LOGWARN("Received IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG  event data:%d", hdmiin_hotplug_port);

                HdmiInput::_instance->hdmiInputHotplug(hdmiin_hotplug_port, hdmiin_hotplug_conn ? HDMI_HOT_PLUG_EVENT_CONNECTED : HDMI_HOT_PLUG_EVENT_DISCONNECTED);
            }
        }

        void HdmiInput::dsHdmiSignalStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if(!HdmiInput::_instance)
                return;

            if (IARM_BUS_DSMGR_EVENT_HDMI_IN_SIGNAL_STATUS == eventId)
            {
                IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                int hdmi_in_port = eventData->data.hdmi_in_sig_status.port;
                int hdmi_in_signal_status = eventData->data.hdmi_in_sig_status.status;
                LOGWARN("Received IARM_BUS_DSMGR_EVENT_HDMI_IN_SIGNAL_STATUS  event  port: %d, signal status: %d", hdmi_in_port,hdmi_in_signal_status);

                HdmiInput::_instance->hdmiInputSignalChange(hdmi_in_port, hdmi_in_signal_status);

            }
        }

        void HdmiInput::dsHdmiStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if(!HdmiInput::_instance)
                return;

            if (IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS == eventId)
            {
                IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                int hdmi_in_port = eventData->data.hdmi_in_status.port;
                bool hdmi_in_status = eventData->data.hdmi_in_status.isPresented;
                LOGWARN("Received IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS  event  port: %d, started: %d", hdmi_in_port,hdmi_in_status);

                HdmiInput::_instance->hdmiInputStatusChange(hdmi_in_port, hdmi_in_status);

            }
        }

    } // namespace Plugin
} // namespace WPEFramework
