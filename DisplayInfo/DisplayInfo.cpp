/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
 */

#include "DisplayInfo.h"
#include "host.hpp"
#include "exception.hpp"
#include "videoOutputPort.hpp"
#include "videoOutputPortType.hpp"
#include "videoOutputPortConfig.hpp"
#include "videoResolution.hpp"
#include "audioOutputPort.hpp"
#include "audioOutputPortType.hpp"
#include "audioOutputPortConfig.hpp"
#include "manager.hpp"
#include "utils.h"

#if defined(USE_IARM)
#include "libIBus.h"
#include "libIBusDaemon.h"
#include "dsMgr.h"
#endif

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(DisplayInfo, 1, 0);

    static Core::ProxyPoolType<Web::Response> responseFactory(4);
    static Core::ProxyPoolType<Web::JSONBodyType<JsonData::DisplayInfo::DisplayinfoData>> jsonResponseFactory(4);
    DisplayInfo* DisplayInfo::_instance = nullptr;

    /* virtual */ const string DisplayInfo::Initialize(PluginHost::IShell* service)
    {
        ASSERT(service != nullptr);
        ASSERT(_connectionProperties == nullptr);

        string message;
        Config config;

        config.FromString(service->ConfigLine());
        _skipURL = static_cast<uint8_t>(service->WebPrefix().length());

        _connectionProperties = service->Root<Exchange::IConnectionProperties>(_connectionId, 2000, _T("DisplayInfoImplementation"));
        if (_connectionProperties != nullptr) {

            _graphicsProperties = _connectionProperties->QueryInterface<Exchange::IGraphicsProperties>();
            if (_graphicsProperties == nullptr) {

                _connectionProperties->Release();
                _connectionProperties = nullptr;
            } else {
                _notification.Initialize(_connectionProperties);
            }
        }

        if (_connectionProperties == nullptr) {
            message = _T("DisplayInfo could not be instantiated.");
        }

        try
        {
#if defined(USE_IARM)
            Utils::IARM::init();
            IARM_Result_t res;
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_RES_PRECHANGE,ResolutionPreChange) );
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE, ResolutionPostChange) );
#endif
            //TODO: this is probably per process so we either need to be running in our own process or be carefull no other plugin is calling it
            device::Manager::Initialize();
            LOGINFO("device::Manager::Initialize success");
        }
        catch(...)
        {
            LOGINFO("device::Manager::Initialize failed");
        }

        return message;
    }

    /* virtual */ void DisplayInfo::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_connectionProperties != nullptr);

        _notification.Deinitialize();

        ASSERT(_graphicsProperties != nullptr);
        if (_graphicsProperties != nullptr) {
            _graphicsProperties->Release();
            _graphicsProperties = nullptr;
        }

        ASSERT(_connectionProperties != nullptr);
        if (_connectionProperties != nullptr) {
            _connectionProperties->Release();
            _connectionProperties = nullptr;
        }

        _connectionId = 0;

        try
        {
            //TODO(MROLLINS) this is probably per process so we either need to be running in our own process or be carefull no other plugin is calling it
            device::Manager::DeInitialize();
            LOGINFO("device::Manager::DeInitialize success");
        }
        catch(...)
        {
            LOGINFO("device::Manager::DeInitialize failed");
        }
    }

    /* virtual */ string DisplayInfo::Information() const
    {
        // No additional info to report.
        return (string());
    }

    /* virtual */ void DisplayInfo::Inbound(Web::Request& /* request */)
    {
    }

    /* virtual */ Core::ProxyType<Web::Response> DisplayInfo::Process(const Web::Request& request)
    {
        ASSERT(_skipURL <= request.Path.length());

        Core::ProxyType<Web::Response> result(PluginHost::IFactories::Instance().Response());

        // By default, we assume everything works..
        result->ErrorCode = Web::STATUS_OK;
        result->Message = "OK";

        // <GET> - currently, only the GET command is supported, returning system info
        if (request.Verb == Web::Request::HTTP_GET) {

            Core::ProxyType<Web::JSONBodyType<JsonData::DisplayInfo::DisplayinfoData>> response(jsonResponseFactory.Element());

            Core::TextSegmentIterator index(Core::TextFragment(request.Path, _skipURL, static_cast<uint32_t>(request.Path.length()) - _skipURL), false, '/');

            // Always skip the first one, it is an empty part because we start with a '/' if there are more parameters.
            index.Next();

            Info(*response);
            result->ContentType = Web::MIMETypes::MIME_JSON;
            result->Body(Core::proxy_cast<Web::IBody>(response));
        } else {
            result->ErrorCode = Web::STATUS_BAD_REQUEST;
            result->Message = _T("Unsupported request for the [DisplayInfo] service.");
        }

        return result;
    }

