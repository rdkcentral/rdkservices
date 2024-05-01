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

## [1.3.7] - 2024-05-13
### Fixed
- Notify updateTextViewOn on receiving TextViewOn cec message.

## [1.3.6] - 2024-04-18
### Fixed
- Send 'Give Audio status' request to the Audio device upon the volume +/- key press.

## [1.3.5] - 2024-03-04
### Fixed
- Notify DeviceUpdatedInfo when there is a change in physical address from current PA

## [1.3.4] - 2024-02-22
### Fixed
- CEC Certification, Ignore Broadcast/Direct CEC received messages

## [1.3.3] - 2024-02-08
### Fixed
- standby CEC messages from Unregister device

## [1.3.2] - 2024-01-09
### Fixed
- Send Physical address synchronously and retried

## [1.3.1] - 2023-12-12
### Fixed
- Reset the Activesource on putting the TV to standby

## [1.3.0] 2023-11-30
### Added
- Added HDMI-CEC key repeat for press and hold
- Added reportFeatureAbortEvent

## [1.2.4] - 2023-10-25
### Fixed
- Fixed OSDName return value 

## [1.2.3] - 2023-10-10
### Changed
- Changed string functions to safer versions

## [1.2.2] - 2023-10-04
### Added
- Implement Thunder Plugin Configuration for Kirkstone builds(CMake-3.20 & above)

## [1.2.1] 2023-09-01
### Fixed
- Fixed the parameters of setLatencyInfo() in json file such that events following this method
  are displayed properly

## [1.2.0] 2023-08-07
### Added
- Added support for Dynamic Auto LipSync

## [1.1.0] 2023-07-05
### Added
- Added support for Report Features CEC message
- Report Features message will be broadcasted if CEC Version is 2.0

## [1.0.14] 2023-06-20
### Fixed
- During THREAD_STATE_PING, Updating the info if anything that not updated for already present devices

## [1.0.13] - 2023-05-05
### Fixed
- Added Gunit test support

## [1.0.12] - 2023-04-12
### Fixed
- Fixed IARM_Bus_UnRegisterEventHandler  call to IARM_Bus_RemoveEventHandler

## [1.0.11] - 2023-03-07
### Fixed
- Fixed Blocked sending events to Displaysettings when panel power state is STANDBY
- Fixed Parallel execution of event handlers

## [1.0.10] - 2023-02-16
### Fixed
- Updated OSDName to empty string as default and when FeatureAbort frame received for GiveOSDName.

## [1.0.9] - 2022-12-31
### Fixed
- Updated documentation for sendKeyEvent thunder API.

## [1.0.8] 2022-12-21
### Fixed
- Improve the CEC retry mechanism for OSDName on I2c cec bus busy.
 
## [1.0.7] - 2022-11-28
### Fixed
- Treat warnings as errors for unit tests workflow

## [1.0.6] - 2022-11-22
### Fixed
- thread RAII check and revert

## [1.0.5] - 2022-10-25
### Fixed
- hdmi cec async send CEC ver, OSD Name, VendorID

## [1.0.4] - 2022-10-04
### Fixed
- Fixed warnings that are treated as errors with "-Wall -Werror"

## [1.0.3]
### Fixed
- Fix for osdName is NA for connected HDMIs.
 
## [1.0.2]
### Fixed
- Fix for resetting retry count for Physical address query.
- Fix for handling AVR transition state.

## [1.0.1]
### Fixed
- Optimize include

## [1.0.0] - 2022-05-11
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
