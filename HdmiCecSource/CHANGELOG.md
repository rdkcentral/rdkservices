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

## [1.0.8] - 2024-03-21
### Changed
- Fixed IARM event handling to process in thread

## [1.0.7] - 2024-02-02
### Changed
- Fixed IARM unregister API call

## [1.0.6] - 2023-10-10
### Changed
- Changed string functions to safer versions

## [1.0.5] - 2023-09-12
### Added
- Implement Thunder Plugin Configuration for Kirkstone builds(CMake-3.20 & above)

## [1.0.4] - 2023-04-20
### Fixed
- Fixed CEC crash by initializing status variables inside the ctor.

## [1.0.3] - 2023-04-11
### Fixed
- Updated Logging, and initialize function to be more verbose for testing purposes
- Additional thread handling issues

## [1.0.2] - 2023-03-20
### Fixed
- Fixed thread handling issue in CECDisable.

## [1.0.1] - 2022-12-31
### Fixed
- Updated documentation for sendKeyEvent thunder API.

## [1.0.0]
### Added
- Renaming Hdmicec_2 to HdmiCecSource and initializing the HdmiCecSource plugin.

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
