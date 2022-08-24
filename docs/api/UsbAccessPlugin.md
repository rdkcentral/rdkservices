<!-- Generated automatically, DO NOT EDIT! -->
<a name="UsbAccess_Plugin"></a>
# UsbAccess Plugin

**Version: 1.0.0**

A org.rdk.UsbAccess plugin for Thunder framework.

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

The `UsbAccess` plugin provides the ability to examine the contents on a USB drive and access the content through HTTP URLs.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.UsbAccess*) |
| classname | string | Class name: *org.rdk.UsbAccess* |
| locator | string | Library name: *libWPEFrameworkUsbAccess.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.UsbAccess plugin:

UsbAccess interface methods:

| Method | Description |
| :-------- | :-------- |
| [clearLink](#clearLink) | Clears or removes the symbolic link created by the `createLink` method |
| [createLink](#createLink) | Creates a symbolic link to the root folder of the USB drive |
| [getAvailableFirmwareFiles](#getAvailableFirmwareFiles) | (Version 2) Gets a list of firmware files on the device |
| [getFileList](#getFileList) | Gets a list of files and folders from the specified directory or path |
| [getMounted](#getMounted) | (Version 2) Returns a list of mounted USB devices |
| [updateFirmware](#updateFirmware) | (Version 2) Updates the firmware using the specified file retrieved from the `getAvailableFirmwareFiles` method |
| [ArchiveLogs](#ArchiveLogs) | (Version 2) Compresses and uploads device logs into attached USB drive from /opt/logs with a name comprises of Mac of the device , date and time in a `tgz` format |


<a name="clearLink"></a>
## *clearLink*

Clears or removes the symbolic link created by the `createLink` 
 
### Events 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |
| result?.error | string | <sup>*(optional)*</sup> An error message in case of a failure |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UsbAccess.1.clearLink"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true,
        "error": "could not remove symlink"
    }
}
```

<a name="createLink"></a>
## *createLink*

Creates a symbolic link to the root folder of the USB drive. If called, and a link already exists, then it errors out. Symbolic link has read-only access. Use the name `usbdrive`. 
 
### Events 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.baseURL | string | The URL of the web server that points to this location |
| result.success | boolean | Whether the request succeeded |
| result?.error | string | <sup>*(optional)*</sup> An error message in case of a failure |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UsbAccess.1.createLink"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "baseURL": "http://localhost/usbdrive",
        "success": true,
        "error": "could not create symlink"
    }
}
```

<a name="getAvailableFirmwareFiles"></a>
## *getAvailableFirmwareFiles*

(Version 2) Gets a list of firmware files on the device. These files should start with the PMI or model number for that device and end with `.bin`. For example `HSTP11MWR_4.11p5s1_VBN_sdy.bin`.  
Firmware files are scanned in the root directories. If multiple USB devices are found, then the available firmware files are checked and listed from each device.
 
### Events 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.availableFirmwareFiles | array | The list of firmware files including the full path name |
| result.availableFirmwareFiles[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UsbAccess.1.getAvailableFirmwareFiles"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "availableFirmwareFiles": [
            "/tmp/usbmnts/sda1/HSTP11MWR_4.11p5s1_VBN_sdy.bin"
        ],
        "success": true
    }
}
```

<a name="getFileList"></a>
## *getFileList*

Gets a list of files and folders from the specified directory or path.
 
### Events 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.path | string | <sup>*(optional)*</sup> The directory name for which the contents are listed. If no value is specified, then the contents of the root folder is listed |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.contents | array | A list of files and directories (empty if there are no results) |
| result.contents[#] | object |  |
| result.contents[#].name | string | the name of the file or directory |
| result.contents[#].t | string | The type. Either `d` for directory or `f` for file |
| result.success | boolean | Whether the request succeeded |
| result?.error | string | <sup>*(optional)*</sup> An error message in case of a failure |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UsbAccess.1.getFileList",
    "params": {
        "path": "..."
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "contents": [
            {
                "name": "img1.jpg",
                "t": "f"
            }
        ],
        "success": true,
        "error": "no disk"
    }
}
```

<a name="getMounted"></a>
## *getMounted*

(Version 2) Returns a list of mounted USB devices.
 
### Events 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.mounted | array | The list of file paths where USB devices are mounted |
| result.mounted[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UsbAccess.1.getMounted"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "mounted": [
            "/tmp/usbmnts/sda"
        ],
        "success": true
    }
}
```

<a name="updateFirmware"></a>
## *updateFirmware*

(Version 2) Updates the firmware using the specified file retrieved from the `getAvailableFirmwareFiles` 
 
### Events 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.fileName | string | <sup>*(optional)*</sup> The firmware file path to use for the update |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |
| result?.error | string | <sup>*(optional)*</sup> An error message in case of a failure |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UsbAccess.1.updateFirmware",
    "params": {
        "fileName": "/tmp/usbmnts/sda/this_is_test.bin"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true,
        "error": "invalid filename"
    }
}
```

<a name="ArchiveLogs"></a>
## *ArchiveLogs*

(Version 2) Compresses and uploads device logs into attached USB drive from /opt/logs with a name comprises of Mac of the device , date and time in a `tgz` format. For example `18310C696834_Logs_10-13-21-04-42PM.tgz` `(<MAC address>_Logs_<unix epoch time>.tgz)`.
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onArchiveLogs`| Triggered to archive the device logs and returns the status of the archive |.

Also see: [onArchiveLogs](#onArchiveLogs)

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
    "method": "org.rdk.UsbAccess.1.ArchiveLogs"
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

The following events are provided by the org.rdk.UsbAccess plugin:

UsbAccess interface events:

| Event | Description |
| :-------- | :-------- |
| [onUSBMountChanged](#onUSBMountChanged) | (Version 2) Triggered when a USB drive is mounted or unmounted |
| [onArchiveLogs](#onArchiveLogs) | (Version 2) Triggered to archive the device logs and returns the status of the archive |


<a name="onUSBMountChanged"></a>
## *onUSBMountChanged*

(Version 2) Triggered when a USB drive is mounted or unmounted.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.mounted | boolean | `true` when the USB device is mounted or `false` when the USB device is unmounted |
| params.device | string | The location where the device is mounted |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onUSBMountChanged",
    "params": {
        "mounted": true,
        "device": "/dev/sda1"
    }
}
```

<a name="onArchiveLogs"></a>
## *onArchiveLogs*

(Version 2) Triggered to archive the device logs and returns the status of the archive.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.error | string | Specifies the status of upload logs (must be one of the following: *script error*, *none*, *Locked*, *No USB*, *Writing Error*) |
| params.success | boolean | Whether the request succeeded |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onArchiveLogs",
    "params": {
        "error": "none",
        "success": true
    }
}
```

