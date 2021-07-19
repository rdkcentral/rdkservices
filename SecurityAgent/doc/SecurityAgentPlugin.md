<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.SecurityAgent_Plugin"></a>
# SecurityAgent Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

SecurityAgent plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Methods](#head.Methods)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the SecurityAgent plugin. It includes detailed specification about its configuration and methods provided.

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

The `SecurityAgent` plugin is responsible for allowing or blocking access to the Thunder APIs.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *SecurityAgent*) |
| classname | string | Class name: *SecurityAgent* |
| locator | string | Library name: *libWPEFrameworkSecurityAgent.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |
| configuration | object | <sup>*(optional)*</sup>  |
| configuration?.acl | string | <sup>*(optional)*</sup> ACL |
| configuration?.connector | string | <sup>*(optional)*</sup> Connector |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the SecurityAgent plugin:

SecurityAgent interface methods:

| Method | Description |
| :-------- | :-------- |
| [createtoken](#method.createtoken) | Creates a Token |
| [validate](#method.validate) | Validates Token |


<a name="method.createtoken"></a>
## *createtoken <sup>method</sup>*

Creates a Token.

### Description

Create a signed JsonWeb token with the specified parameters

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.url | string | Url of application origin |
| params.user | string | The user name |
| params.hash | string | A random hash |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.token | string | Signed JsonWeb token |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | Token creation failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "SecurityAgent.1.createtoken",
    "params": {
        "url": "https://test.comcast.com",
        "user": "Test",
        "hash": "1CLYex47SY"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.ewogICAgImpzb25ycGMiOiAiMi4wIiwgCiAgICAiaWQiOiAxMjM0NTY3ODkwLCAKICAgICJtZXRob2QiOiAiQ29udHJvbGxlci4xLmFjdGl2YXRlIiwgCiAgICAicGFyYW1zIjogewogICAgICAgICJjYWxsc2lnbiI6ICJTZWN1cml0eUFnZW50IgogICAgfQp9.lL40nTwRyBvMwiglZhl5_rB8ycY1uhAJRFx9pGATMRQ"
    }
}
```

<a name="method.validate"></a>
## *validate <sup>method</sup>*

Validates Token.

### Description

Checks whether the token is valid and properly signed

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.token | string | Token to validate |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.valid | boolean | Whether the token is signature is correct |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "SecurityAgent.1.validate",
    "params": {
        "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.ewogICAgImpzb25ycGMiOiAiMi4wIiwgCiAgICAiaWQiOiAxMjM0NTY3ODkwLCAKICAgICJtZXRob2QiOiAiQ29udHJvbGxlci4xLmFjdGl2YXRlIiwgCiAgICAicGFyYW1zIjogewogICAgICAgICJjYWxsc2lnbiI6ICJTZWN1cml0eUFnZW50IgogICAgfQp9.lL40nTwRyBvMwiglZhl5_rB8ycY1uhAJRFx9pGATMRQ"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "valid": false
    }
}
```

