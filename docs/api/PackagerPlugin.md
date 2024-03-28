<!-- Generated automatically, DO NOT EDIT! -->
<a name="Packager_Plugin"></a>
# Packager Plugin

**Version: [1.0.4](https://github.com/rdkcentral/rdkservices/blob/main/Packager/CHANGELOG.md)**

A Packager plugin for Thunder framework.

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

The `Packager` plugin allows installation of OPKG, IPKG, and DEB packages to the system from a remote repository.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *Packager*) |
| classname | string | Class name: *Packager* |
| locator | string | Library name: *libWPEFrameworkPackager.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the Packager plugin:

Packager interface methods:

| Method | Description |
| :-------- | :-------- |
| [install](#install) | Installs a package given by a name, a URL, or a file path |
| [synchronize](#synchronize) | Synchronizes the repository manifest with a repository |


<a name="install"></a>
## *install*

Installs a package given by a name, a URL, or a file path.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.package | string | A name, a URL, or a file path of the package to install |
| params?.version | string | <sup>*(optional)*</sup> Version of the package to install |
| params?.architecture | string | <sup>*(optional)*</sup> Architecture of the package to install |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | null | Always null |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 12 | ```ERROR_INPROGRESS``` | Returned when the function is called while other installation/synchronization is already in progress |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "Packager.install",
    "params": {
        "package": "wpeframework-plugin-netflix",
        "version": "1.0",
        "architecture": "arm"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": null
}
```

<a name="synchronize"></a>
## *synchronize*

Synchronizes the repository manifest with a repository.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | null | Always null |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 12 | ```ERROR_INPROGRESS``` | Returned when the function is called while other installation/synchronization is already in progress. |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "Packager.synchronize"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": null
}
```

