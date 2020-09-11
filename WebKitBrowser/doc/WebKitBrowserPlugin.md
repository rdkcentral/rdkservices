<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.WebKit_Browser_Plugin"></a>
# WebKit Browser Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

WebKitBrowser plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Methods](#head.Methods)
- [Properties](#head.Properties)
- [Notifications](#head.Notifications)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the WebKitBrowser plugin. It includes detailed specification of its configuration, methods and properties provided, as well as notifications sent.

<a name="head.Case_Sensitivity"></a>
## Case Sensitivity

All identifiers on the interface described in this document are case-sensitive. Thus, unless stated otherwise, all keywords, entities, properties, relations and actions should be treated as such.

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

The WebKitBrowser plugin provides web browsing functionality based on the WebKit engine.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *WebKitBrowser*) |
| classname | string | Class name: *WebKitBrowser* |
| locator | string | Library name: *libWPEFrameworkWebKitBrowser.so* |
| autostart | boolean | Determines if the plugin is to be started automatically along with the framework |
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
| configuration?.whitelist?.domain | array | <sup>*(optional)*</sup>  |
| configuration?.whitelist?.domain[#] | string | <sup>*(optional)*</sup> Domain allowed to access from origin |
| configuration?.whitelist?.subdomain | string | <sup>*(optional)*</sup> whether it is also OK to access subdomains of domains listed in domain |
| configuration?.localstorageenabled | boolean | <sup>*(optional)*</sup> Controls the local storage availability |
| configuration?.logtosystemconsoleenabled | boolean | <sup>*(optional)*</sup> Enable page loging to system console (stderr) |
| configuration?.watchdogchecktimeoutinseconds | number | <sup>*(optional)*</sup> How often to check main event loop for responsiveness (0 - disable) |
| configuration?.watchdoghangthresholdtinseconds | number | <sup>*(optional)*</sup> The amount of time to give a process to recover before declaring a hang state |
| configuration?.loadblankpageonsuspendenabled | boolean | <sup>*(optional)*</sup> Load 'about:blank' before suspending the page |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the WebKitBrowser plugin:

WebKitBrowser interface methods:

| Method | Description |
| :-------- | :-------- |
| [bridgereply](#method.bridgereply) | A response for legacy $badger |
| [bridgeevent](#method.bridgeevent) | Send legacy $badger event |

Browser interface methods:

| Method | Description |
| :-------- | :-------- |
| [delete](#method.delete) | Removes contents of a directory from the persistent storage |

<a name="method.bridgereply"></a>
## *bridgereply <sup>method</sup>*

A response for legacy $badger.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | string | A base64 encoded JSON string response to be delivered to $badger.callback(pid, success, json) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | null |  |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "WebKitBrowser.1.bridgereply",
    "params": ""
}
```
#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": null
}
```
<a name="method.bridgeevent"></a>
## *bridgeevent <sup>method</sup>*

Send legacy $badger event.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | string | A base64 encoded JSON string response to be delivered to window.$badger.event(handlerId, json) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | null |  |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "WebKitBrowser.1.bridgeevent",
    "params": ""
}
```
#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": null
}
```
<a name="method.delete"></a>
## *delete <sup>method</sup>*

Removes contents of a directory from the persistent storage.

### Description

Use this method to recursively delete contents of a directory

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.path | string | Path to directory (within the persistent storage) to delete contents of |

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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": null
}
```
<a name="head.Properties"></a>
# Properties

The following properties are provided by the WebKitBrowser plugin:

WebKitBrowser interface properties:

| Property | Description |
| :-------- | :-------- |
| [useragent](#property.useragent) | UserAgent string used by browser |
| [httpcookieacceptpolicy](#property.httpcookieacceptpolicy) | HTTP cookies accept policy |
| [localstorageenabled](#property.localstorageenabled) | Controls the local storage availability |
| [languages](#property.languages) | User preferred languages |
| [headers](#property.headers) | Headers to send on all requests that the browser makes |

Browser interface properties:

| Property | Description |
| :-------- | :-------- |
| [url](#property.url) | URL loaded in the browser |
| [visibility](#property.visibility) | Current browser visibility |
| [fps](#property.fps) <sup>RO</sup> | Current number of frames per second the browser is rendering |

StateControl interface properties:

| Property | Description |
| :-------- | :-------- |
| [state](#property.state) | Running state of the service |

<a name="property.useragent"></a>
## *useragent <sup>property</sup>*

Provides access to the userAgent string used by browser.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | UserAgent string used by browser |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "WebKitBrowser.1.useragent"
}
```
#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": "Mozilla/5.0 (Linux; x86_64 GNU/Linux) AppleWebKit/601.1 (KHTML, like Gecko) Version/8.0 Safari/601.1 WP"
}
```
#### Set Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "WebKitBrowser.1.useragent",
    "params": "Mozilla/5.0 (Linux; x86_64 GNU/Linux) AppleWebKit/601.1 (KHTML, like Gecko) Version/8.0 Safari/601.1 WP"
}
```
#### Set Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": "null"
}
```
<a name="property.httpcookieacceptpolicy"></a>
## *httpcookieacceptpolicy <sup>property</sup>*

Provides access to the HTTP cookies accept policy.

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
    "id": 1234567890,
    "method": "WebKitBrowser.1.httpcookieacceptpolicy"
}
```
#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": "always"
}
```
#### Set Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "WebKitBrowser.1.httpcookieacceptpolicy",
    "params": "always"
}
```
#### Set Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": "null"
}
```
<a name="property.localstorageenabled"></a>
## *localstorageenabled <sup>property</sup>*

Provides access to the controls the local storage availability.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | boolean | Controls the local storage availability |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "WebKitBrowser.1.localstorageenabled"
}
```
#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": false
}
```
#### Set Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "WebKitBrowser.1.localstorageenabled",
    "params": false
}
```
#### Set Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": "null"
}
```
<a name="property.languages"></a>
## *languages <sup>property</sup>*

Provides access to the user preferred languages.

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
    "id": 1234567890,
    "method": "WebKitBrowser.1.languages"
}
```
#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": [
        "en-US"
    ]
}
```
#### Set Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
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
    "id": 1234567890,
    "result": "null"
}
```
<a name="property.headers"></a>
## *headers <sup>property</sup>*

