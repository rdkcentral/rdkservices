<!-- Generated automatically, DO NOT EDIT! -->
<a name="Timer_Plugin"></a>
# Timer Plugin

**Version: [1.0.3](https://github.com/rdkcentral/rdkservices/blob/main/Timer/CHANGELOG.md)**

A org.rdk.Timer plugin for Thunder framework.

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

The `Timer` plugin controls timer execution. It allows you to start, suspend, resume timers and receive timer notifications.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.Timer*) |
| classname | string | Class name: *org.rdk.Timer* |
| locator | string | Library name: *libWPEFrameworkTimer.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.Timer plugin:

Timer interface methods:

| Method | Description |
| :-------- | :-------- |
| [cancel](#cancel) | Stops the specified timer |
| [getTimers](#getTimers) | Gets the status of all timers |
| [getTimerStatus](#getTimerStatus) | Gets the status of the specified timer |
| [resume](#resume) | Resumes the specified timer |
| [startTimer](#startTimer) | Starts a timer with the specified interval |
| [suspend](#suspend) | Suspends the specified timer |


<a name="cancel"></a>
## *cancel*

Stops the specified timer.

### Events

No Events

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
    "method": "org.rdk.Timer.cancel",
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

<a name="getTimers"></a>
## *getTimers*

Gets the status of all timers.

### Events

No Events

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
    "method": "org.rdk.Timer.getTimers"
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

<a name="getTimerStatus"></a>
## *getTimerStatus*

Gets the status of the specified timer.

### Events

No Events

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
    "method": "org.rdk.Timer.getTimerStatus",
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

<a name="resume"></a>
## *resume*

Resumes the specified timer.

### Events

No Events

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
    "method": "org.rdk.Timer.resume",
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

<a name="startTimer"></a>
## *startTimer*

Starts a timer with the specified interval. After the timer expires, a `timerExpired `notification is sent. The timer can execute once (one-shot mode) or repeatedly.

### Events

| Event | Description |
| :-------- | :-------- |
| [timerExpired](#timerExpired) | Triggered when a timer expires |
| [timerExpiryReminder](#timerExpiryReminder) | Triggered to remind that, the timer will expire in remindBefore value in seconds |
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
    "method": "org.rdk.Timer.startTimer",
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

<a name="suspend"></a>
## *suspend*

Suspends the specified timer.

### Events

No Events

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
    "method": "org.rdk.Timer.suspend",
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

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.Timer plugin:

Timer interface events:

| Event | Description |
| :-------- | :-------- |
| [timerExpired](#timerExpired) | Triggered when a timer expires |
| [timerExpiryReminder](#timerExpiryReminder) | Triggered before a timer actually expires |


<a name="timerExpired"></a>
## *timerExpired*

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
    "method": "client.events.timerExpired",
    "params": {
        "timerId": 0,
        "mode": "WAKE",
        "status": 0
    }
}
```

<a name="timerExpiryReminder"></a>
## *timerExpiryReminder*

Triggered before a timer actually expires. It is triggered only when a non-zero remindBefore parameter is passed in the `startTimer` 

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
    "method": "client.events.timerExpiryReminder",
    "params": {
        "timerId": 0,
        "mode": "WAKE",
        "timeRemaining": 0
    }
}
```

