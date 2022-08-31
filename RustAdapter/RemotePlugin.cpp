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
#include "RemotePlugin.h"
#include "SocketServer.h"
#include "Logger.h"
#include <plugins/Channel.h>

namespace WPEFramework {
namespace Plugin {
namespace Rust {

RemotePlugin::RemotePlugin(const RustAdapter::Config &conf)
  : m_remotePid(0)
  , m_config(conf)
{
}

const string
RemotePlugin::Initialize(PluginHost::IShell *shell)
{
  m_service = shell;
  m_auth_token = RustAdapter::GetAuthToken(shell, shell->Callsign());

  string address = m_config.Address.Value();
  printf("ADDR=%s\n", address.c_str());
  if (address.empty())
    address = "127.0.0.1";

  if (m_stream.Open(address, 
                    m_config.Port.Value(),
                    std::bind(&RemotePlugin::onRead, this, std::placeholders::_1)) < 0)
  {
    return string("RustAdapter RemotePlugin couldn't open socket stream");
  }

  if (m_stream.RunThread() < 0)
  {
    return string("RustAdapter RemotePlugin failed to run stream thread");
  }

  if (m_config.AutoExec)
  {
    std::string lib_name = RustAdapter::GetLibraryPathOrName(m_config.LibName.Value(), shell->Callsign());
    if ((m_remotePid = LaunchRemoteProcess(lib_name, m_stream.GetAddress(), m_stream.GetPort())) < 0)
    {
      return string("RustAdapter RemotePlugin failed spawn remote process");
    }
  }

  return {};
}

void
RemotePlugin::Deinitialize(PluginHost::IShell *shell)
{
  
  LOGDBG("Deinitialize: send exit message to any connected client");
  m_stream.SendExit();

  if (m_remotePid > 0)
  {
    int status;
    LOGDBG("Deinitialize: waiting on remote %d to close", m_remotePid);
    waitpid(m_remotePid, &status, 0);
    LOGDBG("Deinitialize:remote closed status=%d", status);
  }

  m_stream.Close();
  LOGDBG("RemotePlugin::Deinitialize exit");
}

void
RemotePlugin::SendTo(uint32_t channel_id, const char *json)
{
  auto res = Core::ProxyType<Web::JSONBodyType<Core::JSONRPC::Message>>::Create();
  res->FromString(json);
  m_service->Submit(channel_id, Core::ProxyType<Core::JSON::IElement>(res));
}
#if JSON_RPC_CONTEXT
WPEFramework::Core::ProxyType<WPEFramework::Core::JSONRPC::Message>
RemotePlugin::Invoke(
  const WPEFramework::Core::JSONRPC::Context &ctx,
  const WPEFramework::Core::JSONRPC::Message &req)
{
  string json;
  req.ToString(json);
  m_stream.SendInvoke(ctx.ChannelId(), ctx.Token(), json);
  return {};
}
#else
WPEFramework::Core::ProxyType<WPEFramework::Core::JSONRPC::Message>
RemotePlugin::Invoke(
    const string& token, const uint32_t channelId, const Core::JSONRPC::Message& req)
{
  string json;
  req.ToString(json);
  m_stream.SendInvoke(channelId, token, json);
  return {};
}
#endif
void
RemotePlugin::Activate(
  WPEFramework::PluginHost::IShell *shell)
{
}

void
RemotePlugin::Deactivate()
{
}

bool
RemotePlugin::Attach(PluginHost::Channel &channel)
{
  LOGDBG("RemotePlugin::Attach %d\n", channel.Id());
  m_stream.SendAttach(channel.Id(), true);
  return true;
}

void
RemotePlugin::Detach(PluginHost::Channel &channel)
{
  LOGDBG("RemotePlugin::Detach %d\n", channel.Id());
  m_stream.SendAttach(channel.Id(), false);
}

WPEFramework::Core::ProxyType<WPEFramework::Core::JSON::IElement>
RemotePlugin::Inbound(const string &identifier)
{
  return WPEFramework::Core::ProxyType<WPEFramework::Core::JSON::IElement>(
    WPEFramework::PluginHost::IFactories::Instance().JSONRPC());
}

WPEFramework::Core::ProxyType<WPEFramework::Core::JSON::IElement>
RemotePlugin::Inbound(const uint32_t id,
    const Core::ProxyType<Core::JSON::IElement> &element)
{
  LOGDBG("NO IMPL %s\n", __PRETTY_FUNCTION__);
  return {};
}

void
RemotePlugin::AddRef() const
{
  // TODO
}

uint32_t
RemotePlugin::Release() const
{
  // TODO
  return 0;
}

string
RemotePlugin::Information() const
{
  return { };
}

void RemotePlugin::onRead(const Response& rsp)
{
    LOGDBG("RemotePlugin::onRead response: channel_id=%u, json=\"%s\"", rsp.channel_id, rsp.json.c_str());
    SendTo(rsp.channel_id, rsp.json.c_str());
}

int RemotePlugin::LaunchRemoteProcess(const string& rust_shared_lib, const string& host_ip, int port)
{
  string appName = "WPEHost";

  int pid = fork();

  if (pid == 0)
  {
    std::stringstream ssPort;
    ssPort << port;
    
    char const* argv[] = { 
      appName.c_str(),
      rust_shared_lib.c_str(),
      host_ip.c_str(), 
      ssPort.str().c_str(), 
      nullptr
    };


    if (!m_auth_token.empty())
      setenv("THUNDER_SECURITY_TOKEN", m_auth_token.c_str(), 1);

    if (execvp(appName.c_str(), (char**)argv) < 0)
    {
      LOGERR("Failed launch remote app %s: %s", appName.c_str(), strerror(errno));
      _exit(errno);
    }
  }
  else if (pid < 0)
  {
    LOGERR("Failed to fork process");
  }
  else
  {
    //TODO: verify child is running
  }

  return pid;
}

} } }
