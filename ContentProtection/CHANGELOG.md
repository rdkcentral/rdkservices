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

## [3.0.3] - 2025-09-30
### Fixed
- Skip palette size parameter in watermark plugin call, similarly to aamp

## [3.0.2] - 2025-08-08
### Fixed
- JSON-RPC calls changed from sync to a-sync

## [3.0.1] - 2025-07-28
### Fixed
- Format fix for parameters openSessionResponse, updateSessionResponse

## [3.0.0] - 2025-07-08
### Changed
- Map SecManager errors to the CPS plug-in error codes

## [2.1.0] - 2025-06-25
### Changed
- Support for returning video platform data in closeDrmSession result

## [2.0.0] - 2025-06-12
### Changed
- Parameters initData, openSessionResponse, updateSessionResponse to be stringified json instead of json

## [1.1.1] - 2025-05-26
### Fixed
- Increase secmanager timeout
- Fix typo in json-rpc param

## [1.1.0] - 2025-05-13
### Changed
- New appId parameter in openDrmSession and onWatermarkStatusChanged, opacityLevel parameter in showWatermark

## [1.0.1] - 2025-05-05
### Fixed
- Fix callsign - "org.rdk." missing

## [1.0.0] - 2024-07-15
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
