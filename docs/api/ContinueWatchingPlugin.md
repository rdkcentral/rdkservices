<!-- Generated automatically, DO NOT EDIT! -->
<a name="ContinueWatching_Plugin"></a>
# ContinueWatching Plugin

**Version: [1.0.3](https://github.com/rdkcentral/rdkservices/blob/main/ContinueWatching/CHANGELOG.md)**

A org.rdk.ContinueWatching plugin for Thunder framework.

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

Initial version of the `ContinueWatching` API.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.ContinueWatching*) |
| classname | string | Class name: *org.rdk.ContinueWatching* |
| locator | string | Library name: *libWPEFrameworkContinueWatching.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.ContinueWatching plugin:

ContinueWatching interface methods:

| Method | Description |
| :-------- | :-------- |
| [deleteApplicationToken](#deleteApplicationToken) | Deletes the stored tokens for a particular application |
| [getApplicationToken](#getApplicationToken) | Retrieves the tokens for a particular application |
| [setApplicationToken](#setApplicationToken) | Sets the given tokens for an application |


<a name="deleteApplicationToken"></a>
## *deleteApplicationToken*

Deletes the stored tokens for a particular application.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.applicationName | string | The application name |

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
    "method": "org.rdk.ContinueWatching.deleteApplicationToken",
    "params": {
        "applicationName": "netflix"
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

<a name="getApplicationToken"></a>
## *getApplicationToken*

Retrieves the tokens for a particular application.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.applicationName | string | The application name |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.application_token | array | A string [] containing tokens. If no tokens are set for the application, then an empty array returns |
| result.application_token[#] | object |  |
| result.application_token[#]?.ABC | string | <sup>*(optional)*</sup> A token value |
| result.application_token[#]?.DEF | string | <sup>*(optional)*</sup> A token value |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.ContinueWatching.getApplicationToken",
    "params": {
        "applicationName": "netflix"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "application_token": [
            {
                "ABC": "xyz",
                "DEF": "uvw"
            }
        ],
        "success": true
    }
}
```

<a name="setApplicationToken"></a>
## *setApplicationToken*

Sets the given tokens for an application.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.applicationName | string | The application name |
| params.application_token | object | Any number of tokens to be set |
| params.application_token?.ABC | string | <sup>*(optional)*</sup> A token value |
| params.application_token?.DEF | string | <sup>*(optional)*</sup> A token value |

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
    "method": "org.rdk.ContinueWatching.setApplicationToken",
    "params": {
        "applicationName": "netflix",
        "application_token": {
            "ABC": "xyz",
            "DEF": "uvw"
        }
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

