<!-- Generated automatically, DO NOT EDIT! -->
<a name="DisplaySettings_Plugin"></a>
# DisplaySettings Plugin

**Version: [1.4.5](https://github.com/rdkcentral/rdkservices/blob/main/DisplaySettings/CHANGELOG.md)**

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
| result.lang | string | 3 letter lang code should be used as per ISO 639 |
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

<a name="setSecondaryLanguage"></a>
## *setSecondaryLanguage*

Sets the secondary language.

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
    "method": "org.rdk.DisplaySettings.setSecondaryLanguage",
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

<a name="getSecondaryLanguage"></a>
## *getSecondaryLanguage*

Returns the Secondary language.

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
| result.lang | string | 3 letter lang code should be used as per ISO 639 |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getSecondaryLanguage",
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

<a name="getGain"></a>
## *getGain*

Returns the current gain value.

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
| result.gain | number | Value between -2080 and 480, where -2080 means negative gain and 480 means maximum gain |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getGain",
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

<a name="getGraphicEqualizerMode"></a>
## *getGraphicEqualizerMode*

Returns the current Graphic Equalizer Mode setting (port HDMI0).

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
| result.enable | boolean | `true` if Graphic Equalizer Mode is enabled, otherwise `false` |
| result.mode | integer | Graphic Equalizer mode (`0` = mode not set or in case of error, `1` = open, `2` = rich, `3` = focused) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getGraphicEqualizerMode",
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

<a name="getIntelligentEqualizerMode"></a>
## *getIntelligentEqualizerMode*

Returns the current Intelligent Equalizer Mode setting (port HDMI0).

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
| result.enable | boolean | `true` if Intelligent Equalizer Mode is enabled, otherwise `false` |
| result.mode | integer | Intelligent Equalizer mode (`0` = off, `1` = open, `2` = rich, `3` = focused) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getIntelligentEqualizerMode",
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

<a name="getMISteering"></a>
## *getMISteering*

Returns the current status of Media Intelligence Steering settings.

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
| result?.MISteeringEnable | boolean | <sup>*(optional)*</sup> Whether Media Intelligence Steering is enabled (`true`) or disabled (`false`) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getMISteering",
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

<a name="getMS12AudioCompression"></a>
## *getMS12AudioCompression*

Returns the current audio compression settings.

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
| result.enable | boolean | `true` if audio compression is enabled, otherwise `false` |
| result.compressionLevel | integer | Value between 0 and 10, where 0 means no compression and 10 means maximum compression |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getMS12AudioCompression",
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

<a name="getMS12AudioProfile"></a>
## *getMS12AudioProfile*

Returns the current MS12 audio profile settings.

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
| result.ms12AudioProfile | string | An MS12 audio profile name from `getSupportedMS12AudioProfile` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getMS12AudioProfile",
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

<a name="getMuted"></a>
## *getMuted*

Returns whether audio is muted on a given port.

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
| result.muted | boolean | mute or unmute audio |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getMuted",
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

<a name="getPreferredColorDepth"></a>
## *getPreferredColorDepth*

Returns the current color depth on the selected video display port.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.videoDisplay | string | <sup>*(optional)*</sup> Video display port name. The default port is `HDMI0` if no port is specified |
| params?.persist | boolean | <sup>*(optional)*</sup> Persists the color depth |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.resolution | string | <sup>*(optional)*</sup> Video display color depth. (must be one of the following: *8 Bit*, *10 Bit*, *12 Bit*, *Auto*) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getPreferredColorDepth",
    "params": {
        "videoDisplay": "HDMI0",
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
        "resolution": "12 Bit",
        "success": true
    }
}
```

<a name="getSettopAudioCapabilities"></a>
## *getSettopAudioCapabilities*

Returns the set-top audio capabilities for the specified audio port.

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
| result?.AudioCapabilities | array | <sup>*(optional)*</sup> An array of audio capabilities supported by STB |
| result?.AudioCapabilities[#] | string | <sup>*(optional)*</sup> Audio capability name (None in case of no audio capability support)  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getSettopAudioCapabilities",
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

<a name="getSettopHDRSupport"></a>
## *getSettopHDRSupport*

Returns an HDR support object (list of standards that the STB supports).

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.standards | array | A string [] of HDR capabilities. Possible values: `none`, `HDR10`, `HDR10PLUS`, `Dolby Vision`, `Technicolor Prime`, `SDR` |
| result.standards[#] | string |  |
| result.supportsHDR | boolean | Indicates support for HDR |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getSettopHDRSupport"
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

<a name="getSettopMS12Capabilities"></a>
## *getSettopMS12Capabilities*

Returns the set-top MS12 audio capabilities for the specified audio port.

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
| result.ms12Capabilities | array | A string [] of MS12 audio capabilities |
| result.ms12Capabilities[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getSettopMS12Capabilities",
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

<a name="getSinkAtmosCapability"></a>
## *getSinkAtmosCapability*

Returns the ATMOS capability of the sink (HDMI0).

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
| result?.atmos_capability | integer | <sup>*(optional)*</sup> ATMOS Capability (`0` = off, `1` = open, `2` = rich, `3` = focused) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getSinkAtmosCapability",
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
        "atmos_capability": 2,
        "success": true
    }
}
```

