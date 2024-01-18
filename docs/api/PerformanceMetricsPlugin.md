<!-- Generated automatically, DO NOT EDIT! -->
<a name="PerformanceMetrics_Plugin"></a>
# PerformanceMetrics Plugin

**Version: [1.0.0](https://github.com/rdkcentral/rdkservices/blob/main/PerformanceMetrics/CHANGELOG.md)**

A PerformanceMetrics plugin for Thunder framework.

### Table of Contents

- [Abbreviation, Acronyms and Terms](#Abbreviation,_Acronyms_and_Terms)
- [Description](#Description)
- [Configuration](#Configuration)

<a name="Abbreviation,_Acronyms_and_Terms"></a>
# Abbreviation, Acronyms and Terms

[[Refer to this link](userguide/aat.md)]

<a name="Description"></a>
# Description

The Performance Metrics plugin can output metrics on a plugin (e.g. uptime, resource usage).

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *PerformanceMetrics*) |
| classname | string | Class name: *PerformanceMetrics* |
| locator | string | Library name: *libWPEFrameworkPerformanceMetrics.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