/*begin methods*/
    void DisplayInfo::Info(JsonData::DisplayInfo::DisplayinfoData& displayInfo) const
    {
        displayInfo.Totalgpuram = _graphicsProperties->TotalGpuRam();
        displayInfo.Freegpuram = _graphicsProperties->FreeGpuRam();

        displayInfo.Audiopassthrough = _connectionProperties->IsAudioPassthrough();
        displayInfo.Connected = _connectionProperties->Connected();
        displayInfo.Width = _connectionProperties->Width();
        displayInfo.Height = _connectionProperties->Height();
        displayInfo.Hdcpmajor = _connectionProperties->HDCPMajor();
        displayInfo.Hdcpminor = _connectionProperties->HDCPMinor();
        displayInfo.Hdrtype = static_cast<JsonData::DisplayInfo::DisplayinfoData::HdrtypeType>(_connectionProperties->Type());
    }

    void setResponseArray(JsonObject& response, const char* key, const std::vector<string>& items)
    {
        JsonArray arr;
        for(auto& i : items) arr.Add(JsonValue(i));

        response[key] = arr;

        string json;
        response.ToString(json);
    }

    uint32_t DisplayInfo::getCurrentResolution(const JsonObject& parameters, JsonObject& response)
    {   //sample servicemanager response:{"success":true,"resolution":"720p"}
        LOGINFOMETHOD();
        string videoDisplay = parameters.HasLabel("videoDisplay") ? parameters["videoDisplay"].String() : "HDMI0";
        bool success = true;
        try
        {
            device::VideoOutputPort &vPort = device::Host::getInstance().getVideoOutputPort(videoDisplay);
            response["resolution"] = vPort.getResolution().getName();
        }
        catch(const device::Exception& err)
        {
            LOG_DEVICE_EXCEPTION1(videoDisplay);
            success = false;
        }
        returnResponse(success);
    }

    uint32_t DisplayInfo::getConnectedVideoDisplays(const JsonObject& parameters, JsonObject& response)
    {   //sample servicemanager response: {"connectedVideoDisplays":["HDMI0"],"success":true}
        //this                          : {"connectedVideoDisplays":["HDMI0"]}
        LOGINFOMETHOD();

        std::vector<string> connectedVideoDisplays;
        getConnectedVideoDisplaysHelper(connectedVideoDisplays);
        setResponseArray(response, "connectedVideoDisplays", connectedVideoDisplays);
        returnResponse(true);
    }

    uint32_t DisplayInfo::getTVHDRCapabilities (const JsonObject& parameters, JsonObject& response)
    {   //sample servicemanager response:
        LOGINFOMETHOD();
        bool success = true;
        int capabilities = dsHDRSTANDARD_NONE;
        try
        {
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort("HDMI0");
            if (vPort.isDisplayConnected()) {
                vPort.getTVHDRCapabilities(&capabilities);
                response["capabilities"] = capabilities;
            }
            else {
                LOGERR("getTVHDRCapabilities failure: HDMI0 not connected!\n");
                success = false;
            }
        }
        catch(const device::Exception& err)
        {
            LOG_DEVICE_EXCEPTION1(string("HDMI0"));
            success = false;
        }
        returnResponse(success);
    }

    uint32_t DisplayInfo::getSettopHDRSupport(const JsonObject& parameters, JsonObject& response)
    {   //sample servicemanager response:{"standards":["HDR10"],"supportsHDR":true}
        LOGINFOMETHOD();

        JsonArray hdrCapabilities;
        int capabilities = dsHDRSTANDARD_NONE;

        try
        {
            device::VideoDevice &device = device::Host::getInstance().getVideoDevices().at(0);
            device.getHDRCapabilities(&capabilities);
        }
        catch(const device::Exception& err)
        {
            LOG_DEVICE_EXCEPTION0();
        }

        if(!capabilities)hdrCapabilities.Add("none");
        if(capabilities & dsHDRSTANDARD_HDR10)hdrCapabilities.Add("HDR10");
        if(capabilities & dsHDRSTANDARD_DolbyVision)hdrCapabilities.Add("Dolby Vision");
        if(capabilities & dsHDRSTANDARD_TechnicolorPrime)hdrCapabilities.Add("Technicolor Prime");

        if(capabilities)
        {
            response["supportsHDR"] = true;
        }
        else
        {
            response["supportsHDR"] = false;
        }
        response["standards"] = hdrCapabilities;
        for (uint32_t i = 0; i < hdrCapabilities.Length(); i++)
        {
            LOGINFO("capabilities: %s", hdrCapabilities[i].String().c_str());
        }
        returnResponse(true);
    }

    uint32_t DisplayInfo::IsOutputHDR(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFOMETHOD();
        bool success = true;
        try
        {
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort("HDMI0");
            if (vPort.isDisplayConnected()) {
                response["IsOutputHDR"] = vPort.IsOutputHDR();
            }
            else
            {
                LOGERR("IsOutputHDR failure: HDMI0 not connected!\n");
                success = false;
            }
        }
        catch(const device::Exception& err)
        {
            LOG_DEVICE_EXCEPTION1(string("HDMI0"));
            success = false;
        }
        returnResponse(success);
    }

    uint32_t DisplayInfo::setHdmiPreferences(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFOMETHOD();
        returnIfParamNotFound(parameters, "hdcpProtocol");
        dsHdcpProtocolVersion_t hdcpCurrentProtocol = static_cast<dsHdcpProtocolVersion_t>(parameters["hdcpProtocol"].Number());
        bool success = true;
        try
        {
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort("HDMI0");
            if (vPort.isDisplayConnected())
            {
                success = vPort.SetHdmiPreference(hdcpCurrentProtocol);
            }
            else
            {
                LOGERR("setHdmiPreferences failure: HDMI0 not connected!\n");
                success = false;
            }
        }
        catch(const device::Exception& err)
        {
            LOG_DEVICE_EXCEPTION1(string("HDMI0"));
            success = false;
        }
        returnResponse(success);
    }

    uint32_t DisplayInfo::getHdmiPreferences(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFOMETHOD();
        bool success = true;
        try
        {
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort("HDMI0");
            if (vPort.isDisplayConnected()) {
                response["hdcpProtocol"] = vPort.GetHdmiPreference();
            }
            else
            {
                LOGERR("getHdmiPreferences failure: HDMI0 not connected!\n");
                success = false;
            }
        }
        catch(const device::Exception& err)
        {
            LOG_DEVICE_EXCEPTION1(string("HDMI0"));
            success = false;
        }
        returnResponse(success);
    }

    uint32_t DisplayInfo::isAudioEquivalenceEnabled(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFOMETHOD();
        bool success = true;
        try
        {
            device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI0");
            if (aPort.isConnected()) {
                response["isAudioEquivalenceEnabled"] = aPort.GetLEConfig();
            }
            else
            {
                LOGERR("isAudioEquivalenceEnabled failure: HDMI0 not connected!\n");
                success = false;
            }
        }
        catch(const device::Exception& err)
        {
            LOG_DEVICE_EXCEPTION1(string("HDMI0"));
            success = false;
        }
        returnResponse(success);
    }

    uint32_t DisplayInfo::readEDID(const JsonObject& parameters, JsonObject& response)
    {   //sample servicemanager response: {"EDID":"AP///////wBSYgYCAQEBAQEXAQOAoFp4CvCdo1VJmyYPR0ovzgCBgIvAAQEBAQEBAQEBAQEBAjqAGHE4LUBYLEUAQIRjAAAeZiFQsFEAGzBAcDYAQIRjAAAeAAAA/ABUT1NISUJBLVRWCiAgAAAA/QAXSw9EDwAKICAgICAgAbECAytxSpABAgMEBQYHICImCQcHEQcYgwEAAGwDDAAQADgtwBUVHx/jBQMBAR2AGHEcFiBYLCUAQIRjAACeAR0AclHQHiBuKFUAQIRjAAAejArQiiDgLRAQPpYAsIRDAAAYjAqgFFHwFgAmfEMAsIRDAACYAAAAAAAAAAAAAAAA9w=="
        //sample this thunder plugin    : {"EDID":"AP///////wBSYgYCAQEBAQEXAQOAoFp4CvCdo1VJmyYPR0ovzgCBgIvAAQEBAQEBAQEBAQEBAjqAGHE4LUBYLEUAQIRjAAAeZiFQsFEAGzBAcDYAQIRjAAAeAAAA/ABUT1NISUJBLVRWCiAgAAAA/QAXSw9EDwAKICAgICAgAbECAytxSpABAgMEBQYHICImCQcHEQcYgwEAAGwDDAAQADgtwBUVHx/jBQMBAR2AGHEcFiBYLCUAQIRjAACeAR0AclHQHiBuKFUAQIRjAAAejArQiiDgLRAQPpYAsIRDAAAYjAqgFFHwFgAmfEMAsIRDAACYAAAAAAAAAAAAAAAA9w"}
        LOGINFOMETHOD();

        vector<uint8_t> edidVec({'u','n','k','n','o','w','n' });
        try
        {
            vector<uint8_t> edidVec2;
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort("HDMI0");
            if (vPort.isDisplayConnected())
            {
                vPort.getDisplay().getEDIDBytes(edidVec2);
                edidVec = edidVec2;//edidVec must be "unknown" unless we successfully get to this line
            }
            else
            {
                LOGWARN("failure: HDMI0 not connected!");
            }
        }
        catch (const device::Exception& err)
        {
            LOG_DEVICE_EXCEPTION0();
        }
        //convert to base64
        uint16_t size = min(edidVec.size(), (size_t)numeric_limits<uint16_t>::max());
        if(edidVec.size() > (size_t)numeric_limits<uint16_t>::max())
            LOGERR("Size too large to use ToString base64 wpe api");
        string edidbase64;
        Core::ToString((uint8_t*)&edidVec[0], size, false, edidbase64);
        response["EDID"] = edidbase64;
        returnResponse(true);
    }

