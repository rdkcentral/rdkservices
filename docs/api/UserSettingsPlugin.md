<!-- Generated automatically, DO NOT EDIT! -->
<a name="UserSettings_Plugin"></a>
# UserSettings Plugin

**Version: [1.0.2](https://github.com/rdkcentral/rdkservices/blob/main/UserSettings/CHANGELOG.md)**

A org.rdk.UserSettings plugin for Thunder framework.

### Table of Contents

- [Abbreviation, Acronyms and Terms](#Abbreviation,_Acronyms_and_Terms)
- [Description](#Description)
- [Configuration](#Configuration)
- [Methods](#Methods)
- [Notifications](#Notifications)

<a name="Abbreviation,_Acronyms_and_Terms"></a>
# Abbreviation, Acronyms and Terms

[[Refer to this link](userguide/aat.md)]

<a name="Description"></a>
# Description

The `UserSettings` that is responsible for persisting and notifying listeners of any change of these settings.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.UserSettings*) |
| classname | string | Class name: *org.rdk.UserSettings* |
| locator | string | Library name: *libWPEFrameworkUserSettings.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.UserSettings plugin:

UserSettings interface methods:

| Method | Description |
| :-------- | :-------- |
| [SetAudioDescription](#SetAudioDescription) | Setting Audio Description |
| [SetPreferredAudioLanguages](#SetPreferredAudioLanguages) | Setting Preferred Audio Languages |
| [SetPresentationLanguage](#SetPresentationLanguage) | Setting Presentation Languages |
| [SetCaptions](#SetCaptions) | Setting Captions |
| [SetPreferredCaptionsLanguages](#SetPreferredCaptionsLanguages) | Setting PreferredCaption Languages |
| [SetPreferredClosedCaptionService](#SetPreferredClosedCaptionService) | Setting Preferred Closed Caption Service |
| [SetPrivacyMode](#SetPrivacyMode) | Setting Privacy Mode |
| [GetAudioDescription](#GetAudioDescription) | Returns Audio Description |
| [GetPreferredAudioLanguages](#GetPreferredAudioLanguages) | Returns Audio Description |
| [GetPresentationLanguage](#GetPresentationLanguage) | Getting Presentation Languages |
| [GetCaptions](#GetCaptions) | Getting Captions Enabled |
| [GetPreferredCaptionsLanguages](#GetPreferredCaptionsLanguages) | Getting Preferred Caption Languages |
| [GetPreferredClosedCaptionService](#GetPreferredClosedCaptionService) | Getting Preferred ClosedCaption Service |
| [GetPrivacyMode](#GetPrivacyMode) | Getting Privacy Mode |


<a name="SetAudioDescription"></a>
## *SetAudioDescription*

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
    "method": "org.rdk.UserSettings.SetAudioDescription",
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

<a name="SetPreferredAudioLanguages"></a>
## *SetPreferredAudioLanguages*

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
    "method": "org.rdk.UserSettings.SetPreferredAudioLanguages",
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

<a name="SetPresentationLanguage"></a>
## *SetPresentationLanguage*

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
    "method": "org.rdk.UserSettings.SetPresentationLanguage",
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

<a name="SetCaptions"></a>
## *SetCaptions*

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
    "method": "org.rdk.UserSettings.SetCaptions",
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

<a name="SetPreferredCaptionsLanguages"></a>
## *SetPreferredCaptionsLanguages*

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
    "method": "org.rdk.UserSettings.SetPreferredCaptionsLanguages",
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

<a name="SetPreferredClosedCaptionService"></a>
## *SetPreferredClosedCaptionService*

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
    "method": "org.rdk.UserSettings.SetPreferredClosedCaptionService",
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

<a name="SetPrivacyMode"></a>
## *SetPrivacyMode*

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

<a name="GetAudioDescription"></a>
## *GetAudioDescription*

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
    "result": false
}
```

<a name="GetPreferredAudioLanguages"></a>
## *GetPreferredAudioLanguages*

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
    "result": "..."
}
```

<a name="GetPresentationLanguage"></a>
## *GetPresentationLanguage*

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
    "result": "..."
}
```

<a name="GetCaptions"></a>
## *GetCaptions*

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
    "result": false
}
```

<a name="GetPreferredCaptionsLanguages"></a>
## *GetPreferredCaptionsLanguages*

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
    "result": "..."
}
```

<a name="GetPreferredClosedCaptionService"></a>
## *GetPreferredClosedCaptionService*

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
    "result": "..."
}
```

<a name="GetPrivacyMode"></a>
## *GetPrivacyMode*

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
    "result": "..."
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.UserSettings plugin:

UserSettings interface events:

| Event | Description |
| :-------- | :-------- |
| [OnAudioDescriptionChanged](#OnAudioDescriptionChanged) | Triggered after the audio description changes (see `setaudiodescription`) |
| [OnPreferredAudioLanguagesChanged](#OnPreferredAudioLanguagesChanged) | Triggered after the audio preferred Audio languages changes (see `setpreferredaudiolanguages`) |
| [OnPresentationLanguageChanged](#OnPresentationLanguageChanged) | Triggered after the Presentation Language changes (see `setpresentationlanguages`) |
| [OnCaptionsChanged](#OnCaptionsChanged) | Triggered after the captions changes (see `setcaptionsenabled`) |
| [OnPreferredCaptionsLanguagesChanged](#OnPreferredCaptionsLanguagesChanged) | Triggered after the PreferredCaption Languages changes (see `setpreferredcaptionlanguages`) |
| [OnPreferredClosedCaptionServiceChanged](#OnPreferredClosedCaptionServiceChanged) | Triggered after the Preferred Closed Caption changes (see `setpreferredclosedcaptionservice`) |
| [OnPrivacyModeChanged](#OnPrivacyModeChanged) | Triggered after the Privacy Mode changes (see `SetPrivacyMode`) |


<a name="OnAudioDescriptionChanged"></a>
## *OnAudioDescriptionChanged*

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
    "method": "client.events.OnAudioDescriptionChanged",
    "params": {
        "enabled": true
    }
}
```

<a name="OnPreferredAudioLanguagesChanged"></a>
## *OnPreferredAudioLanguagesChanged*

Triggered after the audio preferred Audio languages changes (see `setpreferredaudiolanguages`).

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

<a name="OnPresentationLanguageChanged"></a>
## *OnPresentationLanguageChanged*

Triggered after the Presentation Language changes (see `setpresentationlanguages`).

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

<a name="OnCaptionsChanged"></a>
## *OnCaptionsChanged*

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
    "method": "client.events.OnCaptionsChanged",
    "params": {
        "enabled": true
    }
}
```

<a name="OnPreferredCaptionsLanguagesChanged"></a>
## *OnPreferredCaptionsLanguagesChanged*

Triggered after the PreferredCaption Languages changes (see `setpreferredcaptionlanguages`).

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

<a name="OnPreferredClosedCaptionServiceChanged"></a>
## *OnPreferredClosedCaptionServiceChanged*

Triggered after the Preferred Closed Caption changes (see `setpreferredclosedcaptionservice`).

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

<a name="OnPrivacyModeChanged"></a>
## *OnPrivacyModeChanged*

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

