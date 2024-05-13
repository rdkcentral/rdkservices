# Changelog

All notable changes to this RDK Service will be documented in this file.

* Each RDK Service has a CHANGELOG file that contains all changes done so far. When version is updated, add a entry in the CHANGELOG.md at the top with user friendly information on what was changed with the new version. Please don't mention JIRA tickets in CHANGELOG. 

* Please Add entry in the CHANGELOG for each version change and indicate the type of change with these labels:
    * **Added** for new features.
    * **Changed** for changes in existing functionality.
    * **Deprecated** for soon-to-be removed features.
    * **Removed** for now removed features.
    * **Fixed** for any bug fixes.
    * **Security** in case of vulnerabilities.

* Changes in CHANGELOG should be updated when commits are added to the main or release branches. There should be one CHANGELOG entry per JIRA Ticket. This is not enforced on sprint branches since there could be multiple changes for the same JIRA ticket during development. 

* For more details, refer to [versioning](https://github.com/rdkcentral/rdkservices#versioning) section under Main README.
## [1.4.3] - 2024-04-29
### Added
- Fail safe check for CEC enable.

## [1.4.2] - 2024-04-18
### Added
- Retrigger the SAD request after 3sec timeout of SAD update.

## [1.4.1] - 2024-03-29
### Security
- Resolved security vulnerabilities

## [1.4.0] - 2024-03-07
### Fixed
- Register the Event IARM_BUS_DSMGR_EVENT_ATMOS_CAPS_CHANGED

## [1.3.9] - 2024-02-02
### Fixed
- Display settings cache implemented

## [1.3.8] - 2023-11-21
### Fixed
- Fixed missing resolutions in getSupportedTvResolutions API list

## [1.3.7] - 2023-11-08
### Fixed
- RDKServices:Event not triggering for Volume level 'zero'

## [1.3.6] - 2023-10-04
### Added
- Implement Thunder Plugin Configuration for Kirkstone builds(CMake-3.20 & above)

## [1.3.5] - 2023-09-12
### Fixed
Fixed the errors in DisplaySettingsPLugin.md file 

## [1.3.4] - 2023-09-05
### Fixed
Updated getCurrentResolution API to return resolution along with width, height and signal type.

## [1.3.3] - 2023-08-22
### Added
- Added Documentation for AC4 implementation in DisaplaySettings

## [1.3.2] - 2023-08-23
### Fixed
- Handled case when HPD of AVR connection comes delayed. Reintiate ARC/eARC connection in this case.

## [1.3.1] - 2023-08-03
### Fixed
- Fixed issues when ARC init and audio disconnected events occur at same time in some AVR
- Fixed issues when Audio routing in progress and user disables CEC through UI

## [1.3.0] - 2023-08-02
### Added
- Added feature to retrieve platform supported ms12 config type.

##[1.2.1] - 2023-07-19

- Adding UtilsSynchro.hpp to provide tools for automatically synchronizing API calls for given plugin
- Added UtilsSynchroIarm.hpp to additionally synchronize iarm event handlers.

## [1.2.0] - 2023-06-12
### Fixed
- Added event for volume increase/decrease/mute/unmute events

## [1.1.0] - 2023-06-08
### Changed
- Handled identifying Atmos sink capability for all audio output ports and host capability

## [1.0.23] - 2023-06-05
### Added 
- Added Support to build the plugin to both R4 & R2

## [1.0.22] - 2023-04-26
### Fixed
- Fixed getVolumeLevel for HDMI_ARC0 port

## [1.0.21] - 2023-04-06
### Fixed
- Fixed Audio Mute Issue seen on LG SB by sending Audio Device power on message
- It prevents LG soundbars to enter into Optical Input mode

## [1.0.20] - 2023-03-29
### Fixed
- Reduced Displaysettings log during volume change

## [1.0.19] - 2023-03-07
### Fixed
- Reduced Arc/eArc audio routing dependency based on HPD
- Fixed Blocked sending events to Displaysettings when panel power state is STANDBY
- Fixed Parallel execution of event handlers


## [1.0.18] - 2023-02-09
### Fixed
- Fixed IARM_Bus_UnRegisterEventHandler  call to IARM_Bus_RemoveEventHandler

## [1.0.17] - 2023-01-19
### Fixed
- Fixed fix DisplaySettings core dump

## [1.0.16] - 2022-12-09
### Fixed
- Fixed  Added support for HDRPLUS format support

## [1.0.15] - 2022-11-15
### Fixed
- Fixed No Audio issue while playing Dolby Mat and HDMI hotplug

## [1.0.14] - 2022-11-08
### Fixed
- Use interfaces instead of JSON-RPC

## [1.0.13] - 2022-10-18
### Fixed
- Fixed Invoke the SAD on ARC InitiationEvent Test

## [1.0.12] - 2022-10-18
### Changed
- API access on all versions of the handler

## [1.0.11] - 2022-10-14
### Changed
- Updated changes for new power manager.

## [1.0.10] - 2022-10-04
### Fixed
- Fixed warnings that are treated as errors with "-Wall -Werror"

## [1.0.9] - 2022-09-29
### Fixed
- Added missing resolutions to getSupportedTvResolutions.
## [1.0.8] - 2022-09-16
### Fixed
- Missing security token

##[1.0.7] - 2022-09-08
### Fixed
- Fixed No audio from AVR issue in case of delayed HPD scenarios

## [1.0.7] - 2022-09-07
### Fixed
- Use Controller via COM-RPC instead of JSON-RPC

##[1.0.6] - 2022-09-01
### Changed
- Fixed DisplaySettings dtor crash in LinkType

##[1.0.5] - 2022-09-01
### Changed
- Fixed DisplaySettings::sendHdmiCecSinkAudioDevicePowerOn fprintf.c crash

##[1.0.4] - 2022-08-31
### Changed
- Changed functionality to route audio to AVR on HDMI CEC off incase of eArc devices as well.

## [1.0.3] - 2022-08-25
### Fixed
- Fixed std:: Displaysettings side JSONRPC::LinkType crash fix

## [1.0.2] - 2022-08-18
### Fixed
- Device locked up for 2 minutes after turning BUILT-IN SPEAKER
  setting OFF while using input



## [1.0.1]
### Fixed
- Optimize include

## [1.0.0] - 2022-05-11
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
