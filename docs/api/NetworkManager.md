<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.NetworkManager"></a>
# NetworkManager

**Version: [1.0.0]()**

A NetworkManager plugin for Thunder framework.

### Table of Contents

- [Abbreviation, Acronyms and Terms](#head.Abbreviation,_Acronyms_and_Terms)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Interfaces](#head.Interfaces)
- [Methods](#head.Methods)
- [Notifications](#head.Notifications)

<a name="head.Abbreviation,_Acronyms_and_Terms"></a>
# Abbreviation, Acronyms and Terms

[[Refer to this link](userguide/aat.md)]

<a name="head.Description"></a>
# Description

The `NetworkManager` plugin allows you to manage network and wifi interfaces on a set-top box.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| classname | string | Class name: *NetworkManager* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Interfaces"></a>
# Interfaces

This plugin implements the following interfaces:

- [NetworkManager.json](https://github.com/rdkcentral/ThunderInterfaces/blob/master/interfaces/NetworkManager.json)

<a name="head.Methods"></a>
# Methods

The following methods are provided by the NetworkManager plugin:

NetworkManager interface methods:

| Method | Description |
| :-------- | :-------- |
| [GetAvailableInterfaces](#method.GetAvailableInterfaces) | Returns a list of interfaces supported by this device including their state |
| [GetPrimaryInterface](#method.GetPrimaryInterface) | Gets the Primary/default network interface |
| [SetPrimaryInterface](#method.SetPrimaryInterface) | Sets the Primary/default interface |
| [GetIPSettings](#method.GetIPSettings) | Gets the IP setting for the given interface |
| [SetIPSettings](#method.SetIPSettings) | Sets the IP settings |
| [GetStunEndpoint](#method.GetStunEndpoint) | Get the Stun Endpoint used for getPublicIP |
| [setStunEndPoint](#method.setStunEndPoint) | Set the Stun Endpoint used for getPublicIP |
| [GetConnectivityTestEndpoints](#method.GetConnectivityTestEndpoints) | Get connectivity test endpoints |
| [SetConnectivityTestEndpoints](#method.SetConnectivityTestEndpoints) | Define up to 5 endpoints for a connectivity test |
| [IsConnectedToInternet](#method.IsConnectedToInternet) | Whether the device has internet connectivity |
| [GetCaptivePortalURI](#method.GetCaptivePortalURI) | Returns the captive portal URI if connected to any captive portal network |
| [StartConnectivityMonitoring](#method.StartConnectivityMonitoring) | Enable a continuous monitoring of internet connectivity with heart beat interval thats given |
| [StopConnectivityMonitoring](#method.StopConnectivityMonitoring) | Stops the connectivity monitoring |
| [getPublicIP](#method.getPublicIP) | It allows either zero parameter or with only interface and ipv6 parameter to determine WAN ip address |
| [ping](#method.ping) | Pings the specified endpoint with the specified number of packets |
| [trace](#method.trace) | Traces the specified endpoint with the specified number of packets using `traceroute` |
| [StartWiFiScan](#method.StartWiFiScan) | Scans for available SSIDs |
| [StopWiFiScan](#method.StopWiFiScan) | Stops scanning for SSIDs |


<a name="method.GetAvailableInterfaces"></a>
## *GetAvailableInterfaces [<sup>method</sup>](#head.Methods)*

Returns a list of interfaces supported by this device including their state.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.Interfaces | array | <sup>*(optional)*</sup> An interface |
| result?.Interfaces[#] | object | <sup>*(optional)*</sup>  |
| result?.Interfaces[#].type | string | Type |
| result?.Interfaces[#].name | string | Name |
| result?.Interfaces[#].mac | string | Interface MAC address |
| result?.Interfaces[#].isEnabled | boolean | Whether the interface is currently enabled |
| result?.Interfaces[#].isConnected | boolean | Whether the interface is currently connected |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "NetworkManager.GetAvailableInterfaces"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "Interfaces": [
            {
                "type": "...",
                "name": "...",
                "mac": "AA:AA:AA:AA:AA:AA",
                "isEnabled": true,
                "isConnected": true
            }
        ],
        "success": true
    }
}
```

<a name="method.GetPrimaryInterface"></a>
## *GetPrimaryInterface [<sup>method</sup>](#head.Methods)*

Gets the Primary/default network interface. The active network interface is defined as the one that can make requests to the external network. Returns one of the supported interfaces as per `getInterfaces`, or an empty value which indicates that there is no default network interface.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.interface | string | An interface, such as `eth0` or `wlan0`, depending upon availability of the given interface in `getInterfaces` |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "NetworkManager.GetPrimaryInterface"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "interface": "wlan0"
    }
}
```

<a name="method.SetPrimaryInterface"></a>
## *SetPrimaryInterface [<sup>method</sup>](#head.Methods)*

Sets the Primary/default interface. The call fails if the interface is not enabled.

### Events

| Event | Description |
| :-------- | :-------- |
| [onActiveInterfaceChanged](#event.onActiveInterfaceChanged) | Triggered when device’s default interface changed. |
| [onInterfaceStateChanged](#event.onInterfaceStateChanged) | Triggered when interface’s state changed |
| [onIPAddressChanged](#event.onIPAddressChanged) | Triggered when the device connects to router. |
| [onInternetStatusChanged](#event.onInternetStatusChanged) | Triggered when each IP address is lost or acquired. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.interface | string | An interface, such as `eth0` or `wlan0`, depending upon availability of the given interface in `getInterfaces` |
| params.persist | boolean | Whether the default interface setting persists after reboot. When `true`, this interface is enabled as the default interface currently AND on the next reboot. When `false`, this interface is only the default during this session |

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
    "method": "NetworkManager.SetPrimaryInterface",
    "params": {
        "interface": "wlan0",
        "persist": true
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

<a name="method.GetIPSettings"></a>
## *GetIPSettings [<sup>method</sup>](#head.Methods)*

Gets the IP setting for the given interface.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.interface | string | An interface, such as `eth0` or `wlan0`, depending upon availability of the given interface in `getInterfaces` |
| params?.ipversion | string | <sup>*(optional)*</sup> either IPv4 or IPv6 |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.interface | string | An interface, such as `eth0` or `wlan0`, depending upon availability of the given interface in `getInterfaces` |
| result.ipversion | string | either IPv4 or IPv6 |
| result.autoconfig | boolean | `true` if DHCP is used, `false` if IP is configured manually |
| result?.dhcpserver | string | <sup>*(optional)*</sup> The DHCP Server address |
| result.ipaddress | string | The IP address |
| result.prefix | integer | The prefix number |
| result.gateway | string | The gateway address |
| result.v6LinkLocal | string | The v6LinkLocal |
| result.primarydns | string | The primary DNS address |
| result.secondarydns | string | The secondary DNS address |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "NetworkManager.GetIPSettings",
    "params": {
        "interface": "wlan0",
        "ipversion": "IPv4"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "interface": "wlan0",
        "ipversion": "IPv4",
        "autoconfig": true,
        "dhcpserver": "192.168.1.1",
        "ipaddress": "192.168.1.101",
        "prefix": 123,
        "gateway": "192.168.1.1",
        "v6LinkLocal": "192.168.1.1",
        "primarydns": "192.168.1.1",
        "secondarydns": "192.168.1.2",
        "success": true
    }
}
```

<a name="method.SetIPSettings"></a>
## *SetIPSettings [<sup>method</sup>](#head.Methods)*

Sets the IP settings.All the inputs are mandatory for v1. But for v2, the interface and autconfig params are mandatory input to autoconfig IP settings & other parameters not required. For manual IP, all the input parameters are mandatory except secondaryDNS.

### Events

| Event | Description |
| :-------- | :-------- |
| [onInternetStatusChanged](#event.onInternetStatusChanged) | Triggered when each IP address is lost or acquired. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.interface | string | An interface, such as `eth0` or `wlan0`, depending upon availability of the given interface in `getInterfaces` |
| params.ipversion | string | either IPv4 or IPv6 |
| params.autoconfig | boolean | `true` if DHCP is used, `false` if IP is configured manually |
| params.ipaddress | string | The IP address |
| params.prefix | integer | The prefix number |
| params.gateway | string | The gateway address |
| params.primarydns | string | The primary DNS address |
| params.secondarydns | string | The secondary DNS address |

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
    "method": "NetworkManager.SetIPSettings",
    "params": {
        "interface": "wlan0",
        "ipversion": "IPv4",
        "autoconfig": true,
        "ipaddress": "192.168.1.101",
        "prefix": 123,
        "gateway": "192.168.1.1",
        "primarydns": "192.168.1.1",
        "secondarydns": "192.168.1.2"
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

<a name="method.GetStunEndpoint"></a>
## *GetStunEndpoint [<sup>method</sup>](#head.Methods)*

Get the Stun Endpoint used for getPublicIP.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.endPoint | string | STUN server endPoint |
| result.port | integer | STUN server port |
| result.bindTimeout | integer | STUN server bind timeout |
| result.cacheTimeout | integer | STUN server cache timeout |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "NetworkManager.GetStunEndpoint"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "endPoint": "stun.l.google.com",
        "port": 3478,
        "bindTimeout": 30,
        "cacheTimeout": 0,
        "success": true
    }
}
```

<a name="method.setStunEndPoint"></a>
## *setStunEndPoint [<sup>method</sup>](#head.Methods)*

Set the Stun Endpoint used for getPublicIP.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.endPoint | string | STUN server endPoint |
| params.port | integer | STUN server port |
| params.bindTimeout | integer | STUN server bind timeout |
| params.cacheTimeout | integer | STUN server cache timeout |

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
    "method": "NetworkManager.setStunEndPoint",
    "params": {
        "endPoint": "stun.l.google.com",
        "port": 3478,
        "bindTimeout": 30,
        "cacheTimeout": 0
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

<a name="method.GetConnectivityTestEndpoints"></a>
## *GetConnectivityTestEndpoints [<sup>method</sup>](#head.Methods)*

Get connectivity test endpoints. on success list out the connectivity test points connections.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.endpoints | array |  |
| result.endpoints[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "NetworkManager.GetConnectivityTestEndpoints"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "endpoints": [
            "http://clients3.google.com/generate_204"
        ],
        "success": true
    }
}
```

<a name="method.SetConnectivityTestEndpoints"></a>
## *SetConnectivityTestEndpoints [<sup>method</sup>](#head.Methods)*

Define up to 5 endpoints for a connectivity test. Successful connections are verified with HTTP Status code 204 (No Content). Priority is given to endpoints configured in /etc/netsrvmgr.conf. In case of errors or if not configured, the default endpoints are considered: `http://clients3.google.com/generate_204`.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.endpoints | array | A list of endpoints to test |
| params.endpoints[#] | string |  |

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
    "method": "NetworkManager.SetConnectivityTestEndpoints",
    "params": {
        "endpoints": [
            "http://clients3.google.com/generate_204"
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

<a name="method.IsConnectedToInternet"></a>
## *IsConnectedToInternet [<sup>method</sup>](#head.Methods)*

Whether the device has internet connectivity. This API might take up to 2s to validate internet connectivity.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ipversion | string | either IPv4 or IPv6 |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.isConnectedToInternet | boolean | `true` if internet connectivity is detected, otherwise `false` |
| result.internetState | integer | Internet Connection state |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "NetworkManager.IsConnectedToInternet",
    "params": {
        "ipversion": "IPv4"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "isConnectedToInternet": true,
        "internetState": 2,
        "success": true
    }
}
```

<a name="method.GetCaptivePortalURI"></a>
## *GetCaptivePortalURI [<sup>method</sup>](#head.Methods)*

Returns the captive portal URI if connected to any captive portal network.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.uri | string | Captive portal URI |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "NetworkManager.GetCaptivePortalURI"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "uri": "http://10.0.0.1/captiveportal.jst",
        "success": true
    }
}
```

<a name="method.StartConnectivityMonitoring"></a>
## *StartConnectivityMonitoring [<sup>method</sup>](#head.Methods)*

Enable a continuous monitoring of internet connectivity with heart beat interval thats given.

### Events

| Event | Description |
| :-------- | :-------- |
| [onInternetStatusChanged](#event.onInternetStatusChanged) | Triggered when internet connection state changed. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.interval | number | Interval in sec. Default value 60 sec and interval should be greater than 5 sec |

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
    "method": "NetworkManager.StartConnectivityMonitoring",
    "params": {
        "interval": 30
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

<a name="method.StopConnectivityMonitoring"></a>
## *StopConnectivityMonitoring [<sup>method</sup>](#head.Methods)*

Stops the connectivity monitoring.

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
    "method": "NetworkManager.StopConnectivityMonitoring"
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

<a name="method.getPublicIP"></a>
## *getPublicIP [<sup>method</sup>](#head.Methods)*

It allows either zero parameter or with only interface and ipv6 parameter to determine WAN ip address.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | it allows empty parameter too |
| params.ipversion | string | either IPv4 or IPv6 |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.publicIP | string | Returns an public ip of the device ,if ipv6 is `true`,returns IPv6 public ip , otherwise returns IPv4 public ip |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "NetworkManager.getPublicIP",
    "params": {
        "ipversion": "IPv4"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "publicIP": "69.136.49.95",
        "success": true
    }
}
```

<a name="method.ping"></a>
## *ping [<sup>method</sup>](#head.Methods)*

Pings the specified endpoint with the specified number of packets.

### Events

| Event | Description |
| :-------- | :-------- |
| [onPingResponse](#event.onPingResponse) | Triggered when Ping request received. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.endpoint | string | The host name or IP address |
| params.ipversion | string | either IPv4 or IPv6 |
| params.noOfRequest | integer | The number of packets to send. Default is 15 |
| params.timeout | integer | STUN server bind timeout |
| params.guid | string | The globally unique identifier |

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
    "method": "NetworkManager.ping",
    "params": {
        "endpoint": "45.57.221.20",
        "ipversion": "IPv4",
        "noOfRequest": 10,
        "timeout": 30,
        "guid": "..."
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

<a name="method.trace"></a>
## *trace [<sup>method</sup>](#head.Methods)*

Traces the specified endpoint with the specified number of packets using `traceroute`.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.endpoint | string | The host name or IP address |
| params.ipversion | string | either IPv4 or IPv6 |
| params.noOfRequest | integer | The number of packets to send. Default is 15 |
| params.guid | string | The globally unique identifier |

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
    "method": "NetworkManager.trace",
    "params": {
        "endpoint": "45.57.221.20",
        "ipversion": "IPv4",
        "noOfRequest": 10,
        "guid": "..."
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

<a name="method.StartWiFiScan"></a>
## *StartWiFiScan [<sup>method</sup>](#head.Methods)*

Scans for available SSIDs. Available SSIDs are returned in an `onAvailableSSIDs` event.

### Events

| Event | Description |
| :-------- | :-------- |
| [onAvailableSSIDs](#event.onAvailableSSIDs) | Triggered when list of SSIDs is available after the scan completes. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.frequency | string | The frequency to scan. An empty or `null` value scans all frequencies. If a frequency is specified (2.4 or 5.0), then the results are only returned for matching frequencies |

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
    "method": "NetworkManager.StartWiFiScan",
    "params": {
        "frequency": "..."
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

<a name="method.StopWiFiScan"></a>
## *StopWiFiScan [<sup>method</sup>](#head.Methods)*

Stops scanning for SSIDs. Any discovered SSIDs from the call to the `startScan` method up to the point where this method is called are still returned.

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
    "method": "NetworkManager.StopWiFiScan"
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

The following events are provided by the NetworkManager plugin:

NetworkManager interface events:

| Event | Description |
| :-------- | :-------- |
| [onInterfaceStateChanged](#event.onInterfaceStateChanged) | Triggered when an interface becomes enabled or disabled |
| [onIPAddressChanged](#event.onIPAddressChanged) | Triggered when an IP Address is assigned or lost |
| [onActiveInterfaceChanged](#event.onActiveInterfaceChanged) | Triggered when the ac interface changes, regardless if it's from a system operation or through the `setDefaultInterface` method |
| [onInternetStatusChange](#event.onInternetStatusChange) | Triggered when internet connection state changed |
| [onPingResponse](#event.onPingResponse) | Triggered when a connection is made or lost |
| [onTraceResponse](#event.onTraceResponse) | Triggered when a connection is made or lost |
| [onAvailableSSIDs](#event.onAvailableSSIDs) | Triggered when got for scan |
| [onWiFiStateChanged](#event.onWiFiStateChanged) | Triggered when WIFI connection state get changed |
| [onWiFiSignalStrengthChanged](#event.onWiFiSignalStrengthChanged) | Triggered when WIFI connection Signal Strength get changed |


<a name="event.onInterfaceStateChanged"></a>
## *onInterfaceStateChanged [<sup>event</sup>](#head.Notifications)*

Triggered when an interface becomes enabled or disabled.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.interface | string | An interface, such as `eth0` or `wlan0`, depending upon availability of the given interface in `getInterfaces` |
| params.state | string | Whether the interface is enabled (`true`) or disabled (`false`) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onInterfaceStateChanged",
    "params": {
        "interface": "wlan0",
        "state": "Interface_ADDED"
    }
}
```

<a name="event.onIPAddressChanged"></a>
## *onIPAddressChanged [<sup>event</sup>](#head.Notifications)*

Triggered when an IP Address is assigned or lost.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.interface | string | An interface, such as `eth0` or `wlan0`, depending upon availability of the given interface in `getInterfaces` |
| params.ipv6 | string | The IPv6 address for the interface |
| params?.ipv4 | string | <sup>*(optional)*</sup> The IPv4 address for the interface |
| params.status | string | Whether IP address was acquired or lost (must be one of the following: *`ACQUIRED`*, *`LOST`*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onIPAddressChanged",
    "params": {
        "interface": "wlan0",
        "ipv6": "2001:0xx8:85a3:0000:0000:8x2x:0370:7334",
        "ipv4": "192.168.1.2",
        "status": "ACQUIRED"
    }
}
```

<a name="event.onActiveInterfaceChanged"></a>
## *onActiveInterfaceChanged [<sup>event</sup>](#head.Notifications)*

Triggered when the ac interface changes, regardless if it's from a system operation or through the `setDefaultInterface` method.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.oldInterfaceName | string | The previous interface that was changed |
| params.newInterfaceName | string | The current interface |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onActiveInterfaceChanged",
    "params": {
        "oldInterfaceName": "ETHERNET",
        "newInterfaceName": "WIFI"
    }
}
```

<a name="event.onInternetStatusChange"></a>
## *onInternetStatusChange [<sup>event</sup>](#head.Notifications)*

Triggered when internet connection state changed.The possible internet connection status are `NO_INTERNET`, `LIMITED_INTERNET`, `CAPTIVE_PORTAL`, `FULLY_CONNECTED`.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.PrevState | integer | The privious internet connection state |
| params.PrevStatus | string | The previous internet connection status |
| params.state | integer | The internet connection state |
| params.status | string | The internet connection status |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onInternetStatusChange",
    "params": {
        "PrevState": 0,
        "PrevStatus": "NO_INTERNET",
        "state": 0,
        "status": "NO_INTERNET"
    }
}
```

<a name="event.onPingResponse"></a>
## *onPingResponse [<sup>event</sup>](#head.Notifications)*

Triggered when a connection is made or lost.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.pingStatistics | string | Ping Statistics |
| params.guid | string | The globally unique identifier |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onPingResponse",
    "params": {
        "pingStatistics": "...",
        "guid": "..."
    }
}
```

<a name="event.onTraceResponse"></a>
## *onTraceResponse [<sup>event</sup>](#head.Notifications)*

Triggered when a connection is made or lost.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.traceResult | string | Trace Result |
| params.guid | string | The globally unique identifier |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onTraceResponse",
    "params": {
        "traceResult": "...",
        "guid": "..."
    }
}
```

<a name="event.onAvailableSSIDs"></a>
## *onAvailableSSIDs [<sup>event</sup>](#head.Notifications)*

Triggered when got for scan.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ssids | string | On Available SSID's |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onAvailableSSIDs",
    "params": {
        "ssids": "..."
    }
}
```

<a name="event.onWiFiStateChanged"></a>
## *onWiFiStateChanged [<sup>event</sup>](#head.Notifications)*

Triggered when WIFI connection state get changed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.state | integer | onWiFiStateChanged |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onWiFiStateChanged",
    "params": {
        "state": 3
    }
}
```

<a name="event.onWiFiSignalStrengthChanged"></a>
## *onWiFiSignalStrengthChanged [<sup>event</sup>](#head.Notifications)*

Triggered when WIFI connection Signal Strength get changed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ssid | string | Signal Strength changed SSID |
| params.signalStrength | string | Signal Strength |
| params.quality | string | Signal quality |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onWiFiSignalStrengthChanged",
    "params": {
        "ssid": "comcast123",
        "signalStrength": "...",
        "quality": "..."
    }
}
```

