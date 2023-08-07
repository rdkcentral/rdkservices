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
#include "dsMgr.h"
#include "hdmiIn.hpp"
#include "compositeIn.hpp"

#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"

#include "exception.hpp"
#include <vector>
#include <algorithm>

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 2
#define API_VERSION_NUMBER_PATCH 1

#define HDMI 0
#define COMPOSITE 1
#define AV_HOT_PLUG_EVENT_CONNECTED 0
#define AV_HOT_PLUG_EVENT_DISCONNECTED 1
#define AVINPUT_METHOD_NUMBER_OF_INPUTS "numberOfInputs"
#define AVINPUT_METHOD_GET_INPUT_DEVICES "getInputDevices"
#define AVINPUT_METHOD_WRITE_EDID "writeEDID"
#define AVINPUT_METHOD_READ_EDID "readEDID"
#define AVINPUT_METHOD_READ_RAWSPD "getRawSPD"
#define AVINPUT_METHOD_READ_SPD "getSPD"
#define AVINPUT_METHOD_SET_EDID_VERSION "setEdidVersion"
#define AVINPUT_METHOD_GET_EDID_VERSION "getEdidVersion"
#define AVINPUT_METHOD_START_INPUT "startInput"
#define AVINPUT_METHOD_STOP_INPUT "stopInput"
#define AVINPUT_METHOD_SCALE_INPUT "setVideoRectangle"
#define AVINPUT_METHOD_CURRENT_VIDEO_MODE "currentVideoMode"
#define AVINPUT_METHOD_CONTENT_PROTECTED "contentProtected"
#define AVINPUT_METHOD_SUPPORTED_GAME_FEATURES "getSupportedGameFeatures"
#define AVINPUT_METHOD_GAME_FEATURE_STATUS "getGameFeatureStatus"

#define AVINPUT_EVENT_ON_DEVICES_CHANGED "onDevicesChanged"
#define AVINPUT_EVENT_ON_SIGNAL_CHANGED "onSignalChanged"
#define AVINPUT_EVENT_ON_STATUS_CHANGED "onInputStatusChanged"
#define AVINPUT_EVENT_ON_VIDEO_MODE_UPDATED "videoStreamInfoUpdate"
#define AVINPUT_EVENT_ON_GAME_FEATURE_STATUS_CHANGED "gameFeatureStatusUpdate"
#define AVINPUT_EVENT_ON_AVI_CONTENT_TYPE_CHANGED "aviContentTypeUpdate"

using namespace std;
int getTypeOfInput(string sType)
{
    int iType = -1;
    if (strcmp (sType.c_str(), "HDMI") == 0)
        iType = HDMI;
    else if (strcmp (sType.c_str(), "COMPOSITE") ==0)
        iType = COMPOSITE;
    else
        throw "Invalide type of INPUT, please specify HDMI/COMPOSITE";
    return iType;
}

namespace WPEFramework {
namespace {

