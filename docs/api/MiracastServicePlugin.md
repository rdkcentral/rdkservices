<!-- Generated automatically, DO NOT EDIT! -->
<a name="MiracastService_Plugin"></a>
# MiracastService Plugin

**Version: [1.0.4](https://github.com/rdkcentral/rdkservices/blob/main/Miracast/CHANGELOG.md)**

A org.rdk.MiracastService plugin for Thunder framework.

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

The `MiracastService` plugin will manage Peer-to-Peer events from WiFi driver then It will launch Miracast player for screen mirroring.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.MiracastService*) |
| classname | string | Class name: *org.rdk.MiracastService* |
| locator | string | Library name: *libWPEFrameworkMiracastService.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.MiracastService plugin:

MiracastService interface methods:

| Method | Description |
| :-------- | :-------- |
| [setEnable](#setEnable) | To enable /disable the Miracast feature |
| [getEnable](#getEnable) | To get the enable status of the Miracast feature |
| [acceptClientConnection](#acceptClientConnection) | To accept /reject new client connection requests for the Miracast feature |
| [updatePlayerState](#updatePlayerState) | Update the Miracast Player State to the Miracast Service Plugin |
| [stopClientConnection](#stopClientConnection) | To abort the ongoing connection after accepted connection request |
| [setLogging](#setLogging) | Enable/Disable/Reduce Logging level for Miracast |


<a name="setEnable"></a>
## *setEnable*

To enable /disable the Miracast feature.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | `true` for enabled or `false` for disabled |

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
    "method": "org.rdk.MiracastService.setEnable",
    "params": {
        "enabled": true
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

<a name="getEnable"></a>
## *getEnable*

To get the enable status of the Miracast feature.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.enabled | boolean | `true` for enabled or `false` for disabled |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.MiracastService.getEnable"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "enabled": true,
        "success": true
    }
}
```

<a name="acceptClientConnection"></a>
## *acceptClientConnection*

To accept /reject new client connection requests for the Miracast feature.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.requestStatus | string | `Accept` for connect device request or `Reject` for denying connect device request |

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
    "method": "org.rdk.MiracastService.acceptClientConnection",
    "params": {
        "requestStatus": "Accept or Reject"
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

<a name="updatePlayerState"></a>
## *updatePlayerState*

Update the Miracast Player State to the Miracast Service Plugin.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.mac | string | Mac id of the source device |
| params.state | string | Current state of the Miracast player |
| params.reason_code | number | player Reason codes (must be one of the following: *MIRACAST_PLAYER_REASON_CODE_SUCCESS = 200*, *MIRACAST_PLAYER_REASON_CODE_APP_REQ_TO_STOP = 201*, *MIRACAST_PLAYER_REASON_CODE_SRC_DEV_REQ_TO_STOP = 202*, *MIRACAST_PLAYER_REASON_CODE_RTSP_ERROR = 203*, *MIRACAST_PLAYER_REASON_CODE_RTSP_TIMEOUT = 204*, *MIRACAST_PLAYER_REASON_CODE_RTSP_METHOD_NOT_SUPPORTED = 205*, *MIRACAST_PLAYER_REASON_CODE_GST_ERROR = 206*, *MIRACAST_PLAYER_REASON_CODE_INT_FAILURE = 207*, *MIRACAST_PLAYER_REASON_CODE_NEW_SRC_DEV_CONNECT_REQ = 208*) |
| params.reason | string | Description about the player's reason code |

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
    "method": "org.rdk.MiracastService.updatePlayerState",
    "params": {
        "mac": "ab:cd:ef:12:34:56",
        "state": "(INITIATED | INPROGRESS | PLAYING | STOPPED/IDLE(Default State))",
        "reason_code": 200,
        "reason": "read Description"
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

<a name="stopClientConnection"></a>
## *stopClientConnection*

To abort the ongoing connection after accepted connection request.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.mac | string | Mac id of the source device |
| params.name | string | Name of the source device |

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
    "method": "org.rdk.MiracastService.stopClientConnection",
    "params": {
        "mac": "ab:cd:ef:12:34:56",
        "name": "Manufacturer WiFi-Direct Name"
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

<a name="setLogging"></a>
## *setLogging*

Enable/Disable/Reduce Logging level for Miracast.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.level | string | Set the log level for Miracast service plugin |
| params?.separate_logger | object | <sup>*(optional)*</sup> Route the Miracast service to separate file if required |
| params?.separate_logger.logfilename | string | Name of separate logging filename. Logfile will be created under /opt/logs/ |
| params?.separate_logger.status | string | Enable/Disable the separate logging |

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
    "method": "org.rdk.MiracastService.setLogging",
    "params": {
        "level": "FATAL|ERROR|WARNING|INFO|VERBOSE|TRACE",
        "separate_logger": {
            "logfilename": "sample.log",
            "status": "ENABLE|DISABLE"
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

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.MiracastService plugin:

MiracastService interface events:

| Event | Description |
| :-------- | :-------- |
| [onClientConnectionRequest](#onClientConnectionRequest) | Triggered when the Miracast Service plugin receives a new connection request from a client |
| [onLaunchRequest](#onLaunchRequest) | Miracast Service Plugin raises this Event to request RA to Launch the Miracast Player |
| [onClientConnectionError](#onClientConnectionError) | It triggered when the Miracast Service plugin failed to connect with the source streaming device due to some error, like P2P related errors during activation or while streaming |


<a name="onClientConnectionRequest"></a>
## *onClientConnectionRequest*

Triggered when the Miracast Service plugin receives a new connection request from a client. The application should respond with acceptClientConnection call to accept or reject the request.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.mac | string | Mac id of the source device |
| params.name | string | Name of the source device |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onClientConnectionRequest",
    "params": {
        "mac": "ab:cd:ef:12:34:56",
        "name": "Manufacturer WiFi-Direct Name"
    }
}
```

<a name="onLaunchRequest"></a>
## *onLaunchRequest*

Miracast Service Plugin raises this Event to request RA to Launch the Miracast Player.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.source_dev_ip | string | Source Device IP address |
| params.source_dev_mac | string | Mac id of the source device |
| params.source_dev_name | string | Name of the source device |
| params.sink_dev_ip | string | IP address of the Local/Sink device |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onLaunchRequest",
    "params": {
        "source_dev_ip": "xx.xx.xx.xx",
        "source_dev_mac": "ab:cd:ef:12:34:56",
        "source_dev_name": "Manufacturer WiFi-Direct Name",
        "sink_dev_ip": "xx.xx.xx.xx"
    }
}
```

<a name="onClientConnectionError"></a>
## *onClientConnectionError*

It triggered when the Miracast Service plugin failed to connect with the source streaming device due to some error, like P2P related errors during activation or while streaming.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.name | string | Name of the source device |
| params.mac | string | Mac id of the source device |
| params.error_code | number | Miracast Service plugin error codes (must be one of the following: *MIRACAST_SERVICE_ERR_CODE_SUCCESS = 100*, *MIRACAST_SERVICE_ERR_CODE_P2P_CONNECT_ERROR = 102*, *MIRACAST_SERVICE_ERR_CODE_P2P_GROUP_NEGO_ERROR = 103*, *MIRACAST_SERVICE_ERR_CODE_P2P_GROUP_FORMATION_ERROR = 104*, *MIRACAST_SERVICE_ERR_CODE_GENERIC_FAILURE = 105*) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onClientConnectionError",
    "params": {
        "name": "Manufacturer WiFi-Direct Name",
        "mac": "ab:cd:ef:12:34:56",
        "error_code": 100
    }
}
```

