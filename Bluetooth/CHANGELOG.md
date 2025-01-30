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

## [1.0.12] - 2024-09-26
### Added
- Make sure connect response is sent on a connect request

## [1.0.11] - 2024-05-25
### Added
- Make plugin autostart configurable from recipe

## [1.0.10] - 2024-03-29
### Added
- Device class and appearance to all "get Devices" functions
- Connect call when JOYSTICK specified should not start streaming out

## [1.0.9] - 2024-03-29
### Security
- Resolved security vulnerabilities

## [1.0.8] - 2024-01-26
## Added
- view all audio and HID devices by default when doing a scan

## [1.0.7] - 2024-01-22
### Added
- add appearance value in discovered devices to be able to distinguish different types of device

## [1.0.6] - 2024-01-02
### Security
- resolved security vulnerabilities

## [1.0.5] - 2023-11-21
### Added
- add modalias and firmware revision string (derived from modalias) to device info

## [1.0.4] - 2023-10-30
### Added
- added batteryLevel parameter in getDeviceInfo

## [1.0.3] - 2023-09-12
### Added
- added rawDeviceType parameter in getDiscoveredDevices

## [1.0.3] - 2023-09-12
### Added
- Implement Thunder Plugin Configuration for Kirkstone builds(CMake-3.20 & above)

## [1.0.2] - 2022-11-21
### Fixed
- Publish all the connect/disconnect events. Do not filter out

## [1.0.1] - 2022-10-04
### Fixed
- Fixed warnings that are treated as errors with "-Wall -Werror"

## [1.0.0] - 2022-05-11
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
