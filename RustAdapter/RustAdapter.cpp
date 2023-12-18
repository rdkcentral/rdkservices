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

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 1

namespace WPEFramework {
namespace {
    static Plugin::Metadata<Plugin::RustAdapter> metadata(
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

#if (THUNDER_VERSION_MAJOR == 4 && THUNDER_VERSION_MINOR == 4)
WPEFramework::Core::hresult
WPEFramework::Plugin::RustAdapter::Invoke(ICallback* callback, const uint32_t channelId, const uint32_t id, const string& token, const string& method, const string& parameters, string& response )
{
  return m_impl->Invoke(callback, channelId,id, token, method, parameters, response);
}
#else
WPEFramework::Core::ProxyType<WPEFramework::Core::JSONRPC::Message>
WPEFramework::Plugin::RustAdapter::Invoke(
  const WPEFramework::Core::JSONRPC::Context &ctx,
  const WPEFramework::Core::JSONRPC::Message &req)
{
  return m_impl->Invoke(ctx, req);
}
#endif

#else
WPEFramework::Core::ProxyType<WPEFramework::Core::JSONRPC::Message> 
  WPEFramework::Plugin::RustAdapter::Invoke(
    const string& token, const uint32_t channelId, const WPEFramework::Core::JSONRPC::Message& req)
{
  return m_impl->Invoke(token, channelId, req);
}
#endif

#if (THUNDER_VERSION_MAJOR == 4 && THUNDER_VERSION_MINOR != 4)
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

#endif
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

#if (THUNDER_VERSION_MAJOR == 4 && THUNDER_VERSION_MINOR != 4)
void
WPEFramework::Plugin::RustAdapter::Close(const uint32_t channelId)
{
    m_impl->Close(channelId);
}
#endif /* THUNDER_VERSION */

#if (THUNDER_VERSION_MAJOR == 4 && THUNDER_VERSION_MINOR == 4)
WPEFramework::Core::hresult WPEFramework::Plugin::RustAdapter::Revoke(ICallback* callback)
{
     return {};
}

WPEFramework::Core::hresult WPEFramework::Plugin::RustAdapter::Validate(const string& token, const string& method, const string& paramaters) const
{
  return {};
}

#endif

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

std::string
WPEFramework::Plugin::RustAdapter::GetAuthToken(WPEFramework::PluginHost::IShell* service, const std::string &callsign)
{
  std::string auth_token;

  char buff[256];
  memset(buff, 0, sizeof(buff));

  int n = snprintf(buff, sizeof(buff), "plugin://%s", callsign.c_str());
  buff[255] = '\0';

  auto auth = service->QueryInterfaceByCallsign<WPEFramework::PluginHost::IAuthenticate>("SecurityAgent");
  if (auth != nullptr) {
    string encoded;
    if (auth->CreateToken(n, (const uint8_t*) buff, encoded) == WPEFramework::Core::ERROR_NONE) {
      auth_token = encoded;
    }
    auth->Release();
  }

  return auth_token;
}
