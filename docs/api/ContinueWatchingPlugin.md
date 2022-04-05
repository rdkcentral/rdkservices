<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.ContinueWatching_Plugin"></a>
# ContinueWatching Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A org.rdk.ContinueWatching plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Methods](#head.Methods)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the org.rdk.ContinueWatching plugin. It includes detailed specification about its configuration and methods provided.

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

Initial version of the `ContinueWatching` API.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.ContinueWatching*) |
| classname | string | Class name: *org.rdk.ContinueWatching* |
| locator | string | Library name: *libWPEFrameworkContinueWatching.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.ContinueWatching plugin:

ContinueWatching interface methods:

| Method | Description |
| :-------- | :-------- |
| [deleteApplicationToken](#method.deleteApplicationToken) | Deletes the stored tokens for a particular application |
| [getApplicationToken](#method.getApplicationToken) | Retrieves the tokens for a particular application |
| [setApplicationToken](#method.setApplicationToken) | Sets the given tokens for an application |


<a name="method.deleteApplicationToken"></a>
## *deleteApplicationToken [<sup>method</sup>](#head.Methods)*

Deletes the stored tokens for a particular application.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.applicationName | string | The application name |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.ContinueWatching.1.deleteApplicationToken",
    "params": {
        "applicationName": "netflix"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.getApplicationToken"></a>
## *getApplicationToken [<sup>method</sup>](#head.Methods)*

Retrieves the tokens for a particular application.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.applicationName | string | The application name |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.application_token | array | A string [] containing tokens. If no tokens are set for the application, then an empty array returns |
| result.application_token[#] | object |  |
| result.application_token[#]?.ABC | string | <sup>*(optional)*</sup> A token value |
| result.application_token[#]?.DEF | string | <sup>*(optional)*</sup> A token value |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.ContinueWatching.1.getApplicationToken",
    "params": {
        "applicationName": "netflix"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "application_token": [
            {
                "ABC": "xyz",
                "DEF": "uvw"
            }
        ],
        "success": true
    }
}
```

<a name="method.setApplicationToken"></a>
## *setApplicationToken [<sup>method</sup>](#head.Methods)*

Sets the given tokens for an application.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.applicationName | string | The application name |
| params.application_token | object | Any number of tokens to be set |
| params.application_token?.ABC | string | <sup>*(optional)*</sup> A token value |
| params.application_token?.DEF | string | <sup>*(optional)*</sup> A token value |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.ContinueWatching.1.setApplicationToken",
    "params": {
        "applicationName": "netflix",
        "application_token": {
            "ABC": "xyz",
            "DEF": "uvw"
        }
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

