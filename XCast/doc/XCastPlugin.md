<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.XCast_Plugin"></a>
# XCast Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

org.rdk.Xcast plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.Xcast plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

This XCast plugin provides methods and events to support launching applications from an external source (for example, DIAL, Alexa, or WebPA). The RT implementation should use a RT service name that complies to the convention `com.comcast.<name>cast`. For example, `com.comcast.xdialcast` is used by `xdialserver`.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.Xcast*) |
| classname | string | Class name: *org.rdk.Xcast* |
| locator | string | Library name: *libWPEFrameworkXCast.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.Xcast plugin:

XCast interface methods:

| Method | Description |
| :-------- | :-------- |
| [setEnabled](#method.setEnabled) | Enables, or disables, the cast service |
| [getEnabled](#method.getEnabled) | Reports whether the cast service is enabled or disabled |
| [getApiVersionNumber](#method.getApiVersionNumber) | Gets the API version |
| [onApplicationStateChanged](#method.onApplicationStateChanged) | provides notification whenever an application changes state (due to user activity, an internal error, or other reasons) |


<a name="method.setEnabled"></a>
## *setEnabled <sup>method</sup>*

Enables, or disables, the cast service.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | `true` for enable or `false` for disable |

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
    "id": 1234567890,
    "method": "org.rdk.Xcast.1.setEnabled",
    "params": {
        "enabled": true
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.getEnabled"></a>
## *getEnabled <sup>method</sup>*

Reports whether the cast service is enabled or disabled.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.enabled | boolean | `true` for enable or `false` for disable |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Xcast.1.getEnabled"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "enabled": true,
        "success": true
    }
}
```

<a name="method.getApiVersionNumber"></a>
## *getApiVersionNumber <sup>method</sup>*

Gets the API version.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.version | integer | a version number |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Xcast.1.getApiVersionNumber"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "version": 1,
        "success": true
    }
}
```

<a name="method.onApplicationStateChanged"></a>
## *onApplicationStateChanged <sup>method</sup>*

provides notification whenever an application changes state (due to user activity, an internal error, or other reasons). For singleton applications, the `applicationId` parameter is optional. If an application request is denied, fails to fulfill, or the state change is triggered by an internal error, a predefined error string should be included. This error may be translated to an XCast client.  
 Client Error Mapping Example:  
| Error | Description | HTTP Status Codes |  
 | :-------- | :-------- | :-------- |   
| `none` | The request (start/stop) is fulfilled successfully | HTTP 200 OK |  
| `forbidden` | The user is not allowed to change the state of the application. This is not related to user account authentication of the native application | HTTP 403 Forbidden |  
| `unavailable` | The target native application is not available on the device | HTTP 404 Not Found |  
| `invalid` | The request is invalid (bad parameter for example) | HTTP 400 Bad Request |  
| `internal` | The server failed to fulfill the request (server error) | HTTP 500 Internal |.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.applicationName | string | Registered application name |
| params.state | string | A predefined application state. Either `running` or `stopped` |
| params?.applicationId | string | <sup>*(optional)*</sup> Application instance ID |
| params?.error | string | <sup>*(optional)*</sup> A predefined error from the cast target application. Either `none`, `forbidden` `unavailable` `invalid` or `internal` |

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
    "id": 1234567890,
    "method": "org.rdk.Xcast.1.onApplicationStateChanged",
    "params": {
        "applicationName": "NetflixApp",
        "state": "running",
        "applicationId": "1234",
        "error": ""
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.Xcast plugin:

XCast interface events:

| Event | Description |
| :-------- | :-------- |
| [onApplicationLaunchRequest](#event.onApplicationLaunchRequest) | Triggered when the cast service receives a launch request from a client |
| [onApplicationHideRequest](#event.onApplicationHideRequest) | Triggered when the cast service receives a hide request from a client |
| [onApplicationResumeRequest](#event.onApplicationResumeRequest) | Triggered when the cast service receives a resume request from a client |
| [onApplicationStopRequest](#event.onApplicationStopRequest) | Triggered when the cast service receives a stop request from a client |
| [onApplicationStateRequest](#event.onApplicationStateRequest) | Triggered when the cast service needs an update of the application state |


<a name="event.onApplicationLaunchRequest"></a>
## *onApplicationLaunchRequest <sup>event</sup>*

Triggered when the cast service receives a launch request from a client. This is a request to launch an application. The resident application can determine if the application should be launched based on the current context.  If the application is not already running, the requested application is started. If the application is already running and is in background mode, the requested application enters foreground mode (`optimus::running`, `xcast::running`). If the application is already in foreground mode, the request does not change the application state.  
Upon launching the application, the resident application is responsible for calling the `onApplicationStateChanged` method, which sends the notification back to the XCast client (for example, `Dial`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.applicationName | string | Registered application name |
| params.parameters | object | The format and interpretation is determined between the application launcher or cast target and the cast client for each application. For example, a Netflix DIAL-client returns a `pluginURL` parameter with the application launch string. A Youtube DIAL-client returns a `url` parameter with the application launch string |
| params.parameters.pluginUrl | string | Application launch string |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onApplicationLaunchRequest",
    "params": {
        "applicationName": "NetflixApp",
        "parameters": {
            "pluginUrl": "https://www.netflix.com"
        }
    }
}
```

<a name="event.onApplicationHideRequest"></a>
## *onApplicationHideRequest <sup>event</sup>*

Triggered when the cast service receives a hide request from a client. This is a request to hide an application from the foreground (suspend/run in background).  
 Upon hiding the application, the resident application is responsible for calling the `onApplicationStateChanged` method if hiding the application changes its running state.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.applicationName | string | Registered application name |
| params.applicationId | string | Application instance ID |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onApplicationHideRequest",
    "params": {
        "applicationName": "NetflixApp",
        "applicationId": "1234"
    }
}
```

<a name="event.onApplicationResumeRequest"></a>
## *onApplicationResumeRequest <sup>event</sup>*

Triggered when the cast service receives a resume request from a client. This is a request to resume an application.  
 Upon resuming the application, the resident application is responsible for calling the `onApplicationStateChanged` method.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.applicationName | string | Registered application name |
| params.applicationId | string | Application instance ID |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onApplicationResumeRequest",
    "params": {
        "applicationName": "NetflixApp",
        "applicationId": "1234"
    }
}
```

<a name="event.onApplicationStopRequest"></a>
## *onApplicationStopRequest <sup>event</sup>*

Triggered when the cast service receives a stop request from a client. This is a request to stop an application. If the application is already running and either in foreground or background mode, then the requested application is destroyed (`optimus::destroyed`, `xcast::stopped`). If the application is not running, this request triggers an error `onApplicationStateChanged` message with `Invalid`.  
 Upon stopping the application, the resident application is responsible for calling the `onApplicationStateChanged` method.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.applicationName | string | Registered application name |
| params.applicationId | string | Application instance ID |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onApplicationStopRequest",
    "params": {
        "applicationName": "NetflixApp",
        "applicationId": "1234"
    }
}
```

<a name="event.onApplicationStateRequest"></a>
## *onApplicationStateRequest <sup>event</sup>*

Triggered when the cast service needs an update of the application state.  
 The resident application is responsible for calling the `onApplicationStateChanged` method indicating the current state.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.applicationName | string | Registered application name |
| params.applicationId | string | Application instance ID |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onApplicationStateRequest",
    "params": {
        "applicationName": "NetflixApp",
        "applicationId": "1234"
    }
}
```

