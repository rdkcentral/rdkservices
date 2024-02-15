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

## [1.1.2] - 2024-03-28
### Security
- Resolved security vulnerabilities

## [1.1.1] - 2023-10-04
### Added
- Implement Thunder Plugin Configuration for Kirkstone builds(CMake-3.20 & above)

## [1.1.0] - 2023-08-07
### Added
- Added support for Dynamic Auto LipSync

## [1.0.6] - 2023-06-12
### Added
- AVI Content Type Signalling

## [1.0.5] - 2023-06-05
### Added 
- Added Support to build the plugin to both R4 & R2

## [1.0.4] - 2023-04-12
### Fixed
- Fixed IARM_Bus_UnRegisterEventHandler  call to IARM_Bus_RemoveEventHandler

## [1.0.3] - 2022-10-21
### Fixed
- Added Gunit test support

## [1.0.2] - 2022-10-18
### Changed
- API access on all versions of the handler

## [1.0.1] - 2022-10-04
### Fixed
- Fixed warnings that are treated as errors with "-Wall -Werror"

## [1.0.0] - 2022-05-11
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