    static Plugin::Metadata<Plugin::AVInput> metadata(
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

SERVICE_REGISTRATION(AVInput, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

AVInput* AVInput::_instance = nullptr;

AVInput::AVInput()
    : PluginHost::JSONRPC()
{
    RegisterAll();
}

AVInput::~AVInput()
{
    UnregisterAll();
}

const string AVInput::Initialize(PluginHost::IShell * /* service */)
{
    AVInput::_instance = this;
    InitializeIARM();

    return (string());
}

void AVInput::Deinitialize(PluginHost::IShell * /* service */)
{
    DeinitializeIARM();
    AVInput::_instance = nullptr;
}

string AVInput::Information() const
{
    return (string());
}

void AVInput::InitializeIARM()
{
    if (Utils::IARM::init()) {
        IARM_Result_t res;
        IARM_CHECK(IARM_Bus_RegisterEventHandler(
            IARM_BUS_DSMGR_NAME,
            IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG,
            dsAVEventHandler));
        IARM_CHECK(IARM_Bus_RegisterEventHandler(
            IARM_BUS_DSMGR_NAME,
            IARM_BUS_DSMGR_EVENT_HDMI_IN_SIGNAL_STATUS,
            dsAVSignalStatusEventHandler));
        IARM_CHECK(IARM_Bus_RegisterEventHandler(
            IARM_BUS_DSMGR_NAME,
            IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS,
            dsAVStatusEventHandler));
        IARM_CHECK(IARM_Bus_RegisterEventHandler(
            IARM_BUS_DSMGR_NAME,
            IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE,
            dsAVVideoModeEventHandler));
        IARM_CHECK(IARM_Bus_RegisterEventHandler(
            IARM_BUS_DSMGR_NAME,
            IARM_BUS_DSMGR_EVENT_HDMI_IN_ALLM_STATUS,
            dsAVGameFeatureStatusEventHandler));
        IARM_CHECK(IARM_Bus_RegisterEventHandler(
            IARM_BUS_DSMGR_NAME,
            IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_HOTPLUG,
            dsAVEventHandler));
        IARM_CHECK(IARM_Bus_RegisterEventHandler(
            IARM_BUS_DSMGR_NAME,
            IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_SIGNAL_STATUS,
            dsAVSignalStatusEventHandler));
        IARM_CHECK(IARM_Bus_RegisterEventHandler(
            IARM_BUS_DSMGR_NAME,
            IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_STATUS,
            dsAVStatusEventHandler));
	IARM_CHECK(IARM_Bus_RegisterEventHandler(
            IARM_BUS_DSMGR_NAME,
            IARM_BUS_DSMGR_EVENT_HDMI_IN_AVI_CONTENT_TYPE,
            dsAviContentTypeEventHandler));
    }
}

void AVInput::DeinitializeIARM()
{
    if (Utils::IARM::isConnected()) {
        IARM_Result_t res;
        IARM_CHECK(IARM_Bus_RemoveEventHandler(
            IARM_BUS_DSMGR_NAME,
            IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG, dsAVEventHandler));
        IARM_CHECK(IARM_Bus_RemoveEventHandler(
            IARM_BUS_DSMGR_NAME,
            IARM_BUS_DSMGR_EVENT_HDMI_IN_SIGNAL_STATUS, dsAVSignalStatusEventHandler));
        IARM_CHECK(IARM_Bus_RemoveEventHandler(
            IARM_BUS_DSMGR_NAME,
            IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS, dsAVStatusEventHandler));
        IARM_CHECK(IARM_Bus_RemoveEventHandler(
            IARM_BUS_DSMGR_NAME,
            IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE, dsAVVideoModeEventHandler));
        IARM_CHECK(IARM_Bus_RemoveEventHandler(
            IARM_BUS_DSMGR_NAME,
            IARM_BUS_DSMGR_EVENT_HDMI_IN_ALLM_STATUS, dsAVGameFeatureStatusEventHandler));
        IARM_CHECK(IARM_Bus_RemoveEventHandler(
            IARM_BUS_DSMGR_NAME,
            IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_HOTPLUG, dsAVEventHandler));
        IARM_CHECK(IARM_Bus_RemoveEventHandler(
            IARM_BUS_DSMGR_NAME,
            IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_SIGNAL_STATUS, dsAVSignalStatusEventHandler));
        IARM_CHECK(IARM_Bus_RemoveEventHandler(
            IARM_BUS_DSMGR_NAME,
            IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_STATUS, dsAVStatusEventHandler));
	IARM_CHECK(IARM_Bus_RemoveEventHandler(
            IARM_BUS_DSMGR_NAME,
            IARM_BUS_DSMGR_EVENT_HDMI_IN_AVI_CONTENT_TYPE, dsAviContentTypeEventHandler));
    }
}

void AVInput::RegisterAll()
{
    Register<JsonObject, JsonObject>(_T(AVINPUT_METHOD_NUMBER_OF_INPUTS), &AVInput::endpoint_numberOfInputs, this);
    Register<JsonObject, JsonObject>(_T(AVINPUT_METHOD_CURRENT_VIDEO_MODE), &AVInput::endpoint_currentVideoMode, this);
    Register<JsonObject, JsonObject>(_T(AVINPUT_METHOD_CONTENT_PROTECTED), &AVInput::endpoint_contentProtected, this);
    Register<JsonObject, JsonObject>(_T(AVINPUT_METHOD_GET_INPUT_DEVICES), &AVInput::getInputDevicesWrapper, this);
    Register<JsonObject, JsonObject>(_T(AVINPUT_METHOD_WRITE_EDID), &AVInput::writeEDIDWrapper, this);
    Register<JsonObject, JsonObject>(_T(AVINPUT_METHOD_READ_EDID), &AVInput::readEDIDWrapper, this);
    Register<JsonObject, JsonObject>(_T(AVINPUT_METHOD_READ_RAWSPD), &AVInput::getRawSPDWrapper, this);
    Register<JsonObject, JsonObject>(_T(AVINPUT_METHOD_READ_SPD), &AVInput::getSPDWrapper, this);
    Register<JsonObject, JsonObject>(_T(AVINPUT_METHOD_SET_EDID_VERSION), &AVInput::setEdidVersionWrapper, this);
    Register<JsonObject, JsonObject>(_T(AVINPUT_METHOD_GET_EDID_VERSION), &AVInput::getEdidVersionWrapper, this);
    Register<JsonObject, JsonObject>(_T(AVINPUT_METHOD_START_INPUT), &AVInput::startInput, this);
    Register<JsonObject, JsonObject>(_T(AVINPUT_METHOD_STOP_INPUT), &AVInput::stopInput, this);
    Register<JsonObject, JsonObject>(_T(AVINPUT_METHOD_SCALE_INPUT), &AVInput::setVideoRectangleWrapper, this);
    Register<JsonObject, JsonObject>(_T(AVINPUT_METHOD_SUPPORTED_GAME_FEATURES), &AVInput::getSupportedGameFeatures, this);
    Register<JsonObject, JsonObject>(_T(AVINPUT_METHOD_GAME_FEATURE_STATUS), &AVInput::getGameFeatureStatusWrapper, this);
}

void AVInput::UnregisterAll()
{
    Unregister(_T(AVINPUT_METHOD_NUMBER_OF_INPUTS));
    Unregister(_T(AVINPUT_METHOD_CURRENT_VIDEO_MODE));
    Unregister(_T(AVINPUT_METHOD_CONTENT_PROTECTED));
    Unregister(_T(AVINPUT_METHOD_GET_INPUT_DEVICES));
    Unregister(_T(AVINPUT_METHOD_WRITE_EDID));
    Unregister(_T(AVINPUT_METHOD_READ_EDID));
    Unregister(_T(AVINPUT_METHOD_READ_RAWSPD));
    Unregister(_T(AVINPUT_METHOD_READ_SPD));
    Unregister(_T(AVINPUT_METHOD_SET_EDID_VERSION));
    Unregister(_T(AVINPUT_METHOD_GET_EDID_VERSION));
    Unregister(_T(AVINPUT_METHOD_START_INPUT));
    Unregister(_T(AVINPUT_METHOD_STOP_INPUT));
    Unregister(_T(AVINPUT_METHOD_SCALE_INPUT));
    Unregister(_T(AVINPUT_METHOD_SUPPORTED_GAME_FEATURES));
    Unregister(_T(AVINPUT_METHOD_GAME_FEATURE_STATUS));
}

void setResponseArray(JsonObject& response, const char* key, const vector<string>& items)
{
    JsonArray arr;
    for(auto& i : items) arr.Add(JsonValue(i));

    response[key] = arr;

    string json;
    response.ToString(json);
    LOGINFO("%s: result json %s\n", __FUNCTION__, json.c_str());
}

uint32_t AVInput::endpoint_numberOfInputs(const JsonObject &parameters, JsonObject &response)
{
    LOGINFOMETHOD();

    bool success = false;

    auto result = numberOfInputs(success);
    if (success) {
        response[_T("numberOfInputs")] = result;
    }

    returnResponse(success);
}

uint32_t AVInput::endpoint_currentVideoMode(const JsonObject &parameters, JsonObject &response)
{
    LOGINFOMETHOD();

    bool success = false;

    auto result = currentVideoMode(success);
    if (success) {
        response[_T("currentVideoMode")] = result;
    }

    returnResponse(success);
}

uint32_t AVInput::endpoint_contentProtected(const JsonObject &parameters, JsonObject &response)
{
    LOGINFOMETHOD();

    // "Ths is the way it's done in Service Manager"
    response[_T("isContentProtected")] = true;

    returnResponse(true);
}

int AVInput::numberOfInputs(bool &success)
{
    int result = 0;

    try {
        result = device::HdmiInput::getInstance().getNumberOfInputs();
        success = true;
    }
    catch (...) {
        LOGERR("Exception caught");
        success = false;
    }

    return result;
}

string AVInput::currentVideoMode(bool &success)
{
    string result;

    try {
        result = device::HdmiInput::getInstance().getCurrentVideoMode();
        success = true;
    }
    catch (...) {
        LOGERR("Exception caught");
        success = false;
    }

    return result;
}


uint32_t AVInput::startInput(const JsonObject& parameters, JsonObject& response)
{
    LOGINFOMETHOD();

    string sType = parameters["typeOfInput"].String();
    int portId = 0;
    int iType = 0;

    if (parameters.HasLabel("portId") && parameters.HasLabel("typeOfInput"))
    {
        try {
            portId = parameters["portId"].Number();
            iType = getTypeOfInput (sType);
        }catch (...) {
            LOGWARN("Invalid Arguments");
            returnResponse(false);
        }
    }
    else {
        LOGWARN("Required parameters are not passed");
        returnResponse(false);
    }

    try
    {
        if (iType == HDMI) {
            device::HdmiInput::getInstance().selectPort(portId);
    }
    else if(iType == COMPOSITE) {
            device::CompositeInput::getInstance().selectPort(portId);
        }
    }
    catch (const device::Exception& err) {
        LOG_DEVICE_EXCEPTION1(std::to_string(portId));
        returnResponse(false);
    }
    returnResponse(true);
}

uint32_t AVInput::stopInput(const JsonObject& parameters, JsonObject& response)
{
    LOGINFOMETHOD();

    string sType = parameters["typeOfInput"].String();
    int iType = 0;

    if (parameters.HasLabel("typeOfInput"))
        try {
            iType = getTypeOfInput (sType);
        }catch (...) {
            LOGWARN("Invalid Arguments");
            returnResponse(false);
        }
    else {
        LOGWARN("Required parameters are not passed");
        returnResponse(false);
    }

    try
    {
        if (iType == HDMI) {
            device::HdmiInput::getInstance().selectPort(-1);
        }
        else if (iType == COMPOSITE) {
            device::CompositeInput::getInstance().selectPort(-1);
        }
    }
    catch (const device::Exception& err) {
        LOGWARN("AVInputService::stopInput Failed");
        returnResponse(false);
    }
    returnResponse(true);
}

uint32_t AVInput::setVideoRectangleWrapper(const JsonObject& parameters, JsonObject& response)
{
    LOGINFOMETHOD();

    bool result = true;
    if (!parameters.HasLabel("x") && !parameters.HasLabel("y")) {
        result = false;
        LOGWARN("please specif coordinates (x,y)");
    }

    if (!parameters.HasLabel("w") && !parameters.HasLabel("h")) {
        result = false;
        LOGWARN("please specify window width and height (w,h)");
    }

    if (!parameters.HasLabel("typeOfInput")) {
        result = false;
        LOGWARN("please specify type of input HDMI/COMPOSITE");
    }

    if (result) {
        int x = 0;
        int y = 0;
        int w = 0;
        int h = 0;
        int t = 0;
        string sType;

        try {
            if (parameters.HasLabel("x")) {
                x = parameters["x"].Number();
            }
            if (parameters.HasLabel("y")) {
                y = parameters["y"].Number();
            }
            if (parameters.HasLabel("w")) {
                w = parameters["w"].Number();
            }
            if (parameters.HasLabel("h")) {
                h = parameters["h"].Number();
            }
            if (parameters.HasLabel("typeOfInput")) {
                sType = parameters["typeOfInput"].String();
                t = getTypeOfInput (sType);
            }
        }
        catch (...) {
            LOGWARN("Invalid Arguments");
            returnResponse(false);
        }

        result = setVideoRectangle(x, y, w, h, t);
        if (false == result) {
            LOGWARN("AVInputService::setVideoRectangle Failed");
            returnResponse(false);
        }
        returnResponse(true);
    }
    returnResponse(false);
}

bool AVInput::setVideoRectangle(int x, int y, int width, int height, int type)
{
    bool ret = true;

    try
    {
        if (type == HDMI) {
            device::HdmiInput::getInstance().scaleVideo(x, y, width, height);
        }
        else {
            device::CompositeInput::getInstance().scaleVideo(x, y, width, height);
        }
    }
    catch (const device::Exception& err) {
        ret = false;
    }

    return ret;
}

uint32_t AVInput::getInputDevicesWrapper(const JsonObject& parameters, JsonObject& response)
{
    LOGINFOMETHOD();

    if (parameters.HasLabel("typeOfInput")) {
        string sType = parameters["typeOfInput"].String();
        int iType = 0;
        try {
            iType = getTypeOfInput (sType);
        }catch (...) {
            LOGWARN("Invalid Arguments");
            returnResponse(false);
        }
        response["devices"] = getInputDevices(iType);
    }
    else {
        JsonArray listHdmi = getInputDevices(HDMI);
        JsonArray listComposite = getInputDevices(COMPOSITE);
        for (int i = 0; i < listComposite.Length(); i++) {
            listHdmi.Add(listComposite.Get(i));
        }
        response["devices"] = listHdmi;
    }
    returnResponse(true);
}

uint32_t AVInput::writeEDIDWrapper(const JsonObject& parameters, JsonObject& response)
{
    LOGINFOMETHOD();

    int portId;
    std::string message;

    if (parameters.HasLabel("portId") && parameters.HasLabel("message")) {
        portId = parameters["portId"].Number();
        message = parameters["message"].String();
    }
    else {
        LOGWARN("Required parameters are not passed");
        returnResponse(false);
    }

    writeEDID(portId, message);
    returnResponse(true);
}

uint32_t AVInput::readEDIDWrapper(const JsonObject& parameters, JsonObject& response)
{
    LOGINFOMETHOD();

    int portId = 0;
    try {
        portId = parameters["portId"].Number();
    }catch (...) {
            LOGWARN("Invalid Arguments");
            returnResponse(false);
    }

    string edid = readEDID (portId);
    if (edid.empty()) {
        returnResponse(false);
    }
    else {
        response["EDID"] = edid;
        returnResponse(true);
    }
}

JsonArray AVInput::getInputDevices(int iType)
{
    JsonArray list;
    try
    {
        int num = 0;
        if (iType == HDMI) {
            num = device::HdmiInput::getInstance().getNumberOfInputs();
        }
        else if (iType == COMPOSITE) {
            num = device::CompositeInput::getInstance().getNumberOfInputs();
        }
        if (num > 0) {
            int i = 0;
            for (i = 0; i < num; i++) {
                //Input ID is aleays 0-indexed, continuous number starting 0
                JsonObject hash;
                hash["id"] = i;
                std::stringstream locator;
                if (iType == HDMI) {
                    locator << "hdmiin://localhost/deviceid/" << i;
                    hash["connected"] = device::HdmiInput::getInstance().isPortConnected(i);
                }
                else if (iType == COMPOSITE) {
                    locator << "cvbsin://localhost/deviceid/" << i;
                    hash["connected"] = device::CompositeInput::getInstance().isPortConnected(i);
                }
                hash["locator"] = locator.str();
                LOGWARN("AVInputService::getInputDevices id %d, locator=[%s], connected=[%d]", i, hash["locator"].String().c_str(), hash["connected"].Boolean());
                list.Add(hash);
            }
        }
    }
    catch (const std::exception &e) {
        LOGWARN("AVInputService::getInputDevices Failed");
    }
    return list;
}

void AVInput::writeEDID(int portId, std::string message)
{
}

std::string AVInput::readEDID(int iPort)
{
    vector<uint8_t> edidVec({'u','n','k','n','o','w','n' });
    string edidbase64 = "";
    try {
        vector<uint8_t> edidVec2;
        device::HdmiInput::getInstance().getEDIDBytesInfo (iPort, edidVec2);
        edidVec = edidVec2;//edidVec must be "unknown" unless we successfully get to this line

        //convert to base64
        uint16_t size = min(edidVec.size(), (size_t)numeric_limits<uint16_t>::max());

        LOGWARN("AVInput::readEDID size:%d edidVec.size:%zu", size, edidVec.size());
        if(edidVec.size() > (size_t)numeric_limits<uint16_t>::max()) {
            LOGERR("Size too large to use ToString base64 wpe api");
            return edidbase64;
        }
        Core::ToString((uint8_t*)&edidVec[0], size, true, edidbase64);
    }
    catch (const device::Exception& err) {
        LOG_DEVICE_EXCEPTION1(std::to_string(iPort));
    }
    return edidbase64;
}

/**
 * @brief This function is used to translate HDMI/COMPOSITE input hotplug to
 * deviceChanged event.
 *
 * @param[in] input Number of input port integer.
 * @param[in] connection status of input port integer.
 */
void AVInput::AVInputHotplug( int input , int connect, int type)
{
    LOGWARN("AVInputHotplug [%d, %d, %d]", input, connect, type);

    JsonObject params;
    params["devices"] = getInputDevices(type);
    sendNotify(AVINPUT_EVENT_ON_DEVICES_CHANGED, params);
}

/**
 * @brief This function is used to translate HDMI/COMPOSITE input signal change to
 * signalChanged event.
 *
 * @param[in] port HDMI/COMPOSITE In port id.
 * @param[in] signalStatus signal status of HDMI/COMPOSITE In port.
 */
void AVInput::AVInputSignalChange( int port , int signalStatus, int type)
{
    LOGWARN("AVInputSignalStatus [%d, %d, %d]", port, signalStatus, type);

    JsonObject params;
    params["id"] = port;
    std::stringstream locator;
    if (type == HDMI) {
        locator << "hdmiin://localhost/deviceid/" << port;
    }
    else {
        locator << "cvbsin://localhost/deviceid/" << port;
    }
    params["locator"] = locator.str();
    /* values of dsHdmiInSignalStatus_t and dsCompInSignalStatus_t are same
   Hence used only HDMI macro for case statement */
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
    sendNotify(AVINPUT_EVENT_ON_SIGNAL_CHANGED, params);
}

/**
 * @brief This function is used to translate HDMI/COMPOSITE input status change to
 * inputStatusChanged event.
 *
 * @param[in] port HDMI/COMPOSITE In port id.
 * @param[bool] isPresented HDMI/COMPOSITE In presentation started/stopped.
 */
void AVInput::AVInputStatusChange( int port , bool isPresented, int type)
{
    LOGWARN("avInputStatus [%d, %d, %d]", port, isPresented, type);

    JsonObject params;
    params["id"] = port;
    std::stringstream locator;
    if (type == HDMI) {
        locator << "hdmiin://localhost/deviceid/" << port;
    }
    else if (type == COMPOSITE) {
        locator << "cvbsin://localhost/deviceid/" << port;
    }
    params["locator"] = locator.str();

    if(isPresented) {
        params["status"] = "started";
    }
    else {
        params["status"] = "stopped";
    }
    sendNotify(AVINPUT_EVENT_ON_STATUS_CHANGED, params);
}

/**
 * @brief This function is used to translate HDMI input video mode change to
 * videoStreamInfoUpdate event.
 *
 * @param[in] port HDMI In port id.
 * @param[dsVideoPortResolution_t] video resolution data
 */
void AVInput::AVInputVideoModeUpdate( int port , dsVideoPortResolution_t resolution)
{
    LOGWARN("AVInputVideoModeUpdate [%d]", port);

    JsonObject params;
    params["id"] = port;
    std::stringstream locator;
    locator << "hdmiin://localhost/deviceid/" << port;
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

        case dsVIDEO_PIXELRES_1280x720:
            params["width"] = 1280;
            params["height"] = 720;
            break;

        case dsVIDEO_PIXELRES_1920x1080:
            params["width"] = 1920;
            params["height"] = 1080;
            break;

        case dsVIDEO_PIXELRES_3840x2160:
            params["width"] = 3840;
            params["height"] = 2160;
            break;

        case dsVIDEO_PIXELRES_4096x2160:
            params["width"] = 4096;
            params["height"] = 2160;
            break;

        default:
            params["width"] = 1920;
            params["height"] = 1080;
            break;
    }

    params["progressive"] = (!resolution.interlaced);

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

    sendNotify(AVINPUT_EVENT_ON_VIDEO_MODE_UPDATED, params);
}

void AVInput::dsAviContentTypeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
	if(!AVInput::_instance)
		return;

	if (IARM_BUS_DSMGR_EVENT_HDMI_IN_AVI_CONTENT_TYPE == eventId)
	{
		IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
		int hdmi_in_port = eventData->data.hdmi_in_content_type.port;
		int avi_content_type = eventData->data.hdmi_in_content_type.aviContentType;
		LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_IN_AVI_CONTENT_TYPE  event  port: %d, Content Type : %d", hdmi_in_port,avi_content_type);
AVInput::_instance->hdmiInputAviContentTypeChange(hdmi_in_port, avi_content_type);
	}
}

void AVInput::hdmiInputAviContentTypeChange( int port , int content_type)
{
	JsonObject params;
	params["id"] = port;
	params["aviContentType"] = content_type;
	sendNotify(AVINPUT_EVENT_ON_AVI_CONTENT_TYPE_CHANGED, params);
}


void AVInput::dsAVEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
    if(!AVInput::_instance)
        return;

    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
    if (IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG == eventId) {
        int hdmiin_hotplug_port = eventData->data.hdmi_in_connect.port;
        int hdmiin_hotplug_conn = eventData->data.hdmi_in_connect.isPortConnected;
        LOGWARN("Received IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG  event data:%d", hdmiin_hotplug_port);
        AVInput::_instance->AVInputHotplug(hdmiin_hotplug_port, hdmiin_hotplug_conn ? AV_HOT_PLUG_EVENT_CONNECTED : AV_HOT_PLUG_EVENT_DISCONNECTED, HDMI);
    }
    else if (IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_HOTPLUG == eventId) {
        int compositein_hotplug_port = eventData->data.composite_in_connect.port;
        int compositein_hotplug_conn = eventData->data.composite_in_connect.isPortConnected;
        LOGWARN("Received IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_HOTPLUG  event data:%d", compositein_hotplug_port);
        AVInput::_instance->AVInputHotplug(compositein_hotplug_port, compositein_hotplug_conn ? AV_HOT_PLUG_EVENT_CONNECTED : AV_HOT_PLUG_EVENT_DISCONNECTED, COMPOSITE);
    }
}

void AVInput::dsAVSignalStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
    if(!AVInput::_instance)
        return;
    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
    if (IARM_BUS_DSMGR_EVENT_HDMI_IN_SIGNAL_STATUS == eventId) {
        int hdmi_in_port = eventData->data.hdmi_in_sig_status.port;
        int hdmi_in_signal_status = eventData->data.hdmi_in_sig_status.status;
        LOGWARN("Received IARM_BUS_DSMGR_EVENT_HDMI_IN_SIGNAL_STATUS  event  port: %d, signal status: %d", hdmi_in_port,hdmi_in_signal_status);
        AVInput::_instance->AVInputSignalChange(hdmi_in_port, hdmi_in_signal_status, HDMI);
    }
    else if (IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_SIGNAL_STATUS == eventId) {
        int composite_in_port = eventData->data.composite_in_sig_status.port;
        int composite_in_signal_status = eventData->data.composite_in_sig_status.status;
        LOGWARN("Received IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_SIGNAL_STATUS  event  port: %d, signal status: %d", composite_in_port,composite_in_signal_status);
        AVInput::_instance->AVInputSignalChange(composite_in_port, composite_in_signal_status, COMPOSITE);
    }
}

