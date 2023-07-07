# 3. Headers Files

## a) Create a `Module.h` header file

- The file `Module.h` defines the mandatory `MODULE_NAME`. Thunder provides a trace and warning reporting feature. To accurately identify the source of a warning, Thunder needs to know the human-readable name of the package (executable or library). This package name is defined by the `MODULE_NAME` and declared by the `MODULE_NAME_DECLARATION()`.

- It also includes wpeframework headers "<plugins/plugins.h>".

    Example:
    ```C
    #pragma once
    #ifndef MODULE_NAME
    #define MODULE_NAME Plugin_FooPlugin
    #endif

    #include <plugins/plugins.h>
    #include <tracing/tracing.h>

    #undef EXTERNAL
    #define EXTERNAL
    ```

## b) Create a `Module.cpp` file

- This file defines the mandatory `MODULE_NAME_DECLARATION` as mentioned below.
    ```C++
    #include "Module.h"
    MODULE_NAME_DECLARATION(BUILD_REFERENCE)
    ```

## c) Create `FooPlugin.h` main header file

- This main header file declares the Plugin implementation class.

    Example:
    ```C++
    #pragma once

    #include "Module.h"

    namespace WPEFramework
    {
        namespace Plugin
        {
            class FooPlugin : public PluginHost::IPlugin, public PluginHost::JSONRPC
            {
            private:
                FooPlugin(const FooPlugin &) = delete;
                FooPlugin &operator=(const FooPlugin &) = delete;

            public:
                FooPlugin();
                virtual ~FooPlugin();

                // Build QueryInterface implementation, specifying all possible interfaces to be returned.
                BEGIN_INTERFACE_MAP(FooPlugin)
                INTERFACE_ENTRY(PluginHost::IPlugin)
                INTERFACE_ENTRY(PluginHost::IDispatcher)
                END_INTERFACE_MAP

            public:
                //   IPlugin methods
                // -------------------------------------------------------------------------------------------------------
                virtual const string Initialize(PluginHost::IShell *service) override;
                virtual void Deinitialize(PluginHost::IShell *service) override;
            virtual string Information() const override;

            protected:
                void RegisterAll();
                void UnregisterAll();

                uint32_t endpoint_setValue(const JsonObject &parameters, JsonObject &response);

                void event_onValueChanged(const string &key, const string &value);
            };

        } // namespace Plugin
    } // namespace WPEFramework
    ```

## Highlighting important sections from the Header file:

- RDK services are implemented as Thunder Plugins and must adhere to the `PluginHost::IPlugin` interface. If it activates/deactivates and handles JSON-RPC it implements `PluginHost::IDispatcher` (or derives from `PluginHost::JSONRPC`).

    ```C++
    class FooPlugin : public PluginHost::IPlugin, public PluginHost::JSONRPC
    ```

- A plugin specifies the interfaces that it implements within the `BEGIN_INTERFACE_MAP/END_INTERFACE_MAP` macros. These are basically macros for QueryInterface defined in [WPEFramework](https://github.com/rdkcentral/Thunder/blob/master/Source/core/Services.h#L362).

    ```C++
    BEGIN_INTERFACE_MAP(FooPlugin)
    INTERFACE_ENTRY(PluginHost::IPlugin)
    INTERFACE_ENTRY(PluginHost::IDispatcher)
    END_INTERFACE_MAP
    ```

- Each plugin should override the `Initialize() & Deinitialize()` APIs that are provided by WPEFramework for plugin `Initialization & Clean up` respectively. `Initialize()` is called when a plugin gets activated and `Deinitialize()` when the plugin gets deactivated. Plugins should handle most of their initialization & cleanup within these APIs instead of the constructor or destructor. This is because most plugins are not autostarted or activated on bootup. They are only started on demand by an application. Thus it makes sense to have the plugin initialization logic within Initialize() which will get called when the plugin is getting activated to be used. This will also ensure that WPEFramework bootups up faster since the constructors would be lean and run faster.

    ```C++
    virtual const string Initialize(PluginHost::IShell *service) override;
    virtual void Deinitialize(PluginHost::IShell *service) override;
    ```

- Declare the API and event handlers.
    ```C++
    uint32_t endpoint_setValue(const JsonObject &parameters, JsonObject &response);
    void event_onValueChanged(const string &key, const string &value);
    ```