/* Begin Helper methods */

    void DisplayInfo::getConnectedVideoDisplaysHelper(vector<string>& connectedDisplays)
    {
        LOGINFO();
        try
        {
            device::List<device::VideoOutputPort> vPorts = device::Host::getInstance().getVideoOutputPorts();
            for (size_t i = 0; i < vPorts.size(); i++)
            {
                device::VideoOutputPort &vPort = vPorts.at(i);
                if (vPort.isDisplayConnected())
                {
                    string displayName = vPort.getName();
                    if (strncasecmp(displayName.c_str(), "hdmi", 4)==0)
                    {
                        connectedDisplays.clear();
                        connectedDisplays.emplace_back(displayName);
                        break;
                    }
                    else
                    {
                        vectorSet(connectedDisplays, displayName);
                    }
                }
            }
        }
        catch(const device::Exception& err)
        {
            LOG_DEVICE_EXCEPTION0();
        }
    }

/* Begin Events */
#if defined(USE_IARM)
    void DisplayInfo::ResolutionPreChange(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        LOGINFO();
        if(DisplayInfo::_instance)
        {
            DisplayInfo::_instance->resolutionPreChange();
        }
    }

    void DisplayInfo::ResolutionPostChange(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        LOGINFO();

        int dw = 1280;
        int dh = 720;

        if (strcmp(owner, IARM_BUS_DSMGR_NAME) == 0)
        {
            switch (eventId) {
                case IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE:
                    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                    dw = eventData->data.resn.width;
                    dh = eventData->data.resn.height;
                    LOGINFO("width: %d, height: %d", dw, dh);
                    break;
            }
        }

        if(DisplayInfo::_instance)
        {
            DisplayInfo::_instance->resolutionChanged(dw, dh);
        }
    }
