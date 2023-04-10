<!-- Generated automatically, DO NOT EDIT! -->
<a name="RemoteControl_Plugin"></a>
# RemoteControl Plugin

**Version: [1.0.0](https://github.com/rdkcentral/rdkservices/blob/main/RemoteControl/CHANGELOG.md)**

A org.rdk.RemoteControl plugin for Thunder framework.

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

The `RemoteControl` plugin provides the ability to pair and IR-program remote controls.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.RemoteControl*) |
| classname | string | Class name: *org.rdk.RemoteControl* |
| locator | string | Library name: *libWPEFrameworkRemoteControl.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.RemoteControl plugin:

RemoteControl interface methods:

| Method | Description |
| :-------- | :-------- |
| [clearIRCodes](#clearIRCodes) | Clears the IR codes from the specified remote |
| [configureWakeupKeys](#configureWakeupKeys) | Configures which keys on the remote will wake the target from deepsleep |
| [getApiVersionNumber](#getApiVersionNumber) | Gets the current API version number |
| [getIRCodesByAutoLookup](#getIRCodesByAutoLookup) | Returns a list of available IR codes for the TV and AVRs specified by the input parameters |
| [getIRCodesByNames](#getIRCodesByNames) | Returns a list of IR codes for the AV device specified by the input parameters |
| [getLastKeypressSource](#getLastKeypressSource) | Returns last key press source data |
| [getIRDBManufacturers](#getIRDBManufacturers) | Returns a list of manufacturer names based on the specified input parameters |
| [getIRDBModels](#getIRDBModels) | Returns a list of model names based on the specified input parameters |
| [getNetStatus](#getNetStatus) | Returns the status information provided by the last `onStatus` event for the specified network |
| [startPairing](#startPairing) | Initiates pairing a remote with the STB on the specified network |
| [setIRCode](#setIRCode) | Programs an IR code into the specified remote control |
| [initializeIRDB](#initializeIRDB) | Initializes the IR database |


<a name="clearIRCodes"></a>
## *clearIRCodes*

Clears the IR codes from the specified remote.
 
Events
 
| Event | Description | 
| :-------- | :-------- | 
| `onStatus` | Triggered if the IR codes cleared from remote on the specified network |.

### Events

| Event | Description |
| :-------- | :-------- |
