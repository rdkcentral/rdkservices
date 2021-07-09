<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.PersistentStore_Plugin"></a>
# PersistentStore Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

org.rdk.PersistentStore plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.PersistentStore plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `PersistentStore` plugin allows you to persist key/value pairs by namespace.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.PersistentStore*) |
| classname | string | Class name: *org.rdk.PersistentStore* |
| locator | string | Library name: *libWPEFrameworkPersistentStore.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.PersistentStore plugin:

PersistentStore interface methods:

| Method | Description |
| :-------- | :-------- |
| [deleteKey](#method.deleteKey) | Deletes a key from the specified namespace |
| [deleteNamespace](#method.deleteNamespace) | Deletes the specified namespace |
| [flushCache](#method.flushCache) | Flushes the database cache by invoking `flush` in SQLite |
| [getKeys](#method.getKeys) | Returns the keys that are stored in the specified namespace |
| [getNamespaces](#method.getNamespaces) | Returns the namespaces in the datastore |
| [getStorageSize](#method.getStorageSize) | Returns the size occupied by each namespace |
| [getValue](#method.getValue) | Returns the value of a key from the specified namespace |
| [setValue](#method.setValue) | Sets the value of a key in the the specified namespace |


<a name="method.deleteKey"></a>
## *deleteKey <sup>method</sup>*

Deletes a key from the specified namespace.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.namespace | string | A namespace in the datastore as a valid UTF-8 string |
| params.key | string | The key name as a valid UTF-8 string |

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
    "id": 1234567890,
    "method": "org.rdk.PersistentStore.1.deleteKey",
    "params": {
        "namespace": "ns1",
        "key": "key1"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.deleteNamespace"></a>
## *deleteNamespace <sup>method</sup>*

Deletes the specified namespace.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.namespace | string | A namespace in the datastore as a valid UTF-8 string |

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
    "id": 1234567890,
    "method": "org.rdk.PersistentStore.1.deleteNamespace",
    "params": {
        "namespace": "ns1"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.flushCache"></a>
## *flushCache <sup>method</sup>*

Flushes the database cache by invoking `flush` in SQLite.

### Parameters

This method takes no parameters.

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
    "id": 1234567890,
    "method": "org.rdk.PersistentStore.1.flushCache"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.getKeys"></a>
## *getKeys <sup>method</sup>*

Returns the keys that are stored in the specified namespace.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.namespace | string | A namespace in the datastore as a valid UTF-8 string |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.keys | array | A list of keys |
| result.keys[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.PersistentStore.1.getKeys",
    "params": {
        "namespace": "ns1"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "keys": [
            "key1"
        ],
        "success": true
    }
}
```

<a name="method.getNamespaces"></a>
## *getNamespaces <sup>method</sup>*

Returns the namespaces in the datastore.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.namespaces | array | A list of namespaces |
| result.namespaces[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.PersistentStore.1.getNamespaces"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "namespaces": [
            "ns1"
        ],
        "success": true
    }
}
```

<a name="method.getStorageSize"></a>
## *getStorageSize <sup>method</sup>*

Returns the size occupied by each namespace. This is a processing-intense operation. The total size of the datastore should not exceed more than 1MB in size. If the storage size is exceeded, then new values are not stored and the `onStorageExceeded` event is sent.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.namespaceSizes | object | The namespaces and their respective size |
| result.namespaceSizes?.ns1 | integer | <sup>*(optional)*</sup>  |
| result.namespaceSizes?.ns2 | integer | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.PersistentStore.1.getStorageSize"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "namespaceSizes": {
            "ns1": 534,
            "ns2": 234
        },
        "success": true
    }
}
```

<a name="method.getValue"></a>
## *getValue <sup>method</sup>*

Returns the value of a key from the specified namespace.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.namespace | string | A namespace in the datastore as a valid UTF-8 string |
| params.key | string | The key name as a valid UTF-8 string |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.value | string | The key value. Values are capped at 1000 characters in size |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.PersistentStore.1.getValue",
    "params": {
        "namespace": "ns1",
        "key": "key1"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "value": "value1",
        "success": true
    }
}
```

<a name="method.setValue"></a>
## *setValue <sup>method</sup>*

Sets the value of a key in the the specified namespace.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.namespace | string | A namespace in the datastore as a valid UTF-8 string |
| params.key | string | The key name as a valid UTF-8 string |
| params.value | string | The key value. Values are capped at 1000 characters in size |

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
    "id": 1234567890,
    "method": "org.rdk.PersistentStore.1.setValue",
    "params": {
        "namespace": "ns1",
        "key": "key1",
        "value": "value1"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.PersistentStore plugin:

PersistentStore interface events:

| Event | Description |
| :-------- | :-------- |
| [onStorageExceeded](#event.onStorageExceeded) | Triggered when the storage size has surpassed the storage capacity |


<a name="event.onStorageExceeded"></a>
## *onStorageExceeded <sup>event</sup>*

Triggered when the storage size has surpassed the storage capacity. The total size of the datastore should not exceed more than 1MB in size.

### Parameters

This event carries no parameters.

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onStorageExceeded"
}
```

