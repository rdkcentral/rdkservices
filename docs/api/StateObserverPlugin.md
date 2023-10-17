<!-- Generated automatically, DO NOT EDIT! -->
<a name="StateObserver_Plugin"></a>
# StateObserver Plugin

**Version: [1.0.3](https://github.com/rdkcentral/rdkservices/blob/main/StateObserver/CHANGELOG.md)**

A com.comcast.StateObserver plugin for Thunder framework.

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

The `StateObserver` plugin is used to monitor property values.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *com.comcast.StateObserver*) |
| classname | string | Class name: *com.comcast.StateObserver* |
| locator | string | Library name: *libWPEFrameworkStateObserver.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the com.comcast.StateObserver plugin:

StateObserver interface methods:

| Method | Description |
| :-------- | :-------- |
| [getApiVersionNumber](#getApiVersionNumber) | Returns the API version number |
| [getName](#getName) | Returns the plugin name |
| [getRegisteredPropertyNames](#getRegisteredPropertyNames) | Returns all properties which have active listeners |
| [getValues](#getValues) | Returns the values and errors for the specified properties |
| [registerListeners](#registerListeners) | Register a listener on the specified properties for value change notifications |
| [setApiVersionNumber](#setApiVersionNumber) | Sets the API version number |
| [unregisterListeners](#unregisterListeners) | Removes the listeners on the specified properties |


<a name="getApiVersionNumber"></a>
## *getApiVersionNumber*

Returns the API version number.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.version | integer | The API version number |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "com.comcast.StateObserver.getApiVersionNumber"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "version": 1,
        "success": true
    }
}
```

<a name="getName"></a>
## *getName*

Returns the plugin name.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.name | string | The plugin name |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "com.comcast.StateObserver.getName"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "name": "com.comcast.StateObserver",
        "success": true
    }
}
```

<a name="getRegisteredPropertyNames"></a>
## *getRegisteredPropertyNames*

Returns all properties which have active listeners.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.properties | array | A string [] of properties |
| result.properties[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "com.comcast.StateObserver.getRegisteredPropertyNames"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "properties": [
            "com.comcast.channel_map"
        ],
        "success": true
    }
}
```

<a name="getValues"></a>
## *getValues*

Returns the values and errors for the specified properties.  
**Error Code of Properties**  
* `com.comcast.channel_map` - RDK-03005  
* `com.comcast.card.disconnected` - RDK-03007  
* `com.comcast.cmac` - RDK-03002  
* `com.comcast.time_source` - RDK-03006  
* `com.comcast.estb_ip` - RDK-03009  
* `com.comcast.ecm_ip` - RDK-03004  
* `com.comcast.dsg_ca_tunnel` - RDK-03003  
* `com.comcast.cable_card` - RDK-03001.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.propertyNames | array | The fully qualified property name |
| params.propertyNames[#] | string |  |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.properties | array | The requested properties and respective values |
| result.properties[#] | object |  |
| result.properties[#].propertyName | string | The fully qualified property name |
| result.properties[#].value | integer | The property value |
| result.properties[#].error | string | The error state |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "com.comcast.StateObserver.getValues",
    "params": {
        "propertyNames": [
            "com.comcast.channel_map"
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
        "properties": [
            {
                "propertyName": "com.comcast.channel_map",
                "value": 2,
                "error": "none"
            }
        ],
        "success": true
    }
}
```

<a name="registerListeners"></a>
## *registerListeners*

Register a listener on the specified properties for value change notifications. These properties are added to a registered properties list. Internally, this method calls the `getValues` method and hence it returns the current value of those properties.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.propertyNames | array | The fully qualified property name |
| params.propertyNames[#] | string |  |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.properties | array | The requested properties and respective values |
| result.properties[#] | object |  |
| result.properties[#].propertyName | string | The fully qualified property name |
| result.properties[#].value | integer | The property value |
| result.properties[#].error | string | The error state |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "com.comcast.StateObserver.registerListeners",
    "params": {
        "propertyNames": [
            "com.comcast.channel_map"
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
        "properties": [
            {
                "propertyName": "com.comcast.channel_map",
                "value": 2,
                "error": "none"
            }
        ],
        "success": true
    }
}
```

<a name="setApiVersionNumber"></a>
## *setApiVersionNumber*

Sets the API version number.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.version | integer | The API version number |

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
    "method": "com.comcast.StateObserver.setApiVersionNumber",
    "params": {
        "version": 1
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

<a name="unregisterListeners"></a>
## *unregisterListeners*

Removes the listeners on the specified properties. The properties are removed from the registered properties list.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.propertyNames | array | The fully qualified property name |
| params.propertyNames[#] | string |  |

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
    "method": "com.comcast.StateObserver.unregisterListeners",
    "params": {
        "propertyNames": [
            "com.comcast.channel_map"
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

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the com.comcast.StateObserver plugin:

StateObserver interface events:

| Event | Description |
| :-------- | :-------- |
| [propertyChanged](#propertyChanged) | Triggered whenever a device property value changes |


<a name="propertyChanged"></a>
## *propertyChanged*

Triggered whenever a device property value changes. A handler function is called which then sends out a notification about the change.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.propertyName | string | The fully qualified property name |
| params.value | integer | The property value |
| params.error | integer | The error state |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.propertyChanged",
    "params": {
        "propertyName": "com.comcast.channel_map",
        "value": 2,
        "error": 0
    }
}
```

