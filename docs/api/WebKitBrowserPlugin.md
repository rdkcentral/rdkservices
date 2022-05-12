<!-- Generated automatically, DO NOT EDIT! -->
<a name="WebKitBrowser_Plugin"></a>
# WebKitBrowser Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A WebKitBrowser plugin for Thunder framework.

### Table of Contents

- [Introduction](#Introduction)
- [Description](#Description)
- [Configuration](#Configuration)
- [Methods](#Methods)
- [Properties](#Properties)
- [Notifications](#Notifications)

<a name="Introduction"></a>
# Introduction

<a name="Scope"></a>
## Scope

This document describes purpose and functionality of the WebKitBrowser plugin. It includes detailed specification about its configuration, methods and properties provided, as well as notifications sent.

<a name="Case_Sensitivity"></a>
## Case Sensitivity

All identifiers of the interfaces described in this document are case-sensitive. Thus, unless stated otherwise, all keywords, entities, properties, relations and actions should be treated as such.

<a name="Acronyms,_Abbreviations_and_Terms"></a>
## Acronyms, Abbreviations and Terms

The table below provides and overview of acronyms used in this document and their definitions.

| Acronym | Description |
| :-------- | :-------- |
| <a name="API">API</a> | Application Programming Interface |
| <a name="HTTP">HTTP</a> | Hypertext Transfer Protocol |
| <a name="JSON">JSON</a> | JavaScript Object Notation; a data interchange format |
| <a name="JSON-RPC">JSON-RPC</a> | A remote procedure call protocol encoded in JSON |

The table below provides and overview of terms and abbreviations used in this document and their definitions.

| Term | Description |
| :-------- | :-------- |
| <a name="callsign">callsign</a> | The name given to an instance of a plugin. One plugin can be instantiated multiple times, but each instance the instance name, callsign, must be unique. |

<a name="References"></a>
## References

| Ref ID | Description |
| :-------- | :-------- |
| <a name="HTTP">[HTTP](http://www.w3.org/Protocols)</a> | HTTP specification |
| <a name="JSON-RPC">[JSON-RPC](https://www.jsonrpc.org/specification)</a> | JSON-RPC 2.0 specification |
| <a name="JSON">[JSON](http://www.json.org/)</a> | JSON specification |
| <a name="Thunder">[Thunder](https://github.com/WebPlatformForEmbedded/Thunder/blob/master/doc/WPE%20-%20API%20-%20WPEFramework.docx)</a> | Thunder API Reference |

<a name="Description"></a>
# Description

The `WebKitBrowser` plugin provides web browsing functionality based on the WebKit engine.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *WebKitBrowser*) |
| classname | string | Class name: *WebKitBrowser* |
| locator | string | Library name: *libWPEFrameworkWebKitBrowser.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |
| configuration | object | <sup>*(optional)*</sup>  |
| configuration?.useragent | string | <sup>*(optional)*</sup> The UserAgent used during communication with the web server |
| configuration?.url | string | <sup>*(optional)*</sup> The URL that is loaded upon starting the browser |
| configuration?.injectedbundle | string | <sup>*(optional)*</sup> The name of the .so loaded to extend the HTML5 with customized JavaScript handlers |
| configuration?.cookiestorage | string | <sup>*(optional)*</sup> Post-fixed path to the persistent location on where cookies will be stored |
| configuration?.localstorage | string | <sup>*(optional)*</sup> Post-fixed path to the persistent location on where local-storage will be stored |
| configuration?.certificatecheck | boolean | <sup>*(optional)*</sup> If enabled, SSL certificate error will lead to a page error |
| configuration?.javascript | object | <sup>*(optional)*</sup>  |
| configuration?.javascript?.useLLInt | boolean | <sup>*(optional)*</sup> Enable Low Level Interpreter |
| configuration?.javascript?.useJIT | boolean | <sup>*(optional)*</sup> Enable JIT |
| configuration?.javascript?.useDFG | boolean | <sup>*(optional)*</sup> Enable Data-Flow-Graph-JIT compiler |
| configuration?.javascript?.useFTL | boolean | <sup>*(optional)*</sup> Enable Faster-Than-Light-JIT compiler |
| configuration?.javascript?.useDOM | boolean | <sup>*(optional)*</sup> Enable the DOM-JIT compiler |
| configuration?.javascript?.dumpOptions | string | <sup>*(optional)*</sup> Dump options: 0 = None, 1 = Overridden only, 2 = All, 3 = Verbose |
| configuration?.secure | boolean | <sup>*(optional)*</sup> If false, every domain can access any other domain, otherwise fill in fields in whitelist definition table |
| configuration?.whitelist | object | <sup>*(optional)*</sup>  |
| configuration?.whitelist?.origin | string | <sup>*(optional)*</sup> Origin domain allowed to access domains in domain |
| configuration?.whitelist?.domain | array | <sup>*(optional)*</sup> Domain allowed to access from origin |
| configuration?.whitelist?.domain[#] | string | <sup>*(optional)*</sup>  |
| configuration?.whitelist?.subdomain | string | <sup>*(optional)*</sup> whether it is also OK to access subdomains of domains listed in domain |
| configuration?.localstorageenabled | boolean | <sup>*(optional)*</sup> Controls the local storage availability |
| configuration?.logtosystemconsoleenabled | boolean | <sup>*(optional)*</sup> Enable page loging to system console (stderr) |
| configuration?.watchdogchecktimeoutinseconds | number | <sup>*(optional)*</sup> How often to check main event loop for responsiveness (0 - disable) |
| configuration?.watchdoghangthresholdtinseconds | number | <sup>*(optional)*</sup> The amount of time to give a process to recover before declaring a hang state |
| configuration?.loadblankpageonsuspendenabled | boolean | <sup>*(optional)*</sup> Load 'about:blank' before suspending the page |

<a name="Methods"></a>
# Methods

The following methods are provided by the WebKitBrowser plugin:

WebKitBrowser interface methods:

| Method | Description |
| :-------- | :-------- |
| [bridgeevent](#bridgeevent) | Sends a legacy `$badger` event |
| [bridgereply](#bridgereply) | A response for legacy `$badger` |
| [delete](#delete) | Removes the contents of a directory recursively from the persistent storage |


<a name="bridgeevent"></a>
## *bridgeevent*

Sends a legacy `$badger` 
 
### Events 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | string | A base64 encoded JSON string response to be delivered to the `window.$badger.event(handlerId, json)` method |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | null | Always null |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "WebKitBrowser.1.bridgeevent",
    "params": "..."
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

<a name="bridgereply"></a>
## *bridgereply*

A response for legacy `$badger`.
 
### Events 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | string | A base64 encoded JSON string response to be delivered to the `$badger.callback(pid, success, json)` method |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | null | Always null |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "WebKitBrowser.1.bridgereply",
    "params": "..."
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

<a name="delete"></a>
## *delete*

Removes the contents of a directory recursively from the persistent storage.
 
### Events 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.path | string | Path to directory (within the persistent storage) from which to delete contents |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | null | Always null |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 22 | ```ERROR_UNKNOWN_KEY``` | The given path was incorrect |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "WebKitBrowser.1.delete",
    "params": {
        "path": ".cache/wpe/disk-cache"
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

<a name="Properties"></a>
# Properties

The following properties are provided by the WebKitBrowser plugin:

WebKitBrowser interface properties:

| Property | Description |
| :-------- | :-------- |
| [fps](#fps) <sup>RO</sup> | Current number of frames-per-second the browser is rendering |
| [headers](#headers) | Headers to send on all requests that the browser makes |
| [httpcookieacceptpolicy](#httpcookieacceptpolicy) | HTTP cookies accept policy |
| [languages](#languages) | User preferred languages |
| [localstorageenabled](#localstorageenabled) | Local storage availability |
| [state](#state) | Running state of the service |
| [url](#url) | URL loaded in the browser |
| [useragent](#useragent) | `UserAgent` string used by the browser |
| [visibility](#visibility) | Current browser visibility |


<a name="fps"></a>
## *fps [<sup>property</sup>](#Properties)*

Provides access to the current number of frames-per-second the browser is rendering.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | number | Current number of frames-per-second the browser is rendering |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "WebKitBrowser.1.fps"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": 30
}
```

<a name="headers"></a>
## *headers [<sup>property</sup>](#Properties)*

Provides access to the headers to send on all requests that the browser makes.

### Description

Use this property to send on all requests that the browser makes.
 
### Events 

 No Events.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | Headers to send on all requests that the browser makes |
| (property)[#] | object |  |
| (property)[#]?.name | string | <sup>*(optional)*</sup> The header name |
| (property)[#]?.value | string | <sup>*(optional)*</sup> The header value |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "WebKitBrowser.1.headers"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": [
        {
            "name": "X-Forwarded-For",
            "value": "::1"
        }
    ]
}
```

#### Set Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "WebKitBrowser.1.headers",
    "params": [
        {
            "name": "X-Forwarded-For",
            "value": "::1"
        }
    ]
}
```

#### Set Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "null"
}
```

<a name="httpcookieacceptpolicy"></a>
## *httpcookieacceptpolicy [<sup>property</sup>](#Properties)*

Provides access to the HTTP cookies accept policy.

### Description

Use this property to accept HTTP cookie policy.
 
### Events 

 No Events.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | HTTP cookies accept policy (must be one of the following: *always*, *never*, *onlyfrommaindocumentdomain*, *exclusivelyfrommaindocumentdomain*) |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 22 | ```ERROR_UNKNOWN_KEY``` | Unknown policy |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "WebKitBrowser.1.httpcookieacceptpolicy"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "always"
}
```

#### Set Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "WebKitBrowser.1.httpcookieacceptpolicy",
    "params": "always"
}
```

#### Set Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "null"
}
```

<a name="languages"></a>
## *languages [<sup>property</sup>](#Properties)*

Provides access to the user preferred languages.

### Description

Use this property to return User preferred languages.
 
### Events 

 No Events.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | User preferred languages |
| (property)[#] | string |  |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "WebKitBrowser.1.languages"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": [
        "en-US"
    ]
}
```

#### Set Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "WebKitBrowser.1.languages",
    "params": [
        "en-US"
    ]
}
```

#### Set Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "null"
}
```

<a name="localstorageenabled"></a>
## *localstorageenabled [<sup>property</sup>](#Properties)*

Provides access to the local storage availability.

### Description

Use this property to return Local storage availability.
 
### Events 

 No Events.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | boolean | Local storage availability |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "WebKitBrowser.1.localstorageenabled"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": false
}
```

#### Set Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "WebKitBrowser.1.localstorageenabled",
    "params": false
}
```

#### Set Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "null"
}
```

<a name="state"></a>
## *state [<sup>property</sup>](#Properties)*

Provides access to the running state of the service.

### Description

Use this property to return the running state of the service.
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `statechange`| Triggered if the state of the service changes.|

Also see: [statechange](#statechange)

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | Running state of the service (must be one of the following: *resumed*, *suspended*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "WebKitBrowser.1.state"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "resumed"
}
```

#### Set Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "WebKitBrowser.1.state",
    "params": "resumed"
}
```

#### Set Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "null"
}
```

<a name="url"></a>
## *url [<sup>property</sup>](#Properties)*

Provides access to the URL loaded in the browser.

### Description

Use this property to load URL in the browser.
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `urlchange`| Triggered if the URL changes in the browser | 
 |`loadfinished`| Triggered if the `urlchange` event returns `true` as URL loaded successfully |
 |`loadfailed`| Triggered if the `urlchange` event returns `false` as URL failed to load |

Also see: [urlchange](#urlchange)

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | URL loaded in the browser |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 15 | ```ERROR_INCORRECT_URL``` | Incorrect URL given |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "WebKitBrowser.1.url"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "https://www.google.com"
}
```

#### Set Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "WebKitBrowser.1.url",
    "params": "https://www.google.com"
}
```

#### Set Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "null"
}
```

<a name="useragent"></a>
## *useragent [<sup>property</sup>](#Properties)*

Provides access to the `UserAgent` string used by the browser.

### Description

Use this property to return `UserAgent` string used by the browser.
 
### Events 

 No Events. 

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | `UserAgent` string used by the browser |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "WebKitBrowser.1.useragent"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "Mozilla/5.0 (Linux; x86_64 GNU/Linux) AppleWebKit/601.1 (KHTML, like Gecko) Version/8.0 Safari/601.1 WP"
}
```

#### Set Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "WebKitBrowser.1.useragent",
    "params": "Mozilla/5.0 (Linux; x86_64 GNU/Linux) AppleWebKit/601.1 (KHTML, like Gecko) Version/8.0 Safari/601.1 WP"
}
```

#### Set Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "null"
}
```

<a name="visibility"></a>
## *visibility [<sup>property</sup>](#Properties)*

Provides access to the current browser visibility.

### Description

Use this property to return visibilty status of current browser.
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `visibilitychange`| Triggered if the browser visibility changes.|

Also see: [visibilitychange](#visibilitychange)

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | Current browser visibility |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 2 | ```ERROR_UNAVAILABLE``` | Returned when the operation is unavailable |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "WebKitBrowser.1.visibility"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "visible"
}
```

#### Set Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "WebKitBrowser.1.visibility",
    "params": "visible"
}
```

#### Set Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "null"
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the WebKitBrowser plugin:

WebKitBrowser interface events:

| Event | Description |
| :-------- | :-------- |
| [bridgequery](#bridgequery) | A Base64 encoded JSON message from legacy `$badger` bridge |
| [loadfailed](#loadfailed) | Triggered when the browser fails to load a page |
| [loadfinished](#loadfinished) | Triggered when the initial HTML document has been completely loaded and parsed |
| [pageclosure](#pageclosure) | Triggered when the web page requests to close its window |
| [statechange](#statechange) | Triggered when the state of the service changes |
| [urlchange](#urlchange) | Triggered when the URL changes in the browser |
| [visibilitychange](#visibilitychange) | Triggered when the browser visibility changes |


<a name="bridgequery"></a>
## *bridgequery*

A Base64 encoded JSON message from legacy `$badger` bridge.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | string |  |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.bridgequery",
    "params": "..."
}
```

<a name="loadfailed"></a>
## *loadfailed*

Triggered when the browser fails to load a page.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.url | string | The URL |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.loadfailed",
    "params": {
        "url": "https://example.com"
    }
}
```

<a name="loadfinished"></a>
## *loadfinished*

Triggered when the initial HTML document has been completely loaded and parsed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.url | string | The URL |
| params.httpstatus | integer | The response code of main resource request |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.loadfinished",
    "params": {
        "url": "https://example.com",
        "httpstatus": 200
    }
}
```

<a name="pageclosure"></a>
## *pageclosure*

Triggered when the web page requests to close its window.

### Parameters

This event carries no parameters.

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.pageclosure"
}
```

<a name="statechange"></a>
## *statechange*

Triggered when the state of the service changes.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.suspended | boolean | `true` if the service has entered a suspended state  or `false` if the service has entered a resumed state |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.statechange",
    "params": {
        "suspended": false
    }
}
```

<a name="urlchange"></a>
## *urlchange*

Triggered when the URL changes in the browser.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.url | string | The URL |
| params.loaded | boolean | `true` if the URL has been loaded or `false` if the URL change has been requested |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.urlchange",
    "params": {
        "url": "https://example.com",
        "loaded": false
    }
}
```

<a name="visibilitychange"></a>
## *visibilitychange*

Triggered when the browser visibility changes.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.hidden | boolean | `true` if the browser has been hidden or `false` if the browser is made visible |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.visibilitychange",
    "params": {
        "hidden": false
    }
}
```

