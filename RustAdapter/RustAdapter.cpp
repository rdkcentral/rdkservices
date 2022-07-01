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
#include "RustAdapter.h"
#include "LocalPlugin.h"
#include "RemotePlugin.h"
#include "Logger.h"
#include <plugins/Channel.h>

namespace WPEFramework {
namespace Plugin {
  SERVICE_REGISTRATION(RustAdapter, 1, 0);
} }

WPEFramework::Plugin::RustAdapter::RustAdapter()
  : m_refcount(1)
{
}

const string
WPEFramework::Plugin::RustAdapter::Initialize(PluginHost::IShell *shell)
{
  /* The RustAdapter plugin should always run in-process
     We run the rust plugin itself either in-process or out-of-process
     base the config setting */

  RustAdapter::Config conf;
  conf.FromString(shell->ConfigLine());

  LOGINFO("RustAdapter::Initialize Config=%s", shell->ConfigLine().c_str());

  if (conf.OutOfProcess)
    m_impl.reset(new WPEFramework::Plugin::Rust::RemotePlugin(conf));
  else
    m_impl.reset(new WPEFramework::Plugin::Rust::LocalPlugin(conf));

  return m_impl->Initialize(shell);
}

void
WPEFramework::Plugin::RustAdapter::Deinitialize(PluginHost::IShell *shell)
{
  return m_impl->Deinitialize(shell);
}

string
WPEFramework::Plugin::RustAdapter::Information() const
{
  return m_impl->Information();
}

void
WPEFramework::Plugin::RustAdapter::AddRef() const
{
  m_refcount++;
}

uint32_t
WPEFramework::Plugin::RustAdapter::Release() const
{
  uint32_t n = m_refcount.fetch_sub(1);
  if (n == 1) {
    delete this;
    return Core::ERROR_DESTRUCTION_SUCCEEDED;
  }

  return Core::ERROR_NONE;
}

#if JSON_RPC_CONTEXT
WPEFramework::Core::ProxyType<WPEFramework::Core::JSONRPC::Message>
WPEFramework::Plugin::RustAdapter::Invoke(
  const WPEFramework::Core::JSONRPC::Context &ctx,
  const WPEFramework::Core::JSONRPC::Message &req)
{
  return m_impl->Invoke(ctx, req);
}
#else
WPEFramework::Core::ProxyType<WPEFramework::Core::JSONRPC::Message> 
  WPEFramework::Plugin::RustAdapter::Invoke(
    const string& token, const uint32_t channelId, const WPEFramework::Core::JSONRPC::Message& req)
{
  return m_impl->Invoke(token, channelId, req);
}
#endif

void
WPEFramework::Plugin::RustAdapter::Activate(
  WPEFramework::PluginHost::IShell *shell)
{
  m_impl->Activate(shell);
}

void
WPEFramework::Plugin::RustAdapter::Deactivate()
{
  m_impl->Deactivate();
}

bool
WPEFramework::Plugin::RustAdapter::Attach(PluginHost::Channel &channel)
{
  return m_impl->Attach(channel);
}

void
WPEFramework::Plugin::RustAdapter::Detach(PluginHost::Channel &channel)
{
  m_impl->Detach(channel);
}

WPEFramework::Core::ProxyType<WPEFramework::Core::JSON::IElement>
WPEFramework::Plugin::RustAdapter::Inbound(const string &identifier)
{
  return m_impl->Inbound(identifier);
}

WPEFramework::Core::ProxyType<WPEFramework::Core::JSON::IElement>
WPEFramework::Plugin::RustAdapter::Inbound(const uint32_t id,
    const Core::ProxyType<Core::JSON::IElement> &element)
{
  return m_impl->Inbound(id, element);
}

std::string
WPEFramework::Plugin::RustAdapter::GetLibraryPathOrName(
  const std::string &lib_name,
  const std::string &callsign)
{
  std::stringstream shared_library_name;
  if (!lib_name.empty())
    shared_library_name << lib_name;
  else {
    shared_library_name << "lib";
    for (char c : callsign)
      shared_library_name << static_cast<char>(std::tolower(c));
    shared_library_name << ".so";
  }
  return shared_library_name.str();
}

//
// TODO: remove this and link directly with security agent. Didn't want to break build
//
// copied from securityagent.so (ipclink.cpp)
int RustGetAuthToken(unsigned short maxLength, unsigned short inLength, unsigned char buffer[])
{
  using namespace WPEFramework;

  auto nodeId = Core::NodeId{ "/tmp/SecurityAgent/token" };
  auto engine = Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create();
  auto client = Core::ProxyType<RPC::CommunicatorClient>::Create(nodeId, Core::ProxyType<Core::IIPCServer>(engine));

  int result = -1;
  if ((client.IsValid() == true) && (client->IsOpen() == false)) {
    PluginHost::IAuthenticate* securityAgentInterface = client->Open<PluginHost::IAuthenticate>("SecurityAgent");

    if (securityAgentInterface != nullptr) {
      std::string token;
      uint32_t error = securityAgentInterface->CreateToken(inLength, buffer, token);

      if (error == Core::ERROR_NONE) {
        result = static_cast<uint32_t>(token.length());

        if (result <= maxLength) {
          std::copy(std::begin(token), std::end(token), buffer);
        } else {
          printf("%s:%d [%s] Received token is too long [%d].\n", __FILE__, __LINE__, __func__, result);
          result = -result;
        }
      } else {
        result = error;
        result = -result;
      }

      securityAgentInterface->Release();
    }

    client.Release();
  } else {
    printf("%s:%d [%s] Could not open link. error=%d\n", __FILE__, __LINE__, __func__, result);
  }

  return (result);
}

std::string
WPEFramework::Plugin::RustAdapter::GetAuthToken(const std::string &callsign)
{
  std::string auth_token;

  char buff[256];
  memset(buff, 0, sizeof(buff));

  int n = snprintf(buff, sizeof(buff), "plugin://%s", callsign.c_str());
  buff[255] = '\0';

  // GetToken is from securityagent.h in ThunderClientLibraries lib
  int ret = RustGetAuthToken(sizeof(buff), n, (unsigned char *) buff);
  if (ret > 0)
    auth_token = std::string(buff, 0, ret);
  else {
    LOGERR("Failed to get authorizaton token for plugin %s. %d",
      callsign.c_str(), ret);
  }

  return auth_token;
}