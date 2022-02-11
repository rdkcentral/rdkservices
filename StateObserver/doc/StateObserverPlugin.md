<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.StateObserver_Plugin"></a>
# StateObserver Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A com.comcast.StateObserver plugin for Thunder framework.

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

This document describes purpose and functionality of the com.comcast.StateObserver plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `StateObserver` plugin is used to monitor property values.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *com.comcast.StateObserver*) |
| classname | string | Class name: *com.comcast.StateObserver* |
| locator | string | Library name: *libWPEFrameworkStateObserver.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the com.comcast.StateObserver plugin:

StateObserver interface methods:

| Method | Description |
| :-------- | :-------- |
| [getApiVersionNumber](#method.getApiVersionNumber) | Returns the API version number |
| [getName](#method.getName) | Returns the plugin name |
| [getRegisteredPropertyNames](#method.getRegisteredPropertyNames) | Returns all properties which have active listeners |
| [getValues](#method.getValues) | Returns the values and errors for the specified properties |
| [registerListeners](#method.registerListeners) | Register a listener on the specified properties for value change notifications |
| [setApiVersionNumber](#method.setApiVersionNumber) | Sets the API version number |
| [unregisterListeners](#method.unregisterListeners) | Removes the listeners on the specified properties |


<a name="method.getApiVersionNumber"></a>
## *getApiVersionNumber [<sup>method</sup>](#head.Methods)*

Returns the API version number.
 
### Events 

 No Events.

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
    "method": "com.comcast.StateObserver.1.getApiVersionNumber"
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

<a name="method.getName"></a>
## *getName [<sup>method</sup>](#head.Methods)*

Returns the plugin name.
 
### Events 

 No Events.

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
    "method": "com.comcast.StateObserver.1.getName"
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

<a name="method.getRegisteredPropertyNames"></a>
## *getRegisteredPropertyNames [<sup>method</sup>](#head.Methods)*

Returns all properties which have active listeners.
 
### Events 

 No Events.

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
    "method": "com.comcast.StateObserver.1.getRegisteredPropertyNames"
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

<a name="method.getValues"></a>
## *getValues [<sup>method</sup>](#head.Methods)*

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

 No Events.

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
    "method": "com.comcast.StateObserver.1.getValues",
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

<a name="method.registerListeners"></a>
## *registerListeners [<sup>method</sup>](#head.Methods)*

Register a listener on the specified properties for value change notifications. These properties are added to a registered properties list. Internally, this method calls the `getValues` method and hence it returns the current value of those properties.
 
### Events 

 No Events.

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
    "method": "com.comcast.StateObserver.1.registerListeners",
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

<a name="method.setApiVersionNumber"></a>
## *setApiVersionNumber [<sup>method</sup>](#head.Methods)*

Sets the API version number.
 
### Events 

 No Events.

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
    "method": "com.comcast.StateObserver.1.setApiVersionNumber",
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

<a name="method.unregisterListeners"></a>
## *unregisterListeners [<sup>method</sup>](#head.Methods)*

Removes the listeners on the specified properties. The properties are removed from the registered properties list.
 
### Events 

 No Events.

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
    "method": "com.comcast.StateObserver.1.unregisterListeners",
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

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the com.comcast.StateObserver plugin:

StateObserver interface events:

| Event | Description |
| :-------- | :-------- |
| [propertyChanged](#event.propertyChanged) | Triggered whenever a device property value changes |


<a name="event.propertyChanged"></a>
## *propertyChanged [<sup>event</sup>](#head.Notifications)*

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
    "method": "client.events.1.propertyChanged",
    "params": {
        "propertyName": "com.comcast.channel_map",
        "value": 2,
        "error": 0
    }
}
```

