<!-- Generated automatically, DO NOT EDIT! -->
<a name="ResourceManager_Plugin"></a>
# ResourceManager Plugin

**Version: [1.0.2](https://github.com/rdkcentral/rdkservices/blob/main/ResourceManager/CHANGELOG.md)**

A org.rdk.ResourceManager plugin for Thunder framework.

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

The `ResourceManager`plugin controls the management of platform resource reservations.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.ResourceManager*) |
| classname | string | Class name: *org.rdk.ResourceManager* |
| locator | string | Library name: *libWPEFrameworkResourceManager.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.ResourceManager plugin:

ResourceManager interface methods:

| Method | Description |
| :-------- | :-------- |
| [setAVBlocked](#setAVBlocked) | adds/removes the list of applications with the given callsigns to/from the blacklist |
| [getBlockedAVApplications](#getBlockedAVApplications) | Gets a list of blacklisted clients |
| [reserveTTSResource](#reserveTTSResource) | Reserves the Text To speech Resource for specified client |


<a name="setAVBlocked"></a>
## *setAVBlocked*

adds/removes the list of applications with the given callsigns to/from the blacklist.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.appId | string | The application callsign |
| params.blocked | boolean | Whether to block (`true`) or unblock (`false`) AV for the callsign |

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
    "method": "org.rdk.ResourceManager.setAVBlocked",
    "params": {
        "appId": "searchanddiscovery",
        "blocked": true
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

<a name="getBlockedAVApplications"></a>
## *getBlockedAVApplications*

Gets a list of blacklisted clients.

### Events

No Events

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
    "method": "org.rdk.ResourceManager.getBlockedAVApplications"
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

<a name="reserveTTSResource"></a>
## *reserveTTSResource*

Reserves the Text To speech Resource for specified client.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.appId | string | The application callsign |

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
    "method": "org.rdk.ResourceManager.reserveTTSResource",
    "params": {
        "appId": "xumo"
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

