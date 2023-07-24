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
## [1.0.11] - 2023-07-24
### Fixed
- TexttoSpeech Plugin Compilation Error for Thunder R4

## [1.0.10] - 2023-07-14
### Fixed
- Proper deactivation of TTS plugin

## [1.0.9] - 2023-07-12
### Added
- SAT token appending,speechrate parameter for flex 2.0

## [1.0.8] - 2023-07-03
### Fixed
- Rate not intialized to 0

## [1.0.7] - 2023-06-29
### Changed
- Refine TTS COM-RPC interface

## [1.0.6] - 2023-06-05
### Added 
- Added Support to build the plugin to both R4 & R2

## [1.0.5] - 2022-10-04
### Fixed
- Fixed warnings that are treated as errors with "-Wall -Werror"

## [1.0.4] - 2022-09-15
### Fixed
- Fixed the issue VG not announcing activate URL properly

## [1.0.3] - 2022-09-13
### Fixed
- Fixed TTS plugin crash when invalid language is specified from application

## [1.0.2] - 2022-08-29
### Changed
- Modified setttsconfiguration API to include primvolduckpercent as an attribute.

## [1.0.1] - 2022-08-29
### Fixed 
- Reverted a change which caused increased volume for TTS audio on Xione-US devices

## [1.0.0] - 2022-05-11
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
