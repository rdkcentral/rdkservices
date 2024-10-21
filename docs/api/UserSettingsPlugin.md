<!-- Generated automatically, DO NOT EDIT! -->
<a name="UserSettings_Plugin"></a>
# UserSettings Plugin

**Version: [1.2.0](https://github.com/rdkcentral/rdkservices/blob/main/UserSettings/CHANGELOG.md)**

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

org.rdk.UserSettings interface methods:

| Method | Description |
| :-------- | :-------- |
| [setAudioDescription](#setAudioDescription) | Setting Audio Description |
| [setPreferredAudioLanguages](#setPreferredAudioLanguages) | Setting Preferred Audio Languages |
| [setPresentationLanguage](#setPresentationLanguage) | Setting Presentation Languages |
| [setCaptions](#setCaptions) | Setting Captions |
| [setPreferredCaptionsLanguages](#setPreferredCaptionsLanguages) | Setting PreferredCaption Languages |
| [setPreferredClosedCaptionService](#setPreferredClosedCaptionService) | Setting Preferred Closed Caption Service |
| [setPinControl](#setPinControl) | Setting PinControl |
| [setViewingRestrictions](#setViewingRestrictions) | Setting ViewingRestrictions |
| [setViewingRestrictionsWindow](#setViewingRestrictionsWindow) | Setting viewingRestrictionsWindow |
| [setLiveWatershed](#setLiveWatershed) | Setting LiveWatershed |
| [setPlaybackWatershed](#setPlaybackWatershed) | Setting PlaybackWatershed |
| [setBlockNotRatedContent](#setBlockNotRatedContent) | Setting BlockNotRatedContent |
| [setPinOnPurchase](#setPinOnPurchase) | Setting setPinOnPurchase |
| [getAudioDescription](#getAudioDescription) | Returns Audio Description |
| [getPreferredAudioLanguages](#getPreferredAudioLanguages) | Returns Preferred Audio Languages |
| [getPresentationLanguage](#getPresentationLanguage) | Getting Presentation Languages |
| [getCaptions](#getCaptions) | Getting Captions Enabled |
| [getPreferredCaptionsLanguages](#getPreferredCaptionsLanguages) | Getting Preferred Caption Languages |
| [getPreferredClosedCaptionService](#getPreferredClosedCaptionService) | Getting Preferred ClosedCaption Service |
| [getPinControl](#getPinControl) | Returns Pin Control |
| [getViewingRestrictions](#getViewingRestrictions) | Returns Get Viewing Restrictions |
| [getViewingRestrictionsWindow](#getViewingRestrictionsWindow) | Returns Get Viewing Restrictions Window |
| [getLiveWatershed](#getLiveWatershed) | Returns Live Watershed |
| [getPlaybackWatershed](#getPlaybackWatershed) | Returns Playback Watershed |
| [getBlockNotRatedContent](#getBlockNotRatedContent) | Returns BlockNotRatedContent |
| [getPinOnPurchase](#getPinOnPurchase) | Returns PinOnPurchase |


<a name="setAudioDescription"></a>
## *setAudioDescription*

Setting Audio Description.

### Events

| Event | Description |
| :-------- | :-------- |
| [OnAudioDescriptionChanged](#OnAudioDescriptionChanged) | Triggered when the audio description changes. |
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

<a name="setPreferredAudioLanguages"></a>
## *setPreferredAudioLanguages*

Setting Preferred Audio Languages.

### Events

| Event | Description |
| :-------- | :-------- |
| [onPreferredAudioLanguagesChanged](#onPreferredAudioLanguagesChanged) | Triggered when the audio preferred Audio languages changes. |
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

<a name="setPresentationLanguage"></a>
## *setPresentationLanguage*

Setting Presentation Languages.

### Events

| Event | Description |
| :-------- | :-------- |
| [onPresentationLanguageChanged](#onPresentationLanguageChanged) | Triggered when the presentation Language changes. |
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

<a name="setCaptions"></a>
## *setCaptions*

Setting Captions.

### Events

| Event | Description |
| :-------- | :-------- |
| [onCaptionsChanged](#onCaptionsChanged) | Triggered when the captions changes. |
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

<a name="setPreferredCaptionsLanguages"></a>
## *setPreferredCaptionsLanguages*

Setting PreferredCaption Languages.

### Events

| Event | Description |
| :-------- | :-------- |
| [onPreferredCaptionsLanguagesChanged](#onPreferredCaptionsLanguagesChanged) | Triggered when the PreferredCaption Languages changes. |
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

<a name="setPreferredClosedCaptionService"></a>
## *setPreferredClosedCaptionService*

Setting Preferred Closed Caption Service.

### Events

| Event | Description |
| :-------- | :-------- |
| [onPreferredClosedCaptionServiceChanged](#onPreferredClosedCaptionServiceChanged) | Triggered when the Preferred Closed Caption changes. |
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

<a name="setPinControl"></a>
## *setPinControl*

Setting PinControl.

### Events

| Event | Description |
| :-------- | :-------- |
| [onPinControlChanged](#onPinControlChanged) | Triggered when the pincontrol changes. |
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

<a name="setViewingRestrictions"></a>
## *setViewingRestrictions*

Setting ViewingRestrictions.

### Events

| Event | Description |
| :-------- | :-------- |
| [OnViewingRestrictionsChanged](#OnViewingRestrictionsChanged) | Triggered when the viewingRestrictions changes. |
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

<a name="setViewingRestrictionsWindow"></a>
## *setViewingRestrictionsWindow*

Setting viewingRestrictionsWindow.

### Events

| Event | Description |
| :-------- | :-------- |
| [OnViewingRestrictionsWindowChanged](#OnViewingRestrictionsWindowChanged) | Triggered when the viewingRestrictionsWindow changes. |
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

<a name="setLiveWatershed"></a>
## *setLiveWatershed*

Setting LiveWatershed.

### Events

| Event | Description |
| :-------- | :-------- |
| [OnLiveWatershedChanged](#OnLiveWatershedChanged) | Triggered when the liveWatershed changes. |
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

<a name="setPlaybackWatershed"></a>
## *setPlaybackWatershed*

Setting PlaybackWatershed.

### Events

| Event | Description |
| :-------- | :-------- |
| [OnPlaybackWatershedChanged](#OnPlaybackWatershedChanged) | Triggered when the playbackWatershed changes. |
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

<a name="setBlockNotRatedContent"></a>
## *setBlockNotRatedContent*

Setting BlockNotRatedContent.

### Events

| Event | Description |
| :-------- | :-------- |
| [OnBlockNotRatedContentChanged](#OnBlockNotRatedContentChanged) | Triggered when the blockNotRatedContent changes. |
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

<a name="setPinOnPurchase"></a>
## *setPinOnPurchase*

Setting setPinOnPurchase.

### Events

| Event | Description |
| :-------- | :-------- |
| [OnPinOnPurchaseChanged](#OnPinOnPurchaseChanged) | Triggered when the pin on the purchase changes. |
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

<a name="getAudioDescription"></a>
## *getAudioDescription*

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

<a name="getPreferredAudioLanguages"></a>
## *getPreferredAudioLanguages*

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

<a name="getPresentationLanguage"></a>
## *getPresentationLanguage*

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

<a name="getCaptions"></a>
## *getCaptions*

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

<a name="getPreferredCaptionsLanguages"></a>
## *getPreferredCaptionsLanguages*

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

<a name="getPreferredClosedCaptionService"></a>
## *getPreferredClosedCaptionService*

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

<a name="getPinControl"></a>
## *getPinControl*

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

<a name="getViewingRestrictions"></a>
## *getViewingRestrictions*

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

<a name="getViewingRestrictionsWindow"></a>
## *getViewingRestrictionsWindow*

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

<a name="getLiveWatershed"></a>
## *getLiveWatershed*

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

<a name="getPlaybackWatershed"></a>
## *getPlaybackWatershed*

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

<a name="getBlockNotRatedContent"></a>
## *getBlockNotRatedContent*

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

<a name="getPinOnPurchase"></a>
## *getPinOnPurchase*

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

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.UserSettings plugin:

org.rdk.UserSettings interface events:

| Event | Description |
| :-------- | :-------- |
| [onAudioDescriptionChanged](#onAudioDescriptionChanged) | Triggered after the audio description changes (see `SetAudioDescription`) |
| [onPreferredAudioLanguagesChanged](#onPreferredAudioLanguagesChanged) | Triggered after the audio preferred Audio languages changes (see `SetPreferredAudioLanguages`) |
| [onPresentationLanguageChanged](#onPresentationLanguageChanged) | Triggered after the Presentation Language changes (see `SetPresentationLanguage`) |
| [onCaptionsChanged](#onCaptionsChanged) | Triggered after the captions changes (see `SetCaptions`) |
| [onPreferredCaptionsLanguagesChanged](#onPreferredCaptionsLanguagesChanged) | Triggered after the PreferredCaption Languages changes (see `SetPreferredCaptionsLanguages`) |
| [onPreferredClosedCaptionServiceChanged](#onPreferredClosedCaptionServiceChanged) | Triggered after the Preferred Closed Caption changes (see `SetPreferredClosedCaptionService`) |
| [onPinControlChanged](#onPinControlChanged) | Triggered after the pin control changes (see `setPinControl`) |
| [onViewingRestrictionsChanged](#onViewingRestrictionsChanged) | Triggered after the viewingRestrictions changes (see `setViewingRestrictions`) |
| [onViewingRestrictionsWindowChanged](#onViewingRestrictionsWindowChanged) | Triggered after the viewingRestrictionsWindow changes (see `setViewingRestrictionsWindow`) |
| [onLiveWatershedChanged](#onLiveWatershedChanged) | Triggered after the liveWatershed changes (see `setLiveWatershed`) |
| [onPlaybackWatershedChanged](#onPlaybackWatershedChanged) | Triggered after the playbackWatershed changes (see `setPlaybackWatershed`) |
| [onBlockNotRatedContentChanged](#onBlockNotRatedContentChanged) | Triggered after the blockNotRatedContent changes (see `setBlockNotRatedContent`) |
| [onPinOnPurchaseChanged](#onPinOnPurchaseChanged) | Triggered after the pinOnPurchase changes (see `setPinOnPurchase`) |


<a name="onAudioDescriptionChanged"></a>
## *onAudioDescriptionChanged*

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

<a name="onPreferredAudioLanguagesChanged"></a>
## *onPreferredAudioLanguagesChanged*

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

<a name="onPresentationLanguageChanged"></a>
## *onPresentationLanguageChanged*

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

<a name="onCaptionsChanged"></a>
## *onCaptionsChanged*

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

<a name="onPreferredCaptionsLanguagesChanged"></a>
## *onPreferredCaptionsLanguagesChanged*

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

<a name="onPreferredClosedCaptionServiceChanged"></a>
## *onPreferredClosedCaptionServiceChanged*

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

<a name="onPinControlChanged"></a>
## *onPinControlChanged*

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

<a name="onViewingRestrictionsChanged"></a>
## *onViewingRestrictionsChanged*

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

<a name="onViewingRestrictionsWindowChanged"></a>
## *onViewingRestrictionsWindowChanged*

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

<a name="onLiveWatershedChanged"></a>
## *onLiveWatershedChanged*

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

<a name="onPlaybackWatershedChanged"></a>
## *onPlaybackWatershedChanged*

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

<a name="onBlockNotRatedContentChanged"></a>
## *onBlockNotRatedContentChanged*

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

<a name="onPinOnPurchaseChanged"></a>
## *onPinOnPurchaseChanged*

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

