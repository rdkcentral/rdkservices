<!-- Generated automatically, DO NOT EDIT! -->
<a name="SecurityAgent_Plugin"></a>
# SecurityAgent Plugin

**Version: [1.1.5](https://github.com/rdkcentral/rdkservices/blob/main/SecurityAgent/CHANGELOG.md)**

A SecurityAgent plugin for Thunder framework.

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

The `SecurityAgent` plugin is responsible for allowing or blocking access to the Thunder APIs.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *SecurityAgent*) |
| classname | string | Class name: *SecurityAgent* |
| locator | string | Library name: *libWPEFrameworkSecurityAgent.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |
| configuration | object | <sup>*(optional)*</sup>  |
| configuration?.acl | string | <sup>*(optional)*</sup> ACL |
| configuration?.connector | string | <sup>*(optional)*</sup> Connector |

<a name="Methods"></a>
# Methods

The following methods are provided by the SecurityAgent plugin:

SecurityAgent interface methods:

| Method | Description |
| :-------- | :-------- |
| [createtoken](#createtoken) | Creates a signed JsonWeb token |
| [validate](#validate) | Validates the token whether it is valid and properly signed |


<a name="createtoken"></a>
## *createtoken*

Creates a signed JsonWeb token. On success, returns Signed JsonWeb token and on failure, returns error message and error code as mentioned in below Errors table.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.url | string | Url of application origin |
| params.user | string | The user name |
| params.hash | string | A random hash |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.token | string | Signed JsonWeb token |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | Token creation failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "SecurityAgent.createtoken",
    "params": {
        "url": "https://test.comcast.com",
        "user": "Test",
        "hash": "1CLYex47SY"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.ewogICAgImpzb25ycGMiOiAiMi4wIiwgCiAgICAiaWQiOiAxMjM0NTY3ODkwLCAKICAgICJtZXRob2QiOiAiQ29udHJvbGxlci4xLmFjdGl2YXRlIiwgCiAgICAicGFyYW1zIjogewogICAgICAgICJjYWxsc2lnbiI6ICJTZWN1cml0eUFnZW50IgogICAgfQp9.lL40nTwRyBvMwiglZhl5_rB8ycY1uhAJRFx9pGATMRQ"
    }
}
```

<a name="validate"></a>
## *validate*

Validates the token whether it is valid and properly signed.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.token | string | Token to validate |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.valid | boolean | Whether the token is signature is correct |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "SecurityAgent.validate",
    "params": {
        "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.ewogICAgImpzb25ycGMiOiAiMi4wIiwgCiAgICAiaWQiOiAxMjM0NTY3ODkwLCAKICAgICJtZXRob2QiOiAiQ29udHJvbGxlci4xLmFjdGl2YXRlIiwgCiAgICAicGFyYW1zIjogewogICAgICAgICJjYWxsc2lnbiI6ICJTZWN1cml0eUFnZW50IgogICAgfQp9.lL40nTwRyBvMwiglZhl5_rB8ycY1uhAJRFx9pGATMRQ"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "valid": false
    }
}
```

