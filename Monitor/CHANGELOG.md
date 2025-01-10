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

## [2.0.0] - 2024-12-12
### Changed
- Synced Monitor Plugin with WebPlatformForEmbedded/ThunderNanoServicesRDK/tree/R4_4/Monitor

## [1.0.9] - 2024-07-25
### Changed
- Added the NetworkManager Plugin into MOnitor config list

## [1.0.8] - 2024-07-01
### Changed
- Added Delay in Thread Restart Logic

## [1.0.7] - 2024-05-31
### Changed
- RDK-45345: Upgrade Sky Glass devices to use Thunder R4.4.1

## [1.0.6] - 2024-03-29
### Security
- Resolved security vulnerabilities

## [1.0.5] - 2023-09-26
### Changed
- RDK-44991: Upgrade Flex-2.0 devices to use Thunder R4.4.1

## [1.0.5] - 2024-02-15
### Fixed
- State Change Handling is fixed for Thunder R4

## [1.0.4] - 2023-09-12
### Added
- Implement Thunder Plugin Configuration for Kirkstone builds(CMake-3.20 & above)

## [1.0.3] - 2023-06-21
### Fixed
- Fixed the ResidentApp failed to activate & crash in Thunder R4

## [1.0.2] - 2023-06-05
### Added 
- Added Support to build the plugin to both R4 & R2

## [1.0.1] - 2023-04-20
### Fixed
- Fix a crash in Measure() method caused by a race for '\_source' member

## [1.0.0] - 2022-05-11
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
