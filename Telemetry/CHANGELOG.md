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

## [1.4.0] - 2025-6-10
### Changed
- Changed to use UserSettings plugin instead of SystemServices to get Privacy Mode.

## [1.3.0] - 2024-10-3
### Added
- Added notifications for Telemetry component about changes in PrivacyMode.

## [1.2.2] - 2024-02-02
### Fixed
- Fixed IARM unregister calls.

## [1.2.1] - 2023-09-12
### Added
- Implement Thunder Plugin Configuration for Kirkstone builds(CMake-3.20 & above)

## [1.2.0] - 2023-4-18
### Added
- Added abortReport call (stops uploading of telemetry report)
- Added listener for power state changes, it calls uploadReport for STANDBY or LIGHTSLEEP state and abortReport for DEEPSLEEP.

## [1.1.0] - 2022-10-21
### Added
- Added uploadReport call (triggers processing and uploading of telemetry report for legacy Xconf based configuration)

## [1.0.0] - 2022-05-11
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
