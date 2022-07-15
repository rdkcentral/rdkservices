<!-- Generated automatically, DO NOT EDIT! -->
<a name="RDKShell_Plugin"></a>
# RDKShell Plugin

**Version: 2.0**

**Status: :black_circle::black_circle::black_circle:**

A org.rdk.RDKShell plugin for Thunder framework.

### Table of Contents

- [Introduction](#Introduction)
- [Description](#Description)
- [Configuration](#Configuration)
- [Methods](#Methods)
- [Notifications](#Notifications)

<a name="Introduction"></a>
# Introduction

<a name="Scope"></a>
## Scope

This document describes purpose and functionality of the org.rdk.RDKShell plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

<a name="Case_Sensitivity"></a>
## Case Sensitivity

All identifiers of the interfaces described in this document are case-sensitive. Thus, unless stated otherwise, all keywords, entities, properties, relations and actions should be treated as such.

<a name="Acronyms,_Abbreviations_and_Terms"></a>
## Acronyms, Abbreviations and Terms

The table below provides and overview of acronyms used in this document and their definitions.

| Acronym | Description |
| :-------- | :-------- |
| <a name="API">API</a> | Application Programming Interface |
| <a name="HTTP">HTTP</a> | Hypertext Transfer Protocol |
| <a name="JSON">JSON</a> | JavaScript Object Notation; a data interchange format |
| <a name="JSON-RPC">JSON-RPC</a> | A remote procedure call protocol encoded in JSON |

The table below provides and overview of terms and abbreviations used in this document and their definitions.

| Term | Description |
| :-------- | :-------- |
| <a name="callsign">callsign</a> | The name given to an instance of a plugin. One plugin can be instantiated multiple times, but each instance the instance name, callsign, must be unique. |

<a name="References"></a>
## References

| Ref ID | Description |
| :-------- | :-------- |
| <a name="HTTP">[HTTP](http://www.w3.org/Protocols)</a> | HTTP specification |
| <a name="JSON-RPC">[JSON-RPC](https://www.jsonrpc.org/specification)</a> | JSON-RPC 2.0 specification |
| <a name="JSON">[JSON](http://www.json.org/)</a> | JSON specification |
| <a name="Thunder">[Thunder](https://github.com/WebPlatformForEmbedded/Thunder/blob/master/doc/WPE%20-%20API%20-%20WPEFramework.docx)</a> | Thunder API Reference |

<a name="Description"></a>
# Description

The `RDKShell` plugin controls the management of composition, layout, Z order, and key handling.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.RDKShell*) |
| classname | string | Class name: *org.rdk.RDKShell* |
| locator | string | Library name: *libWPEFrameworkRDKShell.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.RDKShell plugin:

RDKShell interface methods:

| Method | Description |
| :-------- | :-------- |
| [addAnimation](#addAnimation) | Performs the set of animations |
| [addKeyIntercept](#addKeyIntercept) | Adds a key intercept to the client application specified |
| [addKeyIntercepts](#addKeyIntercepts) | Adds the list of key intercepts |
| [addKeyListener](#addKeyListener) | Adds a key listener to an application |
| [addKeyMetadataListener](#addKeyMetadataListener) | Adds the key metadata listeners |
| [createDisplay](#createDisplay) |  Creates a display for the specified client using the configuration parameters |
| [destroy](#destroy) | Destroys an application |
| [enableInactivityReporting](#enableInactivityReporting) | Enables or disables inactivity reporting and events |
| [enableKeyRepeats](#enableKeyRepeats) | Enables or disables key repeats |
| [enableLogsFlushing](#enableLogsFlushing) | Enables or disables flushing all logs |
| [enableVirtualDisplay](#enableVirtualDisplay) | Enables or disables a virtual display for the specified client |
| [generateKey](#generateKey) | Triggers the key events (key press and release) |
| [getAvailableTypes](#getAvailableTypes) | Returns the list of application types available on the firmware |
| [getBounds](#getBounds) | Gets the bounds of the specified client |
| [getClients](#getClients) | Gets a list of clients |
| [getCursorSize](#getCursorSize) | Returns the currently set cursor size |
| [getHolePunch](#getHolePunch) | Returns whether video hole punching is enabled or disabled for the specified client |
| [getKeyRepeatsEnabled](#getKeyRepeatsEnabled) | Returns whether key repeating is enabled or disabled |
| [getLastWakeupKey](#getLastWakeupKey) | Returns the last key press prior to a device wakeup |
| [getLogLevel](#getLogLevel) | Returns the currently set logging level |
| [getLogsFlushingEnabled](#getLogsFlushingEnabled) | Returns whether log flushing is enabled or disabled |
| [getOpacity](#getOpacity) | Gets the opacity of the specified client |
| [getScale](#getScale) | Returns the scale of an application |
| [getScreenResolution](#getScreenResolution) | Gets the screen resolution |
| [getScreenshot](#getScreenshot) | Captures a screenshot |
| [getState](#getState) | Returns the state of all applications |
| [getSystemMemory](#getSystemMemory) | Gets the information of System Memory |
| [getSystemResourceInfo](#getSystemResourceInfo) | Returns system resource information about each application |
| [getVirtualDisplayEnabled](#getVirtualDisplayEnabled) | Returns whether virtual display is enabled or disabled for the specified client |
| [getVirtualResolution](#getVirtualResolution) | Returns the virtual display resolution for the specified client |
| [getVisibility](#getVisibility) | Gets the visibility of the specified client |
| [getZOrder](#getZOrder) | Returns an array of clients in Z order, starting with the top most application client first |
| [hideAllClients](#hideAllClients) | Hides/Unhides all the clients |
| [hideCursor](#hideCursor) | Hides the cursor from showing on the display |
| [hideFullScreenImage](#hideFullScreenImage) | Hides the Full Screen Image |
| [hideSplashLogo](#hideSplashLogo) | Removes the splash screen |
| [ignoreKeyInputs](#ignoreKeyInputs) | Blocks user key inputs |
| [injectKey](#injectKey) | Injects the keys |
| [kill](#kill) | Kills the specified client |
| [launch](#launch) | Launches an application |
| [launchApplication](#launchApplication) | Launches an application |
| [launchResidentApp](#launchResidentApp) | Launches the Resident application |
| [moveBehind](#moveBehind) | Moves the specified client behind the specified target client |
| [moveToBack](#moveToBack) | Moves the specified client to the back or bottom of the Z order |
| [moveToFront](#moveToFront) | Moves the specified client to the front or top of the Z order |
| [removeAllKeyIntercepts](#removeAllKeyIntercepts) | Removes all key intercepts |
| [removeAllKeyListeners](#removeAllKeyListeners) | Removes all key listeners |
| [removeAnimation](#removeAnimation) | Removes the current animation for the specified client |
| [removeKeyIntercept](#removeKeyIntercept) | Removes a key intercept |
| [removeKeyListener](#removeKeyListener) | Removes a key listener for an application |
| [removeKeyMetadataListener](#removeKeyMetadataListener) | Removes the key metadata listeners |
| [resetInactivityTime](#resetInactivityTime) | Resets the inactivity notification interval |
| [resumeApplication](#resumeApplication) | Resumes an application |
| [scaleToFit](#scaleToFit) | Scales the specified client to fit the current bounds |
| [setBounds](#setBounds) | Sets the bounds of the specified client |
| [setCursorSize](#setCursorSize) | Sets the cursor size |
| [setFocus](#setFocus) | Sets focus to the specified client |
| [setHolePunch](#setHolePunch) | Enables or disables video hole punching for the specified client |
| [setInactivityInterval](#setInactivityInterval) | Sets the inactivity notification interval |
| [setLogLevel](#setLogLevel) | Sets the logging level |
| [setMemoryMonitor](#setMemoryMonitor) | Enables or disables RAM memory monitoring on the device |
| [setOpacity](#setOpacity) | Sets the opacity of the specified client |
| [setScale](#setScale) | Scales an application |
| [setScreenResolution](#setScreenResolution) | Sets the screen resolution |
| [setTopmost](#setTopmost) | Sets whether the specified client appears above all other clients on the display |
| [setVirtualResolution](#setVirtualResolution) | Sets the virtual resolution for the specified client |
| [setVisibility](#setVisibility) | Sets whether the specified client should be visible |
| [showCursor](#showCursor) | Shows the cursor on the display using the current cursor size |
| [showFullScreenImage](#showFullScreenImage) | Shows the Full Screen Image |
| [showSplashLogo](#showSplashLogo) | Displays the splash screen |
| [showWatermark](#showWatermark) | Sets whether a watermark shows on the display |
| [suspend](#suspend) | Suspends an application |
| [suspendApplication](#suspendApplication) | Suspends an application |


<a name="addAnimation"></a>
## *addAnimation*

Performs the set of animations. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.animations | array | A list of animations to perform |
| params.animations[#] | object |  |
| params.animations[#].client | string | The client name |
| params.animations[#]?.x | number | <sup>*(optional)*</sup> The x location |
| params.animations[#]?.y | number | <sup>*(optional)*</sup> The y location |
| params.animations[#]?.w | number | <sup>*(optional)*</sup> The width |
| params.animations[#]?.h | number | <sup>*(optional)*</sup> The height |
| params.animations[#]?.sx | number | <sup>*(optional)*</sup> The x scale factor |
| params.animations[#]?.sy | number | <sup>*(optional)*</sup> The y scale factor |
| params.animations[#]?.a | number | <sup>*(optional)*</sup> The alpha/opacity level to animate to (between 0 and 100) |
| params.animations[#]?.duration | number | <sup>*(optional)*</sup> The duration of the animation in seconds |
| params.animations[#]?.tween | string | <sup>*(optional)*</sup> The animation tween type. The default is `linear` (must be one of the following: *linear*, *exp1*, *exp2*, *exp3*, *inquad*, *incubic*, *nback*, *inelastic*, *outelastic*, *outbounce*) |
| params.animations[#]?.delay | number | <sup>*(optional)*</sup> Set delay for an animation |

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
    "method": "org.rdk.RDKShell.1.addAnimation",
    "params": {
        "animations": [
            {
                "client": "org.rdk.Netflix",
                "x": 0,
                "y": 0,
                "w": 1920,
                "h": 1080,
                "sx": 0.5,
                "sy": 0.5,
                "a": 0,
                "duration": 2,
                "tween": "exp1",
                "delay": 0
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

<a name="addKeyIntercept"></a>
## *addKeyIntercept*

Adds a key intercept to the client application specified. The keys are specified by a key code and a set of modifiers. Regardless of the application that has focus, key presses that match the key code and modifiers will be sent to the client application. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.keyCode | number | The key code of the key to intercept (only symbol * (string data type) is acceptable) |
| params.modifiers | array | A list of modifiers that need to be present to intercept (`ctrl`, `alt`, and `shift` are supported) |
| params.modifiers[#] | string |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |

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
    "method": "org.rdk.RDKShell.1.addKeyIntercept",
    "params": {
        "keyCode": 10,
        "modifiers": [
            "shift"
        ],
        "client": "org.rdk.Netflix",
        "callsign": "org.rdk.Netflix"
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

<a name="addKeyIntercepts"></a>
## *addKeyIntercepts*

Adds the list of key intercepts. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.intercepts | array | A list of intercepts |
| params.intercepts[#] | object |  |
| params.intercepts[#].client | string | The client name |
| params.intercepts[#].keys | array | A list of keys to simulate |
| params.intercepts[#].keys[#] | object |  |
| params.intercepts[#].keys[#].keycode | number | The key code of the key to intercept (only symbol * (string data type) is acceptable) |
| params.intercepts[#].keys[#].modifiers | array | A list of modifiers that need to be present to intercept (`ctrl`, `alt`, and `shift` are supported) |
| params.intercepts[#].keys[#].modifiers[#] | string |  |

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
    "method": "org.rdk.RDKShell.1.addKeyIntercepts",
    "params": {
        "intercepts": [
            {
                "client": "searchanddiscovery",
                "keys": [
                    {
                        "keycode": 10,
                        "modifiers": [
                            "shift"
                        ]
                    }
                ]
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

<a name="addKeyListener"></a>
## *addKeyListener*

Adds a key listener to an application. The keys are bubbled up based on their z-order. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |
| params.keys | array | A list of keys on which to add a listener for the specified application |
| params.keys[#] | object |  |
| params.keys[#].keyCode | number | The key code of the key to intercept (only symbol * (string data type) is acceptable) |
| params.keys[#]?.nativekeyCode | number | <sup>*(optional)*</sup> The native key code |
| params.keys[#].modifiers | array | A list of modifiers that need to be present to intercept (`ctrl`, `alt`, and `shift` are supported) |
| params.keys[#].modifiers[#] | string |  |
| params.keys[#].activate | boolean | Activate an application on key  The default is `false` |
| params.keys[#].propagate | boolean | Propagate to the next application in the z-order. The default is `true` |

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
    "method": "org.rdk.RDKShell.1.addKeyListener",
    "params": {
        "client": "org.rdk.Netflix",
        "callsign": "org.rdk.Netflix",
        "keys": [
            {
                "keyCode": 10,
                "nativekeyCode": 10,
                "modifiers": [
                    "shift"
                ],
                "activate": false,
                "propagate": true
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

<a name="addKeyMetadataListener"></a>
## *addKeyMetadataListener*

Adds the key metadata listeners. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |

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
    "method": "org.rdk.RDKShell.1.addKeyMetadataListener",
    "params": {
        "client": "searchanddiscovery",
        "callsign": "searchanddiscovery"
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

<a name="createDisplay"></a>
## *createDisplay*

 Creates a display for the specified client using the configuration parameters. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |
| params.displayName | string | A name for the display |
| params.displayWidth | number | The width of the display |
| params.displayHeight | number | The height of the display |
| params.virtualDisplay | boolean | Whether to create a virtual display (`true`) or not (`false`) |
| params.virtualWidth | number | The width of the virtual display |
| params.virtualHeight | number | The height of the virtual display |
| params?.topmost | boolean | <sup>*(optional)*</sup> Whether to set topmost (true) or not (false) |
| params?.focus | boolean | <sup>*(optional)*</sup> Whether to setfocus (true) or not (false) |

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
    "method": "org.rdk.RDKShell.1.createDisplay",
    "params": {
        "client": "org.rdk.Netflix",
        "callsign": "org.rdk.Netflix",
        "displayName": "test",
        "displayWidth": 1920,
        "displayHeight": 1080,
        "virtualDisplay": true,
        "virtualWidth": 1920,
        "virtualHeight": 1080,
        "topmost": false,
        "focus": false
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

<a name="destroy"></a>
## *destroy*

Destroys an application. 
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `OnDestroyed` | Triggers when a runtime is successfully destroyed |.

Also see: [onDestroyed](#onDestroyed)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.callsign | string | The application callsign |

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
    "method": "org.rdk.RDKShell.1.destroy",
    "params": {
        "callsign": "Cobalt"
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

<a name="enableInactivityReporting"></a>
## *enableInactivityReporting*

Enables or disables inactivity reporting and events. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enable | boolean | Whether to enable (`true`) or disable (`false`) inactivity reporting |

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
    "method": "org.rdk.RDKShell.1.enableInactivityReporting",
    "params": {
        "enable": true
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

<a name="enableKeyRepeats"></a>
## *enableKeyRepeats*

Enables or disables key repeats. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enable | boolean | Whether to enable (`true`) or disable (`false`) key repeats |

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
    "method": "org.rdk.RDKShell.1.enableKeyRepeats",
    "params": {
        "enable": true
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

<a name="enableLogsFlushing"></a>
## *enableLogsFlushing*

Enables or disables flushing all logs. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enable | boolean | Whether to enable (`true`) or disable (`false`) log flushing |

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
    "method": "org.rdk.RDKShell.1.enableLogsFlushing",
    "params": {
        "enable": true
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

<a name="enableVirtualDisplay"></a>
## *enableVirtualDisplay*

Enables or disables a virtual display for the specified client. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |
| params.enable | boolean | Whether to enable (`true`) or disable (`false`) a virtual display |

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
    "method": "org.rdk.RDKShell.1.enableVirtualDisplay",
    "params": {
        "client": "org.rdk.Netflix",
        "callsign": "org.rdk.Netflix",
        "enable": true
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

<a name="generateKey"></a>
## *generateKey*

Triggers the key events (key press and release). 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.keys | array | A list of keys to simulate |
| params.keys[#] | object |  |
| params.keys[#].keyCode | number | The key code of the key to intercept (only symbol * (string data type) is acceptable) |
| params.keys[#].modifiers | array | A list of modifiers that need to be present to intercept (`ctrl`, `alt`, and `shift` are supported) |
| params.keys[#].modifiers[#] | string |  |
| params.keys[#].delay | number | The amount of time to wait (in seconds) before sending the key event |
| params.keys[#]?.callsign | string | <sup>*(optional)*</sup> The application callsign |
| params.keys[#]?.client | string | <sup>*(optional)*</sup> The client name can be used instead of callsign |

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
    "method": "org.rdk.RDKShell.1.generateKey",
    "params": {
        "keys": [
            {
                "keyCode": 10,
                "modifiers": [
                    "shift"
                ],
                "delay": 1.0,
                "callsign": "Cobalt",
                "client": "Cobalt"
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

<a name="getAvailableTypes"></a>
## *getAvailableTypes*

Returns the list of application types available on the firmware. 
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.types | array | application types |
| result.types[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RDKShell.1.getAvailableTypes"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "types": [
            "HtmlBrowser"
        ],
        "success": true
    }
}
```

<a name="getBounds"></a>
## *getBounds*

Gets the bounds of the specified client. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.bounds | object |  |
| result.bounds.x | number | The x location |
| result.bounds.y | number | The y location |
| result.bounds.w | number | The width |
| result.bounds.h | number | The height |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RDKShell.1.getBounds",
    "params": {
        "client": "org.rdk.Netflix",
        "callsign": "org.rdk.Netflix"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "bounds": {
            "x": 0,
            "y": 0,
            "w": 1920,
            "h": 1080
        },
        "success": true
    }
}
```

<a name="getClients"></a>
## *getClients*

Gets a list of clients. 
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.clients | array | A list of clients |
| result.clients[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RDKShell.1.getClients"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "clients": [
            "org.rdk.Netflix"
        ],
        "success": true
    }
}
```

<a name="getCursorSize"></a>
## *getCursorSize*

Returns the currently set cursor size. 
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.width | number | The cursor width |
| result.height | number | The cursor height |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RDKShell.1.getCursorSize"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "width": 255,
        "height": 255,
        "success": true
    }
}
```

<a name="getHolePunch"></a>
## *getHolePunch*

Returns whether video hole punching is enabled or disabled for the specified client. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.holePunch | boolean | Whether hole punching is enabled (`true`) or disabled (`false`) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RDKShell.1.getHolePunch",
    "params": {
        "client": "org.rdk.Netflix",
        "callsign": "org.rdk.Netflix"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "holePunch": true,
        "success": true
    }
}
```

<a name="getKeyRepeatsEnabled"></a>
## *getKeyRepeatsEnabled*

Returns whether key repeating is enabled or disabled. 
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.keyRepeat | boolean | `true` if enabled, otherwise `false` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RDKShell.1.getKeyRepeatsEnabled"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "keyRepeat": true,
        "success": true
    }
}
```

<a name="getLastWakeupKey"></a>
## *getLastWakeupKey*

Returns the last key press prior to a device wakeup. 
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.keyCode | number | The key code of the key to intercept (only symbol * (string data type) is acceptable) |
| result.modifiers | array | A list of modifiers that need to be present to intercept (`ctrl`, `alt`, and `shift` are supported) |
| result.modifiers[#] | string |  |
| result.timestampInSeconds | boolean | The time, in seconds, of the last wakeup |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RDKShell.1.getLastWakeupKey"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "keyCode": 10,
        "modifiers": [
            "shift"
        ],
        "timestampInSeconds": true,
        "success": true
    }
}
```

<a name="getLogLevel"></a>
## *getLogLevel*

Returns the currently set logging level. 
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.logLevel | string | The log level (must be one of the following: *DEBUG*, *INFO*, *WARN*, *ERROR*, *FATAL*) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RDKShell.1.getLogLevel"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "logLevel": "INFO",
        "success": true
    }
}
```

<a name="getLogsFlushingEnabled"></a>
## *getLogsFlushingEnabled*

Returns whether log flushing is enabled or disabled. 
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.enabled | boolean | `true` if enabled, otherwise `false` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RDKShell.1.getLogsFlushingEnabled"
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

<a name="getOpacity"></a>
## *getOpacity*

Gets the opacity of the specified client. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.opacity | integer | The opacity level (between 0 and 100) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RDKShell.1.getOpacity",
    "params": {
        "client": "org.rdk.Netflix"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "opacity": 100,
        "success": true
    }
}
```

<a name="getScale"></a>
## *getScale*

Returns the scale of an application. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.sx | string | The x scale factor |
| result.sy | string | The y scale factor |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RDKShell.1.getScale",
    "params": {
        "client": "org.rdk.Netflix"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "sx": "0.5",
        "sy": "0.5",
        "success": true
    }
}
```

<a name="getScreenResolution"></a>
## *getScreenResolution*

Gets the screen resolution. 
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.w | number | The width |
| result.h | number | The height |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RDKShell.1.getScreenResolution"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "w": 1920,
        "h": 1080,
        "success": true
    }
}
```

<a name="getScreenshot"></a>
## *getScreenshot*

Captures a screenshot. 
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onScreenshotComplete` | Triggers when a screenshot is captured successfully |.

Also see: [onScreenshotComplete](#onScreenshotComplete)

### Parameters

This method takes no parameters.

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
    "method": "org.rdk.RDKShell.1.getScreenshot"
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

<a name="getState"></a>
## *getState*

Returns the state of all applications.
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.runtimes | array | A list of apps and their current state |
| result.runtimes[#] | object |  |
| result.runtimes[#].callsign | string | The application callsign |
| result.runtimes[#].state | string | The runtime state of the app |
| result.runtimes[#].uri | string | The URI of the app (empty, if unable to get URI status) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RDKShell.1.getState"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "runtimes": [
            {
                "callsign": "Cobalt",
                "state": "suspended",
                "uri": "https://..."
            }
        ],
        "success": true
    }
}
```

<a name="getSystemMemory"></a>
## *getSystemMemory*

Gets the information of System Memory. 
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.freeRam | number | Free RAM memory (kB) |
| result.swapRam | number | Swap memory (kB) |
| result.totalRam | number | Total RAM memory (kB) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RDKShell.1.getSystemMemory"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "freeRam": 321944,
        "swapRam": 0,
        "totalRam": 624644,
        "success": true
    }
}
```

<a name="getSystemResourceInfo"></a>
## *getSystemResourceInfo*

Returns system resource information about each application. 
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.types | array | A list of apps and their system resource information |
| result.types[#] | object |  |
| result.types[#].callsign | string | The application callsign |
| result.types[#].ram | integer | The amount of memory the runtime is consuming in kb |
| result.types[#].vram | integer | The amount of graphics memory the runtime is consuming in kb (if supported) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RDKShell.1.getSystemResourceInfo"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "types": [
            {
                "callsign": "Cobalt",
                "ram": 123,
                "vram": 50
            }
        ],
        "success": true
    }
}
```

<a name="getVirtualDisplayEnabled"></a>
## *getVirtualDisplayEnabled*

Returns whether virtual display is enabled or disabled for the specified client. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.enabled | boolean | `true` if a virtual display is enabled, otherwise `false` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RDKShell.1.getVirtualDisplayEnabled",
    "params": {
        "client": "org.rdk.Netflix",
        "callsign": "org.rdk.Netflix"
    }
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

<a name="getVirtualResolution"></a>
## *getVirtualResolution*

Returns the virtual display resolution for the specified client.
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.w | number | The width |
| result.h | number | The height |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RDKShell.1.getVirtualResolution",
    "params": {
        "client": "org.rdk.Netflix",
        "callsign": "org.rdk.Netflix"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "w": 1920,
        "h": 1080,
        "success": true
    }
}
```

<a name="getVisibility"></a>
## *getVisibility*

Gets the visibility of the specified client. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.visible | boolean | Whether the client is visible (`true`) or not (`false`) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RDKShell.1.getVisibility",
    "params": {
        "client": "org.rdk.Netflix",
        "callsign": "org.rdk.Netflix"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "visible": true,
        "success": true
    }
}
```

<a name="getZOrder"></a>
## *getZOrder*

Returns an array of clients in Z order, starting with the top most application client first. 
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.clients | array | A list of clients |
| result.clients[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RDKShell.1.getZOrder"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "clients": [
            "org.rdk.Netflix"
        ],
        "success": true
    }
}
```

<a name="hideAllClients"></a>
## *hideAllClients*

Hides/Unhides all the clients. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.hide | boolean | true to hide all the clients, and false to unhide all the clients |

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
    "method": "org.rdk.RDKShell.1.hideAllClients",
    "params": {
        "hide": true
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

<a name="hideCursor"></a>
## *hideCursor*

Hides the cursor from showing on the display. The cursor is hidden by default. 
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

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
    "method": "org.rdk.RDKShell.1.hideCursor"
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

<a name="hideFullScreenImage"></a>
## *hideFullScreenImage*

Hides the Full Screen Image. 
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

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
    "method": "org.rdk.RDKShell.1.hideFullScreenImage"
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

<a name="hideSplashLogo"></a>
## *hideSplashLogo*

Removes the splash screen. 
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

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
    "method": "org.rdk.RDKShell.1.hideSplashLogo"
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

<a name="ignoreKeyInputs"></a>
## *ignoreKeyInputs*

Blocks user key inputs.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ignore | boolean | Whether key inputs are ignored |

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
    "method": "org.rdk.RDKShell.1.ignoreKeyInputs",
    "params": {
        "ignore": false
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

<a name="injectKey"></a>
## *injectKey*

Injects the keys. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.keycode | number | The key code of the key to intercept (only symbol * (string data type) is acceptable) |
| params.modifiers | array | A list of modifiers that need to be present to intercept (`ctrl`, `alt`, and `shift` are supported) |
| params.modifiers[#] | string |  |

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
    "method": "org.rdk.RDKShell.1.injectKey",
    "params": {
        "keycode": 10,
        "modifiers": [
            "shift"
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

<a name="kill"></a>
## *kill*

Kills the specified client. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |

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
    "method": "org.rdk.RDKShell.1.kill",
    "params": {
        "client": "org.rdk.Netflix",
        "callsign": "org.rdk.Netflix"
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

<a name="launch"></a>
## *launch*

Launches an application. 
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onLaunched` | Triggers when the runtime of an application is launched successfully |.

Also see: [onLaunched](#onLaunched)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.callsign | string | The application callsign |
| params.type | string | The ID of the runtime package or the callsign of the plugin desired to be cloned |
| params?.version | string | <sup>*(optional)*</sup> The version of the package. Defaults to the latest version |
| params.uri | string | The URI of the app (empty, if unable to get URI status) |
| params?.x | number | <sup>*(optional)*</sup> The x location |
| params?.y | number | <sup>*(optional)*</sup> The y location |
| params?.w | number | <sup>*(optional)*</sup> The width |
| params?.h | number | <sup>*(optional)*</sup> The height |
| params?.suspend | boolean | <sup>*(optional)*</sup> Whether to suspend the app on launch (`true`) or not (`false`). Default is `false` |
| params?.visible | boolean | <sup>*(optional)*</sup> Whether the app is visible on launch (`true`) or not (`false`). Default is `true`. The value will be `false` if the `suspend` argument is `true` |
| params?.focused | boolean | <sup>*(optional)*</sup> Whether the app is focused on launch (`true`) or not (`false`). Default is `true`. The app cannot be focused if the `visible` argument is set to `false` |
| params?.configuration | string | <sup>*(optional)*</sup> The plugin configuration overrides. Empty by default |
| params?.behind | string | <sup>*(optional)*</sup> The client to put behind. Defaults to top of z-order |
| params?.displayName | string | <sup>*(optional)*</sup> A name for the display |
| params?.scaleToFit | boolean | <sup>*(optional)*</sup> Whether the app can be scaled to fit the current bounds. Default is `false` |
| params?.holePunch | boolean | <sup>*(optional)*</sup> Whether the video hole punching can be enabled for the client. Default is `true` |
| params?.topmost | boolean | <sup>*(optional)*</sup> Whether the app appears above all other apps on the display. Default is `false` |
| params?.focus | boolean | <sup>*(optional)*</sup> Whether the app should be under focus. Default is `false` |
| params?.autodestroy | boolean | <sup>*(optional)*</sup> Whether the application can be automatically destroyed. Default is `true` |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.launchType | string | The launch type of client |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RDKShell.1.launch",
    "params": {
        "callsign": "Cobalt",
        "type": "HtmlApp",
        "version": "1.0",
        "uri": "https://...",
        "x": 0,
        "y": 0,
        "w": 1920,
        "h": 1080,
        "suspend": false,
        "visible": true,
        "focused": true,
        "configuration": "...",
        "behind": "...",
        "displayName": "test",
        "scaleToFit": false,
        "holePunch": false,
        "topmost": false,
        "focus": false,
        "autodestroy": false
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "launchType": "activate",
        "success": true
    }
}
```

<a name="launchApplication"></a>
## *launchApplication*

Launches an application. 
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onApplicationLaunched` | Triggers when an application is launched successfully |.

Also see: [onApplicationLaunched](#onApplicationLaunched)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params.uri | string | The client uri |
| params.mimeType | string | The mime type |
| params?.topmost | boolean | <sup>*(optional)*</sup> true to put the application at the top, otherwise false |
| params?.focus | boolean | <sup>*(optional)*</sup> true to put the application above all other applications, otherwise false |

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
    "method": "org.rdk.RDKShell.1.launchApplication",
    "params": {
        "client": "HtmlApp",
        "uri": "https://x1box-app.xumo.com/3.0.70/index.html%22",
        "mimeType": "application/native",
        "topmost": true,
        "focus": true
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

<a name="launchResidentApp"></a>
## *launchResidentApp*

Launches the Resident application. 
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

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
    "method": "org.rdk.RDKShell.1.launchResidentApp"
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

<a name="moveBehind"></a>
## *moveBehind*

Moves the specified client behind the specified target client. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params.target | string | The target in which the client moves behind |

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
    "method": "org.rdk.RDKShell.1.moveBehind",
    "params": {
        "client": "org.rdk.Netflix",
        "target": "org.rdk.RDKBrowser2"
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

<a name="moveToBack"></a>
## *moveToBack*

Moves the specified client to the back or bottom of the Z order. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |

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
    "method": "org.rdk.RDKShell.1.moveToBack",
    "params": {
        "client": "org.rdk.Netflix",
        "callsign": "org.rdk.Netflix"
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

<a name="moveToFront"></a>
## *moveToFront*

Moves the specified client to the front or top of the Z order. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |

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
    "method": "org.rdk.RDKShell.1.moveToFront",
    "params": {
        "client": "org.rdk.Netflix",
        "callsign": "org.rdk.Netflix"
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

<a name="removeAllKeyIntercepts"></a>
## *removeAllKeyIntercepts*

Removes all key intercepts. 
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

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
    "method": "org.rdk.RDKShell.1.removeAllKeyIntercepts"
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

<a name="removeAllKeyListeners"></a>
## *removeAllKeyListeners*

Removes all key listeners. 
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

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
    "method": "org.rdk.RDKShell.1.removeAllKeyListeners"
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

<a name="removeAnimation"></a>
## *removeAnimation*

Removes the current animation for the specified client. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |

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
    "method": "org.rdk.RDKShell.1.removeAnimation",
    "params": {
        "client": "org.rdk.Netflix",
        "callsign": "org.rdk.Netflix"
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

<a name="removeKeyIntercept"></a>
## *removeKeyIntercept*

Removes a key intercept. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.keyCode | number | The key code of the key to intercept (only symbol * (string data type) is acceptable) |
| params.modifiers | array | A list of modifiers that need to be present to intercept (`ctrl`, `alt`, and `shift` are supported) |
| params.modifiers[#] | string |  |
| params.client | string | The client name |

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
    "method": "org.rdk.RDKShell.1.removeKeyIntercept",
    "params": {
        "keyCode": 10,
        "modifiers": [
            "shift"
        ],
        "client": "org.rdk.Netflix"
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

<a name="removeKeyListener"></a>
## *removeKeyListener*

Removes a key listener for an application. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |
| params.keys | array | A list of keys for which to remove a listener for the specified application (only symbol * (string data type) is acceptable) |
| params.keys[#] | object |  |
| params.keys[#].keyCode | number | The key code of the key to intercept (only symbol * (string data type) is acceptable) |
| params.keys[#].modifiers | array | A list of modifiers that need to be present to intercept (`ctrl`, `alt`, and `shift` are supported) |
| params.keys[#].modifiers[#] | string |  |

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
    "method": "org.rdk.RDKShell.1.removeKeyListener",
    "params": {
        "client": "org.rdk.Netflix",
        "callsign": "org.rdk.Netflix",
        "keys": [
            {
                "keyCode": 10,
                "modifiers": [
                    "shift"
                ]
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

<a name="removeKeyMetadataListener"></a>
## *removeKeyMetadataListener*

Removes the key metadata listeners. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |

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
    "method": "org.rdk.RDKShell.1.removeKeyMetadataListener",
    "params": {
        "client": "searchanddiscovery",
        "callsign": "searchanddiscovery"
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

<a name="resetInactivityTime"></a>
## *resetInactivityTime*

Resets the inactivity notification interval. 
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

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
    "method": "org.rdk.RDKShell.1.resetInactivityTime"
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

<a name="resumeApplication"></a>
## *resumeApplication*

Resumes an application. 
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onApplicationResumed` | Triggers when an application resumes from a suspended state |.

Also see: [onApplicationResumed](#onApplicationResumed)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

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
    "method": "org.rdk.RDKShell.1.resumeApplication",
    "params": {
        "client": "HtmlApp"
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

<a name="scaleToFit"></a>
## *scaleToFit*

Scales the specified client to fit the current bounds. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |
| params.x | number | The x location |
| params.y | number | The y location |
| params.w | number | The width |
| params.h | number | The height |

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
    "method": "org.rdk.RDKShell.1.scaleToFit",
    "params": {
        "client": "org.rdk.Netflix",
        "callsign": "org.rdk.Netflix",
        "x": 0,
        "y": 0,
        "w": 1920,
        "h": 1080
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

<a name="setBounds"></a>
## *setBounds*

Sets the bounds of the specified client. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |
| params.x | number | The x location |
| params.y | number | The y location |
| params.w | number | The width |
| params.h | number | The height |

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
    "method": "org.rdk.RDKShell.1.setBounds",
    "params": {
        "client": "org.rdk.Netflix",
        "callsign": "org.rdk.Netflix",
        "x": 0,
        "y": 0,
        "w": 1920,
        "h": 1080
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

<a name="setCursorSize"></a>
## *setCursorSize*

Sets the cursor size. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.width | number | The cursor width |
| params.height | number | The cursor height |

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
    "method": "org.rdk.RDKShell.1.setCursorSize",
    "params": {
        "width": 255,
        "height": 255
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

<a name="setFocus"></a>
## *setFocus*

Sets focus to the specified client. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |

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
    "method": "org.rdk.RDKShell.1.setFocus",
    "params": {
        "client": "org.rdk.Netflix",
        "callsign": "org.rdk.Netflix"
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

<a name="setHolePunch"></a>
## *setHolePunch*

Enables or disables video hole punching for the specified client. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |
| params.holePunch | boolean | Whether hole punching is enabled (`true`) or disabled (`false`) |

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
    "method": "org.rdk.RDKShell.1.setHolePunch",
    "params": {
        "client": "org.rdk.Netflix",
        "callsign": "org.rdk.Netflix",
        "holePunch": true
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

<a name="setInactivityInterval"></a>
## *setInactivityInterval*

Sets the inactivity notification interval. 
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onUserInactivity` | Triggers only if the device is inactive for the specified time interval |.

Also see: [onUserInactivity](#onUserInactivity)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.interval | integer | The inactivity event interval in minutes |

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
    "method": "org.rdk.RDKShell.1.setInactivityInterval",
    "params": {
        "interval": 15
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

<a name="setLogLevel"></a>
## *setLogLevel*

Sets the logging level. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logLevel | string | The log level (must be one of the following: *DEBUG*, *INFO*, *WARN*, *ERROR*, *FATAL*) |

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
    "method": "org.rdk.RDKShell.1.setLogLevel",
    "params": {
        "logLevel": "INFO"
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

<a name="setMemoryMonitor"></a>
## *setMemoryMonitor*

Enables or disables RAM memory monitoring on the device. Upon enabling, triggers possible events are onDeviceLowRamWarning, onDeviceCriticallyLowRamWarning, onDeviceLowRamWarningCleared, and onDeviceCriticallyLowRamWarningCleared. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enable | boolean | `true` to enable memory monitoring or `false` to disable memory monitoring |
| params.interval | number | The duration, in seconds, between memory checks |
| params.lowRam | number | The threshold, in Megabytes, after which a `onDeviceLowRamWarning` event is generated |
| params.criticallyLowRam | number | The threshold, in Megabytes, after which a critically `onDeviceCriticallyLowRamWarning` event is generated |

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
    "method": "org.rdk.RDKShell.1.setMemoryMonitor",
    "params": {
        "enable": true,
        "interval": 300,
        "lowRam": 128,
        "criticallyLowRam": 64
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

<a name="setOpacity"></a>
## *setOpacity*

Sets the opacity of the specified client. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params.opacity | integer | The opacity level (between 0 and 100) |

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
    "method": "org.rdk.RDKShell.1.setOpacity",
    "params": {
        "client": "org.rdk.Netflix",
        "opacity": 100
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

<a name="setScale"></a>
## *setScale*

Scales an application. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |
| params.sx | number | The x scale factor |
| params.sy | number | The y scale factor |

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
    "method": "org.rdk.RDKShell.1.setScale",
    "params": {
        "client": "org.rdk.Netflix",
        "callsign": "org.rdk.Netflix",
        "sx": 0.5,
        "sy": 0.5
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

<a name="setScreenResolution"></a>
## *setScreenResolution*

Sets the screen resolution. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.w | number | The width |
| params.h | number | The height |

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
    "method": "org.rdk.RDKShell.1.setScreenResolution",
    "params": {
        "w": 1920,
        "h": 1080
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

<a name="setTopmost"></a>
## *setTopmost*

Sets whether the specified client appears above all other clients on the display. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params.topmost | boolean | `true` to set the client as the top most client |
| params?.focus | boolean | <sup>*(optional)*</sup> `true' to set focus for the client |

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
    "method": "org.rdk.RDKShell.1.setTopmost",
    "params": {
        "client": "org.rdk.Netflix",
        "topmost": true,
        "focus": true
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

<a name="setVirtualResolution"></a>
## *setVirtualResolution*

Sets the virtual resolution for the specified client. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.width | number | <sup>*(optional)*</sup> The width of the virtual resolution |
| params?.height | number | <sup>*(optional)*</sup> The height of the virtual resolution |

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
    "method": "org.rdk.RDKShell.1.setVirtualResolution",
    "params": {
        "client": "org.rdk.Netflix",
        "width": 1920,
        "height": 1080
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

<a name="setVisibility"></a>
## *setVisibility*

Sets whether the specified client should be visible. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params?.callsign | string | <sup>*(optional)*</sup> The application callsign |
| params.visible | boolean | Whether the client is visible (`true`) or not (`false`) |

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
    "method": "org.rdk.RDKShell.1.setVisibility",
    "params": {
        "client": "org.rdk.Netflix",
        "callsign": "org.rdk.Netflix",
        "visible": true
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

<a name="showCursor"></a>
## *showCursor*

Shows the cursor on the display using the current cursor size. See `setCursorSize`. The cursor automatically disappears after 5 seconds of inactivity. 
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

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
    "method": "org.rdk.RDKShell.1.showCursor"
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

<a name="showFullScreenImage"></a>
## *showFullScreenImage*

Shows the Full Screen Image. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.path | string | The image path |

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
    "method": "org.rdk.RDKShell.1.showFullScreenImage",
    "params": {
        "path": "/tmp/netflix.png"
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

<a name="showSplashLogo"></a>
## *showSplashLogo*

Displays the splash screen. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.displayTime | number | The amount of the time to show the splash screen |

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
    "method": "org.rdk.RDKShell.1.showSplashLogo",
    "params": {
        "displayTime": 5
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

<a name="showWatermark"></a>
## *showWatermark*

Sets whether a watermark shows on the display. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.show | boolean | `true` to show the watermark or `false` to hide the watermark |

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
    "method": "org.rdk.RDKShell.1.showWatermark",
    "params": {
        "show": true
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

<a name="suspend"></a>
## *suspend*

Suspends an application. 
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onSuspended` | Triggers when the runtime of an application is suspended |.

Also see: [onSuspended](#onSuspended)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.callsign | string | The application callsign |

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
    "method": "org.rdk.RDKShell.1.suspend",
    "params": {
        "callsign": "Cobalt"
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

<a name="suspendApplication"></a>
## *suspendApplication*

Suspends an application. 
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onApplicationSuspended` | Triggers when an application is suspended |.

Also see: [onApplicationSuspended](#onApplicationSuspended)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

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
    "method": "org.rdk.RDKShell.1.suspendApplication",
    "params": {
        "client": "HtmlApp"
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

The following events are provided by the org.rdk.RDKShell plugin:

RDKShell interface events:

| Event | Description |
| :-------- | :-------- |
| [onApplicationActivated](#onApplicationActivated) | Triggered when an application is activated |
| [onApplicationConnected](#onApplicationConnected) | Triggered when a connection to an application succeeds |
| [onApplicationDisconnected](#onApplicationDisconnected) | Triggered when an attempt to disconnect from an application succeeds |
| [onApplicationFirstFrame](#onApplicationFirstFrame) | Triggered when the first frame of an application is loaded |
| [onApplicationLaunched](#onApplicationLaunched) | Triggered when an application launches successfully |
| [onApplicationResumed](#onApplicationResumed) | Triggered when an application resumes from a suspended state |
| [onApplicationSuspended](#onApplicationSuspended) | Triggered when an application is suspended |
| [onApplicationTerminated](#onApplicationTerminated) | Triggered when an application terminates |
| [onDestroyed](#onDestroyed) | Triggered when a runtime is destroyed |
| [onDeviceCriticallyLowRamWarning](#onDeviceCriticallyLowRamWarning) | Triggered when the RAM memory on the device exceeds the configured `criticallyLowRam` threshold value |
| [onDeviceCriticallyLowRamWarningCleared](#onDeviceCriticallyLowRamWarningCleared) | Triggered when the RAM memory on the device no longer exceeds the configured `criticallyLowRam` threshold value |
| [onDeviceLowRamWarning](#onDeviceLowRamWarning) | Triggered when the RAM memory on the device exceeds the configured `lowRam` threshold value |
| [onDeviceLowRamWarningCleared](#onDeviceLowRamWarningCleared) | Triggered when the RAM memory on the device no longer exceeds the configured `lowRam` threshold value |
| [onLaunched](#onLaunched) | Triggered when a runtime is launched |
| [onSuspended](#onSuspended) | Triggered when a runtime is suspended |
| [onUserInactivity](#onUserInactivity) | Triggered when a device has been inactive for a period of time |
| [onWillDestroy](#onWillDestroy) | Triggered when an application is set to be destroyed |
| [onPluginSuspended](#onPluginSuspended) | Triggered when a plugin is suspended |
| [onScreenshotComplete](#onScreenshotComplete) | Triggered when a screenshot is captured successfully using `getScreenshot` method |
| [onBlur](#onBlur) | Triggered when the focused client is blurred |
| [onFocus](#onFocus) | Triggered when a client is set to focus |


<a name="onApplicationActivated"></a>
## *onApplicationActivated*

Triggered when an application is activated.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onApplicationActivated",
    "params": {
        "client": "org.rdk.Netflix"
    }
}
```

<a name="onApplicationConnected"></a>
## *onApplicationConnected*

Triggered when a connection to an application succeeds.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onApplicationConnected",
    "params": {
        "client": "org.rdk.Netflix"
    }
}
```

<a name="onApplicationDisconnected"></a>
## *onApplicationDisconnected*

Triggered when an attempt to disconnect from an application succeeds.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onApplicationDisconnected",
    "params": {
        "client": "org.rdk.Netflix"
    }
}
```

<a name="onApplicationFirstFrame"></a>
## *onApplicationFirstFrame*

Triggered when the first frame of an application is loaded.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onApplicationFirstFrame",
    "params": {
        "client": "org.rdk.Netflix"
    }
}
```

<a name="onApplicationLaunched"></a>
## *onApplicationLaunched*

Triggered when an application launches successfully.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onApplicationLaunched",
    "params": {
        "client": "org.rdk.Netflix"
    }
}
```

<a name="onApplicationResumed"></a>
## *onApplicationResumed*

Triggered when an application resumes from a suspended state.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onApplicationResumed",
    "params": {
        "client": "org.rdk.Netflix"
    }
}
```

<a name="onApplicationSuspended"></a>
## *onApplicationSuspended*

Triggered when an application is suspended.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onApplicationSuspended",
    "params": {
        "client": "org.rdk.Netflix"
    }
}
```

<a name="onApplicationTerminated"></a>
## *onApplicationTerminated*

Triggered when an application terminates.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onApplicationTerminated",
    "params": {
        "client": "org.rdk.Netflix"
    }
}
```

<a name="onDestroyed"></a>
## *onDestroyed*

Triggered when a runtime is destroyed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onDestroyed",
    "params": {
        "client": "org.rdk.Netflix"
    }
}
```

<a name="onDeviceCriticallyLowRamWarning"></a>
## *onDeviceCriticallyLowRamWarning*

Triggered when the RAM memory on the device exceeds the configured `criticallyLowRam` threshold value. See `setMemoryMonitor`.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ram | integer | The amount of free memory remaining in Kilobytes |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onDeviceCriticallyLowRamWarning",
    "params": {
        "ram": 65536
    }
}
```

<a name="onDeviceCriticallyLowRamWarningCleared"></a>
## *onDeviceCriticallyLowRamWarningCleared*

Triggered when the RAM memory on the device no longer exceeds the configured `criticallyLowRam` threshold value. See `setMemoryMonitor`.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ram | integer | The amount of free memory remaining in Kilobytes |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onDeviceCriticallyLowRamWarningCleared",
    "params": {
        "ram": 65536
    }
}
```

<a name="onDeviceLowRamWarning"></a>
## *onDeviceLowRamWarning*

Triggered when the RAM memory on the device exceeds the configured `lowRam` threshold value. See `setMemoryMonitor`.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ram | integer | The amount of free memory remaining in Kilobytes |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onDeviceLowRamWarning",
    "params": {
        "ram": 65536
    }
}
```

<a name="onDeviceLowRamWarningCleared"></a>
## *onDeviceLowRamWarningCleared*

Triggered when the RAM memory on the device no longer exceeds the configured `lowRam` threshold value. See `setMemoryMonitor`.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ram | integer | The amount of free memory remaining in Kilobytes |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onDeviceLowRamWarningCleared",
    "params": {
        "ram": 65536
    }
}
```

<a name="onLaunched"></a>
## *onLaunched*

Triggered when a runtime is launched.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params.launchType | string | The launch type of an application (must be one of the following: *create*, *active*, *suspend*, *resume*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onLaunched",
    "params": {
        "client": "org.rdk.Netflix",
        "launchType": "create"
    }
}
```

<a name="onSuspended"></a>
## *onSuspended*

Triggered when a runtime is suspended.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onSuspended",
    "params": {
        "client": "org.rdk.Netflix"
    }
}
```

<a name="onUserInactivity"></a>
## *onUserInactivity*

Triggered when a device has been inactive for a period of time. This event is broadcasted at the frequency specified by `setInactivityInterval` if the device is not active. The default frequency is 15 minutes.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.minutes | number | The number of minutes that the device has been inactive |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onUserInactivity",
    "params": {
        "minutes": 5
    }
}
```

<a name="onWillDestroy"></a>
## *onWillDestroy*

Triggered when an application is set to be destroyed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.callsign | string | The application callsign |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onWillDestroy",
    "params": {
        "callsign": "Cobalt"
    }
}
```

<a name="onPluginSuspended"></a>
## *onPluginSuspended*

Triggered when a plugin is suspended.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onPluginSuspended",
    "params": {
        "client": "searchanddiscovery"
    }
}
```

<a name="onScreenshotComplete"></a>
## *onScreenshotComplete*

Triggered when a screenshot is captured successfully using `getScreenshot` 

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.imageData | string | Base64 encoded image data |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onScreenshotComplete",
    "params": {
        "imageData": "AAAAAAAAAA"
    }
}
```

<a name="onBlur"></a>
## *onBlur*

Triggered when the focused client is blurred.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onBlur",
    "params": {
        "client": "searchanddiscovery"
    }
}
```

<a name="onFocus"></a>
## *onFocus*

Triggered when a client is set to focus.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onFocus",
    "params": {
        "client": "HtmlApp"
    }
}
```

