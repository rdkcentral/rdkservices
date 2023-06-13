# 4. Source Files

Create `FooPlugin.cpp` main source file.

Example:
```C++
#include "FooPlugin.h"
    
#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework {

namespace {

    static Plugin::Metadata<Plugin::FooPlugin> metadata(
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

SERVICE_REGISTRATION(FooPlugin, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

FooPlugin::FooPlugin()
    : PluginHost::JSONRPC(),
{
    RegisterAll();
}

FooPlugin::~FooPlugin()
{
    UnregisterAll();
}

const string FooPlugin::Initialize(PluginHost::IShell *service)
{
    string result;

    ASSERT(service != nullptr);

    string configLine = service->ConfigLine();
    _config.FromString(configLine);

    ASSERT(!_config.Path.Value().empty());

    Core::File file(_config.Path.Value());

    Core::Directory(file.PathName().c_str()).CreatePath();

    if (!file.Exists()) {
        for (auto i : LegacyLocations()) {
            Core::File from(i);

            if (from.Exists()) {
                if (!Utils::MoveFile(from.Name(), file.Name())) {
                    result = "move failed";
                }
                break;
            }
        }
    }

    if (result.empty()) {
        if (static_cast<SqliteStore *>(_store)->Open(
            _config.Path.Value(),
            _config.Key.Value(),
            _config.MaxSize.Value(),
            _config.MaxValue.Value()) != Core::ERROR_NONE) {
            result = "init failed";
        }
    }

    if (result.empty()) {
        _storeSink.Initialize(_store);
    }

    return result;
}

void FooPlugin::Deinitialize(PluginHost::IShell * /* service */)
{

}

string FooPlugin::Information() const
{
    return (string());
}

void PersistentStore::RegisterAll()
{
    Register<JsonObject, JsonObject>(_T("setValue"), &PersistentStore::endpoint_setValue, this);
}

void PersistentStore::UnregisterAll()
{
    Unregister(_T("setValue"));
}

uint32_t PersistentStore::endpoint_setValue(const JsonObject &parameters, JsonObject &response)
{
    LOGINFOMETHOD();

    bool success = false;

    if (!parameters.HasLabel("key") ||
        !parameters.HasLabel("value")) {
        response["error"] = "params missing";
    }
    else {
        string key = parameters["key"].String();
        string value = parameters["value"].String();

        if (key.empty()) {
            response["error"] = "params empty";
        }
        else {
            auto status = _store->SetValue(key, value);
            if (status == Core::ERROR_INVALID_INPUT_LENGTH) {
                response["error"] = "params too long";
            }
            success = (status == Core::ERROR_NONE);
        }
    }

    returnResponse(success);
}

void PersistentStore::event_onValueChanged(const string &key, const string &value)
{
    JsonObject params;
    params["key"] = key;
    params["value"] = value;

    sendNotify(_T("onValueChanged"), params);
}

} // namespace Plugin
} // namespace WPEFramework
```

## Highlighting important sections from the Source file:

- All Plugins support semantic versioning (Major.Minor.Patch) to track their changes. Refer to API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH macros defined in the plugin source file for semantic versioning. These should be updated when there are changes done to the plugin. For a new plugin that is ready for release, it should be set to 1.0.0.

- Refer to [RDK Services versioning](https://github.com/rdkcentral/RDKServices#versioning) for more details.

    ```C++
    #define API_VERSION_NUMBER_MAJOR 1
    #define API_VERSION_NUMBER_MINOR 0
    #define API_VERSION_NUMBER_PATCH 0

    namespace WPEFramework {

    namespace {

    static Plugin::Metadata<Plugin::FooPlugin> metadata(
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
    }
    ```

- The plugin is registered in a translation unit via a mandatory SERVICE_REGISTRATION   (MyService, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH).
    ```C++
    SERVICE_REGISTRATION(FooPlugin, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);
    ```

- JSON-RPC method registration - All plugins implement JSON-RPC interface. The APIs for a plugin are registered through Register() which gets called from the constructor. Register() is provided by the base [PluginHost::JSONRPC](https://github.com/rdkcentral/Thunder/blob/master/Source/plugins/JSONRPC.h#L305) interface. 

    ```C++ 
    Register<JsonObject, JsonObject>(_T("setValue"), &PersistentStore::endpoint_setValue, this);
    ```

- Initialization and Cleanup:
Keep Plugin Constructors & Destructors lean. Most initialization should be done within Initialize() and cleanup within DeInitialize(). 
    - If there is any error in initialization return non-empty string with useful error information from Initialize().This will ensure that plugin doesn't get activated and also return this error information to the caller.

    ```C++
    const string FooPlugin::Initialize(PluginHost::IShell *service)
    {
        string result;

        ASSERT(service != nullptr);

        string configLine = service->ConfigLine();
            
        // if there is error in initialization, return useful error information in result
        return result;
    }
    
    void FooPlugin::Deinitialize(PluginHost::IShell * /* service */)
    {
    //Any initialization in Initialize() should be cleaned up here in Deinitialize()
    }
    ```