void AVInput::dsAVStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
    if(!AVInput::_instance)
        return;
    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
    if (IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS == eventId) {
        int hdmi_in_port = eventData->data.hdmi_in_status.port;
        bool hdmi_in_status = eventData->data.hdmi_in_status.isPresented;
        LOGWARN("Received IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS  event  port: %d, started: %d", hdmi_in_port,hdmi_in_status);
        AVInput::_instance->AVInputStatusChange(hdmi_in_port, hdmi_in_status, HDMI);
    }
    else if (IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_STATUS == eventId) {
        int composite_in_port = eventData->data.composite_in_status.port;
        bool composite_in_status = eventData->data.composite_in_status.isPresented;
        LOGWARN("Received IARM_BUS_DSMGR_EVENT_COMPOSITE_IN_STATUS  event  port: %d, started: %d", composite_in_port,composite_in_status);
        AVInput::_instance->AVInputStatusChange(composite_in_port, composite_in_status, COMPOSITE);
    }
}

void AVInput::dsAVVideoModeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
    if(!AVInput::_instance)
        return;

    if (IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE == eventId) {
        IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
        int hdmi_in_port = eventData->data.hdmi_in_video_mode.port;
        dsVideoPortResolution_t resolution;
        resolution.pixelResolution =  eventData->data.hdmi_in_video_mode.resolution.pixelResolution;
        resolution.interlaced =  eventData->data.hdmi_in_video_mode.resolution.interlaced;
        resolution.frameRate =  eventData->data.hdmi_in_video_mode.resolution.frameRate;
        LOGWARN("Received IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE  event  port: %d, pixelResolution: %d, interlaced : %d, frameRate: %d \n", hdmi_in_port,resolution.pixelResolution, resolution.interlaced, resolution.frameRate);
        AVInput::_instance->AVInputVideoModeUpdate(hdmi_in_port, resolution);
    }
}

