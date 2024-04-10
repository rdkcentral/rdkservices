<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.L2Tests_API"></a>
# L2Tests API

**Version: [1.0.0](https://github.com/rdkcentral/rdkservices/blob/main/L2Tests/CHANGELOG.md)**

A L2Tests plugin for Thunder framework.

### Table of Contents

- [Abbreviation, Acronyms and Terms](#head.Abbreviation,_Acronyms_and_Terms)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Methods](#head.Methods)

<a name="head.Abbreviation,_Acronyms_and_Terms"></a>
# Abbreviation, Acronyms and Terms

[[Refer to this link](userguide/aat.md)]

<a name="head.Description"></a>
# Description

The L2Tests plugin provides an interface to start L2 tests.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| classname | string | Class name: *L2Tests* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the L2Tests plugin:

L2Tests interface methods:

| Method | Description |
| :-------- | :-------- |
| [PerformL2Tests](#method.PerformL2Tests) | Start L2 google tests for all RDKServices |


<a name="method.PerformL2Tests"></a>
## *PerformL2Tests [<sup>method</sup>](#head.Methods)*

Start L2 google tests for all RDKServices.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.test_suite_list | string | RDK service name |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.Success | integer | Value zero for no error |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "L2Tests.PerformL2Tests",
    "params": {
        "test_suite_list": "System"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "Success": 0
    }
}
```

