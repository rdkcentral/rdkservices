<!-- Generated automatically, DO NOT EDIT! -->
<a name="CloudStore_Plugin"></a>
# CloudStore Plugin

**Version: [1.0.0](https://github.com/rdkcentral/rdkservices/blob/main/CloudStore/CHANGELOG.md)**

A org.rdk.CloudStore plugin for Thunder framework.

### Table of Contents

- [Abbreviation, Acronyms and Terms](#Abbreviation,_Acronyms_and_Terms)
- [Description](#Description)
- [Configuration](#Configuration)

<a name="Abbreviation,_Acronyms_and_Terms"></a>
# Abbreviation, Acronyms and Terms

[[Refer to this link](userguide/aat.md)]

<a name="Description"></a>
# Description

The `CloudStore` plugin allows you to persist key/value pairs by namespace.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.CloudStore*) |
| classname | string | Class name: *org.rdk.CloudStore* |
| locator | string | Library name: *libWPEFrameworkCloudStore.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

