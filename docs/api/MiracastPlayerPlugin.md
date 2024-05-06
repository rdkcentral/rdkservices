<!-- Generated automatically, DO NOT EDIT! -->
<a name="MiracastPlayer_Plugin"></a>
# MiracastPlayer Plugin

**Version: [1.0.6](https://github.com/rdkcentral/rdkservices/blob/main/Miracast/CHANGELOG.md)**

A org.rdk.MiracastPlayer plugin for Thunder framework.

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

The `MiracastPlayer` plugin will do RTSP message exchange for Source and Sink Device capabilities and Session Management. Afterwards, it will start streaming for screen mirroring.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.MiracastPlayer*) |
| classname | string | Class name: *org.rdk.MiracastPlayer* |
| locator | string | Library name: *libWPEFrameworkMiracastPlayer.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.MiracastPlayer plugin:

MiracastPlayer interface methods:

| Method | Description |
| :-------- | :-------- |
| [playRequest](#playRequest) | To set the Miracast Player State to Play after the Miracast session like RTSP communication and GStreamer Playback |
| [stopRequest](#stopRequest) | To stop the Miracast Player to tear down the RTSP communication, stop/close the GStreamer pipeline, clean up, and reset the player state |
| [setPlayerState](#setPlayerState) | To set the Miracast Player to STOP/PLAY/PAUSE |
| [setVideoRectangle](#setVideoRectangle) | Set the Video Rectangle |
| [setRTSPWaitTimeout](#setRTSPWaitTimeout) | Set the RTSP Socket Receive timeout for M1-M7 messages |
| [setLogging](#setLogging) | Enable/Disable/Reduce the Logging level for Miracast |
| [setVideoFormats](#setVideoFormats) | Set the Video formats for RTSP capability negotiation |
| [setAudioFormats](#setAudioFormats) | Set the Audio formats for RTSP capability negotiation |


<a name="playRequest"></a>
## *playRequest*

To set the Miracast Player State to Play after the Miracast session like RTSP communication and GStreamer Playback.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.device_parameters | object | Parameters about Source and Sink Device |
| params.device_parameters.source_dev_ip | string | Source Device IP address |
| params.device_parameters.source_dev_mac | string | Mac id of the source device |
| params.device_parameters.source_dev_name | string | Name of the source device |
| params.device_parameters.sink_dev_ip | string | IP address of the Local/Sink device |
| params.video_rectangle | object | Video streaming rectangle |
| params.video_rectangle.X | number | Start X of the Video rectangle |
| params.video_rectangle.Y | number | Start Y of the Video rectangle |
| params.video_rectangle.W | number | Width of the Video rectangle |
| params.video_rectangle.H | number | Height of the Video rectangle |

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
    "method": "org.rdk.MiracastPlayer.playRequest",
    "params": {
        "device_parameters": {
            "source_dev_ip": "xx.xx.xx.xx",
            "source_dev_mac": "XX:XX:XX:XX:XX:XX",
            "source_dev_name": "Manufacturer WiFi-Direct Name",
            "sink_dev_ip": "xx.xx.xx.xx"
        },
        "video_rectangle": {
            "X": 0,
            "Y": 0,
            "W": 1920,
            "H": 1080
        }
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

<a name="stopRequest"></a>
## *stopRequest*

To stop the Miracast Player to tear down the RTSP communication, stop/close the GStreamer pipeline, clean up, and reset the player state.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.mac | string | Mac id of the source device |
| params.name | string | Name of the source device |
| params.reason_code | number | player stop Reason codes (must be one of the following: *MIRACAST_PLAYER_APP_REQ_TO_STOP_ON_EXIT = 301*, *MIRACAST_PLAYER_APP_REQ_TO_STOP_ON_NEW_CONNECTION = 302*) |

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
    "method": "org.rdk.MiracastPlayer.stopRequest",
    "params": {
        "mac": "XX:XX:XX:XX:XX:XX",
        "name": "Manufacturer WiFi-Direct Name",
        "reason_code": 301
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

<a name="setPlayerState"></a>
## *setPlayerState*

To set the Miracast Player to STOP/PLAY/PAUSE.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.state | string | Video streaming rectangle |

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
    "method": "org.rdk.MiracastPlayer.setPlayerState",
    "params": {
        "state": "STOP|PLAY|PAUSE"
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

<a name="setVideoRectangle"></a>
## *setVideoRectangle*

Set the Video Rectangle.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.X | number | Start X of the Video rectangle |
| params.Y | number | Start Y of the Video rectangle |
| params.W | number | Width of the Video rectangle |
| params.H | number | Height of the Video rectangle |

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
    "method": "org.rdk.MiracastPlayer.setVideoRectangle",
    "params": {
        "X": 0,
        "Y": 0,
        "W": 1920,
        "H": 1080
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

<a name="setRTSPWaitTimeout"></a>
## *setRTSPWaitTimeout*

Set the RTSP Socket Receive timeout for M1-M7 messages.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.Request | number | Set RTSP Socket Receive timeout for WFD Sink RTSP request messages |
| params.Response | number | Set RTSP Socket Receive timeout for WFD Sink RTSP response messages |

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
    "method": "org.rdk.MiracastPlayer.setRTSPWaitTimeout",
    "params": {
        "Request": 6,
        "Response": 5
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

<a name="setLogging"></a>
## *setLogging*

Enable/Disable/Reduce the Logging level for Miracast.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.level | string | Set the log level for Miracast player plugin |
| params?.separate_logger | object | <sup>*(optional)*</sup> Route the Miracast player to a separate file if required |
| params?.separate_logger.logfilename | string | Name of a separate logging filename. Logfile will be created under /opt/logs/ |
| params?.separate_logger.status | string | Enable/Disable the separate logging |

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
    "method": "org.rdk.MiracastPlayer.setLogging",
    "params": {
        "level": "FATAL|ERROR|WARNING|INFO|VERBOSE|TRACE",
        "separate_logger": {
            "logfilename": "sample.log",
            "status": "ENABLE|DISABLE"
        }
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

<a name="setVideoFormats"></a>
## *setVideoFormats*

Set the Video formats for RTSP capability negotiation.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.native | number | WFD Display Native Resolution Refresh Rate (must be one of the following: *RTSP_NATIVE_NO_RESOLUTION_SUPPORTED = 0x00*, *RTSP_NATIVE_CEA_RESOLUTION_SUPPORTED = 0x01*, *RTSP_NATIVE_VESA_RESOLUTION_SUPPORTED = 0x02*, *RTSP_NATIVE_HH_RESOLUTION_SUPPORTED = 0x04*) |
| params.display_mode_supported | boolean | WFD prefered display mode |
| params.h264_codecs | object | WFD Sink's h264_codecs information |
| params.h264_codecs.profile | number | This profile bitmap is also used by the wfd-video-formats parameter (must be one of the following: *WFD_NO_CONSTRAINED_PROFILE = 0x00*, *WFD_CONSTRAINED_BASELINE_PROFILE = 0x01*, *WFD_CONSTRAINED_HIGH_PROFILE = 0x02*, *WFD_BOTH_CONSTRAINED_PROFILES = 0x03*) |
| params.h264_codecs.level | number | The Levels bitmap indicates the maximum H.264 level supported for the corresponding H.264 profile indicated in the Profiles Bitmap. Only one bit in the Levels Bitmap used in a WFD subelement shall be set to one. This bitmap is also used in the WFD 3D Video Formats subelement. 
This bitmap is also used by the wfd-video-formats parameter, the wfd-3d-formats parameter, and the wfd-preferred-display-mode parameter. In this case, the bitmap represents either:
 the maximum level supported for the H.264 profile indicated in the Profiles Bitmap supported by the WFD Device in an RTSP M3 Response message (only one bit set to one) or 
 the level and the corresponding H.264 profile selected by the WFD Source in an RTSP M4 Request message (only one bit set to one). (must be one of the following: *WFD_H264_LEVEL_3p1 = 0x01*, *WFD_H264_LEVEL_3p2 = 0x02*, *WFD_H264_LEVEL_4 = 0x04*, *WFD_H264_LEVEL_4p1 = 0x08*, *WFD_H264_LEVEL_4p2 = 0x10*) |
| params.h264_codecs.cea_mask | number | The CEA Resolutions/Refresh rates bitmap represents the set of CEA resolutions and corresponding refresh rates that a WFD Device supports. This bitmap is also used by the wfd-video-formats parameter (must be one of the following: *WFD_NO_CEA_RESOLUTION_SUPPORTED = 0x00000000*, *WFD_CEA_RESOLUTION_640x480p60 = 0x00000001*, *WFD_CEA_RESOLUTION_720x480p60 = 0x00000002*, *WFD_CEA_RESOLUTION_720x480i60 = 0x00000004*, *WFD_CEA_RESOLUTION_720x576p50 = 0x00000008*, *WFD_CEA_RESOLUTION_720x576i50 = 0x00000010*, *WFD_CEA_RESOLUTION_1280x720p30 = 0x00000020*, *WFD_CEA_RESOLUTION_1280x720p60 = 0x00000040*, *WFD_CEA_RESOLUTION_1920x1080p30 = 0x00000080*, *WFD_CEA_RESOLUTION_1920x1080p60 = 0x00000100*, *WFD_CEA_RESOLUTION_1920x1080i60 = 0x00000200*, *WFD_CEA_RESOLUTION_1280x720p25 = 0x00000400*, *WFD_CEA_RESOLUTION_1280x720p50 = 0x00000800*, *WFD_CEA_RESOLUTION_1920x1080p25 = 0x00001000*, *WFD_CEA_RESOLUTION_1920x1080p50 = 0x00002000*, *WFD_CEA_RESOLUTION_1920x1080i50 = 0x00004000*, *WFD_CEA_RESOLUTION_1280x720p24 = 0x00008000*, *WFD_CEA_RESOLUTION_1920x1080p24 = 0x00010000*) |
| params.h264_codecs.vesa_mask | number | The VESA Resolutions/Refresh Rates bitmap represents the set of VESA resolutions and corresponding refresh rates that a WFD Device supports. This bitmap is also used by the wfd-video-formats parameter (must be one of the following: *WFD_NO_VESA_RESOLUTION_SUPPORTED = 0x00000000*, *WFD_VESA_RESOLUTION_800x600p30 = 0x00000001*, *WFD_VESA_RESOLUTION_800x600p60 = 0x00000002*, *WFD_VESA_RESOLUTION_1024x768p30 = 0x00000004*, *WFD_VESA_RESOLUTION_1024x768p60 = 0x00000008*, *WFD_VESA_RESOLUTION_1152x864p30 = 0x00000010*, *WFD_VESA_RESOLUTION_1152x864p60 = 0x00000020*, *WFD_VESA_RESOLUTION_1280x768p30 = 0x00000040*, *WFD_VESA_RESOLUTION_1280x768p60 = 0x00000080*, *WFD_VESA_RESOLUTION_1280x800p30 = 0x00000100*, *WFD_VESA_RESOLUTION_1280x800p60 = 0x00000200*, *WFD_VESA_RESOLUTION_1360x768p30 = 0x00000400*, *WFD_VESA_RESOLUTION_1360x768p60 = 0x00000800*, *WFD_VESA_RESOLUTION_1366x768p30 = 0x00001000*, *WFD_VESA_RESOLUTION_1366x768p60 = 0x00002000*, *WFD_VESA_RESOLUTION_1280x1024p30 = 0x00004000*, *WFD_VESA_RESOLUTION_1280x1024p60 = 0x00008000*, *WFD_VESA_RESOLUTION_1400x1050p30 = 0x00010000*, *WFD_VESA_RESOLUTION_1400x1050p60 = 0x00020000*, *WFD_VESA_RESOLUTION_1440x900p30 = 0x00040000*, *WFD_VESA_RESOLUTION_1440x900p60 = 0x00080000*, *WFD_VESA_RESOLUTION_1600x900p30 = 0x00100000*, *WFD_VESA_RESOLUTION_1600x900p60 = 0x00200000*, *WFD_VESA_RESOLUTION_1600x1200p30 = 0x00400000*, *WFD_VESA_RESOLUTION_1600x1200p60 = 0x00800000*, *WFD_VESA_RESOLUTION_1680x1024p30 = 0x01000000*, *WFD_VESA_RESOLUTION_1680x1024p60 = 0x02000000*, *WFD_VESA_RESOLUTION_1680x1050p30 = 0x04000000*, *WFD_VESA_RESOLUTION_1680x1050p60 = 0x08000000*, *WF_VESA_RESOLUTION_1920x1200p60 = 0x10000000*) |
| params.h264_codecs.hh_mask | number | The HH Resolutions/Refresh Rates bitmap represents the set of resolutions and corresponding refresh rates commonly supported in handheld devices that a WFD Device supports. This bitmap is also used by the wfd-video-formats parameter (must be one of the following: *WFD_NO_HH_RESOLUTION_SUPPORTED = 0x00000000*, *WFD_HH_RESOLUTION_800x480p60 = 0x00000001*, *WFD_HH_RESOLUTION_854x480p30 = 0x00000002*, *WFD_HH_RESOLUTION_854x480p60 = 0x00000004*, *WFD_HH_RESOLUTION_864x480p30 = 0x00000008*, *WFD_HH_RESOLUTION_864x480p60 = 0x00000010*, *WFD_HH_RESOLUTION_600x360p30 = 0x00000020*, *WFD_HH_RESOLUTION_600x360p60 = 0x00000040*, *WFD_HH_RESOLUTION_960x540p30 = 0x00000080*, *WFD_HH_RESOLUTION_960x540p60 = 0x00000100*, *WFD_HH_RESOLUTION_848x480p30 = 0x00000200*, *WFD_HH_RESOLUTION_848x480p60 = 0x00000400*) |
| params.h264_codecs?.latency | number | <sup>*(optional)*</sup> It specifies the latency of the video decoder at the Primary Sink as an integer multiple of 5 msec. This field shall be set to all zeros when transmitted by the WFD Source in a WFD Service Discovery Response frame. This field should be set to all zeros when transmitted by the WFD Source in an RTSP M4 Request message and the WFD Sink shall ignore this field upon reception. If the Primary Sink does not support this field, it shall set this field to all zeros. Otherwise, the WFD Sink shall set this field to a best-effort estimate of the worst-case time between the availability of source data at the input interface of the decoder, and the presentation of the corresponding decoded data at the input interface of the rendering device, rounded up to the next higher multiple of 5 msec |
| params.h264_codecs?.min_slice | number | <sup>*(optional)*</sup> It specifying the smallest slice size expressed in number of macroblocks. If this field is transmitted by the WFD Source, this value shall be the smallest encoded slice it can support. If this field is transmitted by the Primary Sink, this value shall be the smallest slice size it can decode. WFD devices that do not support slice encoding in which a picture is constructed by multiple slices shall set this field to 0x00 00 |
| params.h264_codecs?.slice_encode | number | <sup>*(optional)*</sup> Bitmap for the slice encoding in which a picture is constructed by multiple slices.
Bits [9:0] are named as Max Slice Num bits and used for the Maximum number of slices per picture, minus 1.
Bit [10:12] named as Max Slice Size Ratio bits and used for When this bitmap is used in a WFD subelement: Ratio of Maximum slice size to be used and Minimum slice size indicated in the minimum-slice-size field in WFD Video Formats or WFD 3D Video Formats subelement. When this bitmap is used in an RTSP message: Ratio of Maximum slice size to be used and Minimum slice size indicated in the minimum-slice-size field in wfd-video-formats or wfd-3d-formats. Rest of Bits are reserved |
| params.h264_codecs?.video_frame_skip_support | boolean | <sup>*(optional)*</sup> WFD Video Frame Skipping Support |
| params.h264_codecs?.max_skip_intervals | number | <sup>*(optional)*</sup> It indicates the maximum/allowable time-interval between two video frames after skipped as expressed equation as follows, except for 0b000. (value in decimal) x 0.5 second(s)
0b000 : No limitation
0b001~ 0b111 : parameter for indicating time-interval |
| params.h264_codecs?.video_frame_rate_change_support | boolean | <sup>*(optional)*</sup> WFD Video Framerate change Support |

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
    "method": "org.rdk.MiracastPlayer.setVideoFormats",
    "params": {
        "native": 1,
        "display_mode_supported": false,
        "h264_codecs": {
            "profile": "0x03",
            "level": "0x08",
            "cea_mask": "0x0001FFFF",
            "vesa_mask": "0x00000000",
            "hh_mask": "0x00000000",
            "latency": 0,
            "min_slice": 0,
            "slice_encode": 0,
            "video_frame_skip_support": false,
            "max_skip_intervals": 0,
            "video_frame_rate_change_support": false
        }
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

<a name="setAudioFormats"></a>
## *setAudioFormats*

Set the Audio formats for RTSP capability negotiation.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audio_codecs | object | <sup>*(optional)*</sup>  |
| params?.audio_codecs.audio_format | number | It specifies the type of the Audio decoder at the WFD Sink (must be one of the following: *WFD_LPCM_AUDIO_FORMAT = 0x00000001*, *WFD_AAC_AUDIO_FORMAT = 0x00000002*, *WFD_AC3_AUDIO_FORMAT = 0x00000003*) |
| params?.audio_codecs.modes | number | The LPCM/AAC/AC3 Modes bitmap represents configurations supported by the WFD Device. This bitmap is also used in the ‘modes’ field of wfd-audio-codecs.
If audio_format is WFD_LPCM_AUDIO_FORMAT, then use mode value from prefixed with WFD_LPCM_. Similarly use corresponding value for AAC and AC3 (must be one of the following: *WFD_LPCM_CH2_44p1kHz  = 0x00000001*, *WFD_LPCM_CH2_48kHz  = 0x00000002*, *WFD_AAC_CH2_48kHz      = 0x00000001*, *WFD_AAC_CH4_48kHz      = 0x00000002*, *WFD_AAC_CH6_48kHz      = 0x00000004*, *WFD_AAC_CH8_48kHz      = 0x00000008*, *WFD_AC3_CH2_48kHz      = 0x00000001*, *WFD_AC3_CH4_48kHz      = 0x00000002*, *WFD_AC3_CH6_48kHz      = 0x00000004*) |
| params?.audio_codecs.latency | number | It specifies the latency of the Audio decoder at the WFD Sink as an integer multiple of 5 msec. This field shall be set to all zeros when transmitted by the WFD Source in a WFD Service Discovery Response frame. This field should be set to all zeros when transmitted by the WFD Source in an RTSP M4 Request message and the WFD Sink shall ignore this field upon reception. If the WFD Sink does not support this field, it shall set this field to all zeros. Otherwise, the WFD Sink shall set this field to a best-effort estimate of the worst-case time between the availability of source data at the input interface of the decoder, and the presentation of the corresponding decoded data at the input interface of the rendering device, rounded up to the next higher multiple of 5 msec |

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
    "method": "org.rdk.MiracastPlayer.setAudioFormats",
    "params": {
        "audio_codecs": {
            "audio_format": "0x00000002",
            "modes": "0x00000001",
            "latency": 0
        }
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

The following events are provided by the org.rdk.MiracastPlayer plugin:

MiracastPlayer interface events:

| Event | Description |
| :-------- | :-------- |
| [onStateChange](#onStateChange) | Triggered when the Miracast Player Plugin changes the states |


<a name="onStateChange"></a>
## *onStateChange*

Triggered when the Miracast Player Plugin changes the states. (INITIATED | INPROGRESS | PLAYING | IDLE/STOPPED(Default State)).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.name | string | Name of the source device |
| params.mac | string | Mac id of the source device |
| params.state | string | Current state of the Miracast player |
| params.reason_code | number | Player Reason codes (must be one of the following: *MIRACAST_PLAYER_REASON_CODE_SUCCESS = 200*, *MIRACAST_PLAYER_REASON_CODE_APP_REQ_TO_STOP = 201*, *MIRACAST_PLAYER_REASON_CODE_SRC_DEV_REQ_TO_STOP = 202*, *MIRACAST_PLAYER_REASON_CODE_RTSP_ERROR = 203*, *MIRACAST_PLAYER_REASON_CODE_RTSP_TIMEOUT = 204*, *MIRACAST_PLAYER_REASON_CODE_RTSP_METHOD_NOT_SUPPORTED = 205*, *MIRACAST_PLAYER_REASON_CODE_GST_ERROR = 206*, *MIRACAST_PLAYER_REASON_CODE_INT_FAILURE = 207*, *MIRACAST_PLAYER_REASON_CODE_NEW_SRC_DEV_CONNECT_REQ = 208*) |
| params.reason | string | Description about reason_code |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onStateChange",
    "params": {
        "name": "Manufacturer WiFi-Direct Name",
        "mac": "XX:XX:XX:XX:XX:XX",
        "state": "(INITIATED | INPROGRESS | PLAYING | STOPPED/IDLE(Default State))",
        "reason_code": 200,
        "reason": "read Description"
    }
}
```

