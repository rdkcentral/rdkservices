<!-- Generated automatically, DO NOT EDIT! -->
<a name="DeviceIdentification_Plugin"></a>
# DeviceIdentification Plugin

**Version: 1.0.0**

A DeviceIdentification plugin for Thunder framework.

### Table of Contents

- [Abbreviation, Acronyms and Terms](#Abbreviation,_Acronyms_and_Terms)
- [Description](#Description)
- [Configuration](#Configuration)
- [Properties](#Properties)

<a name="Abbreviation,_Acronyms_and_Terms"></a>
# Abbreviation, Acronyms and Terms

[[Refer to this link](userguide/aat.md)]

<a name="Description"></a>
# Description

The `DeviceIdentification` plugin allows you to retrieve various device-related information.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *DeviceIdentification*) |
| classname | string | Class name: *DeviceIdentification* |
| locator | string | Library name: *libWPEFrameworkDeviceIdentification.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Properties"></a>
# Properties

The following properties are provided by the DeviceIdentification plugin:

DeviceIdentification interface properties:

| Property | Description |
| :-------- | :-------- |
| [deviceidentification](#deviceidentification) <sup>RO</sup> | Device platform specific information |


<a name="deviceidentification"></a>
## *deviceidentification*

Provides access to the device platform specific information.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Device platform specific information |
| (property).firmwareversion | string | Version of the device firmware |
| (property).chipset | string | Chipset used for this device |
| (property)?.identifier | string | <sup>*(optional)*</sup> Device unique identifier |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceIdentification.1.deviceidentification"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "firmwareversion": "1.0.0",
        "chipset": "BCM2711",
        "identifier": "WPEuCfrLF45"
    }
}
```

