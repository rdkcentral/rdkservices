<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.UserSettings_API"></a>
# UserSettings API

**Version: [1.0.0]()**

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
| [setAudioDescription](#method.setAudioDescription) | Setting Audio Description |
| [setPreferredAudioLanguages](#method.setPreferredAudioLanguages) | Setting Preferred Audio Languages |
| [setPresentationLanguage](#method.setPresentationLanguage) | Setting Presentation Languages |
| [setCaptions](#method.setCaptions) | Setting Captions |
| [setPreferredCaptionsLanguages](#method.setPreferredCaptionsLanguages) | Setting PreferredCaption Languages |
| [setPreferredClosedCaptionService](#method.setPreferredClosedCaptionService) | Setting Preferred Closed Caption Service |
| [setPinControl](#method.setPinControl) | Setting PinControl |
| [setViewingRestrictions](#method.setViewingRestrictions) | Setting ViewingRestrictions |
| [setViewingRestrictionsWindow](#method.setViewingRestrictionsWindow) | Setting viewingRestrictionsWindow |
| [setLiveWatershed](#method.setLiveWatershed) | Setting LiveWatershed |
| [setPlaybackWatershed](#method.setPlaybackWatershed) | Setting PlaybackWatershed |
| [setBlockNotRatedContent](#method.setBlockNotRatedContent) | Setting BlockNotRatedContent |
| [setPinOnPurchase](#method.setPinOnPurchase) | Setting setPinOnPurchase |
| [getAudioDescription](#method.getAudioDescription) | Returns Audio Description |
| [getPreferredAudioLanguages](#method.getPreferredAudioLanguages) | Returns Preferred Audio Languages |
| [getPresentationLanguage](#method.getPresentationLanguage) | Getting Presentation Languages |
| [getCaptions](#method.getCaptions) | Getting Captions Enabled |
| [getPreferredCaptionsLanguages](#method.getPreferredCaptionsLanguages) | Getting Preferred Caption Languages |
| [getPreferredClosedCaptionService](#method.getPreferredClosedCaptionService) | Getting Preferred ClosedCaption Service |
| [getPinControl](#method.getPinControl) | Returns Pin Control |
| [getViewingRestrictions](#method.getViewingRestrictions) | Returns Get Viewing Restrictions |
| [getViewingRestrictionsWindow](#method.getViewingRestrictionsWindow) | Returns Get Viewing Restrictions Window |
| [getLiveWatershed](#method.getLiveWatershed) | Returns Live Watershed |
| [getPlaybackWatershed](#method.getPlaybackWatershed) | Returns Playback Watershed |
| [getBlockNotRatedContent](#method.getBlockNotRatedContent) | Returns BlockNotRatedContent |
| [getPinOnPurchase](#method.getPinOnPurchase) | Returns PinOnPurchase |


<a name="method.setAudioDescription"></a>
## *setAudioDescription [<sup>method</sup>](#head.Methods)*

Setting Audio Description.

### Events

| Event | Description |
| :-------- | :-------- |
| [OnAudioDescriptionChanged](#event.OnAudioDescriptionChanged) | Triggered when the audio description changes. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | Audio Description Enabled: true/false |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | On success null will be returned |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.setAudioDescription",
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

<a name="method.setPreferredAudioLanguages"></a>
## *setPreferredAudioLanguages [<sup>method</sup>](#head.Methods)*

Setting Preferred Audio Languages.

### Events

| Event | Description |
| :-------- | :-------- |
| [onPreferredAudioLanguagesChanged](#event.onPreferredAudioLanguagesChanged) | Triggered when the audio preferred Audio languages changes. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.preferredLanguages | string | A prioritized list of ISO 639-2/B codes for the preferred audio languages |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | On success null will be returned |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.setPreferredAudioLanguages",
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

<a name="method.setPresentationLanguage"></a>
## *setPresentationLanguage [<sup>method</sup>](#head.Methods)*

Setting Presentation Languages.

### Events

| Event | Description |
| :-------- | :-------- |
| [onPresentationLanguageChanged](#event.onPresentationLanguageChanged) | Triggered when the presentation Language changes. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.presentationLanguage | string | The preferred presentationLanguage in a full BCP 47 value, including script, * region, variant The language set and used by Immerse UI |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | On success null will be returned |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.setPresentationLanguage",
    "params": {
        "presentationLanguage": "en-US"
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

<a name="method.setCaptions"></a>
## *setCaptions [<sup>method</sup>](#head.Methods)*

Setting Captions.

### Events

| Event | Description |
| :-------- | :-------- |
| [onCaptionsChanged](#event.onCaptionsChanged) | Triggered when the captions changes. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | Captions Enabled: true/false |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | On success null will be returned |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.setCaptions",
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

<a name="method.setPreferredCaptionsLanguages"></a>
## *setPreferredCaptionsLanguages [<sup>method</sup>](#head.Methods)*

Setting PreferredCaption Languages.

### Events

| Event | Description |
| :-------- | :-------- |
| [onPreferredCaptionsLanguagesChanged](#event.onPreferredCaptionsLanguagesChanged) | Triggered when the PreferredCaption Languages changes. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.preferredLanguages | string | A prioritized list of ISO 639-2/B codes for the preferred captions languages |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | On success null will be returned |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.setPreferredCaptionsLanguages",
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

<a name="method.setPreferredClosedCaptionService"></a>
## *setPreferredClosedCaptionService [<sup>method</sup>](#head.Methods)*

Setting Preferred Closed Caption Service.

### Events

| Event | Description |
| :-------- | :-------- |
| [onPreferredClosedCaptionServiceChanged](#event.onPreferredClosedCaptionServiceChanged) | Triggered when the Preferred Closed Caption changes. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.service | string | A string for the preferred closed captions service.  Valid values are AUTO, CC[1-4], TEXT[1-4], SERVICE[1-64] where CC and TEXT is CTA-608 and SERVICE is CTA-708.  AUTO indicates that the choice is left to the player |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | On success null will be returned |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.setPreferredClosedCaptionService",
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

<a name="method.setPinControl"></a>
## *setPinControl [<sup>method</sup>](#head.Methods)*

Setting PinControl.

### Events

| Event | Description |
| :-------- | :-------- |
| [onPinControlChanged](#event.onPinControlChanged) | Triggered when the pincontrol changes. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | PinControl Enabled: true/false |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | On success null will be returned |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.setPinControl",
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

<a name="method.setViewingRestrictions"></a>
## *setViewingRestrictions [<sup>method</sup>](#head.Methods)*

Setting ViewingRestrictions.

### Events

| Event | Description |
| :-------- | :-------- |
| [OnViewingRestrictionsChanged](#event.OnViewingRestrictionsChanged) | Triggered when the viewingRestrictions changes. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.viewingRestrictions | string | A project-specific representation of the time interval when viewing |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | On success null will be returned |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.setViewingRestrictions",
    "params": {
        "viewingRestrictions": "ALWAYS"
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

<a name="method.setViewingRestrictionsWindow"></a>
## *setViewingRestrictionsWindow [<sup>method</sup>](#head.Methods)*

Setting viewingRestrictionsWindow.

### Events

| Event | Description |
| :-------- | :-------- |
| [OnViewingRestrictionsWindowChanged](#event.OnViewingRestrictionsWindowChanged) | Triggered when the viewingRestrictionsWindow changes. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.viewingRestrictionsWindow | string | A project-specific representation of the time interval |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | On success null will be returned |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.setViewingRestrictionsWindow",
    "params": {
        "viewingRestrictionsWindow": "ALWAYS"
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

<a name="method.setLiveWatershed"></a>
## *setLiveWatershed [<sup>method</sup>](#head.Methods)*

Setting LiveWatershed.

### Events

| Event | Description |
| :-------- | :-------- |
| [OnLiveWatershedChanged](#event.OnLiveWatershedChanged) | Triggered when the liveWatershed changes. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | LiveWatershed Enabled: true/false |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | On success null will be returned |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.setLiveWatershed",
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

<a name="method.setPlaybackWatershed"></a>
## *setPlaybackWatershed [<sup>method</sup>](#head.Methods)*

Setting PlaybackWatershed.

### Events

| Event | Description |
| :-------- | :-------- |
| [OnPlaybackWatershedChanged](#event.OnPlaybackWatershedChanged) | Triggered when the playbackWatershed changes. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | PlaybackWatershed Enabled: true/false |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | On success null will be returned |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.setPlaybackWatershed",
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

<a name="method.setBlockNotRatedContent"></a>
## *setBlockNotRatedContent [<sup>method</sup>](#head.Methods)*

Setting BlockNotRatedContent.

### Events

| Event | Description |
| :-------- | :-------- |
| [OnBlockNotRatedContentChanged](#event.OnBlockNotRatedContentChanged) | Triggered when the blockNotRatedContent changes. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | BlockNotRatedContent Enabled: true/false |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | On success null will be returned |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.setBlockNotRatedContent",
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

<a name="method.setPinOnPurchase"></a>
## *setPinOnPurchase [<sup>method</sup>](#head.Methods)*

Setting setPinOnPurchase.

### Events

| Event | Description |
| :-------- | :-------- |
| [OnPinOnPurchaseChanged](#event.OnPinOnPurchaseChanged) | Triggered when the pin on the purchase changes. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | setPinOnPurchase Enabled: true/false |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | On success null will be returned |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.setPinOnPurchase",
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

<a name="method.getAudioDescription"></a>
## *getAudioDescription [<sup>method</sup>](#head.Methods)*

Returns Audio Description.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | boolean | Audio Description Enabled: true/false |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.getAudioDescription"
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

<a name="method.getPreferredAudioLanguages"></a>
## *getPreferredAudioLanguages [<sup>method</sup>](#head.Methods)*

Returns Preferred Audio Languages.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | A prioritized list of ISO 639-2/B codes for the preferred audio languages |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.getPreferredAudioLanguages"
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

<a name="method.getPresentationLanguage"></a>
## *getPresentationLanguage [<sup>method</sup>](#head.Methods)*

Getting Presentation Languages.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | The preferred presentationLanguage in a full BCP 47 value, including script, * region, variant The language set and used by Immerse UI |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.getPresentationLanguage"
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

<a name="method.getCaptions"></a>
## *getCaptions [<sup>method</sup>](#head.Methods)*

Getting Captions Enabled.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | boolean | Captions Enabled: true/false |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.getCaptions"
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

<a name="method.getPreferredCaptionsLanguages"></a>
## *getPreferredCaptionsLanguages [<sup>method</sup>](#head.Methods)*

Getting Preferred Caption Languages.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | A prioritized list of ISO 639-2/B codes for the preferred captions languages |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.getPreferredCaptionsLanguages"
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

<a name="method.getPreferredClosedCaptionService"></a>
## *getPreferredClosedCaptionService [<sup>method</sup>](#head.Methods)*

Getting Preferred ClosedCaption Service.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | A string for the preferred closed captions service.  Valid values are AUTO, CC[1-4], TEXT[1-4], SERVICE[1-64] where CC and TEXT is CTA-608 and SERVICE is CTA-708.  AUTO indicates that the choice is left to the player |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.getPreferredClosedCaptionService"
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

<a name="method.getPinControl"></a>
## *getPinControl [<sup>method</sup>](#head.Methods)*

Returns Pin Control.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | boolean | Pin Control Enabled: true/false |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.getPinControl"
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

<a name="method.getViewingRestrictions"></a>
## *getViewingRestrictions [<sup>method</sup>](#head.Methods)*

Returns Get Viewing Restrictions.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | A project-specific representation of the time interval when viewing |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.getViewingRestrictions"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "ALWAYS"
}
```

<a name="method.getViewingRestrictionsWindow"></a>
## *getViewingRestrictionsWindow [<sup>method</sup>](#head.Methods)*

Returns Get Viewing Restrictions Window.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | A project-specific representation of the time interval |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.getViewingRestrictionsWindow"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "ALWAYS"
}
```

<a name="method.getLiveWatershed"></a>
## *getLiveWatershed [<sup>method</sup>](#head.Methods)*

Returns Live Watershed.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | boolean | Live Watershed Enabled: true/false |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.getLiveWatershed"
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

<a name="method.getPlaybackWatershed"></a>
## *getPlaybackWatershed [<sup>method</sup>](#head.Methods)*

Returns Playback Watershed.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | boolean | Playback Watershed Enabled: true/false |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.getPlaybackWatershed"
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

<a name="method.getBlockNotRatedContent"></a>
## *getBlockNotRatedContent [<sup>method</sup>](#head.Methods)*

Returns BlockNotRatedContent.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | boolean | BlockNotRatedContent Enabled: true/false |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.getBlockNotRatedContent"
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

<a name="method.getPinOnPurchase"></a>
## *getPinOnPurchase [<sup>method</sup>](#head.Methods)*

Returns PinOnPurchase.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | boolean | PinOnPurchase Enabled: true/false |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.UserSettings.getPinOnPurchase"
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

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.UserSettings plugin:

org.rdk.UserSettings interface events:

| Event | Description |
| :-------- | :-------- |
| [onAudioDescriptionChanged](#event.onAudioDescriptionChanged) | Triggered after the audio description changes (see `SetAudioDescription`) |
| [onPreferredAudioLanguagesChanged](#event.onPreferredAudioLanguagesChanged) | Triggered after the audio preferred Audio languages changes (see `SetPreferredAudioLanguages`) |
| [onPresentationLanguageChanged](#event.onPresentationLanguageChanged) | Triggered after the Presentation Language changes (see `SetPresentationLanguage`) |
| [onCaptionsChanged](#event.onCaptionsChanged) | Triggered after the captions changes (see `SetCaptions`) |
| [onPreferredCaptionsLanguagesChanged](#event.onPreferredCaptionsLanguagesChanged) | Triggered after the PreferredCaption Languages changes (see `SetPreferredCaptionsLanguages`) |
| [onPreferredClosedCaptionServiceChanged](#event.onPreferredClosedCaptionServiceChanged) | Triggered after the Preferred Closed Caption changes (see `SetPreferredClosedCaptionService`) |
| [onPinControlChanged](#event.onPinControlChanged) | Triggered after the pin control changes (see `setPinControl`) |
| [onViewingRestrictionsChanged](#event.onViewingRestrictionsChanged) | Triggered after the viewingRestrictions changes (see `setViewingRestrictions`) |
| [onViewingRestrictionsWindowChanged](#event.onViewingRestrictionsWindowChanged) | Triggered after the viewingRestrictionsWindow changes (see `setViewingRestrictionsWindow`) |
| [onLiveWatershedChanged](#event.onLiveWatershedChanged) | Triggered after the liveWatershed changes (see `setLiveWatershed`) |
| [onPlaybackWatershedChanged](#event.onPlaybackWatershedChanged) | Triggered after the playbackWatershed changes (see `setPlaybackWatershed`) |
| [onBlockNotRatedContentChanged](#event.onBlockNotRatedContentChanged) | Triggered after the blockNotRatedContent changes (see `setBlockNotRatedContent`) |
| [onPinOnPurchaseChanged](#event.onPinOnPurchaseChanged) | Triggered after the pinOnPurchase changes (see `setPinOnPurchase`) |


<a name="event.onAudioDescriptionChanged"></a>
## *onAudioDescriptionChanged [<sup>event</sup>](#head.Notifications)*

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
    "method": "client.events.onAudioDescriptionChanged",
    "params": {
        "enabled": true
    }
}
```

<a name="event.onPreferredAudioLanguagesChanged"></a>
## *onPreferredAudioLanguagesChanged [<sup>event</sup>](#head.Notifications)*

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
    "method": "client.events.onPreferredAudioLanguagesChanged",
    "params": {
        "preferredLanguages": "eng"
    }
}
```

<a name="event.onPresentationLanguageChanged"></a>
## *onPresentationLanguageChanged [<sup>event</sup>](#head.Notifications)*

Triggered after the Presentation Language changes (see `SetPresentationLanguage`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.presentationLanguage | string | Receive Presentation Language changes |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onPresentationLanguageChanged",
    "params": {
        "presentationLanguage": "en-US"
    }
}
```

<a name="event.onCaptionsChanged"></a>
## *onCaptionsChanged [<sup>event</sup>](#head.Notifications)*

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
    "method": "client.events.onCaptionsChanged",
    "params": {
        "enabled": true
    }
}
```

<a name="event.onPreferredCaptionsLanguagesChanged"></a>
## *onPreferredCaptionsLanguagesChanged [<sup>event</sup>](#head.Notifications)*

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
    "method": "client.events.onPreferredCaptionsLanguagesChanged",
    "params": {
        "preferredLanguages": "eng"
    }
}
```

<a name="event.onPreferredClosedCaptionServiceChanged"></a>
## *onPreferredClosedCaptionServiceChanged [<sup>event</sup>](#head.Notifications)*

Triggered after the Preferred Closed Caption changes (see `SetPreferredClosedCaptionService`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.service | string | Receive Preferred Closed Caption Service changes |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onPreferredClosedCaptionServiceChanged",
    "params": {
        "service": "CC3"
    }
}
```

<a name="event.onPinControlChanged"></a>
## *onPinControlChanged [<sup>event</sup>](#head.Notifications)*

Triggered after the pin control changes (see `setPinControl`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | Receive pin control changes enable or not |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onPinControlChanged",
    "params": {
        "enabled": true
    }
}
```

<a name="event.onViewingRestrictionsChanged"></a>
## *onViewingRestrictionsChanged [<sup>event</sup>](#head.Notifications)*

Triggered after the viewingRestrictions changes (see `setViewingRestrictions`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | Receive viewingRestrictions changes enable or not |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onViewingRestrictionsChanged",
    "params": {
        "enabled": true
    }
}
```

<a name="event.onViewingRestrictionsWindowChanged"></a>
## *onViewingRestrictionsWindowChanged [<sup>event</sup>](#head.Notifications)*

Triggered after the viewingRestrictionsWindow changes (see `setViewingRestrictionsWindow`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | Receive viewingRestrictionsWindow changes enable or not |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onViewingRestrictionsWindowChanged",
    "params": {
        "enabled": true
    }
}
```

<a name="event.onLiveWatershedChanged"></a>
## *onLiveWatershedChanged [<sup>event</sup>](#head.Notifications)*

Triggered after the liveWatershed changes (see `setLiveWatershed`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | Receives liveWatershed changes enable or not |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onLiveWatershedChanged",
    "params": {
        "enabled": true
    }
}
```

<a name="event.onPlaybackWatershedChanged"></a>
## *onPlaybackWatershedChanged [<sup>event</sup>](#head.Notifications)*

Triggered after the playbackWatershed changes (see `setPlaybackWatershed`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | Receive playbackWatershed changes enable or not |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onPlaybackWatershedChanged",
    "params": {
        "enabled": true
    }
}
```

<a name="event.onBlockNotRatedContentChanged"></a>
## *onBlockNotRatedContentChanged [<sup>event</sup>](#head.Notifications)*

Triggered after the blockNotRatedContent changes (see `setBlockNotRatedContent`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | Receive blockNotRatedContent changes enable or not |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onBlockNotRatedContentChanged",
    "params": {
        "enabled": true
    }
}
```

<a name="event.onPinOnPurchaseChanged"></a>
## *onPinOnPurchaseChanged [<sup>event</sup>](#head.Notifications)*

Triggered after the pinOnPurchase changes (see `setPinOnPurchase`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | Receive pinOnPurchase changes enable or not |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onPinOnPurchaseChanged",
    "params": {
        "enabled": true
    }
}
```

