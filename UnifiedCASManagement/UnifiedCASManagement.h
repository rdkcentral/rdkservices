/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright Synamedia, All rights reserved
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

#ifndef UNIFIEDCASMANAGEMENT_H
#define UNIFIEDCASMANAGEMENT_H

#include "Module.h"
#include "MediaPlayer.h"

namespace WPEFramework 
{

namespace Plugin 
{

class UnifiedCASManagement : public PluginHost::IPlugin, PluginHost::JSONRPC 
{

public:
    UnifiedCASManagement();
    UnifiedCASManagement(const UnifiedCASManagement& orig) = delete;
    virtual ~UnifiedCASManagement();

    BEGIN_INTERFACE_MAP(UnifiedCASManagement)
    INTERFACE_ENTRY(PluginHost::IPlugin)
    INTERFACE_ENTRY(PluginHost::IDispatcher)
    END_INTERFACE_MAP

public/*members*/:
    //   IPlugin methods
    // -------------------------------------------------------------------------------------------------------
    virtual const std::string Initialize(PluginHost::IShell *service) override;
    virtual void Deinitialize(PluginHost::IShell *service) override;
    virtual std::string Information() const override; 

    void event_data(const std::string& payload, const std::string& source);
    static UnifiedCASManagement* _instance;

    static const std::string METHOD_MANAGE;
    static const std::string METHOD_UNMANAGE;
    static const std::string METHOD_SEND;    
    static const std::string EVENT_DATA;    
        
private/*registered methods*/:
    void RegisterAll();
    void UnregisterAll();
    uint32_t manage(const JsonObject& params, JsonObject& response);
    uint32_t unmanage(const JsonObject& params, JsonObject& response);
    uint32_t send(const JsonObject& params, JsonObject& response);

private/*members*/:
    std::shared_ptr<MediaPlayer> m_player;
        
};
    
} // namespace Plugin

} // namespace WPEFramework
#endif /* UNIFIEDCASMANAGEMENT_H */

