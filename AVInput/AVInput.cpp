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

#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

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

namespace {
void dsHdmiEventHandler(const char *, IARM_EventId_t eventId, void *, size_t)
{
    if ((IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG == eventId) &&
        (AVInput::_instance != nullptr)) {
        try {
            int num = device::HdmiInput::getInstance().getNumberOfInputs();
            if (num > 0) {
                for (int i = 0; i < num; i++) {
                    if (device::HdmiInput::getInstance().isPortConnected(i)) {
                        AVInput::_instance->event_onAVInputActive(i);
                    }
                    else {
                        AVInput::_instance->event_onAVInputInactive(i);
                    }
                }
            }
        }
        catch (...) {
            LOGERR("Exception caught");
        }
    }
}
}

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
            dsHdmiEventHandler));
    }
}

void AVInput::DeinitializeIARM()
{
    if (Utils::IARM::isConnected()) {
        IARM_Result_t res;
        IARM_CHECK(IARM_Bus_UnRegisterEventHandler(
            IARM_BUS_DSMGR_NAME,
            IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG));
    }
}

void AVInput::RegisterAll()
{
    Register<JsonObject, JsonObject>(_T("numberOfInputs"), &AVInput::endpoint_numberOfInputs, this);
    Register<JsonObject, JsonObject>(_T("currentVideoMode"), &AVInput::endpoint_currentVideoMode, this);
    Register<JsonObject, JsonObject>(_T("contentProtected"), &AVInput::endpoint_contentProtected, this);
}

void AVInput::UnregisterAll()
{
    Unregister(_T("numberOfInputs"));
    Unregister(_T("currentVideoMode"));
    Unregister(_T("contentProtected"));
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

void AVInput::event_onAVInputActive(int id)
{
    JsonObject params;
    params[_T("url")] = "avin://input" + std::to_string(id);
    sendNotify(_T("onAVInputActive"), params);
}

void AVInput::event_onAVInputInactive(int id)
{
    JsonObject params;
    params[_T("url")] = "avin://input" + std::to_string(id);
    sendNotify(_T("onAVInputInactive"), params);
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

} // namespace Plugin
} // namespace WPEFramework
