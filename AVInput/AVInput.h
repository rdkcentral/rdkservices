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

public:
    void event_onAVInputActive(int id);
    void event_onAVInputInactive(int id);

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

public:
    static AVInput* _instance;
};

} // namespace Plugin
} // namespace WPEFramework
