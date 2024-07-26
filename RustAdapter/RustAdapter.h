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
    Config& operator=(const Config&);
  public:
    Config(const Config &rhs)
    {
      Add(_T("outofprocess"), &OutOfProcess);
      Add(_T("address"), &Address);
      Add(_T("port"), &Port);
      Add(_T("autoexec"), &AutoExec);
      Add(_T("libname"), &LibName);
      OutOfProcess = rhs.OutOfProcess;;
      Address = rhs.Address;
      Port = rhs.Port;
      AutoExec = rhs.AutoExec;
      LibName = rhs.LibName;
    }

    Config() : Core::JSON::Container(), OutOfProcess(false)
    {
      Add(_T("outofprocess"), &OutOfProcess);
      Add(_T("address"), &Address);
      Add(_T("port"), &Port);
      Add(_T("autoexec"), &AutoExec);
      Add(_T("libname"), &LibName);
    }
    ~Config()
    {
    }
  public:
    Core::JSON::Boolean OutOfProcess;
    Core::JSON::String Address;
    Core::JSON::DecUInt16 Port;
    Core::JSON::Boolean AutoExec;
    Core::JSON::String LibName;
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
#if ((THUNDER_VERSION == 2) || ((THUNDER_VERSION == 4) && (THUNDER_VERSION_MINOR == 2)))
  void Activate(PluginHost::IShell *shell) override;
  /**
   *
   */
  void Deactivate() override;
#endif
  /**
   *
   */
  bool Attach(PluginHost::Channel &channel) override;
  void Detach(PluginHost::Channel &channel) override;

  /**
   * IDispatcher::Close
   */
#if (THUNDER_VERSION >= 4)
#if (THUNDER_VERSION_MINOR == 2)
  void Close(const uint32_t channelId) override;
#endif

#if (THUNDER_VERSION_MINOR == 4)
  Core::hresult Revoke(ICallback* callback) override;
  Core::hresult Validate(const string& token, const string& method, const string& paramaters /* @restrict:(4M-1) */) const override;
#endif
#endif
  /**
   * WPEFramework::PluginHost::IDispatcher::Invoke
   */
#if JSON_RPC_CONTEXT

#if ((THUNDER_VERSION >= 4) && (THUNDER_VERSION_MINOR == 4))
  Core::hresult Invoke(ICallback* callback, const uint32_t channelId, const uint32_t id, const string& token, const string& method, const string& parameters, string& response ) override;
#else
  Core::ProxyType<Core::JSONRPC::Message> Invoke(
    const Core::JSONRPC::Context& context,
    const Core::JSONRPC::Message& message) override;
#endif

#else 
  Core::ProxyType<Core::JSONRPC::Message> Invoke(
    const string& token, const uint32_t channelId, const Core::JSONRPC::Message& message) override;
#endif

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

  static std::string GetLibraryPathOrName(
    const std::string& libname,
    const std::string& callsign);

  static std::string GetAuthToken(WPEFramework::PluginHost::IShell* service, const std::string &callsign);

#if ((THUNDER_VERSION >= 4) && (THUNDER_VERSION_MINOR == 4))
public:
   WPEFramework::PluginHost::ILocalDispatcher* Local() override {
        return nullptr; // Replace nullptr with your actual implementation.
    }
#endif

private:
  std::unique_ptr<Rust::IPlugin> m_impl;

  // needs to be mutable because Release() is const
  mutable std::atomic<uint32_t> m_refcount;
};

} }
