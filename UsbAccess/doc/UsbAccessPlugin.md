<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.UsbAccess_Plugin"></a>
# UsbAccess Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

org.rdk.UsbAccess plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.UsbAccess plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `UsbAccess` plugin provides the ability to examine the contents on a USB drive and access the content through HTTP URLs.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.UsbAccess*) |
| classname | string | Class name: *org.rdk.UsbAccess* |
| locator | string | Library name: *libWPEFrameworkUsbAccess.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.UsbAccess plugin:

UsbAccess interface methods:

| Method | Description |
| :-------- | :-------- |
| [clearLink](#method.clearLink) | Clears or removes the symbolic link created by the `createLink` method |
| [createLink](#method.createLink) | Creates a symbolic link to the root folder of the USB drive |
| [getAvailableFirmwareFiles](#method.getAvailableFirmwareFiles) | (Version 2) Gets a list of firmware files on the device |
| [getFileList](#method.getFileList) | Gets a list of files and folders from the specified directory or path |
| [getMounted](#method.getMounted) | (Version 2) Returns a list of mounted USB devices |
| [updateFirmware](#method.updateFirmware) | (Version 2) Updates the firmware using the specified file retrieved from the `getAvailableFirmwareFiles` method |


<a name="method.clearLink"></a>
## *clearLink <sup>method</sup>*

Clears or removes the symbolic link created by the `createLink` method.

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
    "id": 1234567890,
    "method": "org.rdk.UsbAccess.1.clearLink"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "error": "could not remove symlink"
    }
}
```

<a name="method.createLink"></a>
## *createLink <sup>method</sup>*

Creates a symbolic link to the root folder of the USB drive. If called, and a link already exists, then it errors out. Symbolic link has read-only access. Use the name `usbdrive`.

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
    "id": 1234567890,
    "method": "org.rdk.UsbAccess.1.createLink"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "baseURL": "http://localhost/usbdrive",
        "success": true,
        "error": "could not create symlink"
    }
}
```

<a name="method.getAvailableFirmwareFiles"></a>
## *getAvailableFirmwareFiles <sup>method</sup>*

(Version 2) Gets a list of firmware files on the device. These files should start with the PMI or model number for that device and end with `.bin`. For example `HSTP11MWR_4.11p5s1_VBN_sdy.bin`.  
Firmware files are scanned in the root directories. If multiple USB devices are found, then the available firmware files are checked and listed from each device.

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
    "id": 1234567890,
    "method": "org.rdk.UsbAccess.1.getAvailableFirmwareFiles"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "availableFirmwareFiles": [
            "/tmp/usbmnts/sda1/HSTP11MWR_4.11p5s1_VBN_sdy.bin"
        ],
        "success": true
    }
}
```

<a name="method.getFileList"></a>
## *getFileList <sup>method</sup>*

Gets a list of files and folders from the specified directory or path.

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
    "id": 1234567890,
    "method": "org.rdk.UsbAccess.1.getFileList",
    "params": {
        "path": ""
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
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

<a name="method.getMounted"></a>
## *getMounted <sup>method</sup>*

(Version 2) Returns a list of mounted USB devices.

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
    "id": 1234567890,
    "method": "org.rdk.UsbAccess.1.getMounted"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "mounted": [
            "/tmp/usbmnts/sda"
        ],
        "success": true
    }
}
```

<a name="method.updateFirmware"></a>
## *updateFirmware <sup>method</sup>*

(Version 2) Updates the firmware using the specified file retrieved from the `getAvailableFirmwareFiles` method.

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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": {
        "success": true,
        "error": "invalid filename"
    }
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.UsbAccess plugin:

UsbAccess interface events:

| Event | Description |
| :-------- | :-------- |
| [onUsbMountChanged](#event.onUsbMountChanged) | (Version 2) Triggered when a USB drive is mounted or unmounted |


<a name="event.onUsbMountChanged"></a>
## *onUsbMountChanged <sup>event</sup>*

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
    "method": "client.events.1.onUsbMountChanged",
    "params": {
        "mounted": true,
        "device": "/dev/sda1"
    }
}
```

