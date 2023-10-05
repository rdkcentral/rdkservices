<!-- Generated automatically, DO NOT EDIT! -->
<a name="UserPreferences_Plugin"></a>
# UserPreferences Plugin

**Version: [1.0.1](https://github.com/rdkcentral/rdkservices/blob/main/UserPreferences/CHANGELOG.md)**

A org.rdk.UserPreferences plugin for Thunder framework.

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

The `UserPreferences` plugin controls user preference settings on a set-top device.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.UserPreferences*) |
| classname | string | Class name: *org.rdk.UserPreferences* |
| locator | string | Library name: *libWPEFrameworkUserPreferences.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.UserPreferences plugin:

UserPreferences interface methods:

| Method | Description |
| :-------- | :-------- |
| [getUILanguage](#getUILanguage) | Returns the preferred user interface language |
| [setUILanguage](#setUILanguage) | Sets the preferred user interface language |


<a name="getUILanguage"></a>
## *getUILanguage*

Returns the preferred user interface language.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.ui_language | string | The preferred user interface language. The language is written to the `/opt/user_preferences.conf` file on the device. It is the responsibility of the client application to validate the language value and process it if required. Any language string that is valid on the client can be set |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserPreferences.getUILanguage"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "ui_language": "US_en",
        "success": true
    }
}
```

<a name="setUILanguage"></a>
## *setUILanguage*

Sets the preferred user interface language.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ui_language | string | The preferred user interface language. The language is written to the `/opt/user_preferences.conf` file on the device. It is the responsibility of the client application to validate the language value and process it if required. Any language string that is valid on the client can be set |

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
    "method": "org.rdk.UserPreferences.setUILanguage",
    "params": {
        "ui_language": "US_en"
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

