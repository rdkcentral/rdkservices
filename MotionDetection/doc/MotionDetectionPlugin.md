<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.MotionDetectionPlugin"></a>
# MotionDetectionPlugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

org.rdk.MotionDetection plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.MotionDetection plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `MotionDetection` plugin allows you to control the motion sensors that are available on a set-top box.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.MotionDetection*) |
| classname | string | Class name: *org.rdk.MotionDetection* |
| locator | string | Library name: *libWPEFrameworkMotionDetection.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.MotionDetection plugin:

MotionDetection interface methods:

| Method | Description |
| :-------- | :-------- |
| [arm](#method.arm) | Enables a motion detector in the mode requested |
| [disarm](#method.disarm) | Disables the specified motion detector |
| [getLastMotionEventElapsedTime](#method.getLastMotionEventElapsedTime) | Returns the elapsed time since the last motion event occurred for the specified motion detector |
| [getMotionDetectors](#method.getMotionDetectors) | Returns the available motion detectors and then lists information for each detector including their supported sensitivity mode |
| [getNoMotionPeriod](#method.getNoMotionPeriod) | Returns the no-motion period for the specified motion detector |
| [getSensitivity](#method.getSensitivity) | Returns the current sensitivity configuration for the specified motion detector |
| [isarmed](#method.isarmed) | Returns whether the specified motion detector is enabled |
| [setNoMotionPeriod](#method.setNoMotionPeriod) | Sets the no-motion period, in seconds, for the specified motion detector |
| [setSensitivity](#method.setSensitivity) | Sets the sensitivity of the sensor for the specified motion detector |


<a name="method.arm"></a>
## *arm <sup>method</sup>*

Enables a motion detector in the mode requested. This enables a single shot event. Once an event is sent, the device is in the disarmed state. If the application wishes to receive another event, then the application must re-arm.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.index | string | The name of a motion detector that is returned by the `getMotionDetectors` method |
| params.mode | string | The detection mode. Valid values are `0`: Detect no motion; or `1`: Detect motion (must be one of the following: *0*, *1*) |

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
    "method": "org.rdk.MotionDetection.1.arm",
    "params": {
        "index": "FP_MD",
        "mode": "0"
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

<a name="method.disarm"></a>
## *disarm <sup>method</sup>*

Disables the specified motion detector.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.index | string | The name of a motion detector that is returned by the `getMotionDetectors` method |

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
    "method": "org.rdk.MotionDetection.1.disarm",
    "params": {
        "index": "FP_MD"
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

<a name="method.getLastMotionEventElapsedTime"></a>
## *getLastMotionEventElapsedTime <sup>method</sup>*

Returns the elapsed time since the last motion event occurred for the specified motion detector.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.index | string | The name of a motion detector that is returned by the `getMotionDetectors` method |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.time | integer | The elapsed time in seconds |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.MotionDetection.1.getLastMotionEventElapsedTime",
    "params": {
        "index": "FP_MD"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "time": 1964,
        "success": true
    }
}
```

<a name="method.getMotionDetectors"></a>
## *getMotionDetectors <sup>method</sup>*

Returns the available motion detectors and then lists information for each detector including their supported sensitivity mode.  
  
**Note:** The `sensitivityMode` property that is returned by this method indicates whether a number or a name controls the sensitivity of a motion detector. If `sensitivityMode` is `1`, then a set of properties (`min`, `max`, and `step`) are returned that define a valid number range to use. If `sensitivityMode` is `2`, then a `sensitivities` property is returned that contains valid names to use.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.supportedMotionDetectors | array | Name of each motion detector which can be used as indices in other `MotionDetector` API functions |
| result.supportedMotionDetectors[#] | string |  |
| result.supportedMotionDetectorsInfo | object | a JSON object containing information about each motion detector |
| result.supportedMotionDetectorsInfo.FP_MD | object | A motion detector index name |
| result.supportedMotionDetectorsInfo.FP_MD.description | string | A description of the motion detector |
| result.supportedMotionDetectorsInfo.FP_MD.type | string | The motion detector type. For example, PIR, Microwave, video, hybrid, etc |
| result.supportedMotionDetectorsInfo.FP_MD.distance | integer | The range, in cm, of the motion detection field |
| result.supportedMotionDetectorsInfo.FP_MD.angle | integer | Field of view angle |
| result.supportedMotionDetectorsInfo.FP_MD.sensitivityMode | integer | The supported sensitivity mode. Valid values are `0`: motion sensitivity cannot be changed, `1`: motion sensitivity can be set using an integer within the range defined by the `min`, `max`, and `step` properties, `2`: motion sensitivity can be set using the names listed in the `sensitivities` property. (must be one of the following: *0*, *1*, *2*) |
| result.supportedMotionDetectorsInfo.FP_MD?.sensitivities | array | <sup>*(optional)*</sup> A string [] of sensitivity names that can be set for a motion detector. This property is only valid if the `SensitivityMode` property is set to `2` |
| result.supportedMotionDetectorsInfo.FP_MD?.sensitivities[#] | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.MotionDetection.1.getMotionDetectors"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "supportedMotionDetectors": [
            "FP_MD"
        ],
        "supportedMotionDetectorsInfo": {
            "FP_MD": {
                "description": "The only motion detector",
                "type": "PIR",
                "distance": 5000,
                "angle": 72,
                "sensitivityMode": 2,
                "sensitivities": [
                    "low"
                ]
            }
        },
        "success": true
    }
}
```

<a name="method.getNoMotionPeriod"></a>
## *getNoMotionPeriod <sup>method</sup>*

Returns the no-motion period for the specified motion detector.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.index | string | The name of a motion detector that is returned by the `getMotionDetectors` method |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.period | string | The no-motion period duration in seconds. The value must be greater than 0 |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.MotionDetection.1.getNoMotionPeriod",
    "params": {
        "index": "FP_MD"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "period": "1800",
        "success": true
    }
}
```

<a name="method.getSensitivity"></a>
## *getSensitivity <sup>method</sup>*

Returns the current sensitivity configuration for the specified motion detector. The result is either a `name` property with the sensitivity name or a `value` property with the sensitivity number. See `getMotionDetectors`.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.index | string | The name of a motion detector that is returned by the `getMotionDetectors` method |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.name | string | <sup>*(optional)*</sup> A sensitivity setting name |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.MotionDetection.1.getSensitivity",
    "params": {
        "index": "FP_MD"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "name": "low",
        "success": true
    }
}
```

<a name="method.isarmed"></a>
## *isarmed <sup>method</sup>*

Returns whether the specified motion detector is enabled.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.index | string | The name of a motion detector that is returned by the `getMotionDetectors` method |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.state | boolean | The motion detector is enabled (`true`) or disabled (`false`) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.MotionDetection.1.isarmed",
    "params": {
        "index": "FP_MD"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "state": true,
        "success": true
    }
}
```

<a name="method.setNoMotionPeriod"></a>
## *setNoMotionPeriod <sup>method</sup>*

Sets the no-motion period, in seconds, for the specified motion detector. When a motion detector is set to detect motion, this is the period of time, in seconds, that MUST elapse with no motion before a motion event is generated. If motion is detected within this period of time, then the time is reset and the countdown begins again. When a motion detector is set to detect no motion, then this is the period of time with no motion detected that MUST elapse before a no-motion event is generated.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.index | string | The name of a motion detector that is returned by the `getMotionDetectors` method |
| params.period | string | The no-motion period duration in seconds. The value must be greater than 0 |

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
    "method": "org.rdk.MotionDetection.1.setNoMotionPeriod",
    "params": {
        "index": "FP_MD",
        "period": "1800"
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

<a name="method.setSensitivity"></a>
## *setSensitivity <sup>method</sup>*

Sets the sensitivity of the sensor for the specified motion detector. The argument required depends on the supported sensitivity mode and can be one of:  
* `name`: Used when `sensitivityMode` is set to `2` requiring a sensitivity name.  
* `value`: Used when the `sensitivityMode` is set to `1` requiring a sensitivity number within a valid range.  
  
See `getMotionDetectors` to get the supported sensitivity mode.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.index | string | The name of a motion detector that is returned by the `getMotionDetectors` method |
| params?.name | string | <sup>*(optional)*</sup> A sensitivity setting name |

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
    "method": "org.rdk.MotionDetection.1.setSensitivity",
    "params": {
        "index": "FP_MD",
        "name": "low"
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

The following events are provided by the org.rdk.MotionDetection plugin:

MotionDetection interface events:

| Event | Description |
| :-------- | :-------- |
| [onMotionEvent](#event.onMotionEvent) | Triggered when a motion detector is enabled and either motion or no motion is detected (as indicated by the mode setting) |


<a name="event.onMotionEvent"></a>
## *onMotionEvent <sup>event</sup>*

Triggered when a motion detector is enabled and either motion or no motion is detected (as indicated by the mode setting).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.index | string | The name of a motion detector that is returned by the `getMotionDetectors` method |
| params.mode | string | The detection mode. Valid values are `0`: Detect no motion; or `1`: Detect motion (must be one of the following: *0*, *1*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onMotionEvent",
    "params": {
        "index": "FP_MD",
        "mode": "0"
    }
}
```

