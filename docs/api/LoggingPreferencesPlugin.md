<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.LoggingPreferencesPlugin"></a>
# LoggingPreferencesPlugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A org.rdk.LoggingPreferences plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Methods](#head.Methods)
- [Notifications](#head.Notifications)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the org.rdk.LoggingPreferences plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `LoggingPreferences` plugin allows you to control key press logging on a set-top box.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.LoggingPreferences*) |
| classname | string | Class name: *org.rdk.LoggingPreferences* |
| locator | string | Library name: *libWPEFrameworkLoggingPreferences.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.LoggingPreferences plugin:

LoggingPreferences interface methods:

| Method | Description |
| :-------- | :-------- |
| [isKeystrokeMaskEnabled](#method.isKeystrokeMaskEnabled) | Gets logging keystroke mask status (enabled or disabled) |
| [setKeystrokeMaskEnabled](#method.setKeystrokeMaskEnabled) | Sets the keystroke logging mask property |


<a name="method.isKeystrokeMaskEnabled"></a>
## *isKeystrokeMaskEnabled [<sup>method</sup>](#head.Methods)*

Gets logging keystroke mask status (enabled or disabled). 
 
### Events 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.keystrokeMaskEnabled | boolean | Whether keystroke mask is enabled (`true`) or disabled (`false`). If `true`, then any system that logs keystrokes must mask the actual keystroke being used. Default value is `false` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.LoggingPreferences.1.isKeystrokeMaskEnabled"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "keystrokeMaskEnabled": false,
        "success": true
    }
}
```

<a name="method.setKeystrokeMaskEnabled"></a>
## *setKeystrokeMaskEnabled [<sup>method</sup>](#head.Methods)*

Sets the keystroke logging mask property. If a keystroke mask is successfully changed, then this method triggers an `onKeystrokeMaskEnabledChange` event.
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onKeystrokeMaskEnabledChange`| Triggered if the keystroke mask is changed successfully |.

Also see: [onKeystrokeMaskEnabledChange](#event.onKeystrokeMaskEnabledChange)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.keystrokeMaskEnabled | boolean | Whether keystroke mask is enabled (`true`) or disabled (`false`). If `true`, then any system that logs keystrokes must mask the actual keystroke being used. Default value is `false` |

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
    "method": "org.rdk.LoggingPreferences.1.setKeystrokeMaskEnabled",
    "params": {
        "keystrokeMaskEnabled": false
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

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.LoggingPreferences plugin:

LoggingPreferences interface events:

| Event | Description |
| :-------- | :-------- |
| [onKeystrokeMaskEnabledChange](#event.onKeystrokeMaskEnabledChange) | Triggered when the keystroke mask is changed |


<a name="event.onKeystrokeMaskEnabledChange"></a>
## *onKeystrokeMaskEnabledChange [<sup>event</sup>](#head.Notifications)*

Triggered when the keystroke mask is changed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.keystrokeMaskEnabled | boolean | Whether keystroke mask is enabled (`true`) or disabled (`false`). If `true`, then any system that logs keystrokes must mask the actual keystroke being used. Default value is `false` |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onKeystrokeMaskEnabledChange",
    "params": {
        "keystrokeMaskEnabled": false
    }
}
```

