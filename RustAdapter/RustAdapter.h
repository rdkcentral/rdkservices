/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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
#pragma once

#include "Module.h"
#include "RustPlugin.h"

namespace WPEFramework {
namespace Plugin {

namespace Rust {
  struct RequestContext;
  struct PluginContext;
  struct Plugin;
}

class RustAdapter : public Rust::IPlugin
{
public:
  class Config : public Core::JSON::Container
  {
  private:
    Config(const Config&);
    Config& operator=(const Config&);
  public:
    Config() : Core::JSON::Container(), OutOfProcess(true)
    {
      Add(_T("outofprocess"), &OutOfProcess);
      Add(_T("address"), &Address);
      Add(_T("port"), &Port);
      Add(_T("autoexec"), &AutoExec);
    }
    ~Config()
    {
    }
  public:
    Core::JSON::Boolean OutOfProcess;
    Core::JSON::String Address;
    Core::JSON::DecUInt16 Port;
    Core::JSON::Boolean AutoExec;
  };

  /**
   *
   */
  RustAdapter();

  /**
   *
   */
  ~RustAdapter() override = default;

  RustAdapter(const RustAdapter &) = delete;
  RustAdapter& operator = (const RustAdapter &) = delete;

  /**
   * IPlugin::Initialize
   */
  const string Initialize(PluginHost::IShell *shell) override;

  /**
   * IPlugin::Deinitialize
   */
  void Deinitialize(PluginHost::IShell *shell) override;

  /**
   * IPlugin Information
   */
  string Information() const override;

  /**
   * IDispatcher -> IUknown -> IReferenceCounted::AddRef
   */
  void AddRef() const override;

  /**
   * IDispatcher -> IUnknown -> IReferenceCounted::AddRef
   */
  uint32_t Release() const override;

  /**
   * IDispatcher::Activate
   */
  void Activate(PluginHost::IShell *shell) override;

  /**
   *
   */
  void Deactivate() override;

  /**
   *
   */
  bool Attach(PluginHost::Channel &channel) override;
  void Detach(PluginHost::Channel &channel) override;

  /**
   * WPEFramework::PluginHost::IDispatcher::Invoke
   */
  Core::ProxyType<Core::JSONRPC::Message> Invoke(
    const Core::JSONRPC::Context& context,
    const Core::JSONRPC::Message& message) override;

  /**
   *
   */
  Core::ProxyType<Core::JSON::IElement> Inbound(const string &identifier) override;
  Core::ProxyType<Core::JSON::IElement> Inbound(const uint32_t id,
    const Core::ProxyType<Core::JSON::IElement> &element) override;

  BEGIN_INTERFACE_MAP(RustAdapter)
  INTERFACE_ENTRY(PluginHost::IPlugin)
  INTERFACE_ENTRY(PluginHost::IPluginExtended)
  INTERFACE_ENTRY(PluginHost::IDispatcher)
  INTERFACE_ENTRY(PluginHost::IWebSocket)
  END_INTERFACE_MAP

  const Config& GetConfig() const
  {
    return m_config;
  }
private:
  std::unique_ptr<Rust::IPlugin> m_impl;
  Config m_config;

  // needs to be mutable because Release() is const
  mutable std::atomic<uint32_t> m_refcount;
};

} }
