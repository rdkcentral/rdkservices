<!-- Generated automatically, DO NOT EDIT! -->
<a name="MotionDetectionPlugin"></a>
# MotionDetectionPlugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A org.rdk.MotionDetection plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.MotionDetection plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `MotionDetection` plugin allows you to control the motion sensors that are available on a set-top box.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.MotionDetection*) |
| classname | string | Class name: *org.rdk.MotionDetection* |
| locator | string | Library name: *libWPEFrameworkMotionDetection.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.MotionDetection plugin:

MotionDetection interface methods:

| Method | Description |
| :-------- | :-------- |
| [arm](#arm) | Enables a motion detector in the mode requested |
| [disarm](#disarm) | Disables the specified motion detector |
| [getLastMotionEventElapsedTime](#getLastMotionEventElapsedTime) | Returns the elapsed time since the last motion event occurred for the specified motion detector |
| [getMotionDetectors](#getMotionDetectors) | Returns the available motion detectors and then lists information for each detector including their supported sensitivity mode |
| [getMotionEventsActivePeriod](#getMotionEventsActivePeriod) | Returns the configured times during the day when the motion sensor is active and detecting motion |
| [getNoMotionPeriod](#getNoMotionPeriod) | Returns the no-motion period for the specified motion detector |
| [getSensitivity](#getSensitivity) | Returns the current sensitivity configuration for the specified motion detector |
| [isarmed](#isarmed) | Returns whether the specified motion detector is enabled |
| [setMotionEventsActivePeriod](#setMotionEventsActivePeriod) | Sets the period of time during the day when the motion sensor is active and detecting motion |
| [setNoMotionPeriod](#setNoMotionPeriod) | Sets the no-motion period, in seconds, for the specified motion detector |
| [setSensitivity](#setSensitivity) | Sets the sensitivity of the sensor for the specified motion detector |


<a name="arm"></a>
## *arm*

Enables a motion detector in the mode requested. This enables a single shot  Once an event is sent, the device is in the disarmed state. If the application wishes to receive another event, then the application must re-arm.
 
### Events 

 No Events.

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
    "id": 42,
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
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="disarm"></a>
## *disarm*

Disables the specified motion detector.
 
### Events 

 No Events.

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
    "id": 42,
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
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="getLastMotionEventElapsedTime"></a>
## *getLastMotionEventElapsedTime*

Returns the elapsed time since the last motion event occurred for the specified motion detector.
 
### Events 

 No Events.

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
    "id": 42,
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
    "id": 42,
    "result": {
        "time": 1964,
        "success": true
    }
}
```

<a name="getMotionDetectors"></a>
## *getMotionDetectors*

Returns the available motion detectors and then lists information for each detector including their supported sensitivity mode.  
  
**Note:** The `sensitivityMode` property that is returned by this method indicates whether a number or a name controls the sensitivity of a motion detector. If `sensitivityMode` is `1`, then a set of properties (`min`, `max`, and `step`) are returned that define a valid number range to use. If `sensitivityMode` is `2`, then a `sensitivities` property is returned that contains valid names to use.
 
### Events 

 No Events.

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
| result.supportedMotionDetectorsInfo.FP_MD.sensitivityMode | integer | The supported sensitivity mode. Valid values are `0`: motion sensitivity cannot be changed, `1`: motion sensitivity can be set using an integer within the range defined by the `min`, `max`, and `step` properties, `2`: motion sensitivity can be set using the names listed in the `sensitivities`  (must be one of the following: *0*, *1*, *2*) |
| result.supportedMotionDetectorsInfo.FP_MD?.sensitivities | array | <sup>*(optional)*</sup> A string [] of sensitivity names that can be set for a motion detector. This property is only valid if the `SensitivityMode` property is set to `2` |
| result.supportedMotionDetectorsInfo.FP_MD?.sensitivities[#] | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.MotionDetection.1.getMotionDetectors"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
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

<a name="getMotionEventsActivePeriod"></a>
## *getMotionEventsActivePeriod*

Returns the configured times during the day when the motion sensor is active and detecting motion.
 
### Events 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.ranges | array | <sup>*(optional)*</sup> Active period time ranges. Setting a single range with startTime and endTime set to 0 disables the active period and allows the sensor to be armed 24 hours per day. In this case, the nowTime parameter is ignored |
| result?.ranges[#] | object | <sup>*(optional)*</sup>  |
| result?.ranges[#].startTime | integer | The start time of the active sensor period, in seconds, since midnight in local time |
| result?.ranges[#].endTime | integer | The end time of the active sensor period, in seconds, since midnight in local time |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.MotionDetection.1.getMotionEventsActivePeriod"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "ranges": [
            {
                "startTime": 21600,
                "endTime": 71000
            }
        ],
        "success": true
    }
}
```

<a name="getNoMotionPeriod"></a>
## *getNoMotionPeriod*

Returns the no-motion period for the specified motion detector.
 
### Events 

 No Events.

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
    "id": 42,
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
    "id": 42,
    "result": {
        "period": "1800",
        "success": true
    }
}
```

<a name="getSensitivity"></a>
## *getSensitivity*

Returns the current sensitivity configuration for the specified motion detector. The result is either a `name` property with the sensitivity name or a `value` property with the sensitivity number. See `getMotionDetectors`.
 
### Events 

 No Events.

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
    "id": 42,
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
    "id": 42,
    "result": {
        "name": "low",
        "success": true
    }
}
```

<a name="isarmed"></a>
## *isarmed*

Returns whether the specified motion detector is enabled.
 
### Events 

 No Events.

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
    "id": 42,
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
    "id": 42,
    "result": {
        "state": true,
        "success": true
    }
}
```

<a name="setMotionEventsActivePeriod"></a>
## *setMotionEventsActivePeriod*

Sets the period of time during the day when the motion sensor is active and detecting motion. Any motion notifications outside of this period should be deferred until the start of the active period or cancelled if the notification is no longer valid. If this method is not called, then the active period is considered disabled and the sensor is armed 24 hours per day.  
**Note:** The start time may be a higher value than the end time (for example, when a configured activation period spans across midnight from 09:00 pm to 01:00 am). Also, Daylight savings time (DST) may apply to the time zone where this feature is being used and the caller should be aware of the 23 hour and 25 hour days which occur during the shift days. For this reason it is advised that the caller reprograms the active period the day before and the day after the shift days to ensure reliable operation. If the caller is reprogramming this value every 24 hours then this should not be an issue.
 
### Events 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.index | string | The name of a motion detector that is returned by the `getMotionDetectors` method |
| params.nowTime | integer | The present time, in seconds, since midnight in local time |
| params.ranges | array | Active period time ranges. Setting a single range with startTime and endTime set to 0 disables the active period and allows the sensor to be armed 24 hours per day. In this case, the nowTime parameter is ignored |
| params.ranges[#] | object |  |
| params.ranges[#].startTime | integer | The start time of the active sensor period, in seconds, since midnight in local time |
| params.ranges[#].endTime | integer | The end time of the active sensor period, in seconds, since midnight in local time |

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
    "method": "org.rdk.MotionDetection.1.setMotionEventsActivePeriod",
    "params": {
        "index": "FP_MD",
        "nowTime": 1234,
        "ranges": [
            {
                "startTime": 21600,
                "endTime": 71000
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

<a name="setNoMotionPeriod"></a>
## *setNoMotionPeriod*

Sets the no-motion period, in seconds, for the specified motion detector. When a motion detector is set to detect motion, this is the period of time, in seconds, that MUST elapse with no motion before a motion event is generated. If motion is detected within this period of time, then the time is reset and the countdown begins again. When a motion detector is set to detect no motion, then this is the period of time with no motion detected that MUST elapse before a no-motion event is generated.
 
### Events 

 No Events.

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
    "id": 42,
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
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="setSensitivity"></a>
## *setSensitivity*

Sets the sensitivity of the sensor for the specified motion detector. The argument required depends on the supported sensitivity mode and can be one of:  
* `name`: Used when `sensitivityMode` is set to `2` requiring a sensitivity name.  
* `value`: Used when the `sensitivityMode` is set to `1` requiring a sensitivity number within a valid range.  
  
See `getMotionDetectors` to get the supported sensitivity mode.
 
### Events 

 No Events.

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
    "id": 42,
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
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.MotionDetection plugin:

MotionDetection interface events:

| Event | Description |
| :-------- | :-------- |
| [onMotionEvent](#onMotionEvent) | Triggered when a motion detector is enabled and either motion or no motion is detected (as indicated by the mode setting) |


<a name="onMotionEvent"></a>
## *onMotionEvent*

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

