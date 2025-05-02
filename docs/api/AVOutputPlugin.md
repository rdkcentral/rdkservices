<!-- Generated automatically, DO NOT EDIT! -->
<a name="AVOutput_Plugin"></a>
# AVOutput Plugin

**Version: [1.1.0](https://github.com/rdkcentral/rdkservices/blob/main/AVOutput/CHANGELOG.md)**

A org.rdk.AVOutput plugin for Thunder framework.

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

The AVOutput plugin lets you control the picture properties. It provides flexibility to modify picture properties per picture mode, source and format to the application. The `AVOutput` plugin replaces `ControlSettings` plugin.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.AVOutput*) |
| classname | string | Class name: *org.rdk.AVOutput* |
| locator | string | Library name: *libWPEFrameworkAVOutput.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.AVOutput plugin:

AVOutput interface methods:

| Method | Description |
| :-------- | :-------- |
| [getBacklight](#getBacklight) | Gets the TV user backlight level for a given picture mode, source and format |
| [getBacklightCaps](#getBacklightCaps) | Gets the capability and boundary range for TV user backlight |
| [getBrightness](#getBrightness) | Gets the TV user brightness level for a given picture mode, source and format |
| [getBrightnessCaps](#getBrightnessCaps) | Gets the capability and boundary range for TV brightness |
| [getContrast](#getContrast) | Gets the TV user contrast level for a given picture mode, source and format |
| [getContrastCaps](#getContrastCaps) | Gets the capability and boundary range for TV user contrast |
| [getSharpness](#getSharpness) | Gets the TV user sharpness level for a given picture mode, source and format |
| [getSharpnessCaps](#getSharpnessCaps) | Gets the capability and boundary range for TV user sharpness |
| [getSaturation](#getSaturation) | Gets the TV user saturation level for a given picture mode, source and format |
| [getSaturationCaps](#getSaturationCaps) | Gets the capability and boundary range for TV user saturation |
| [getHue](#getHue) | Gets the TV hue level for a given picture mode, source and format |
| [getHueCaps](#getHueCaps) | Gets the capability and boundary range for TV hue |
| [getColorTemperature](#getColorTemperature) | Gets the TV color temperature level for a given picture mode, source and format |
| [getColorTemperatureCaps](#getColorTemperatureCaps) | Gets the capability and boundary range for TV color temperature |
| [getCMS](#getCMS) | Gets the TV CMS(Color Management System) color component level for a given picture mode, source and format for the specified color and component |
| [getCMSCaps](#getCMSCaps) | Gets the capability and boundary range for TV CMS(Color Management System) |
| [getLowLatencyState](#getLowLatencyState) | Gets the TV low latency State for a given picture mode, source and format |
| [getLowLatencyStateCaps](#getLowLatencyStateCaps) | Gets the capability and boundary range for low latency state |
| [getZoomMode](#getZoomMode) | Gets the current TV zoom mode |
| [getZoomModeCaps](#getZoomModeCaps) | Gets the capability for TV zoom mode |
| [getBacklightDimmingMode](#getBacklightDimmingMode) | Gets the TV backlight dimming mode for a given picture mode, source and format |
| [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps) | Gets the capabilities and boundary range for TV backlight dimming mode |
| [getDolbyVisionMode](#getDolbyVisionMode) | Gets the DolbyVision mode for a given picture mode and video source |
| [getDolbyVisionModeCaps](#getDolbyVisionModeCaps) | Gets the capability for DolbyVision mode |
| [getHDRMode](#getHDRMode) | Gets the HDR10 mode for a given picture mode, video format and video source |
| [getHDRModeCaps](#getHDRModeCaps) | Gets the capability for HDR mode |
| [getVideoFormatCaps](#getVideoFormatCaps) | Get all supported video formats for the platform |
| [getVideoSourceCaps](#getVideoSourceCaps) | Get all supported video sources for the platform |
| [getVideoFrameRateCaps](#getVideoFrameRateCaps) | Get all supported video frame rates for the platform |
| [getVideoResolutionCaps](#getVideoResolutionCaps) | Get all supported video resolutions for the platform |
| [getPictureMode](#getPictureMode) | Get picture mode for a given video source and video format |
| [getPictureModeCaps](#getPictureModeCaps) | Get the list of picture modes supported and the list of video sources and video formats against which a picture mode can be associated with |
| [getVideoFormat](#getVideoFormat) | Returns the video format of the currently played video content |
| [getVideoSource](#getVideoSource) | Returns the video source selected |
| [getVideoFrameRate](#getVideoFrameRate) | Returns the video frame rate of the video being played |
| [getVideoContentType](#getVideoContentType) | Returns the FilmMaker mode status if it is enabled or disabled and in which sources the FilmMaker mode is active |
| [getVideoResolution](#getVideoResolution) | Returns the video resolution of the video being played |
| [getAutoBackLightMode](#getAutoBackLightMode) | Gets the TV auto backlight mode for a given picture mode, source and format |
| [getAutoBacklightModeCaps](#getAutoBacklightModeCaps) | Gets the capabilities and boundary range for TV auto backlight mode |
| [getFadeDisplayCaps](#getFadeDisplayCaps) | Gets the capabilities and boundary range for fading the display |
| [setPictureMode](#setPictureMode) | Set picture mode for a given video source and video format |
| [setBacklight](#setBacklight) | Sets the TV user backlight level for a given picture mode, video format and video source |
| [setBrightness](#setBrightness) | Sets the TV brightness level for a given picture mode, video format and video source |
| [setContrast](#setContrast) | Sets the TV contrast level for a given picture mode, video format and video source |
| [setSharpness](#setSharpness) | Sets the TV sharpness level for a given picture mode, video format and video source |
| [setSaturation](#setSaturation) | Sets the TV saturation level for a given picture mode, video format and video source |
| [setHue](#setHue) | Sets the TV hue level for a given picture mode, video format and video source |
| [setColorTemperature](#setColorTemperature) | Sets the TV color temperature level for a given picture mode, video format and video source |
| [setCMS](#setCMS) | Sets the TV CMS(Color Management System) for a given picture mode, video format and video source for the specified color and component |
| [setLowLatencyState](#setLowLatencyState) | Sets the low latency state for a given picture mode, video format and video source |
| [setZoomMode](#setZoomMode) | Sets the TV zoom mode for a given picture mode, video format and video source |
| [setBacklightDimmingMode](#setBacklightDimmingMode) | Sets the TV backlight dimming mode for a given picture mode, video format and video source |
| [fadeDisplay](#fadeDisplay) | Fades the backlight between two different backlight percentage values specified in a given duration in ms |
| [setDolbyVisionMode](#setDolbyVisionMode) | Sets the DolbyVision mode for a given picture mode and video format |
| [setHDRMode](#setHDRMode) | Sets the HDR mode for a given picture mode, video source and video format |
| [setAutoBacklightMode](#setAutoBacklightMode) | Sets the TV auto backlight mode for a given picture mode, video format and video source |
| [resetPictureMode](#resetPictureMode) | Reset picture mode to default for a given video source and video format |
| [resetBacklight](#resetBacklight) | Resets the TV user Backlight to factory default for a given picture mode, video format and video source |
| [resetBrightness](#resetBrightness) | Resets the TV brightness to factory default for a given picture mode, video format and video source |
| [resetContrast](#resetContrast) | Resets the TV contrast to factory default for a given picture mode, video format and video source |
| [resetSharpness](#resetSharpness) | Resets the TV sharpness to factory default for a given picture mode, video format and video source |
| [resetSaturation](#resetSaturation) | Resets the TV saturation to factory default for a given picture mode, video format and video source |
| [resetHue](#resetHue) | Resets the TV hue to factory default for a given picture mode, video format and video source |
| [resetColorTemperature](#resetColorTemperature) | Resets the TV color temperature to factory default for a given picture mode, video format and video source |
| [resetCMS](#resetCMS) | Resets the Color Management System (CMS) to factory defaults for a given picture mode, video format and video source for the given component and color values |
| [resetLowLatencyState](#resetLowLatencyState) | Resets the low latency state to factory default for a given picture mode, video format and video source |
| [resetZoomMode](#resetZoomMode) | Resets the TV zoom mode to the factory default |
| [resetBacklightDimmingMode](#resetBacklightDimmingMode) | Resets the TV backlight dimming mode to factory default for a given picture mode, video format and video source |
| [resetDolbyVisionMode](#resetDolbyVisionMode) | Resets the DolbyVision mode to factory default for a given picture mode and video format |
| [resetHDRMode](#resetHDRMode) | Resets the HDR mode to factory default for a given picture mode, video source and video format |
| [resetAutoBacklightMode](#resetAutoBacklightMode) | Resets the TV auto backlight mode to factory default for a given picture mode, video format and video source |
| [setWBMode](#setWBMode) | This function will reconfigure the PQ pipeline for white balance calibration by disabling Local Dimming, Local Contrast, and Dynamic Contrast |
| [getWBMode](#getWBMode) | Get the white balance mode |
| [get2PointWB](#get2PointWB) | Gets the TV 2 point white balance control value for a given picture mode, video format, video source and color temperature |
| [get2PointWBCaps](#get2PointWBCaps) | Gets the capabilities and boundary range for TV 2 point white balance |
| [set2PointWB](#set2PointWB) | Sets the TV 2 point white balance for a specified videoSource, colorTemperature, color and control value |
| [reset2PointWB](#reset2PointWB) | Resets the TV 2 point white balance to factory default for a given picture mode, video format and video source |


<a name="getBacklight"></a>
## *getBacklight*

Gets the TV user backlight level for a given picture mode, source and format. Regardless of whether auto backlight control is set to ambient or manual getBacklight always return the manual user set backlight level. This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | string | <sup>*(optional)*</sup> The picture mode that the backlight level should be fetched for. e.g. "Current", "Standard", "Sports", "EnergySaving". "Current" - the current selected picture mode. Other valid values for a platform can be obtained from options field of [getPictureModeCaps](#getPictureModeCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoSource | string | <sup>*(optional)*</sup> The video source that the backlight level should be fetched for. e.g. "Current", "Composite1", "HDMI1", "HDMI2", "HDMI3", "IP", "Tuner". "Current" - the current primary active source. When there is no active video playback or input port selected, "Current" refers to "IP" video source as default. Other valid values for a platform can be obtained from options field of [getVideoSourceCaps](#getVideoSourceCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoFormat | string | <sup>*(optional)*</sup> The video format that the backlight level should be fetched for. e.g. "Current", "SDR", "HDR10", "HLG", "DV". "Current" - the current format of the current primary active source. When there is no active video playback or input port selected, "Current" refers to "SDR" video format as default. Other valid values for a platform can be obtained from options field of [getVideoFormatCaps](#getVideoFormatCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.backlight | number | The backlight level in the range from - to inclusive |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getBacklight",
    "params": {
        "pictureMode": "Custom",
        "videoSource": "Current",
        "videoFormat": "SDR"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "backlight": 50,
        "success": true
    }
}
```

<a name="getBacklightCaps"></a>
## *getBacklightCaps*

Gets the capability and boundary range for TV user backlight. It returns the range of TV user backlight values that the user can set and the list of possible picture modes, video sources and video formats that the TV user backlight may be customised for.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.platformSupport | boolean | true: The platform support backlight. false:  The platform does not support backlight |
| result.rangeInfo | object | Range of values for the TV user backlight |
| result.rangeInfo.from | number | The lower-inclusive range value for result.backlight |
| result.rangeInfo.to | number | The higher-inclusive range value for result.backlight |
| result?.pictureModeInfo | array | <sup>*(optional)*</sup> Array of picture modes that the backlight could be customised for. These are few examples of picture modes available in this array: "Standard", "Vivid", "Custom", "Movie", "Sports"]. This API will return the entire list of picture modes returned by [getPictureModeCaps](#getPictureModeCaps) or a subset of it.<br><br>If backlight cannot be customised for picture modes, then this property is not returned |
| result?.pictureModeInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoSourceInfo | array | <sup>*(optional)*</sup> Array of video sources that the backlight could be customised for. These are few examples of video sources available in this array: ["Composite1", "HDMI1", "HDMI2", "HDMI3", "IP" ].This API will return the entire list of video sources returned by [getVideoSourceCaps](#getVideoSourceCaps), or a subset of it. If backlight cannot be customised for video sources, then this property is not returned |
| result?.videoSourceInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoFormatInfo | array | <sup>*(optional)*</sup> Array of video formats that the backlight could be customised for. These are few examples of formats available in this array: ["SDR","HLG","HDR10","DV"]. This API will return the entire list of video formats returned by [getVideoFormatCaps](#getVideoFormatCaps), or a subset of it. If backlight cannot be customised for video format, then this property is not returned |
| result?.videoFormatInfo[#] | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getBacklightCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "platformSupport": true,
        "rangeInfo": {
            "from": 0,
            "to": 100
        },
        "pictureModeInfo": [
            "`Custom`, `Movie`, `Game`, `Sports`, `Standard`"
        ],
        "videoSourceInfo": [
            "`HDMI1`,`HDMI2`, `HDMI3`, `IP`"
        ],
        "videoFormatInfo": [
            "`SDR`,`HLG`,`HDR10`,`DV`"
        ],
        "success": true
    }
}
```

<a name="getBrightness"></a>
## *getBrightness*

Gets the TV user brightness level for a given picture mode, source and format. This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | string | <sup>*(optional)*</sup> The picture mode that the brightness level should be fetched for. e.g. "Current", "Standard", "Sports", "EnergySaving". "Current" - the current selected picture mode. Other valid values for a platform can be obtained from options field of [getPictureModeCaps](#getPictureModeCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoSource | string | <sup>*(optional)*</sup> The video source that the brightness level should be fetched for. e.g. "Current", "Composite1", "HDMI1", "HDMI2", "HDMI3", "IP", "Tuner". "Current" - the current primary active source. When there is no active video playback or input port selected, "Current" refers to "IP" video source as default. Other valid values for a platform can be obtained from options field of [getVideoSourceCaps](#getVideoSourceCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoFormat | string | <sup>*(optional)*</sup> The picture format that the brightness level should be fetched for. e.g. "Current", "SDR", "HDR10", "HLG", "DV". "Current" - the current format of the current primary active source. When there is no active video playback or input port selected, "Current" refers to "SDR" video format as default. Other valid values for a platform can be obtained from options field of [getVideoFormatCaps](#getVideoFormatCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.brightness | number | The brightness level in the range from - to inclusive |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getBrightness",
    "params": {
        "pictureMode": "Custom",
        "videoSource": "Current",
        "videoFormat": "SDR"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "brightness": 50,
        "success": true
    }
}
```

<a name="getBrightnessCaps"></a>
## *getBrightnessCaps*

Gets the capability and boundary range for TV brightness. It returns the range of TV brightness values that the user can set and the list of possible picture modes, video sources and video formats that the TV brightness may be customised for.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.rangeInfo | object | Range of values for the TV user brightness |
| result.rangeInfo.from | number | The lower-inclusive range value for result.brightness |
| result.rangeInfo.to | number | The higher-inclusive range value for result.brightness |
| result?.pictureModeInfo | array | <sup>*(optional)*</sup> Array of picture modes that the brightness could be customised for. These are few examples of picture modes available in this array: ["Standard", "Vivid", "Custom", "Movie", "Sports"]. This API will return the entire list of picture modes returned by [getPictureModeCaps](#getPictureModeCaps) or a subset of it.<br><br>If brightness cannot be customised for picture modes, then this property is not returned |
| result?.pictureModeInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoSourceInfo | array | <sup>*(optional)*</sup> Array of video sources that the brightness could be customised for. These are few examples of video sources available in this array: ["Composite1", "HDMI1", "HDMI2", "HDMI3", "IP" ]. This API will return the entire list of video sources returned by [getVideoSourceCaps](#getVideoSourceCaps), or a subset of it.<br><br>If brightness cannot be customised for video sources, then this property is not returned |
| result?.videoSourceInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoFormatInfo | array | <sup>*(optional)*</sup> Array of video formats that the brightness could be customised for. These are few examples of formats available in this array: ["SDR","HLG","HDR10","DV"].This API will return the entire list of video formats returned by [getVideoFormatCaps](#getVideoFormatCaps), or a subset of it.<br><br>If brightness cannot be customised for video format, then this property is not returned |
| result?.videoFormatInfo[#] | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getBrightnessCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "rangeInfo": {
            "from": 0,
            "to": 100
        },
        "pictureModeInfo": [
            "`Custom`, `Movie`, `Game`, `Sports`, `Standard`"
        ],
        "videoSourceInfo": [
            "`HDMI1`,`HDMI2`, `HDMI3`, `IP`"
        ],
        "videoFormatInfo": [
            "`SDR`,`HLG`,`HDR10`,`DV`"
        ],
        "success": true
    }
}
```

<a name="getContrast"></a>
## *getContrast*

Gets the TV user contrast level for a given picture mode, source and format. This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | string | <sup>*(optional)*</sup> The picture mode that the contrast level should be fetched for. e.g. "Current", "Standard", "Sports", "EnergySaving". "Current" - the current selected picture mode. Other valid values for a platform can be obtained from options field of [getPictureModeCaps](#getPictureModeCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoSource | string | <sup>*(optional)*</sup> The video source that the contrast level should be fetched for. e.g. "Current", "Composite1", "HDMI1", "HDMI2", "HDMI3", "IP", "Tuner". "Current" - the current primary active source. When there is no active video playback or input port selected, "Current" refers to "IP" video source as default. Other valid values for a platform can be obtained from options field of [getVideoSourceCaps](#getVideoSourceCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoFormat | string | <sup>*(optional)*</sup> The video format that the contrast level should be fetched for. e.g. "Current", "SDR", "HDR10", "HLG", "DV". "Current" - the current format of the current primary active source. When there is no active video playback or input port selected, "Current" refers to "SDR" video format as default. Other valid values for a platform can be obtained from options field of [getVideoFormatCaps](#getVideoFormatCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.contrast | number | The contrast level in the range from - to inclusive |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getContrast",
    "params": {
        "pictureMode": "Custom",
        "videoSource": "Current",
        "videoFormat": "SDR"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "contrast": 50,
        "success": true
    }
}
```

<a name="getContrastCaps"></a>
## *getContrastCaps*

Gets the capability and boundary range for TV user contrast. It returns the range of TV user contrast values that the user can set and the list of possible picture modes, video sources and video formats that the TV user contrast may be customised for.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.rangeInfo | object | Range of values for the TV user contrast |
| result.rangeInfo.from | number | The lower-inclusive range value for result.contrast |
| result.rangeInfo.to | number | The higher-inclusive range value for result.contrast |
| result?.pictureModeInfo | array | <sup>*(optional)*</sup> Array of picture modes that the contrast could be customised for. These are few examples of picture modes available in this array: ["Standard", "Vivid", "Custom", "Movie", "Sports"]. This API will return the entire list of picture modes returned by [getPictureModeCaps](#getPictureModeCaps) or a subset of it.<br><br>If contrast cannot be customised for picture modes, then this property is not returned |
| result?.pictureModeInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoSourceInfo | array | <sup>*(optional)*</sup> Array of video sources that the contrast could be customised for. These are few examples of video sources available in this array: ["Composite1", "HDMI1", "HDMI2", "HDMI3", "IP" ]. This API will return the entire list of video sources returned by [getVideoSourceCaps](#getVideoSourceCaps), or a subset of it.<br><br>If contrast cannot be customised for video sources, then this property is not returned |
| result?.videoSourceInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoFormatInfo | array | <sup>*(optional)*</sup> Array of video formats that the contrast could be customised for. These are few examples of formats available in this array: ["SDR","HLG","HDR10","DV"].This API will return the entire list of video formats returned by [getVideoFormatCaps](#getVideoFormatCaps), or a subset of it.<br><br>If contrast cannot be customised for video format, then this property is not returned |
| result?.videoFormatInfo[#] | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getContrastCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "rangeInfo": {
            "from": 0,
            "to": 100
        },
        "pictureModeInfo": [
            "`Custom`, `Movie`, `Game`, `Sports`, `Standard`"
        ],
        "videoSourceInfo": [
            "`HDMI1`,`HDMI2`, `HDMI3`, `IP`"
        ],
        "videoFormatInfo": [
            "`SDR`,`HLG`,`HDR10`,`DV`"
        ],
        "success": true
    }
}
```

<a name="getSharpness"></a>
## *getSharpness*

Gets the TV user sharpness level for a given picture mode, source and format. This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | string | <sup>*(optional)*</sup> The picture mode that the sharpness level should be fetched for. e.g. "Current", "Standard", "Sports", "EnergySaving". "Current" - the current selected picture mode. Other valid values for a platform can be obtained from options field of [getPictureModeCaps](#getPictureModeCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoSource | string | <sup>*(optional)*</sup> The video source that the sharpness level should be fetched for. e.g. "Current", "Composite1", "HDMI1", "HDMI2", "HDMI3", "IP", "Tuner". "Current" - the current primary active source. When there is no active video playback or input port selected, "Current" refers to "IP" video source as default. Other valid values for a platform can be obtained from options field of [getVideoSourceCaps](#getVideoSourceCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoFormat | string | <sup>*(optional)*</sup> The video format that the sharpness level should be fetched for. e.g. "Current", "SDR", "HDR10", "HLG", "DV". "Current" - the current format of the current primary active source. When there is no active video playback or input port selected, "Current" refers to "SDR" video format as default. Other valid values for a platform can be obtained from options field of [getVideoFormatCaps](#getVideoFormatCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.sharpness | number | The sharpness level in the range from - to inclusive |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getSharpness",
    "params": {
        "pictureMode": "Custom",
        "videoSource": "Current",
        "videoFormat": "SDR"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "sharpness": 50,
        "success": true
    }
}
```

<a name="getSharpnessCaps"></a>
## *getSharpnessCaps*

Gets the capability and boundary range for TV user sharpness. It returns the range of TV user sharpness values that the user can set and the list of possible picture modes, video sources and video formats that the TV user sharpness may be customised for.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.rangeInfo | object | Range of values for the TV user sharpness |
| result.rangeInfo.from | number | The lower-inclusive range value for result.sharpness |
| result.rangeInfo.to | number | The higher-inclusive range value for result.sharpness |
| result?.pictureModeInfo | array | <sup>*(optional)*</sup> Array of picture modes that the sharpness could be customised for. These are few examples of picture modes available in this array: ["Standard", "Vivid", "Custom", "Movie", "Sports"]. This API will return the entire list of picture modes returned by [getPictureModeCaps](#getPictureModeCaps) or a subset of it.<br><br>If sharpness cannot be customised for picture modes, then this property is not returned |
| result?.pictureModeInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoSourceInfo | array | <sup>*(optional)*</sup> Array of video sources that the sharpness could be customised for. These are few examples of video sources available in this array: ["Composite1", "HDMI1", "HDMI2", "HDMI3", "IP" ]. This API will return the entire list of video sources returned by [getVideoSourceCaps](#getVideoSourceCaps), or a subset of it.<br><br>If sharpness cannot be customised for video sources, then this property is not returned |
| result?.videoSourceInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoFormatInfo | array | <sup>*(optional)*</sup> Array of video formats that the sharpness could be customised for. These are few examples of formats available in this array: ["SDR","HLG","HDR10","DV"].This API will return the entire list of video formats returned by [getVideoFormatCaps](#getVideoFormatCaps), or a subset of it.<br><br>If sharpness cannot be customised for video format, then this property is not returned |
| result?.videoFormatInfo[#] | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getSharpnessCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "rangeInfo": {
            "from": 0,
            "to": 100
        },
        "pictureModeInfo": [
            "`Custom`, `Movie`, `Game`, `Sports`, `Standard`"
        ],
        "videoSourceInfo": [
            "`HDMI1`,`HDMI2`, `HDMI3`, `IP`"
        ],
        "videoFormatInfo": [
            "`SDR`,`HLG`,`HDR10`,`DV`"
        ],
        "success": true
    }
}
```

<a name="getSaturation"></a>
## *getSaturation*

Gets the TV user saturation level for a given picture mode, source and format. This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | string | <sup>*(optional)*</sup> The picture mode that the saturation level should be fetched for. e.g. "Current", "Standard", "Sports", "EnergySaving". "Current" - the current selected picture mode. Other valid values for a platform can be obtained from options field of [getPictureModeCaps](#getPictureModeCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoSource | string | <sup>*(optional)*</sup> The video source that the saturation level should be fetched for. e.g. "Current", "Composite1", "HDMI1", "HDMI2", "HDMI3", "IP", "Tuner". "Current" - the current primary active source. When there is no active video playback or input port selected, "Current" refers to "IP" video source as default. Other valid values for a platform can be obtained from options field of [getVideoSourceCaps](#getVideoSourceCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoFormat | string | <sup>*(optional)*</sup> The video format that the saturation level should be fetched for. e.g. "Current", "SDR", "HDR10", "HLG", "DV". "Current" - the current format of the current primary active source. When there is no active video playback or input port selected, "Current" refers to "SDR" video format as default. Other valid values for a platform can be obtained from options field of [getVideoFormatCaps](#getVideoFormatCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.saturation | number | The saturation level in the range from - to inclusive |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getSaturation",
    "params": {
        "pictureMode": "Custom",
        "videoSource": "Current",
        "videoFormat": "SDR"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "saturation": 50,
        "success": true
    }
}
```

<a name="getSaturationCaps"></a>
## *getSaturationCaps*

Gets the capability and boundary range for TV user saturation. It returns the range of TV user saturation values that the user can set and the list of possible picture modes, video sources and video formats that the TV user saturation may be customised for.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.rangeInfo | object | Range of values for the TV user saturation |
| result.rangeInfo.from | number | The lower-inclusive range value for result.saturation |
| result.rangeInfo.to | number | The higher-inclusive range value for result.saturation |
| result?.pictureModeInfo | array | <sup>*(optional)*</sup> Array of picture modes that the saturation could be customised for. These are few examples of picture modes available in this array: ["Standard", "Vivid", "Custom", "Movie", "Sports"]. This API will return the entire list of picture modes returned by [getPictureModeCaps](#getPictureModeCaps) or a subset of it.<br><br>If saturation cannot be customised for picture modes, then this property is not returned |
| result?.pictureModeInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoSourceInfo | array | <sup>*(optional)*</sup> Array of video sources that the saturation could be customised for. These are few examples of video sources available in this array: ["Composite1", "HDMI1", "HDMI2", "HDMI3", "IP" ]. This API will return the entire list of video sources returned by [getVideoSourceCaps](#getVideoSourceCaps), or a subset of it.<br><br>If saturation cannot be customised for video sources, then this property is not returned |
| result?.videoSourceInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoFormatInfo | array | <sup>*(optional)*</sup> Array of video formats that the saturation could be customised for. These are few examples of formats available in this array: ["SDR","HLG","HDR10","DV"].This API will return the entire list of video formats returned by [getVideoFormatCaps](#getVideoFormatCaps), or a subset of it.<br><br>If saturation cannot be customised for video format, then this property is not returned |
| result?.videoFormatInfo[#] | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getSaturationCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "rangeInfo": {
            "from": 0,
            "to": 100
        },
        "pictureModeInfo": [
            "`Custom`, `Movie`, `Game`, `Sports`, `Standard`"
        ],
        "videoSourceInfo": [
            "`HDMI1`,`HDMI2`, `HDMI3`, `IP`"
        ],
        "videoFormatInfo": [
            "`SDR`,`HLG`,`HDR10`,`DV`"
        ],
        "success": true
    }
}
```

<a name="getHue"></a>
## *getHue*

Gets the TV hue level for a given picture mode, source and format. This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | string | <sup>*(optional)*</sup> The picture mode that the hue level should be fetched for. e.g. "Current", "Standard", "Sports", "EnergySaving". "Current" - the current selected picture mode. Other valid values for a platform can be obtained from options field of [getPictureModeCaps](#getPictureModeCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoSource | string | <sup>*(optional)*</sup> The video source that the hue level should be fetched for. e.g. "Current", "Composite1", "HDMI1", "HDMI2", "HDMI3", "IP", "Tuner". "Current" - the current primary active source. When there is no active video playback or input port selected, "Current" refers to "IP" video source as default. Other valid values for a platform can be obtained from options field of [getVideoSourceCaps](#getVideoSourceCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoFormat | string | <sup>*(optional)*</sup> The video format that the hue level should be fetched for. e.g. "Current", "SDR", "HDR10", "HLG", "DV". "Current" - the current format of the current primary active source. When there is no active video playback or input port selected, "Current" refers to "SDR" video format as default. Other valid values for a platform can be obtained from options field of [getVideoFormatCaps](#getVideoFormatCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.hue | number | The hue level in the range from - to inclusive |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getHue",
    "params": {
        "pictureMode": "Custom",
        "videoSource": "Current",
        "videoFormat": "SDR"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "hue": 50,
        "success": true
    }
}
```

<a name="getHueCaps"></a>
## *getHueCaps*

Gets the capability and boundary range for TV hue. It returns the range of TV hue values that the user can set and the list of possible picture modes, video sources and video formats that the TV hue may be customised for.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.rangeInfo | object | Range of values for the TV hue |
| result.rangeInfo.from | number | The lower-inclusive range value for result.hue |
| result.rangeInfo.to | number | The higher-inclusive range value for result.hue |
| result?.pictureModeInfo | array | <sup>*(optional)*</sup> Array of picture modes that the hue could be customised for. These are few examples of picture modes available in this array: ["Standard", "Vivid", "Custom", "Movie", "Sports"]. This API will return the entire list of picture modes returned by [getPictureModeCaps](#getPictureModeCaps) or a subset of it.<br><br>If hue cannot be customised for picture modes, then this property is not returned |
| result?.pictureModeInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoSourceInfo | array | <sup>*(optional)*</sup> Array of video sources that the hue could be customised for. These are few examples of video sources available in this array: ["Composite1", "HDMI1", "HDMI2", "HDMI3", "IP" ]. This API will return the entire list of video sources returned by [getVideoSourceCaps](#getVideoSourceCaps), or a subset of it.<br><br>If hue cannot be customised for video sources, then this property is not returned |
| result?.videoSourceInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoFormatInfo | array | <sup>*(optional)*</sup> Array of video formats that the hue could be customised for. These are few examples of formats available in this array: ["SDR","HLG","HDR10","DV"].This API will return the entire list of video formats returned by [getVideoFormatCaps](#getVideoFormatCaps), or a subset of it.<br><br>If hue cannot be customised for video format, then this property is not returned |
| result?.videoFormatInfo[#] | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getHueCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "rangeInfo": {
            "from": 0,
            "to": 100
        },
        "pictureModeInfo": [
            "`Custom`, `Movie`, `Game`, `Sports`, `Standard`"
        ],
        "videoSourceInfo": [
            "`HDMI1`,`HDMI2`, `HDMI3`, `IP`"
        ],
        "videoFormatInfo": [
            "`SDR`,`HLG`,`HDR10`,`DV`"
        ],
        "success": true
    }
}
```

<a name="getColorTemperature"></a>
## *getColorTemperature*

Gets the TV color temperature level for a given picture mode, source and format. This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | string | <sup>*(optional)*</sup> The picture mode that the color temperature level should be fetched for. e.g. "Current", "Standard", "Sports", "EnergySaving". "Current" - the current selected picture mode. Other valid values for a platform can be obtained from options field of [getPictureModeCaps](#getPictureModeCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoSource | string | <sup>*(optional)*</sup> The video source that the color temperature level should be fetched for. e.g. "Current", "Composite1", "HDMI1", "HDMI2", "HDMI3", "IP", "Tuner". "Current" - the current primary active source. When there is no active video playback or input port selected, "Current" refers to "IP" video source as default. Other valid values for a platform can be obtained from options field of [getVideoSourceCaps](#getVideoSourceCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoFormat | string | <sup>*(optional)*</sup> The video format that the color temperature level should be fetched for. e.g. "Current", "SDR", "HDR10", "HLG", "DV". "Current" - the current format of the current primary active source. When there is no active video playback or input port selected, "Current" refers to "SDR" video format as default. Other valid values for a platform can be obtained from options field of [getVideoFormatCaps](#getVideoFormatCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.colorTemperature | string | The selected color temperature for the picture mode, video source and video format mentioned in the request |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getColorTemperature",
    "params": {
        "pictureMode": "Custom",
        "videoSource": "Current",
        "videoFormat": "SDR"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "colorTemperature": "Standard",
        "success": true
    }
}
```

<a name="getColorTemperatureCaps"></a>
## *getColorTemperatureCaps*

Gets the capability and boundary range for TV color temperature. It returns the range of TV color temperature values that the user can set and the list of possible picture modes, video sources and video formats that the TV color temperature may be customised for.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.options | array | The array of available color temperature options. e.g., of the color temperatures available in the array: ["Standard", "Warm", "Cold", "UserDefined"] |
| result.options[#] | string |  |
| result?.pictureModeInfo | array | <sup>*(optional)*</sup> Array of picture modes that the hue could be customised for. These are few examples of picture modes available in this array: ["Standard", "Vivid", "Custom", "Movie", "Sports"]. This API will return the entire list of picture modes returned by [getPictureModeCaps](#getPictureModeCaps) or a subset of it.<br><br>If hue cannot be customised for picture modes, then this property is not returned |
| result?.pictureModeInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoSourceInfo | array | <sup>*(optional)*</sup> Array of video sources that the hue could be customised for. These are few examples of video sources available in this array: ["Composite1", "HDMI1", "HDMI2", "HDMI3", "IP" ]. This API will return the entire list of video sources returned by [getVideoSourceCaps](#getVideoSourceCaps), or a subset of it.<br><br>If hue cannot be customised for video sources, then this property is not returned |
| result?.videoSourceInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoFormatInfo | array | <sup>*(optional)*</sup> Array of video formats that the hue could be customised for. These are few examples of formats available in this array: ["SDR","HLG","HDR10","DV"].This API will return the entire list of video formats returned by [getVideoFormatCaps](#getVideoFormatCaps), or a subset of it.<br><br>If hue cannot be customised for video format, then this property is not returned |
| result?.videoFormatInfo[#] | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getColorTemperatureCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "options": [
            "`Standard`, `Warm`, `Cold`, `UserDefined`"
        ],
        "pictureModeInfo": [
            "`Custom`, `Movie`, `Game`, `Sports`, `Standard`"
        ],
        "videoSourceInfo": [
            "`HDMI1`,`HDMI2`, `HDMI3`, `IP`"
        ],
        "videoFormatInfo": [
            "`SDR`,`HLG`,`HDR10`,`DV`"
        ],
        "success": true
    }
}
```

<a name="getCMS"></a>
## *getCMS*

Gets the TV CMS(Color Management System) color component level for a given picture mode, source and format for the specified color and component. This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | string | <sup>*(optional)*</sup> The picture mode that the CMS level should be fetched for. e.g. "Current", "Standard", "Sports", "EnergySaving". "Current" - the current selected picture mode. Other valid values for a platform can be obtained from options field of [getPictureModeCaps](#getPictureModeCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoSource | string | <sup>*(optional)*</sup> The video source that the CMS level should be fetched for. e.g. "Current", "Composite1", "HDMI1", "HDMI2", "HDMI3", "IP", "Tuner". "Current" - the current primary active source. When there is no active video playback or input port selected, "Current" refers to "IP" video source as default. Other valid values for a platform can be obtained from options field of [getVideoSourceCaps](#getVideoSourceCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoFormat | string | <sup>*(optional)*</sup> The video format that the CMS level should be fetched for. e.g. "Current", "SDR", "HDR10", "HLG", "DV". "Current" - the current format of the current primary active source. When there is no active video playback or input port selected, "Current" refers to "SDR" video format as default. Other valid values for a platform can be obtained from options field of [getVideoFormatCaps](#getVideoFormatCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params.color | string | The color that the CMS should be fetched for. e.g. "Red", "Green", "Blue", "Cyan", "Yellow", "Magenta". Valid values for a platform can be obtained from colorInfo field of [getCMSCaps](#getCMSCaps).<br><br>The caller must PASS a single valid value for this parameter |
| params.component | string | The color that the CMS should be fetched for. e.g. "Saturation", "Hue", "Luma". Valid values for a platform can be obtained from componentInfo field of [getCMSCaps](#getCMSCaps).<br><br>The caller must PASS a single valid value for this parameter |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.level | number | The CMS level in the range from - to inclusive |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getCMS",
    "params": {
        "pictureMode": "Custom",
        "videoSource": "Current",
        "videoFormat": "SDR",
        "color": "Red",
        "component": "Saturation"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "level": 50,
        "success": true
    }
}
```

<a name="getCMSCaps"></a>
## *getCMSCaps*

Gets the capability and boundary range for TV CMS(Color Management System). It returns the range of TV CMS value that the user can set and the list of possible picture modes, video sources and video formats that the TV CMS may be customised for.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.platformSupport | boolean | true: The platform support CMS. false:  The platform does not support CMS |
| result.componentSaturationRange | object | Range of values for the componentSaturation |
| result.componentSaturationRange.from | number | The lower-inclusive range value for componentSaturation |
| result.componentSaturationRange.to | number | The higher-inclusive range value for componentSaturation |
| result.componentHueRange | object | Range of values for componentHue |
| result.componentHueRange.from | number | The lower-inclusive range value for componentHue |
| result.componentHueRange.to | number | The higher-inclusive range value for componentHue |
| result.componentLumaRange | object | Range of values for componentLuma |
| result.componentLumaRange.from | number | The lower-inclusive range value for componentLuma |
| result.componentLumaRange.to | number | The higher-inclusive range value for componentLuma |
| result?.pictureModeInfo | array | <sup>*(optional)*</sup> Array of picture modes that the CMS could be customised for. These are few examples of picture modes available in this array: "Standard", "Vivid", "Custom", "Movie", "Sports"]. This API will return the entire list of picture modes returned by [getPictureModeCaps](#getPictureModeCaps) or a subset of it.<br><br>If CMS cannot be customised for picture modes, then this property is not returned |
| result?.pictureModeInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoSourceInfo | array | <sup>*(optional)*</sup> Array of video sources that the CMS could be customised for. These are few examples of video sources available in this array: ["Composite1", "HDMI1", "HDMI2", "HDMI3", "IP" ].This API will return the entire list of video sources returned by [getVideoSourceCaps](#getVideoSourceCaps), or a subset of it. If CMS cannot be customised for video sources, then this property is not returned |
| result?.videoSourceInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoFormatInfo | array | <sup>*(optional)*</sup> Array of video formats that the CMS could be customised for. These are few examples of formats available in this array: ["SDR","HLG","HDR10","DV"]. This API will return the entire list of video formats returned by [getVideoFormatCaps](#getVideoFormatCaps), or a subset of it. If CMS cannot be customised for video format, then this property is not returned |
| result?.videoFormatInfo[#] | string | <sup>*(optional)*</sup>  |
| result.colorInfo | array | Array of color values that the CMS could be customized for. These are few examples of formats available in this array: ["Red","Green","Blue","Cyan","Magenta","Yellow"] |
| result.colorInfo[#] | string |  |
| result.componentInfo | array | Array of component values that the CMS could be customized for. These are few examples of formats available in this array: ["Saturation","Hue","Luma"] |
| result.componentInfo[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getCMSCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "platformSupport": true,
        "componentSaturationRange": {
            "from": 0,
            "to": 100
        },
        "componentHueRange": {
            "from": 0,
            "to": 100
        },
        "componentLumaRange": {
            "from": 0,
            "to": 30
        },
        "pictureModeInfo": [
            "`Custom`, `Movie`, `Game`, `Sports`, `Standard`"
        ],
        "videoSourceInfo": [
            "`HDMI1`,`HDMI2`, `HDMI3`, `IP`"
        ],
        "videoFormatInfo": [
            "`SDR`,`HLG`,`HDR10`,`DV`"
        ],
        "colorInfo": [
            "`Red`, `Green`, `Blue`, `Cyan`, `Magenta`, `Yellow`"
        ],
        "componentInfo": [
            "`Saturation`, `Hue`, `Luma`"
        ],
        "success": true
    }
}
```

<a name="getLowLatencyState"></a>
## *getLowLatencyState*

Gets the TV low latency State for a given picture mode, source and format. This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | string | <sup>*(optional)*</sup> The picture mode that the lowLatencyState level should be fetched for. e.g. "Current", "Standard", "Sports", "EnergySaving". "Current" - the current selected picture mode. Other valid values for a platform can be obtained from options field of [getPictureModeCaps](#getPictureModeCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoSource | string | <sup>*(optional)*</sup> The video source that the lowLatencyState level should be fetched for. e.g. "Current", "Composite1", "HDMI1", "HDMI2", "HDMI3", "IP", "Tuner". "Current" - the current primary active source. When there is no active video playback or input port selected, "Current" refers to "IP" video source as default. Other valid values for a platform can be obtained from options field of [getVideoSourceCaps](#getVideoSourceCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoFormat | string | <sup>*(optional)*</sup> The video format that the lowLatencyState level should be fetched for. e.g. "Current", "SDR", "HDR10", "HLG", "DV". "Current" - the current format of the current primary active source. When there is no active video playback or input port selected, "Current" refers to "SDR" video format as default. Other valid values for a platform can be obtained from options field of [getVideoFormatCaps](#getVideoFormatCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.backlight | boolean | <sup>*(optional)*</sup> True if device is in lowLatencyState |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getLowLatencyState",
    "params": {
        "pictureMode": "Custom",
        "videoSource": "Current",
        "videoFormat": "SDR"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "backlight": true,
        "success": true
    }
}
```

<a name="getLowLatencyStateCaps"></a>
## *getLowLatencyStateCaps*

Gets the capability and boundary range for low latency state. It returns the range of low latency state that the user can set and the list of possible picture modes, video sources and video formats that the low latency state may be customised for.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.platformSupport | boolean | true: The platform support lowLatencyState. false:  The platform does not support lowLatencyState |
| result?.pictureModeInfo | array | <sup>*(optional)*</sup> Array of picture modes that the lowLatencyState could be customised for. These are few examples of picture modes available in this array: "Standard", "Vivid", "Custom", "Movie", "Sports"]. This API will return the entire list of picture modes returned by [getPictureModeCaps](#getPictureModeCaps) or a subset of it.<br><br>If lowLatencyState cannot be customised for picture modes, then this property is not returned |
| result?.pictureModeInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoSourceInfo | array | <sup>*(optional)*</sup> Array of video sources that the lowLatencyState could be customised for. These are few examples of video sources available in this array: ["Composite1", "HDMI1", "HDMI2", "HDMI3", "IP" ].This API will return the entire list of video sources returned by [getVideoSourceCaps](#getVideoSourceCaps), or a subset of it. If lowLatencyState cannot be customised for video sources, then this property is not returned |
| result?.videoSourceInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoFormatInfo | array | <sup>*(optional)*</sup> Array of video formats that the lowLatencyState could be customised for. These are few examples of formats available in this array: ["SDR","HLG","HDR10","DV"]. This API will return the entire list of video formats returned by [getVideoFormatCaps](#getVideoFormatCaps), or a subset of it. If lowLatencyState cannot be customised for video format, then this property is not returned |
| result?.videoFormatInfo[#] | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getLowLatencyStateCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "platformSupport": true,
        "pictureModeInfo": [
            "`Custom`, `Movie`, `Game`, `Sports`, `Standard`"
        ],
        "videoSourceInfo": [
            "`HDMI1`,`HDMI2`, `HDMI3`, `IP`"
        ],
        "videoFormatInfo": [
            "`SDR`,`HLG`,`HDR10`,`DV`"
        ],
        "success": true
    }
}
```

<a name="getZoomMode"></a>
## *getZoomMode*

Gets the current TV zoom mode. This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | string | <sup>*(optional)*</sup> The picture mode that the zoom should be fetched for. e.g. "Current", "Standard", "Sports", "EnergySaving". "Current" - the current selected picture mode. Other valid values for a platform can be obtained from options field of [getPictureModeCaps](#getPictureModeCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoSource | string | <sup>*(optional)*</sup> The video source that the zoom should be fetched for. e.g. "Current", "Composite1", "HDMI1", "HDMI2", "HDMI3", "IP", "Tuner". "Current" - the current primary active source. When there is no active video playback or input port selected, "Current" refers to "IP" video source as default. Other valid values for a platform can be obtained from options field of [getVideoSourceCaps](#getVideoSourceCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoFormat | string | <sup>*(optional)*</sup> The video format that the zoom should be fetched for. e.g. "Current", "SDR", "HDR10", "HLG", "DV". "Current" - the current format of the current primary active source. When there is no active video playback or input port selected, "Current" refers to "SDR" video format as default. Other valid values for a platform can be obtained from options field of [getVideoFormatCaps](#getVideoFormatCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.zoomMode | string | The zoom mode currently set |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getZoomMode",
    "params": {
        "pictureMode": "Custom",
        "videoSource": "Current",
        "videoFormat": "SDR"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "zoomMode": "TV NORMAL",
        "success": true
    }
}
```

<a name="getZoomModeCaps"></a>
## *getZoomModeCaps*

Gets the capability for TV zoom mode. This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.options | array | Array of available zoom modes. These are few examples of options available in this array: ["TV AUTO","TV DIRECT","TV NORMAL","TV 16X9 STRETCH","TV 4X3 PILLARBOX","TV ZOOM"]<br><br>TV AUTO - Auto mode shall fill the 16:9 panel display, without cropping any video and leaving black bars, while maintaining the pixel aspect ratio.<br>TV DIRECT - Show actual image as it is without cropping<br>TV NORMAL - Same as Auto<br>TV 16X9 STRETCH - 16:9 mode shall always resize the video to fill the 16:9 panel display.<br>TV 4X3 PILLARBOX - 4:3 mode shall always resize the video to fill the centre 4:3 area of the 16:9 panel display.<br>TV ZOOM - Zoom mode shall zoom fill the 16:9 panel display, leaving no black bars and cropping the video, while maintaining the pixel aspect ratio.<br><br>The list options will change per platform |
| result.options[#] | string |  |
| result?.pictureModeInfo | array | <sup>*(optional)*</sup> Array of picture modes that the zoom mode could be customised for. These are few examples of picture modes available in this array: "Standard", "Vivid", "Custom", "Movie", "Sports"]. This API will return the entire list of picture modes returned by [getPictureModeCaps](#getPictureModeCaps) or a subset of it.<br><br>If zoom mode cannot be customised for picture modes, then this property is not returned |
| result?.pictureModeInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoSourceInfo | array | <sup>*(optional)*</sup> Array of video sources that the zoom mode could be customised for. These are few examples of video sources available in this array: ["Composite1", "HDMI1", "HDMI2", "HDMI3", "IP" ].This API will return the entire list of video sources returned by [getVideoSourceCaps](#getVideoSourceCaps), or a subset of it. If zoom mode cannot be customised for video sources, then this property is not returned |
| result?.videoSourceInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoFormatInfo | array | <sup>*(optional)*</sup> Array of video formats that the zoom mode could be customised for. These are few examples of formats available in this array: ["SDR","HLG","HDR10","DV"]. This API will return the entire list of video formats returned by [getVideoFormatCaps](#getVideoFormatCaps), or a subset of it. If zoom mode cannot be customised for video format, then this property is not returned |
| result?.videoFormatInfo[#] | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getZoomModeCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "options": [
            "TV AUTO"
        ],
        "pictureModeInfo": [
            "`Custom`, `Movie`, `Game`, `Sports`, `Standard`"
        ],
        "videoSourceInfo": [
            "`HDMI1`,`HDMI2`, `HDMI3`, `IP`"
        ],
        "videoFormatInfo": [
            "`SDR`,`HLG`,`HDR10`,`DV`"
        ],
        "success": true
    }
}
```

<a name="getBacklightDimmingMode"></a>
## *getBacklightDimmingMode*

Gets the TV backlight dimming mode for a given picture mode, source and format. This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | string | <sup>*(optional)*</sup> The picture mode that the backlight dimming mode should be fetched for. e.g. "Current", "Standard", "Sports", "EnergySaving". "Current" - the current selected picture mode. Other valid values for a platform can be obtained from options field of [getPictureModeCaps](#getPictureModeCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoSource | string | <sup>*(optional)*</sup> The video source that the backlight dimming mode should be fetched for. e.g. "Current", "Composite1", "HDMI1", "HDMI2", "HDMI3", "IP", "Tuner". "Current" - the current primary active source. When there is no active video playback or input port selected, "Current" refers to "IP" video source as default. Other valid values for a platform can be obtained from options field of [getVideoSourceCaps](#getVideoSourceCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoFormat | string | <sup>*(optional)*</sup> The video format that the backlight dimming mode should be fetched for. e.g. "Current", "SDR", "HDR10", "HLG", "DV". "Current" - the current format of the current primary active source. When there is no active video playback or input port selected, "Current" refers to "SDR" video format as default. Other valid values for a platform can be obtained from options field of [getVideoFormatCaps](#getVideoFormatCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.dimmingMode | string | The selected dimming mode for the picture mode, video source and video format mentioned in the request |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getBacklightDimmingMode",
    "params": {
        "pictureMode": "Custom",
        "videoSource": "Current",
        "videoFormat": "SDR"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "dimmingMode": "Fixed",
        "success": true
    }
}
```

<a name="getBacklightDimmingModeCaps"></a>
## *getBacklightDimmingModeCaps*

Gets the capabilities and boundary range for TV backlight dimming mode. It returns the range of TV dimming mode values that the user can set and the list of possible picture modes, video sources and video formats that the TV backlight dimming mode may be customized for. This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.platformSupport | boolean | true: The platform support backlight dimming mode. false:  The platform does not support backlight dimming mode |
| result.options | array | The array of available dimming mode options. e.g., of the color temperatures available in the array: ["Fixed" , "Local" , "Global" ]<br><br>Local :The backlight level can be controlled on individual zones based on the histogram.<br>Fixed: The backlight level is fixed. User backlight will be applied. No dimming.<br>Global: The backlight level is controlled globally based on the current frame histogram |
| result.options[#] | string |  |
| result?.pictureModeInfo | array | <sup>*(optional)*</sup> Array of picture modes that the dimming mode could be customised for. These are few examples of picture modes available in this array: "Standard", "Vivid", "Custom", "Movie", "Sports"]. This API will return the entire list of picture modes returned by [getPictureModeCaps](#getPictureModeCaps) or a subset of it.<br>If dimming mode cannot be customised for picture modes, then this property is not returned |
| result?.pictureModeInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoSourceInfo | array | <sup>*(optional)*</sup> Array of video sources that the dimming mode could be customised for. These are few examples of video sources available in this array: ["Composite1", "HDMI1", "HDMI2", "HDMI3", "IP" ].This API will return the entire list of video sources returned by [getVideoSourceCaps](#getVideoSourceCaps), or a subset of it.<br>If dimming mode cannot be customised for video sources, then this property is not returned |
| result?.videoSourceInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoFormatInfo | array | <sup>*(optional)*</sup> Array of video formats that the dimming mode could be customised for. These are few examples of formats available in this array: ["SDR","HLG","HDR10","DV"]. This API will return the entire list of video formats returned by [getVideoFormatCaps](#getVideoFormatCaps), or a subset of it.<br>If dimming mode cannot be customised for video format, then this property is not returned |
| result?.videoFormatInfo[#] | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getBacklightDimmingModeCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "platformSupport": true,
        "options": [
            "Fixed"
        ],
        "pictureModeInfo": [
            "`Custom`, `Movie`, `Game`, `Sports`, `Standard`"
        ],
        "videoSourceInfo": [
            "`HDMI1`,`HDMI2`, `HDMI3`, `IP`"
        ],
        "videoFormatInfo": [
            "`SDR`,`HLG`,`HDR10`,`DV`"
        ],
        "success": true
    }
}
```

<a name="getDolbyVisionMode"></a>
## *getDolbyVisionMode*

Gets the DolbyVision mode for a given picture mode and video source. This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | string | <sup>*(optional)*</sup> The picture mode that the DolbyVision mode should be fetched for. e.g. "Current", "Standard", "Sports", "EnergySaving". "Current" - the current selected picture mode. Other valid values for a platform can be obtained from options field of [getPictureModeCaps](#getPictureModeCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoSource | string | <sup>*(optional)*</sup> The video source that the DolbyVision mode should be fetched for. e.g. "Current", "Composite1", "HDMI1", "HDMI2", "HDMI3", "IP", "Tuner". "Current" - the current primary active source. When there is no active video playback or input port selected, "Current" refers to "IP" video source as default. Other valid values for a platform can be obtained from options field of [getVideoSourceCaps](#getVideoSourceCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.dolbyVisionMode | string | The DolbyVision mode requested |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getDolbyVisionMode",
    "params": {
        "pictureMode": "Custom",
        "videoSource": "Current"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "dolbyVisionMode": "Bright",
        "success": true
    }
}
```

<a name="getDolbyVisionModeCaps"></a>
## *getDolbyVisionModeCaps*

Gets the capability for DolbyVision mode. It returns the options of DolbyVision mode values that the user can set and the list of possible picture modes and video sources that the DolbyVision mode may be customised for.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.platformSupport | boolean | true: The platform support DolbyVision mode. false:  The platform does not support DolbyVision mode |
| result.options | array | Array of available options of DolbyVision modes. e.g. "Dark", "Bright" |
| result.options[#] | string |  |
| result?.pictureModeInfo | array | <sup>*(optional)*</sup> Array of picture modes that the DolbyVision mode could be customised for. These are few examples of picture modes available in this array: "Standard", "Vivid", "Custom", "Movie", "Sports"]. This API will return the entire list of picture modes returned by [getPictureModeCaps](#getPictureModeCaps) or a subset of it.<br><br>If DolbyVision mode cannot be customised for picture modes, then this property is not returned |
| result?.pictureModeInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoSourceInfo | array | <sup>*(optional)*</sup> Array of video sources that the DolbyVision mode could be customised for. These are few examples of video sources available in this array: ["Composite1", "HDMI1", "HDMI2", "HDMI3", "IP" ].This API will return the entire list of video sources returned by [getVideoSourceCaps](#getVideoSourceCaps), or a subset of it. If DolbyVision mode cannot be customised for video sources, then this property is not returned |
| result?.videoSourceInfo[#] | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getDolbyVisionModeCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "platformSupport": true,
        "options": [
            "`Dark` , `Bright`"
        ],
        "pictureModeInfo": [
            "`Custom`, `Movie`, `Game`, `Sports`, `Standard`"
        ],
        "videoSourceInfo": [
            "`HDMI1`,`HDMI2`, `HDMI3`, `IP`"
        ],
        "success": true
    }
}
```

<a name="getHDRMode"></a>
## *getHDRMode*

Gets the HDR10 mode for a given picture mode, video format and video source. This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | string | <sup>*(optional)*</sup> The picture mode that the HDR mode should be fetched for. e.g. "Current", "Standard", "Sports", "EnergySaving". "Current" - the current selected picture mode. Other valid values for a platform can be obtained from options field of [getPictureModeCaps](#getPictureModeCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoSource | string | <sup>*(optional)*</sup> The video source that the HDR mode should be fetched for. e.g. "Current", "HDMI1", "HDMI2", "HDMI3", "IP". "Current" - the current primary active source. When there is no active video playback or input port selected, "Current" refers to "IP" video source as default. Other valid values for a platform can be obtained from options field of [getVideoSourceCaps](#getVideoSourceCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoFormat | string | <sup>*(optional)*</sup> The video format that the HDR mode should be fetched for. e.g. "Current", "HDR10", "HLG", "DV". "Current" - the current format of the current primary active source. When there is no active video playback or input port selected, "Current" refers to "SDR" video format as default. Other valid values for a platform can be obtained from options field of [getVideoFormatCaps](#getVideoFormatCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.hdrMode | string | <sup>*(optional)*</sup> The HDR mode requested |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getHDRMode",
    "params": {
        "pictureMode": "Custom",
        "videoSource": "Current",
        "videoFormat": "SDR"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "hdrMode": "Bright",
        "success": true
    }
}
```

<a name="getHDRModeCaps"></a>
## *getHDRModeCaps*

Gets the capability for HDR mode. It returns the options of HDR mode values that the user can set and the list of possible picture modes, video sources and video formats that the HDR mode may be customized for.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.platformSupport | boolean | true: The platform support HDR modes. false:  The platform does not support HDR modes |
| result.options | array | Array of HDR Modes. e.g. "Dark", "Bright" |
| result.options[#] | string |  |
| result?.pictureModeInfo | array | <sup>*(optional)*</sup> Array of picture modes that the HDR mode could be customised for. These are few examples of picture modes available in this array: "Standard", "Vivid", "Custom", "Movie", "Sports"]. This API will return the entire list of picture modes returned by [getPictureModeCaps](#getPictureModeCaps) or a subset of it.<br><br>If HDR mode cannot be customised for picture modes, then this property is not returned |
| result?.pictureModeInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoSourceInfo | array | <sup>*(optional)*</sup> Array of video sources that the HDR mode could be customised for. These are few examples of video sources available in this array: ["Composite1", "HDMI1", "HDMI2", "HDMI3", "IP" ].This API will return the entire list of video sources returned by [getVideoSourceCaps](#getVideoSourceCaps), or a subset of it. If HDR mode cannot be customised for video sources, then this property is not returned |
| result?.videoSourceInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoFormatInfo | array | <sup>*(optional)*</sup> Array of video formats that the HDR mode could be customised for. These are few examples of formats available in this array: ["HLG","HDR10","DV"]. This API will return the entire list of video formats returned by [getVideoFormatCaps](#getVideoFormatCaps), or a subset of it. If HDR mode cannot be customised for video format, then this property is not returned |
| result?.videoFormatInfo[#] | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getHDRModeCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "platformSupport": true,
        "options": [
            "`Dark` , `Bright`"
        ],
        "pictureModeInfo": [
            "`Custom`, `Movie`, `Game`, `Sports`, `Standard`"
        ],
        "videoSourceInfo": [
            "`HDMI1`,`HDMI2`, `HDMI3`, `IP`"
        ],
        "videoFormatInfo": [
            "`SDR`,`HLG`,`HDR10`,`DV`"
        ],
        "success": true
    }
}
```

<a name="getVideoFormatCaps"></a>
## *getVideoFormatCaps*

Get all supported video formats for the platform.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.options | array | Array of video formats that the system can support. These are few examples of video formats available in this array:<br><br>e.g,["HDR10", "HLG", "SDR", "DV"] |
| result.options[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getVideoFormatCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "options": [
            "`HDR10`, `HLG`, `SDR`, `DV`"
        ],
        "success": true
    }
}
```

<a name="getVideoSourceCaps"></a>
## *getVideoSourceCaps*

Get all supported video sources for the platform.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.options | array | Array of video sources that the system can support. These are few examples of video sources available in this array:<br><br>e.g, ["HDMI1", "HDMI2", "HDMI3", "IP", "Tuner", "Composite1"] |
| result.options[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getVideoSourceCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "options": [
            "`HDMI1`, `HDMI2`, `HDMI3`, `IP`, `Tuner`, `Composite1`"
        ],
        "success": true
    }
}
```

<a name="getVideoFrameRateCaps"></a>
## *getVideoFrameRateCaps*

Get all supported video frame rates for the platform.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.options | array | Array of video frame rates that the system can support. These are few examples of video frame rates available in this array: [24,30,50,60] |
| result.options[#] | number |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getVideoFrameRateCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "options": [
            24,
            30,
            50,
            60
        ],
        "success": true
    }
}
```

<a name="getVideoResolutionCaps"></a>
## *getVideoResolutionCaps*

Get all supported video resolutions for the platform.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.maxResolution | string | <sup>*(optional)*</sup> The maximum resolution supported by the system.<br>e.g, "4096*2160p" |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getVideoResolutionCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "maxResolution": "4096*2160p",
        "success": true
    }
}
```

<a name="getPictureMode"></a>
## *getPictureMode*

Get picture mode for a given video source and video format.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.videoSource | string | <sup>*(optional)*</sup> The video source for which the picture mode is requested for. e.g. "Current", "HDMI1", "Composite1", "IP". "Current" refers to the current selected video source. When there is no active video playback or input port selected, "Current" refers to "IP" video source as default. Other valid values for a platform can be obtained from options field of [getVideoSourceCaps](#getVideoSourceCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoFormat | string | <sup>*(optional)*</sup> The video format for which the picture mode is requested for. e.g. "Current", "HDR10", "SDR", "DV". "Current" refers to the current video format of the video played. When there is no active video playback or input port selected, "Current" refers to "SDR" video format as default. Other valid values for a platform can be obtained from options field of [getVideoFormatCaps](#getVideoFormatCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.pictureMode | string | The picture mode string requested |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getPictureMode",
    "params": {
        "videoSource": "HDMI1",
        "videoFormat": "Current"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "pictureMode": "Standard",
        "success": true
    }
}
```

<a name="getPictureModeCaps"></a>
## *getPictureModeCaps*

Get the list of picture modes supported and the list of video sources and video formats against which a picture mode can be associated with.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.options | array | Array of picture modes supported.<br>e.g. ["Standard", "Vivid", "Custom", "Movie", "Sports"] |
| result.options[#] | string |  |
| result?.videoSourceInfo | array | <sup>*(optional)*</sup> Array of video sources to which the picture mode can be associated with.<br>e.g. ["Composite1", "HDMI1", "HDMI2", "HDMI3", "IP" ].<br><br>This API will return the entire list of video sources returned by [getVideoSourceCaps](#getVideoSourceCaps), or a subset of it. If the picture mode cannot be associated with any video source, then this property is not returned |
| result?.videoSourceInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoFormatInfo | array | <sup>*(optional)*</sup> Array of video formats to which the picture mode can be associated with.<br>e.g.["SDR", "HDR10","HLG","DV"].<br><br>This API will return the entire list of video formats returned by [getVideoFormatCaps](#getVideoFormatCaps), or a subset of it. If the picture mode cannot be associated with any video format, then this property is not returned |
| result?.videoFormatInfo[#] | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getPictureModeCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "options": [
            "`Standard`,`Movie`,`Game`,`Custom`,`Vivid`"
        ],
        "videoSourceInfo": [
            "`HDMI1`,`HDMI2`, `HDMI3`, `Tuner`, `IP`"
        ],
        "videoFormatInfo": [
            "`SDR`,`HDR10`,`HLG`,`DV`"
        ],
        "success": true
    }
}
```

<a name="getVideoFormat"></a>
## *getVideoFormat*

Returns the video format of the currently played video content.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.currentVideoFormat | string | The current video format of the video played.<br>e.g. ["SDR", "HLG","HDR10","DV","None"].<br><br>None - Returned when no video is played.<br>For the complete list of possible video formats refer [getVideoFormatCaps](#getVideoFormatCaps) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getVideoFormat",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "currentVideoFormat": "HLG",
        "success": true
    }
}
```

<a name="getVideoSource"></a>
## *getVideoSource*

Returns the video source selected.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.currentVideoSource | string | The current video source selected.<br>e.g."HDMI1", "HDMI2", "HDMI3", "IP", "Tuner", "Composite1","None".<br><br>None - Returned when no video is played.<br>For the complete list of possible video sources refer [getVideoSourceCaps](#getVideoSourceCaps) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getVideoSource",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "currentVideoSource": "IP",
        "success": true
    }
}
```

<a name="getVideoFrameRate"></a>
## *getVideoFrameRate*

Returns the video frame rate of the video being played.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.currentVideoFrameRate | number | The current video source selected.<br>e.g., 0,24,30,50,60.<br><br>0 - Returned when no video is played.<br>For the complete list of possible video frame rates refer [getVideoFrameRateCaps](#getVideoFrameRateCaps) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getVideoFrameRate",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "currentVideoFrameRate": 30,
        "success": true
    }
}
```

<a name="getVideoContentType"></a>
## *getVideoContentType*

Returns the FilmMaker mode status if it is enabled or disabled and in which sources the FilmMaker mode is active. This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.currentFilmMakerMode | boolean | Possible values are true and false.<br>true - Film Maker mode active<br> false -Film Maker mode not active |
| result?.currentFilmMakerModeSources | array | <sup>*(optional)*</sup> A list of sources in which the Film Maker mode event occurred.<br>If there "params.currentFilmMakerMode" is false this parameter is not passed.<br>Possible video sources for film maker mode are HDMI sources (as returned by [getVideoSourceCaps](#getVideoSourceCaps)) and IP source |
| result?.currentFilmMakerModeSources[#] | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getVideoContentType",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "currentFilmMakerMode": true,
        "currentFilmMakerModeSources": [
            "`HDMI1`,`IP`"
        ],
        "success": true
    }
}
```

<a name="getVideoResolution"></a>
## *getVideoResolution*

Returns the video resolution of the video being played.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.currentVideoResolution | string | The current video resolution of the video being played.<br>e.g.,"None","720\*240p","720\*288p","720\*480i","720\*480p".<br>None - Returned when no video is played.<br>For the complete list of possible video resolution refer [getVideoResolutionCaps](#getVideoResolutionCaps) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getVideoResolution",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "currentVideoResolution": "720*240p",
        "success": true
    }
}
```

<a name="getAutoBackLightMode"></a>
## *getAutoBackLightMode*

Gets the TV auto backlight mode for a given picture mode, source and format. Auto backlight can also refer to Dolby Light Sense in products that are licensed with Dolby Advanced Imaging.This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | string | <sup>*(optional)*</sup> The picture mode that the auto backlight mode should be fetched for. e.g. "Current", "Standard", "Sports", "EnergySaving". "Current" - the current selected picture mode. Other valid values for a platform can be obtained from options field of [getPictureModeCaps](#getPictureModeCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoSource | string | <sup>*(optional)*</sup> The video source that the auto backlight mode should be fetched for. e.g. "Current", "Composite1", "HDMI1", "HDMI2", "HDMI3", "IP", "Tuner". "Current" - the current primary active source. When there is no active video playback or input port selected, "Current" refers to "IP" video source as default. Other valid values for a platform can be obtained from options field of [getVideoSourceCaps](#getVideoSourceCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoFormat | string | <sup>*(optional)*</sup> The picture format that the auto backlight mode should be fetched for. e.g. "Current", "SDR", "HDR10", "HLG", "DV". "Current" - the current format of the current primary active source. When there is no active video playback or input port selected, "Current" refers to "SDR" video format as default. Other valid values for a platform can be obtained from options field of [getVideoFormatCaps](#getVideoFormatCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.mode | string | The auto backlight mode in the options of [getAutoBacklightModeCaps](#getAutoBacklightModeCaps) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getAutoBackLightMode",
    "params": {
        "pictureMode": "Custom",
        "videoSource": "Current",
        "videoFormat": "SDR"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "mode": "Manual",
        "success": true
    }
}
```

<a name="getAutoBacklightModeCaps"></a>
## *getAutoBacklightModeCaps*

Gets the capabilities and boundary range for TV auto backlight mode. It returns the range of auto backlight mode values that the user can set and the list of possible picture modes, video sources and video formats that the TV auto backlight mode may be customized for.<br><br>Auto backlight can also refer to Dolby Light Sense in products that are licensed with Dolby Advanced Imaging.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.platformSupport | boolean | true: The platform support backlight auto backlight mode. false:  The platform does not support backlight auto backlight mode |
| result.options | array | The array of available auto backlight mode options. e.g., of the modes available in the array: ["Manual", "Ambient"]<br><br>Ambient : The backlight level is controlled globally based on the room ambience level.<br>Manual : The backlight level is fixed. User backlight will be applied |
| result.options[#] | string |  |
| result?.pictureModeInfo | array | <sup>*(optional)*</sup> Array of picture modes that the auto backlight mode could be customised for. These are few examples of picture modes available in this array: "Standard", "Vivid", "Custom", "Movie", "Sports"]. This API will return the entire list of picture modes returned by [getPictureModeCaps](#getPictureModeCaps) or a subset of it.<br><br>If auto backlight mode cannot be customised for picture modes, then this property is not returned |
| result?.pictureModeInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoSourceInfo | array | <sup>*(optional)*</sup> Array of video sources that the auto backlight mode could be customised for. These are few examples of video sources available in this array: ["Composite1", "HDMI1", "HDMI2", "HDMI3", "IP" ].This API will return the entire list of video sources returned by [getVideoSourceCaps](#getVideoSourceCaps), or a subset of it.<br><br>If auto backlight mode cannot be customised for video sources, then this property is not returned |
| result?.videoSourceInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoFormatInfo | array | <sup>*(optional)*</sup> Array of video formats that the auto backlight mode could be customised for. These are few examples of formats available in this array: ["SDR","HLG","HDR10","DV"]. This API will return the entire list of video formats returned by [getVideoFormatCaps](#getVideoFormatCaps), or a subset of it.<br><br>If auto backlight mode cannot be customised for video format, then this property is not returned |
| result?.videoFormatInfo[#] | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getAutoBacklightModeCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "platformSupport": true,
        "options": [
            "`Manual`,`Ambient`"
        ],
        "pictureModeInfo": [
            "`Custom`, `Movie`, `Game`, `Sports`, `Standard`"
        ],
        "videoSourceInfo": [
            "`HDMI1`,`HDMI2`, `HDMI3`, `IP`"
        ],
        "videoFormatInfo": [
            "`SDR`,`HLG`,`HDR10`,`DV`"
        ],
        "success": true
    }
}
```

<a name="getFadeDisplayCaps"></a>
## *getFadeDisplayCaps*

Gets the capabilities and boundary range for fading the display.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.platformSupport | boolean | true: The platform supports display fading.<br>false: The platform does not support display fading |
| result?.from | number | <sup>*(optional)*</sup> The percentage of current backlight value from where fade starts |
| result?.to | number | <sup>*(optional)*</sup> The Percentage of current backlight value from where fade ends |
| result.durationInfo | object | Range of values for the fadeDisplay duration value |
| result.durationInfo.from | number | The lower-inclusive range value for durationInfo |
| result.durationInfo.to | number | The higher-inclusive range value for durationInfo |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getFadeDisplayCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "platformSupport": true,
        "from": 100,
        "to": 0,
        "durationInfo": {
            "from": 0,
            "to": 10000
        },
        "success": true
    }
}
```

<a name="setPictureMode"></a>
## *setPictureMode*

Set picture mode for a given video source and video format.<br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params.pictureMode | string | The picture mode to be associated with the specified video source and video format.<br><br>e.g. "sports", "Theatre", "game"<br><br>Other valid values for a platform can be obtained from "pictureModeInfo" field returned by [getPictureModeCaps](#getPictureModeCaps) |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source(s) to which the picture mode needs to be associated with.<br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from "videoSourceInfo" field returned by [getPictureModeCaps](#getPictureModeCaps)<br><br>current refers to current video source selected. When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If current selected video source is not part of videoSourceInfo field returned by [getPictureModeCaps](#getPictureModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that picture mode is associated for all customisable video sources that are returned in videoSourceInfo field by [getPictureModeCaps](#getPictureModeCaps). When there is no videoSourceInfo field returned by [getPictureModeCaps](#getPictureModeCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format(s) to which the picture mode needs to be associated with.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from "videoFormatInfo" field returned by [getPictureModeCaps](#getPictureModeCaps)<br><br>current refers to current video format selected. When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If current selected video source is not part of "videoFormatInfo" field returned by [getPictureModeCaps](#getPictureModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that picture mode is associated for all customisable video formats that are returned in "videoFormatInfo" field by getPictureModeCaps. When there is no "videoFormatInfo" field returned by [getPictureModeCaps](#getPictureModeCaps) then "Global" refers to all video formats returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |

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
    "method": "org.rdk.AVOutput.setPictureMode",
    "params": {
        "pictureMode": "Sports",
        "videoSource": [
            "`HDMI1`, `Tuner`"
        ],
        "videoFormat": [
            "`SDR`, `HLG`"
        ]
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

<a name="setBacklight"></a>
## *setBacklight*

Sets the TV user backlight level for a given picture mode, video format and video source. The backlight value that is to be set should be within the range which is returned by the API [getBacklightCaps](#getBacklightCaps).<br><br>If the auto backlight control is set to ambient, the TV user backlight changes made by this API will not take effect until auto backlight control is switched to manual.<br><br>The set value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the backlight level is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getBacklightCaps](#getBacklightCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getBacklightCaps](#getBacklightCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that backlight is changed for all customisable pictures modes that are returned in pictureModeInfo field by [getBacklightCaps](#getBacklightCaps). When there is no pictureModeInfo field returned by [getBacklightCaps](#getBacklightCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the backlight level is customised for.<br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getBacklightCaps](#getBacklightCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If current selected video source is not part of videoSourceInfo field returned by [getBacklightCaps](#getBacklightCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that backlight is changed for all customisable video sources that are returned in videoSourceInfo field by [getBacklightCaps](#getBacklightCaps). When there is no videoSourceInfo field returned by [getBacklightCaps](#getBacklightCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the backlight level is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getBacklightCaps](#getBacklightCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If current selected video format is not part of videoSourceInfo field returned by [getBacklightCaps](#getBacklightCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that backlight is changed for all customisable video formats that are returned in videoFormatInfo field by [getBacklightCaps](#getBacklightCaps). When there is no videoFormatInfo field returned by [getBacklightCaps](#getBacklightCaps) then "Global" refers to all video formats returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |
| params.backlight | number | The backlight value to be set. It should be within the range as retrieved from [getBacklightCaps](#getBacklightCaps) |

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
    "method": "org.rdk.AVOutput.setBacklight",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "Current"
        ],
        "backlight": 50
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

<a name="setBrightness"></a>
## *setBrightness*

Sets the TV brightness level for a given picture mode, video format and video source. The brightness value that is to be set should be within the range which is returned by the API [getBrightnessCaps](#getBrightnessCaps).<br><br>The set value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the brightness level is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getBrightnessCaps](#getBrightnessCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getBrightnessCaps](#getBrightnessCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that brightness is changed for all customisable pictures modes that are returned in pictureModeInfo field by [getBrightnessCaps](#getBrightnessCaps). When there is no pictureModeInfo field returned by [getBrightnessCaps](#getBrightnessCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the brightness level is customised for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getBrightnessCaps](#getBrightnessCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If current selected video source is not part of videoSourceInfo field returned by [getBrightnessCaps](#getBrightnessCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that brightness is changed for all customisable video sources that are returned in videoSourceInfo field by [getBrightnessCaps](#getBrightnessCaps). When there is no videoSourceInfo field returned by [getBrightnessCaps](#getBrightnessCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the brightness level is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getBrightnessCaps](#getBrightnessCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If current selected video format is not part of videoSourceInfo field returned by [getBrightnessCaps](#getBrightnessCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that brightness is changed for all customisable video formats that are returned in videoFormatInfo field by [getBrightnessCaps](#getBrightnessCaps). When there is no videoFormatInfo field returned by [getBrightnessCaps](#getBrightnessCaps) then "Global" refers to all video formats returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |
| params.brightness | number | The brightness value to be set. It should be within the range as retrieved from [getBrightnessCaps](#getBrightnessCaps) |

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
    "method": "org.rdk.AVOutput.setBrightness",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "Current"
        ],
        "brightness": 50
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

<a name="setContrast"></a>
## *setContrast*

Sets the TV contrast level for a given picture mode, video format and video source. The contrast value that is to be set should be within the range which is returned by the API [getContrastCaps](#getContrastCaps).<br><br>The set value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the contrast level is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getContrastCaps](#getContrastCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getContrastCaps](#getContrastCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that contrast is changed for all customisable pictures modes that are returned in pictureModeInfo field by [getContrastCaps](#getContrastCaps). When there is no pictureModeInfo field returned by [getContrastCaps](#getContrastCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the contrast level is customised for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getContrastCaps](#getContrastCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If current selected video source is not part of videoSourceInfo field returned by [getContrastCaps](#getContrastCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that contrast is changed for all customisable video sources that are returned in videoSourceInfo field by [getContrastCaps](#getContrastCaps). When there is no videoSourceInfo field returned by [getContrastCaps](#getContrastCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the contrast level is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getContrastCaps](#getContrastCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If current selected video format is not part of videoSourceInfo field returned by [getContrastCaps](#getContrastCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that contrast is changed for all customisable video formats that are returned in videoFormatInfo field by [getContrastCaps](#getContrastCaps). When there is no videoFormatInfo field returned by [getContrastCaps](#getContrastCaps) then "Global" refers to all video formats returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |
| params.contrast | number | The contrast value to be set. It should be within the range as retrieved from [getContrastCaps](#getContrastCaps) |

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
    "method": "org.rdk.AVOutput.setContrast",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "Current"
        ],
        "contrast": 50
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

<a name="setSharpness"></a>
## *setSharpness*

Sets the TV sharpness level for a given picture mode, video format and video source. The sharpness value that is to be set should be within the range which is returned by the API [getSharpnessCaps](#getSharpnessCaps).<br><br>The set value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the sharpness level is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getSharpnessCaps](#getSharpnessCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getSharpnessCaps](#getSharpnessCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that sharpness is changed for all customisable pictures modes that are returned in pictureModeInfo field by [getSharpnessCaps](#getSharpnessCaps). When there is no pictureModeInfo field returned by [getSharpnessCaps](#getSharpnessCaps) then "Global" refers to all picture modes returned by "pictureModes" field of getPictureModeCaps |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the sharpness level is customised for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getSharpnessCaps](#getSharpnessCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If current selected video source is not part of videoSourceInfo field returned by [getSharpnessCaps](#getSharpnessCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that sharpness is changed for all customisable video sources that are returned in videoSourceInfo field by[getSharpnessCaps](#getSharpnessCaps). When there is no videoSourceInfo field returned by [getSharpnessCaps](#getSharpnessCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the sharpness level is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getSharpnessCaps](#getSharpnessCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If current selected video format is not part of videoSourceInfo field returned by [getSharpnessCaps](#getSharpnessCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that sharpness is changed for all customisable video formats that are returned in videoFormatInfo field by [getSharpnessCaps](#getSharpnessCaps). When there is no videoFormatInfo field returned by [getSharpnessCaps](#getSharpnessCaps) then "Global" refers to all video formats returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |
| params.sharpness | number | The sharpness value to be set. It should be within the range as retrieved from [getSharpnessCaps](#getSharpnessCaps) |

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
    "method": "org.rdk.AVOutput.setSharpness",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "Current"
        ],
        "sharpness": 50
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

<a name="setSaturation"></a>
## *setSaturation*

Sets the TV saturation level for a given picture mode, video format and video source. The saturation value that is to be set should be within the range which is returned by the API [getSaturationCaps](#getSaturationCaps).<br><br>The set value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the saturation level is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getSaturationCaps](#getSaturationCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getSaturationCaps](#getSaturationCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that saturation is changed for all customisable pictures modes that are returned in pictureModeInfo field by [getSaturationCaps](#getSaturationCaps). When there is no pictureModeInfo field returned by [getSaturationCaps](#getSaturationCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the saturation level is customised for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getSaturationCaps](#getSaturationCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If current selected video source is not part of videoSourceInfo field returned by [getSaturationCaps](#getSaturationCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that saturation is changed for all customisable video sources that are returned in videoSourceInfo field by [getSaturationCaps](#getSaturationCaps). When there is no videoSourceInfo field returned by [getSaturationCaps](#getSaturationCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the saturation level is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getSaturationCaps](#getSaturationCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If current selected video format is not part of videoSourceInfo field returned by [getSaturationCaps](#getSaturationCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that saturation is changed for all customisable video formats that are returned in videoFormatInfo field by [getSaturationCaps](#getSaturationCaps). When there is no videoFormatInfo field returned by [getSaturationCaps](#getSaturationCaps) then "Global" refers to all video formats returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |
| params.saturation | number | The saturation value to be set. It should be within the range as retrieved from [getSaturationCaps](#getSaturationCaps) |

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
    "method": "org.rdk.AVOutput.setSaturation",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "Current"
        ],
        "saturation": 50
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

<a name="setHue"></a>
## *setHue*

Sets the TV hue level for a given picture mode, video format and video source. The hue value that is to be set should be within the range which is returned by the API [getHueCaps](#getHueCaps).<br><br>The set value will be persisted across reboots<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the hue level is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getHueCaps](#getHueCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getHueCaps](#getHueCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that hue is changed for all customisable pictures modes that are returned in pictureModeInfo field by [getHueCaps](#getHueCaps). When there is no pictureModeInfo field returned by [getHueCaps](#getHueCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the hue level is customised for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getHueCaps](#getHueCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If current selected video source is not part of videoSourceInfo field returned by [getHueCaps](#getHueCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that hue is changed for all customisable video sources that are returned in videoSourceInfo field by [getHueCaps](#getHueCaps). When there is no videoSourceInfo field returned by [getHueCaps](#getHueCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the hue level is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getHueCaps](#getHueCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If current selected video format is not part of videoSourceInfo field returned by [getHueCaps](#getHueCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that hue is changed for all customisable video formats that are returned in videoFormatInfo field by [getHueCaps](#getHueCaps). When there is no videoFormatInfo field returned by [getHueCaps](#getHueCaps) then "Global" refers to all video formats returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |
| params.hue | number | The hue value to be set. It should be within the range as retrieved from [getHueCaps](#getHueCaps) |

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
    "method": "org.rdk.AVOutput.setHue",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "Current"
        ],
        "hue": 50
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

<a name="setColorTemperature"></a>
## *setColorTemperature*

Sets the TV color temperature level for a given picture mode, video format and video source. The color temperature value that is to be set should be within the range which is returned by the API [getColorTemperatureCaps](#getColorTemperatureCaps).<br><br>The set value will be persisted across reboots<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the color temperature level is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getColorTemperatureCaps](#getColorTemperatureCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getColorTemperatureCaps](#getColorTemperatureCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that color temperature is changed for all customisable pictures modes that are returned in pictureModeInfo field by [getColorTemperatureCaps](#getColorTemperatureCaps). When there is no pictureModeInfo field returned by [getColorTemperatureCaps](#getColorTemperatureCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the color temperature level is customised for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getColorTemperatureCaps](#getColorTemperatureCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If current selected video source is not part of videoSourceInfo field returned by [getColorTemperatureCaps](#getColorTemperatureCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that color temperature is changed for all customisable video sources that are returned in videoSourceInfo field by [getColorTemperatureCaps](#getColorTemperatureCaps). When there is no videoSourceInfo field returned by [getColorTemperatureCaps](#getColorTemperatureCaps) then "Global" refers to all video sources returned by "videoSources" field of getVideoSourceCaps |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the color temperature level is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getColorTemperatureCaps](#getColorTemperatureCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If current selected video format is not part of videoSourceInfo field returned by [getColorTemperatureCaps](#getColorTemperatureCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that color temperature is changed for all customisable video formats that are returned in videoFormatInfo field by [getColorTemperatureCaps](#getColorTemperatureCaps). When there is no videoFormatInfo field returned by [getColorTemperatureCaps](#getColorTemperatureCaps) then "Global" refers to all video formats returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |
| params.colorTemperature | string | The color temperature value to be set. It should be within the range as retrieved from [getColorTemperatureCaps](#getColorTemperatureCaps) |

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
    "method": "org.rdk.AVOutput.setColorTemperature",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "Current"
        ],
        "colorTemperature": "Standard"
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

<a name="setCMS"></a>
## *setCMS*

Sets the TV CMS(Color Management System) for a given picture mode, video format and video source for the specified color and component. The CMS value that is to be set should be within the range which is returned by the API [getCMSCaps](#getCMSCaps).<br><br>The set value will be persisted across reboots<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the CMS is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getCMSCaps](#getCMSCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getCMSCaps](#getCMSCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that CMS is changed for all customisable pictures modes that are returned in pictureModeInfo field by [getCMSCaps](#getCMSCaps). When there is no pictureModeInfo field returned by [getCMSCaps](#getCMSCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the CMS is customised for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getCMSCaps](#getCMSCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If current selected video source is not part of videoSourceInfo field returned by [getCMSCaps](#getCMSCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that CMS is changed for all customisable video sources that are returned in videoSourceInfo field by [getCMSCaps](#getCMSCaps). When there is no videoSourceInfo field returned by [getCMSCaps](#getCMSCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the CMS is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getCMSCaps](#getCMSCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If current selected video format is not part of videoSourceInfo field returned by [getCMSCaps](#getCMSCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that CMS is changed for all customisable video formats that are returned in videoFormatInfo field by [getCMSCaps](#getCMSCaps). When there is no videoFormatInfo field returned by [getCMSCaps](#getCMSCaps) then "Global" refers to all video formats returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |
| params.color | string | The color value to be set. It should be within the range as retrieved from colorInfo of [getCMSCaps](#getCMSCaps) |
| params.component | string | The component value to be set. It should be within the range as retrieved from componentInfo of [getCMSCaps](#getCMSCaps) |
| params.level | number | The CMS level to be set. It should be within the range as retrieved from rangeInfo of [getCMSCaps](#getCMSCaps) |

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
    "method": "org.rdk.AVOutput.setCMS",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "Current"
        ],
        "color": "Red",
        "component": "Saturation",
        "level": 30
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

<a name="setLowLatencyState"></a>
## *setLowLatencyState*

Sets the low latency state for a given picture mode, video format and video source. The low latency state that is to be set should be within the range which is returned by the API [getLowLatencyStateCaps](#getLowLatencyStateCaps).<br><br>The set value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the lowLatencyState is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getLowLatencyStateCaps](#getLowLatencyStateCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getLowLatencyStateCaps](#getLowLatencyStateCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that lowLatencyState is changed for all customisable pictures modes that are returned in pictureModeInfo field by [getLowLatencyStateCaps](#getLowLatencyStateCaps). When there is no pictureModeInfo field returned by [getLowLatencyStateCaps](#getLowLatencyStateCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the lowLatencyState is customised for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getLowLatencyStateCaps](#getLowLatencyStateCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If current selected video source is not part of videoSourceInfo field returned by [getLowLatencyStateCaps](#getLowLatencyStateCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that lowLatencyState is changed for all customisable video sources that are returned in videoSourceInfo field by [getLowLatencyStateCaps](#getLowLatencyStateCaps). When there is no videoSourceInfo field returned by [getLowLatencyStateCaps](#getLowLatencyStateCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the lowLatencyState is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getLowLatencyStateCaps](#getLowLatencyStateCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If current selected video format is not part of videoSourceInfo field returned by [getLowLatencyStateCaps](#getLowLatencyStateCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that lowLatencyState is changed for all customisable video formats that are returned in videoFormatInfo field by [getLowLatencyStateCaps](#getLowLatencyStateCaps). When there is no videoFormatInfo field returned by [getLowLatencyStateCaps](#getLowLatencyStateCaps) then "Global" refers to all video formats returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |
| params.LowLatencyState | number | 1 or 0 representing the lowLatencyState value to be set |

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
    "method": "org.rdk.AVOutput.setLowLatencyState",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "Current"
        ],
        "LowLatencyState": 1
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

<a name="setZoomMode"></a>
## *setZoomMode*

Sets the TV zoom mode for a given picture mode, video format and video source. The zoom mode that is to be set should be within the range which is returned by the API [getZoomModeCaps](#getZoomModeCaps).<br><br>The set value will be persisted across reboots.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the zoom mode is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getZoomModeCaps](#getZoomModeCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getZoomModeCaps](#getZoomModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that zoom mode is changed for all customisable pictures modes that are returned in pictureModeInfo field by [getZoomModeCaps](#getZoomModeCaps). When there is no pictureModeInfo field returned by [getZoomModeCaps](#getZoomModeCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the zoom mode is customised for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getZoomModeCaps](#getZoomModeCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If current selected video source is not part of videoSourceInfo field returned by [getZoomModeCaps](#getZoomModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that zoom mode is changed for all customisable video sources that are returned in videoSourceInfo field by [getZoomModeCaps](#getZoomModeCaps). When there is no videoSourceInfo field returned by [getZoomModeCaps](#getZoomModeCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the zoom mode is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getZoomModeCaps](#getZoomModeCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If current selected video format is not part of videoSourceInfo field returned by [getZoomModeCaps](#getZoomModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that zoom mode is changed for all customisable video formats that are returned in videoFormatInfo field by [getZoomModeCaps](#getZoomModeCaps). When there is no videoFormatInfo field returned by [getZoomModeCaps](#getZoomModeCaps) then "Global" refers to all video formats returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |
| params.zoomMode | string | The zoom mode to be set. It should be within the range as retrieved from [getZoomModeCaps](#getZoomModeCaps) |

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
    "method": "org.rdk.AVOutput.setZoomMode",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "Current"
        ],
        "zoomMode": "TV AUTO"
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

<a name="setBacklightDimmingMode"></a>
## *setBacklightDimmingMode*

Sets the TV backlight dimming mode for a given picture mode, video format and video source. The dimming mode value that is to be set should be one of the values which is returned by the API [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps).<br><br>The set value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the backlight dimming mode is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that backlight dimming mode is changed for all customisable pictures modes that are returned in pictureModeInfo field by [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps). When there is no pictureModeInfo field returned by [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the backlight dimming mode is customised for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If current selected video source is not part of videoSourceInfo field returned by [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that backlight dimming mode is changed for all customisable video sources that are returned in videoSourceInfo field by [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps). When there is no videoSourceInfo field returned by [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the backlight dimming mode is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If current selected video format is not part of videoSourceInfo field returned by [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that backlight dimming mode is changed for all customisable video formats that are returned in videoFormatInfo field by [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps). When there is no videoFormatInfo field returned by [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps) then "Global" refers to all video formats returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |
| params.dimmingMode | string | The dimming mode value to be set. It should be one of the options retrieved from [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps) |

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
    "method": "org.rdk.AVOutput.setBacklightDimmingMode",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "Current"
        ],
        "dimmingMode": "Fixed"
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

<a name="fadeDisplay"></a>
## *fadeDisplay*

Fades the backlight between two different backlight percentage values specified in a given duration in ms. The current backlight will be mapped to the "from" percentage and backlight will be faded to the target value (to percentage) on a specified duration in ms.<br><br>The function call is asynchronous and expected to return immediately after initiating the fading process. If there is change in backlight during middle of fade due to [setBacklight](#setBacklight) API call or a change initiated due to picture mode change or primary video source change or primary video format change, then fade params will be recalculated according to new backlight values.<br><br>This method is only available in the RDK TV device profile.<br><br>Fade values are not persisted.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params.from | number | The percentage of current backlight value from where fade starts.<br><br>When omitted 100% is the default assumed |
| params.to | number | The percentage of current backlight value from where fade ends.<br><br>When omitted 0% is the default assumed |
| params.durationMs | number | The duration of Fade in milliseconds. Time duration for which the backlight needs to be faded to reach new value. The durationMs range supported by a platform can be obtained from the [getFadeDisplayCaps](#getFadeDisplayCaps) "durationInfo" property<br><br>When omitted 0 is the default assumed |

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
    "method": "org.rdk.AVOutput.fadeDisplay",
    "params": {
        "from": 100,
        "to": 10,
        "durationMs": 1000
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

<a name="setDolbyVisionMode"></a>
## *setDolbyVisionMode*

Sets the DolbyVision mode for a given picture mode and video format. The DolbyVision mode that is to be set should be within the possible values which are returned by the API [getDolbyVisionModeCaps](#getDolbyVisionModeCaps).<br><br>The set value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the DolbyVision mode is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getDolbyVisionModeCaps](#getDolbyVisionModeCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getDolbyVisionModeCaps](#getDolbyVisionModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that DolbyVision mode is changed for all customisable pictures modes that are returned in pictureModeInfo field by [getDolbyVisionModeCaps](#getDolbyVisionModeCaps). When there is no pictureModeInfo field returned by [getDolbyVisionModeCaps](#getDolbyVisionModeCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the DolbyVision mode is customised for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getDolbyVisionModeCaps](#getDolbyVisionModeCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If current selected video source is not part of videoSourceInfo field returned by [getDolbyVisionModeCaps](#getDolbyVisionModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that DolbyVision dimming mode is changed for all customisable video sources that are returned in videoSourceInfo field by [getDolbyVisionModeCaps](#getDolbyVisionModeCaps). When there is no videoSourceInfo field returned by [getDolbyVisionModeCaps](#getDolbyVisionModeCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params.dolbyVisionMode | string | The DolbyVision mode to be set. It should be within the possible values returned by [getDolbyVisionModeCaps](#getDolbyVisionModeCaps) |

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
    "method": "org.rdk.AVOutput.setDolbyVisionMode",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "dolbyVisionMode": "Dark"
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

<a name="setHDRMode"></a>
## *setHDRMode*

Sets the HDR mode for a given picture mode, video source and video format. The HDR mode that is to be set should be within the possible values which are returned by the API [getHDRModeCaps](#getHDRModeCaps).<br><br>The set value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the HDR mode is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getHDRModeCaps](#getHDRModeCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getHDRModeCaps](#getHDRModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that HDR mode is changed for all customisable pictures modes that are returned in pictureModeInfo field by [getHDRModeCaps](#getHDRModeCaps). When there is no pictureModeInfo field returned by [getHDRModeCaps](#getHDRModeCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the HDR mode is customised for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getHDRModeCaps](#getHDRModeCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If current selected video source is not part of videoSourceInfo field returned by [getHDRModeCaps](#getHDRModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that HDR mode is changed for all customisable video sources that are returned in videoSourceInfo field by [getHDRModeCaps](#getHDRModeCaps). When there is no videoSourceInfo field returned by [getHDRModeCaps](#getHDRModeCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the HDR mode is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getHDRModeCaps](#getHDRModeCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If current selected video format is not part of videoSourceInfo field returned by [getHDRModeCaps](#getHDRModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that HDR mode is changed for all customisable video formats that are returned in videoFormatInfo field by [getHDRModeCaps](#getHDRModeCaps). When there is no videoFormatInfo field returned by [getHDRModeCaps](#getHDRModeCaps) then "Global" refers to all video formats returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |
| params.hdrMode | string | The HDR mode to be set. It should be within the possible values returned by [getHDRModeCaps](#getHDRModeCaps) |

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
    "method": "org.rdk.AVOutput.setHDRMode",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "`HDR10` , `HLG`"
        ],
        "hdrMode": "Dark"
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

<a name="setAutoBacklightMode"></a>
## *setAutoBacklightMode*

Sets the TV auto backlight mode for a given picture mode, video format and video source. The auto backlight mode value that is to be set should be within the range which is returned by the API [getAutoBacklightModeCaps](#getAutoBacklightModeCaps).<br><br>Auto backlight can also refer to Dolby Light Sense in products that are licensed with Dolby Advanced Imaging.<br><br>The set value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the auto backlight mode is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getAutoBacklightModeCaps](#getAutoBacklightModeCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getAutoBacklightModeCaps](#getAutoBacklightModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that auto backlight mode is changed for all customisable pictures modes that are returned in pictureModeInfo field by [getAutoBacklightModeCaps](#getAutoBacklightModeCaps). When there is no pictureModeInfo field returned by [getAutoBacklightModeCaps](#getAutoBacklightModeCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the auto backlight mode is customised for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getAutoBacklightModeCaps](#getAutoBacklightModeCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If current selected video source is not part of videoSourceInfo field returned by [getAutoBacklightModeCaps](#getAutoBacklightModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that auto backlight mode is changed for all customisable video sources that are returned in videoSourceInfo field by [getAutoBacklightModeCaps](#getAutoBacklightModeCaps). When there is no videoSourceInfo field returned by [getAutoBacklightModeCaps](#getAutoBacklightModeCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the auto backlight mode is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getAutoBacklightModeCaps](#getAutoBacklightModeCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If current selected video format is not part of videoSourceInfo field returned by [getAutoBacklightModeCaps](#getAutoBacklightModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that auto backlight mode is changed for all customisable video formats that are returned in videoFormatInfo field by [getAutoBacklightModeCaps](#getAutoBacklightModeCaps). When there is no videoFormatInfo field returned by [getAutoBacklightModeCaps](#getAutoBacklightModeCaps) then "Global" refers to all video formats returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |
| params.mode | string | The auto backlight mode to be set. It should be within the range as retrieved from [getAutoBacklightModeCaps](#getAutoBacklightModeCaps) |

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
    "method": "org.rdk.AVOutput.setAutoBacklightMode",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "Current"
        ],
        "mode": "Manual"
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

<a name="resetPictureMode"></a>
## *resetPictureMode*

Reset picture mode to default for a given video source and video format. The reset value will be persisted across reboots.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source(s) to which the picture mode needs to be associated with.<br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from "videoSourceInfo" field returned by [getPictureModeCaps](#getPictureModeCaps)<br><br>current refers to current video source selected. When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If current selected video source is not part of videoSourceInfo field returned by [getPictureModeCaps](#getPictureModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that picture mode is associated for all customisable video sources that are returned in videoSourceInfo field by [getPictureModeCaps](#getPictureModeCaps). When there is no videoSourceInfo field returned by [getPictureModeCaps](#getPictureModeCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format(s) to which the picture mode needs to be associated with.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from "videoFormatInfo" field returned by [getPictureModeCaps](#getPictureModeCaps)<br><br>current refers to current video format selected. When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If current selected video source is not part of "videoFormatInfo" field returned by [getPictureModeCaps](#getPictureModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that picture mode is associated for all customisable video formats that are returned in "videoFormatInfo" field by [getPictureModeCaps](#getPictureModeCaps). When there is no "videoFormatInfo" field returned by [getPictureModeCaps](#getPictureModeCaps) then "Global" refers to all video formats returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |

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
    "method": "org.rdk.AVOutput.resetPictureMode",
    "params": {
        "videoSource": [
            "`HDMI1`, `Tuner`"
        ],
        "videoFormat": [
            "`SDR`, `HLG`"
        ]
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

<a name="resetBacklight"></a>
## *resetBacklight*

Resets the TV user Backlight to factory default for a given picture mode, video format and video source.<br><br>If the auto backlight control is set to ambient, the TV user backlight change to default value made by this API will not take effect until auto backlight control is switched to manual.<br><br>The reset value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the backlight level is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes that the backlight need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getBacklightCaps](#getBacklightCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getBacklightCaps](#getBacklightCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that backlight is reset for all customisable pictures modes that are returned in pictureModeInfo field by [getBacklightCaps](#getBacklightCaps). When there is no pictureModeInfo field returned by [getBacklightCaps](#getBacklightCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the backlight level is reset for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources that the backlight need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getBacklightCaps](#getBacklightCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If "Current" video source is not part of "videoSourceInfo" field returned by [getBacklightCaps](#getBacklightCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that backlight is reset for all customisable video sources that are returned in "videoSourceInfo" field by [getBacklightCaps](#getBacklightCaps). When there is no "videoSourceInfo" field returned by [getBacklightCaps](#getBacklightCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the backlight level is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats that the backlight need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getBacklightCaps](#getBacklightCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If "Current" video format is not part of "videoFormatInfo" field returned by [getBacklightCaps](#getBacklightCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that backlight is reset for all customisable video formats that are returned in "videoFormatInfo" field by [getBacklightCaps](#getBacklightCaps). When there is no "videoFormatInfo" field returned by [getBacklightCaps](#getBacklightCaps) then "Global" refers to all video sources returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |

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
    "method": "org.rdk.AVOutput.resetBacklight",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "SDR"
        ]
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

<a name="resetBrightness"></a>
## *resetBrightness*

Resets the TV brightness to factory default for a given picture mode, video format and video source.<br><br>The reset value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the brightness level is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes that the brightness need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getBrightnessCaps](#getBrightnessCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getBrightnessCaps](#getBrightnessCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that brightness is reset for all customisable pictures modes that are returned in pictureModeInfo field by [getBrightnessCaps](#getBrightnessCaps). When there is no pictureModeInfo field returned by [getBrightnessCaps](#getBrightnessCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the brightness level is reset for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources that the brightness need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getBrightnessCaps](#getBrightnessCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If "Current" video source is not part of "videoSourceInfo" field returned by [getBrightnessCaps](#getBrightnessCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that brightness is reset for all customisable video sources that are returned in "videoSourceInfo" field by [getBrightnessCaps](#getBrightnessCaps). When there is no "videoSourceInfo" field returned by [getBrightnessCaps](#getBrightnessCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the brightness level is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats that the brightness need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getBrightnessCaps](#getBrightnessCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If "Current" video format is not part of "videoFormatInfo" field returned by [getBrightnessCaps](#getBrightnessCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that brightness is reset for all customisable video formats that are returned in "videoFormatInfo" field by getBrightnessCaps. When there is no "videoFormatInfo" field returned by [getBrightnessCaps](#getBrightnessCaps) then "Global" refers to all video sources returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |

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
    "method": "org.rdk.AVOutput.resetBrightness",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "SDR"
        ]
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

<a name="resetContrast"></a>
## *resetContrast*

Resets the TV contrast to factory default for a given picture mode, video format and video source.<br><br>The reset value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the contrast level is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes that the contrast need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getContrastCaps](#getContrastCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getContrastCaps](#getContrastCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that contrast is reset for all customisable pictures modes that are returned in pictureModeInfo field by [getContrastCaps](#getContrastCaps). When there is no pictureModeInfo field returned by [getContrastCaps](#getContrastCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the contrast level is reset for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources that the contrast need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getContrastCaps](#getContrastCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If "Current" video source is not part of "videoSourceInfo" field returned by [getContrastCaps](#getContrastCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that contrast is reset for all customisable video sources that are returned in "videoSourceInfo" field by [getContrastCaps](#getContrastCaps). When there is no "videoSourceInfo" field returned by [getContrastCaps](#getContrastCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the contrast level is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats that the contrast need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getContrastCaps](#getContrastCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If "Current" video format is not part of "videoFormatInfo" field returned by [getContrastCaps](#getContrastCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that contrast is reset for all customisable video formats that are returned in "videoFormatInfo" field by [getContrastCaps](#getContrastCaps). When there is no "videoFormatInfo" field returned by [getContrastCaps](#getContrastCaps) then "Global" refers to all video sources returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |

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
    "method": "org.rdk.AVOutput.resetContrast",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "SDR"
        ]
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

<a name="resetSharpness"></a>
## *resetSharpness*

Resets the TV sharpness to factory default for a given picture mode, video format and video source.<br><br>The reset value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the sharpness level is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes that the sharpness need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getSharpnessCaps](#getSharpnessCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getSharpnessCaps](#getSharpnessCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that sharpness is reset for all customisable pictures modes that are returned in pictureModeInfo field by [getSharpnessCaps](#getSharpnessCaps). When there is no pictureModeInfo field returned by [getSharpnessCaps](#getSharpnessCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the sharpness level is reset for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources that the sharpness need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getSharpnessCaps](#getSharpnessCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If "Current" video source is not part of "videoSourceInfo" field returned by [getSharpnessCaps](#getSharpnessCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that sharpness is reset for all customisable video sources that are returned in "videoSourceInfo" field by [getSharpnessCaps](#getSharpnessCaps). When there is no "videoSourceInfo" field returned by [getSharpnessCaps](#getSharpnessCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the sharpness level is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats that the sharpness need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getSharpnessCaps](#getSharpnessCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If "Current" video format is not part of "videoFormatInfo" field returned by [getSharpnessCaps](#getSharpnessCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that sharpness is reset for all customisable video formats that are returned in "videoFormatInfo" field by [getSharpnessCaps](#getSharpnessCaps). When there is no "videoFormatInfo" field returned by [getSharpnessCaps](#getSharpnessCaps) then "Global" refers to all video sources returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |

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
    "method": "org.rdk.AVOutput.resetSharpness",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "SDR"
        ]
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

<a name="resetSaturation"></a>
## *resetSaturation*

Resets the TV saturation to factory default for a given picture mode, video format and video source.<br><br>The reset value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the saturation level is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes that the saturation need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getSaturationCaps](#getSaturationCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getSaturationCaps](#getSaturationCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that saturation is reset for all customisable pictures modes that are returned in pictureModeInfo field by [getSaturationCaps](#getSaturationCaps). When there is no pictureModeInfo field returned by [getSaturationCaps](#getSaturationCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the saturation level is reset for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources that the saturation need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getSaturationCaps](#getSaturationCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If "Current" video source is not part of "videoSourceInfo" field returned by [getSaturationCaps](#getSaturationCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that saturation is reset for all customisable video sources that are returned in "videoSourceInfo" field by [getSaturationCaps](#getSaturationCaps). When there is no "videoSourceInfo" field returned by [getSaturationCaps](#getSaturationCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the saturation level is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats that the saturation need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getSaturationCaps](#getSaturationCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If "Current" video format is not part of "videoFormatInfo" field returned by [getSaturationCaps](#getSaturationCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that saturation is reset for all customisable video formats that are returned in "videoFormatInfo" field by [getSaturationCaps](#getSaturationCaps). When there is no "videoFormatInfo" field returned by [getSaturationCaps](#getSaturationCaps) then "Global" refers to all video sources returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |

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
    "method": "org.rdk.AVOutput.resetSaturation",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "SDR"
        ]
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

<a name="resetHue"></a>
## *resetHue*

Resets the TV hue to factory default for a given picture mode, video format and video source.<br><br>The reset value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the hue level is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes that the hue need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getHueCaps](#getHueCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getHueCaps](#getHueCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that hue is reset for all customisable pictures modes that are returned in pictureModeInfo field by [getHueCaps](#getHueCaps). When there is no pictureModeInfo field returned by [getHueCaps](#getHueCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the hue level is reset for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources that the hue need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getHueCaps](#getHueCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If "Current" video source is not part of "videoSourceInfo" field returned by [getHueCaps](#getHueCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that hue is reset for all customisable video sources that are returned in "videoSourceInfo" field by [getHueCaps](#getHueCaps). When there is no "videoSourceInfo" field returned by [getHueCaps](#getHueCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the hue level is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats that the hue need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getHueCaps](#getHueCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If "Current" video format is not part of "videoFormatInfo" field returned by [getHueCaps](#getHueCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that hue is reset for all customisable video formats that are returned in "videoFormatInfo" field by [getHueCaps](#getHueCaps). When there is no "videoFormatInfo" field returned by [getHueCaps](#getHueCaps) then "Global" refers to all video sources returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |

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
    "method": "org.rdk.AVOutput.resetHue",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "SDR"
        ]
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

<a name="resetColorTemperature"></a>
## *resetColorTemperature*

Resets the TV color temperature to factory default for a given picture mode, video format and video source.<br><br>The reset value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the ColorTemperature level is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes that the ColorTemperature need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getColorTemperatureCaps](#getColorTemperatureCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getColorTemperatureCaps](#getColorTemperatureCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that ColorTemperature is reset for all customisable pictures modes that are returned in pictureModeInfo field by [getColorTemperatureCaps](#getColorTemperatureCaps). When there is no pictureModeInfo field returned by [getColorTemperatureCaps](#getColorTemperatureCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the ColorTemperature level is reset for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources that the ColorTemperature need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getColorTemperatureCaps](#getColorTemperatureCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If "Current" video source is not part of "videoSourceInfo" field returned by [getColorTemperatureCaps](#getColorTemperatureCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that ColorTemperature is reset for all customisable video sources that are returned in "videoSourceInfo" field by [getColorTemperatureCaps](#getColorTemperatureCaps). When there is no "videoSourceInfo" field returned by [getColorTemperatureCaps](#getColorTemperatureCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the ColorTemperature level is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats that the ColorTemperature need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getColorTemperatureCaps](#getColorTemperatureCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If "Current" video format is not part of "videoFormatInfo" field returned by [getColorTemperatureCaps](#getColorTemperatureCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that ColorTemperature is reset for all customisable video formats that are returned in "videoFormatInfo" field by [getColorTemperatureCaps](#getColorTemperatureCaps). When there is no "videoFormatInfo" field returned by [getColorTemperatureCaps](#getColorTemperatureCaps) then "Global" refers to all video sources returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |

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
    "method": "org.rdk.AVOutput.resetColorTemperature",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "SDR"
        ]
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

<a name="resetCMS"></a>
## *resetCMS*

Resets the Color Management System (CMS) to factory defaults for a given picture mode, video format and video source for the given component and color values.<br><br>The reset value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the CMS level is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes that the CMS need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getCMSCaps](#getCMSCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getCMSCaps](#getCMSCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that CMS is reset for all customisable pictures modes that are returned in pictureModeInfo field by [getCMSCaps](#getCMSCaps). When there is no pictureModeInfo field returned by [getCMSCaps](#getCMSCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the CMS level is reset for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources that the CMS need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getCMSCaps](#getCMSCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If "Current" video source is not part of "videoSourceInfo" field returned by [getCMSCaps](#getCMSCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that CMS is reset for all customisable video sources that are returned in "videoSourceInfo" field by [getCMSCaps](#getCMSCaps). When there is no "videoSourceInfo" field returned by [getCMSCaps](#getCMSCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the CMS level is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats that the CMS need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getCMSCaps](#getCMSCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If "Current" video format is not part of "videoFormatInfo" field returned by [getCMSCaps](#getCMSCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that CMS is reset for all customisable video formats that are returned in "videoFormatInfo" field by [getCMSCaps](#getCMSCaps). When there is no "videoFormatInfo" field returned by [getCMSCaps](#getCMSCaps) then "Global" refers to all video sources returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |
| params?.color | array | <sup>*(optional)*</sup> The color that the CMS level is reset for. e.g. [ "Global" ] OR "Red", "Green", "Blue", "Cyan", "Yellow", "Magenta".<br><br>The caller can pass "Global" as single value in the array or can pass any number of customisable colors that the CMS shall to reset to defaults.When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getCMSCaps](#getCMSCaps) "colorInfo" property<br><br>"Global" specifies that CMS is reset for all customizable colors that are returned in "colorInfo" field from [getCMSCaps](#getCMSCaps) |
| params?.color[#] | string | <sup>*(optional)*</sup>  |
| params?.component | array | <sup>*(optional)*</sup> The component that the CMS level is reset for. e.g. [ "Global" ] OR "Hue", "Saturation", "Luma".<br><br>The caller can pass "Global" as single value in the array or can pass any number of customisable components that the CMS shall to reset to defaults.When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getCMSCaps](#getCMSCaps) "componentInfo" property<br><br>"Global" specifies that CMS is reset for all customizable components that are returned in "componentInfo" field from [getCMSCaps](#getCMSCaps) |
| params?.component[#] | string | <sup>*(optional)*</sup>  |

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
    "method": "org.rdk.AVOutput.resetCMS",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "SDR"
        ],
        "color": [
            "Red"
        ],
        "component": [
            "`Saturation`, `Hue`"
        ]
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

<a name="resetLowLatencyState"></a>
## *resetLowLatencyState*

Resets the low latency state to factory default for a given picture mode, video format and video source.<br><br>The reset value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the low latency state is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes that the low latency state need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getLowLatencyStateCaps](#getLowLatencyStateCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getLowLatencyStateCaps](#getLowLatencyStateCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that low latency state is reset for all customisable pictures modes that are returned in pictureModeInfo field by [getLowLatencyStateCaps](#getLowLatencyStateCaps). When there is no pictureModeInfo field returned by [getLowLatencyStateCaps](#getLowLatencyStateCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the low latency state is reset for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources that the low latency state need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getLowLatencyStateCaps](#getLowLatencyStateCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If "Current" video source is not part of "videoSourceInfo" field returned by [getLowLatencyStateCaps](#getLowLatencyStateCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that low latency state is reset for all customisable video sources that are returned in "videoSourceInfo" field by [getLowLatencyStateCaps](#getLowLatencyStateCaps). When there is no "videoSourceInfo" field returned by [getLowLatencyStateCaps](#getLowLatencyStateCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the low latency state is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats that the low latency state need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getLowLatencyStateCaps](#getLowLatencyStateCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If "Current" video format is not part of "videoFormatInfo" field returned by [getLowLatencyStateCaps](#getLowLatencyStateCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that low latency state is reset for all customisable video formats that are returned in "videoFormatInfo" field by [getLowLatencyStateCaps](#getLowLatencyStateCaps). When there is no "videoFormatInfo" field returned by [getLowLatencyStateCaps](#getLowLatencyStateCaps) then "Global" refers to all video sources returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |

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
    "method": "org.rdk.AVOutput.resetLowLatencyState",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "SDR"
        ]
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

<a name="resetZoomMode"></a>
## *resetZoomMode*

Resets the TV zoom mode to the factory default.<br><br>The reset value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the ZoomMode is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes that the ZoomMode need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getZoomModeCaps](#getZoomModeCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getZoomModeCaps](#getZoomModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that ZoomMode is reset for all customisable pictures modes that are returned in pictureModeInfo field by [getZoomModeCaps](#getZoomModeCaps). When there is no pictureModeInfo field returned by [getZoomModeCaps](#getZoomModeCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the ZoomMode is reset for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources that the ZoomMode need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getZoomModeCaps](#getZoomModeCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If "Current" video source is not part of "videoSourceInfo" field returned by [getZoomModeCaps](#getZoomModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that ZoomMode is reset for all customisable video sources that are returned in "videoSourceInfo" field by [getZoomModeCaps](#getZoomModeCaps). When there is no "videoSourceInfo" field returned by [getZoomModeCaps](#getZoomModeCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the ZoomMode is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats that the ZoomMode need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getZoomModeCaps](#getZoomModeCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If "Current" video format is not part of "videoFormatInfo" field returned by [getZoomModeCaps](#getZoomModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that ZoomMode is reset for all customisable video formats that are returned in "videoFormatInfo" field by [getZoomModeCaps](#getZoomModeCaps). When there is no "videoFormatInfo" field returned by [getZoomModeCaps](#getZoomModeCaps) then "Global" refers to all video sources returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |

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
    "method": "org.rdk.AVOutput.resetZoomMode",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "SDR"
        ]
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

<a name="resetBacklightDimmingMode"></a>
## *resetBacklightDimmingMode*

Resets the TV backlight dimming mode to factory default for a given picture mode, video format and video source.<br><br>The reset value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the Backlight dimming mode is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes that the dimming mode need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that dimming mode is reset for all customisable pictures modes that are returned in pictureModeInfo field by [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps). When there is no pictureModeInfo field returned by [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the Backlight dimming is reset for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources that the dimming mode need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If "Current" video source is not part of "videoSourceInfo" field returned by [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that dimming mode is reset for all customisable video sources that are returned in "videoSourceInfo" field by [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps). When there is no "videoSourceInfo" field returned by [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the Backlight dimming is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats that the dimming mode need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If "Current" video format is not part of "videoFormatInfo" field returned by [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that dimming mode is reset for all customisable video formats that are returned in "videoFormatInfo" field by [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps). When there is no "videoFormatInfo" field returned by [getBacklightDimmingModeCaps](#getBacklightDimmingModeCaps) then "Global" refers to all video sources returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |

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
    "method": "org.rdk.AVOutput.resetBacklightDimmingMode",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "SDR"
        ]
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

<a name="resetDolbyVisionMode"></a>
## *resetDolbyVisionMode*

Resets the DolbyVision mode to factory default for a given picture mode and video format.<br><br>The reset value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the DolbyVision mode is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes that the DolbyVision mode need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getDolbyVisionModeCaps](#getDolbyVisionModeCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getDolbyVisionModeCaps](#getDolbyVisionModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that DolbyVision mode is reset for all customisable pictures modes that are returned in pictureModeInfo field by [getDolbyVisionModeCaps](#getDolbyVisionModeCaps). When there is no pictureModeInfo field returned by [getDolbyVisionModeCaps](#getDolbyVisionModeCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the DolbyVision mode is reset for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources that the DolbyVision mode need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getDolbyVisionModeCaps](#getDolbyVisionModeCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If "Current" video source is not part of "videoSourceInfo" field returned by [getDolbyVisionModeCaps](#getDolbyVisionModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that DolbyVision mode is reset for all customisable video sources that are returned in "videoSourceInfo" field by [getDolbyVisionModeCaps](#getDolbyVisionModeCaps). When there is no "videoSourceInfo" field returned by [getDolbyVisionModeCaps](#getDolbyVisionModeCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |

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
    "method": "org.rdk.AVOutput.resetDolbyVisionMode",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ]
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

<a name="resetHDRMode"></a>
## *resetHDRMode*

Resets the HDR mode to factory default for a given picture mode, video source and video format.<br><br>The reset value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the HDR mode is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes that the HDR mode need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getHDRModeCaps](#getHDRModeCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getHDRModeCaps](#getHDRModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that HDR mode is reset for all customisable pictures modes that are returned in pictureModeInfo field by [getHDRModeCaps](#getHDRModeCaps). When there is no pictureModeInfo field returned by [getHDRModeCaps](#getHDRModeCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the HDR mode is reset for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources that the HDR mode need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getHDRModeCaps](#getHDRModeCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If "Current" video source is not part of "videoSourceInfo" field returned by [getHDRModeCaps](#getHDRModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that HDR mode is reset for all customisable video sources that are returned in "videoSourceInfo" field by [getHDRModeCaps](#getHDRModeCaps). When there is no "videoSourceInfo" field returned by [getHDRModeCaps](#getHDRModeCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) except Tuner and Composite1 |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the HDR mode is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "DV", "HLG"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats that the HDR mode need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getHDRModeCaps](#getHDRModeCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default and [setHDRMode()](#setHDRMode) will fail. If "Current" video format is not part of "videoFormatInfo" field returned by [getHDRModeCaps](#getHDRModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that HDR mode is reset for all customisable video formats that are returned in "videoFormatInfo" field by [getHDRModeCaps](#getHDRModeCaps). When there is no "videoFormatInfo" field returned by [getHDRModeCaps](#getHDRModeCaps) then "Global" refers to all video sources returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) except SDR |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |

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
    "method": "org.rdk.AVOutput.resetHDRMode",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "HDR10"
        ]
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

<a name="resetAutoBacklightMode"></a>
## *resetAutoBacklightMode*

Resets the TV auto backlight mode to factory default for a given picture mode, video format and video source.<br><br>Auto backlight can also refer to Dolby Light Sense in products that are licensed with Dolby Advanced Imaging.<br><br>The reset value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the auto backlight mode is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes that the auto backlight mode need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getAutoBacklightModeCaps](#getAutoBacklightModeCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [getAutoBacklightModeCaps](#getAutoBacklightModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that auto backlight mode is reset for all customisable pictures modes that are returned in pictureModeInfo field by [getAutoBacklightModeCaps](#getAutoBacklightModeCaps). When there is no pictureModeInfo field returned by [getAutoBacklightModeCaps](#getAutoBacklightModeCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the auto backlight mode is reset for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources that the auto backlight mode need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getAutoBacklightModeCaps](#getAutoBacklightModeCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If "Current" video source is not part of "videoSourceInfo" field returned by [getAutoBacklightModeCaps](#getAutoBacklightModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that auto backlight mode is reset for all customisable video sources that are returned in "videoSourceInfo" field by [getAutoBacklightModeCaps](#getAutoBacklightModeCaps). When there is no "videoSourceInfo" field returned by [getAutoBacklightModeCaps](#getAutoBacklightModeCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the auto backlight mode is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats that the auto backlight mode need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [getAutoBacklightModeCaps](#getAutoBacklightModeCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If "Current" video format is not part of "videoFormatInfo" field returned by [getAutoBacklightModeCaps](#getAutoBacklightModeCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that auto backlight mode is reset for all customisable video formats that are returned in "videoFormatInfo" field by [getAutoBacklightModeCaps](#getAutoBacklightModeCaps). When there is no "videoFormatInfo" field returned by [getAutoBacklightModeCaps](#getAutoBacklightModeCaps) then "Global" refers to all video sources returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |

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
    "method": "org.rdk.AVOutput.resetAutoBacklightMode",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "SDR"
        ]
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

<a name="setWBMode"></a>
## *setWBMode*

This function will reconfigure the PQ pipeline for white balance calibration by disabling Local Dimming, Local Contrast, and Dynamic Contrast.<br><br>setWBMode(true) must be called by the app before  [set2PointWB](#set2PointWB).<br><br>setWBMode(false) must be called by the app after [set2PointWB](#set2PointWB).<br><br>setWBMode() values are not persisted.<br><br>Note: When enabled all set/reset calls will return failure except the white balance methods.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params.wbMode | boolean | true: Enable white balance calibration mode.<br>false: Disable white balance calibration mode |

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
    "method": "org.rdk.AVOutput.setWBMode",
    "params": {
        "wbMode": true
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

<a name="getWBMode"></a>
## *getWBMode*

Get the white balance mode.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.wbMode | boolean | true : If white balance mode calibration mode is enabled.<br>false : If white balance mode calibration mode is disabled |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.getWBMode",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "wbMode": true,
        "success": true
    }
}
```

<a name="get2PointWB"></a>
## *get2PointWB*

Gets the TV 2 point white balance control value for a given picture mode, video format, video source and color temperature.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | string | <sup>*(optional)*</sup> The picture mode that the white balance should be fetched for.<br><br>e.g. "Current", "Standard", "Sports", "EnergySaving".<br><br>"Current" - the current selected picture mode.<br><br>Other valid values for a platform can be obtained from options field of [getPictureModeCaps](#getPictureModeCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoSource | string | <sup>*(optional)*</sup> The video source that the white balance should be fetched for.<br><br>e.g. "Current", "Composite1", "HDMI1", "HDMI2", "HDMI3", "IP", "Tuner".<br><br>"Current" - the current primary active source. When there is no active video playback or input port selected, "Current" refers to "IP" video source as default.<br><br>Other valid values for a platform can be obtained from options field of [getVideoSourceCaps](#getVideoSourceCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params?.videoFormat | string | <sup>*(optional)*</sup> The video format that the white balance should be fetched for.<br><br>e.g. "Current", "SDR", "HDR10", "HLG", "DV".<br><br>"Current" - the current format of the current primary active source. When there is no active video playback or input port selected, "Current" refers to "SDR" video format as default.<br><br>Other valid values for a platform can be obtained from options field of [getVideoFormatCaps](#getVideoFormatCaps).<br><br>The caller can pass a single valid value for this parameter. If omitted the default value assumed is "Current" |
| params.colorTemperature | string | The colorTemperature that the white balance values should be fetched for.<br><br>e.g. "Standard" , "Warm" , "Cold" , "UserDefined".<br><br>Other valid values for a platform can be obtained from colorTemperatureInfo field of [get2PointWBCaps](#get2PointWBCaps).<br><br>The caller can pass a single valid value for this parameter.  |
| params.color | string | The color that the white balance values should be fetched for.<br><br>e.g. "Red" , "Green" , "Blue".<br><br>Other valid values for a platform can be obtained from colorInfo field of [get2PointWBCaps](#get2PointWBCaps).<br><br>The caller can pass a single valid value for this parameter.  |
| params.control | string | The control value that the white balance should be fetched for.<br><br>e.g. "Gain" , "Offset".<br><br>Other valid values for a platform can be obtained from controlInfo field of [get2PointWBCaps](#get2PointWBCaps).<br><br>The caller can pass a single valid value for this parameter.  |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.level | number | The whiteBalance in the range from - to inclusive |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.get2PointWB",
    "params": {
        "pictureMode": "Custom",
        "videoSource": "Current",
        "videoFormat": "SDR",
        "colorTemperature": "UserDefined",
        "color": "Red",
        "control": "Gain"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "level": 1000,
        "success": true
    }
}
```

<a name="get2PointWBCaps"></a>
## *get2PointWBCaps*

Gets the capabilities and boundary range for TV 2 point white balance. It returns the range of TV 2pt white balance value that the user can set and the list of possible picture modes, video sources, video formats, color Temperature, control and color that the white balance may be customized for.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.platformSupport | boolean | true: The platform supports white balance adjustment.<br>false: The platform does not supports white balance adjustment |
| result.gainInfo | object | Range of values for the white balance Gain |
| result.gainInfo.from | number | The lower-inclusive range value for white balance Gain |
| result.gainInfo.to | number | The higher-inclusive range value for white balance Gain |
| result.offsetInfo | object | Range of value for white balance offset |
| result.offsetInfo.from | number | The lower-inclusive range value for white balance offset |
| result.offsetInfo.to | number | The lower-inclusive range value for white balance offset |
| result?.pictureModeInfo | array | <sup>*(optional)*</sup> Array of picture modes that the white balance could be customized for. These are few examples of picture modes available in this array: ["Standard", "Vivid","Custom", "Movie", "Sports"].This API will return the entire list of picture modes returned by [getPictureModeCaps](#getPictureModeCaps) or a subset of it.<br><br>If color temperature cannot be customized for picture modes, then this property is not returned |
| result?.pictureModeInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoSourceInfo | array | <sup>*(optional)*</sup> Array of video sources that the white balance could be customized for. These are few examples of video sources available in this array: ["Composite1", "HDMI1", "HDMI2", "HDMI3", "IP" ]. This API will return the entire list of video sources returned by [getVideoSourceCaps](#getVideoSourceCaps), or a subset of it.<br><br>If WhiteBalance cannot be customized for video sources, then this property is not returned |
| result?.videoSourceInfo[#] | string | <sup>*(optional)*</sup>  |
| result?.videoFormatInfo | array | <sup>*(optional)*</sup> Array of video formats that the white balance could be customized for. These are few examples of formats available in this array: ["SDR","HLG","HDR10","DV"]. This API will return the entire list of video formats returned by [getVideoFormatCaps](#getVideoFormatCaps), or a subset of it.<br><br>If white balance cannot be customized for video format, then this property is not returned |
| result?.videoFormatInfo[#] | string | <sup>*(optional)*</sup>  |
| result.colorInfo | array | Array of color values that the white balance could be customized for. These are few examples of formats available in this array: ["Red", "Green", "Blue"] |
| result.colorInfo[#] | string |  |
| result.controlInfo | array | Array of control values that the white balance could be customized for. These are few examples of formats available in this array: ["Gain", "Offset"] |
| result.controlInfo[#] | string |  |
| result.colorTemperatureInfo | array | Array of colorTemperature values that the white balance could be customized for. These are few examples of formats available in this array: ["Standard","Warm","Cold","UserDefined"] |
| result.colorTemperatureInfo[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVOutput.get2PointWBCaps",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "platformSupport": true,
        "gainInfo": {
            "from": 0,
            "to": 2047
        },
        "offsetInfo": {
            "from": -1024,
            "to": 1023
        },
        "pictureModeInfo": [
            "`Custom`, `Movie`, `Game`, `Sports`, `Standard`"
        ],
        "videoSourceInfo": [
            "`HDMI1`,`HDMI2`, `HDMI3`, `IP`"
        ],
        "videoFormatInfo": [
            "`SDR`,`HLG`,`HDR10`,`DV`"
        ],
        "colorInfo": [
            "`Red`, `Green`, `Blue`"
        ],
        "controlInfo": [
            "`Gain`, `Offset`"
        ],
        "colorTemperatureInfo": [
            "`Standard`,`Warm`,`Cold`,`UserDefined`"
        ],
        "success": true
    }
}
```

<a name="set2PointWB"></a>
## *set2PointWB*

Sets the TV 2 point white balance for a specified videoSource, colorTemperature, color and control value. The white balance value that is to be set should be within the range which is returned by the API [get2PointWBCaps](#get2PointWBCaps).<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the white balance is customized for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [get2PointWBCaps](#get2PointWBCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [get2PointWBCaps](#get2PointWBCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that white balance is changed for all customisable pictures modes that are returned in pictureModeInfo field by [get2PointWBCaps](#get2PointWBCaps). When there is no pictureModeInfo field returned by [get2PointWBCaps](#get2PointWBCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the white balance level is customised for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [get2PointWBCaps](#get2PointWBCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If current selected video source is not part of videoSourceInfo field returned by [get2PointWBCaps](#get2PointWBCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that white balance is changed for all customisable video sources that are returned in videoSourceInfo field by [get2PointWBCaps](#get2PointWBCaps). When there is no videoSourceInfo field returned by [get2PointWBCaps](#get2PointWBCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the white balance level is customised for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [get2PointWBCaps](#get2PointWBCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If current selected video format is not part of videoSourceInfo field returned by [get2PointWBCaps](#get2PointWBCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that white balance is changed for all customisable video formats that are returned in videoFormatInfo field by [get2PointWBCaps](#get2PointWBCaps). When there is no videoFormatInfo field returned by [get2PointWBCaps](#get2PointWBCaps) then "Global" refers to all video formats returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |
| params.color | string | The color value for WhiteBalance customization. It should be within the range as retrieved from colorInfo of [get2PointWBCaps](#get2PointWBCaps) |
| params.control | string | The control value for WhiteBalance customization. It should be within the range as retrieved from controlInfo of [get2PointWBCaps](#get2PointWBCaps) |
| params.colorTemperature | string | The colorTemperature value for WhiteBalance customization. It should be within the range as retrieved from colorTemperatureInfo of [get2PointWBCaps](#get2PointWBCaps) |
| params.level | number | The WB to be set. It should be within the range as retrieved from [get2PointWBCaps](#get2PointWBCaps) |

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
    "method": "org.rdk.AVOutput.set2PointWB",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "Current"
        ],
        "color": "Red",
        "control": "Gain",
        "colorTemperature": "UserDefined",
        "level": 1000
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

<a name="reset2PointWB"></a>
## *reset2PointWB*

Resets the TV 2 point white balance to factory default for a given picture mode, video format and video source.<br><br>The reset value will be persisted across reboots.<br><br>This method is only available in the RDK TV device profile.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |
| params?.pictureMode | array | <sup>*(optional)*</sup> The picture mode that the white balance is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "Standard", "Sports", "EnergySaving" ]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable picture modes that the white balance need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [get2PointWBCaps](#get2PointWBCaps) "pictureModeInfo" property<br><br>"Current" specifies the current selected picture mode which can be obtained from [getPictureMode](#getPictureMode). If current selected picture mode is not part of pictureModeInfo field returned by [get2PointWBCaps](#get2PointWBCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that white balance is reset for all customisable pictures modes that are returned in pictureModeInfo field by [get2PointWBCaps](#get2PointWBCaps). When there is no pictureModeInfo field returned by [get2PointWBCaps](#get2PointWBCaps) then "Global" refers to all picture modes returned by "pictureModes" field of [getPictureModeCaps](#getPictureModeCaps) |
| params?.pictureMode[#] | string | <sup>*(optional)*</sup>  |
| params?.videoSource | array | <sup>*(optional)*</sup> The video source that the white balance is reset for.<br><br>e.g. [ "Global" ] OR [ "Current", "HDMI1", "Composite1", "IP"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video sources that the white balance need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [get2PointWBCaps](#get2PointWBCaps) "videoSourceInfo" property<br><br>"Current" specifies the current selected video source which can be obtained from [getVideoSource](#getVideoSource). When there is no active video playback or input port selected, usage of "Current" will refer to "IP" video source as default. If "Current" video source is not part of "videoSourceInfo" field returned by [get2PointWBCaps](#get2PointWBCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that white balance is reset for all customisable video sources that are returned in "videoSourceInfo" field by [get2PointWBCaps](#get2PointWBCaps). When there is no "videoSourceInfo" field returned by [get2PointWBCaps](#get2PointWBCaps) then "Global" refers to all video sources returned by "videoSources" field of [getVideoSourceCaps](#getVideoSourceCaps) |
| params?.videoSource[#] | string | <sup>*(optional)*</sup>  |
| params?.videoFormat | array | <sup>*(optional)*</sup> The video format that the white balance is reset for.<br><br>e.g. [ "Global" ] OR ["Current", "HDR10", "SDR", "DV"]<br><br>The caller can either pass "Current" or "Global" as single value in the array or can pass any number of customisable video formats that the white balance need to reset to default. When omitted "Global" is the default assumed.<br><br>Other valid values for a platform can be obtained from the [get2PointWBCaps](#get2PointWBCaps) "videoFormatInfo" property<br><br>"Current" specifies the current selected video format which can be obtained from [getVideoFormat](#getVideoFormat). When there is no active video playback or input port selected, usage of "Current" will refer to "SDR" video format as default. If "Current" video format is not part of "videoFormatInfo" field returned by [get2PointWBCaps](#get2PointWBCaps), then it is not customizable and cannot be used.<br><br>"Global" specifies that white balance is reset for all customisable video formats that are returned in "videoFormatInfo" field by [get2PointWBCaps](#get2PointWBCaps). When there is no "videoFormatInfo" field returned by [get2PointWBCaps](#get2PointWBCaps) then "Global" refers to all video sources returned by "videoFormats" field of [getVideoFormatCaps](#getVideoFormatCaps) |
| params?.videoFormat[#] | string | <sup>*(optional)*</sup>  |

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
    "method": "org.rdk.AVOutput.reset2PointWB",
    "params": {
        "pictureMode": [
            "`Custom`, `Game`"
        ],
        "videoSource": [
            "Global"
        ],
        "videoFormat": [
            "SDR"
        ]
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

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.AVOutput plugin:

AVOutput interface events:

| Event | Description |
| :-------- | :-------- |
| [onVideoSourceChanged](#onVideoSourceChanged) | Event notification when there is a change in video source of the video being played |
| [onVideoFormatChanged](#onVideoFormatChanged) | Event notification when there is a change in video format of the video being played |
| [onVideoResolutionChanged](#onVideoResolutionChanged) | Event notification when there is a change in video resolution of the current video played out on HDMI and Composite1 video sources when they are selected |
| [onVideoFrameRateChanged](#onVideoFrameRateChanged) | Event notification when there is a change in video frame rate of the video being played |
| [onVideoContentChanged](#onVideoContentChanged) | Event notification when there is any form of video content changes were detected |


<a name="onVideoSourceChanged"></a>
## *onVideoSourceChanged*

Event notification when there is a change in video source of the video being played.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.currentVideoSource | string | Video source of the video currently being played.<br><br>e.g. "HDMI1", "HDMI2", "HDMI4", "IP", "Tuner", "Composite1", "None"<br><br>None - Returned when no active video is played.<br><br>For full list of supported video formats refer [getVideoSourceCaps](#getVideoSourceCaps) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onVideoSourceChanged",
    "params": {
        "currentVideoSource": "HDMI1"
    }
}
```

<a name="onVideoFormatChanged"></a>
## *onVideoFormatChanged*

Event notification when there is a change in video format of the video being played.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.currentVideoFormat | string | Video format of the video currently being played.<br><br>e.g. "SDR","HLG","HDR10","DV","None"<br><br>None - Returned when no active video is played.<br><br>For full list of supported video formats refer [getVideoFormatCaps](#getVideoFormatCaps) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onVideoFormatChanged",
    "params": {
        "currentVideoFormat": "DV"
    }
}
```

<a name="onVideoResolutionChanged"></a>
## *onVideoResolutionChanged*

Event notification when there is a change in video resolution of the current video played out on HDMI and Composite1 video sources when they are selected.<br><br>This method is only available in the RDK TV device profile.<br><br>NOTE: At present this event is supported only for HDMI and Composite1 video sources only.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.currentVideoResolution | string | Video resolution of the video currently being played.<br><br>e.g.<br>"None","720\*240p","720\*288p","720\*480i","720\*480p","960\*540p","720\*576i","720\*576p","800\*600p","1280\*720p","1280\*768p","1280\*960p","1280\*1024p","1920\*1080i","2560\*1080p","1920\*1200p","2560\*1440p","4096\*2160p"<br><br>None - Returned when no active video is played.<br><br>For full list of supported video formats refer [getVideoResolutionCaps](#getVideoResolutionCaps) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onVideoResolutionChanged",
    "params": {
        "currentVideoResolution": "1280*720p"
    }
}
```

<a name="onVideoFrameRateChanged"></a>
## *onVideoFrameRateChanged*

Event notification when there is a change in video frame rate of the video being played.<br><br>This method is only available in the RDK TV device profile.<br><br>NOTE: At present this event is supported only for HDMI and Composite1 video sources only.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.currentVideoFrameRate | number | Video frame rate of the video currently being played.<br><br>e.g. 24,25,30,50,60,23.98,29.97,59.94, None<br><br>None - Returned when no active video is played.<br><br>For full list of supported video frame rates refer [getVideoFrameRateCaps](#getVideoFrameRateCaps) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onVideoFrameRateChanged",
    "params": {
        "currentVideoFrameRate": 50
    }
}
```

<a name="onVideoContentChanged"></a>
## *onVideoContentChanged*

Event notification when there is any form of video content changes were detected. An example of such a change is detection of FilmMaker mode.<br><br>This method is only available in the RDK TV device profile.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.filmMakerMode | boolean | Possible values are true and false<br><br>true - Film Maker mode active<br>false -Film Maker mode not active |
| params?.filmMakerModeSources | array | <sup>*(optional)*</sup> A list of sources in which the Film Maker mode event occurred.<br><br>If there "params.filmMakerMode" is false this parmaeter is not passed.<br><br>Possible video sources for film maker mode are HDMI sources (as returned by [getVideoSourceCaps](#getVideoSourceCaps)) and IP source |
| params?.filmMakerModeSources[#] | string | <sup>*(optional)*</sup>  |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onVideoContentChanged",
    "params": {
        "filmMakerMode": true,
        "filmMakerModeSources": [
            "`HDMI1`, `IP`"
        ]
    }
}
```