Provides access to the headers to send on all requests that the browser makes.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | Headers to send on all requests that the browser makes |
| (property)[#] | object |  |
| (property)[#]?.name | string | <sup>*(optional)*</sup> Header name |
| (property)[#]?.value | string | <sup>*(optional)*</sup> Header value |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "WebKitBrowser.1.headers"
}
```
#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": "null"
}
```
<a name="property.url"></a>
## *url <sup>property</sup>*

Provides access to the URL loaded in the browser.

Also see: [urlchange](#event.urlchange)

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
    "id": 1234567890,
    "method": "WebKitBrowser.1.url"
}
```
#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": "https://www.google.com"
}
```
#### Set Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "WebKitBrowser.1.url",
    "params": "https://www.google.com"
}
```
#### Set Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": "null"
}
```
<a name="property.visibility"></a>
## *visibility <sup>property</sup>*

Provides access to the current browser visibility.

Also see: [visibilitychange](#event.visibilitychange)

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | Current browser visibility (must be one of the following: *visible*, *hidden*) |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 2 | ```ERROR_UNAVAILABLE``` | Returned when the operation is unavailable |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "WebKitBrowser.1.visibility"
}
```
#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": "visible"
}
```
#### Set Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "WebKitBrowser.1.visibility",
    "params": "visible"
}
```
#### Set Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": "null"
}
```
<a name="property.fps"></a>
## *fps <sup>property</sup>*

Provides access to the current number of frames per second the browser is rendering.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | number | Current number of frames per second the browser is rendering |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "WebKitBrowser.1.fps"
}
```
#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": 30
}
```
<a name="property.state"></a>
## *state <sup>property</sup>*

Provides access to the running state of the service.

Also see: [statechange](#event.statechange)

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | Running state of the service (must be one of the following: *resumed*, *suspended*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "WebKitBrowser.1.state"
}
```
#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": "resumed"
}
```
#### Set Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "WebKitBrowser.1.state",
    "params": "resumed"
}
```
#### Set Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": "null"
}
```
<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers.Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the WebKitBrowser plugin:

WebKitBrowser interface events:

| Event | Description |
| :-------- | :-------- |
| [loadfinished](#event.loadfinished) | Initial HTML document has been completely loaded and parsed |
| [loadfailed](#event.loadfailed) | Browser failed to load page |
| [bridgequery](#event.bridgequery) | A Base64 encoded JSON message from legacy $badger bridge |

Browser interface events:

| Event | Description |
| :-------- | :-------- |
| [urlchange](#event.urlchange) | Signals a URL change in the browser |
| [visibilitychange](#event.visibilitychange) | Signals a visibility change of the browser |
| [pageclosure](#event.pageclosure) | Notifies that the web page requests to close its window |

StateControl interface events:

| Event | Description |
| :-------- | :-------- |
| [statechange](#event.statechange) | Signals a state change of the service |

<a name="event.loadfinished"></a>
## *loadfinished <sup>event</sup>*

Initial HTML document has been completely loaded and parsed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.url | string | The URL that has been loaded |
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
<a name="event.loadfailed"></a>
## *loadfailed <sup>event</sup>*

Browser failed to load page.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.url | string | The URL that browser failed to load |

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
<a name="event.bridgequery"></a>
## *bridgequery <sup>event</sup>*

A Base64 encoded JSON message from legacy $badger bridge.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | string |  |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.bridgequery",
    "params": ""
}
```
<a name="event.urlchange"></a>
## *urlchange <sup>event</sup>*

Signals a URL change in the browser.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.url | string | The URL that has been loaded or requested |
| params.loaded | boolean | Determines if the URL has just been loaded (true) or if URL change has been requested (false) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.urlchange",
    "params": {
        "url": "https://www.google.com",
        "loaded": false
    }
}
```
<a name="event.visibilitychange"></a>
## *visibilitychange <sup>event</sup>*

Signals a visibility change of the browser.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.hidden | boolean | Determines if the browser has been hidden (true) or made visible (false) |

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
<a name="event.pageclosure"></a>
## *pageclosure <sup>event</sup>*

Notifies that the web page requests to close its window.

### Parameters

This event carries no parameters.

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.pageclosure"
}
```
<a name="event.statechange"></a>
## *statechange <sup>event</sup>*

Signals a state change of the service.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.suspended | boolean | Determines if the service has entered suspended state (true) or resumed state (false) |

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
