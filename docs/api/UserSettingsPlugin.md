<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.UserSettings_API"></a>
# UserSettings API

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

The `UserSettings`, that is responsible for persisting and notifying listeners of any change of these settings..

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| classname | string | Class name: *org.rdk.UserSettings* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.UserSettings plugin:

org.rdk.UserSettings interface methods:

| Method | Description |
| :-------- | :-------- |
| [SetAudioDescription](#method.SetAudioDescription) | Setting Audio Description |
| [SetPreferredAudioLanguages](#method.SetPreferredAudioLanguages) | Setting Preferred Audio Languages |
| [SetPresentationLanguage](#method.SetPresentationLanguage) | Setting Presentation Languages |
| [SetCaptions](#method.SetCaptions) | Setting Captions |
| [SetPreferredCaptionsLanguages](#method.SetPreferredCaptionsLanguages) | Setting PreferredCaption Languages |
| [SetPreferredClosedCaptionService](#method.SetPreferredClosedCaptionService) | Setting Preferred Closed Caption Service |
| [SetPrivacyMode](#method.SetPrivacyMode) | Setting Privacy Mode |
| [GetAudioDescription](#method.GetAudioDescription) | Returns Audio Description |
| [GetPreferredAudioLanguages](#method.GetPreferredAudioLanguages) | Returns Audio Description |
| [GetPresentationLanguage](#method.GetPresentationLanguage) | Getting Presentation Languages |
| [GetCaptions](#method.GetCaptions) | Getting Captions Enabled |
| [GetPreferredCaptionsLanguages](#method.GetPreferredCaptionsLanguages) | Getting Preferred Caption Languages |
| [GetPreferredClosedCaptionService](#method.GetPreferredClosedCaptionService) | Getting Preferred ClosedCaption Service |
| [GetPrivacyMode](#method.GetPrivacyMode) | Getting Privacy Mode |


<a name="method.SetAudioDescription"></a>
## *SetAudioDescription [<sup>method</sup>](#head.Methods)*

Setting Audio Description.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | Audio Description Enabled: true/false |

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
    "method": "org.rdk.UserSettings.SetAudioDescription",
    "params": {
        "enabled": true
    }
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

<a name="method.SetPreferredAudioLanguages"></a>
## *SetPreferredAudioLanguages [<sup>method</sup>](#head.Methods)*

Setting Preferred Audio Languages.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.preferredLanguages | string | A prioritized list of ISO 639-2/B codes for the preferred audio languages |

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
    "method": "org.rdk.UserSettings.SetPreferredAudioLanguages",
    "params": {
        "preferredLanguages": "eng"
    }
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

<a name="method.SetPresentationLanguage"></a>
## *SetPresentationLanguage [<sup>method</sup>](#head.Methods)*

Setting Presentation Languages.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.presentationLanguages | string | The preferred presentationLanguages in a full BCP 47 value, including script, * region, variant The language set and used by Immerse UI |

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
    "method": "org.rdk.UserSettings.SetPresentationLanguage",
    "params": {
        "presentationLanguages": "en-US"
    }
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

<a name="method.SetCaptions"></a>
## *SetCaptions [<sup>method</sup>](#head.Methods)*

Setting Captions.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | Captions Enabled: true/false |

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
    "method": "org.rdk.UserSettings.SetCaptions",
    "params": {
        "enabled": true
    }
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

<a name="method.SetPreferredCaptionsLanguages"></a>
## *SetPreferredCaptionsLanguages [<sup>method</sup>](#head.Methods)*

Setting PreferredCaption Languages.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.preferredLanguages | string | A prioritized list of ISO 639-2/B codes for the preferred captions languages |

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
    "method": "org.rdk.UserSettings.SetPreferredCaptionsLanguages",
    "params": {
        "preferredLanguages": "eng"
    }
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

<a name="method.SetPreferredClosedCaptionService"></a>
## *SetPreferredClosedCaptionService [<sup>method</sup>](#head.Methods)*

Setting Preferred Closed Caption Service.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.service | string | A string for the preferred closed captions service.  Valid values are AUTO, CC[1-4], TEXT[1-4], SERVICE[1-64] where CC and TEXT is CTA-608 and SERVICE is CTA-708.  AUTO indicates that the choice is left to the player |

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
    "method": "org.rdk.UserSettings.SetPreferredClosedCaptionService",
    "params": {
        "service": "CC3"
    }
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

<a name="method.SetPrivacyMode"></a>
## *SetPrivacyMode [<sup>method</sup>](#head.Methods)*

Setting Privacy Mode.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.privacyMode | string | New Privacy Mode |

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
    "method": "org.rdk.UserSettings.SetPrivacyMode",
    "params": {
        "privacyMode": "DO_NOT_SHARE"
    }
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

<a name="method.GetAudioDescription"></a>
## *GetAudioDescription [<sup>method</sup>](#head.Methods)*

Returns Audio Description.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | boolean |  |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.GetAudioDescription"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": true
}
```

<a name="method.GetPreferredAudioLanguages"></a>
## *GetPreferredAudioLanguages [<sup>method</sup>](#head.Methods)*

Returns Audio Description.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string |  |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.GetPreferredAudioLanguages"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "eng"
}
```

<a name="method.GetPresentationLanguage"></a>
## *GetPresentationLanguage [<sup>method</sup>](#head.Methods)*

Getting Presentation Languages.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string |  |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.GetPresentationLanguage"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "en-US"
}
```

<a name="method.GetCaptions"></a>
## *GetCaptions [<sup>method</sup>](#head.Methods)*

Getting Captions Enabled.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | boolean |  |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.GetCaptions"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": true
}
```

<a name="method.GetPreferredCaptionsLanguages"></a>
## *GetPreferredCaptionsLanguages [<sup>method</sup>](#head.Methods)*

Getting Preferred Caption Languages.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string |  |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.GetPreferredCaptionsLanguages"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "eng"
}
```

<a name="method.GetPreferredClosedCaptionService"></a>
## *GetPreferredClosedCaptionService [<sup>method</sup>](#head.Methods)*

Getting Preferred ClosedCaption Service.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string |  |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.GetPreferredClosedCaptionService"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "CC3"
}
```

<a name="method.GetPrivacyMode"></a>
## *GetPrivacyMode [<sup>method</sup>](#head.Methods)*

Getting Privacy Mode.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string |  |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.GetPrivacyMode"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "DO_NOT_SHARE"
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.UserSettings plugin:

org.rdk.UserSettings interface events:

| Event | Description |
| :-------- | :-------- |
| [OnAudioDescriptionChanged](#event.OnAudioDescriptionChanged) | Triggered after the audio description changes (see `SetAudioDescription`) |
| [OnPreferredAudioLanguagesChanged](#event.OnPreferredAudioLanguagesChanged) | Triggered after the audio preferred Audio languages changes (see `SetPreferredAudioLanguages`) |
| [OnPresentationLanguageChanged](#event.OnPresentationLanguageChanged) | Triggered after the Presentation Language changes (see `SetPresentationLanguage`) |
| [OnCaptionsChanged](#event.OnCaptionsChanged) | Triggered after the captions changes (see `SetCaptions`) |
| [OnPreferredCaptionsLanguagesChanged](#event.OnPreferredCaptionsLanguagesChanged) | Triggered after the PreferredCaption Languages changes (see `SetPreferredCaptionsLanguages`) |
| [OnPreferredClosedCaptionServiceChanged](#event.OnPreferredClosedCaptionServiceChanged) | Triggered after the Preferred Closed Caption changes (see `SetPreferredClosedCaptionService`) |
| [OnPrivacyModeChanged](#event.OnPrivacyModeChanged) | Triggered after the Privacy Mode changes (see `SetPrivacyMode`) |


<a name="event.OnAudioDescriptionChanged"></a>
## *OnAudioDescriptionChanged [<sup>event</sup>](#head.Notifications)*

Triggered after the audio description changes (see `SetAudioDescription`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | Receive audio description changes enable or not |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.OnAudioDescriptionChanged",
    "params": {
        "enabled": true
    }
}
```

<a name="event.OnPreferredAudioLanguagesChanged"></a>
## *OnPreferredAudioLanguagesChanged [<sup>event</sup>](#head.Notifications)*

Triggered after the audio preferred Audio languages changes (see `SetPreferredAudioLanguages`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.preferredLanguages | string | Receive preferred Audio languages changes |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.OnPreferredAudioLanguagesChanged",
    "params": {
        "preferredLanguages": "eng"
    }
}
```

<a name="event.OnPresentationLanguageChanged"></a>
## *OnPresentationLanguageChanged [<sup>event</sup>](#head.Notifications)*

Triggered after the Presentation Language changes (see `SetPresentationLanguage`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.presentationLanguages | string | Receive Presentation Language changes |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.OnPresentationLanguageChanged",
    "params": {
        "presentationLanguages": "en-US"
    }
}
```

<a name="event.OnCaptionsChanged"></a>
## *OnCaptionsChanged [<sup>event</sup>](#head.Notifications)*

Triggered after the captions changes (see `SetCaptions`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean |  |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.OnCaptionsChanged",
    "params": {
        "enabled": true
    }
}
```

<a name="event.OnPreferredCaptionsLanguagesChanged"></a>
## *OnPreferredCaptionsLanguagesChanged [<sup>event</sup>](#head.Notifications)*

Triggered after the PreferredCaption Languages changes (see `SetPreferredCaptionsLanguages`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.preferredLanguages | string | Receive PreferredCaption Languages changes |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.OnPreferredCaptionsLanguagesChanged",
    "params": {
        "preferredLanguages": "eng"
    }
}
```

<a name="event.OnPreferredClosedCaptionServiceChanged"></a>
## *OnPreferredClosedCaptionServiceChanged [<sup>event</sup>](#head.Notifications)*

Triggered after the Preferred Closed Caption changes (see `SetPreferredClosedCaptionService`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.service | string | Receive Preferred Closed Caption changes |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.OnPreferredClosedCaptionServiceChanged",
    "params": {
        "service": "CC3"
    }
}
```

<a name="event.OnPrivacyModeChanged"></a>
## *OnPrivacyModeChanged [<sup>event</sup>](#head.Notifications)*

Triggered after the Privacy Mode changes (see `SetPrivacyMode`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.privacyMode | string | Receive Privacy Mode changes |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.OnPrivacyModeChanged",
    "params": {
        "privacyMode": "DO_NOT_SHARE"
    }
}
```