<a name="getSoundMode"></a>
## *getSoundMode*

Returns the sound mode for the incoming video display. If the argument is `Null` or empty (although not recommended), this returns the output mode of all connected ports, whichever is connected, while giving priority to the HDMI port. If the video display is not connected, then it returns `Stereo` as a safe default.

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
| result.soundMode | string | Sound mode. Possible values: `AUTO (Dolby Digital Plus)`, `AUTO (Dolby Digital 5.1)`, `AUTO (Stereo)`, `MONO`, `STEREO`, `SURROUND`, PASSTHRU |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getSoundMode",
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

<a name="getSupportedAudioModes"></a>
## *getSupportedAudioModes*

Returns a list of strings containing the supported audio modes. If `Null` or empty, this returns the supported audio modes of the audio processor (regardless of the the output port).  
If a port name is specified, this returns the audio output modes supported by the connected sink device (EDID based). If the port is not connected, the return value is same as if `Null` is specified as the parameter.  
For **Auto** mode in DS5, this API has the following extra specification:  
* For HDMI port, this API returns `Stereo` mode, `Dolby Digital 5.1` mode and `Auto` mode; 
* For SPDIF and HDMI ARC port, this API always returns `Surround` mode, `Stereo` mode, and `PASSTHRU` Mode;  
* When `AUTO` mode is returned, it includes in parenthesis the best sound mode that the STB can output and the connected sink device can support, in the format of `AUTO` _(`Best Format`)_. For example, if the connected device supports surround, the auto mode string will be `AUTO (Dolby Digital 5.1)`.

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
| result.supportedAudioModes | array | A string [] of supported audio modes |
| result.supportedAudioModes[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getSupportedAudioModes",
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

<a name="getSupportedAudioPorts"></a>
## *getSupportedAudioPorts*

Returns all audio ports supported on the device (all ports that are physically present).

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.getSupportedAudioPorts"
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

<a name="getSupportedMS12AudioProfiles"></a>
## *getSupportedMS12AudioProfiles*

Returns list of platform supported MS12 audio profiles for the specified audio port.

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
| result.supportedMS12AudioProfiles | array | A string [] of MS12 audio profiles |
| result.supportedMS12AudioProfiles[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getSupportedMS12AudioProfiles",
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

<a name="getSupportedResolutions"></a>
## *getSupportedResolutions*

Returns supported resolutions on the selected video display port (both TV and STB) by its name.

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
| result.supportedResolutions | array | A string array of supported resolutions on the selected video display port |
| result.supportedResolutions[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getSupportedResolutions",
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

<a name="getSupportedSettopResolutions"></a>
## *getSupportedSettopResolutions*

Returns supported STB resolutions.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.getSupportedSettopResolutions"
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

<a name="getSupportedTvResolutions"></a>
## *getSupportedTvResolutions*

Returns supported TV resolutions on the selected video display port.

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
| result.supportedTvResolutions | array | A string [] of supported TV resolutions |
| result.supportedTvResolutions[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getSupportedTvResolutions",
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

<a name="getSupportedVideoDisplays"></a>
## *getSupportedVideoDisplays*

Returns all video ports supported on the device (all ports that are physically present).

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.getSupportedVideoDisplays"
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

<a name="getSurroundVirtualizer"></a>
## *getSurroundVirtualizer*

(Version 2) Returns the current surround virtualizer boost settings.

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
| result.mode | integer | Enables or disables volume leveling (`0` = off, `1` = on, `2` = auto) |
| result.boost | integer | Value between 0 and 96, where 0 means no boost and 96 means maximum boost |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getSurroundVirtualizer",
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

<a name="getTVHDRCapabilities"></a>
## *getTVHDRCapabilities*

Gets HDR capabilities supported by the TV. The following values (OR-ed value) are possible:  
* 0x0 - HDRSTANDARD_NONE  
* 0x1 - HDRSTANDARD_HDR10  
* 0x2 - HDRSTANDARD_HLG  
* 0x4 - HDRSTANDARD_DolbyVision  
* 0x8 - HDRSTANDARD_TechnicolorPrime. 
* 0x10 - HDRSTANDARD_HDR10PLUS  
* 0x20 - HDRSTANDARD_SDR.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.getTVHDRCapabilities"
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

<a name="getTvHDRSupport"></a>
## *getTvHDRSupport*

Returns an HDR support object (list of standards that the TV supports).

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.standards | array | A string [] of HDR capabilities. Possible values: `none`, `HDR10`, `HDR10PLUS`, `Dolby Vision`, `Technicolor Prime`, `SDR` |
| result.standards[#] | string |  |
| result.supportsHDR | boolean | Indicates support for HDR |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getTvHDRSupport"
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

<a name="getVideoFormat"></a>
## *getVideoFormat*

Returns the current and supported video formats.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.getVideoFormat"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "supportedVideoFormat": [
            "`SDR`, `HDR10`, `HDR10PLUS`, `HLG`, `DV`, `Technicolor Prime`"
        ],
        "currentVideoFormat": "SDR",
        "success": true
    }
}
```

<a name="getVideoPortStatusInStandby"></a>
## *getVideoPortStatusInStandby*

Returns video port status in standby mode (failure if the port name is missing).

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.getVideoPortStatusInStandby",
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

<a name="getVolumeLevel"></a>
## *getVolumeLevel*

Returns the current volume level.

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
| result.volumeLevel | number | Value between 0 and 100, where 0 means no level and 100 means maximum level |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getVolumeLevel",
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

<a name="getVolumeLeveller"></a>
## *getVolumeLeveller*

(Version 2) Returns the current Volume Leveller setting.

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
| result.mode | integer | Enables or disables volume leveling (`0` = off, `1` = on, `2` = auto) |
| result.level | integer | Value between 0 and 10, where 0 means no level and 10 means maximum level |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getVolumeLeveller",
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

<a name="getZoomSetting"></a>
## *getZoomSetting*

Returns the zoom setting value.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.getZoomSetting"
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

<a name="isConnectedDeviceRepeater"></a>
## *isConnectedDeviceRepeater*

Indicates whether the device connected to the HDMI0 video output port is an HDCP repeater.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.isConnectedDeviceRepeater"
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

<a name="isSurroundDecoderEnabled"></a>
## *isSurroundDecoderEnabled*

Returns the current status of Surround Decoder.

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
| result?.surroundDecoderEnable | boolean | <sup>*(optional)*</sup> Whether Surround Decoder is is enabled (`true`) or disabled (`false`) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.isSurroundDecoderEnabled",
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

<a name="readEDID"></a>
## *readEDID*

Reads the EDID from the connected HDMI (output) device. Returns a key of `EDID` with a value of the base64 encoded byte array string representing the EDID.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.readEDID"
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

<a name="readHostEDID"></a>
## *readHostEDID*

Reads the EDID of the host. Returns a key of `EDID` with a value of the base64 encoded raw byte array string representing the EDID.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.readHostEDID"
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

<a name="resetBassEnhancer"></a>
## *resetBassEnhancer*

Resets the dialog enhancer level to its default bassboost value.

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
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.resetBassEnhancer",
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

<a name="resetDialogEnhancement"></a>
## *resetDialogEnhancement*

Resets the dialog enhancer level to its default enhancer level.

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
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.resetDialogEnhancement",
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

<a name="resetSurroundVirtualizer"></a>
## *resetSurroundVirtualizer*

Resets the surround virtualizer to its default boost value.

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
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.resetSurroundVirtualizer",
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

<a name="resetVolumeLeveller"></a>
## *resetVolumeLeveller*

Resets the Volume Leveller level to default volume value.

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
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.resetVolumeLeveller",
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

<a name="setAudioAtmosOutputMode"></a>
## *setAudioAtmosOutputMode*

Sets ATMOS audio output mode (on HDMI0).

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.setAudioAtmosOutputMode",
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

<a name="setAudioDelay"></a>
## *setAudioDelay*

Sets the audio delay (in ms) on the selected audio port. If the `audioPort` argument is not specified, it will browse all ports (checking HDMI0 first). If there is no display connected, then it defaults to `HDMI0`.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.setAudioDelay",
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

<a name="setBassEnhancer"></a>
## *setBassEnhancer*

Sets the Bass Enhancer. Bass Enhancer provides the consumer a single control to apply a fixed bass boost to correct for a lack of bass reproduction in the playback system.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.setBassEnhancer",
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

<a name="setCurrentResolution"></a>
## *setCurrentResolution*

Sets the current resolution.

### Events

| Event | Description |
| :-------- | :-------- |
| [resolutionPreChange](#resolutionPreChange) | Triggered when the resolution of the video display is about to change. |
| [resolutionChanged](#resolutionChanged) | Triggered when the resolution is changed by the user and returns the current resolution. |
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
    "method": "org.rdk.DisplaySettings.setCurrentResolution",
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

<a name="setDialogEnhancement"></a>
## *setDialogEnhancement*

Sets the Dialog Enhancer level.A dialog enhancer boosts the speech audio separately from other background content, without increasing the loudness.The method fails if no value is set.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.setDialogEnhancement",
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

<a name="setDolbyVolumeMode"></a>
## *setDolbyVolumeMode*

Enables or disables Dolby Volume mode on audio track (audio output port HDMI0).

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.setDolbyVolumeMode",
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

<a name="setDRCMode"></a>
## *setDRCMode*

Sets the Dynamic Range Control (DRC) setting. DRC is a compression control applied to audio to limit the dynamic range to suit a specific listening situation. For default settings, RF mode is preferred for two-channel outputs (television speaker or headphone) and Line mode for multichannel outputs.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.setDRCMode",
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

<a name="setEnableAudioPort"></a>
## *setEnableAudioPort*

Enable or disable the specified audio port based on the input audio port name. This feature provides the consumer with a single user control to enable or disable the specified audio port.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.setEnableAudioPort",
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

<a name="setForceHDRMode"></a>
## *setForceHDRMode*

Enables or disables the force HDR mode. If enabled, the HDR format that is currently configured on the device is used.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.setForceHDRMode",
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

<a name="setGain"></a>
## *setGain*

Adjusts the gain on a specific port.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name (`HDMI0`, `SPEAKER0`, `SPDIF0`, and so on). The default port is `HDMI0` if no port is specified |
| params.gain | number | Value between -2080 and 480, where -2080 means negative gain and 480 means maximum gain |

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
    "method": "org.rdk.DisplaySettings.setGain",
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

<a name="setGraphicEqualizerMode"></a>
## *setGraphicEqualizerMode*

Sets the Graphic Equalizer Mode. The Graphic Equalizer is a multi-band equalizer that allows the end user to customize the sonic qualities of the system.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.setGraphicEqualizerMode",
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

<a name="setIntelligentEqualizerMode"></a>
## *setIntelligentEqualizerMode*

Sets the Intelligent Equalizer Mode (port HDMI0). An Intelligent Equalizer continuously monitors the audio spectrum and adjusts its equalization filter to transform the original audio tone into desired tone.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.setIntelligentEqualizerMode",
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

<a name="setMISteering"></a>
## *setMISteering*

Enables or Disables Media Intelligent Steering. Media Intelligence analyzes audio content and steers the Volume Leveler, the Dialogue Enhancer, the Intelligent Equalizer, and the Speaker Virtualizer, based on the type of audio content.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.setMISteering",
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

<a name="setMS12AudioCompression"></a>
## *setMS12AudioCompression*

Sets the audio dynamic range compression level (port HDMI0).

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.setMS12AudioCompression",
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

<a name="setMS12AudioProfile"></a>
## *setMS12AudioProfile*

Sets the selected MS12 audio profile.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.setMS12AudioProfile",
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

<a name="setMS12ProfileSettingsOverride"></a>
## *setMS12ProfileSettingsOverride*

Overrides individual MS12 audio settings in order to optimize the customer experience (for example, enabling dialog enhancement in sports mode).

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.setMS12ProfileSettingsOverride",
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

<a name="setMuted"></a>
## *setMuted*

Mutes or unmutes audio on a specific port.

### Events

| Event | Description |
| :-------- | :-------- |
| [muteStatusChanged](#muteStatusChanged) | Triggered when the mute status changed. |
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
    "method": "org.rdk.DisplaySettings.setMuted",
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

<a name="setPreferredColorDepth"></a>
## *setPreferredColorDepth*

Sets the current color depth for the videoDisplay.
.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.videoDisplay | string | Video display port name. The default port is `HDMI0` if no port is specified |
| params.colorDepth | string | Video display color depth. (must be one of the following: *8 Bit*, *10 Bit*, *12 Bit*, *Auto*) |
| params?.persist | boolean | <sup>*(optional)*</sup> Persists the color depth |

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
    "method": "org.rdk.DisplaySettings.setPreferredColorDepth",
    "params": {
        "videoDisplay": "HDMI0",
        "colorDepth": "12 Bit",
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

<a name="setScartParameter"></a>
## *setScartParameter*

Sets SCART parameters.  
   
Possible values:  
| **Parameter** | **ParameterData** |  
| `aspect_ratio` | `4x3` or `16x9` |  
| `tv_startup` | `on` or `off` |  
| `rgb` | `on` (disables cvbs) |  
| `cvbs` | `on` (disables rgb) |  
| `macrovision` | not implemented |  
| `cgms` |  `disabled`, `copyNever`, `copyOnce`, `copyFreely`, or `copyNoMore` |  
| `port` | `on` or `off` |.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.setScartParameter",
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

<a name="setSoundMode"></a>
## *setSoundMode*

Sets the current sound mode for the corresponding video display. If the `audioPort` argument value is missing or empty all ports are set.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.setSoundMode",
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

<a name="setSurroundVirtualizer"></a>
## *setSurroundVirtualizer*

(Version 2) Sets the Surround Virtualizer boost. The Speaker/Surround Virtualizer enables a surround sound signal (including one generated by the Surround Decoder) to be rendered over a device with built-in speakers or headphones.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.setSurroundVirtualizer",
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

<a name="setVideoPortStatusInStandby"></a>
## *setVideoPortStatusInStandby*

Sets the specified video port status to be used in standby mode (failure if the port name is missing).

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.setVideoPortStatusInStandby",
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

<a name="setVolumeLevel"></a>
## *setVolumeLevel*

Adjusts the Volume Level on a specific port.

### Events

| Event | Description |
| :-------- | :-------- |
| [volumeLevelChanged](#volumeLevelChanged) | Triggered when the volume level changed. |
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
    "method": "org.rdk.DisplaySettings.setVolumeLevel",
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

<a name="setVolumeLeveller"></a>
## *setVolumeLeveller*

(Version 2) Sets the Volume Leveller level. Volume Leveller is an advanced volume-control solution that maintains consistent playback levels for content from different sources.

### Events

No Events

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
    "method": "org.rdk.DisplaySettings.setVolumeLeveller",
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

<a name="setZoomSetting"></a>
## *setZoomSetting*

Sets the current zoom value.

### Events

| Event | Description |
| :-------- | :-------- |
| [zoomSettingUpdated](#zoomSettingUpdated) | Triggered when the zoom setting changes and returns the zoom setting values for all video display types. |
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
    "method": "org.rdk.DisplaySettings.setZoomSetting",
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

<a name="getColorDepthCapabilities"></a>
## *getColorDepthCapabilities*

Returns supported color depth capabilities.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.getColorDepthCapabilities | array | <sup>*(optional)*</sup> A string array of supported STB color depth capabilities |
| result?.getColorDepthCapabilities[#] | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getColorDepthCapabilities"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "getColorDepthCapabilities": [
            "8 Bit"
        ],
        "success": true
    }
}
```

<a name="getSupportedMS12Config"></a>
## *getSupportedMS12Config*

Returns supported ms12 config by the platform, possible values couldbe CONFG_Z, CONFIG_X, CONFIG_Y, CONFIG_NONE.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.ms12config | string | Supported ms12 config by platforms, possible values, CONFIG_Z, CONFIG_X, CONFIG_Y, CONFIG_NONE |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DisplaySettings.getSupportedMS12Config"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "ms12config": "CONFIG_X",
        "success": true
    }
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.DisplaySettings plugin:

DisplaySettings interface events:

| Event | Description |
| :-------- | :-------- |
| [activeInputChanged](#activeInputChanged) | Triggered on active input change (RxSense) |
| [audioFormatChanged](#audioFormatChanged) | Triggered when the configured audio format changes |
| [connectedAudioPortUpdated](#connectedAudioPortUpdated) | Triggered when the connected audio port is updated |
| [connectedVideoDisplaysUpdated](#connectedVideoDisplaysUpdated) | Triggered when the connected video display is updated and returns the connected video displays |
| [resolutionChanged](#resolutionChanged) | Triggered when the resolution is changed by the user and returns the current resolution |
| [resolutionPreChange](#resolutionPreChange) | Triggered on resolution pre-change |
| [zoomSettingUpdated](#zoomSettingUpdated) | Triggered when the zoom setting changes and returns the zoom setting values for all video display types |
| [videoFormatChanged](#videoFormatChanged) | Triggered when the video format of connected video port changes and returns the new video format along with other supported formats of that video port |
| [AtmosCapabilityChanged](#AtmosCapabilityChanged) | Triggered when the audio sink device Atmos capability is changed |
| [muteStatusChanged](#muteStatusChanged) | Triggered when the mute status changed |
| [volumeLevelChanged](#volumeLevelChanged) | Triggered when the volume level changed |


<a name="activeInputChanged"></a>
## *activeInputChanged*

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
    "method": "client.events.activeInputChanged",
    "params": {
        "activeInput": true
    }
}
```

<a name="audioFormatChanged"></a>
## *audioFormatChanged*

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
    "method": "client.events.audioFormatChanged",
    "params": {
        "supportedAudioFormat": [
            "`NONE`, `PCM`, `DOLBY AC3`, `DOLBY EAC3`, `DOLBY AC4`, `DOLBY MAT', 'DOLBY TRUEHD', 'DOLBY EAC3 ATMOS', 'DOLBY TRUEHD ATMOS', 'DOLBY MAT ATMOS', 'DOLBY AC4 ATMOS'"
        ],
        "currentAudioFormat": "PCM"
    }
}
```

<a name="connectedAudioPortUpdated"></a>
## *connectedAudioPortUpdated*

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
    "method": "client.events.connectedAudioPortUpdated",
    "params": {
        "HotpluggedAudioPort": "HDMI_ARC0",
        "isConnected": "connected"
    }
}
```

<a name="connectedVideoDisplaysUpdated"></a>
## *connectedVideoDisplaysUpdated*

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
    "method": "client.events.connectedVideoDisplaysUpdated",
    "params": {
        "connectedVideoDisplays": [
            "HDMI0"
        ]
    }
}
```

<a name="resolutionChanged"></a>
## *resolutionChanged*

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
    "method": "client.events.resolutionChanged",
    "params": {
        "width": 1920,
        "height": 1080,
        "videoDisplayType": "HDMI0",
        "resolution": "1080p"
    }
}
```

<a name="resolutionPreChange"></a>
## *resolutionPreChange*

Triggered on resolution pre-change.

### Parameters

This event carries no parameters.

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.resolutionPreChange"
}
```

<a name="zoomSettingUpdated"></a>
## *zoomSettingUpdated*

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
    "method": "client.events.zoomSettingUpdated",
    "params": {
        "zoomSetting": "FULL",
        "videoDisplayType": "HDMI0"
    }
}
```

<a name="videoFormatChanged"></a>
## *videoFormatChanged*

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
    "method": "client.events.videoFormatChanged",
    "params": {
        "supportedVideoFormat": [
            "`SDR`, `HDR10`, `HDR10PLUS`, `HLG`, `DV`, `Technicolor Prime`"
        ],
        "currentVideoFormat": "SDR"
    }
}
```

<a name="AtmosCapabilityChanged"></a>
## *AtmosCapabilityChanged*

Triggered when the audio sink device Atmos capability is changed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.currentAtmosCapability | string | Atmos capability of the connected device, ATMOS_SUPPORTED, ATMOS_NOT_SUPPORTED |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.AtmosCapabilityChanged",
    "params": {
        "currentAtmosCapability": "ATMOS_SUPPORTED"
    }
}
```

<a name="muteStatusChanged"></a>
## *muteStatusChanged*

Triggered when the mute status changed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.muted | boolean | mute status value |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.muteStatusChanged",
    "params": {
        "muted": true
    }
}
```

<a name="volumeLevelChanged"></a>
## *volumeLevelChanged*

Triggered when the volume level changed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.volumeLevel | integer | volume level |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.volumeLevelChanged",
    "params": {
        "volumeLevel": 10
    }
}
```

