# 6. Plugin metadata JSON file

Create `(PluginName)Plugin.json` file that provides various metadata about the plugin.

Example:
```JSON
{
    "$schema": "https://raw.githubusercontent.com/rdkcentral/rdkservices/main/Tools/json_generator/schemas/plugin.schema.json",
    "info": {
      "title": "FooPlugin Plugin",
      "callsign": "org.rdk.FooPlugin",
      "locator": "libWPEFrameworkFooPlugin.so",
      "status": "production",
      "description": "The `FooPlugin` plugin allows you to set a value and notify changes to the value"
    },
    "interface": {
      "$ref": "FooPlugin.json#"
    }
}
```

The `interface` refers to the JSON API Spec file that was defined in [Step 2](developerguide/PluginJSONfile.md).