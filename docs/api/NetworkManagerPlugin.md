<!-- Generated automatically, DO NOT EDIT! -->
<a name="NetworkManager_Plugin"></a>
# NetworkManager Plugin

**Version: [0.2.10](https://github.com/rdkcentral/rdkservices/blob/main/NetworkManager/CHANGELOG.md)**

A org.rdk.NetworkManager plugin for Thunder framework.

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

A Unified `NetworkManager` plugin that allows you to manage Ethernet and Wifi interfaces on the device.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.NetworkManager*) |
| classname | string | Class name: *org.rdk.NetworkManager* |
| locator | string | Library name: *libWPEFrameworkNetworkManager.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.NetworkManager plugin:

NetworkManager interface methods:

| Method | Description |
| :-------- | :-------- |
| [GetAvailableInterfaces](#GetAvailableInterfaces) | Get device supported list of available interface including their state |
| [GetPrimaryInterface](#GetPrimaryInterface) | Gets the primary/default network interface for the device |
| [SetPrimaryInterface](#SetPrimaryInterface) | Sets the primary/default interface for the device |
| [GetIPSettings](#GetIPSettings) | Gets the IP setting for the given interface |
| [SetIPSettings](#SetIPSettings) | Sets the IP settings for the given interface |
| [GetStunEndpoint](#GetStunEndpoint) | Get the STUN Endpoint that is used to identify public IP of the device |
| [SetStunEndpoint](#SetStunEndpoint) | Set the STUN Endpoint to be used to identify public IP of the device |
| [GetConnectivityTestEndpoints](#GetConnectivityTestEndpoints) | Gets currently used test endpoints |
| [SetConnectivityTestEndpoints](#SetConnectivityTestEndpoints) | This method used to set up to 5 endpoints for a connectivity test |
| [IsConnectedToInternet](#IsConnectedToInternet) | Seeks Whether the device has internet connectivity |
| [GetCaptivePortalURI](#GetCaptivePortalURI) | Gets the captive portal URI if connected to any captive portal network |
| [StartConnectivityMonitoring](#StartConnectivityMonitoring) | Enable a continuous monitoring of internet connectivity with heart beat interval thats given |
| [StopConnectivityMonitoring](#StopConnectivityMonitoring) | Stops the connectivity monitoring |
| [GetPublicIP](#GetPublicIP) | Gets the internet/public IP Address of the device |
| [Ping](#Ping) | Pings the specified endpoint with the specified number of packets |
| [Trace](#Trace) | Traces the specified endpoint with the specified number of packets using `traceroute` |
| [StartWiFiScan](#StartWiFiScan) | Initiates WiFi scaning |
| [StopWiFiScan](#StopWiFiScan) | Stops WiFi scanning |
| [GetKnownSSIDs](#GetKnownSSIDs) | Gets list of saved SSIDs |
| [AddToKnownSSIDs](#AddToKnownSSIDs) | Saves the SSID, passphrase, and security mode for upcoming and future sessions |
| [RemoveKnownSSID](#RemoveKnownSSID) | Remove given SSID from saved SSIDs |
| [WiFiConnect](#WiFiConnect) | Initiates request to connect to the specified SSID with the given passphrase |
| [WiFiDisconnect](#WiFiDisconnect) | Disconnects from the currently connected SSID |
| [GetConnectedSSID](#GetConnectedSSID) | Returns the connected SSID information |
| [StartWPS](#StartWPS) | Initiates a connection using Wifi Protected Setup (WPS) |
| [StopWPS](#StopWPS) | Cancels the in-progress WPS pairing operation |
| [GetWiFiSignalStrength](#GetWiFiSignalStrength) | Get WiFiSignalStrength of connected SSID |
| [GetSupportedSecurityModes](#GetSupportedSecurityModes) | Returns the Wifi security modes that the device supports |
| [SetLogLevel](#SetLogLevel) | Set Log level for more information |
| [EnableInterface](#EnableInterface) | Enable the specified interface |
| [DisableInterface](#DisableInterface) | Disable the specified interface |
| [GetWifiState](#GetWifiState) | Returns the current Wifi State |


<a name="GetAvailableInterfaces"></a>
## *GetAvailableInterfaces*

Get device supported list of available interface including their state.

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
| result?.Interfaces[#].type | string | Interface  Type |
| result?.Interfaces[#].name | string | Interface Name. ex: eth0 or wlan0 |
| result?.Interfaces[#].mac | string | Interface MAC address |
| result?.Interfaces[#].isEnabled | boolean | Whether the interface is currently enabled |
| result?.Interfaces[#].isConnected | boolean | Whether the interface is currently connected |
