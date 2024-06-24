<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.UserSettings_Plugin"></a>
# UserSettings Plugin

**Version: [1.0.0](https://github.com/rdkcentral/rdkservices/blob/main/UserSettings/CHANGELOG.md)**

A org.rdk.UserSettings plugin for Thunder framework.

### Table of Contents

- [Abbreviation, Acronyms and Terms](#head.Abbreviation,_Acronyms_and_Terms)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Methods](#head.Methods)
- [Notifications](#head.Notifications)

<a name="head.Abbreviation,_Acronyms_and_Terms"></a>
# Abbreviation, Acronyms and Terms

[[Refer to this link](userguide/aat.md)]

<a name="head.Description"></a>
# Description

The `UserSettings` that is responsible for persisting and notifying listeners of any change of these settings.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.UserSettings*) |
| classname | string | Class name: *org.rdk.UserSettings* |
| locator | string | Library name: *libWPEFrameworkUserSettings.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.UserSettings plugin:

UserSettings interface methods:

| Method | Description |
| :-------- | :-------- |
| [setaudiodescription](#method.setaudiodescription) | Setting Audio Description |
| [setpreferredaudiolanguages](#method.setpreferredaudiolanguages) | Setting Preferred Audio Languages |
| [setpresentationlanguages](#method.setpresentationlanguages) | Setting Presentation Languages |
| [setcaptions](#method.setcaptions) | Setting Captions |
| [setpreferredcaptionlanguages](#method.setpreferredcaptionlanguages) | Setting PreferredCaption Languages |
| [setpreferredclosedcaptionservice](#method.setpreferredclosedcaptionservice) | Setting Preferred Closed Caption Service |
| [getaudiodescription](#method.getaudiodescription) | Returns Audio Description |
| [getpreferredaudiolanguages](#method.getpreferredaudiolanguages) | Returns Audio Description |
| [getpresentationlanguages](#method.getpresentationlanguages) | Getting Presentation Languages |
| [getcaptions](#method.getcaptions) | Getting Captions Enabled |
| [getpreferredcaptionslanguages](#method.getpreferredcaptionslanguages) | Getting Preferred Caption Languages |
| [getpreferredclosedcaptionservice](#method.getpreferredclosedcaptionservice) | Getting Preferred ClosedCaption Service |


<a name="method.setaudiodescription"></a>
## *setaudiodescription [<sup>method</sup>](#head.Methods)*

Setting Audio Description.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | boolean | Audio Description Enabled: true/false |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | Null string will display |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.setaudiodescription",
    "params": true
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "null"
}
```

<a name="method.setpreferredaudiolanguages"></a>
## *setpreferredaudiolanguages [<sup>method</sup>](#head.Methods)*

Setting Preferred Audio Languages.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | string | Preferred Audio Languages: eng, wel |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | Null string will display |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.setpreferredaudiolanguages",
    "params": "eng"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "null"
}
```

<a name="method.setpresentationlanguages"></a>
## *setpresentationlanguages [<sup>method</sup>](#head.Methods)*

Setting Presentation Languages.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | string | Presentation Languages: en-US, es-US |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | Null string will display |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.setpresentationlanguages",
    "params": "en-US"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "null"
}
```

<a name="method.setcaptions"></a>
## *setcaptions [<sup>method</sup>](#head.Methods)*

Setting Captions.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | boolean | Captions Enabled: true/false |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | Null string will display  |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.setcaptions",
    "params": true
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "null"
}
```

<a name="method.setpreferredcaptionlanguages"></a>
## *setpreferredcaptionlanguages [<sup>method</sup>](#head.Methods)*

Setting PreferredCaption Languages.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | string | PreferredCaption Languages: eng, fra |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | Null string will display |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.setpreferredcaptionlanguages",
    "params": "eng"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "null"
}
```

<a name="method.setpreferredclosedcaptionservice"></a>
## *setpreferredclosedcaptionservice [<sup>method</sup>](#head.Methods)*

Setting Preferred Closed Caption Service.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | string | Preferred Closed Caption Service: CC3 |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | Null string will display |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.setpreferredclosedcaptionservice",
    "params": "CC3"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "null"
}
```

<a name="method.getaudiodescription"></a>
## *getaudiodescription [<sup>method</sup>](#head.Methods)*

Returns Audio Description.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.enabled | boolean | Enabled (`true`) or disabled (`false`) |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.getaudiodescription"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "enabled": false
    }
}
```

<a name="method.getpreferredaudiolanguages"></a>
## *getpreferredaudiolanguages [<sup>method</sup>](#head.Methods)*

Returns Audio Description.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.preferredLanguages | string | A prioritized list of ISO 639-2/B codes for the preferred audio languages |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.getpreferredaudiolanguages"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "preferredLanguages": "eng"
    }
}
```

<a name="method.getpresentationlanguages"></a>
## *getpresentationlanguages [<sup>method</sup>](#head.Methods)*

Getting Presentation Languages.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.presentationlanguages | string | The preferred presentationLanguages in a full BCP 47 value, including script, * region, variant The language set and used by Immerse UI |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.getpresentationlanguages"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "presentationlanguages": "en-US"
    }
}
```

<a name="method.getcaptions"></a>
## *getcaptions [<sup>method</sup>](#head.Methods)*

Getting Captions Enabled.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.enabled | boolean | Enabled (`true`) or disabled (`false`) |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.getcaptions"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "enabled": false
    }
}
```

<a name="method.getpreferredcaptionslanguages"></a>
## *getpreferredcaptionslanguages [<sup>method</sup>](#head.Methods)*

Getting Preferred Caption Languages.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.preferredLanguages | string | A prioritized list of ISO 639-2/B codes for the preferred captions languages |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.getpreferredcaptionslanguages"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "preferredLanguages": "eng"
    }
}
```

<a name="method.getpreferredclosedcaptionservice"></a>
## *getpreferredclosedcaptionservice [<sup>method</sup>](#head.Methods)*

Getting Preferred ClosedCaption Service.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.service | string | <sup>*(optional)*</sup> A string for the preferred closed captions service.  Valid values are AUTO, CC[1-4], TEXT[1-4], SERVICE[1-64] where CC and TEXT is CTA-608 and SERVICE is CTA-708.  AUTO indicates that the choice is left to the player |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.getpreferredclosedcaptionservice"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "service": "CC3"
    }
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.UserSettings plugin:

UserSettings interface events:

| Event | Description |
| :-------- | :-------- |
| [onaudiodescriptionchanged](#event.onaudiodescriptionchanged) | Triggered after the audio description changes (see `setaudiodescription`) |
| [onpreferredaudiolanguageschanged](#event.onpreferredaudiolanguageschanged) | Triggered after the audio preferred Audio languages changes (see `setpreferredaudiolanguages`) |
| [onpresentationlanguagechanged](#event.onpresentationlanguagechanged) | Triggered after the Presentation Language changes (see `setpresentationlanguages`) |
| [oncaptionschanged](#event.oncaptionschanged) | Triggered after the captions changes (see `setcaptionsenabled`) |
| [onpreferredcaptionslanguageschanged](#event.onpreferredcaptionslanguageschanged) | Triggered after the PreferredCaption Languages changes (see `setpreferredcaptionlanguages`) |
| [onpreferredclosedcaptionservicechanged](#event.onpreferredclosedcaptionservicechanged) | Triggered after the Preferred Closed Caption changes (see `setpreferredclosedcaptionservice`) |


<a name="event.onaudiodescriptionchanged"></a>
## *onaudiodescriptionchanged [<sup>event</sup>](#head.Notifications)*

Triggered after the audio description changes (see `setaudiodescription`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | Receive audio description changes enable or not |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onaudiodescriptionchanged",
    "params": {
        "enabled": true
    }
}
```

<a name="event.onpreferredaudiolanguageschanged"></a>
## *onpreferredaudiolanguageschanged [<sup>event</sup>](#head.Notifications)*

Triggered after the audio preferred Audio languages changes (see `setpreferredaudiolanguages`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.PreferredAudioLanguagesChanged | string | Receive preferred Audio languages changes |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onpreferredaudiolanguageschanged",
    "params": {
        "PreferredAudioLanguagesChanged": "eng"
    }
}
```

<a name="event.onpresentationlanguagechanged"></a>
## *onpresentationlanguagechanged [<sup>event</sup>](#head.Notifications)*

Triggered after the Presentation Language changes (see `setpresentationlanguages`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.PresentationLanguageChanged | string | Receive Presentation Language changes |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onpresentationlanguagechanged",
    "params": {
        "PresentationLanguageChanged": "en-US"
    }
}
```

<a name="event.oncaptionschanged"></a>
## *oncaptionschanged [<sup>event</sup>](#head.Notifications)*

Triggered after the captions changes (see `setcaptionsenabled`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean |  |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.oncaptionschanged",
    "params": {
        "enabled": true
    }
}
```

<a name="event.onpreferredcaptionslanguageschanged"></a>
## *onpreferredcaptionslanguageschanged [<sup>event</sup>](#head.Notifications)*

Triggered after the PreferredCaption Languages changes (see `setpreferredcaptionlanguages`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.PreferredCaptionsLanguagesChanged | string | Receive PreferredCaption Languages changes |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onpreferredcaptionslanguageschanged",
    "params": {
        "PreferredCaptionsLanguagesChanged": "eng"
    }
}
```

<a name="event.onpreferredclosedcaptionservicechanged"></a>
## *onpreferredclosedcaptionservicechanged [<sup>event</sup>](#head.Notifications)*

Triggered after the Preferred Closed Caption changes (see `setpreferredclosedcaptionservice`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.PreferredClosedCaptionServiceChanged | string | Receive Preferred Closed Caption changes |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onpreferredclosedcaptionservicechanged",
    "params": {
        "PreferredClosedCaptionServiceChanged": "eng"
    }
}
```

