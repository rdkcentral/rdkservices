<!-- Generated automatically, DO NOT EDIT! -->
<a name="PersistentStore_Plugin"></a>
# PersistentStore Plugin

**Version: [1.0.4](https://github.com/rdkcentral/rdkservices/blob/main/PersistentStore/CHANGELOG.md)**

A org.rdk.PersistentStore plugin for Thunder framework.

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

The `PersistentStore` plugin allows you to persist key/value pairs by namespace.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.PersistentStore*) |
| classname | string | Class name: *org.rdk.PersistentStore* |
| locator | string | Library name: *libWPEFrameworkPersistentStore.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.PersistentStore plugin:

PersistentStore interface methods:

| Method | Description |
| :-------- | :-------- |
| [deleteKey](#deleteKey) | Deletes a key from the specified namespace |
| [deleteNamespace](#deleteNamespace) | Deletes the specified namespace |
| [flushCache](#flushCache) | Flushes the database cache by invoking `flush` in SQLite |
| [getKeys](#getKeys) | Returns the keys that are stored in the specified namespace |
| [getNamespaces](#getNamespaces) | Returns the namespaces in the datastore |
| [getStorageSize](#getStorageSize) | Returns the size occupied by each namespace |
| [getValue](#getValue) | Returns the value of a key from the specified namespace |
| [setValue](#setValue) | Sets the value of a key in the the specified namespace |


<a name="deleteKey"></a>
## *deleteKey*

Deletes a key from the specified namespace.

### Events

No Events

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
    "id": 42,
    "method": "org.rdk.PersistentStore.deleteKey",
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
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="deleteNamespace"></a>
## *deleteNamespace*

Deletes the specified namespace.

### Events

No Events

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
    "id": 42,
    "method": "org.rdk.PersistentStore.deleteNamespace",
    "params": {
        "namespace": "ns1"
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

<a name="flushCache"></a>
## *flushCache*

Flushes the database cache by invoking `flush` in SQLite.

### Events

No Events

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
    "id": 42,
    "method": "org.rdk.PersistentStore.flushCache"
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

<a name="getKeys"></a>
## *getKeys*

Returns the keys that are stored in the specified namespace.

### Events

No Events

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
    "id": 42,
    "method": "org.rdk.PersistentStore.getKeys",
    "params": {
        "namespace": "ns1"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "keys": [
            "key1"
        ],
        "success": true
    }
}
```

<a name="getNamespaces"></a>
## *getNamespaces*

Returns the namespaces in the datastore.

### Events

No Events

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
    "id": 42,
    "method": "org.rdk.PersistentStore.getNamespaces"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "namespaces": [
            "ns1"
        ],
        "success": true
    }
}
```

<a name="getStorageSize"></a>
## *getStorageSize*

Returns the size occupied by each namespace. This is a processing-intense operation. The total size of the datastore should not exceed more than 1MB in size. If the storage size is exceeded then, new values are not stored and the `onStorageExceeded` event is sent.

### Events

No Events

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
    "id": 42,
    "method": "org.rdk.PersistentStore.getStorageSize"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "namespaceSizes": {
            "ns1": 534,
            "ns2": 234
        },
        "success": true
    }
}
```

<a name="getValue"></a>
## *getValue*

Returns the value of a key from the specified namespace.

### Events

No Events

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
    "id": 42,
    "method": "org.rdk.PersistentStore.getValue",
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
    "id": 42,
    "result": {
        "value": "value1",
        "success": true
    }
}
```

<a name="setValue"></a>
## *setValue*

Sets the value of a key in the the specified namespace.

### Events

| Event | Description |
| :-------- | :-------- |
| [onStorageExceeded](#onStorageExceeded) | Triggered if the storage size has surpassed 1 MB storage size |
| [onValueChanged](#onValueChanged) | Triggered whenever any of the values stored are changed using setValue |
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
    "id": 42,
    "method": "org.rdk.PersistentStore.setValue",
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
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.PersistentStore plugin:

PersistentStore interface events:

| Event | Description |
| :-------- | :-------- |
| [onStorageExceeded](#onStorageExceeded) | Triggered when the storage size has surpassed the storage capacity |
| [onValueChanged](#onValueChanged) | Triggered whenever any of the values stored are changed using setValue |


<a name="onStorageExceeded"></a>
## *onStorageExceeded*

Triggered when the storage size has surpassed the storage capacity. The total size of the datastore should not exceed more than 1MB in size.

### Parameters

This event carries no parameters.

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onStorageExceeded"
}
```

<a name="onValueChanged"></a>
## *onValueChanged*

Triggered whenever any of the values stored are changed using setValue.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.namespace | string | A namespace in the datastore as a valid UTF-8 string |
| params.key | string | The key name as a valid UTF-8 string |
| params.value | string | The key value. Values are capped at 1000 characters in size |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onValueChanged",
    "params": {
        "namespace": "ns1",
        "key": "key1",
        "value": "value1"
    }
}
```

