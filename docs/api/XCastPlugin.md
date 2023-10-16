<!-- Generated automatically, DO NOT EDIT! -->
<a name="XCast_Plugin"></a>
# XCast Plugin

**Version: [1.0.17](https://github.com/rdkcentral/rdkservices/blob/main/XCast/CHANGELOG.md)**

A org.rdk.Xcast plugin for Thunder framework.

### Table of Contents

- [Abbreviation, Acronyms and Terms](#Abbreviation,_Acronyms_and_Terms)
- [Description](#Description)
- [Configuration](#Configuration)
- [Methods](#Methods)
- [Notifications](#Notifications)

<a name="Abbreviation,_Acronyms_and_Terms"></a>
# Abbreviation, Acronyms and Terms

[[Refer to this link](userguide/aat.md)]

<a name="Description"></a>
# Description

This XCast plugin provides methods and events to support launching applications from an external source (for example, DIAL, Alexa, or WebPA). The RT implementation should use a RT service name that complies to the convention `com.comcast.<name>cast`. For example, `com.comcast.xdialcast` is used by `xdialserver`.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.Xcast*) |
| classname | string | Class name: *org.rdk.Xcast* |
| locator | string | Library name: *libWPEFrameworkXCast.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.Xcast plugin:

XCast interface methods:

| Method | Description |
| :-------- | :-------- |
| [getApiVersionNumber](#getApiVersionNumber) | Gets the API version number |
| [getEnabled](#getEnabled) | Reports whether xcast plugin is enabled or disabled |
| [getFriendlyName](#getFriendlyName) | Returns the friendly name set by setFriendlyName API |
| [getProtocolVersion](#getProtocolVersion) | Returns the DIAL protocol version supported by the server |
| [getStandbyBehavior](#getStandbyBehavior) | Return current standby behavior option string set uisng setStandbyBehavior or default value  |
| [onApplicationStateChanged](#onApplicationStateChanged) | Provides notification whenever an application changes state due to user activity, an internal error, or other reasons |
| [registerApplications](#registerApplications) | Registers an application |
| [unregisterApplications](#unregisterApplications) | Unregisters an application |
| [setEnabled](#setEnabled) | Enable or disable XCAST service |
| [setFriendlyName](#setFriendlyName) | Sets the friendly name of device |
| [setStandbyBehavior](#setStandbyBehavior) | Sets the expected xcast behavior in standby mode |


<a name="getApiVersionNumber"></a>
## *getApiVersionNumber*

Gets the API version number.

### Events

No Events

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
    "id": 42,
    "method": "org.rdk.Xcast.getApiVersionNumber"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "version": 1,
        "success": true
    }
}
```

<a name="getEnabled"></a>
## *getEnabled*

Reports whether xcast plugin is enabled or disabled.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.enabled | boolean | `true` for enabled or `false` for disabled |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Xcast.getEnabled"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "enabled": true,
        "success": true
    }
}
```

<a name="getFriendlyName"></a>
## *getFriendlyName*

Returns the friendly name set by setFriendlyName API.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api]( https://rdkcentral.github.io/rdkservices/#/api/SystemPlugin?id=getFriendlyName)

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.friendlyname | string | The friendly name of the device which used to display on the client device list |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Xcast.getFriendlyName"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "friendlyname": "Manufacturer name",
        "success": true
    }
}
```

<a name="getProtocolVersion"></a>
## *getProtocolVersion*

Returns the DIAL protocol version supported by the server.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.version | string | DIAL protocol version |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Xcast.getProtocolVersion"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "version": "2.2.1",
        "success": true
    }
}
```

<a name="getStandbyBehavior"></a>
## *getStandbyBehavior*

Return current standby behavior option string set uisng setStandbyBehavior or default value .

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.standbybehavior | string | whether to remain active or inactive during standby mode (must be one of the following: *active*, *inactive*) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Xcast.getStandbyBehavior"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "standbybehavior": "active",
        "success": true
    }
}
```

<a name="onApplicationStateChanged"></a>
## *onApplicationStateChanged*

Provides notification whenever an application changes state due to user activity, an internal error, or other reasons. For singleton applications, the `applicationId` parameter is optional. If an application request is denied, fails to fulfill, or the state change is triggered by an internal error, then a predefined error string should be included. This error may be translated to an XCast client.  

The following table provides a client error mapping example: 

| Error | Description | HTTP Status Codes |  
| :-------- | :-------- | :-------- |   
| `none` | The request (start/stop) is fulfilled successfully | HTTP 200 OK |  
| `forbidden` | The user is not allowed to change the state of the application. This is not related to user account authentication of the native application | HTTP 403 Forbidden |  
| `unavailable` | The target native application is not available on the device | HTTP 404 Not Found |  
| `invalid` | The request is invalid (bad parameter for example) | HTTP 400 Bad Request |  
| `internal` | The server failed to fulfill the request (server error) | HTTP 500 Internal |.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.applicationName | string | Registered application name |
| params.state | string | A predefined application state. Either `running`, `stopped`, `hidden`, or `suspended` (introduced in DIAL 2.1, `suspended` is a synonym to `hidden`) |
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
    "id": 42,
    "method": "org.rdk.Xcast.onApplicationStateChanged",
    "params": {
        "applicationName": "NetflixApp",
        "state": "running",
        "applicationId": "1234",
        "error": "..."
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

<a name="registerApplications"></a>
## *registerApplications*

Registers an application. This allows to whitelist the apps which support dial service. To dynamically update the app list, same API should be called with the updated list. so that app list will be appended to the existing XCast white list. If a DIAL request for an unregistered application is received by DIAL server, the request will be denied (HTTP 404) per DIAL specification. Optional fields need not be included, or can be included with empty values.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.applications | array | Json array with one or more application details to register |
| params.applications[#] | object |  |
| params.applications[#].names | array | case-sensitive. Group of acceptable names for a related application. Application name in request URI must have exact match to one of the names. Otherwise, matching prefix is needed. If the application name in request URI does not match any names or prefixes, then the request shall fail |
| params.applications[#].names[#] | string |  |
| params.applications[#]?.prefixes | array | <sup>*(optional)*</sup> If the application name in request URI does not match the list of names, it must contain one of the prefixes.If the application name in request URI does not match any names or prefixes, then the request shall fail |
| params.applications[#]?.prefixes[#] | string | <sup>*(optional)*</sup>  |
| params.applications[#]?.cors | array | <sup>*(optional)*</sup> a set of origins allowed for the application. This must not be empty |
| params.applications[#]?.cors[#] | string | <sup>*(optional)*</sup>  |
| params.applications[#]?.properties | object | <sup>*(optional)*</sup> specific application properties applicable to app management. If not present in descriptor, the default value is assumed |
| params.applications[#]?.properties.allowStop | boolean | is the application (matching name list or prefix list) allowed to stop (no PID presence) after launched |
| params.applications[#]?.launchParameters | object | <sup>*(optional)*</sup> launchParameters that application wants dial-server to append before sending the request to launch application |
| params.applications[#]?.launchParameters.query | string | query string that need to be appended in launch request |
| params.applications[#]?.launchParameters.payload | string | optional payload string that need to be appended in launch request |

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
    "method": "org.rdk.Xcast.registerApplications",
    "params": {
        "applications": [
            {
                "names": [
                    "Youtube"
                ],
                "prefixes": [
                    "myYouTube"
                ],
                "cors": [
                    ".youtube.com"
                ],
                "properties": {
                    "allowStop": true
                },
                "launchParameters": {
                    "query": "source_type=12",
                    "payload": "..."
                }
            }
        ]
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

<a name="unregisterApplications"></a>
## *unregisterApplications*

Unregisters an application. This API allows to remove the specified applist from the XCast whitelist. To dynamically delete the specific app list, same API should be called with the app list to remove. so that mentioned app list will be removed from the XCast whitelist. Calling this API with empty list will clear the Xcast Whitelist.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.applications | string | One or more application name to unregister |

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
    "method": "org.rdk.Xcast.unregisterApplications",
    "params": {
        "applications": "['YouTube', 'Netflix']"
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

<a name="setEnabled"></a>
## *setEnabled*

Enable or disable XCAST service.  When disabled, the customer should not be able to discover CPE as a cast target for any client application. After enable(true) server application manger must re-register all app that are available for user to cast. The enabled status is not persisted on device after each reboot or reconnect application should call setEnalbed to initialize XCast thunder plugin.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | `true` for enabled or `false` for disabled |

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
    "method": "org.rdk.Xcast.setEnabled",
    "params": {
        "enabled": true
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

<a name="setFriendlyName"></a>
## *setFriendlyName*

Sets the friendly name of device. It allows an application to override the default friendly name value with the friendly name passed as an argument. The provided name  should not be empty. If a user provided name is not available, the caller of the API should use the default name partnerId+Model  (or any other agreed default name) as the parameter. After initialization, XCastService should not be activated until setFriendlyName() is invoked. Frinedly name is not persisted on device after each reboot or reconnect application should call API to update the friendlyName.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api]( https://rdkcentral.github.io/rdkservices/#/api/SystemPlugin?id=setFriendlyName)

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.friendlyname | string | The friendly name of the device which used to display on the client device list |

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
    "method": "org.rdk.Xcast.setFriendlyName",
    "params": {
        "friendlyname": "Manufacturer name"
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

<a name="setStandbyBehavior"></a>
## *setStandbyBehavior*

Sets the expected xcast behavior in standby mode. It allows an application to override controls on xcast behavior in standby mode. The default behavior in STANDBY mode is inactive, so client device can not discover the server. When STANDBY behavior is active, client device can discover the CPE device and perform the launch operation. Upon reeiving the launch request device will transitioned from STANDBY to ON mode.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.standbybehavior | string | whether to remain active or inactive during standby mode (must be one of the following: *active*, *inactive*) |

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
    "method": "org.rdk.Xcast.setStandbyBehavior",
    "params": {
        "standbybehavior": "active"
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

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.Xcast plugin:

XCast interface events:

| Event | Description |
| :-------- | :-------- |
| [onApplicationHideRequest](#onApplicationHideRequest) | Triggered when the cast service receives a hide request from a client |
| [onApplicationLaunchRequest](#onApplicationLaunchRequest) | Triggered when the cast service receives a launch request from a client |
| [onApplicationResumeRequest](#onApplicationResumeRequest) | Triggered when the cast service receives a resume request from a client |
| [onApplicationStateRequest](#onApplicationStateRequest) | Triggered when the cast service needs an update of the application state |
| [onApplicationStopRequest](#onApplicationStopRequest) | Triggered when the cast service receives a stop request from a client |


<a name="onApplicationHideRequest"></a>
## *onApplicationHideRequest*

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
    "method": "client.events.onApplicationHideRequest",
    "params": {
        "applicationName": "NetflixApp",
        "applicationId": "1234"
    }
}
```

<a name="onApplicationLaunchRequest"></a>
## *onApplicationLaunchRequest*

Triggered when the cast service receives a launch request from a client. This is a request to launch an application. The resident application can determine if the application should be launched based on the current context. If the application is not already running, the requested application is started. If the application is already running and is in background mode, the requested application enters foreground mode (`optimus::running`, `xcast::running`). If the application is already in foreground mode, the request does not change the application state.  
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
    "method": "client.events.onApplicationLaunchRequest",
    "params": {
        "applicationName": "NetflixApp",
        "parameters": {
            "pluginUrl": "https://www.netflix.com"
        }
    }
}
```

<a name="onApplicationResumeRequest"></a>
## *onApplicationResumeRequest*

Triggered when the cast service receives a resume request from a client. This is a request to resume an application.  
Upon resuming the application, the resident application is responsible for calling the `onApplicationStateChanged` 

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
    "method": "client.events.onApplicationResumeRequest",
    "params": {
        "applicationName": "NetflixApp",
        "applicationId": "1234"
    }
}
```

<a name="onApplicationStateRequest"></a>
## *onApplicationStateRequest*

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
    "method": "client.events.onApplicationStateRequest",
    "params": {
        "applicationName": "NetflixApp",
        "applicationId": "1234"
    }
}
```

<a name="onApplicationStopRequest"></a>
## *onApplicationStopRequest*

Triggered when the cast service receives a stop request from a client. This is a request to stop an application. If the application is already running and either in foreground or background mode, then the requested application is destroyed (`optimus::destroyed`, `xcast::stopped`). If the application is not running, this request triggers an error `onApplicationStateChanged` message with `Invalid`.  
Upon stopping the application, the resident application is responsible for calling the `onApplicationStateChanged` 

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
    "method": "client.events.onApplicationStopRequest",
    "params": {
        "applicationName": "NetflixApp",
        "applicationId": "1234"
    }
}
```

