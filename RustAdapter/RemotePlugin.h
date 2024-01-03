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
#include "SocketServer.h"

namespace WPEFramework {
namespace Plugin {
namespace Rust {

class RemotePlugin : public Rust::IPlugin
{
public:

  /**
   *
   */
  RemotePlugin(const RustAdapter::Config &conf);

  /**
   *
   */
  ~RemotePlugin() override = default;

  RemotePlugin(const RemotePlugin &) = delete;
  RemotePlugin& operator = (const RemotePlugin &) = delete;

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

  BEGIN_INTERFACE_MAP(RustAdapter)
  INTERFACE_ENTRY(PluginHost::IPlugin)
  INTERFACE_ENTRY(PluginHost::IPluginExtended)
  INTERFACE_ENTRY(PluginHost::IDispatcher)
  INTERFACE_ENTRY(PluginHost::IWebSocket)
  END_INTERFACE_MAP

  /**
   * IDispatcher::Activate
   */
#if ((THUNDER_VERSION_MAJOR == 4) || (THUNDER_VERSION_MAJOR == 2))
#if (THUNDER_VERSION_MINOR != 4)
  void Activate(PluginHost::IShell *shell) override;
  /**
   *
   */
  void Deactivate() override;
#endif
#endif
  /**
   *
   */
  bool Attach(PluginHost::Channel &channel) override;
  void Detach(PluginHost::Channel &channel) override;

  /**
   * IDispatcher::Close
   */
#if ((THUNDER_VERSION_MAJOR >= 4) && (THUNDER_VERSION_MINOR == 2))
  void Close(const uint32_t channelId) override;
#endif

  /**
   * IDispatcher::Close
   */
#if ((THUNDER_VERSION_MAJOR >= 4 && THUNDER_VERSION_MINOR == 4))
  Core::hresult Revoke(ICallback* callback) override;
  Core::hresult Validate(const string& token, const string& method, const string& paramaters /* @restrict:(4M-1) */) const override;
#endif /* THUNDER_VERSION */

  /**
   * WPEFramework::PluginHost::IDispatcher::Invoke
   */
#if JSON_RPC_CONTEXT   

#if ((THUNDER_VERSION_MAJOR >= 4) && (THUNDER_VERSION_MINOR == 4))
  Core::hresult Invoke(ICallback* callback, const uint32_t channelId, const uint32_t id, const string& token, const string& method, const string& parameters, string& response ) override;
#else
  Core::ProxyType<Core::JSONRPC::Message> Invoke(
    const Core::JSONRPC::Context& context,
    const Core::JSONRPC::Message& message) override;
#endif
#else
  Core::ProxyType<Core::JSONRPC::Message> Invoke(
    const string& token, const uint32_t channelId, const Core::JSONRPC::Message& req) override;
#endif
  /**
   *
   */
  Core::ProxyType<Core::JSON::IElement> Inbound(const string &identifier) override;
  Core::ProxyType<Core::JSON::IElement> Inbound(const uint32_t id,
    const Core::ProxyType<Core::JSON::IElement> &element) override;

  void onRead(const Response& rsp);
#if ((THUNDER_VERSION_MAJOR == 4) && (THUNDER_VERSION_MINOR == 4))
public:
   WPEFramework::PluginHost::ILocalDispatcher* Local() override {
        return nullptr; // Replace nullptr with your actual implementation.
    }
#endif

private:
  int LaunchRemoteProcess(const string& rust_shared_lib, const string& host_ip, int port);
  void SendTo(uint32_t channel_id, const char *json);

  // we keep a pointer to this to allow rust code to callback into
  // the Adapter and send messages/events asynchronously
  // XXX: We could also capture a reference to the channel during
  // attach/detach, but that may require API changes to Thunder/internal
  PluginHost::IShell *m_service;
  int m_remotePid;
  SocketServer m_stream;
  RustAdapter::Config m_config;
  std::string m_auth_token;
};

} } }
