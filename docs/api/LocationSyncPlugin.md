<!-- Generated automatically, DO NOT EDIT! -->
<a name="Location_Sync_Plugin"></a>
# Location Sync Plugin

**Version: [1.0.1](https://github.com/rdkcentral/rdkservices/blob/main/LocationSync/CHANGELOG.md)**

A LocationSync plugin for Thunder framework.

### Table of Contents

- [Abbreviation, Acronyms and Terms](#Abbreviation,_Acronyms_and_Terms)
- [Description](#Description)
- [Configuration](#Configuration)
- [Methods](#Methods)
- [Properties](#Properties)
- [Notifications](#Notifications)

<a name="Abbreviation,_Acronyms_and_Terms"></a>
# Abbreviation, Acronyms and Terms

[[Refer to this link](userguide/aat.md)]

<a name="Description"></a>
# Description

The `LocationSync` plugin provides geo-location functionality.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *LocationSync*) |
| classname | string | Class name: *LocationSync* |
| locator | string | Library name: *libWPELocationSync.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |
| configuration | object | <sup>*(optional)*</sup>  |
| configuration?.interval | number | <sup>*(optional)*</sup> Maximum time duration between each request to the Location Server (default: 10) |
| configuration?.retries | number | <sup>*(optional)*</sup> Maximum number of request reties to the Location Server (default:20) |
| configuration?.source | string | <sup>*(optional)*</sup> URI of the Location Server (default:"location.webplatformforembedded.org") |

<a name="Methods"></a>
# Methods

The following methods are provided by the LocationSync plugin:

LocationSync interface methods:

| Method | Description |
| :-------- | :-------- |
| [sync](#sync) | Synchronizes the location |


<a name="sync"></a>
## *sync*

Synchronizes the location.

### Events

| Event | Description |
| :-------- | :-------- |
| [locationchange](#locationchange) | Signals the change of location |
### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | null | Always null |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | Failed to synchdonize the location |
| 2 | ```ERROR_UNAVAILABLE``` | Unavailable locator |
| 15 | ```ERROR_INCORRECT_URL``` | Incorrect URL |
| 12 | ```ERROR_INPROGRESS``` | Probing in progress |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "LocationSync.sync"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": null
}
```

<a name="Properties"></a>
# Properties

The following properties are provided by the LocationSync plugin:

LocationSync interface properties:

| Property | Description |
| :-------- | :-------- |
| [location](#location) <sup>RO</sup> | Location information |


<a name="location"></a>
## *location*

Provides access to the location information.

> This property is **read-only**.

### Events

No Events

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Location information |
| (property).city | string | City name |
| (property).country | string | Country name |
| (property).region | string | Region name |
| (property).timezone | string | Time zone information |
| (property).publicip | string | Public IP |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "LocationSync.location"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "city": "Wroclaw",
        "country": "Poland",
        "region": "Wroclaw",
        "timezone": "CET-1CEST,M3.5.0,M10.5.0/3",
        "publicip": "78.11.117.118"
    }
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the LocationSync plugin:

LocationSync interface events:

| Event | Description |
| :-------- | :-------- |
| [locationchange](#locationchange) | Signals a location change |


<a name="locationchange"></a>
## *locationchange*

Signals a location change.

### Parameters

This event carries no parameters.

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.locationchange"
}
```

