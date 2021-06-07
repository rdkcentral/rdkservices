<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.RDKShell_Plugin"></a>
# RDKShell Plugin

**Version: 2.0**

**Status: :black_circle::black_circle::black_circle:**

org.rdk.RDKShell plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.RDKShell plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `RDKShell` plugin controls the management of composition, layout, Z order, and key handling.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.RDKShell*) |
| classname | string | Class name: *org.rdk.RDKShell* |
| locator | string | Library name: *libWPEFrameworkRDKShell.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.RDKShell plugin:

RDKShell interface methods:

| Method | Description |
| :-------- | :-------- |
| [addAnimation](#method.addAnimation) | (Version 2) Performs a set of animations |
| [addKeyIntercept](#method.addKeyIntercept) | Adds a key intercept to the client application specified |
| [addKeyListener](#method.addKeyListener) | (Version 2) Adds a key listener to an application |
| [createDisplay](#method.createDisplay) |  Creates a display for the specified client using the configuration parameters |
| [destroy](#method.destroy) | (Version 2) Destroys an application |
| [enableInactivityReporting](#method.enableInactivityReporting) | Enables or disables inactivity reporting and events |
| [enableKeyRepeats](#method.enableKeyRepeats) | Enables or disables key repeats |
| [enableLogsFlushing](#method.enableLogsFlushing) | Enables or disables flushing all logs |
| [enableVirtualDisplay](#method.enableVirtualDisplay) | Enables or disables a virtual display for the specified client |
| [generateKey](#method.generateKey) | (Version 2) Triggers key events (key press and release) |
| [getAvailableTypes](#method.getAvailableTypes) | (Version 2) Returns the list of application types available on the firmware |
| [getBounds](#method.getBounds) | Gets the bounds of the specified client |
| [getClients](#method.getClients) | Gets a list of clients |
| [getHolePunch](#method.getHolePunch) | Returns whether video hole punching is enabled or disabled for the specified client |
| [getKeyRepeatsEnabled](#method.getKeyRepeatsEnabled) | Returns whether key repeating is enabled or disabled |
| [getLastWakeupKey](#method.getLastWakeupKey) | Returns the last key press prior to a device wakeup |
| [getLogsFlushingEnabled](#method.getLogsFlushingEnabled) | Returns whether log flushing is enabled or disabled |
| [getLogLevel](#method.getLogLevel) | Returns the currently set logging level |
| [getOpacity](#method.getOpacity) | Gets the opacity of the specified client |
| [getScale](#method.getScale) | (Version 2) Returns the scale of an application |
| [getScreenResolution](#method.getScreenResolution) | Gets the screen resolution |
| [getState](#method.getState) | (Version 2) Returns the state of all applications |
| [getSystemResourceInfo](#method.getSystemResourceInfo) | (Version 2) Returns system resource information about each application |
| [getVirtualDisplayEnabled](#method.getVirtualDisplayEnabled) | Returns whether virtual display is enabled or disabled for the specified client |
| [getVirtualResolution](#method.getVirtualResolution) | Returns whether virtual display is enabled or disabled for the specified client |
| [getVisibility](#method.getVisibility) | Gets the visibility of the specified client |
| [getZOrder](#method.getZOrder) | Returns an array of clients in Z order, starting with the top most application client first |
| [hideSplashLogo](#method.hideSplashLogo) | Removes the splash screen |
| [kill](#method.kill) | Kills the specified client |
| [launch](#method.launch) | Launches an application |
| [moveBehind](#method.moveBehind) | Moves the specified client behind the specified target client |
| [moveToBack](#method.moveToBack) | Moves the specified client to the back or bottom of the Z order |
| [moveToFront](#method.moveToFront) | Moves the specified client to the front or top of the Z order |
| [removeAllKeyIntercepts](#method.removeAllKeyIntercepts) | Removes all key intercepts |
| [removeAllKeyListeners](#method.removeAllKeyListeners) | Removes all key listeners |
| [removeAnimation](#method.removeAnimation) | Removes the current animation for the specified client |
| [removeKeyIntercept](#method.removeKeyIntercept) | Removes a key intercept |
| [removeKeyListener](#method.removeKeyListener) | (Version 2) Removes a key listener for an application |
| [scaleToFit](#method.scaleToFit) | Scales the specified client to fit the current bounds |
| [SetBounds](#method.SetBounds) | Sets the bounds of the specified client |
| [setFocus](#method.setFocus) | Sets focus to the specified client |
| [setHolePunch](#method.setHolePunch) | Enables or disables video hole punching for the specified client |
| [setInactivityInterval](#method.setInactivityInterval) | Sets the inactivity notification interval |
| [setLogLevel](#method.setLogLevel) | Sets the logging level |
| [setMemoryMonitory](#method.setMemoryMonitory) | Enables or disables RAM memory monitoring on the device |
| [setOpacity](#method.setOpacity) | Sets the opacity of the specified client |
| [setScale](#method.setScale) | (Version 2) Scales an application |
| [setScreenResolution](#method.setScreenResolution) | Sets the screen resolution |
| [setTopmost](#method.setTopmost) | Sets whether the specified client appears above all other clients on the display |
| [setVirtualResolution](#method.setVirtualResolution) | Sets the virtual resolution for the specified client |
| [setVisibility](#method.setVisibility) | Sets whether the specified client should be visible |
| [showSplashLogo](#method.showSplashLogo) | Displays the splash screen |
| [showWatermark](#method.showWatermark) | Sets whether a watermark shows on the display |
| [suspend](#method.suspend) | (Version 2) Suspends an application |


<a name="method.addAnimation"></a>
## *addAnimation <sup>method</sup>*

(Version 2) Performs a set of animations.

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
                "tween": "exp1"
            }
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
        "success": true
    }
}
```

<a name="method.addKeyIntercept"></a>
## *addKeyIntercept <sup>method</sup>*

Adds a key intercept to the client application specified. The keys are specified by a key code and a set of modifiers. Regardless of the application that has focus, key presses that match the key code and modifiers will be sent to the client application.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.keyCode | number | The key code of the key to intercept |
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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.addKeyIntercept",
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.addKeyListener"></a>
## *addKeyListener <sup>method</sup>*

(Version 2) Adds a key listener to an application.  The keys are bubbled up based on their z-order.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params.keys | array | A list of keys on which to add a listener for the specified application |
| params.keys[#] | object |  |
| params.keys[#].keyCode | number | The key code of the key to intercept |
| params.keys[#].modifiers | array | A list of modifiers that need to be present to intercept (`ctrl`, `alt`, and `shift` are supported) |
| params.keys[#].modifiers[#] | string |  |
| params.keys[#].activate | boolean | Activate an application on key event. The default is `false` |
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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.addKeyListener",
    "params": {
        "client": "org.rdk.Netflix",
        "keys": [
            {
                "keyCode": 10,
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.createDisplay"></a>
## *createDisplay <sup>method</sup>*

 Creates a display for the specified client using the configuration parameters.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params.displayName | string | A name for the display |
| params.displayWidth | number | The width of the display |
| params.displayHeight | number | The height of the display |
| params.virtualDisplay | boolean | Whether to create a virtual display (`true`) or not (`false`) |
| params.virtualWidth | number | The width of the virtual display |
| params.virtualHeight | number | The height of the virtual display |

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
    "method": "org.rdk.RDKShell.1.createDisplay",
    "params": {
        "client": "org.rdk.Netflix",
        "displayName": "test",
        "displayWidth": 1920,
        "displayHeight": 1080,
        "virtualDisplay": true,
        "virtualWidth": 1920,
        "virtualHeight": 1080
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

<a name="method.destroy"></a>
## *destroy <sup>method</sup>*

(Version 2) Destroys an application.

Also see: [onDestroyed](#event.onDestroyed)

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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.enableInactivityReporting"></a>
## *enableInactivityReporting <sup>method</sup>*

Enables or disables inactivity reporting and events.

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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.enableKeyRepeats"></a>
## *enableKeyRepeats <sup>method</sup>*

Enables or disables key repeats.

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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.enableLogsFlushing"></a>
## *enableLogsFlushing <sup>method</sup>*

Enables or disables flushing all logs.

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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.enableVirtualDisplay"></a>
## *enableVirtualDisplay <sup>method</sup>*

Enables or disables a virtual display for the specified client.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.enableVirtualDisplay",
    "params": {
        "client": "org.rdk.Netflix",
        "enable": true
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

<a name="method.generateKey"></a>
## *generateKey <sup>method</sup>*

(Version 2) Triggers key events (key press and release).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.keys | array | A list of keys to simulate |
| params.keys[#] | object |  |
| params.keys[#].keyCode | number | The key code of the key to intercept |
| params.keys[#].modifiers | array | A list of modifiers that need to be present to intercept (`ctrl`, `alt`, and `shift` are supported) |
| params.keys[#].modifiers[#] | string |  |
| params.keys[#].delay | number | The amount of time to wait (in seconds) before sending the key event |
| params.keys[#]?.callsign | string | <sup>*(optional)*</sup> The application callsign |

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
    "method": "org.rdk.RDKShell.1.generateKey",
    "params": {
        "keys": [
            {
                "keyCode": 10,
                "modifiers": [
                    "shift"
                ],
                "delay": 1.0,
                "callsign": "Cobalt"
            }
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
        "success": true
    }
}
```

<a name="method.getAvailableTypes"></a>
## *getAvailableTypes <sup>method</sup>*

(Version 2) Returns the list of application types available on the firmware.

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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.getAvailableTypes"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "types": [
            "HtmlBrowser"
        ],
        "success": true
    }
}
```

<a name="method.getBounds"></a>
## *getBounds <sup>method</sup>*

Gets the bounds of the specified client.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.x | number | The x location |
| result.y | number | The y location |
| result.w | number | The width |
| result.h | number | The height |
| result?.success | boolean | <sup>*(optional)*</sup> Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.getBounds",
    "params": {
        "client": "org.rdk.Netflix"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "x": 0,
        "y": 0,
        "w": 1920,
        "h": 1080,
        "success": true
    }
}
```

<a name="method.getClients"></a>
## *getClients <sup>method</sup>*

Gets a list of clients.

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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.getClients"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "clients": [
            "org.rdk.Netflix"
        ],
        "success": true
    }
}
```

<a name="method.getHolePunch"></a>
## *getHolePunch <sup>method</sup>*

Returns whether video hole punching is enabled or disabled for the specified client.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.getHolePunch",
    "params": {
        "client": "org.rdk.Netflix"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "holePunch": true,
        "success": true
    }
}
```

<a name="method.getKeyRepeatsEnabled"></a>
## *getKeyRepeatsEnabled <sup>method</sup>*

Returns whether key repeating is enabled or disabled.

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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.getKeyRepeatsEnabled"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "keyRepeat": true,
        "success": true
    }
}
```

<a name="method.getLastWakeupKey"></a>
## *getLastWakeupKey <sup>method</sup>*

Returns the last key press prior to a device wakeup.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.keyCode | number | The key code of the key to intercept |
| result.modifiers | array | A list of modifiers that need to be present to intercept (`ctrl`, `alt`, and `shift` are supported) |
| result.modifiers[#] | string |  |
| result.timestampInSeconds | boolean | The time, in seconds, of the last wakeup |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.getLastWakeupKey"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
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

<a name="method.getLogsFlushingEnabled"></a>
## *getLogsFlushingEnabled <sup>method</sup>*

Returns whether log flushing is enabled or disabled.

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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.getLogsFlushingEnabled"
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

<a name="method.getLogLevel"></a>
## *getLogLevel <sup>method</sup>*

Returns the currently set logging level.

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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.getLogLevel"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "logLevel": "INFO",
        "success": true
    }
}
```

<a name="method.getOpacity"></a>
## *getOpacity <sup>method</sup>*

Gets the opacity of the specified client.

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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": {
        "opacity": 100,
        "success": true
    }
}
```

<a name="method.getScale"></a>
## *getScale <sup>method</sup>*

(Version 2) Returns the scale of an application.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.sx | number | The x scale factor |
| result.sy | number | The y scale factor |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
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
    "id": 1234567890,
    "result": {
        "sx": 0.5,
        "sy": 0.5,
        "success": true
    }
}
```

<a name="method.getScreenResolution"></a>
## *getScreenResolution <sup>method</sup>*

Gets the screen resolution.

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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.getScreenResolution"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "w": 1920,
        "h": 1080,
        "success": true
    }
}
```

<a name="method.getState"></a>
## *getState <sup>method</sup>*

(Version 2) Returns the state of all applications.

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
| result.runtimes[#].uri | string | The URI of the app |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.getState"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
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

<a name="method.getSystemResourceInfo"></a>
## *getSystemResourceInfo <sup>method</sup>*

(Version 2) Returns system resource information about each application.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.runtimes | array | A list of apps and their system resource information |
| result.runtimes[#] | object |  |
| result.runtimes[#].callsign | string | The application callsign |
| result.runtimes[#].ram | integer | The amount of memory the runtime is consuming in kb |
| result.runtimes[#].vram | integer | The amount of graphics memory the runtime is consuming in kb (if supported) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.getSystemResourceInfo"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "runtimes": [
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

<a name="method.getVirtualDisplayEnabled"></a>
## *getVirtualDisplayEnabled <sup>method</sup>*

Returns whether virtual display is enabled or disabled for the specified client.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.getVirtualDisplayEnabled",
    "params": {
        "client": "org.rdk.Netflix"
    }
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

<a name="method.getVirtualResolution"></a>
## *getVirtualResolution <sup>method</sup>*

Returns whether virtual display is enabled or disabled for the specified client.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.getVirtualResolution",
    "params": {
        "client": "org.rdk.Netflix"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "w": 1920,
        "h": 1080,
        "success": true
    }
}
```

<a name="method.getVisibility"></a>
## *getVisibility <sup>method</sup>*

Gets the visibility of the specified client.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |

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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.getVisibility",
    "params": {
        "client": "org.rdk.Netflix"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "visible": true,
        "success": true
    }
}
```

<a name="method.getZOrder"></a>
## *getZOrder <sup>method</sup>*

Returns an array of clients in Z order, starting with the top most application client first.

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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.getZOrder"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "clients": [
            "org.rdk.Netflix"
        ],
        "success": true
    }
}
```

<a name="method.hideSplashLogo"></a>
## *hideSplashLogo <sup>method</sup>*

Removes the splash screen.

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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.hideSplashLogo"
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

<a name="method.kill"></a>
## *kill <sup>method</sup>*

Kills the specified client.

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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.kill",
    "params": {
        "client": "org.rdk.Netflix"
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

<a name="method.launch"></a>
## *launch <sup>method</sup>*

Launches an application.

Also see: [onApplicationLaunched](#event.onApplicationLaunched)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.callsign | string | The application callsign |
| params.type | string | The ID of the runtime package or the callsign of the plugin desired to be cloned |
| params?.version | string | <sup>*(optional)*</sup> The version of the package. Defaults to the latest version |
| params.uri | string | The URI of the app |
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
        "configuration": "",
        "behind": "",
        "displayName": "test"
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

<a name="method.moveBehind"></a>
## *moveBehind <sup>method</sup>*

Moves the specified client behind the specified target client.

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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.moveToBack"></a>
## *moveToBack <sup>method</sup>*

Moves the specified client to the back or bottom of the Z order.

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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.moveToBack",
    "params": {
        "client": "org.rdk.Netflix"
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

<a name="method.moveToFront"></a>
## *moveToFront <sup>method</sup>*

Moves the specified client to the front or top of the Z order.

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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.moveToFront",
    "params": {
        "client": "org.rdk.Netflix"
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

<a name="method.removeAllKeyIntercepts"></a>
## *removeAllKeyIntercepts <sup>method</sup>*

Removes all key intercepts.

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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.removeAllKeyIntercepts"
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

<a name="method.removeAllKeyListeners"></a>
## *removeAllKeyListeners <sup>method</sup>*

Removes all key listeners.

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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.removeAllKeyListeners"
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

<a name="method.removeAnimation"></a>
## *removeAnimation <sup>method</sup>*

Removes the current animation for the specified client.

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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.removeAnimation",
    "params": {
        "client": "org.rdk.Netflix"
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

<a name="method.removeKeyIntercept"></a>
## *removeKeyIntercept <sup>method</sup>*

Removes a key intercept.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.keyCode | number | The key code of the key to intercept |
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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.removeKeyListener"></a>
## *removeKeyListener <sup>method</sup>*

(Version 2) Removes a key listener for an application.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params.keys | array | A list of keys for which to remove a listener for the specified application |
| params.keys[#] | object |  |
| params.keys[#].keyCode | number | The key code of the key to intercept |
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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.removeKeyListener",
    "params": {
        "client": "org.rdk.Netflix",
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.scaleToFit"></a>
## *scaleToFit <sup>method</sup>*

Scales the specified client to fit the current bounds.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.scaleToFit",
    "params": {
        "client": "org.rdk.Netflix",
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.SetBounds"></a>
## *SetBounds <sup>method</sup>*

Sets the bounds of the specified client.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.SetBounds",
    "params": {
        "client": "org.rdk.Netflix",
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.setFocus"></a>
## *setFocus <sup>method</sup>*

Sets focus to the specified client.

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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.setFocus",
    "params": {
        "client": "org.rdk.Netflix"
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

<a name="method.setHolePunch"></a>
## *setHolePunch <sup>method</sup>*

Enables or disables video hole punching for the specified client.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.setHolePunch",
    "params": {
        "client": "org.rdk.Netflix",
        "holePunch": true
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

<a name="method.setInactivityInterval"></a>
## *setInactivityInterval <sup>method</sup>*

Sets the inactivity notification interval.

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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.setLogLevel"></a>
## *setLogLevel <sup>method</sup>*

Sets the logging level.

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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.setMemoryMonitory"></a>
## *setMemoryMonitory <sup>method</sup>*

Enables or disables RAM memory monitoring on the device.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enable | boolean | `true` to enable memory monitoring or `false` to disable memory monitoring |
| params.interval | number | The duration, in seconds, between memory checks |
| params.lowRam | number | The threshold, in Megabytes, after which a low RAM event is generated |
| params.criticallyLowRam | number | The threshold, in Megabytes, after which a critically low RAM event is generated |

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
    "method": "org.rdk.RDKShell.1.setMemoryMonitory",
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.setOpacity"></a>
## *setOpacity <sup>method</sup>*

Sets the opacity of the specified client.

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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.setScale"></a>
## *setScale <sup>method</sup>*

(Version 2) Scales an application.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.setScale",
    "params": {
        "client": "org.rdk.Netflix",
        "sx": 0.5,
        "sy": 0.5
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

<a name="method.setScreenResolution"></a>
## *setScreenResolution <sup>method</sup>*

Sets the screen resolution.

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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.setTopmost"></a>
## *setTopmost <sup>method</sup>*

Sets whether the specified client appears above all other clients on the display.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params.topmost | boolean | `true` to set the client as the top most client |

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
    "method": "org.rdk.RDKShell.1.setTopmost",
    "params": {
        "client": "org.rdk.Netflix",
        "topmost": true
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

<a name="method.setVirtualResolution"></a>
## *setVirtualResolution <sup>method</sup>*

Sets the virtual resolution for the specified client.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
| params.width | number | The width of the virtual resolution |
| params.height | number | The height of the virtual resolution |

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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.setVisibility"></a>
## *setVisibility <sup>method</sup>*

Sets whether the specified client should be visible.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.client | string | The client name |
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
    "id": 1234567890,
    "method": "org.rdk.RDKShell.1.setVisibility",
    "params": {
        "client": "org.rdk.Netflix",
        "visible": true
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

<a name="method.showSplashLogo"></a>
## *showSplashLogo <sup>method</sup>*

Displays the splash screen.

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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.showWatermark"></a>
## *showWatermark <sup>method</sup>*

Sets whether a watermark shows on the display.

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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.suspend"></a>
## *suspend <sup>method</sup>*

(Version 2) Suspends an application.

Also see: [onSuspended](#event.onSuspended)

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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.RDKShell plugin:

RDKShell interface events:

| Event | Description |
| :-------- | :-------- |
| [onApplicationActivated](#event.onApplicationActivated) | Triggered when an application is activated |
| [onApplicationConnected](#event.onApplicationConnected) | Triggered when a connection to an application succeeds |
| [onApplicationDisconnected](#event.onApplicationDisconnected) | Triggered when an attempt to disconnect from an application succeeds |
| [onApplicationFirstFrame](#event.onApplicationFirstFrame) | Triggered when the first frame of an application is loaded |
| [onApplicationLaunched](#event.onApplicationLaunched) | Triggered when an application launches successfully |
| [onApplicationResumed](#event.onApplicationResumed) | Triggered when an application resumes from a suspended state |
| [onApplicationSuspended](#event.onApplicationSuspended) | Triggered when an application is suspended |
| [onApplicationTerminated](#event.onApplicationTerminated) | Triggered when an application terminates |
| [onDestroyed](#event.onDestroyed) | Triggered when a runtime is destroyed |
| [onDeviceCriticallyLowRamWarning](#event.onDeviceCriticallyLowRamWarning) | Triggered when the RAM memory on the device exceeds the configured `criticallyLowRam` threshold value |
| [onDeviceCriticallyLowRamWarningCleared](#event.onDeviceCriticallyLowRamWarningCleared) | Triggered when the RAM memory on the device no longer exceeds the configured `criticallyLowRam` threshold value |
| [onDeviceLowRamWarning](#event.onDeviceLowRamWarning) | Triggered when the RAM memory on the device exceeds the configured `lowRam` threshold value |
| [onDeviceLowRamWarningCleared](#event.onDeviceLowRamWarningCleared) | Triggered when the RAM memory on the device no longer exceeds the configured `lowRam` threshold value |
| [onLaunched](#event.onLaunched) | Triggered when a runtime is launched |
| [onSuspended](#event.onSuspended) | Triggered when a runtime is suspended |
| [onUserInactivity](#event.onUserInactivity) | Triggered when a device has been inactive for a period of time |
| [onWillDestroy](#event.onWillDestroy) | Triggered when an application is set to be destroyed |


<a name="event.onApplicationActivated"></a>
## *onApplicationActivated <sup>event</sup>*

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

<a name="event.onApplicationConnected"></a>
## *onApplicationConnected <sup>event</sup>*

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

<a name="event.onApplicationDisconnected"></a>
## *onApplicationDisconnected <sup>event</sup>*

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

<a name="event.onApplicationFirstFrame"></a>
## *onApplicationFirstFrame <sup>event</sup>*

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

<a name="event.onApplicationLaunched"></a>
## *onApplicationLaunched <sup>event</sup>*

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

<a name="event.onApplicationResumed"></a>
## *onApplicationResumed <sup>event</sup>*

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

<a name="event.onApplicationSuspended"></a>
## *onApplicationSuspended <sup>event</sup>*

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

<a name="event.onApplicationTerminated"></a>
## *onApplicationTerminated <sup>event</sup>*

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

<a name="event.onDestroyed"></a>
## *onDestroyed <sup>event</sup>*

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

<a name="event.onDeviceCriticallyLowRamWarning"></a>
## *onDeviceCriticallyLowRamWarning <sup>event</sup>*

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

<a name="event.onDeviceCriticallyLowRamWarningCleared"></a>
## *onDeviceCriticallyLowRamWarningCleared <sup>event</sup>*

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

<a name="event.onDeviceLowRamWarning"></a>
## *onDeviceLowRamWarning <sup>event</sup>*

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

<a name="event.onDeviceLowRamWarningCleared"></a>
## *onDeviceLowRamWarningCleared <sup>event</sup>*

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

<a name="event.onLaunched"></a>
## *onLaunched <sup>event</sup>*

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

<a name="event.onSuspended"></a>
## *onSuspended <sup>event</sup>*

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

<a name="event.onUserInactivity"></a>
## *onUserInactivity <sup>event</sup>*

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

<a name="event.onWillDestroy"></a>
## *onWillDestroy <sup>event</sup>*

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

