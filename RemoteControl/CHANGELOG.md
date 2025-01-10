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
* The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
## [1.5.4] - 2025-01-07
### Added
- Added four new APIs unpair and start, cancel, and statusFirmwareUpdate
- Added a new notification onFirmwareUpdateProgress

### Changed
- Changed startPairing API to also accept a list of mac addresses

## [1.4.4] - 2024-03-29
### Fixed
- Fixed coverity reported issues

## [1.4.3] - 2023-01-26
### Changed
- RDK-44991: Upgrade Flex-2.0 devices to use Thunder R4.4.1

## [1.4.2] - 2023-12-13
### Changed
- Remove remoteControlTestClient from build

## [1.4.1] - 2023-10-04
### Added
- Implement Thunder Plugin Configuration for Kirkstone builds(CMake-3.20 & above)

## [1.4.0] - 2023-09-29
### Changed
- Refactor all APIs to become pass throughs but still backwards compatible

## [1.3.0] - 2023-07-05
### Added
- Added factoryReset method

### Changed
- increase IARM timeout to 10 seconds for IRDB calls

## [1.2.0] - 2023-05-18
### Added
- Added findMyRemote method to remoteControlTestClient

## [1.1.0] - 2023-04-12
### Added
- Added findMyRemote method
- Added irProgState to status structure for the IR programming state
- Fixed netTypesSupported to be an array and not a string

## [1.0.0] - 2022-07-20
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
