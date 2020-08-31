# **System Thunder Plugin** (_RDK-25849_)
    This document enlists the supported Methods and Events of System Thunder Plugin. The following section details each.
* [**Methods**](#System-Thunder-Plugin-Methods)
    - The generic thunder JSONRPC API format is `{"jsonrpc":"2.0","id":"","method":"callsign.pluginVersion.methodName",<payload>}` where default value of `callsign` is `org.rdk.System`, `pluginVersion` is `1` and `methodName` is the methods listed in `Methods` section. `Payload` information is implementation specific to each `method` and is described under respective `method` section.
* [**Events**](#System-Thunder-Plugin-Events)
    - These JSONRPC events shall be broadcasted to all subscribed endpoints over websocket. Each `event` payload details are enlisted under respective section below.
* [**Error codes & messages**](#System-Thunder-Plugin-ErrorCodes)
    - This section details system plugin specific error codes & messages which will be generated when `methods/events` fail to perform the requested service.    
* [**Tests**](#System-Thunder-Plugin-Test-Client)
* [**Examples**](#System-Thunder-Plugin-Method-and-Event-Examples)

# **Build Steps**
    bitbake thunder-plugins


## System Thunder Plugin Methods
  - **cacheContains**

    To check if key value present in cache.  
  _**Request payload:**_ `{"params":{"key":"<string>"}}`  
  _**Response payload:**_ `{"result":{"success":<bool>}}`
  - **clearLastDeepSleepReason**

    Used to clear last deep sleep reason.  
  _**Request payload:**_ `{"params":{}}`
  _**Response payload:**_ `{"result":{"success":<bool>}}`
  - **enableMoca/disableMoca**

    Supports to enable/disable Moca Settings.  
  _**Request payload:**_ `{"params":{"value":<bool>}}`  
  _**Response payload:**_ `{"success":<bool>}`
  - **getAvailableStandbyModes**

    Returns an array of strings containing the supported standby modes. Possible values are "LIGHT_SLEEP" and/or "DEEP_SLEEP".  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{"result":{"supportedStandbyModes":["<string1>", "<string2>"],"success":<bool>}}`
  - **getCachedValue**

    To get cached value.  
  _**Request payload:**_ `{"params":{"key":"<string>"}}`  
  _**Response payload:**_ `{"result":{"<cachekey>":"<string>","success":<bool>}}`
  - **getCoreTemperature**

    To fetch core temperature.  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{"result":{"temperature":<float>,"success":<bool>}}`
  - **getDeviceInfo**

    This method supports to query DeviceInfo details.  
  _**Request payload:**_ `{"params":{"params":["<key>"]}}`  
  _**Response payload:**_ `{"result":{<key>:<Device Info Details>,"success":<bool>}}`
  - **getDownloadedFirmwareInfo**

    Retrieves firmware downloaded info.  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{"result":{"currentFWVersion":"<string>","downloadedFWVersion":"<string>""downloadedFWLocation":"<string>","isRebootDeferred":<bool>, "success":<bool>}}`
  - **getFirmwareDownloadPercent**

    To fetch Firmware Download Percentage Information.  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{"result":{"downloadPercent":<long int>, "success":false}}`
  - **getFirmwareUpdateInfo**

    To check Firmware Update Information. The requested details shall be sent as an event `onFirmwareUpdateInfoReceived`.  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{"result":{"asyncResponse":true,"success":true}}`
  - **getFirmwareUpdateState**

    Gets firmware update state.  
  _**Request payload:**_ `{params":{}}`  
  _**Response payload:**_ `{"result":{"firmwareUpdateState":<string fw update state>,"success":<bool>}}`
  - **getLastDeepSleepReason**

    Used to retrieve last deep sleep reason.  
  _**Request payload:**_ `{params":{}}`  
  _**Response payload:**_ `{"result":{"lastDeepSleepReason":"<string>","success":<bool>}}`
  - **getMacAddresses**

    To get device mac addresses. The requested details shall be sent as event `onMacAddressesRetreived`.  
  _**Request payload:**_ `{params":{}}`  
  _**Response payload:**_ `{"result":{"asyncResponse":<bool>,"success":<bool>}}`
  - **getMilestones**

    To fetch the list of milestones.  
  _**Request payload:**_ `{params":{}}`  
  _**Response payload:**_ `{"result":{"milestones":["<string>","<string>","<string>"],"success":<bool>}}`
  - **getMode**

    Returns mode Information, defines two parameters mode and duration.  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{"result":{"modeInfo":{"mode":"<string>","duration":<int>},"success":<bool>}}`
  - **getPowerState**

    To retrieve Device Power State.  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{"result":{"powerState":"<mode>","success":<bool>}}`
  - **getPreferredStandbyMode**

    Returns the preferred standby mode. Possible values are either "LIGHT_SLEEP" or "DEEP_SLEEP". This Will return an empty string if the preferred mode has not been set.  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{"result":{"preferredStandbyMode":"<string>","success":<bool>}}`
  - **getPreviousRebootInfo**

    To get previous reboot information.  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{"result":{"timeStamp":"<string>","reason":"<string>","success":<bool>}}`
  - **getPreviousRebootInfo2**

    Used to retrieve previous reboot details.  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{"result":{"rebootInfo":{"timestamp":"string","source":"string","reason":"string","customReason":"string","lastHardPowerReset":"<string>"},"success":<bool>}}`
  - **getPreviousRebootReason**

    Used to retrieve last reboot reason.  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{"result":{"rebootReason":<string>","success":<bool>}}`
  - **getRFCConfig**

    To retrieve information related to RFC Configurations.  
  _**Request payload:**_ `{"params":{"rfcList":["<rfc1>","<rfc2>"]}}`  
  _**Response payload:**_ `{"result":{"RFCConfig":{"<rfc1>":"<value>","<rfc2>":"<value>"},"success":<bool>}}`
  - **getSerialNumber**

    To retrieve Device Serial Number.  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{"result":{"serialNumber":"<string>","success":true}}`
  - **getStateInfo**

    To query device state information of various properties.  
  _**Request payload:**_ `{"params":{"param":"<queryState>"}`  
  _**Response payload:**_ `{"result":{"<queryState>":<value>,"success":<bool>}}`
  - **getSystemVersions**

    To retrieve system version details.  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{"result":{"stbVersion":"<string>", "receiverVersion":"<string>", "stbTimestamp":"<string>","success":<bool>}}`
  - **getTemperatureThresholds**

    To retrieve Temperature Threshold values.  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{"result":{"temperatureThresholds":{"WARN":"100.000000","MAX":"200.000000","temperature":"62.000000"},"success":<bool>}}`
  - **getTimeZoneDST**

    To fetch timezone from TZ_FILE.  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{","id":3,"result":{"timeZone":"<String>","success":<bool>}}`
  - **getXconfParams**

    This will return configuration parameters such as firmware version, Mac, Model etc.  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{"result":{"xconfParams":{"eStbMac":"<string>","firmwareVersion":"<string>", "env":"<string>","model":"<string>"},"success":<bool>}}`
  - **hasRebootBeenRequested**

    To check if Reboot has been requested or not.  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{"result":{"rebootRequested":false,"success":<bool>}}`
  - **isGzEnabled**

    To check GZ Status.  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{"result":{"enabled":false,"success":true}}`
  - **queryMocaStatus**

    Checks if Moca is Enabled or Not.  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{"result":{"mocaEnabled":<bool>,"success":<bool>}}`
  - **reboot**

    This method shall be used to request the system to perform `reboot`.  
  _**Request payload:**_ `{"params":{"rebootReason":"API Validation"}}`  
  _**Response payload:**_ `{"result":{"IARM_Bus_Call_STATUS":<int>,"success":<bool>}}`
  - **removeCacheKey**

    To delete the key value present in cache.  
  _**Request payload:**_ `{"params":{"key":"<string>"}}`  
  _**Response payload:**_ `{"result":{"success":<bool>}}`
  - **requestSystemUptime**

    To fetch device uptime.  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{"result":{"systemUptime":"378641.03","success":true}}`
  - **setCachedValue**

    To set cache value.  
  _**Request payload:**_ `{"params":{"key":"<string>","value":<double>}}`  
  _**Response payload:**_ `{"result":{"success":<bool>}}`
  - **setDeepSleepTimer**

    Sets the deep sleep time out period, specified in seconds by invoking the corresponding system service method.  
  _**Request payload:**_ `{"params":{"seconds":<unsigned int>}}`  
  _**Response payload:**_ `{"result":{"success":<bool>}}`
  - **setGzEnabled**

    To set GZ Status.  
  _**Request payload:**_ `{"params":{"enabled":true}}`  
  _**Response payload:**_ `{"result":{"success":<bool>}}`
  - **setMode**

    Sets the mode of the STB. The object consists of two properties, mode (String) and duration (Integer) mode can be one of following: `NORMAL` - the STB is operating in normal mode, `EAS` - the STB is operating in EAS mode. This mode is set when the device needs to perform certain tasks when entering EAS mode, such as setting the clock display, or preventing the user from using the diagnostics menu. `WAREHOUSE` - the STB is operating in warehouse mode.  
  _**Request payload:**_ `{"params":{"modeInfo":{"mode":"<string>","duration":<int>}}}`  
  _**Response payload:**_ `{"result":{"success":<bool>}}`
  - **setPowerState**

    To set the Device Power State. The possible values are `STANDBY,DEEP_SLEEP,LIGHT_SLEEP,ON`.  
  _**Request payload:**_ `{"params":{"powerState":<string>, "standbyReason":<string>}}`  
  _**Response payload:**_ `{"result":{"success":<bool>}}`
  - **setPreferredStandbyMode**

    Sets and persists the preferred standby mode.  
    Invoking this function does not change the power state of the device, but sets only the user preference for preferred action when setPowerState is invoked with a value of "STANDBY".  
  _**Request payload:**_ `{"params":{"standbyMode":"<string>"}}`  
  _**Response payload:**_ `{"result":{"success":<bool>}}`
  - **setTemperatureThresholds**

    To set Temperature Threshold values.  
  _**Request payload:**_ `{"params":{"thresholds":{"WARN":"99.000000","MAX":"199.000000"}}}`  
  _**Response payload:**_ `{"result":{"success":<bool>}}`
  - **setTimeZoneDST**

    To set the Time to System TZ_FILE.  
  _**Request payload:**_ `{"params":{"timeZone":"<string>"}}`  
  _**Response payload:**_ `{"result":{"success":<bool>}}`
  - **updateFirmware**

    Initiates a firmware update. This method has no affect if update is not available. The State Observer Method/Event may be used to listen to firmware update events.  
  _**Request payload:**_ `{"params":{}}`  
  _**Response payload:**_ `{"result":{"success":<bool>}}`
## System Thunder Plugin Events
  - **onFirmwareUpdateInfoReceived**

    This JSONRPC event is triggered with respect to the asynchronous method `getFirmwareUpdateInfo` and will have payload details `{"param":{"firmwareUpdateState":<enum:0-6>}}`.    
  - **onFirmwareUpdateStateChange**

    This JSONRPC event is triggered as a result of firmware update state change with the following payload `{"param":{"firmwareUpdateState":<enum:0-6>}}`  
  - **onMacAddressesRetreived**

    This JSONRPC event is generated as a result of `getMacAddresses` asynchronous method with payload information `{"param":{"ecm_mac":"<MAC>","estb_mac":"<MAC>","moca_mac":"<MAC>","eth_mac":"<MAC>","wifi_mac":"<MAC>","info":"<Details fetch status>","success":<bool>}}`  
  - **onSystemModeChanged**

    When the device operating mode changes from one mode to other, this JSONRPC event is generated with payload `{"param":{"mode": <string_mode>}}`  
  - **onSystemPowerStateChanged**

    When the power manager detects device power state change; this JSONRPC event is triggered with payload `{"param":{"powerState": <string new power state mode>}}`  
  - **onTemperatureThresholdChanged**

    When Temperature Threshold is changed beyond the set WARN and MAX limits(See method `setTemperatureThresholds`), this JSONRPC event is triggered with the following payload
    `{"param":{"thresholdType":"<limit type>","exceeded":<bool>,"temperature":"<current temperature in float>"}}`  
  - **onRebootRequest**

    When any application invokes `reboot` method, this JSONRPC event is generated with payload `{"param":{"requestedApp": <string_requesting_application_name>, "rebootReason":<string_reboot_reason>}}`  
## System Thunder Plugin ErrorCodes    
    System plugin specific error codes and messages are listed below.    
   |Enum Name|Error Code|Error Message|Occation/Reason|
   |:---|:---|:---|:---|
   |SysSrv_OK|0|Processed Successfully|Usually this message shall not be added to response payload|
   |SysSrv_MethodNotFound|1|Method not found|When requested method is not supported|
   |SysSrv_MissingKeyValues|2|Missing required key/value(s)|When required key/value(s) are not available to perform execution|
   |SysSrv_UnSupportedFormat|3|Unsupported or malformed format|When expected key/value type is not found|
   |SysSrv_FileNotPresent|4|Expected file not found|When some required file(s) are missing for processing|
   |SysSrv_FileAccessFailed|5|File access failed|When the underlying process is not able to access the required file(s)|
   |SysSrv_FileContentUnsupported|6|Unsupported file content|When the underlying process is not able extract the required content from file(s)|
   |SysSrv_Unexpected|7|Unexpected error|When something unexpected happens while processing the request|
   |SysSrv_SupportNotAvailable|8|Support not available/enabled|When requested support is not enabled from platform(unmet dependencies)|
   |SysSrv_LibcurlError|9|LIbCurl service error|Not able to use the underlying curl library to process|
   |SysSrv_DynamicMemoryAllocationFailed|10|Dynamic Memory Allocation Failed|When memory alloc fails|
   |SysSrv_ManufacturerDataReadFailed|11|Manufacturer Data Read Failed|Unmet/unsupported manufacturer implementation(s)/change(s)|
## System Thunder Plugin Test Client
    The given C++ test client application can be utilized to validate the above mentioned `methods` and `events`. Use `-DBUILD_TESTS` in the configuration option to enable building of C++ test client. Use `-DDEBUG=1` to build sample Method which will generate a sample Event for Test Client verification.  
  **Sample Method/Event Usage :**  
  _**Request payload:**_ `{{"params":<request payload>}}`  
  _**Response payload:**_ onSampleEvent : `{<payload received in the request>}`
## System Thunder Plugin Method and Event Examples
<details>
    <summary>Click to get details</summary>
  The Thunder configuration used for below `curl` commandline utility based Request/Response samples use `PORT 9998`.  
  
  **API request format**  
  `curl --header "Content-Type:application/json" --request POST http://localhost:9998/jsonrpc --data-raw '<request_payload>'` 

_Note:_ Here callsign used is `org.rdk.System` instead of actual `org.rdk.System` since testing purpose it was set so.

Method | Request Payload | Response Payload
:--- | :--- | :---
| cacheContains | {"jsonrpc":"2.0","id":"1","method":"org.rdk.System.1.cacheContains","params":{"key":"sampleCache"}} |  {"jsonrpc":"2.0","id":1,"result":{"success":true}} |  
| clearLastDeepSleepReason | {"jsonrpc":"2.0","id":"29","method":"org.rdk.System.1.clearLastDeepSleepReason","params":{}} | {"jsonrpc":"2.0","id":29,"result":{"success":true}} |  
| enableMoca | {"jsonrpc":"2.0","id":"30","method": "org.rdk.System.1.enableMoca","params":{"value":false}} | {"jsonrpc":"2.0","id":30,"result":{"success":false}} |  
| getAvailableStandbyModes | {"jsonrpc":"2.0","id":"2","method":"org.rdk.System.1.getAvailableStandbyModes","params":{}} | {"jsonrpc":"2.0","id":2,"result":{"supportedStandbyModes":["LIGHT_SLEEP","DEEP_SLEEP"],"success":true}} |  
| getCachedValue | {"jsonrpc":"2.0","id":"3","method":"org.rdk.System.1.getCachedValue","params":{"key":"sampleCache"}} | {"jsonrpc":"2.0","id":3,"result":{"sampleCache":"4343.3434","success":true}} |  
| getCoreTemperature | {"jsonrpc":"2.0","id":"4","method":"org.rdk.System.1.getCoreTemperature","params":{}} | {"jsonrpc":"2.0","id":4,"result":{"temperature":"48.000000","success":true}} |  
| getDeviceInfo | {"jsonrpc":"2.0","id":"5","method":"org.rdk.System.1.getDeviceInfo","params":{"params":["estb_mac"]}} | {"jsonrpc":"2.0","id":5,"result":{"estb_mac":"20:F1:9E:EE:62:08","success":true}} |  
| getDownloadedFirmwareInfo | {"jsonrpc":"2.0","id":"6","method":"org.rdk.System.1.getDownloadedFirmwareInfo","params":{}} | {"jsonrpc":"2.0","id":6,"result":{"currentFWVersion":"AX061AEI_VBN_1911_sprint_20200109040424sdy","downloadedFWVersion":"","downloadedFWLocation":"","isRebootDeferred":false,"success":true}} |  
| getFirmwareDownloadPercent | {"jsonrpc":"2.0","id":"7","method":"org.rdk.System.1.getFirmwareDownloadPercent","params":{}} | {"jsonrpc":"2.0","id":7,"result":{"SysSrv_Status":3,"errorMessage":"Expected file not found","success":false}} |  
| getFirmwareUpdateInfo | {"jsonrpc":"2.0","id":"8","method":"org.rdk.System.1.getFirmwareUpdateInfo","params":{"GUID":"string"}} | {"jsonrpc":"2.0","id":8,"result":{"asyncResponse":true,"success":true}} |  
| getFirmwareUpdateState | {"jsonrpc":"2.0","id":"9","method":"org.rdk.System.1.getFirmwareUpdateState","params":{}} | {"jsonrpc":"2.0","id":9,"result":{"firmwareUpdateState":0,"success":true}} |  
| getLastDeepSleepReason | {"jsonrpc":"2.0","id":"10","method":"org.rdk.System.1.getLastDeepSleepReason","params":{}} | {"jsonrpc":"2.0","id":10,"result":{"SysSrv_Status":3,"errorMessage":"Expected file not found","success":false}} |  
| getMacAddresses | {"jsonrpc":"2.0","id":"11","method":"org.rdk.System.1.getMacAddresses","params":{"GUID":"61734787891723481"}} | {"jsonrpc":"2.0","id":11,"result":{"asyncResponse":true,"success":true}} |  
| getMilestones | {"jsonrpc":"2.0","id":"12","method":"org.rdk.System.1.getMilestones","params":{}} | {"jsonrpc":"2.0","id":12,"result":{"milestones":["RDK_STARTED:20410","2020 Jan 28 08:24:06.762355 arrisxi6 systemd[1]: Starting Log RDK Started Service...","-- Subject: Unit log-rdk-start.service has begun with start-up","-- Defined-By: systemd","-- Support: http://lists.freedesktop.org/mailman/listinfo/systemd-devel","-- ","-- Unit log-rdk-start.service has begun starting up.","2020 Jan 28 08:24:06.791823 arrisxi6 systemd[1]: Started Log RDK Started Service.","-- Subject: Unit log-rdk-start.service has finished start-up","-- Defined-By: systemd","-- Support: http://lists.freedesktop.org/mailman/listinfo/systemd-devel","-- ","-- Unit log-rdk-start.service has finished starting up.","-- ","-- The start-up result is done.","UPNP_START_DISCOVERY:52660","IP_ACQUISTION_COMPLETED:53230","PWIFI_CONNECTED:56270","SEND_DHCP_REQUEST:56370","RECV_DHCP_RESPONSE:56940","IP_ACQUISTION_COMPLETED:57070","CONNECT_TO_APPLICATION:78340","CONNECT_TO_NTP_SERVER:79460","SPLASH_SCREEN_DISPLAYED:81300","SET_XSCT_TOKEN:106480","CONNECT_TO_APPLICATION:113490","CONNECT_TO_APPLICATION:116590","CONNECT_TO_APPLICATION:119460"],"success":true}} |  
| getMode | {"jsonrpc":"2.0","id":"13","method":"org.rdk.System.1.getMode","params":{}} | {"jsonrpc":"2.0","id":13,"result":{"modeInfo":{"mode":"NORMAL","duration":0},"success":true}} |  
| getPowerState | {"jsonrpc":"2.0","id":"14","method":"org.rdk.System.1.getPowerState","params":{}} | {"jsonrpc":"2.0","id":14,"result":{"powerState":"ON","success":true}} |  
| getPreferredStandbyMode | {"jsonrpc":"2.0","id":"15","method":"org.rdk.System.1.getPreferredStandbyMode","params":{}} | {"jsonrpc":"2.0","id":15,"result":{"preferredStandbyMode":"DEEP_SLEEP","success":true}} |  
| getPreviousRebootInfo2 | {"jsonrpc":"2.0","id":"17","method":"org.rdk.System.1.getPreviousRebootInfo2","params":{}} | {"jsonrpc":"2.0","id":17,"result":{"rebootInfo":{"timestamp":"Tue Jan 28 08:35:40 UTC 2020","source":"SystemPlugin","reason":"FIRMWARE_FAILURE","customReason":"API Validation","lastHardPowerReset":"Tue Jan 28 08:22:22 UTC 2020"},"success":true}} |  
| getPreviousRebootInfo | {"jsonrpc":"2.0","id":"16","method":"org.rdk.System.1.getPreviousRebootInfo","params":{}} | {"jsonrpc":"2.0","id":16,"result":{"timeStamp":"20200128083540","reason":" Triggered from System Plugin API Validation","success":true}} |  
| getPreviousRebootReason | {"jsonrpc":"2.0","id":"18","method":"org.rdk.System.1.getPreviousRebootReason","params":{}} | {"jsonrpc":"2.0","id":18,"result":{"reason":"FIRMWARE_FAILURE","success":true}} |  
| getRFCConfig | {"jsonrpc":"2.0","id":"19","method":"org.rdk.System.1.getRFCConfig","params":{"rfcList":["Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AccountInfo.AccountID","Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.UPnP.Enabled","Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AllowOpenPorts.Enabled","Device.DeviceInfo.MemoryStatus.Total","Device.DeviceInfo.MemoryStatus.Free"]}} | {"jsonrpc":"2.0","id":19,"result":{"RFCConfig":{"Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AccountInfo.AccountID":"1181405614235499371","Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.UPnP.Enabled":"Empty response received","Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AllowOpenPorts.Enabled":"Empty response received","Device.DeviceInfo.MemoryStatus.Total":"1144560","Device.DeviceInfo.MemoryStatus.Free":"732144"},"success":true}} |  
| getSerialNumber | {"jsonrpc":"2.0","id":"20","method":"org.rdk.System.1.getSerialNumber","params":{}} | {"jsonrpc":"2.0","id":20,"result":{"serialNumber":"M11806TK0024","success":true}} |  
| getStateInfo | {"jsonrpc":"2.0","id":"43","method":"org.rdk.System.1.getStateInfo","params":{"param":"com.comcast.channel_map"}} | {"jsonrpc":"2.0","id":43,"result":{"com.comcast.channel_map":2,"success":true}} |  
| getSystemVersions | {"jsonrpc":"2.0","id":"21","method":"org.rdk.System.1.getSystemVersions","params":{}} | {"jsonrpc":"2.0","id":21,"result":{"stbVersion":"AX061AEI_VBN_1911_sprint_20200109040424sdy","receiverVersion":"3.14.0.0","stbTimestamp":"Thu 09 Jan 2020 04:04:24 AP UTC","success":true}} |  
| getTemperatureThresholds | {"jsonrpc":"2.0","id":"22","method":"org.rdk.System.1.getTemperatureThresholds","params":{}} | {"jsonrpc":"2.0","id":22,"result":{"temperatureThresholds":{"WARN":"100.000000","MAX":"110.000000","temperature":"48.000000"},"success":true}} |  
| getTimeZoneDST | {"jsonrpc":"2.0","id":"23","method":"org.rdk.System.1.getTimeZoneDST","params":{}} | {"jsonrpc":"2.0","id":23,"result":{"timeZone":"America/New_York","success":true}} |  
| getXconfParams | {"jsonrpc":"2.0","id":"24","method":"org.rdk.System.1.getXconfParams","params":{}} | {"jsonrpc":"2.0","id":24,"result":{"xconfParams":{"env":"vbn","eStbMac":"20:F1:9E:EE:62:08","model":"AX061AEI","firmwareVersion":"AX061AEI_VBN_1911_sprint_20200109040424sdy"},"success":true}} |  
| hasRebootBeenRequested | {"jsonrpc":"2.0","id":"25","method":"org.rdk.System.1.hasRebootBeenRequested","params":{}} | {"jsonrpc":"2.0","id":25,"result":{"rebootRequested":false,"success":true}} |  
| isGzEnabled | {"jsonrpc":"2.0","id":"26","method":"org.rdk.System.1.isGzEnabled","params":{}} | {"jsonrpc":"2.0","id":26,"result":{"enabled":false,"success":true}} |  
| queryMocaStatus | {"jsonrpc":"2.0","id":"27","method": "org.rdk.System.1.queryMocaStatus","params":{}} | {"jsonrpc":"2.0","id":27,"result":{"mocaEnabled":false,"success":true}} |  
| reboot | {"jsonrpc":"2.0","id":"100","method":"org.rdk.System.1.reboot","params":{"reboorReason":"API Validation"}} | {"jsonrpc":"2.0","id":3,"result":{"IARM_Bus_Call_STATUS":1,"success":true}} |  
| removeCacheKey | {"jsonrpc":"2.0","id":"34","method":"org.rdk.System.1.removeCacheKey","params":{"key":"sampleCache"}} | {"jsonrpc":"2.0","id":34,"result":{"success":true}} |  
| requestSystemUptime | {"jsonrpc":"2.0","id":"28","method":"org.rdk.System.1.requestSystemUptime","params":{}} | {"jsonrpc":"2.0","id":28,"result":{"systemUptime":"1666.92","success":true}} |  
| setCachedValue | {"jsonrpc":"2.0","id":"33","method":"org.rdk.System.1.setCachedValue","params":{"key":"sampleCache","value":4343.3434}} | {"jsonrpc":"2.0","id":33,"result":{"success":true}} |  
| setDeepSleepTimer | {"jsonrpc":"2.0","id":"35","method":"org.rdk.System.1.setDeepSleepTimer","params":{"seconds":3}} | {"jsonrpc":"2.0","id":35,"result":{"success":true}} |  
| setGzEnabled | {"jsonrpc":"2.0","id":"36","method":"org.rdk.System.1.setGzEnabled","params":{"enabled":true}} | {"jsonrpc":"2.0","id":36,"result":{"success":true}} |  
| setMode | {"jsonrpc":"2.0","id":"37","method":"org.rdk.System.1.setMode","params":{"modeInfo":{"mode":"NORMAL","duration":20}}} | {"jsonrpc":"2.0","id":37,"result":{"success":true}} |  
| setPowerState | {"jsonrpc":"2.0","id":"38","method":"org.rdk.System.1.setPowerState","params":{"powerState":"ON", "standbyReason":"APIUnitTest"}} | {"jsonrpc":"2.0","id":38,"result":{"success":true}} |  
| setPreferredStandbyMode | {"jsonrpc":"2.0","id":"39","method":"org.rdk.System.1.setPreferredStandbyMode","params":{"standbyMode":"DEEP_SLEEP"}} | {"jsonrpc":"2.0","id":39,"result":{"success":true}} |  
| setTemperatureThresholds | {"jsonrpc":"2.0","id":"40","method":"org.rdk.System.1.setTemperatureThresholds","params":{"thresholds":{"WARN":"50.000000","MAX":"80.000000"}}} | {"jsonrpc":"2.0","id":40,"result":{"success":true}} |  
| setTimeZoneDST | {"jsonrpc":"2.0","id":"41","method":"org.rdk.System.1.setTimeZoneDST","params":{"timeZone":"UTC-5"}} | {"jsonrpc":"2.0","id":41,"result":{"success":true}} |  
| updateFirmware | {"jsonrpc":"2.0","id":"42","method":"org.rdk.System.1.updateFirmware","params":{}} | {"jsonrpc":"2.0","id":42,"result":{"success":true}} |  
</details>
