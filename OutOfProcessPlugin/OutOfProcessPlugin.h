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
#include <interfaces/IOutOfProcessPlugin.h>
namespace WPEFramework {
namespace Plugin {

class OutOfProcessPlugin: public PluginHost::IPlugin,
              public PluginHost::IWeb,
              public PluginHost::JSONRPC {
private:
  OutOfProcessPlugin(const OutOfProcessPlugin&);
  OutOfProcessPlugin& operator=(const OutOfProcessPlugin&);


public:
    OutOfProcessPlugin() 
    : _implementation(nullptr)
    , _connectionId(0)
    , _service(nullptr) {
    RegisterAll();
  }

  virtual ~OutOfProcessPlugin() {
    TRACE_L1("Destructor OutOfProcessPlugin.%d", __LINE__);
    UnregisterAll();
  }

public:
    BEGIN_INTERFACE_MAP (OutOfProcessPlugin)
    INTERFACE_ENTRY (PluginHost::IPlugin)
    INTERFACE_ENTRY (PluginHost::IWeb)
    INTERFACE_ENTRY (PluginHost::IDispatcher)
    INTERFACE_AGGREGATE(Exchange::IOutOfProcessPlugin, _implementation)
    END_INTERFACE_MAP

public:
  //  IPlugin methods
  // -------------------------------------------------------------------------------------------------------
  virtual const string Initialize(PluginHost::IShell* service);
  virtual void Deinitialize(PluginHost::IShell *service);
  virtual string Information() const;

  //  IWeb methods
  // -------------------------------------------------------------------------------------------------------
  virtual void Inbound(Web::Request &request);
  virtual Core::ProxyType<Web::Response> Process(const Web::Request &request);

private:
  void Deactivated(RPC::IRemoteConnection *connection);

  inline void ConnectionTermination(uint32_t connectionId)
  {
    RPC::IRemoteConnection* connection(_service->RemoteConnection(connectionId));
    if (connection != nullptr) {
      connection->Terminate();
      connection->Release();
    }
  }

  // JsonRpc
  void RegisterAll();
  void UnregisterAll();
 
  uint32_t get_fps(Core::JSON::DecUInt32 &response) const;
  uint32_t getpluginid(Core::JSON::DecUInt32 &response) const;

private:
  uint32_t _connectionId;
  PluginHost::IShell *_service;
  Exchange::IOutOfProcessPlugin* _implementation;

};

}  // namespace Plugin
}  // namespace WPEFramework