void AVInput::dsAVGameFeatureStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
    if(!AVInput::_instance)
        return;

    if (IARM_BUS_DSMGR_EVENT_HDMI_IN_ALLM_STATUS == eventId)
    {
        IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
        int hdmi_in_port = eventData->data.hdmi_in_allm_mode.port;
        bool allm_mode = eventData->data.hdmi_in_allm_mode.allm_mode;
        LOGWARN("Received IARM_BUS_DSMGR_EVENT_HDMI_IN_ALLM_STATUS  event  port: %d, ALLM Mode: %d", hdmi_in_port,allm_mode);

        AVInput::_instance->AVInputALLMChange(hdmi_in_port, allm_mode);
    }
}

void AVInput::AVInputALLMChange( int port , bool allm_mode)
{
    JsonObject params;
    params["id"] = port;
    params["gameFeature"] = "ALLM";
    params["mode"] = allm_mode;

    sendNotify(AVINPUT_EVENT_ON_GAME_FEATURE_STATUS_CHANGED, params);
}

uint32_t AVInput::getSupportedGameFeatures(const JsonObject& parameters, JsonObject& response)
{
    LOGINFOMETHOD();
    vector<string> supportedFeatures;
    try
    {
        device::HdmiInput::getInstance().getSupportedGameFeatures (supportedFeatures);
        for (size_t i = 0; i < supportedFeatures.size(); i++)
        {
            LOGINFO("Supported Game Feature [%zu]:  %s\n",i,supportedFeatures.at(i).c_str());
        }
    }
    catch (const device::Exception& err)
    {
        LOG_DEVICE_EXCEPTION0();
    }

    if (supportedFeatures.empty()) {
        returnResponse(false);
    }
    else {
        setResponseArray(response, "supportedGameFeatures", supportedFeatures);
        returnResponse(true);
    }
}

