<!-- Generated automatically, DO NOT EDIT! -->
<a name="MigrationPreparer_Plugin"></a>
# MigrationPreparer Plugin

**Version: [1.0.0](https://github.com/rdkcentral/rdkservices/blob/main/MigrationPreparer/CHANGELOG.md)**

A org.rdk.MigrationPreparer plugin for Thunder framework.

### Table of Contents

- [Abbreviation, Acronyms and Terms](#Abbreviation,_Acronyms_and_Terms)
- [Description](#Description)
- [Configuration](#Configuration)
- [Methods](#Methods)

<a name="Abbreviation,_Acronyms_and_Terms"></a>
# Abbreviation, Acronyms and Terms

[[Refer to this link](userguide/aat.md)]

<a name="Description"></a>
# Description

The `MigrationPreparer` that is responsible for persisting and notifying listeners of any change of key/value pairs and get/set value of MigrationReady RFC value.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.MigrationPreparer*) |
| classname | string | Class name: *org.rdk.MigrationPreparer* |
| locator | string | Library name: *libWPEFrameworkMigrationPreparer.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.MigrationPreparer plugin:

MigrationPreparer interface methods:

| Method | Description |
| :-------- | :-------- |
| [write](#write) | Write key-value to the dataStore |
| [Delete](#Delete) | Delete key-value from the dataStore |
| [read](#read) | Read value from the dataStore |
| [getcomponentreadiness](#getcomponentreadiness) | Returns the list of components that are ready for migration |
| [setcomponentreadiness](#setcomponentreadiness) | Set the component that is ready for migration |
| [reset](#reset) | Reset based on resettype, should delete all entries in the datastore or componentList for migration or both |


<a name="write"></a>
## *write*

Write key-value to the dataStore.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.name | string | key |
| params.value | string | value |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Legacy parameter (always true) |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
