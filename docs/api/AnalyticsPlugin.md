<!-- Generated automatically, DO NOT EDIT! -->
<a name="Analytics_Plugin"></a>
# Analytics Plugin

**Version: [1.0.4](https://github.com/rdkcentral/rdkservices/blob/main/Analytics/CHANGELOG.md)**

A org.rdk.Analytics plugin for Thunder framework.

### Table of Contents

- [Abbreviation, Acronyms and Terms](#Abbreviation,_Acronyms_and_Terms)
- [Description](#Description)
- [Configuration](#Configuration)
- [Methods](#Methods)

<a name="Abbreviation,_Acronyms_and_Terms"></a>
# Abbreviation, Acronyms and Terms

[[Refer to this link](userguide/aat.md)]

<a name="Description"></a>
# Description

The `Analytics` plugin allows to send analytics events to dedicated backends.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.Analytics*) |
| classname | string | Class name: *org.rdk.Analytics* |
| locator | string | Library name: *libWPEFrameworkAnalytics.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |
| configuration | object |  |
| configuration.loggername | string | Logger name used by backend |
| configuration.loggerversion | string | Logger version used by backend |
| configuration?.eventsmap | string | <sup>*(optional)*</sup> Optional path to json file with array of mapped events name |
| configuration.backendlib | string | Name of backend library |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.Analytics plugin:

Analytics interface methods:

| Method | Description |
| :-------- | :-------- |
| [sendEvent](#sendEvent) | Enqueue an event to be sent to the SIFT analytics backend |


<a name="sendEvent"></a>
## *sendEvent*

Enqueue an event to be sent to the SIFT analytics backend.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.eventName | string | Name of the event |
| params?.eventVersion | string | <sup>*(optional)*</sup> Version number of event schema |
| params.eventSource | string | Name of the component that originates the event (Durable App ID if an App) |
| params.eventSourceVersion | string | Version number for the component that originates the event |
| params.cetList | array | An array of Capability Exclusion Tags to be included on the report. Each CET will exclude the event from being processed for the specified process, any may result in the event being dropped. May be an array of length zero |
| params.cetList[#] | string |  |
| params?.epochTimestamp | integer | <sup>*(optional)*</sup> Timestamp for the START of this event, epoch time, in ms UTC |
| params?.uptimeTimestamp | integer | <sup>*(optional)*</sup> Timestamp for the START of this event, uptime of the device, in ms. ONLY to be used when Time quality is not good |
| params?.appId | string | <sup>*(optional)*</sup> Durable App ID string |
| params.eventPayload | object | Custom payload of the event in JSON format. User defined colection of objects and keys. May be an empty object |
| params.eventPayload.keyOrObject | string | User defined custom key or object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | On success null will be returned |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Analytics.sendEvent",
    "params": {
        "eventName": "app_summary",
        "eventVersion": "1.0.0",
        "eventSource": "epg",
        "eventSourceVersion": "1.0.0",
        "cetList": [
            "cet1"
        ],
        "epochTimestamp": 1721906631000,
        "uptimeTimestamp": 35000,
        "appId": "app-id-app1",
        "eventPayload": {
            "keyOrObject": "value1"
        }
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "null"
}
```

