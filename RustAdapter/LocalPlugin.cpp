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
#include "Logger.h"
#include <plugins/Channel.h>
#include <sstream>

namespace WPEFramework {
namespace Plugin {
namespace Rust {
  struct RequestContext {
    uint32_t    channel_id;
    const char *auth_token;
  };

  struct PluginContext {
    std::function<void (uint32_t channel_id, const char *json_res)> send_to;
  };
} } }

namespace {
  // this function is passed into Rust as a pointer
  extern "C" void wpe_send_to(uint32_t channel_id, const char *json, WPEFramework::Plugin::Rust::PluginContext *p_ctx)
  {
    p_ctx->send_to(channel_id, json);
  }

  std::vector<string> get_library_search_paths()
  {
    std::vector<string> paths  = {};

    char *ld_paths = getenv("LD_LIBRARY_PATH");
    if (ld_paths) {
      ld_paths = strdup(ld_paths);

      char *p = nullptr;
      char *saveptr = nullptr;
      while ((p = strtok_r(ld_paths, ":", &saveptr)) != nullptr) {
        paths.push_back(string(p));
        ld_paths = nullptr;
      }
      free(ld_paths);
    }
    return paths;
  }

  inline string to_string(const WPEFramework::Core::JSONRPC::Message &m)
  {
    string s;
    m.ToString(s);
    return s;
  }

  bool file_exists(const string &path)
  {
    struct stat statbuf = {};
    return stat(path.c_str(), &statbuf) != -1;
  }

  void *find_rust_plugin(const string &fname)
  {
    void *lib = nullptr;
    if (file_exists(fname)) {
      LOGDBG("Loading library from:%s\n", fname.c_str());
      lib = dlopen(fname.c_str(), RTLD_LAZY);
      if (!lib)
        LOGERR("Failed to load %s. %s", fname.c_str(), dlerror());
    }
    else {
      //MARKR is this needed because dlopen docs says it searches LD_LIBRARY_PATH paths by default
      for (const string &path : get_library_search_paths()) {
        string full_path = path + "/" + fname;
        if (file_exists(full_path)) {
          LOGDBG("Loading library from:%s\n", full_path.c_str());
          lib = dlopen(full_path.c_str(), RTLD_LAZY);
          if (!lib)
            LOGERR("Failed to load %s. %s", full_path.c_str(), dlerror());
          break;
        }
      }
    }
    return lib;
  }
}

WPEFramework::Plugin::Rust::LocalPlugin::LocalPlugin()
  : m_rust_plugin_create(nullptr)
  , m_rust_plugin_destroy(nullptr)
  , m_rust_plugin_init(nullptr)
  , m_rust_plugin_invoke(nullptr)
  , m_rust_plugin_on_client_connect(nullptr)
  , m_rust_plugin_on_client_disconnect(nullptr)
  , m_refcount(1)
  , m_rust_plugin(nullptr)
  , m_rust_plugin_lib(nullptr)
  , m_service(nullptr)
{
}

const string
WPEFramework::Plugin::Rust::LocalPlugin::Initialize(PluginHost::IShell *shell)
{
  m_service = shell;

  std::stringstream shared_library_name;
  shared_library_name << "lib";
  for (char c : shell->Callsign())
    shared_library_name << static_cast<char>(std::tolower(c));
  shared_library_name << ".so";

  m_rust_plugin_lib = find_rust_plugin(shared_library_name.str());
  if (!m_rust_plugin_lib) {
    std::stringstream buff;
    buff << "cannot find ";
    buff << shared_library_name.str();
    return buff.str();
  }

  //
  // TODO: we should compile libWPEFrameworkRust.so into libWPEFrameworkRust.a
  // and link it with libthunder_rs.so. In that case, we probably won't have to 
  // dynamically bind all the functions
  //
  m_rust_plugin_create = (RustPlugin_Create) dlsym(m_rust_plugin_lib, "wpe_rust_plugin_create");
  m_rust_plugin_init = (RustPlugin_Init) dlsym(m_rust_plugin_lib, "wpe_rust_plugin_init");
  m_rust_plugin_destroy = (RustPlugin_Destroy) dlsym(m_rust_plugin_lib, "wpe_rust_plugin_destroy");
  m_rust_plugin_invoke = (RustPlugin_Invoke) dlsym(m_rust_plugin_lib, "wpe_rust_plugin_invoke");
  m_rust_plugin_on_client_connect = (RustPlugin_OnClientConnect)
    dlsym(m_rust_plugin_lib, "wpe_rust_plugin_on_client_connect");
  m_rust_plugin_on_client_disconnect = (RustPlugin_OnClientDisconnect)
    dlsym(m_rust_plugin_lib, "wpe_rust_plugin_on_client_disconnect");

  Rust::PluginContext *m_plugin_ctx = new Rust::PluginContext();
  m_plugin_ctx->send_to = std::bind(&LocalPlugin::SendTo, this,
    std::placeholders::_1, std::placeholders::_2);

  void *metadata = dlsym(m_rust_plugin_lib, "thunder_service_metadata");

  // create and initialize the rust plugin
  m_rust_plugin = m_rust_plugin_create(shell->ClassName().c_str(), &wpe_send_to, m_plugin_ctx, metadata);

  // XXX: The call to "init" doesn't seem necessary
  m_rust_plugin_init(m_rust_plugin, nullptr);

  return {};
}