#endif

    void DisplayInfo::resolutionPreChange()
    {
        LOGINFO();
        sendNotify("resolutionPreChange", JsonObject());
    }

    void DisplayInfo::resolutionChanged(int width, int height)
    {
        LOGINFO();
        vector<string> connectedDisplays;
        getConnectedVideoDisplaysHelper(connectedDisplays);

        string firstDisplay = "";
        string firstResolution = "";
        bool firstResolutionSet = false;
        for (int i = 0; i < (int)connectedDisplays.size(); i++)
        {
            string resolution;
            string display = connectedDisplays.at(i);
            try
            {
                resolution = device::Host::getInstance().getVideoOutputPort(display).getResolution().getName();
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(display);
            }
            if (!resolution.empty())
            {
                if (Utils::String::stringContains(display,"HDMI"))
                {
                    // only report first HDMI connected device is HDMI is connected
                    JsonObject params;
                    params["width"] = width;
                    params["height"] = height;
                    params["videoDisplayType"] = display;
                    params["resolution"] = resolution;
                    sendNotify("resolutionChanged", params);
                    return;
                }
                else if (!firstResolutionSet)
                {
                    firstDisplay = display;
                    firstResolution = resolution;
                    firstResolutionSet = true;
                }
            }
        }
        if (firstResolutionSet)
        {
            //if HDMI is not connected then notify the server of first connected device
            JsonObject params;
            params["width"] = width;
            params["height"] = height;
            params["videoDisplayType"] = firstDisplay;
            params["resolution"] = firstResolution;
            sendNotify("resolutionChanged", params);
        }
    }

} // namespace Plugin
} // namespace WPEFramework