uint32_t AVInput::getGameFeatureStatusWrapper(const JsonObject& parameters, JsonObject& response)
{
    string sGameFeature = "";
    int portId = 0;

    LOGINFOMETHOD();
    if (parameters.HasLabel("portId") && parameters.HasLabel("gameFeature"))
    {
        try {
            portId = parameters["portId"].Number();
            sGameFeature = parameters["gameFeature"].String();
        }catch (...) {
            LOGWARN("Invalid Arguments");
            returnResponse(false);
        }
    }
    else {
        LOGWARN("Required parameters are not passed");
        returnResponse(false);
    }

    if (strcmp (sGameFeature.c_str(), "ALLM") == 0)
    {
        bool allm = getALLMStatus(portId);
        LOGWARN("AVInput::getGameFeatureStatusWrapper ALLM MODE:%d", allm);
        response["mode"] = allm;
    }
    else
    {
        LOGWARN("AVInput::getGameFeatureStatusWrapper Mode is not supported. Supported mode: ALLM");
        returnResponse(false);
    }
    returnResponse(true);
}

bool AVInput::getALLMStatus(int iPort)
{
    bool allm = false;

    try
    {
        device::HdmiInput::getInstance().getHdmiALLMStatus (iPort, &allm);
        LOGWARN("AVInput::getALLMStatus ALLM MODE: %d", allm);
    }
    catch (const device::Exception& err)
    {
        LOG_DEVICE_EXCEPTION1(std::to_string(iPort));
    }
    return allm;
}

