<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.DisplaySettings_Plugin"></a>
# DisplaySettings Plugin

**Version: 2.0**

**Status: :black_circle::black_circle::black_circle:**

A org.rdk.DisplaySettings plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Methods](#head.Methods)
- [Notifications](#head.Notifications)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the org.rdk.DisplaySettings plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

<a name="head.Case_Sensitivity"></a>
## Case Sensitivity

All identifiers of the interfaces described in this document are case-sensitive. Thus, unless stated otherwise, all keywords, entities, properties, relations and actions should be treated as such.

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

The `DisplaySetting` plugin provides an interface for display information such as current video resolution, supported video displays, zoom setting, sound mode, and much more.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.DisplaySettings*) |
| classname | string | Class name: *org.rdk.DisplaySettings* |
| locator | string | Library name: *libWPEFrameworkDisplaySettings.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.DisplaySettings plugin:

DisplaySettings interface methods:

| Method | Description |
| :-------- | :-------- |
| [enableSurroundDecoder](#method.enableSurroundDecoder) | Enables or disables Surround Decoder capability |
| [getActiveInput](#method.getActiveInput) | Returns `true` if the STB HDMI output is currently connected to the active input of the sink device (determined by `RxSense`) |
| [getAssociatedAudioMixing](#method.getAssociatedAudioMixing) | Returns whether associated audio mixing is enabled or disabled |
| [getAudioDelay](#method.getAudioDelay) | Returns the audio delay (in ms) on the selected audio port |
| [getAudioDelayOffset](#method.getAudioDelayOffset) | Returns the audio delay offset (in ms) on the selected audio port |
| [getAudioFormat](#method.getAudioFormat) | Returns the currently set audio format |
| [getBassEnhancer](#method.getBassEnhancer) | Returns the current status of the Bass Enhancer settings |
| [getConnectedAudioPorts](#method.getConnectedAudioPorts) | Returns connected audio output ports (a subset of the ports supported on the device) |
| [getConnectedVideoDisplays](#method.getConnectedVideoDisplays) | Returns connected video displays |
| [getCurrentOutputSettings](#method.getCurrentOutputSettings) | Returns current output settings |
| [getCurrentResolution](#method.getCurrentResolution) | Returns the current resolution on the selected video display port |
| [getDefaultResolution](#method.getDefaultResolution) | Gets the default resolution supported by the HDMI0 video output port |
| [getDialogEnhancement](#method.getDialogEnhancement) | Returns the current Dialog Enhancer level (port HDMI0) |
| [getDolbyVolumeMode](#method.getDolbyVolumeMode) | Returns whether Dolby Volume mode is enabled or disabled (audio output port HDMI0) |
| [getDRCMode](#method.getDRCMode) | Returns the current Dynamic Range Control mode |
| [getEnableAudioPort](#method.getEnableAudioPort) |  Returns the current status of the specified input audio port |
| [getFaderControl](#method.getFaderControl) | Returns the current audio mixer balance setting |
| [getGain](#method.getGain) | Returns the current gain value |
| [getGraphicEqualizerMode](#method.getGraphicEqualizerMode) | Returns the current Graphic Equalizer Mode setting (port HDMI0) |
| [getIntelligentEqualizerMode](#method.getIntelligentEqualizerMode) | Returns the current Intelligent Equalizer Mode setting (port HDMI0) |
| [getMISteering](#method.getMISteering) | Returns the current status of Media Intelligence Steering settings |
| [getMS12AudioCompression](#method.getMS12AudioCompression) | Returns the current audio compression settings |
| [getMS12AudioProfile](#method.getMS12AudioProfile) | Returns the current MS12 audio profile settings |
| [getMuted](#method.getMuted) | Returns whether audio is muted on a given port |
| [getPrimaryLanguage](#method.getPrimaryLanguage) | Returns the current primary language |
| [getSecondaryLanguage](#method.getSecondaryLanguage) | Returns the current secondary language |
| [getSettopAudioCapabilities](#method.getSettopAudioCapabilities) | Returns the set-top audio capabilities for the specified audio port |
| [getSettopHDRSupport](#method.getSettopHDRSupport) | Returns an HDR support object (list of standards that the STB supports) |
| [getSettopMS12Capabilities](#method.getSettopMS12Capabilities) | Returns the set-top MS12 audio capabilities for the specified audio port |
| [getSinkAtmosCapability](#method.getSinkAtmosCapability) | Returns the ATMOS capability of the sink (HDMI0) |
| [getSoundMode](#method.getSoundMode) | Returns the sound mode for the incoming video display |
| [getSupportedAudioModes](#method.getSupportedAudioModes) | Returns a list of strings containing the supported audio modes |
| [getSupportedAudioPorts](#method.getSupportedAudioPorts) | Returns all audio ports supported on the device (all ports that are physically present) |
| [getSupportedMS12AudioProfiles](#method.getSupportedMS12AudioProfiles) | Returns list of platform supported MS12 audio profiles for the specified audio port |
| [getSupportedResolutions](#method.getSupportedResolutions) | Returns supported resolutions on the selected video display port (both TV and STB) by its name |
| [getSupportedSettopResolutions](#method.getSupportedSettopResolutions) | Returns supported STB resolutions |
| [getSupportedTvResolutions](#method.getSupportedTvResolutions) | Returns supported TV resolutions on the selected video display port |
| [getSupportedVideoDisplays](#method.getSupportedVideoDisplays) | Returns all video ports supported on the device (all ports that are physically present) |
| [getSurroundVirtualizer](#method.getSurroundVirtualizer) | (Version 2) Returns the current surround virtualizer boost settings |
| [getTVHDRCapabilities](#method.getTVHDRCapabilities) | Gets HDR capabilities supported by the TV |
| [getTvHDRSupport](#method.getTvHDRSupport) | Returns an HDR support object (list of standards that the TV supports) |
| [getVideoFormat](#method.getVideoFormat) | Returns the current and supported video formats |
| [getVideoPortStatusInStandby](#method.getVideoPortStatusInStandby) | Returns video port status in standby mode (failure if the port name is missing) |
| [getVolumeLevel](#method.getVolumeLevel) | Returns the current volume level |
| [getVolumeLeveller](#method.getVolumeLeveller) | (Version 2) Returns the current Volume Leveller setting |
| [getZoomSetting](#method.getZoomSetting) | Returns the zoom setting value |
| [isConnectedDeviceRepeater](#method.isConnectedDeviceRepeater) | Indicates whether the device connected to the HDMI0 video output port is an HDCP repeater |
| [isSurroundDecoderEnabled](#method.isSurroundDecoderEnabled) | Returns the current status of Surround Decoder |
| [readEDID](#method.readEDID) | Reads the EDID from the connected HDMI (output) device |
| [readHostEDID](#method.readHostEDID) | Reads the EDID of the host |
| [resetBassEnhancer](#method.resetBassEnhancer) | Resets the dialog enhancer level to its default bassboost value |
| [resetDialogEnhancement](#method.resetDialogEnhancement) | Resets the dialog enhancer level to its default enhancer level |
| [resetSurroundVirtualizer](#method.resetSurroundVirtualizer) | Resets the surround virtualizer to its default boost value |
| [resetVolumeLeveller](#method.resetVolumeLeveller) | Resets the Volume Leveller level to default volume value |
| [setAssociatedAudioMixing](#method.setAssociatedAudioMixing) | Enables or disables associated audio mixing |
| [setAudioAtmosOutputMode](#method.setAudioAtmosOutputMode) | Sets ATMOS audio output mode (on HDMI0) |
| [setAudioDelay](#method.setAudioDelay) | Sets the audio delay (in ms) on the selected audio port |
| [setAudioDelayOffset](#method.setAudioDelayOffset) | Sets the audio delay offset (in ms) on the selected audio port |
| [setBassEnhancer](#method.setBassEnhancer) | Sets the Bass Enhancer |
| [setCurrentResolution](#method.setCurrentResolution) | Sets the current resolution |
| [setDialogEnhancement](#method.setDialogEnhancement) | Sets the Dialog Enhancer level |
| [setDolbyVolumeMode](#method.setDolbyVolumeMode) | Enables or disables Dolby Volume mode on audio track (audio output port HDMI0) |
| [setDRCMode](#method.setDRCMode) | Sets the Dynamic Range Control (DRC) setting |
| [setEnableAudioPort](#method.setEnableAudioPort) | Enable or disable the specified audio port based on the input audio port name |
| [setFaderControl](#method.setFaderControl) | Sets the audio mixer balance |
| [setForceHDRMode](#method.setForceHDRMode) | Enables or disables the force HDR mode |
| [setGain](#method.setGain) | Adjusts the gain on a specific port |
| [setGraphicEqualizerMode](#method.setGraphicEqualizerMode) | Sets the Graphic Equalizer Mode |
| [setIntelligentEqualizerMode](#method.setIntelligentEqualizerMode) | Sets the Intelligent Equalizer Mode (port HDMI0) |
| [setMISteering](#method.setMISteering) | Enables or Disables Media Intelligent Steering |
| [setMS12AudioCompression](#method.setMS12AudioCompression) | Sets the audio dynamic range compression level (port HDMI0) |
| [setMS12AudioProfile](#method.setMS12AudioProfile) | Sets the selected MS12 audio profile |
| [setMS12ProfileSettingsOverride](#method.setMS12ProfileSettingsOverride) | Overrides individual MS12 audio settings in order to optimize the customer experience (for example, enabling dialog enhancement in sports mode) |
| [setMuted](#method.setMuted) | Mutes or unmutes audio on a specific port |
| [setPrimaryLanguage](#method.setPrimaryLanguage) | Sets the primary language |
| [setScartParameter](#method.setScartParameter) | Sets SCART parameters |
| [setSecondaryLanguage](#method.setSecondaryLanguage) | Sets the secondary language |
| [setSoundMode](#method.setSoundMode) | Sets the current sound mode for the corresponding video display |
| [setSurroundVirtualizer](#method.setSurroundVirtualizer) | (Version 2) Sets the Surround Virtualizer boost |
| [setVideoPortStatusInStandby](#method.setVideoPortStatusInStandby) | Sets the specified video port status to be used in standby mode (failure if the port name is missing) |
| [setVolumeLevel](#method.setVolumeLevel) | Adjusts the Volume Level on a specific port |
| [setVolumeLeveller](#method.setVolumeLeveller) | (Version 2) Sets the Volume Leveller level |
| [setZoomSetting](#method.setZoomSetting) | Sets the current zoom value |


<a name="method.enableSurroundDecoder"></a>
## *enableSurroundDecoder [<sup>method</sup>](#head.Methods)*

Enables or disables Surround Decoder capability. The Surround Decoder is an upmixer that takes stereo music content, or surround-encoded two-channel movie content, and creates a high-quality multichannel upmix. If the Surround Decoder is enabled, two-channel signals and 5.1-channel signals are upmixed to 5.1.2.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |
| params.surroundDecoderEnable | boolean | Whether Surround Decoder is is enabled (`true`) or disabled (`false`) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.enableSurroundDecoder",
    "params": {
        "audioPort": "SPEAKER0",
        "surroundDecoderEnable": true
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.getActiveInput"></a>
## *getActiveInput [<sup>method</sup>](#head.Methods)*

Returns `true` if the STB HDMI output is currently connected to the active input of the sink device (determined by `RxSense`).
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.videoDisplay | string | <sup>*(optional)*</sup> Video display port name. The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.activeInput | boolean | `true`:  1. STB is connected to TV's active input 2. Unable to determine if STB is connected to the TV's active input or not (because STB does not support RxSense) ; `false`: 1.STB is not connected to TV's Active input 2. TV is off  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getActiveInput",
    "params": {
        "videoDisplay": "HDMI0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "activeInput": true,
        "success": true
    }
}
```

<a name="method.getAssociatedAudioMixing"></a>
## *getAssociatedAudioMixing [<sup>method</sup>](#head.Methods)*

Returns whether associated audio mixing is enabled or disabled.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name. For STB, the `HDMI0` port handle can be used. The default port is `HDMI0` if no port is specified. For TV products, unlike other MS12 APIs, an audio port is not required because the API is not specific to an audio port but is instead a platform/host configuration |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.mixing | boolean | Whether associated audio mixing is enabled (`true`) or disabled (`false`) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getAssociatedAudioMixing",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "mixing": true,
        "success": true
    }
}
```

<a name="method.getAudioDelay"></a>
## *getAudioDelay [<sup>method</sup>](#head.Methods)*

Returns the audio delay (in ms) on the selected audio port. If the `audioPort` argument is not specified, it will browse all ports (checking HDMI0 first). If there is no display connected, then it defaults to `HDMI0`.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.audioPort | string | Audio port name |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.audioDelay | string | Delay (in ms) on the selected audio port |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getAudioDelay",
    "params": {
        "audioPort": "HDMI0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "audioDelay": "0",
        "success": true
    }
}
```

<a name="method.getAudioDelayOffset"></a>
## *getAudioDelayOffset [<sup>method</sup>](#head.Methods)*

Returns the audio delay offset (in ms) on the selected audio port. If the `audioPort` argument is not specified, it will browse all ports (checking HDMI0 first). If there is no display connected, then it defaults to `HDMI0`.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.audioPort | string | Audio port name |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.audioDelayOffset | string | Delay offset (in ms) on the selected audio port |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getAudioDelayOffset",
    "params": {
        "audioPort": "HDMI0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "audioDelayOffset": "0",
        "success": true
    }
}
```

<a name="method.getAudioFormat"></a>
## *getAudioFormat [<sup>method</sup>](#head.Methods)*

Returns the currently set audio format.
 
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.supportedAudioFormat | array | A list of supported audio formats |
| result.supportedAudioFormat[#] | string |  |
| result.currentAudioFormat | string | The currently set audio format |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getAudioFormat"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "supportedAudioFormat": [
            "`NONE`, `PCM`, `DOLBY AC3`, `DOLBY EAC3`, `DOLBY AC4`, `DOLBY MAT', 'DOLBY TRUEHD', 'DOLBY EAC3 ATMOS', 'DOLBY TRUEHD ATMOS', 'DOLBY MAT ATMOS', 'DOLBY AC4 ATMOS'"
        ],
        "currentAudioFormat": "PCM",
        "success": true
    }
}
```

<a name="method.getBassEnhancer"></a>
## *getBassEnhancer [<sup>method</sup>](#head.Methods)*

Returns the current status of the Bass Enhancer settings.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.enable | boolean | <sup>*(optional)*</sup> `true` if Bass Enhancer is enabled, otherwise `false` |
| result?.bassBoost | integer | <sup>*(optional)*</sup> Value between 0 and 100, where 0 means no bass boost (disabled) and 100 means max bass boost |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getBassEnhancer",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "enable": true,
        "bassBoost": 50,
        "success": true
    }
}
```

<a name="method.getConnectedAudioPorts"></a>
## *getConnectedAudioPorts [<sup>method</sup>](#head.Methods)*

Returns connected audio output ports (a subset of the ports supported on the device). For SPDIF supported platforms, SPDIF port is always considered connected. HDMI port may or may not be connected.
 
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.connectedAudioPorts | array | A string [] of connected audio port names |
| result.connectedAudioPorts[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getConnectedAudioPorts"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "connectedAudioPorts": [
            "HDMI0"
        ],
        "success": true
    }
}
```

<a name="method.getConnectedVideoDisplays"></a>
## *getConnectedVideoDisplays [<sup>method</sup>](#head.Methods)*

Returns connected video displays.
 
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.connectedVideoDisplays | array | A string [] of connected video display port names |
| result.connectedVideoDisplays[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getConnectedVideoDisplays"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "connectedVideoDisplays": [
            "HDMI0"
        ],
        "success": true
    }
}
```

<a name="method.getCurrentOutputSettings"></a>
## *getCurrentOutputSettings [<sup>method</sup>](#head.Methods)*

Returns current output settings.
 
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.colorSpace | integer | The color space. Possible values: `0` (dsDISPLAY_COLORSPACE_UNKNOWN), `1` (sDISPLAY_COLORSPACE_RGB, `2` (dsDISPLAY_COLORSPACE_YCbCr422), `3` (dsDISPLAY_COLORSPACE_YCbCr444), `4` (dsDISPLAY_COLORSPACE_YCbCr420), `5` (dsDISPLAY_COLORSPACE_AUTO) |
| result.colorDepth | integer | The color depth. The value that is returned from `dsGetCurrentOutputSettings` |
| result.matrixCoefficients | integer | matrix coefficients. Possible values: `0` (dsDISPLAY_MATRIXCOEFFICIENT_UNKNOWN), `1` (dsDISPLAY_MATRIXCOEFFICIENT_BT_709), `2` (dsDISPLAY_MATRIXCOEFFICIENT_BT_470_2_BG), `3` (dsDISPLAY_MATRIXCOEFFICIENT_SMPTE_170M), `4` (dsDISPLAY_MATRIXCOEFFICIENT_XvYCC_709), `5` (dsDISPLAY_MATRIXCOEFFICIENT_eXvYCC_601), `6` (dsDISPLAY_MATRIXCOEFFICIENT_BT_2020_NCL), `7` (dsDISPLAY_MATRIXCOEFFICIENT_BT_2020_CL) |
| result.videoEOTF | integer | HDR standard. Possible values: `0x0` (dsHDRSTANDARD_NONE), `0x01` (dsHDRSTANDARD_HDR10), `0x02` (dsHDRSTANDARD_HLG), `0x04` (dsHDRSTANDARD_DolbyVision), `0x08` (dsHDRSTANDARD_TechnicolorPrime), `0x80` (dsHDRSTANDARD_Invalid) |
| result?.quantizationRange | integer | <sup>*(optional)*</sup> The supported quantization range |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getCurrentOutputSettings"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "colorSpace": 5,
        "colorDepth": 0,
        "matrixCoefficients": 0,
        "videoEOTF": 0,
        "quantizationRange": 235,
        "success": true
    }
}
```

<a name="method.getCurrentResolution"></a>
## *getCurrentResolution [<sup>method</sup>](#head.Methods)*

Returns the current resolution on the selected video display port.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.videoDisplay | string | <sup>*(optional)*</sup> Video display port name. The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.resolution | string | <sup>*(optional)*</sup> Video display resolution |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getCurrentResolution",
    "params": {
        "videoDisplay": "HDMI0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "resolution": "1080p",
        "success": true
    }
}
```

<a name="method.getDefaultResolution"></a>
## *getDefaultResolution [<sup>method</sup>](#head.Methods)*

Gets the default resolution supported by the HDMI0 video output port.
 
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.defaultResolution | string | Default resolution supported by the HDMI0 video output port |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getDefaultResolution"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "defaultResolution": "720p",
        "success": true
    }
}
```

<a name="method.getDialogEnhancement"></a>
## *getDialogEnhancement [<sup>method</sup>](#head.Methods)*

Returns the current Dialog Enhancer level (port HDMI0).
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.enable | boolean | `true` if Dialog Enhancer Mode is enabled, otherwise `false` |
| result.enhancerlevel | integer | Value between 0 and 16, where 0 means no enhancement and 16 means maximum enhancement |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getDialogEnhancement",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "enable": false,
        "enhancerlevel": 0,
        "success": true
    }
}
```

<a name="method.getDolbyVolumeMode"></a>
## *getDolbyVolumeMode [<sup>method</sup>](#head.Methods)*

Returns whether Dolby Volume mode is enabled or disabled (audio output port HDMI0).
 
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.dolbyVolumeMode | boolean | <sup>*(optional)*</sup> Whether Dolby Volume mode is enabled (`true`) or disabled (`false`) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getDolbyVolumeMode"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "dolbyVolumeMode": true,
        "success": true
    }
}
```

<a name="method.getDRCMode"></a>
## *getDRCMode [<sup>method</sup>](#head.Methods)*

Returns the current Dynamic Range Control mode.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.DRCMode | string | <sup>*(optional)*</sup> The DRC Mode value: either `line` or `RF` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getDRCMode",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "DRCMode": "line",
        "success": true
    }
}
```

<a name="method.getEnableAudioPort"></a>
## *getEnableAudioPort [<sup>method</sup>](#head.Methods)*

 Returns the current status of the specified input audio port.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.enable | boolean | `true` if the audio port is enabled, otherwise `false` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getEnableAudioPort",
    "params": {
        "audioPort": "HDMI0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "enable": true,
        "success": true
    }
}
```

<a name="method.getFaderControl"></a>
## *getFaderControl [<sup>method</sup>](#head.Methods)*

Returns the current audio mixer balance setting.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name. For STB, the `HDMI0` port handle can be used. The default port is `HDMI0` if no port is specified. For TV products, unlike other MS12 APIs, an audio port is not required because the API is not specific to an audio port but is instead a platform/host configuration |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.mixerBalance | integer | The audi mixer balance. The value should be between -32 to +32 |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getFaderControl",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "mixerBalance": 10,
        "success": true
    }
}
```

<a name="method.getGain"></a>
## *getGain [<sup>method</sup>](#head.Methods)*

Returns the current gain value.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.gain | number | Value between 0 and 100, where 0 means no gain and 100 means maximum gain |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getGain",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "gain": 10.0,
        "success": true
    }
}
```

<a name="method.getGraphicEqualizerMode"></a>
## *getGraphicEqualizerMode [<sup>method</sup>](#head.Methods)*

Returns the current Graphic Equalizer Mode setting (port HDMI0).
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.enable | boolean | `true` if Graphic Equalizer Mode is enabled, otherwise `false` |
| result.mode | integer | Graphic Equalizer mode (`0` = mode not set or in case of error, `1` = open, `2` = rich, `3` = focused) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getGraphicEqualizerMode",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "enable": true,
        "mode": 2,
        "success": true
    }
}
```

<a name="method.getIntelligentEqualizerMode"></a>
## *getIntelligentEqualizerMode [<sup>method</sup>](#head.Methods)*

Returns the current Intelligent Equalizer Mode setting (port HDMI0).
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.enable | boolean | `true` if Intelligent Equalizer Mode is enabled, otherwise `false` |
| result.mode | integer | Intelligent Equalizer mode (`0` = off, `1` = open, `2` = rich, `3` = focused) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getIntelligentEqualizerMode",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "enable": true,
        "mode": 2,
        "success": true
    }
}
```

<a name="method.getMISteering"></a>
## *getMISteering [<sup>method</sup>](#head.Methods)*

Returns the current status of Media Intelligence Steering settings.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.MISteeringEnable | boolean | <sup>*(optional)*</sup> Whether Media Intelligence Steering is enabled (`true`) or disabled (`false`) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getMISteering",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "MISteeringEnable": true,
        "success": true
    }
}
```

<a name="method.getMS12AudioCompression"></a>
## *getMS12AudioCompression [<sup>method</sup>](#head.Methods)*

Returns the current audio compression settings.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.enable | boolean | `true` if audio compression is enabled, otherwise `false` |
| result.compressionLevel | integer | Value between 0 and 10, where 0 means no compression and 10 means maximum compression |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getMS12AudioCompression",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "enable": true,
        "compressionLevel": 5,
        "success": true
    }
}
```

<a name="method.getMS12AudioProfile"></a>
## *getMS12AudioProfile [<sup>method</sup>](#head.Methods)*

Returns the current MS12 audio profile settings.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.ms12AudioProfile | string | An MS12 audio profile name from `getSupportedMS12AudioProfile` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getMS12AudioProfile",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "ms12AudioProfile": "Game",
        "success": true
    }
}
```

<a name="method.getMuted"></a>
## *getMuted [<sup>method</sup>](#head.Methods)*

Returns whether audio is muted on a given port.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.muted | boolean | mute or unmute audio |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getMuted",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "muted": true,
        "success": true
    }
}
```

<a name="method.getPrimaryLanguage"></a>
## *getPrimaryLanguage [<sup>method</sup>](#head.Methods)*

Returns the current primary language.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name. For STB, the `HDMI0` port handle can be used. The default port is `HDMI0` if no port is specified. For TV products, unlike other MS12 APIs, an audio port is not required because the API is not specific to an audio port but is instead a platform/host configuration |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.lang | string | The 3 letter language code as per ISO 639 |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getPrimaryLanguage",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "lang": "eng",
        "success": true
    }
}
```

<a name="method.getSecondaryLanguage"></a>
## *getSecondaryLanguage [<sup>method</sup>](#head.Methods)*

Returns the current secondary language.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name. For STB, the `HDMI0` port handle can be used. The default port is `HDMI0` if no port is specified. For TV products, unlike other MS12 APIs, an audio port is not required because the API is not specific to an audio port but is instead a platform/host configuration |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.lang | string | The 3 letter language code as per ISO 639 |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getSecondaryLanguage",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "lang": "eng",
        "success": true
    }
}
```

<a name="method.getSettopAudioCapabilities"></a>
## *getSettopAudioCapabilities [<sup>method</sup>](#head.Methods)*

Returns the set-top audio capabilities for the specified audio port.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.AudioCapabilities | array | <sup>*(optional)*</sup> An array of audio capabilities supported by STB |
| result?.AudioCapabilities[#] | string | <sup>*(optional)*</sup> Audio capability name (None in case of no audio capability support)  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getSettopAudioCapabilities",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "AudioCapabilities": [
            "None"
        ],
        "success": true
    }
}
```

<a name="method.getSettopHDRSupport"></a>
## *getSettopHDRSupport [<sup>method</sup>](#head.Methods)*

Returns an HDR support object (list of standards that the STB supports).
 
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.standards | array | A string [] of HDR capabilities. Possible values: `none`, `HDR10`, `Dolby Vision`, `Technicolor Prime` |
| result.standards[#] | string |  |
| result.supportsHDR | boolean | Indicates support for HDR |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getSettopHDRSupport"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "standards": [
            "none"
        ],
        "supportsHDR": true,
        "success": true
    }
}
```

<a name="method.getSettopMS12Capabilities"></a>
## *getSettopMS12Capabilities [<sup>method</sup>](#head.Methods)*

Returns the set-top MS12 audio capabilities for the specified audio port.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.ms12Capabilities | array | A string [] of MS12 audio capabilities |
| result.ms12Capabilities[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getSettopMS12Capabilities",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "ms12Capabilities": [
            "Dolby Volume"
        ],
        "success": true
    }
}
```

<a name="method.getSinkAtmosCapability"></a>
## *getSinkAtmosCapability [<sup>method</sup>](#head.Methods)*

Returns the ATMOS capability of the sink (HDMI0).
 
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.atmos_capability | integer | <sup>*(optional)*</sup> ATMOS Capability (`0` = off, `1` = open, `2` = rich, `3` = focused) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getSinkAtmosCapability"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "atmos_capability": 2,
        "success": true
    }
}
```

<a name="method.getSoundMode"></a>
## *getSoundMode [<sup>method</sup>](#head.Methods)*

Returns the sound mode for the incoming video display. If the argument is `Null` or empty (although not recommended), this returns the output mode of all connected ports, whichever is connected, while giving priority to the HDMI port. If the video display is not connected, then it returns `Stereo` as a safe default.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.soundMode | string | Sound mode. Possible values: `AUTO (Dolby Digital Plus)`, `AUTO (Dolby Digital 5.1)`, `AUTO (Stereo)`, `MONO`, `STEREO`, `SURROUND`, PASSTHRU |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getSoundMode",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "soundMode": "STEREO",
        "success": true
    }
}
```

<a name="method.getSupportedAudioModes"></a>
## *getSupportedAudioModes [<sup>method</sup>](#head.Methods)*

Returns a list of strings containing the supported audio modes. If `Null` or empty, this returns the supported audio modes of the audio processor (regardless of the the output port).  
If a port name is specified, this returns the audio output modes supported by the connected sink device (EDID based). If the port is not connected, the return value is same as if `Null` is specified as the parameter.  
For **Auto** mode in DS5, this API has the following extra specification:  
* For HDMI port, this API returns `Stereo` mode, `Dolby Digital 5.1` mode and `Auto` mode; 
* For SPDIF and HDMI ARC port, this API always returns `Surround` mode, `Stereo` mode, and `PASSTHRU` Mode;  
* When `AUTO` mode is returned, it includes in parenthesis the best sound mode that the STB can output and the connected sink device can support, in the format of `AUTO` _(`Best Format`)_. For example, if the connected device supports surround, the auto mode string will be `AUTO (Dolby Digital 5.1)`.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.supportedAudioModes | array | A string [] of supported audio modes |
| result.supportedAudioModes[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getSupportedAudioModes",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "supportedAudioModes": [
            "STEREO"
        ],
        "success": true
    }
}
```

<a name="method.getSupportedAudioPorts"></a>
## *getSupportedAudioPorts [<sup>method</sup>](#head.Methods)*

Returns all audio ports supported on the device (all ports that are physically present).
 
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.supportedAudioPorts | array | A string [] of supported audio ports |
| result.supportedAudioPorts[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getSupportedAudioPorts"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "supportedAudioPorts": [
            "HDMI0"
        ],
        "success": true
    }
}
```

<a name="method.getSupportedMS12AudioProfiles"></a>
## *getSupportedMS12AudioProfiles [<sup>method</sup>](#head.Methods)*

Returns list of platform supported MS12 audio profiles for the specified audio port.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.supportedMS12AudioProfiles | array | A string [] of MS12 audio profiles |
| result.supportedMS12AudioProfiles[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getSupportedMS12AudioProfiles",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "supportedMS12AudioProfiles": [
            "Movie"
        ],
        "success": true
    }
}
```

<a name="method.getSupportedResolutions"></a>
## *getSupportedResolutions [<sup>method</sup>](#head.Methods)*

Returns supported resolutions on the selected video display port (both TV and STB) by its name.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.videoDisplay | string | <sup>*(optional)*</sup> Video display port name. The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.supportedResolutions | array | A string array of supported resolutions on the selected video display port |
| result.supportedResolutions[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getSupportedResolutions",
    "params": {
        "videoDisplay": "HDMI0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "supportedResolutions": [
            "1080p60"
        ],
        "success": true
    }
}
```

<a name="method.getSupportedSettopResolutions"></a>
## *getSupportedSettopResolutions [<sup>method</sup>](#head.Methods)*

Returns supported STB resolutions.
 
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.supportedSettopResolutions | array | A string array of supported STB resolutions |
| result.supportedSettopResolutions[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getSupportedSettopResolutions"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "supportedSettopResolutions": [
            "1080p60"
        ],
        "success": true
    }
}
```

<a name="method.getSupportedTvResolutions"></a>
## *getSupportedTvResolutions [<sup>method</sup>](#head.Methods)*

Returns supported TV resolutions on the selected video display port.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.videoDisplay | string | <sup>*(optional)*</sup> Video display port name. The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.supportedTvResolutions | array | A string [] of supported TV resolutions |
| result.supportedTvResolutions[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getSupportedTvResolutions",
    "params": {
        "videoDisplay": "HDMI0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "supportedTvResolutions": [
            "1080p"
        ],
        "success": true
    }
}
```

<a name="method.getSupportedVideoDisplays"></a>
## *getSupportedVideoDisplays [<sup>method</sup>](#head.Methods)*

Returns all video ports supported on the device (all ports that are physically present).
 
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.supportedVideoDisplays | array | a string [] of supported video display ports |
| result.supportedVideoDisplays[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getSupportedVideoDisplays"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "supportedVideoDisplays": [
            "HDMI0"
        ],
        "success": true
    }
}
```

<a name="method.getSurroundVirtualizer"></a>
## *getSurroundVirtualizer [<sup>method</sup>](#head.Methods)*

(Version 2) Returns the current surround virtualizer boost settings.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.mode | integer | Enables or disables volume leveling (`0` = off, `1` = on, `2` = auto) |
| result.boost | integer | Value between 0 and 96, where 0 means no boost and 96 means maximum boost |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getSurroundVirtualizer",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "mode": 1,
        "boost": 90,
        "success": true
    }
}
```

<a name="method.getTVHDRCapabilities"></a>
## *getTVHDRCapabilities [<sup>method</sup>](#head.Methods)*

Gets HDR capabilities supported by the TV. The following values (OR-ed value) are possible:  
* 0 - HDRSTANDARD_NONE  
* 1 - HDRSTANDARD_HDR10  
* 2 - HDRSTANDARD_HLG  
* 4 - HDRSTANDARD_DolbyVision  
* 8 - HDRSTANDARD_TechnicolorPrime.
 
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.capabilities | integer | The OR-ed value of supported HDR standards by the TV |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getTVHDRCapabilities"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "capabilities": 3,
        "success": true
    }
}
```

<a name="method.getTvHDRSupport"></a>
## *getTvHDRSupport [<sup>method</sup>](#head.Methods)*

Returns an HDR support object (list of standards that the TV supports).
 
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.standards | array | A string [] of HDR capabilities. Possible values: `none`, `HDR10`, `Dolby Vision`, `Technicolor Prime` |
| result.standards[#] | string |  |
| result.supportsHDR | boolean | Indicates support for HDR |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getTvHDRSupport"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "standards": [
            "none"
        ],
        "supportsHDR": true,
        "success": true
    }
}
```

<a name="method.getVideoFormat"></a>
## *getVideoFormat [<sup>method</sup>](#head.Methods)*

Returns the current and supported video formats.
 
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.supportedVideoFormat | array | A list of supported Video formats |
| result.supportedVideoFormat[#] | string |  |
| result.currentVideoFormat | string | The current video format |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getVideoFormat"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "supportedVideoFormat": [
            "`SDR`, `HDR10`, `HLG`, `DV`, `Technicolor Prime`"
        ],
        "currentVideoFormat": "SDR",
        "success": true
    }
}
```

<a name="method.getVideoPortStatusInStandby"></a>
## *getVideoPortStatusInStandby [<sup>method</sup>](#head.Methods)*

Returns video port status in standby mode (failure if the port name is missing).
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.portName | string | Video port name |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.videoPortStatusInStandby | boolean | <sup>*(optional)*</sup> video port status (enabled/disabled) in standby mode in case of success |
| result?.error_message | string | <sup>*(optional)*</sup> Error message in case of failure |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getVideoPortStatusInStandby",
    "params": {
        "portName": "HDMI0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "videoPortStatusInStandby": true,
        "error_message": "Internal error",
        "success": true
    }
}
```

<a name="method.getVolumeLevel"></a>
## *getVolumeLevel [<sup>method</sup>](#head.Methods)*

Returns the current volume level.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.volumeLevel | number | Value between 0 and 100, where 0 means no level and 100 means maximum level |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getVolumeLevel",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "volumeLevel": 50,
        "success": true
    }
}
```

<a name="method.getVolumeLeveller"></a>
## *getVolumeLeveller [<sup>method</sup>](#head.Methods)*

(Version 2) Returns the current Volume Leveller setting.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.mode | integer | Enables or disables volume leveling (`0` = off, `1` = on, `2` = auto) |
| result.level | integer | Value between 0 and 10, where 0 means no level and 10 means maximum level |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getVolumeLeveller",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "mode": 1,
        "level": 9,
        "success": true
    }
}
```

<a name="method.getZoomSetting"></a>
## *getZoomSetting [<sup>method</sup>](#head.Methods)*

Returns the zoom setting value.
 
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.zoomSetting | string | Zoom setting. Possible values: `FULL`,  `NONE,`  `Letterbox 16x9`, `Letterbox 14x9`, `CCO`, `PanScan`, `Letterbox 2.21 on 4x3`, `Letterbox 2.21 on 16x9`, `Platform`, `Zoom 16x9`, `Pillarbox 4x3`, `Widescreen 4x3` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.getZoomSetting"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "zoomSetting": "FULL",
        "success": true
    }
}
```

<a name="method.isConnectedDeviceRepeater"></a>
## *isConnectedDeviceRepeater [<sup>method</sup>](#head.Methods)*

Indicates whether the device connected to the HDMI0 video output port is an HDCP repeater.
 
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.HdcpRepeater | boolean | `true` if the device is an HDCP repeater otherwise `false` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.isConnectedDeviceRepeater"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "HdcpRepeater": true,
        "success": true
    }
}
```

<a name="method.isSurroundDecoderEnabled"></a>
## *isSurroundDecoderEnabled [<sup>method</sup>](#head.Methods)*

Returns the current status of Surround Decoder.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.surroundDecoderEnable | boolean | <sup>*(optional)*</sup> Whether Surround Decoder is is enabled (`true`) or disabled (`false`) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.isSurroundDecoderEnabled",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "surroundDecoderEnable": true,
        "success": true
    }
}
```

<a name="method.readEDID"></a>
## *readEDID [<sup>method</sup>](#head.Methods)*

Reads the EDID from the connected HDMI (output) device. Returns a key of `EDID` with a value of the base64 encoded byte array string representing the EDID.
 
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.EDID | string | <sup>*(optional)*</sup> A base64 encoded byte array string representing the EDID |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.readEDID"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "EDID": "AP///////wAQrMLQVEJTMQUdAQOANR546q11qVRNnSYPUFSlSwCBALMA0QBxT6lAgYDRwAEBVl4AoKCgKVAwIDUADighAAAaAAAA/wBNWTNORDkxVjFTQlQKAAAA/ABERUxMIFAyNDE4RAogAAAA/QAxVh1xHAAKICAgICAgARsCAxuxUJAFBAMCBxYBBhESFRMUHyBlAwwAEAACOoAYcTgtQFgsRQAOKCEAAB4BHYAYcRwWIFgsJQAOKCEAAJ6/FgCggDgTQDAgOgAOKCEAABp+OQCggDgfQDAgOgAOKCEAABoAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA2A",
        "success": true
    }
}
```

<a name="method.readHostEDID"></a>
## *readHostEDID [<sup>method</sup>](#head.Methods)*

Reads the EDID of the host. Returns a key of `EDID` with a value of the base64 encoded raw byte array string representing the EDID.
 
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.EDID | string | <sup>*(optional)*</sup> A base64 encoded byte array string representing the EDID |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.readHostEDID"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "EDID": "AP///////wAQrMLQVEJTMQUdAQOANR546q11qVRNnSYPUFSlSwCBALMA0QBxT6lAgYDRwAEBVl4AoKCgKVAwIDUADighAAAaAAAA/wBNWTNORDkxVjFTQlQKAAAA/ABERUxMIFAyNDE4RAogAAAA/QAxVh1xHAAKICAgICAgARsCAxuxUJAFBAMCBxYBBhESFRMUHyBlAwwAEAACOoAYcTgtQFgsRQAOKCEAAB4BHYAYcRwWIFgsJQAOKCEAAJ6/FgCggDgTQDAgOgAOKCEAABp+OQCggDgfQDAgOgAOKCEAABoAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA2A",
        "success": true
    }
}
```

<a name="method.resetBassEnhancer"></a>
## *resetBassEnhancer [<sup>method</sup>](#head.Methods)*

Resets the dialog enhancer level to its default bassboost value.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.resetBassEnhancer",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.resetDialogEnhancement"></a>
## *resetDialogEnhancement [<sup>method</sup>](#head.Methods)*

Resets the dialog enhancer level to its default enhancer level.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.resetDialogEnhancement",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.resetSurroundVirtualizer"></a>
## *resetSurroundVirtualizer [<sup>method</sup>](#head.Methods)*

Resets the surround virtualizer to its default boost value.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.resetSurroundVirtualizer",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.resetVolumeLeveller"></a>
## *resetVolumeLeveller [<sup>method</sup>](#head.Methods)*

Resets the Volume Leveller level to default volume value.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.resetVolumeLeveller",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setAssociatedAudioMixing"></a>
## *setAssociatedAudioMixing [<sup>method</sup>](#head.Methods)*

Enables or disables associated audio mixing.  

### Event  
| Event | Description | 
| :----------- | :----------- |
| `associatedAudioMixingChanged`|Triggered when associated audio mixing is enabled or disabled|.

Also see: [associatedAudioMixingChanged](#event.associatedAudioMixingChanged)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name. For STB, the `HDMI0` port handle can be used. The default port is `HDMI0` if no port is specified. For TV products, unlike other MS12 APIs, an audio port is not required because the API is not specific to an audio port but is instead a platform/host configuration |
| params.mixing | boolean | Whether associated audio mixing is enabled (`true`) or disabled (`false`) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setAssociatedAudioMixing",
    "params": {
        "audioPort": "SPEAKER0",
        "mixing": true
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setAudioAtmosOutputMode"></a>
## *setAudioAtmosOutputMode [<sup>method</sup>](#head.Methods)*

Sets ATMOS audio output mode (on HDMI0).
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enable | boolean | enable or disable ATMOS audio output mode |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setAudioAtmosOutputMode",
    "params": {
        "enable": true
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setAudioDelay"></a>
## *setAudioDelay [<sup>method</sup>](#head.Methods)*

Sets the audio delay (in ms) on the selected audio port. If the `audioPort` argument is not specified, it will browse all ports (checking HDMI0 first). If there is no display connected, then it defaults to `HDMI0`.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.audioPort | string | Audio port name |
| params.audioDelay | string | Delay (in ms) on the selected audio port |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setAudioDelay",
    "params": {
        "audioPort": "HDMI0",
        "audioDelay": "0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setAudioDelayOffset"></a>
## *setAudioDelayOffset [<sup>method</sup>](#head.Methods)*

Sets the audio delay offset (in ms) on the selected audio port. If the `audioPort` argument is not specified, it will browse all ports (checking HDMI0 first). If there is no display connected, then it defaults to `HDMI0`.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.audioPort | string | Audio port name |
| params.audioDelayOffset | string | Delay offset (in ms) on the selected audio port |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setAudioDelayOffset",
    "params": {
        "audioPort": "HDMI0",
        "audioDelayOffset": "0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setBassEnhancer"></a>
## *setBassEnhancer [<sup>method</sup>](#head.Methods)*

Sets the Bass Enhancer. Bass Enhancer provides the consumer a single control to apply a fixed bass boost to correct for a lack of bass reproduction in the playback system.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |
| params.bassBoost | integer | Value between 0 and 100, where 0 means no bass boost (disabled) and 100 means max bass boost |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setBassEnhancer",
    "params": {
        "audioPort": "SPEAKER0",
        "bassBoost": 50
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setCurrentResolution"></a>
## *setCurrentResolution [<sup>method</sup>](#head.Methods)*

Sets the current resolution.
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `resolutionPreChange`| Triggered when the resolution of the video display is about to change.|
| `resolutionChanged`| Triggered when the resolution is changed by the user and returns the current resolution.|.

Also see: [resolutionPreChange](#event.resolutionPreChange), [resolutionChanged](#event.resolutionChanged)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.videoDisplay | string | Video display port name. The default port is `HDMI0` if no port is specified |
| params.resolution | string | Video display resolution |
| params?.persist | boolean | <sup>*(optional)*</sup> Persists the resolution |
| params?.ignoreEdid | boolean | <sup>*(optional)*</sup> Ignore the supported resolutions as transmitted by the connected TV EDID |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setCurrentResolution",
    "params": {
        "videoDisplay": "HDMI0",
        "resolution": "1080p",
        "persist": true,
        "ignoreEdid": true
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setDialogEnhancement"></a>
## *setDialogEnhancement [<sup>method</sup>](#head.Methods)*

Sets the Dialog Enhancer level.A dialog enhancer boosts the speech audio separately from other background content, without increasing the loudness.The method fails if no value is set.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |
| params.enhancerlevel | integer | Value between 0 and 16, where 0 means no enhancement and 16 means maximum enhancement |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setDialogEnhancement",
    "params": {
        "audioPort": "SPEAKER0",
        "enhancerlevel": 0
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setDolbyVolumeMode"></a>
## *setDolbyVolumeMode [<sup>method</sup>](#head.Methods)*

Enables or disables Dolby Volume mode on audio track (audio output port HDMI0).
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.dolbyVolumeMode | boolean | Whether Dolby Volume mode is enabled (`true`) or disabled (`false`) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setDolbyVolumeMode",
    "params": {
        "dolbyVolumeMode": true
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setDRCMode"></a>
## *setDRCMode [<sup>method</sup>](#head.Methods)*

Sets the Dynamic Range Control (DRC) setting. DRC is a compression control applied to audio to limit the dynamic range to suit a specific listening situation. For default settings, RF mode is preferred for two-channel outputs (television speaker or headphone) and Line mode for multichannel outputs.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |
| params.DRCMode | integer | Value of 0 or 1, where 0 is Line mode and 1 is RF mode |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setDRCMode",
    "params": {
        "audioPort": "SPEAKER0",
        "DRCMode": 1
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setEnableAudioPort"></a>
## *setEnableAudioPort [<sup>method</sup>](#head.Methods)*

Enable or disable the specified audio port based on the input audio port name. This feature provides the consumer with a single user control to enable or disable the specified audio port.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.audioPort | string | Audio port name |
| params.enable | boolean | `true` enables the specified audio port. `false` disables the specified audio port |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setEnableAudioPort",
    "params": {
        "audioPort": "HDMI0",
        "enable": true
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setFaderControl"></a>
## *setFaderControl [<sup>method</sup>](#head.Methods)*

Sets the audio mixer balance.  

### Event  
| Event | Description | 
| :----------- | :----------- |
| `faderControlChanged`|Triggered when the audio mixer balance is changed|.

Also see: [faderControlChanged](#event.faderControlChanged)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name. For STB, the `HDMI0` port handle can be used. The default port is `HDMI0` if no port is specified. For TV products, unlike other MS12 APIs, an audio port is not required because the API is not specific to an audio port but is instead a platform/host configuration |
| params.mixerBalance | integer | The audi mixer balance. The value should be between -32 to +32 |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setFaderControl",
    "params": {
        "audioPort": "SPEAKER0",
        "mixerBalance": 10
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setForceHDRMode"></a>
## *setForceHDRMode [<sup>method</sup>](#head.Methods)*

Enables or disables the force HDR mode. If enabled, the HDR format that is currently configured on the device is used.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.hdr_mode | boolean | <sup>*(optional)*</sup> `true` to force the HDR format or `false` to reset the mode to the default |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setForceHDRMode",
    "params": {
        "hdr_mode": true
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setGain"></a>
## *setGain [<sup>method</sup>](#head.Methods)*

Adjusts the gain on a specific port.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |
| params.gain | number | Value between 0 and 100, where 0 means no gain and 100 means maximum gain |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setGain",
    "params": {
        "audioPort": "SPEAKER0",
        "gain": 10.0
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setGraphicEqualizerMode"></a>
## *setGraphicEqualizerMode [<sup>method</sup>](#head.Methods)*

Sets the Graphic Equalizer Mode. The Graphic Equalizer is a multi-band equalizer that allows the end user to customize the sonic qualities of the system.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |
| params.graphicEqualizerMode | integer | Graphic Equalizer mode (`0` = off, `1` = open, `2` = rich, `3` = focused) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setGraphicEqualizerMode",
    "params": {
        "audioPort": "SPEAKER0",
        "graphicEqualizerMode": 2
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setIntelligentEqualizerMode"></a>
## *setIntelligentEqualizerMode [<sup>method</sup>](#head.Methods)*

Sets the Intelligent Equalizer Mode (port HDMI0). An Intelligent Equalizer continuously monitors the audio spectrum and adjusts its equalization filter to transform the original audio tone into desired tone.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |
| params.intelligentEqualizerMode | integer | Intelligent Equalizer mode (`0` = unset, `1` = open, `2` = rich, `3` = focused) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setIntelligentEqualizerMode",
    "params": {
        "audioPort": "SPEAKER0",
        "intelligentEqualizerMode": 2
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setMISteering"></a>
## *setMISteering [<sup>method</sup>](#head.Methods)*

Enables or Disables Media Intelligent Steering. Media Intelligence analyzes audio content and steers the Volume Leveler, the Dialogue Enhancer, the Intelligent Equalizer, and the Speaker Virtualizer, based on the type of audio content.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |
| params.MISteeringEnable | boolean | Whether Media Intelligence Steering is enabled (`true`) or disabled (`false`) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setMISteering",
    "params": {
        "audioPort": "SPEAKER0",
        "MISteeringEnable": true
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setMS12AudioCompression"></a>
## *setMS12AudioCompression [<sup>method</sup>](#head.Methods)*

Sets the audio dynamic range compression level (port HDMI0).
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |
| params.compresionLevel | integer | Value between 0 and 10, where 0 means no compression and 10 means maximum compression |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setMS12AudioCompression",
    "params": {
        "audioPort": "SPEAKER0",
        "compresionLevel": 5
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setMS12AudioProfile"></a>
## *setMS12AudioProfile [<sup>method</sup>](#head.Methods)*

Sets the selected MS12 audio profile.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |
| params.ms12AudioProfile | string | An MS12 audio profile name from `getSupportedMS12AudioProfile` |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setMS12AudioProfile",
    "params": {
        "audioPort": "SPEAKER0",
        "ms12AudioProfile": "Game"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setMS12ProfileSettingsOverride"></a>
## *setMS12ProfileSettingsOverride [<sup>method</sup>](#head.Methods)*

Overrides individual MS12 audio settings in order to optimize the customer experience (for example, enabling dialog enhancement in sports mode).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.audioPort | string | Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |
| params.operation | string | The audio profile state |
| params.profileName | string | An MS12 audio profile name from `getSupportedMS12AudioProfile` |
| params.ms12SettingsName | string | An ms12 setting name |
| params.ms12SettingsValue | string | The value to set |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setMS12ProfileSettingsOverride",
    "params": {
        "audioPort": "SPEAKER0",
        "operation": "...",
        "profileName": "Sports",
        "ms12SettingsName": "Dialog Enhance",
        "ms12SettingsValue": "On"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setMuted"></a>
## *setMuted [<sup>method</sup>](#head.Methods)*

Mutes or unmutes audio on a specific port.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |
| params.muted | boolean | mute or unmute audio |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setMuted",
    "params": {
        "audioPort": "SPEAKER0",
        "muted": true
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setPrimaryLanguage"></a>
## *setPrimaryLanguage [<sup>method</sup>](#head.Methods)*

Sets the primary language.  

### Event  
| Event | Description | 
| :----------- | :----------- |
| `primaryLanguageChanged`|Triggered when the primary language changes|.

Also see: [primaryLanguageChanged](#event.primaryLanguageChanged)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name. For STB, the `HDMI0` port handle can be used. The default port is `HDMI0` if no port is specified. For TV products, unlike other MS12 APIs, an audio port is not required because the API is not specific to an audio port but is instead a platform/host configuration |
| params.lang | string | The 3 letter language code as per ISO 639 |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setPrimaryLanguage",
    "params": {
        "audioPort": "SPEAKER0",
        "lang": "eng"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setScartParameter"></a>
## *setScartParameter [<sup>method</sup>](#head.Methods)*

Sets SCART parameters.  

Possible SCART values:  
| **Parameter** | **ParameterData** | 
| :----------- | :----------- |
| `aspect_ratio` | `4x3` or `16x9` |  
| `tv_startup` | `on` or `off` |  
| `rgb` | `on` (disables cvbs) |  
| `cvbs` | `on` (disables rgb) |  
| `macrovision` | not implemented |  
| `cgms` |  `disabled`, `copyNever`, `copyOnce`, `copyFreely`, or `copyNoMore` |  
| `port` | `on` or `off` | 

### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.scartParameter | string | SCART parameter name |
| params?.scartParameterData | string | <sup>*(optional)*</sup> SCART parameter data |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setScartParameter",
    "params": {
        "scartParameter": "aspect_ratio",
        "scartParameterData": "4x3"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setSecondaryLanguage"></a>
## *setSecondaryLanguage [<sup>method</sup>](#head.Methods)*

Sets the secondary language.  

### Event  
| Event | Description | 
| :----------- | :----------- |
| `secondaryLanguageChanged`|Triggered when the secondary language changes|.

Also see: [secondaryLanguageChanged](#event.secondaryLanguageChanged)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name. For STB, the `HDMI0` port handle can be used. The default port is `HDMI0` if no port is specified. For TV products, unlike other MS12 APIs, an audio port is not required because the API is not specific to an audio port but is instead a platform/host configuration |
| params.lang | string | The 3 letter language code as per ISO 639 |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setSecondaryLanguage",
    "params": {
        "audioPort": "SPEAKER0",
        "lang": "eng"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setSoundMode"></a>
## *setSoundMode [<sup>method</sup>](#head.Methods)*

Sets the current sound mode for the corresponding video display. If the `audioPort` argument value is missing or empty all ports are set.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.audioPort | string | Audio port name |
| params.soundMode | string | Sound mode. Possible values: `AUTO (Dolby Digital Plus)`, `AUTO (Dolby Digital 5.1)`, `AUTO (Stereo)`, `MONO`, `STEREO`, `SURROUND`, PASSTHRU |
| params?.persist | boolean | <sup>*(optional)*</sup> persists the sound mode |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setSoundMode",
    "params": {
        "audioPort": "HDMI0",
        "soundMode": "STEREO",
        "persist": true
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setSurroundVirtualizer"></a>
## *setSurroundVirtualizer [<sup>method</sup>](#head.Methods)*

(Version 2) Sets the Surround Virtualizer boost. The Speaker/Surround Virtualizer enables a surround sound signal (including one generated by the Surround Decoder) to be rendered over a device with built-in speakers or headphones.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |
| params.mode | integer | Enables or disables volume leveling (`0` = off, `1` = on, `2` = auto) |
| params.boost | integer | Value between 0 and 96, where 0 means no boost and 96 means maximum boost |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setSurroundVirtualizer",
    "params": {
        "audioPort": "SPEAKER0",
        "mode": 1,
        "boost": 90
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setVideoPortStatusInStandby"></a>
## *setVideoPortStatusInStandby [<sup>method</sup>](#head.Methods)*

Sets the specified video port status to be used in standby mode (failure if the port name is missing).
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.portName | string | Video port name |
| params.enabled | boolean | Enable video port status to be used in standby mode |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.error_message | string | <sup>*(optional)*</sup> Error message in case of failure |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setVideoPortStatusInStandby",
    "params": {
        "portName": "HDMI0",
        "enabled": true
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "error_message": "internal error",
        "success": true
    }
}
```

<a name="method.setVolumeLevel"></a>
## *setVolumeLevel [<sup>method</sup>](#head.Methods)*

Adjusts the Volume Level on a specific port.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |
| params.volumeLevel | number | Value between 0 and 100, where 0 means no level and 100 means maximum level |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setVolumeLevel",
    "params": {
        "audioPort": "SPEAKER0",
        "volumeLevel": 50
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setVolumeLeveller"></a>
## *setVolumeLeveller [<sup>method</sup>](#head.Methods)*

(Version 2) Sets the Volume Leveller level. Volume Leveller is an advanced volume-control solution that maintains consistent playback levels for content from different sources.
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |
| params.mode | integer | Enables or disables volume leveling (`0` = off, `1` = on, `2` = auto) |
| params.level | integer | Value between 0 and 10, where 0 means no level and 10 means maximum level |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setVolumeLeveller",
    "params": {
        "audioPort": "SPEAKER0",
        "mode": 1,
        "level": 9
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="method.setZoomSetting"></a>
## *setZoomSetting [<sup>method</sup>](#head.Methods)*

Sets the current zoom value.
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `zoomSettingsUpdated`| Triggered when the zoom setting changes and returns the zoom setting values for all video display types.|.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.zoomSetting | string | Zoom setting. Possible values: `FULL`,  `NONE,`  `Letterbox 16x9`, `Letterbox 14x9`, `CCO`, `PanScan`, `Letterbox 2.21 on 4x3`, `Letterbox 2.21 on 16x9`, `Platform`, `Zoom 16x9`, `Pillarbox 4x3`, `Widescreen 4x3` |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.1.setZoomSetting",
    "params": {
        "zoomSetting": "FULL"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.DisplaySettings plugin:

DisplaySettings interface events:

| Event | Description |
| :-------- | :-------- |
| [activeInputChanged](#event.activeInputChanged) | Triggered on active input change (RxSense) |
| [associatedAudioMixingChanged](#event.associatedAudioMixingChanged) | Triggered when associated audio mixing is enabled or disabled |
| [audioFormatChanged](#event.audioFormatChanged) | Triggered when the configured audio format changes |
| [connectedAudioPortUpdated](#event.connectedAudioPortUpdated) | Triggered when the connected audio port is updated |
| [connectedVideoDisplaysUpdated](#event.connectedVideoDisplaysUpdated) | Triggered when the connected video display is updated and returns the connected video displays |
| [faderControlChanged](#event.faderControlChanged) | Triggered when the audio mixer balance is changed |
| [primaryLanguageChanged](#event.primaryLanguageChanged) | Triggered when the primary language changes |
| [resolutionChanged](#event.resolutionChanged) | Triggered when the resolution is changed by the user and returns the current resolution |
| [resolutionPreChange](#event.resolutionPreChange) | Triggered on resolution pre-change |
| [secondaryLanguageChanged](#event.secondaryLanguageChanged) | Triggered when the secondary language changes |
| [zoomSettingUpdated](#event.zoomSettingUpdated) | Triggered when the zoom setting changes and returns the zoom setting values for all video display types |
| [videoFormatChanged](#event.videoFormatChanged) | Triggered when the video format of connected video port changes and returns the new video format along with other supported formats of that video port |


<a name="event.activeInputChanged"></a>
## *activeInputChanged [<sup>event</sup>](#head.Notifications)*

Triggered on active input change (RxSense).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.activeInput | boolean | `true` = RXSENSE_ON, `false` = RXSENSE_OFF |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.activeInputChanged",
    "params": {
        "activeInput": true
    }
}
```

<a name="event.associatedAudioMixingChanged"></a>
## *associatedAudioMixingChanged [<sup>event</sup>](#head.Notifications)*

Triggered when associated audio mixing is enabled or disabled.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.mixing | boolean | Whether associated audio mixing is enabled (`true`) or disabled (`false`) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.associatedAudioMixingChanged",
    "params": {
        "mixing": true
    }
}
```

<a name="event.audioFormatChanged"></a>
## *audioFormatChanged [<sup>event</sup>](#head.Notifications)*

Triggered when the configured audio format changes.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.supportedAudioFormat | array | A list of supported audio formats |
| params.supportedAudioFormat[#] | string |  |
| params.currentAudioFormat | string | The currently set audio format |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.audioFormatChanged",
    "params": {
        "supportedAudioFormat": [
            "`NONE`, `PCM`, `DOLBY AC3`, `DOLBY EAC3`, `DOLBY AC4`, `DOLBY MAT', 'DOLBY TRUEHD', 'DOLBY EAC3 ATMOS', 'DOLBY TRUEHD ATMOS', 'DOLBY MAT ATMOS', 'DOLBY AC4 ATMOS'"
        ],
        "currentAudioFormat": "PCM"
    }
}
```

<a name="event.connectedAudioPortUpdated"></a>
## *connectedAudioPortUpdated [<sup>event</sup>](#head.Notifications)*

Triggered when the connected audio port is updated.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.HotpluggedAudioPort | string | <sup>*(optional)*</sup> Audio port ID for which the connection status has changed. Possible audio port IDs are (`HDMI_ARC0`, `HEADPHONE0`,  etc) |
| params.isConnected | string | Current connection status of the audio port (must be one of the following: *connected*, *disconnected*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.connectedAudioPortUpdated",
    "params": {
        "HotpluggedAudioPort": "HDMI_ARC0",
        "isConnected": "connected"
    }
}
```

<a name="event.connectedVideoDisplaysUpdated"></a>
## *connectedVideoDisplaysUpdated [<sup>event</sup>](#head.Notifications)*

Triggered when the connected video display is updated and returns the connected video displays.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.connectedVideoDisplays | array | A string [] of connected video display port names |
| params.connectedVideoDisplays[#] | string |  |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.connectedVideoDisplaysUpdated",
    "params": {
        "connectedVideoDisplays": [
            "HDMI0"
        ]
    }
}
```

<a name="event.faderControlChanged"></a>
## *faderControlChanged [<sup>event</sup>](#head.Notifications)*

Triggered when the audio mixer balance is changed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.mixerBalance | integer | The audi mixer balance. The value should be between -32 to +32 |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.faderControlChanged",
    "params": {
        "mixerBalance": 10
    }
}
```

<a name="event.primaryLanguageChanged"></a>
## *primaryLanguageChanged [<sup>event</sup>](#head.Notifications)*

Triggered when the primary language changes.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.primaryLanguage | string | The primary language |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.primaryLanguageChanged",
    "params": {
        "primaryLanguage": "eng"
    }
}
```

<a name="event.resolutionChanged"></a>
## *resolutionChanged [<sup>event</sup>](#head.Notifications)*

Triggered when the resolution is changed by the user and returns the current resolution.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.width | integer | Width of the video display |
| params.height | integer | Height of the video display |
| params.videoDisplayType | string | Type of video display (port) |
| params.resolution | string | Video display resolution |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.resolutionChanged",
    "params": {
        "width": 1920,
        "height": 1080,
        "videoDisplayType": "HDMI0",
        "resolution": "1080p"
    }
}
```

<a name="event.resolutionPreChange"></a>
## *resolutionPreChange [<sup>event</sup>](#head.Notifications)*

Triggered on resolution pre-change.

### Parameters

This event carries no parameters.

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.resolutionPreChange"
}
```

<a name="event.secondaryLanguageChanged"></a>
## *secondaryLanguageChanged [<sup>event</sup>](#head.Notifications)*

Triggered when the secondary language changes.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.secondaryLanguage | string | The secondary language |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.secondaryLanguageChanged",
    "params": {
        "secondaryLanguage": "eng"
    }
}
```

<a name="event.zoomSettingUpdated"></a>
## *zoomSettingUpdated [<sup>event</sup>](#head.Notifications)*

Triggered when the zoom setting changes and returns the zoom setting values for all video display types.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.zoomSetting | string | Zoom setting. Possible values: `FULL`,  `NONE,`  `Letterbox 16x9`, `Letterbox 14x9`, `CCO`, `PanScan`, `Letterbox 2.21 on 4x3`, `Letterbox 2.21 on 16x9`, `Platform`, `Zoom 16x9`, `Pillarbox 4x3`, `Widescreen 4x3` |
| params.videoDisplayType | string | Type of video display (port) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.zoomSettingUpdated",
    "params": {
        "zoomSetting": "FULL",
        "videoDisplayType": "HDMI0"
    }
}
```

<a name="event.videoFormatChanged"></a>
## *videoFormatChanged [<sup>event</sup>](#head.Notifications)*

Triggered when the video format of connected video port changes and returns the new video format along with other supported formats of that video port.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.supportedVideoFormat | array | A list of supported Video formats |
| params.supportedVideoFormat[#] | string |  |
| params.currentVideoFormat | string | The current video format |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.videoFormatChanged",
    "params": {
        "supportedVideoFormat": [
            "`SDR`, `HDR10`, `HLG`, `DV`, `Technicolor Prime`"
        ],
        "currentVideoFormat": "SDR"
    }
}
```