void
WPEFramework::Plugin::Rust::LocalPlugin::Deinitialize(PluginHost::IShell *shell)
{
  if (m_rust_plugin) {
    m_rust_plugin_destroy(m_rust_plugin);
    m_rust_plugin = nullptr;
  }
}

void
WPEFramework::Plugin::Rust::LocalPlugin::SendTo(uint32_t channel_id, const char *json)
{
  auto res = Core::ProxyType<Web::JSONBodyType<Core::JSONRPC::Message>>::Create();
  res->FromString(json);
  m_service->Submit(channel_id, Core::ProxyType<Core::JSON::IElement>(res));
}

WPEFramework::Core::ProxyType<WPEFramework::Core::JSONRPC::Message>
WPEFramework::Plugin::Rust::LocalPlugin::Invoke(
  const WPEFramework::Core::JSONRPC::Context &ctx,
  const WPEFramework::Core::JSONRPC::Message &req)
{
  Rust::RequestContext req_ctx;
  req_ctx.channel_id = ctx.ChannelId();
  req_ctx.auth_token = ctx.Token().c_str();

  m_rust_plugin_invoke(m_rust_plugin, to_string(req).c_str(), req_ctx);

  // indicates to Thunder that this request is being processed asynchronously
  return {};
}

void
WPEFramework::Plugin::Rust::LocalPlugin::Activate(
  WPEFramework::PluginHost::IShell *shell)
{
}

void
WPEFramework::Plugin::Rust::LocalPlugin::Deactivate()
{
}

bool
WPEFramework::Plugin::Rust::LocalPlugin::Attach(PluginHost::Channel &channel)
{
  m_rust_plugin_on_client_connect(m_rust_plugin, channel.Id());
  return true;
}

void
WPEFramework::Plugin::Rust::LocalPlugin::Detach(PluginHost::Channel &channel)
{
  m_rust_plugin_on_client_disconnect(m_rust_plugin, channel.Id());
}

WPEFramework::Core::ProxyType<WPEFramework::Core::JSON::IElement>
WPEFramework::Plugin::Rust::LocalPlugin::Inbound(const string &identifier)
{
  return WPEFramework::Core::ProxyType<WPEFramework::Core::JSON::IElement>(
    WPEFramework::PluginHost::IFactories::Instance().JSONRPC());
}

WPEFramework::Core::ProxyType<WPEFramework::Core::JSON::IElement>
WPEFramework::Plugin::Rust::LocalPlugin::Inbound(const uint32_t id,
    const Core::ProxyType<Core::JSON::IElement> &element)
{
  Core::ProxyType<Core::JSONRPC::Message> req(element);

  Rust::RequestContext req_ctx;
  req_ctx.channel_id = id;
  req_ctx.auth_token = "";

  m_rust_plugin_invoke(m_rust_plugin, to_string(*req).c_str(), req_ctx);

  // I don't know what this return value is used for, but Invoke() returns {} to
  // indicate that the request is being processed asynchronously
  return {};
}

void
WPEFramework::Plugin::Rust::LocalPlugin::AddRef() const
{
  // TODO
}

uint32_t
WPEFramework::Plugin::Rust::LocalPlugin::Release() const
{
  // TODO
  return 0;
}

string
WPEFramework::Plugin::Rust::LocalPlugin::Information() const
{
  return { };
}