uint32_t AVInput::getRawSPDWrapper(const JsonObject& parameters, JsonObject& response)
{
    LOGINFOMETHOD();

    int portId = 0;
    if (parameters.HasLabel("portId"))
    {
        try {
            portId = parameters["portId"].Number();
        }catch (...) {
            LOGWARN("Invalid Arguments");
            returnResponse(false);
        }
    }
    else {
        LOGWARN("Required parameters are not passed");
        returnResponse(false);
    }

    string spdInfo = getRawSPD (portId);
    response["HDMISPD"] = spdInfo;
    if (spdInfo.empty()) {
        returnResponse(false);
    }
    else {
        returnResponse(true);
    }
}

uint32_t AVInput::getSPDWrapper(const JsonObject& parameters, JsonObject& response)
{
    LOGINFOMETHOD();

    int portId = 0;
    if (parameters.HasLabel("portId"))
    {
        try {
        portId = parameters["portId"].Number();
        }catch (...) {
            LOGWARN("Invalid Arguments");
            returnResponse(false);
        }
    }
    else {
        LOGWARN("Required parameters are not passed");
        returnResponse(false);
    }

    string spdInfo = getSPD (portId);
    response["HDMISPD"] = spdInfo;
    if (spdInfo.empty()) {
        returnResponse(false);
    }
    else {
        returnResponse(true);
    }
}

