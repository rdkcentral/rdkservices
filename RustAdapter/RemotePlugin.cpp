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

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 55350

RemotePlugin::RemotePlugin()
  : m_service(nullptr)
{
}

const std::string
RemotePlugin::Initialize(PluginHost::IShell *shell)
{
  m_service = shell;

  std::stringstream shared_library_name;
  shared_library_name << "lib";
  for (char c : shell->Callsign())
    shared_library_name << static_cast<char>(std::tolower(c));
  shared_library_name << ".so";

  if (m_stream.Open(DEFAULT_HOST, DEFAULT_PORT, std::bind(&RemotePlugin::onRead, this, std::placeholders::_1)) < 0)
  {
    return string("RustAdapter RemotePlugin couldn't open socket stream");
  }

  if (m_stream.RunThread() < 0)
  {
    return string("RustAdapter RemotePlugin failed to run stream thread");
  }

  if ((m_remotePid = LaunchRemoteProcess(shared_library_name.str(), DEFAULT_HOST, DEFAULT_PORT)) < 0)
  {
    return string("RustAdapter RemotePlugin failed spawn remote process");
  }

  return {};
}

void
RemotePlugin::Deinitialize(PluginHost::IShell *shell)
{
  if (m_remotePid > 0)
  {
    int status;
    LOGDBG("Deinitialize: send exit message");
    m_stream.SendExit();

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

WPEFramework::Core::ProxyType<WPEFramework::Core::JSONRPC::Message>
RemotePlugin::Invoke(
  const WPEFramework::Core::JSONRPC::Context &ctx,
  const WPEFramework::Core::JSONRPC::Message &req)
{
  std::string json;
  req.ToString(json);
  m_stream.SendInvoke(ctx.ChannelId(), ctx.Token(), json);
  return {};
}

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
RemotePlugin::Inbound(const std::string &identifier)
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

std::string
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
  std::string appName = "rust_adapter_process";

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

    if(execvp(appName.c_str(), (char**)argv) < 0)
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