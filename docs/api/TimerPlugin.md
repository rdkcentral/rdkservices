<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.Timer_Plugin"></a>
# Timer Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A org.rdk.Timer plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.Timer plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `Timer` plugin controls timer execution. It allows you to start, suspend, resume timers and receive timer notifications.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.Timer*) |
| classname | string | Class name: *org.rdk.Timer* |
| locator | string | Library name: *libWPEFrameworkTimer.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.Timer plugin:

Timer interface methods:

| Method | Description |
| :-------- | :-------- |
| [cancel](#method.cancel) | Stops the specified timer |
| [getTimers](#method.getTimers) | Gets the status of all timers |
| [getTimerStatus](#method.getTimerStatus) | Gets the status of the specified timer |
| [resume](#method.resume) | Resumes the specified timer |
| [startTimer](#method.startTimer) | Starts a timer with the specified interval |
| [suspend](#method.suspend) | Suspends the specified timer |


<a name="method.cancel"></a>
## *cancel [<sup>method</sup>](#head.Methods)*

Stops the specified timer.
 
### Events
 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.timerID | integer | The timer ID |

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
    "method": "org.rdk.Timer.1.cancel",
    "params": {
        "timerID": 0
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

<a name="method.getTimers"></a>
## *getTimers [<sup>method</sup>](#head.Methods)*

Gets the status of all timers.
 
### Events
 
No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.timers | array | An [] of timers and their information |
| result.timers[#] | object |  |
| result.timers[#]?.timerID | integer | <sup>*(optional)*</sup> The timer ID |
| result.timers[#].state | string | The state of the timer (must be one of the following: *RUNNING*, *SUSPENDED*, *CANCELED*, *EXPIRED*) |
| result.timers[#].mode | string | The mode of the timer (must be one of the following: *GENERIC*, *SLEEP*, *WAKE*) |
| result.timers[#].timeRemaining | string | The time remaining, in seconds, until expiration |
| result.timers[#].repeatInterval | string | The repeat interval in seconds. See `startTimer` for more details |
| result.timers[#].remindBefore | string | The send notification expiration timeout in seconds. See `startTimer` for more details |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Timer.1.getTimers"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "timers": [
            {
                "timerID": 0,
                "state": "RUNNING",
                "mode": "WAKE",
                "timeRemaining": "1894.646",
                "repeatInterval": "2000.000",
                "remindBefore": "1000.000"
            }
        ],
        "success": true
    }
}
```

<a name="method.getTimerStatus"></a>
## *getTimerStatus [<sup>method</sup>](#head.Methods)*

Gets the status of the specified timer.
 
### Events
 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.timerId | integer | The timer ID |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.state | string | The state of the timer (must be one of the following: *RUNNING*, *SUSPENDED*, *CANCELED*, *EXPIRED*) |
| result.mode | string | The mode of the timer (must be one of the following: *GENERIC*, *SLEEP*, *WAKE*) |
| result.timeRemaining | string | The time remaining, in seconds, until expiration |
| result.repeatInterval | string | The repeat interval in seconds. See `startTimer` for more details |
| result.remindBefore | string | The send notification expiration timeout in seconds. See `startTimer` for more details |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Timer.1.getTimerStatus",
    "params": {
        "timerId": 0
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "state": "RUNNING",
        "mode": "WAKE",
        "timeRemaining": "1894.646",
        "repeatInterval": "2000.000",
        "remindBefore": "1000.000",
        "success": true
    }
}
```

<a name="method.resume"></a>
## *resume [<sup>method</sup>](#head.Methods)*

Resumes the specified timer.
 
### Events
 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.timerID | integer | The timer ID |

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
    "method": "org.rdk.Timer.1.resume",
    "params": {
        "timerID": 0
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

<a name="method.startTimer"></a>
## *startTimer [<sup>method</sup>](#head.Methods)*

Starts a timer with the specified interval. After the timer expires, a `timerExpired `notification is sent. The timer can execute once (one-shot mode) or repeatedly.
 
### Events
 
| Event | Description | 
| :-------- | :-------- | 
| `timerExpired` | Triggered when a timer expires | 
| `timerExpiryReminder` | Triggered to remind that, the timer will expire in remindBefore value in seconds |.

Also see: [timerExpired](#event.timerExpired), [timerExpiryReminder](#event.timerExpiryReminder)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.interval | number | The time interval in seconds |
| params?.mode | string | <sup>*(optional)*</sup> The mode of the timer (must be one of the following: *GENERIC*, *SLEEP*, *WAKE*) |
| params?.repeatInterval | number | <sup>*(optional)*</sup> The repeat interval in seconds. For non-zero values, this is the period of subsequent timers execution. If `0.0` or not provided, then the timer executes once (one-shot mode) |
| params?.remindBefore | number | <sup>*(optional)*</sup> The send notification expiration timeout in seconds. A `timerExpiryReminder` notification is sent if the duration to the timer expiration is less than the `remindBefore` value. After sending the notification, an internal flag is set and the reminder is not sent again for the given timer interval. If the value is 0.0 or not provided, then the reminder is not sent |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.timerId | integer | The timer ID |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Timer.1.startTimer",
    "params": {
        "interval": 1.2,
        "mode": "WAKE",
        "repeatInterval": 1.4,
        "remindBefore": 0.2
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "timerId": 0,
        "success": true
    }
}
```

<a name="method.suspend"></a>
## *suspend [<sup>method</sup>](#head.Methods)*

Suspends the specified timer.
 
### Events
 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.timerID | integer | The timer ID |

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
    "method": "org.rdk.Timer.1.suspend",
    "params": {
        "timerID": 0
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

The following events are provided by the org.rdk.Timer plugin:

Timer interface events:

| Event | Description |
| :-------- | :-------- |
| [timerExpired](#event.timerExpired) | Triggered when a timer expires |
| [timerExpiryReminder](#event.timerExpiryReminder) | Triggered before a timer actually expires |


<a name="event.timerExpired"></a>
## *timerExpired [<sup>event</sup>](#head.Notifications)*

Triggered when a timer expires.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.timerId | integer | The timer ID |
| params.mode | string | The mode of the timer (must be one of the following: *GENERIC*, *SLEEP*, *WAKE*) |
| params.status | integer | The timer status |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.timerExpired",
    "params": {
        "timerId": 0,
        "mode": "WAKE",
        "status": 0
    }
}
```

<a name="event.timerExpiryReminder"></a>
## *timerExpiryReminder [<sup>event</sup>](#head.Notifications)*

Triggered before a timer actually expires. It is triggered only when a non-zero remindBefore parameter is passed in the `startTimer` method.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.timerId | integer | The timer ID |
| params.mode | string | The mode of the timer (must be one of the following: *GENERIC*, *SLEEP*, *WAKE*) |
| params.timeRemaining | integer | The time remaining, in seconds, until expiration |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.timerExpiryReminder",
    "params": {
        "timerId": 0,
        "mode": "WAKE",
        "timeRemaining": 0
    }
}
```