std::string AVInput::getRawSPD(int iPort)
{
    LOGINFO("AVInput::getSPDInfo");
    vector<uint8_t> spdVect({'u','n','k','n','o','w','n' });
    std::string spdbase64 = "";
    try {
        LOGWARN("AVInput::getSPDInfo");
        vector<uint8_t> spdVect2;
        device::HdmiInput::getInstance().getHDMISPDInfo(iPort, spdVect2);
        spdVect = spdVect2;//edidVec must be "unknown" unless we successfully get to this line

        //convert to base64
        uint16_t size = min(spdVect.size(), (size_t)numeric_limits<uint16_t>::max());

        LOGWARN("AVInput::getSPD size:%d spdVec.size:%zu", size, spdVect.size());

        if(spdVect.size() > (size_t)numeric_limits<uint16_t>::max()) {
            LOGERR("Size too large to use ToString base64 wpe api");
            return spdbase64;
        }

        LOGINFO("------------getSPD: ");
        for (size_t itr =0; itr < spdVect.size(); itr++) {
            LOGINFO("%02X ", spdVect[itr]);
        }
        Core::ToString((uint8_t*)&spdVect[0], size, false, spdbase64);
    }
    catch (const device::Exception& err) {
        LOG_DEVICE_EXCEPTION1(std::to_string(iPort));
    }
    return spdbase64;
}

