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

## [1.0.11] - 2025-02-05
### Fixed
- Moved interface into ThunderInterfaces

## [1.0.10] - 2024-07-12
### Fixed
- Changed set volume for realtek soc

## [1.0.9] - 2024-01-05
### Added
- Added Unit Testcases

## [1.0.8] - 2023-10-10
### Changed
- Changed string functions to safer versions

## [1.0.7] - 2023-09-12
### Added
- Implement Thunder Plugin Configuration for Kirkstone builds(CMake-3.20 & above)

## [1.0.6] - 2023-08-02
### Fixed
- Fixed warnings treated as errors for emulator build

## [1.0.5] - 2023-06-05
### Added 
- Added Support to build the plugin to both R4 & R2

## [1.0.4] - 2023-01-25
### Added
- Added Support for Broadcom Platform

## [1.0.3] - 2022-10-04
### Fixed
- Fixed warnings that are treated as errors with "-Wall -Werror"

## [1.0.2]
### Fixed
- Optimize include
- Add MODULE_NAME_DECLARATION

## [1.0.1]
### Fixed
- Missing return statements (undefined behavior)

## [1.0.0] - 2022-05-11
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
