<!-- Generated automatically, DO NOT EDIT! -->
<a name="ControlService_Plugin"></a>
# ControlService Plugin

**Version: 1.0.0**

A org.rdk.ControlService plugin for Thunder framework.

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

The `ControlService` plugin provides the ability to manage the remote controls of a STB.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.ControlService*) |
| classname | string | Class name: *org.rdk.ControlService* |
| locator | string | Library name: ** |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.ControlService plugin:

ControlService interface methods:

| Method | Description |
| :-------- | :-------- |
| [canFindMyRemote](#canFindMyRemote) | Checks if the Control Manager can search for the remote |
| [checkRf4ceChipConnectivity](#checkRf4ceChipConnectivity) | Checks Rf4ce chip connectivity status |
| [endPairingMode](#endPairingMode) | Leaves pairing mode |
| [findLastUsedRemote](#findLastUsedRemote) | Searches for the last used remote |
| [getAllRemoteData](#getAllRemoteData) | Returns all remote data |
| [getLastKeypressSource](#getLastKeypressSource) | Returns last key press source data |
| [getLastPairedRemoteData](#getLastPairedRemoteData) | Returns all remote data for the last paired remote |
| [getQuirks](#getQuirks) | Gets quirks |
| [getSingleRemoteData](#getSingleRemoteData) | Returns all remote data for the specified remote |
| [getValues](#getValues) | Returns remote setting values |
| [setValues](#setValues) | Sets remote setting values |
| [startPairingMode](#startPairingMode) | Enters pairing mode |


<a name="canFindMyRemote"></a>
## *canFindMyRemote*

Checks if the Control Manager can search for the remote. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty `params` object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.result | boolean | The operation status |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.ControlService.1.canFindMyRemote",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "result": true,
        "success": true
    }
}
```

<a name="checkRf4ceChipConnectivity"></a>
## *checkRf4ceChipConnectivity*

Checks Rf4ce chip connectivity status. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty `params` object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.rf4ceChipConnected | integer | The rf4ce chip connectivity status |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.ControlService.1.checkRf4ceChipConnectivity",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "rf4ceChipConnected": 1,
        "success": true
    }
}
```

<a name="endPairingMode"></a>
## *endPairingMode*

Leaves pairing mode. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty `params` object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.bindStatus | integer | The binding status |
| result.status_code | integer | An operations status code |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.ControlService.1.endPairingMode",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "bindStatus": 1,
        "status_code": 0,
        "success": true
    }
}
```

<a name="findLastUsedRemote"></a>
## *findLastUsedRemote*

Searches for the last used remote.
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onControl`| Triggered when the last used remote is successfully found |.

Also see: [onControl](#onControl)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.timeOutPeriod | integer | The timeout interval in milliseconds |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.status_code | integer | An operations status code |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.ControlService.1.findLastUsedRemote",
    "params": {
        "timeOutPeriod": 20
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "status_code": 0,
        "success": true
    }
}
```

<a name="getAllRemoteData"></a>
## *getAllRemoteData*

Returns all remote data. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty `params` object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.stbRf4ceMACAddress | string | The STB MAC address for the RF4CE network |
| result.stbRf4ceSocMfr | string | The STB SOC Manufacturer |
| result.stbHALVersion | string | The STB HAL version |
| result.stbRf4ceShortAddress | integer | The STB short address for the RF4CE |
| result.stbPanId | integer | The STB PAN Identifier |
| result.stbActiveChannel | integer | The STB active channel |
| result.stbNumPairedRemotes | integer | The number of remotes paired to the STB |
| result.stbNumScreenBindFailures | integer | The number of screen bind failures for the STB |
| result.stbLastScreenBindErrorCode | integer | The last screen bind error code on the STB |
| result.stbLastScreenBindErrorRemoteType | string | The remote type for the last screen bind error on the STB |
| result.stbLastScreenBindErrorTimestamp | integer | The last screen bind error timestamp on the STB |
| result.stbNumOtherBindFailures | integer | The number of other bind failures on the STB |
| result.stbLastOtherBindErrorCode | integer | The last other bind error code on the STB |
| result.stbLastOtherBindErrorRemoteType | string | The remote type for the last other bind error on the STB |
| result.stbLastOtherBindErrorBindType | integer | The last other bind error bind type on the STB |
| result.stbLastOtherBindErrorTimestamp | integer | The last other bind error timestamp |
| result.bHasIrRemotePreviousDay | boolean | `true` or `false` |
| result.bHasIrRemoteCurrentDay | boolean | `true` or `false` |
| result?.remoteData | object | <sup>*(optional)*</sup>  |
| result?.remoteData?.remoteId | integer | <sup>*(optional)*</sup> The remote control identifier |
| result?.remoteData?.remoteMACAddress | string | <sup>*(optional)*</sup> The MAC address of remote |
| result?.remoteData?.remoteModel | string | <sup>*(optional)*</sup> The remote model |
| result?.remoteData?.remoteModelVersion | string | <sup>*(optional)*</sup> The version of remote model |
| result?.remoteData?.batteryLevelLoaded | string | <sup>*(optional)*</sup> The loaded battery level |
| result?.remoteData?.batteryLevelPercentage | integer | <sup>*(optional)*</sup> The battery percentage |
| result?.remoteData?.batteryLastEvent | integer | <sup>*(optional)*</sup> The battery last event |
| result?.remoteData?.batteryLastEventTimestamp | integer | <sup>*(optional)*</sup> The timestamp of last battery event (in milliseconds) |
| result?.remoteData?.numVoiceCommandsPreviousDay | integer | <sup>*(optional)*</sup> Number of voice commands the previous day |
| result?.remoteData?.numVoiceCommandsCurrentDay | integer | <sup>*(optional)*</sup> Number of voice commands the current day |
| result?.remoteData?.numVoiceShortUtterancesPreviousDay | integer | <sup>*(optional)*</sup> Number of short voice commands the previous day |
| result?.remoteData?.numVoiceShortUtterancesCurrentDay | integer | <sup>*(optional)*</sup> Number of short voice commands the current day |
| result?.remoteData?.numVoicePacketsSentPreviousDay | integer | <sup>*(optional)*</sup> Number of voice packets sent the previous day |
| result?.remoteData?.numVoicePacketsSentCurrentDay | integer | <sup>*(optional)*</sup> Number of voice packets sent the current day |
| result?.remoteData?.numVoicePacketsLostPreviousDay | integer | <sup>*(optional)*</sup> Number of voice packets lost the previous day |
| result?.remoteData?.numVoicePacketsLostCurrentDay | integer | <sup>*(optional)*</sup> Number of voice packets lost the current day |
| result?.remoteData?.aveVoicePacketLossPreviousDay | string | <sup>*(optional)*</sup> Average number of voice packets lost the previous day |
| result?.remoteData?.aveVoicePacketLossCurrentDay | string | <sup>*(optional)*</sup> Average number of voice packets lost the current day |
| result?.remoteData?.numVoiceCmdsHighLossPreviousDay | integer | <sup>*(optional)*</sup> Number of voice commands exceeding packet loss threshold the previous day |
| result?.remoteData?.numVoiceCmdsHighLossCurrentDay | integer | <sup>*(optional)*</sup> Number of voice commands exceeding packet loss threshold the current day |
| result?.remoteData?.versionInfoSw | string | <sup>*(optional)*</sup> The software version of remote |
| result?.remoteData?.versionInfoHw | string | <sup>*(optional)*</sup> The hardware version of remote |
| result?.remoteData?.versionInfoIrdb | string | <sup>*(optional)*</sup> The IR database version of remote |
| result?.remoteData?.irdbType | integer | <sup>*(optional)*</sup> The IR database type |
| result?.remoteData?.irdbState | integer | <sup>*(optional)*</sup> The state of IR database |
| result?.remoteData?.programmedTvIRCode | string | <sup>*(optional)*</sup> The IR code programmed for TV |
| result?.remoteData?.programmedAvrIRCode | string | <sup>*(optional)*</sup> The IR code programmed for AVR |
| result?.remoteData?.lastCommandTimeDate | integer | <sup>*(optional)*</sup> The time when last key was pressed |
| result?.remoteData?.rf4ceRemoteSocMfr | string | <sup>*(optional)*</sup> The remote chipset manufacturer |
| result?.remoteData?.remoteMfr | string | <sup>*(optional)*</sup> The remote manufacturer |
| result?.remoteData?.signalStrengthPercentage | integer | <sup>*(optional)*</sup> The percentage of signal strength |
| result?.remoteData?.linkQuality | integer | <sup>*(optional)*</sup> The link quality |
| result?.remoteData?.howRemoteIsPaired | string | <sup>*(optional)*</sup> The binding type of remote |
| result?.remoteData?.bHasCheckedIn | boolean | <sup>*(optional)*</sup> `true` if the remote has checkedin for update and `false` otherwise |
| result?.remoteData?.bHasRemoteBeenUpdated | boolean | <sup>*(optional)*</sup> `true` if the remote's firmware is updated and `false` otherwise |
| result?.remoteData?.bIrdbDownloadSupported | boolean | <sup>*(optional)*</sup> `true` if the IR database download is supported and `false` otherwise |
| result?.remoteData?.pairingTimestamp | integer | <sup>*(optional)*</sup> The binding time (in milliseconds) |
| result?.remoteData?.lastRebootErrorCode | integer | <sup>*(optional)*</sup> The error code for previous reboot (in milliseconds) |
| result?.remoteData?.lastRebootTimestamp | integer | <sup>*(optional)*</sup> The previous reboot time (in milliseconds) |
| result?.remoteData?.securityType | integer | <sup>*(optional)*</sup> The security type |
| result?.remoteData?.bHasBattery | boolean | <sup>*(optional)*</sup> `true` if remote has battery and `false` otherwise |
| result?.remoteData?.batteryChangedTimestamp | integer | <sup>*(optional)*</sup> The time of battery change (in milliseconds) |
| result?.remoteData?.batteryChangedActualPercentage | integer | <sup>*(optional)*</sup> The battery changed actual percentage |
| result?.remoteData?.batteryChangedUnloadedVoltage | string | <sup>*(optional)*</sup> The battery changed unloaded voltage |
| result?.remoteData?.battery75PercentTimestamp | integer | <sup>*(optional)*</sup> The timestamp when battery was 75% (in milliseconds) |
| result?.remoteData?.battery75PercentActualPercentage | integer | <sup>*(optional)*</sup> The actual percentage for 75% battery |
| result?.remoteData?.battery75PercentUnloadedVoltage | string | <sup>*(optional)*</sup> The unloaded voltage of 75% battery |
| result?.remoteData?.battery50PercentTimestamp | integer | <sup>*(optional)*</sup> The timestamp when battery was 50% (in milliseconds) |
| result?.remoteData?.battery50PercentActualPercentage | integer | <sup>*(optional)*</sup> The actual percentage for 50% battery |
| result?.remoteData?.battery50PercentUnloadedVoltage | string | <sup>*(optional)*</sup> The unloaded voltage of 50% battery |
| result?.remoteData?.battery25PercentTimestamp | integer | <sup>*(optional)*</sup> The timestamp when battery was 25% (in milliseconds) |
| result?.remoteData?.battery25PercentActualPercentage | integer | <sup>*(optional)*</sup> The actual percentage for 25% battery |
| result?.remoteData?.battery25PercentUnloadedVoltage | string | <sup>*(optional)*</sup> The unloaded voltage of 25% battery |
| result?.remoteData?.battery5PercentTimestamp | integer | <sup>*(optional)*</sup> The timestamp when battery was 5% (in milliseconds) |
| result?.remoteData?.battery5PercentActualPercentage | integer | <sup>*(optional)*</sup> The actual percentage for 5% battery |
| result?.remoteData?.battery5PercentUnloadedVoltage | string | <sup>*(optional)*</sup> The unloaded voltage of 5% battery |
| result?.remoteData?.battery0PercentTimestamp | integer | <sup>*(optional)*</sup> The timestamp for 0% battery (in milliseconds) |
| result?.remoteData?.battery0PercentActualPercentage | integer | <sup>*(optional)*</sup> The actual percentage for 0% battery |
| result?.remoteData?.battery0PercentUnloadedVoltage | string | <sup>*(optional)*</sup> The unloaded voltage of 0% battery |
| result?.remoteData?.batteryVoltageLargeJumpCounter | integer | <sup>*(optional)*</sup> Number of large voltage jumps in battery |
| result?.remoteData?.batteryVoltageLargeDeclineDetected | boolean | <sup>*(optional)*</sup> `true` if large voltage decline in battery is detected, otherwise `false` |
| result?.remoteData?.bHasDSP | boolean | <sup>*(optional)*</sup> `true` if remote has digital signal processor and `false` otherwise |
| result?.remoteData?.averageTimeInPrivacyMode | integer | <sup>*(optional)*</sup> Average time in privacy mode (in milliseconds) |
| result?.remoteData?.bInPrivacyMode | boolean | <sup>*(optional)*</sup> `true` if remote is in privacy mode and `false` otherwise |
| result?.remoteData?.averageSNR | integer | <sup>*(optional)*</sup> Average signal to noise ratio |
| result?.remoteData?.averageKeywordConfidence | integer | <sup>*(optional)*</sup> Average keyword confidence |
| result?.remoteData?.totalNumberOfMicsWorking | integer | <sup>*(optional)*</sup> Number of mics working |
| result?.remoteData?.totalNumberOfSpeakersWorking | integer | <sup>*(optional)*</sup> Number of speakers working |
| result?.remoteData?.endOfSpeechInitialTimeoutCount | integer | <sup>*(optional)*</sup> Initial time out count after end of speech |
| result?.remoteData?.endOfSpeechTimeoutCount | integer | <sup>*(optional)*</sup> Time out count after end of speech |
| result?.remoteData?.uptimeStartTime | integer | <sup>*(optional)*</sup> Start of uptime (in milliseconds) |
| result?.remoteData?.uptimeInSeconds | integer | <sup>*(optional)*</sup> Uptime (in seconds) |
| result?.remoteData?.privacyTimeInSeconds | integer | <sup>*(optional)*</sup> Privacy time (in seconds) |
| result?.remoteData?.versionDSPBuildId | string | <sup>*(optional)*</sup> Version of dsp build ID |
| result.status_code | integer | An operations status code |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.ControlService.1.getAllRemoteData",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "stbRf4ceMACAddress": "0x00155F00205E1XXX",
        "stbRf4ceSocMfr": "GP502KXBG",
        "stbHALVersion": "GPv2.6.3.514598",
        "stbRf4ceShortAddress": 64253,
        "stbPanId": 25684,
        "stbActiveChannel": 25,
        "stbNumPairedRemotes": 0,
        "stbNumScreenBindFailures": 1,
        "stbLastScreenBindErrorCode": 1,
        "stbLastScreenBindErrorRemoteType": "...",
        "stbLastScreenBindErrorTimestamp": 1589356931000,
        "stbNumOtherBindFailures": 3,
        "stbLastOtherBindErrorCode": 1,
        "stbLastOtherBindErrorRemoteType": "...",
        "stbLastOtherBindErrorBindType": 2,
        "stbLastOtherBindErrorTimestamp": 1589359161000,
        "bHasIrRemotePreviousDay": false,
        "bHasIrRemoteCurrentDay": false,
        "remoteData": {
            "remoteId": 1,
            "remoteMACAddress": "0x00155F011C7F7359",
            "remoteModel": "XR15",
            "remoteModelVersion": "v1",
            "batteryLevelLoaded": "2.619608",
            "batteryLevelPercentage": 60,
            "batteryLastEvent": 0,
            "batteryLastEventTimestamp": 1602879639000,
            "numVoiceCommandsPreviousDay": 0,
            "numVoiceCommandsCurrentDay": 0,
            "numVoiceShortUtterancesPreviousDay": 0,
            "numVoiceShortUtterancesCurrentDay": 0,
            "numVoicePacketsSentPreviousDay": 0,
            "numVoicePacketsSentCurrentDay": 0,
            "numVoicePacketsLostPreviousDay": 0,
            "numVoicePacketsLostCurrentDay": 0,
            "aveVoicePacketLossPreviousDay": "0.000000",
            "aveVoicePacketLossCurrentDay": "0.000000",
            "numVoiceCmdsHighLossPreviousDay": 0,
            "numVoiceCmdsHighLossCurrentDay": 0,
            "versionInfoSw": "2.0.1.2",
            "versionInfoHw": "2.3.1.0",
            "versionInfoIrdb": "4.3.2.0",
            "irdbType": 0,
            "irdbState": 3,
            "programmedTvIRCode": "12731",
            "programmedAvrIRCode": "31360",
            "lastCommandTimeDate": 1580263335000,
            "rf4ceRemoteSocMfr": "QORVO",
            "remoteMfr": "RS",
            "signalStrengthPercentage": 50,
            "linkQuality": 0,
            "howRemoteIsPaired": "manual",
            "bHasCheckedIn": true,
            "bHasRemoteBeenUpdated": true,
            "bIrdbDownloadSupported": true,
            "pairingTimestamp": 1538782229000,
            "lastRebootErrorCode": 1538782229000,
            "lastRebootTimestamp": 12342000,
            "securityType": 0,
            "bHasBattery": true,
            "batteryChangedTimestamp": 1641294389000,
            "batteryChangedActualPercentage": 64,
            "batteryChangedUnloadedVoltage": "2.729412",
            "battery75PercentTimestamp": 1641294389000,
            "battery75PercentActualPercentage": 64,
            "battery75PercentUnloadedVoltage": "2.729412",
            "battery50PercentTimestamp": 0,
            "battery50PercentActualPercentage": 0,
            "battery50PercentUnloadedVoltage": "0.000000",
            "battery25PercentTimestamp": 0,
            "battery25PercentActualPercentage": 0,
            "battery25PercentUnloadedVoltage": "0.000000",
            "battery5PercentTimestamp": 0,
            "battery5PercentActualPercentage": 0,
            "battery5PercentUnloadedVoltage": "0.000000",
            "battery0PercentTimestamp": 0,
            "battery0PercentActualPercentage": 0,
            "battery0PercentUnloadedVoltage": "0.000000",
            "batteryVoltageLargeJumpCounter": 0,
            "batteryVoltageLargeDeclineDetected": false,
            "bHasDSP": false,
            "averageTimeInPrivacyMode": 0,
            "bInPrivacyMode": false,
            "averageSNR": 0,
            "averageKeywordConfidence": 0,
            "totalNumberOfMicsWorking": 0,
            "totalNumberOfSpeakersWorking": 0,
            "endOfSpeechInitialTimeoutCount": 0,
            "endOfSpeechTimeoutCount": 0,
            "uptimeStartTime": 1641460244000,
            "uptimeInSeconds": 0,
            "privacyTimeInSeconds": 0,
            "versionDSPBuildId": "..."
        },
        "status_code": 0,
        "success": true
    }
}
```

<a name="getLastKeypressSource"></a>
## *getLastKeypressSource*

Returns last key press source data. The data, if any, is returned as part of the `result` object. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty `params` object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.remoteId | integer | The remote control identifier |
| result.timestamp | integer | The timestamp of last key press (in milliseconds) |
| result.sourceName | string | The source name |
| result.sourceType | string | The source type |
| result.sourceKeyCode | integer | The source keycode |
| result.bIsScreenBindMode | boolean | `true` if screen bind mode is ON and `false` otherwise |
| result.remoteKeypadConfig | integer | The remote keypad configuration |
| result.status_code | integer | An operations status code |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.ControlService.1.getLastKeypressSource",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "remoteId": 1,
        "timestamp": 1598470622000,
        "sourceName": "XR15-10",
        "sourceType": "RF",
        "sourceKeyCode": 133,
        "bIsScreenBindMode": true,
        "remoteKeypadConfig": 1,
        "status_code": 0,
        "success": true
    }
}
```

<a name="getLastPairedRemoteData"></a>
## *getLastPairedRemoteData*

Returns all remote data for the last paired remote. The data, if any, is returned as part of the `result` object. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty `params` object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.remoteData | object | <sup>*(optional)*</sup>  |
| result?.remoteData?.remoteId | integer | <sup>*(optional)*</sup> The remote control identifier |
| result?.remoteData?.remoteMACAddress | string | <sup>*(optional)*</sup> The MAC address of remote |
| result?.remoteData?.remoteModel | string | <sup>*(optional)*</sup> The remote model |
| result?.remoteData?.remoteModelVersion | string | <sup>*(optional)*</sup> The version of remote model |
| result?.remoteData?.batteryLevelLoaded | string | <sup>*(optional)*</sup> The loaded battery level |
| result?.remoteData?.batteryLevelPercentage | integer | <sup>*(optional)*</sup> The battery percentage |
| result?.remoteData?.batteryLastEvent | integer | <sup>*(optional)*</sup> The battery last event |
| result?.remoteData?.batteryLastEventTimestamp | integer | <sup>*(optional)*</sup> The timestamp of last battery event (in milliseconds) |
| result?.remoteData?.numVoiceCommandsPreviousDay | integer | <sup>*(optional)*</sup> Number of voice commands the previous day |
| result?.remoteData?.numVoiceCommandsCurrentDay | integer | <sup>*(optional)*</sup> Number of voice commands the current day |
| result?.remoteData?.numVoiceShortUtterancesPreviousDay | integer | <sup>*(optional)*</sup> Number of short voice commands the previous day |
| result?.remoteData?.numVoiceShortUtterancesCurrentDay | integer | <sup>*(optional)*</sup> Number of short voice commands the current day |
| result?.remoteData?.numVoicePacketsSentPreviousDay | integer | <sup>*(optional)*</sup> Number of voice packets sent the previous day |
| result?.remoteData?.numVoicePacketsSentCurrentDay | integer | <sup>*(optional)*</sup> Number of voice packets sent the current day |
| result?.remoteData?.numVoicePacketsLostPreviousDay | integer | <sup>*(optional)*</sup> Number of voice packets lost the previous day |
| result?.remoteData?.numVoicePacketsLostCurrentDay | integer | <sup>*(optional)*</sup> Number of voice packets lost the current day |
| result?.remoteData?.aveVoicePacketLossPreviousDay | string | <sup>*(optional)*</sup> Average number of voice packets lost the previous day |
| result?.remoteData?.aveVoicePacketLossCurrentDay | string | <sup>*(optional)*</sup> Average number of voice packets lost the current day |
| result?.remoteData?.numVoiceCmdsHighLossPreviousDay | integer | <sup>*(optional)*</sup> Number of voice commands exceeding packet loss threshold the previous day |
| result?.remoteData?.numVoiceCmdsHighLossCurrentDay | integer | <sup>*(optional)*</sup> Number of voice commands exceeding packet loss threshold the current day |
| result?.remoteData?.versionInfoSw | string | <sup>*(optional)*</sup> The software version of remote |
| result?.remoteData?.versionInfoHw | string | <sup>*(optional)*</sup> The hardware version of remote |
| result?.remoteData?.versionInfoIrdb | string | <sup>*(optional)*</sup> The IR database version of remote |
| result?.remoteData?.irdbType | integer | <sup>*(optional)*</sup> The IR database type |
| result?.remoteData?.irdbState | integer | <sup>*(optional)*</sup> The state of IR database |
| result?.remoteData?.programmedTvIRCode | string | <sup>*(optional)*</sup> The IR code programmed for TV |
| result?.remoteData?.programmedAvrIRCode | string | <sup>*(optional)*</sup> The IR code programmed for AVR |
| result?.remoteData?.lastCommandTimeDate | integer | <sup>*(optional)*</sup> The time when last key was pressed |
| result?.remoteData?.rf4ceRemoteSocMfr | string | <sup>*(optional)*</sup> The remote chipset manufacturer |
| result?.remoteData?.remoteMfr | string | <sup>*(optional)*</sup> The remote manufacturer |
| result?.remoteData?.signalStrengthPercentage | integer | <sup>*(optional)*</sup> The percentage of signal strength |
| result?.remoteData?.linkQuality | integer | <sup>*(optional)*</sup> The link quality |
| result?.remoteData?.howRemoteIsPaired | string | <sup>*(optional)*</sup> The binding type of remote |
| result?.remoteData?.bHasCheckedIn | boolean | <sup>*(optional)*</sup> `true` if the remote has checkedin for update and `false` otherwise |
| result?.remoteData?.bHasRemoteBeenUpdated | boolean | <sup>*(optional)*</sup> `true` if the remote's firmware is updated and `false` otherwise |
| result?.remoteData?.bIrdbDownloadSupported | boolean | <sup>*(optional)*</sup> `true` if the IR database download is supported and `false` otherwise |
| result?.remoteData?.pairingTimestamp | integer | <sup>*(optional)*</sup> The binding time (in milliseconds) |
| result?.remoteData?.lastRebootErrorCode | integer | <sup>*(optional)*</sup> The error code for previous reboot (in milliseconds) |
| result?.remoteData?.lastRebootTimestamp | integer | <sup>*(optional)*</sup> The previous reboot time (in milliseconds) |
| result?.remoteData?.securityType | integer | <sup>*(optional)*</sup> The security type |
| result?.remoteData?.bHasBattery | boolean | <sup>*(optional)*</sup> `true` if remote has battery and `false` otherwise |
| result?.remoteData?.batteryChangedTimestamp | integer | <sup>*(optional)*</sup> The time of battery change (in milliseconds) |
| result?.remoteData?.batteryChangedActualPercentage | integer | <sup>*(optional)*</sup> The battery changed actual percentage |
| result?.remoteData?.batteryChangedUnloadedVoltage | string | <sup>*(optional)*</sup> The battery changed unloaded voltage |
| result?.remoteData?.battery75PercentTimestamp | integer | <sup>*(optional)*</sup> The timestamp when battery was 75% (in milliseconds) |
| result?.remoteData?.battery75PercentActualPercentage | integer | <sup>*(optional)*</sup> The actual percentage for 75% battery |
| result?.remoteData?.battery75PercentUnloadedVoltage | string | <sup>*(optional)*</sup> The unloaded voltage of 75% battery |
| result?.remoteData?.battery50PercentTimestamp | integer | <sup>*(optional)*</sup> The timestamp when battery was 50% (in milliseconds) |
| result?.remoteData?.battery50PercentActualPercentage | integer | <sup>*(optional)*</sup> The actual percentage for 50% battery |
| result?.remoteData?.battery50PercentUnloadedVoltage | string | <sup>*(optional)*</sup> The unloaded voltage of 50% battery |
| result?.remoteData?.battery25PercentTimestamp | integer | <sup>*(optional)*</sup> The timestamp when battery was 25% (in milliseconds) |
| result?.remoteData?.battery25PercentActualPercentage | integer | <sup>*(optional)*</sup> The actual percentage for 25% battery |
| result?.remoteData?.battery25PercentUnloadedVoltage | string | <sup>*(optional)*</sup> The unloaded voltage of 25% battery |
| result?.remoteData?.battery5PercentTimestamp | integer | <sup>*(optional)*</sup> The timestamp when battery was 5% (in milliseconds) |
| result?.remoteData?.battery5PercentActualPercentage | integer | <sup>*(optional)*</sup> The actual percentage for 5% battery |
| result?.remoteData?.battery5PercentUnloadedVoltage | string | <sup>*(optional)*</sup> The unloaded voltage of 5% battery |
| result?.remoteData?.battery0PercentTimestamp | integer | <sup>*(optional)*</sup> The timestamp for 0% battery (in milliseconds) |
| result?.remoteData?.battery0PercentActualPercentage | integer | <sup>*(optional)*</sup> The actual percentage for 0% battery |
| result?.remoteData?.battery0PercentUnloadedVoltage | string | <sup>*(optional)*</sup> The unloaded voltage of 0% battery |
| result?.remoteData?.batteryVoltageLargeJumpCounter | integer | <sup>*(optional)*</sup> Number of large voltage jumps in battery |
| result?.remoteData?.batteryVoltageLargeDeclineDetected | boolean | <sup>*(optional)*</sup> `true` if large voltage decline in battery is detected, otherwise `false` |
| result?.remoteData?.bHasDSP | boolean | <sup>*(optional)*</sup> `true` if remote has digital signal processor and `false` otherwise |
| result?.remoteData?.averageTimeInPrivacyMode | integer | <sup>*(optional)*</sup> Average time in privacy mode (in milliseconds) |
| result?.remoteData?.bInPrivacyMode | boolean | <sup>*(optional)*</sup> `true` if remote is in privacy mode and `false` otherwise |
| result?.remoteData?.averageSNR | integer | <sup>*(optional)*</sup> Average signal to noise ratio |
| result?.remoteData?.averageKeywordConfidence | integer | <sup>*(optional)*</sup> Average keyword confidence |
| result?.remoteData?.totalNumberOfMicsWorking | integer | <sup>*(optional)*</sup> Number of mics working |
| result?.remoteData?.totalNumberOfSpeakersWorking | integer | <sup>*(optional)*</sup> Number of speakers working |
| result?.remoteData?.endOfSpeechInitialTimeoutCount | integer | <sup>*(optional)*</sup> Initial time out count after end of speech |
| result?.remoteData?.endOfSpeechTimeoutCount | integer | <sup>*(optional)*</sup> Time out count after end of speech |
| result?.remoteData?.uptimeStartTime | integer | <sup>*(optional)*</sup> Start of uptime (in milliseconds) |
| result?.remoteData?.uptimeInSeconds | integer | <sup>*(optional)*</sup> Uptime (in seconds) |
| result?.remoteData?.privacyTimeInSeconds | integer | <sup>*(optional)*</sup> Privacy time (in seconds) |
| result?.remoteData?.versionDSPBuildId | string | <sup>*(optional)*</sup> Version of dsp build ID |
| result.status_code | integer | An operations status code |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.ControlService.1.getLastPairedRemoteData",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "remoteData": {
            "remoteId": 1,
            "remoteMACAddress": "0x00155F011C7F7359",
            "remoteModel": "XR15",
            "remoteModelVersion": "v1",
            "batteryLevelLoaded": "2.619608",
            "batteryLevelPercentage": 60,
            "batteryLastEvent": 0,
            "batteryLastEventTimestamp": 1602879639000,
            "numVoiceCommandsPreviousDay": 0,
            "numVoiceCommandsCurrentDay": 0,
            "numVoiceShortUtterancesPreviousDay": 0,
            "numVoiceShortUtterancesCurrentDay": 0,
            "numVoicePacketsSentPreviousDay": 0,
            "numVoicePacketsSentCurrentDay": 0,
            "numVoicePacketsLostPreviousDay": 0,
            "numVoicePacketsLostCurrentDay": 0,
            "aveVoicePacketLossPreviousDay": "0.000000",
            "aveVoicePacketLossCurrentDay": "0.000000",
            "numVoiceCmdsHighLossPreviousDay": 0,
            "numVoiceCmdsHighLossCurrentDay": 0,
            "versionInfoSw": "2.0.1.2",
            "versionInfoHw": "2.3.1.0",
            "versionInfoIrdb": "4.3.2.0",
            "irdbType": 0,
            "irdbState": 3,
            "programmedTvIRCode": "12731",
            "programmedAvrIRCode": "31360",
            "lastCommandTimeDate": 1580263335000,
            "rf4ceRemoteSocMfr": "QORVO",
            "remoteMfr": "RS",
            "signalStrengthPercentage": 50,
            "linkQuality": 0,
            "howRemoteIsPaired": "manual",
            "bHasCheckedIn": true,
            "bHasRemoteBeenUpdated": true,
            "bIrdbDownloadSupported": true,
            "pairingTimestamp": 1538782229000,
            "lastRebootErrorCode": 1538782229000,
            "lastRebootTimestamp": 12342000,
            "securityType": 0,
            "bHasBattery": true,
            "batteryChangedTimestamp": 1641294389000,
            "batteryChangedActualPercentage": 64,
            "batteryChangedUnloadedVoltage": "2.729412",
            "battery75PercentTimestamp": 1641294389000,
            "battery75PercentActualPercentage": 64,
            "battery75PercentUnloadedVoltage": "2.729412",
            "battery50PercentTimestamp": 0,
            "battery50PercentActualPercentage": 0,
            "battery50PercentUnloadedVoltage": "0.000000",
            "battery25PercentTimestamp": 0,
            "battery25PercentActualPercentage": 0,
            "battery25PercentUnloadedVoltage": "0.000000",
            "battery5PercentTimestamp": 0,
            "battery5PercentActualPercentage": 0,
            "battery5PercentUnloadedVoltage": "0.000000",
            "battery0PercentTimestamp": 0,
            "battery0PercentActualPercentage": 0,
            "battery0PercentUnloadedVoltage": "0.000000",
            "batteryVoltageLargeJumpCounter": 0,
            "batteryVoltageLargeDeclineDetected": false,
            "bHasDSP": false,
            "averageTimeInPrivacyMode": 0,
            "bInPrivacyMode": false,
            "averageSNR": 0,
            "averageKeywordConfidence": 0,
            "totalNumberOfMicsWorking": 0,
            "totalNumberOfSpeakersWorking": 0,
            "endOfSpeechInitialTimeoutCount": 0,
            "endOfSpeechTimeoutCount": 0,
            "uptimeStartTime": 1641460244000,
            "uptimeInSeconds": 0,
            "privacyTimeInSeconds": 0,
            "versionDSPBuildId": "..."
        },
        "status_code": 0,
        "success": true
    }
}
```

<a name="getQuirks"></a>
## *getQuirks*

Gets quirks. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty `params` object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.quirks | array | The list of quirks |
| result.quirks[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.ControlService.1.getQuirks",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "quirks": [
            "DELIA-43686, RDK-28767, RDK-31263, RDK-32347"
        ],
        "success": true
    }
}
```

<a name="getSingleRemoteData"></a>
## *getSingleRemoteData*

Returns all remote data for the specified remote. The data, if any, is returned as part of the `result` object. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.remoteId | integer | The remote control identifier |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.remoteData | object | <sup>*(optional)*</sup>  |
| result?.remoteData?.remoteId | integer | <sup>*(optional)*</sup> The remote control identifier |
| result?.remoteData?.remoteMACAddress | string | <sup>*(optional)*</sup> The MAC address of remote |
| result?.remoteData?.remoteModel | string | <sup>*(optional)*</sup> The remote model |
| result?.remoteData?.remoteModelVersion | string | <sup>*(optional)*</sup> The version of remote model |
| result?.remoteData?.batteryLevelLoaded | string | <sup>*(optional)*</sup> The loaded battery level |
| result?.remoteData?.batteryLevelPercentage | integer | <sup>*(optional)*</sup> The battery percentage |
| result?.remoteData?.batteryLastEvent | integer | <sup>*(optional)*</sup> The battery last event |
| result?.remoteData?.batteryLastEventTimestamp | integer | <sup>*(optional)*</sup> The timestamp of last battery event (in milliseconds) |
| result?.remoteData?.numVoiceCommandsPreviousDay | integer | <sup>*(optional)*</sup> Number of voice commands the previous day |
| result?.remoteData?.numVoiceCommandsCurrentDay | integer | <sup>*(optional)*</sup> Number of voice commands the current day |
| result?.remoteData?.numVoiceShortUtterancesPreviousDay | integer | <sup>*(optional)*</sup> Number of short voice commands the previous day |
| result?.remoteData?.numVoiceShortUtterancesCurrentDay | integer | <sup>*(optional)*</sup> Number of short voice commands the current day |
| result?.remoteData?.numVoicePacketsSentPreviousDay | integer | <sup>*(optional)*</sup> Number of voice packets sent the previous day |
| result?.remoteData?.numVoicePacketsSentCurrentDay | integer | <sup>*(optional)*</sup> Number of voice packets sent the current day |
| result?.remoteData?.numVoicePacketsLostPreviousDay | integer | <sup>*(optional)*</sup> Number of voice packets lost the previous day |
| result?.remoteData?.numVoicePacketsLostCurrentDay | integer | <sup>*(optional)*</sup> Number of voice packets lost the current day |
| result?.remoteData?.aveVoicePacketLossPreviousDay | string | <sup>*(optional)*</sup> Average number of voice packets lost the previous day |
| result?.remoteData?.aveVoicePacketLossCurrentDay | string | <sup>*(optional)*</sup> Average number of voice packets lost the current day |
| result?.remoteData?.numVoiceCmdsHighLossPreviousDay | integer | <sup>*(optional)*</sup> Number of voice commands exceeding packet loss threshold the previous day |
| result?.remoteData?.numVoiceCmdsHighLossCurrentDay | integer | <sup>*(optional)*</sup> Number of voice commands exceeding packet loss threshold the current day |
| result?.remoteData?.versionInfoSw | string | <sup>*(optional)*</sup> The software version of remote |
| result?.remoteData?.versionInfoHw | string | <sup>*(optional)*</sup> The hardware version of remote |
| result?.remoteData?.versionInfoIrdb | string | <sup>*(optional)*</sup> The IR database version of remote |
| result?.remoteData?.irdbType | integer | <sup>*(optional)*</sup> The IR database type |
| result?.remoteData?.irdbState | integer | <sup>*(optional)*</sup> The state of IR database |
| result?.remoteData?.programmedTvIRCode | string | <sup>*(optional)*</sup> The IR code programmed for TV |
| result?.remoteData?.programmedAvrIRCode | string | <sup>*(optional)*</sup> The IR code programmed for AVR |
| result?.remoteData?.lastCommandTimeDate | integer | <sup>*(optional)*</sup> The time when last key was pressed |
| result?.remoteData?.rf4ceRemoteSocMfr | string | <sup>*(optional)*</sup> The remote chipset manufacturer |
| result?.remoteData?.remoteMfr | string | <sup>*(optional)*</sup> The remote manufacturer |
| result?.remoteData?.signalStrengthPercentage | integer | <sup>*(optional)*</sup> The percentage of signal strength |
| result?.remoteData?.linkQuality | integer | <sup>*(optional)*</sup> The link quality |
| result?.remoteData?.howRemoteIsPaired | string | <sup>*(optional)*</sup> The binding type of remote |
| result?.remoteData?.bHasCheckedIn | boolean | <sup>*(optional)*</sup> `true` if the remote has checkedin for update and `false` otherwise |
| result?.remoteData?.bHasRemoteBeenUpdated | boolean | <sup>*(optional)*</sup> `true` if the remote's firmware is updated and `false` otherwise |
| result?.remoteData?.bIrdbDownloadSupported | boolean | <sup>*(optional)*</sup> `true` if the IR database download is supported and `false` otherwise |
| result?.remoteData?.pairingTimestamp | integer | <sup>*(optional)*</sup> The binding time (in milliseconds) |
| result?.remoteData?.lastRebootErrorCode | integer | <sup>*(optional)*</sup> The error code for previous reboot (in milliseconds) |
| result?.remoteData?.lastRebootTimestamp | integer | <sup>*(optional)*</sup> The previous reboot time (in milliseconds) |
| result?.remoteData?.securityType | integer | <sup>*(optional)*</sup> The security type |
| result?.remoteData?.bHasBattery | boolean | <sup>*(optional)*</sup> `true` if remote has battery and `false` otherwise |
| result?.remoteData?.batteryChangedTimestamp | integer | <sup>*(optional)*</sup> The time of battery change (in milliseconds) |
| result?.remoteData?.batteryChangedActualPercentage | integer | <sup>*(optional)*</sup> The battery changed actual percentage |
| result?.remoteData?.batteryChangedUnloadedVoltage | string | <sup>*(optional)*</sup> The battery changed unloaded voltage |
| result?.remoteData?.battery75PercentTimestamp | integer | <sup>*(optional)*</sup> The timestamp when battery was 75% (in milliseconds) |
| result?.remoteData?.battery75PercentActualPercentage | integer | <sup>*(optional)*</sup> The actual percentage for 75% battery |
| result?.remoteData?.battery75PercentUnloadedVoltage | string | <sup>*(optional)*</sup> The unloaded voltage of 75% battery |
| result?.remoteData?.battery50PercentTimestamp | integer | <sup>*(optional)*</sup> The timestamp when battery was 50% (in milliseconds) |
| result?.remoteData?.battery50PercentActualPercentage | integer | <sup>*(optional)*</sup> The actual percentage for 50% battery |
| result?.remoteData?.battery50PercentUnloadedVoltage | string | <sup>*(optional)*</sup> The unloaded voltage of 50% battery |
| result?.remoteData?.battery25PercentTimestamp | integer | <sup>*(optional)*</sup> The timestamp when battery was 25% (in milliseconds) |
| result?.remoteData?.battery25PercentActualPercentage | integer | <sup>*(optional)*</sup> The actual percentage for 25% battery |
| result?.remoteData?.battery25PercentUnloadedVoltage | string | <sup>*(optional)*</sup> The unloaded voltage of 25% battery |
| result?.remoteData?.battery5PercentTimestamp | integer | <sup>*(optional)*</sup> The timestamp when battery was 5% (in milliseconds) |
| result?.remoteData?.battery5PercentActualPercentage | integer | <sup>*(optional)*</sup> The actual percentage for 5% battery |
| result?.remoteData?.battery5PercentUnloadedVoltage | string | <sup>*(optional)*</sup> The unloaded voltage of 5% battery |
| result?.remoteData?.battery0PercentTimestamp | integer | <sup>*(optional)*</sup> The timestamp for 0% battery (in milliseconds) |
| result?.remoteData?.battery0PercentActualPercentage | integer | <sup>*(optional)*</sup> The actual percentage for 0% battery |
| result?.remoteData?.battery0PercentUnloadedVoltage | string | <sup>*(optional)*</sup> The unloaded voltage of 0% battery |
| result?.remoteData?.batteryVoltageLargeJumpCounter | integer | <sup>*(optional)*</sup> Number of large voltage jumps in battery |
| result?.remoteData?.batteryVoltageLargeDeclineDetected | boolean | <sup>*(optional)*</sup> `true` if large voltage decline in battery is detected, otherwise `false` |
| result?.remoteData?.bHasDSP | boolean | <sup>*(optional)*</sup> `true` if remote has digital signal processor and `false` otherwise |
| result?.remoteData?.averageTimeInPrivacyMode | integer | <sup>*(optional)*</sup> Average time in privacy mode (in milliseconds) |
| result?.remoteData?.bInPrivacyMode | boolean | <sup>*(optional)*</sup> `true` if remote is in privacy mode and `false` otherwise |
| result?.remoteData?.averageSNR | integer | <sup>*(optional)*</sup> Average signal to noise ratio |
| result?.remoteData?.averageKeywordConfidence | integer | <sup>*(optional)*</sup> Average keyword confidence |
| result?.remoteData?.totalNumberOfMicsWorking | integer | <sup>*(optional)*</sup> Number of mics working |
| result?.remoteData?.totalNumberOfSpeakersWorking | integer | <sup>*(optional)*</sup> Number of speakers working |
| result?.remoteData?.endOfSpeechInitialTimeoutCount | integer | <sup>*(optional)*</sup> Initial time out count after end of speech |
| result?.remoteData?.endOfSpeechTimeoutCount | integer | <sup>*(optional)*</sup> Time out count after end of speech |
| result?.remoteData?.uptimeStartTime | integer | <sup>*(optional)*</sup> Start of uptime (in milliseconds) |
| result?.remoteData?.uptimeInSeconds | integer | <sup>*(optional)*</sup> Uptime (in seconds) |
| result?.remoteData?.privacyTimeInSeconds | integer | <sup>*(optional)*</sup> Privacy time (in seconds) |
| result?.remoteData?.versionDSPBuildId | string | <sup>*(optional)*</sup> Version of dsp build ID |
| result.status_code | integer | An operations status code |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.ControlService.1.getSingleRemoteData",
    "params": {
        "remoteId": 1
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "remoteData": {
            "remoteId": 1,
            "remoteMACAddress": "0x00155F011C7F7359",
            "remoteModel": "XR15",
            "remoteModelVersion": "v1",
            "batteryLevelLoaded": "2.619608",
            "batteryLevelPercentage": 60,
            "batteryLastEvent": 0,
            "batteryLastEventTimestamp": 1602879639000,
            "numVoiceCommandsPreviousDay": 0,
            "numVoiceCommandsCurrentDay": 0,
            "numVoiceShortUtterancesPreviousDay": 0,
            "numVoiceShortUtterancesCurrentDay": 0,
            "numVoicePacketsSentPreviousDay": 0,
            "numVoicePacketsSentCurrentDay": 0,
            "numVoicePacketsLostPreviousDay": 0,
            "numVoicePacketsLostCurrentDay": 0,
            "aveVoicePacketLossPreviousDay": "0.000000",
            "aveVoicePacketLossCurrentDay": "0.000000",
            "numVoiceCmdsHighLossPreviousDay": 0,
            "numVoiceCmdsHighLossCurrentDay": 0,
            "versionInfoSw": "2.0.1.2",
            "versionInfoHw": "2.3.1.0",
            "versionInfoIrdb": "4.3.2.0",
            "irdbType": 0,
            "irdbState": 3,
            "programmedTvIRCode": "12731",
            "programmedAvrIRCode": "31360",
            "lastCommandTimeDate": 1580263335000,
            "rf4ceRemoteSocMfr": "QORVO",
            "remoteMfr": "RS",
            "signalStrengthPercentage": 50,
            "linkQuality": 0,
            "howRemoteIsPaired": "manual",
            "bHasCheckedIn": true,
            "bHasRemoteBeenUpdated": true,
            "bIrdbDownloadSupported": true,
            "pairingTimestamp": 1538782229000,
            "lastRebootErrorCode": 1538782229000,
            "lastRebootTimestamp": 12342000,
            "securityType": 0,
            "bHasBattery": true,
            "batteryChangedTimestamp": 1641294389000,
            "batteryChangedActualPercentage": 64,
            "batteryChangedUnloadedVoltage": "2.729412",
            "battery75PercentTimestamp": 1641294389000,
            "battery75PercentActualPercentage": 64,
            "battery75PercentUnloadedVoltage": "2.729412",
            "battery50PercentTimestamp": 0,
            "battery50PercentActualPercentage": 0,
            "battery50PercentUnloadedVoltage": "0.000000",
            "battery25PercentTimestamp": 0,
            "battery25PercentActualPercentage": 0,
            "battery25PercentUnloadedVoltage": "0.000000",
            "battery5PercentTimestamp": 0,
            "battery5PercentActualPercentage": 0,
            "battery5PercentUnloadedVoltage": "0.000000",
            "battery0PercentTimestamp": 0,
            "battery0PercentActualPercentage": 0,
            "battery0PercentUnloadedVoltage": "0.000000",
            "batteryVoltageLargeJumpCounter": 0,
            "batteryVoltageLargeDeclineDetected": false,
            "bHasDSP": false,
            "averageTimeInPrivacyMode": 0,
            "bInPrivacyMode": false,
            "averageSNR": 0,
            "averageKeywordConfidence": 0,
            "totalNumberOfMicsWorking": 0,
            "totalNumberOfSpeakersWorking": 0,
            "endOfSpeechInitialTimeoutCount": 0,
            "endOfSpeechTimeoutCount": 0,
            "uptimeStartTime": 1641460244000,
            "uptimeInSeconds": 0,
            "privacyTimeInSeconds": 0,
            "versionDSPBuildId": "..."
        },
        "status_code": 0,
        "success": true
    }
}
```

<a name="getValues"></a>
## *getValues*

Returns remote setting values. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty `params` object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.supportsASB | boolean | Whether the remote supports ASB |
| result.enableASB | boolean | Whether ASB is enabled |
| result.enableOpenChime | boolean | Whether the open chime is enabled |
| result.enableCloseChime | boolean | Whether the close chime is enabled |
| result.enablePrivacyChime | boolean | Whether the privacy chime is enabled |
| result.conversationalMode | integer | The conversational mode |
| result.chimeVolume | integer | The chime volume |
| result.irCommandRepeats | integer | The number of command repeats |
| result.status_code | integer | An operations status code |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.ControlService.1.getValues",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "supportsASB": true,
        "enableASB": false,
        "enableOpenChime": false,
        "enableCloseChime": true,
        "enablePrivacyChime": true,
        "conversationalMode": 6,
        "chimeVolume": 1,
        "irCommandRepeats": 3,
        "status_code": 0,
        "success": true
    }
}
```

<a name="setValues"></a>
## *setValues*

Sets remote setting values. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.enableASB | boolean | <sup>*(optional)*</sup> Whether ASB is enabled |
| params?.enableOpenChime | boolean | <sup>*(optional)*</sup> Whether the open chime is enabled |
| params?.enableCloseChime | boolean | <sup>*(optional)*</sup> Whether the close chime is enabled |
| params?.enablePrivacyChime | boolean | <sup>*(optional)*</sup> Whether the privacy chime is enabled |
| params?.conversationalMode | integer | <sup>*(optional)*</sup> The conversational mode |
| params?.chimeVolume | integer | <sup>*(optional)*</sup> The chime volume |
| params?.irCommandRepeats | integer | <sup>*(optional)*</sup> The number of command repeats |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.status_code | integer | An operations status code |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.ControlService.1.setValues",
    "params": {
        "enableASB": false,
        "enableOpenChime": false,
        "enableCloseChime": true,
        "enablePrivacyChime": true,
        "conversationalMode": 6,
        "chimeVolume": 1,
        "irCommandRepeats": 3
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "status_code": 0,
        "success": true
    }
}
```

<a name="startPairingMode"></a>
## *startPairingMode*

Enters pairing mode. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.pairingMode | integer | The pairing mode |
| params.restrictPairing | integer | The pairing restrictions |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.status_code | integer | An operations status code |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.ControlService.1.startPairingMode",
    "params": {
        "pairingMode": 0,
        "restrictPairing": 0
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "status_code": 0,
        "success": true
    }
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.ControlService plugin:

ControlService interface events:

| Event | Description |
| :-------- | :-------- |
| [onControl](#onControl) | Triggered on control manager events |
| [onXRConfigurationComplete](#onXRConfigurationComplete) | Triggered on control manager configuration complete event |
| [onXRPairingStart](#onXRPairingStart) | Triggered on control manager validation/pairing key press event |
| [onXRValidationComplete](#onXRValidationComplete) | Triggered on control manager validation/pairing end event |
| [onXRValidationUpdate](#onXRValidationUpdate) | Triggered on control manager events |


<a name="onControl"></a>
## *onControl*

Triggered on control manager events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.remoteId | integer | The remote control identifier |
| params.eventValue | integer | The event value |
| params.eventSource | string | The event source |
| params.eventType | string | The event type |
| params.eventData | string | The event Data |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onControl",
    "params": {
        "remoteId": 1,
        "eventValue": 0,
        "eventSource": "...",
        "eventType": "...",
        "eventData": "..."
    }
}
```

<a name="onXRConfigurationComplete"></a>
## *onXRConfigurationComplete*

Triggered on control manager configuration complete 

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.remoteId | integer | The remote control identifier |
| params.remoteType | string | The remote control type |
| params.bindingType | integer | The remote control binding type |
| params.configurationStatus | integer | The remote control configuration status |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onXRConfigurationComplete",
    "params": {
        "remoteId": 1,
        "remoteType": "XR11",
        "bindingType": 1,
        "configurationStatus": 1
    }
}
```

<a name="onXRPairingStart"></a>
## *onXRPairingStart*

Triggered on control manager validation/pairing key press 

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.remoteId | integer | The remote control identifier |
| params.remoteType | string | The remote control type |
| params.bindingType | integer | The remote control binding type |
| params.validationDigit1 | integer | The first digit for pairing |
| params.validationDigit2 | integer | The second digit for pairing |
| params.validationDigit3 | integer | The third digit for pairing |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onXRPairingStart",
    "params": {
        "remoteId": 1,
        "remoteType": "XR11",
        "bindingType": 1,
        "validationDigit1": 1,
        "validationDigit2": 3,
        "validationDigit3": 5
    }
}
```

<a name="onXRValidationComplete"></a>
## *onXRValidationComplete*

Triggered on control manager validation/pairing end   
Validation status codes:  
* `0` - VALIDATION_SUCCESS   
* `1` - VALIDATION_TIMEOUT  
* `2` - VALIDATION_FAILURE  
* `3` - VALIDATION_ABORT  
* `4` - VALIDATION_WRONG_CODE.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.remoteId | integer | The remote control identifier |
| params.remoteType | string | The remote control type |
| params.bindingType | integer | The remote control binding type |
| params.validationStatus | integer | The validation status code |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onXRValidationComplete",
    "params": {
        "remoteId": 1,
        "remoteType": "XR11",
        "bindingType": 1,
        "validationStatus": 0
    }
}
```

<a name="onXRValidationUpdate"></a>
## *onXRValidationUpdate*

Triggered on control manager events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.remoteId | integer | The remote control identifier |
| params.remoteType | string | The remote control type |
| params.bindingType | integer | The remote control binding type |
| params.validationDigit1 | integer | The first digit for pairing |
| params.validationDigit2 | integer | The second digit for pairing |
| params.validationDigit3 | integer | The third digit for pairing |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onXRValidationUpdate",
    "params": {
        "remoteId": 1,
        "remoteType": "XR11",
        "bindingType": 1,
        "validationDigit1": 1,
        "validationDigit2": 3,
        "validationDigit3": 5
    }
}
```