std::string AVInput::getSPD(int iPort)
{
    LOGINFO("AVInput::getSPDInfo");
    vector<uint8_t> spdVect({'u','n','k','n','o','w','n' });
    std::string spdbase64 = "";
    try {
        LOGWARN("AVInput::getSPDInfo");
        vector<uint8_t> spdVect2;
        device::HdmiInput::getInstance().getHDMISPDInfo(iPort, spdVect2);
        spdVect = spdVect2;//edidVec must be "unknown" unless we successfully get to this line

        //convert to base64
        uint16_t size = min(spdVect.size(), (size_t)numeric_limits<uint16_t>::max());

        LOGWARN("AVInput::getSPD size:%d spdVec.size:%zu", size, spdVect.size());

        if(spdVect.size() > (size_t)numeric_limits<uint16_t>::max()) {
            LOGERR("Size too large to use ToString base64 wpe api");
            return spdbase64;
        }

        LOGINFO("------------getSPD: ");
        for (size_t itr =0; itr < spdVect.size(); itr++) {
          LOGINFO("%02X ", spdVect[itr]);
        }
        if (spdVect.size() > 0) {
            struct dsSpd_infoframe_st pre;
            memcpy(&pre,spdVect.data(),sizeof(struct dsSpd_infoframe_st));

            char str[200] = {0};
            sprintf(str, "Packet Type:%02X,Version:%u,Length:%u,vendor name:%s,product des:%s,source info:%02X",
            pre.pkttype,pre.version,pre.length,pre.vendor_name,pre.product_des,pre.source_info);
            spdbase64 = str;
        }
    }
    catch (const device::Exception& err) {
        LOG_DEVICE_EXCEPTION1(std::to_string(iPort));
    }
    return spdbase64;
}

uint32_t AVInput::setEdidVersionWrapper(const JsonObject& parameters, JsonObject& response)
{
    LOGINFOMETHOD();
    int portId = 0;
    string sVersion = "";
    if (parameters.HasLabel("portId") && parameters.HasLabel("edidVersion"))
    {
        try {
            portId = parameters["portId"].Number();
            sVersion = parameters["edidVersion"].String();
        }catch (...) {
            LOGWARN("Invalid Arguments");
            returnResponse(false);
        }
    }
    else {
        LOGWARN("Required parameters are not passed");
        returnResponse(false);
    }

    int edidVer = -1;
    if (strcmp (sVersion.c_str(), "HDMI1.4") == 0) {
        edidVer = HDMI_EDID_VER_14;
    }
    else if (strcmp (sVersion.c_str(), "HDMI2.0") == 0) {
        edidVer = HDMI_EDID_VER_20;
    }

    if (edidVer < 0) {
        returnResponse(false);
    }
    bool result = setEdidVersion (portId, edidVer);
    if (result == false) {
        returnResponse(false);
    }
    else {
        returnResponse(true);
    }
}

int AVInput::setEdidVersion(int iPort, int iEdidVer)
{
    bool ret = true;
    try {
        device::HdmiInput::getInstance().setEdidVersion (iPort, iEdidVer);
        LOGWARN("AVInput::setEdidVersion EDID Version:%d", iEdidVer);
    }
    catch (const device::Exception& err) {
        LOG_DEVICE_EXCEPTION1(std::to_string(iPort));
        ret = false;
    }
    return ret;
}

uint32_t AVInput::getEdidVersionWrapper(const JsonObject& parameters, JsonObject& response)
{
    int portId = 0;

    LOGINFOMETHOD();
    if (parameters.HasLabel("portId"))
    {
        try {
            portId = parameters["portId"].Number();
        }
        catch (...) {
            LOGWARN("Invalid Arguments");
            returnResponse(false);
        }
    }
    else {
        LOGWARN("Required parameters are not passed");
        returnResponse(false);
    }

    int edidVer = getEdidVersion (portId);
    switch (edidVer) {
        case HDMI_EDID_VER_14:
            response["edidVersion"] = "HDMI1.4";
            break;
        case HDMI_EDID_VER_20:
            response["edidVersion"] = "HDMI2.0";
            break;
    }

    if (edidVer < 0) {
        returnResponse(false);
    }
    else {
        returnResponse(true);
    }
}

int AVInput::getEdidVersion(int iPort)
{
    int edidVersion = -1;

    try {
        device::HdmiInput::getInstance().getEdidVersion (iPort, &edidVersion);
        LOGWARN("AVInput::getEdidVersion EDID Version:%d", edidVersion);
    }
    catch (const device::Exception& err) {
        LOG_DEVICE_EXCEPTION1(std::to_string(iPort));
    }
    return edidVersion;
}

} // namespace Plugin
} // namespace WPEFramework
