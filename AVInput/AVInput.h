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

#include "Module.h"
#include "libIBus.h"
#include "dsTypes.h"
#include <interfaces/json/JsonData_AVInput.h>

namespace WPEFramework {
namespace Plugin {

class AVInput: public PluginHost::IPlugin, public PluginHost::JSONRPC
{
private:
    AVInput(const AVInput &) = delete;
    AVInput &operator=(const AVInput &) = delete;

public:
    AVInput();
    virtual ~AVInput();

    BEGIN_INTERFACE_MAP(AVInput)
    INTERFACE_ENTRY(PluginHost::IPlugin)
    INTERFACE_ENTRY(PluginHost::IDispatcher)
    END_INTERFACE_MAP

public:
    //   IPlugin methods
    // -------------------------------------------------------------------------------------------------------
    virtual const string Initialize(PluginHost::IShell *service) override;
    virtual void Deinitialize(PluginHost::IShell *service) override;
    virtual string Information() const override;

protected:
    void InitializeIARM();
    void DeinitializeIARM();

    void RegisterAll();
    void UnregisterAll();

    uint32_t endpoint_numberOfInputs(const JsonObject &parameters, JsonObject &response);
    uint32_t endpoint_currentVideoMode(const JsonObject &parameters, JsonObject &response);
    uint32_t endpoint_contentProtected(const JsonObject &parameters, JsonObject &response);

private:
    static int numberOfInputs(bool &success);
    static string currentVideoMode(bool &success);

    //Begin methods
    uint32_t getInputDevicesWrapper(const JsonData::AVInput::GetInputDevicesParamsInfo& parameters, JsonData::AVInput::GetInputDevicesResultInfo& response);
    uint32_t writeEDIDWrapper(const JsonData::AVInput::WriteEDIDParamsData& parameters);
    uint32_t readEDIDWrapper(const JsonData::AVInput::GetEdidVersionParamsInfo& parameters, JsonData::AVInput::ReadEDIDResultData& response);
    uint32_t getRawSPDWrapper(const JsonData::AVInput::GetEdidVersionParamsInfo& parameters, JsonData::AVInput::GetSPDResultInfo& response);
    uint32_t getSPDWrapper(const JsonData::AVInput::GetEdidVersionParamsInfo& parameters, JsonData::AVInput::GetSPDResultInfo& response);
    uint32_t setEdidVersionWrapper(const JsonData::AVInput::SetEdidVersionParamsData& parameters);
    uint32_t getEdidVersionWrapper(const JsonData::AVInput::GetEdidVersionParamsInfo& parameters, JsonData::AVInput::GetEdidVersionResultData& response);
    uint32_t startInput(const JsonData::AVInput::StartInputParamsData& parameters);
    uint32_t stopInput(const JsonData::AVInput::GetInputDevicesParamsInfo& parameters);
    uint32_t setVideoRectangleWrapper(const JsonData::AVInput::SetVideoRectangleParamsData& parameters);
    uint32_t getSupportedGameFeatures(JsonData::AVInput::GetSupportedGameFeaturesResultData& response);
    uint32_t getGameFeatureStatusWrapper(const JsonData::AVInput::GetGameFeatureStatusParamsData& parameters, JsonData::AVInput::GetGameFeatureStatusResultData& response);
    //End methods

    JsonArray getInputDevices(int iType);
    void writeEDID(int deviceId, std::string message);
    std::string readEDID(int iPort);
    std::string getRawSPD(int iPort);
    std::string getSPD(int iPort);
    int setEdidVersion(int iPort, int iEdidVer);
    int getEdidVersion(int iPort);
    bool setVideoRectangle(int x, int y, int width, int height, int type);
    bool getALLMStatus(int iPort);

    void AVInputHotplug(int input , int connect, int type);
    static void dsAVEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

    void AVInputSignalChange( int port , int signalStatus, int type);
    static void dsAVSignalStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

    void AVInputStatusChange( int port , bool isPresented, int type);
    static void dsAVStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

    void AVInputVideoModeUpdate( int port , dsVideoPortResolution_t resolution);
    static void dsAVVideoModeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

    void AVInputALLMChange( int port , bool allmMode);
    static void dsAVGameFeatureStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

public:
    static AVInput* _instance;
};

} // namespace Plugin
} // namespace WPEFramework
