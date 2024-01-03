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
    uint32_t id;
    std::function<void (uint32_t channel_id, const char *json_res)> send_to;
  };


} } }

namespace {
  std::atomic_int ctx_next_id = {1};
  std::vector< WPEFramework::Plugin::Rust::PluginContext * > ctx_array;

  // this function is passed into Rust as a pointer
  extern "C" void wpe_send_to(uint32_t channel_id, const char *json, uint32_t ctx_id)
  {
    for (WPEFramework::Plugin::Rust::PluginContext *ctx : ctx_array) {
      if (ctx->id == ctx_id) {
        ctx->send_to(channel_id, json);
        break;
      }
    }
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

WPEFramework::Plugin::Rust::LocalPlugin::LocalPlugin(const RustAdapter::Config &config)
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
  , m_config(config)
{
}

const string
WPEFramework::Plugin::Rust::LocalPlugin::Initialize(PluginHost::IShell *shell)
{
  m_service = shell;
  m_auth_token = RustAdapter::GetAuthToken(shell, shell->Callsign());

  std::string lib_name = RustAdapter::GetLibraryPathOrName(m_config.LibName.Value(), shell->Callsign());

  m_rust_plugin_lib = find_rust_plugin(lib_name);
  if (!m_rust_plugin_lib) {
    std::stringstream buff;
    buff << "cannot find ";
    buff << lib_name;
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

  Rust::PluginContext *plugin_ctx = new Rust::PluginContext();
  plugin_ctx->send_to = std::bind(&LocalPlugin::SendTo, this,
    std::placeholders::_1, std::placeholders::_2);
  plugin_ctx->id = ctx_next_id++;
  ctx_array.push_back(plugin_ctx);

  void *metadata = dlsym(m_rust_plugin_lib, "thunder_service_metadata");

  // create and initialize the rust plugin
  m_rust_plugin = m_rust_plugin_create(shell->ClassName().c_str(), &wpe_send_to, plugin_ctx->id,
    m_auth_token.c_str(),
    metadata);

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

#if JSON_RPC_CONTEXT

#if ((THUNDER_VERSION_MAJOR >= 4) && (THUNDER_VERSION_MINOR == 4))
WPEFramework::Core::hresult
  WPEFramework::Plugin::Rust::LocalPlugin::Invoke(ICallback* callback, const uint32_t channelId, const uint32_t id, const string& token, const string& method, const string& parameters, string& response)
{
  Rust::RequestContext req_ctx;
  req_ctx.channel_id = channelId;
  req_ctx.auth_token = token.c_str();

  m_rust_plugin_invoke(m_rust_plugin, response.c_str(), req_ctx);

  // indicates to Thunder that this request is being processed asynchronously
  return {};
}
#else
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
#endif

#else
WPEFramework::Core::ProxyType<WPEFramework::Core::JSONRPC::Message>
  WPEFramework::Plugin::Rust::LocalPlugin::Invoke(
    const string& token, const uint32_t channelId, const Core::JSONRPC::Message& req)
{
  Rust::RequestContext req_ctx;
  req_ctx.channel_id = channelId;
  req_ctx.auth_token = token.c_str();

  m_rust_plugin_invoke(m_rust_plugin, to_string(req).c_str(), req_ctx);

  // indicates to Thunder that this request is being processed asynchronously
  return {};
}
#endif

#if ((THUNDER_VERSION_MAJOR == 4) && (THUNDER_VERSION_MINOR == 4))
WPEFramework::Core::hresult WPEFramework::Plugin::Rust::LocalPlugin::Revoke(ICallback* callback)
{
    return {};
}

WPEFramework::Core::hresult WPEFramework::Plugin::Rust::LocalPlugin::Validate(const string& token, const string& method, const string& paramaters) const
{
  return {};
}
#endif


#if ((THUNDER_VERSION_MAJOR == 2) || ((THUNDER_VERSION_MAJOR == 4) && (THUNDER_VERSION_MINOR == 2)))
void
WPEFramework::Plugin::Rust::LocalPlugin::Activate(
  WPEFramework::PluginHost::IShell *shell)
{
}

void
WPEFramework::Plugin::Rust::LocalPlugin::Deactivate()
{
}
#endif
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

#if ((THUNDER_VERSION_MAJOR >= 4) && (THUNDER_VERSION_MINOR == 2))
void WPEFramework::Plugin::Rust::LocalPlugin::Close(const uint32_t channelId) /* override */
{
  return;
}
#endif
