<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.DeviceDiagnostics_Plugin"></a>
# DeviceDiagnostics Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

org.rdk.DeviceDiagnostics plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Methods](#head.Methods)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the org.rdk.DeviceDiagnostics plugin. It includes detailed specification about its configuration and methods provided.

<a name="head.Case_Sensitivity"></a>
## Case Sensitivity

All identifiers of the interfaces described in this document are case-sensitive. Thus, unless stated otherwise, all keywords, entities, properties, relations and actions should be treated as such.

<a name="head.Acronyms,_Abbreviations_and_Terms"></a>
## Acronyms, Abbreviations and Terms

The table below provides and overview of acronyms used in this document and their definitions.

| Acronym | Description |
| :-------- | :-------- |
| <a name="acronym.API">API</a> | Application Programming Interface |
| <a name="acronym.HTTP">HTTP</a> | Hypertext Transfer Protocol |
| <a name="acronym.JSON">JSON</a> | JavaScript Object Notation; a data interchange format |
| <a name="acronym.JSON-RPC">JSON-RPC</a> | A remote procedure call protocol encoded in JSON |

The table below provides and overview of terms and abbreviations used in this document and their definitions.

| Term | Description |
| :-------- | :-------- |
| <a name="term.callsign">callsign</a> | The name given to an instance of a plugin. One plugin can be instantiated multiple times, but each instance the instance name, callsign, must be unique. |

<a name="head.References"></a>
## References

| Ref ID | Description |
| :-------- | :-------- |
| <a name="ref.HTTP">[HTTP](http://www.w3.org/Protocols)</a> | HTTP specification |
| <a name="ref.JSON-RPC">[JSON-RPC](https://www.jsonrpc.org/specification)</a> | JSON-RPC 2.0 specification |
| <a name="ref.JSON">[JSON](http://www.json.org/)</a> | JSON specification |
| <a name="ref.Thunder">[Thunder](https://github.com/WebPlatformForEmbedded/Thunder/blob/master/doc/WPE%20-%20API%20-%20WPEFramework.docx)</a> | Thunder API Reference |

<a name="head.Description"></a>
# Description

The DeviceDiagnostics plugin provides an interface for the WebPA service for this device only. It allows you to get device configurations.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.DeviceDiagnostics*) |
| classname | string | Class name: *org.rdk.DeviceDiagnostics* |
| locator | string | Library name: *libWPEFrameworkDeviceDiagnostics.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.DeviceDiagnostics plugin:

DeviceDiagnostics interface methods:

| Method | Description |
| :-------- | :-------- |
| [getConfiguration](#method.getConfiguration) | Gets the values associated with the corresponding property names |


<a name="method.getConfiguration"></a>
## *getConfiguration <sup>method</sup>*

Gets the values associated with the corresponding property names.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.names | array | String array of property names |
| params.names[#] | string |  |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.paramList | array | An array of JSON objects with the specified properties and their values |
| result.paramList[#] | object |  |
| result.paramList[#].name | string | The property name |
| result.paramList[#].value | string | The property value |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.DeviceDiagnostics.1.getConfiguration",
    "params": {
        "names": [
            "Device.X_CISCO_COM_LED.RedPwm"
        ]
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "paramList": [
            {
                "name": "Device.X_CISCO_COM_LED.RedPwm",
                "value": "123"
            }
        ],
        "success": true
    }
}
```

