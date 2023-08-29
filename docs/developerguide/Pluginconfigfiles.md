# 5. Plugin Config File

Create `FooPlugin.config` main configuration file.

This config file is used to specify autostart, startuporder, custom properties (passed to the service during activation via `PluginHost::IShell::ConfigLine()`). During the project configuration, write_config(FooPlugin) in `CMakeLists.txt` uses `FooPlugin.config` to generate and install a corresponding json file `FooPlugin.json` on the device (usually under /etc/WPEFramework/plugins).

Please note that the generated FooPlugin.json is packaged in the builds and installed on the device is different from the API Spec of the same name. The API spec only resides in the source repo and is not packaged/installed on the devices.

Example:
```C++
set (autostart false)
set (preconditions Platform)
set (callsign "org.rdk.FooPlugin")

map()
    kv(key ${PLUGIN_FOOPLUGIN_KEY})
    kv(maxsize 1000000)
    kv(maxvalue 1000)
end()
ans(configuration)
```

## Highlighting important sections from the configuation file:

- **autostart** - set this flag to false to indicate that plugin will not getting autostarted or activated by default when wpeframework is started. This will then get activated on demand by a resident app or another application. Most plugins should have autostart set to false. It should be set to true only if this plugin is truly needed on wpeframework startup.

- **preconditions** - Plugin can state dependencies that it needs for it to be activated and used. WPEFramework provides such dependencies through subsystems that can be implemented by core framework or by some other Plugin.

- **callsign** - this is the name that apps/clients would use to invoke APIs on the plugin. Best practice is to start names with `org.rdk.(PluginName)`. Plugin name should be camel cased starting with Upper Case.


