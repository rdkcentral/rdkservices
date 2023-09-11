<!-- Generated automatically, DO NOT EDIT! -->
<a name="DisplaySettings_Plugin"></a>
# DisplaySettings Plugin

**Version: [1.3.4](https://github.com/rdkcentral/rdkservices/blob/main/DisplaySettings/CHANGELOG.md)**

A org.rdk.DisplaySettings plugin for Thunder framework.

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

The `DisplaySetting` plugin provides an interface for display information such as current video resolution, supported video displays, zoom setting, sound mode, and much more.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.DisplaySettings*) |
| classname | string | Class name: *org.rdk.DisplaySettings* |
| locator | string | Library name: *libWPEFrameworkDisplaySettings.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.DisplaySettings plugin:

DisplaySettings interface methods:

| Method | Description |
| :-------- | :-------- |
| [enableSurroundDecoder](#enableSurroundDecoder) | Enables or disables Surround Decoder capability |
| [getActiveInput](#getActiveInput) | Returns `true` if the STB HDMI output is currently connected to the active input of the sink device (determined by `RxSense`) |
| [getAudioDelay](#getAudioDelay) | Returns the audio delay (in ms) on the selected audio port |
| [getAudioDelayOffset](#getAudioDelayOffset) | Returns the audio delay offset (in ms) on the selected audio port |
| [getAudioFormat](#getAudioFormat) | Returns the currently set audio format |
| [getBassEnhancer](#getBassEnhancer) | Returns the current status of the Bass Enhancer settings |
| [getConnectedAudioPorts](#getConnectedAudioPorts) | Returns connected audio output ports (a subset of the ports supported on the device) |
| [getConnectedVideoDisplays](#getConnectedVideoDisplays) | Returns connected video displays |
| [getCurrentOutputSettings](#getCurrentOutputSettings) | Returns current output settings |
| [getCurrentResolution](#getCurrentResolution) | Returns the current resolution on the selected video display port |
| [getDefaultResolution](#getDefaultResolution) | Gets the default resolution supported by the HDMI0 video output port |
| [getDialogEnhancement](#getDialogEnhancement) | Returns the current Dialog Enhancer level (port HDMI0) |
| [getDolbyVolumeMode](#getDolbyVolumeMode) | Returns whether Dolby Volume mode is enabled or disabled (audio output port HDMI0) |
| [getDRCMode](#getDRCMode) | Returns the current Dynamic Range Control mode |
| [getEnableAudioPort](#getEnableAudioPort) |  Returns the current status of the specified input audio port |
| [setAssociatedAudioMixing](#setAssociatedAudioMixing) | Sets the Associated Audio Mixing Enable/Disable |
| [getAssociatedAudioMixing](#getAssociatedAudioMixing) | Returns the Associated Audio Mixing status |
| [setFaderControl](#setFaderControl) | Sets the set the mixerbalance betweeen main and associated audio |
| [getFaderControl](#getFaderControl) | Returns the mixerbalance betweeen main and associated audio |
| [setPrimaryLanguage](#setPrimaryLanguage) | Sets the Primary language |
| [getPrimaryLanguage](#getPrimaryLanguage) | Returns the Primary language |
| [setSecondaryLanguage](#setSecondaryLanguage) | Sets the secondary language |
| [getSecondaryLanguage](#getSecondaryLanguage) | Returns the Secondary language |
| [getGain](#getGain) | Returns the current gain value |
| [getGraphicEqualizerMode](#getGraphicEqualizerMode) | Returns the current Graphic Equalizer Mode setting (port HDMI0) |
| [getIntelligentEqualizerMode](#getIntelligentEqualizerMode) | Returns the current Intelligent Equalizer Mode setting (port HDMI0) |
| [getMISteering](#getMISteering) | Returns the current status of Media Intelligence Steering settings |
| [getMS12AudioCompression](#getMS12AudioCompression) | Returns the current audio compression settings |
| [getMS12AudioProfile](#getMS12AudioProfile) | Returns the current MS12 audio profile settings |
| [getMuted](#getMuted) | Returns whether audio is muted on a given port |
| [getPreferredColorDepth](#getPreferredColorDepth) | Returns the current color depth on the selected video display port |
| [getSettopAudioCapabilities](#getSettopAudioCapabilities) | Returns the set-top audio capabilities for the specified audio port |
| [getSettopHDRSupport](#getSettopHDRSupport) | Returns an HDR support object (list of standards that the STB supports) |
| [getSettopMS12Capabilities](#getSettopMS12Capabilities) | Returns the set-top MS12 audio capabilities for the specified audio port |
| [getSinkAtmosCapability](#getSinkAtmosCapability) | Returns the ATMOS capability of the sink (HDMI0) |
| [getSoundMode](#getSoundMode) | Returns the sound mode for the incoming video display |
| [getSupportedAudioModes](#getSupportedAudioModes) | Returns a list of strings containing the supported audio modes |
| [getSupportedAudioPorts](#getSupportedAudioPorts) | Returns all audio ports supported on the device (all ports that are physically present) |
| [getSupportedMS12AudioProfiles](#getSupportedMS12AudioProfiles) | Returns list of platform supported MS12 audio profiles for the specified audio port |
| [getSupportedResolutions](#getSupportedResolutions) | Returns supported resolutions on the selected video display port (both TV and STB) by its name |
| [getSupportedSettopResolutions](#getSupportedSettopResolutions) | Returns supported STB resolutions |
| [getSupportedTvResolutions](#getSupportedTvResolutions) | Returns supported TV resolutions on the selected video display port |
| [getSupportedVideoDisplays](#getSupportedVideoDisplays) | Returns all video ports supported on the device (all ports that are physically present) |
| [getSurroundVirtualizer](#getSurroundVirtualizer) | (Version 2) Returns the current surround virtualizer boost settings |
| [getTVHDRCapabilities](#getTVHDRCapabilities) | Gets HDR capabilities supported by the TV |
| [getTvHDRSupport](#getTvHDRSupport) | Returns an HDR support object (list of standards that the TV supports) |
| [getVideoFormat](#getVideoFormat) | Returns the current and supported video formats |
| [getVideoPortStatusInStandby](#getVideoPortStatusInStandby) | Returns video port status in standby mode (failure if the port name is missing) |
| [getVolumeLevel](#getVolumeLevel) | Returns the current volume level |
| [getVolumeLeveller](#getVolumeLeveller) | (Version 2) Returns the current Volume Leveller setting |
| [getZoomSetting](#getZoomSetting) | Returns the zoom setting value |
| [isConnectedDeviceRepeater](#isConnectedDeviceRepeater) | Indicates whether the device connected to the HDMI0 video output port is an HDCP repeater |
| [isSurroundDecoderEnabled](#isSurroundDecoderEnabled) | Returns the current status of Surround Decoder |
| [readEDID](#readEDID) | Reads the EDID from the connected HDMI (output) device |
| [readHostEDID](#readHostEDID) | Reads the EDID of the host |
| [resetBassEnhancer](#resetBassEnhancer) | Resets the dialog enhancer level to its default bassboost value |
| [resetDialogEnhancement](#resetDialogEnhancement) | Resets the dialog enhancer level to its default enhancer level |
| [resetSurroundVirtualizer](#resetSurroundVirtualizer) | Resets the surround virtualizer to its default boost value |
| [resetVolumeLeveller](#resetVolumeLeveller) | Resets the Volume Leveller level to default volume value |
| [setAudioAtmosOutputMode](#setAudioAtmosOutputMode) | Sets ATMOS audio output mode (on HDMI0) |
| [setAudioDelay](#setAudioDelay) | Sets the audio delay (in ms) on the selected audio port |
| [setAudioDelayOffset](#setAudioDelayOffset) | Sets the audio delay offset (in ms) on the selected audio port |
| [setBassEnhancer](#setBassEnhancer) | Sets the Bass Enhancer |
| [setCurrentResolution](#setCurrentResolution) | Sets the current resolution |
| [setDialogEnhancement](#setDialogEnhancement) | Sets the Dialog Enhancer level |
| [setDolbyVolumeMode](#setDolbyVolumeMode) | Enables or disables Dolby Volume mode on audio track (audio output port HDMI0) |
| [setDRCMode](#setDRCMode) | Sets the Dynamic Range Control (DRC) setting |
| [setEnableAudioPort](#setEnableAudioPort) | Enable or disable the specified audio port based on the input audio port name |
| [setForceHDRMode](#setForceHDRMode) | Enables or disables the force HDR mode |
| [setGain](#setGain) | Adjusts the gain on a specific port |
| [setGraphicEqualizerMode](#setGraphicEqualizerMode) | Sets the Graphic Equalizer Mode |
| [setIntelligentEqualizerMode](#setIntelligentEqualizerMode) | Sets the Intelligent Equalizer Mode (port HDMI0) |
| [setMISteering](#setMISteering) | Enables or Disables Media Intelligent Steering |
| [setMS12AudioCompression](#setMS12AudioCompression) | Sets the audio dynamic range compression level (port HDMI0) |
| [setMS12AudioProfile](#setMS12AudioProfile) | Sets the selected MS12 audio profile |
| [setMS12ProfileSettingsOverride](#setMS12ProfileSettingsOverride) | Overrides individual MS12 audio settings in order to optimize the customer experience (for example, enabling dialog enhancement in sports mode) |
| [setMuted](#setMuted) | Mutes or unmutes audio on a specific port |
| [setPreferredColorDepth](#setPreferredColorDepth) | Sets the current color depth for the videoDisplay |
| [setScartParameter](#setScartParameter) | Sets SCART parameters |
| [setSoundMode](#setSoundMode) | Sets the current sound mode for the corresponding video display |
| [setSurroundVirtualizer](#setSurroundVirtualizer) | (Version 2) Sets the Surround Virtualizer boost |
| [setVideoPortStatusInStandby](#setVideoPortStatusInStandby) | Sets the specified video port status to be used in standby mode (failure if the port name is missing) |
| [setVolumeLevel](#setVolumeLevel) | Adjusts the Volume Level on a specific port |
| [setVolumeLeveller](#setVolumeLeveller) | (Version 2) Sets the Volume Leveller level |
| [setZoomSetting](#setZoomSetting) | Sets the current zoom value |
| [getColorDepthCapabilities](#getColorDepthCapabilities) | Returns supported color depth capabilities |
| [getSupportedMS12Config](#getSupportedMS12Config) | Returns supported ms12 config by the platform, possible values couldbe CONFG_Z, CONFIG_X, CONFIG_Y, CONFIG_NONE |


<a name="enableSurroundDecoder"></a>
## *enableSurroundDecoder*

Enables or disables Surround Decoder capability. The Surround Decoder is an upmixer that takes stereo music content, or surround-encoded two-channel movie content, and creates a high-quality multichannel upmix. If the Surround Decoder is enabled, two-channel signals and 5.1-channel signals are upmixed to 5.1.2.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.enableSurroundDecoder",
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

<a name="getActiveInput"></a>
## *getActiveInput*

Returns `true` if the STB HDMI output is currently connected to the active input of the sink device (determined by `RxSense`).

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.getActiveInput",
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

<a name="getAudioDelay"></a>
## *getAudioDelay*

Returns the audio delay (in ms) on the selected audio port. If the `audioPort` argument is not specified, it will browse all ports (checking HDMI0 first). If there is no display connected, then it defaults to `HDMI0`.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.getAudioDelay",
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

<a name="getAudioDelayOffset"></a>
## *getAudioDelayOffset*

Returns the audio delay offset (in ms) on the selected audio port. If the `audioPort` argument is not specified, it will browse all ports (checking HDMI0 first). If there is no display connected, then it defaults to `HDMI0`.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.getAudioDelayOffset",
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

<a name="getAudioFormat"></a>
## *getAudioFormat*

Returns the currently set audio format.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.getAudioFormat"
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

<a name="getBassEnhancer"></a>
## *getBassEnhancer*

Returns the current status of the Bass Enhancer settings.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.getBassEnhancer",
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

<a name="getConnectedAudioPorts"></a>
## *getConnectedAudioPorts*

Returns connected audio output ports (a subset of the ports supported on the device). For SPDIF supported platforms, SPDIF port is always considered connected. HDMI port may or may not be connected.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.getConnectedAudioPorts"
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

<a name="getConnectedVideoDisplays"></a>
## *getConnectedVideoDisplays*

Returns connected video displays.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.getConnectedVideoDisplays"
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

<a name="getCurrentOutputSettings"></a>
## *getCurrentOutputSettings*

Returns current output settings.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.colorSpace | integer | The color space. Possible values: `0` (dsDISPLAY_COLORSPACE_UNKNOWN), `1` (sDISPLAY_COLORSPACE_RGB, `2` (dsDISPLAY_COLORSPACE_YCbCr422), `3` (dsDISPLAY_COLORSPACE_YCbCr444), `4` (dsDISPLAY_COLORSPACE_YCbCr420), `5` (dsDISPLAY_COLORSPACE_AUTO) |
| result.colorDepth | integer | The color depth. The value that is returned from `dsGetCurrentOutputSettings` |
| result.matrixCoefficients | integer | matrix coefficients. Possible values: `0` (dsDISPLAY_MATRIXCOEFFICIENT_UNKNOWN), `1` (dsDISPLAY_MATRIXCOEFFICIENT_BT_709), `2` (dsDISPLAY_MATRIXCOEFFICIENT_BT_470_2_BG), `3` (dsDISPLAY_MATRIXCOEFFICIENT_SMPTE_170M), `4` (dsDISPLAY_MATRIXCOEFFICIENT_XvYCC_709), `5` (dsDISPLAY_MATRIXCOEFFICIENT_eXvYCC_601), `6` (dsDISPLAY_MATRIXCOEFFICIENT_BT_2020_NCL), `7` (dsDISPLAY_MATRIXCOEFFICIENT_BT_2020_CL) |
| result.videoEOTF | integer | HDR standard. Possible values: `0x0` (dsHDRSTANDARD_NONE), `0x01` (dsHDRSTANDARD_HDR10), `0x02` (dsHDRSTANDARD_HLG), `0x04` (dsHDRSTANDARD_DolbyVision), `0x08` (dsHDRSTANDARD_TechnicolorPrime), `0x10` (dsHDRSTANDARD_HDR10PLUS), `0x20` (dsHDRSTANDARD_SDR), `0x80` (dsHDRSTANDARD_Invalid) |
| result?.quantizationRange | integer | <sup>*(optional)*</sup> The supported quantization range |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getCurrentOutputSettings"
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

<a name="getCurrentResolution"></a>
## *getCurrentResolution*

Returns the current resolution on the selected video display port.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.videoDisplay | string | <sup>*(optional)*</sup> Video display port name. The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.resolution | string | Video display resolution |
| result.w | number | The width |
| result.h | number | The height |
| result?.progressive | boolean | <sup>*(optional)*</sup> The type of scan signal |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getCurrentResolution",
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
        "w": 1920,
        "h": 1080,
        "progressive": true,
        "success": true
    }
}
```

<a name="getDefaultResolution"></a>
## *getDefaultResolution*

Gets the default resolution supported by the HDMI0 video output port.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.getDefaultResolution"
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

<a name="getDialogEnhancement"></a>
## *getDialogEnhancement*

Returns the current Dialog Enhancer level (port HDMI0).

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.getDialogEnhancement",
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

<a name="getDolbyVolumeMode"></a>
## *getDolbyVolumeMode*

Returns whether Dolby Volume mode is enabled or disabled (audio output port HDMI0).

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.getDolbyVolumeMode"
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

<a name="getDRCMode"></a>
## *getDRCMode*

Returns the current Dynamic Range Control mode.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.getDRCMode",
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

<a name="getEnableAudioPort"></a>
## *getEnableAudioPort*

 Returns the current status of the specified input audio port.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.getEnableAudioPort",
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

<a name="setAssociatedAudioMixing"></a>
## *setAssociatedAudioMixing*

Sets the Associated Audio Mixing Enable/Disable.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |
| params.mixing | boolean | `true` enables the Associated Audio Mixing for specified audio port. `false` to disables |

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
    "method": "org.rdk.DisplaySettings.setAssociatedAudioMixing",
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

<a name="getAssociatedAudioMixing"></a>
## *getAssociatedAudioMixing*

Returns the Associated Audio Mixing status.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.mixing | boolean | `true` if Associated Audio Mixing enabled for the specified audio port, otherwise `false` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getAssociatedAudioMixing",
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

<a name="setFaderControl"></a>
## *setFaderControl*

Sets the set the mixerbalance betweeen main and associated audio.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |
| params.mixerBalance | integer | Value between -32 to +32, where -32 means mute associated and +32 means mute main |

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
    "method": "org.rdk.DisplaySettings.setFaderControl",
    "params": {
        "audioPort": "SPEAKER0",
        "mixerBalance": 31
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

<a name="getFaderControl"></a>
## *getFaderControl*

Returns the mixerbalance betweeen main and associated audio.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.mixerBalance | integer | Value between -32 to +32, where -32 means mute associated and +32 means mute main |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getFaderControl",
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
        "mixerBalance": 31,
        "success": true
    }
}
```

<a name="setPrimaryLanguage"></a>
## *setPrimaryLanguage*

Sets the Primary language.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |
| params.lang | string | 3 letter lang code should be used as per ISO 639 |

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
    "method": "org.rdk.DisplaySettings.setPrimaryLanguage",
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

<a name="getPrimaryLanguage"></a>
## *getPrimaryLanguage*

Returns the Primary language.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.lang | integer | 3 letter lang code should be used as per ISO 639 |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getPrimaryLanguage",
    "params": {
        "audioPort": "SPEAKER0"
    }
}
```

#### Response

