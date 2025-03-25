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

## [1.1.0] - 2025-03-14
### Added
- Add additional features on AVOutput

## [1.0.11] - 2025-01-02
### Security
- Resolved security vulnerabilities

## [1.0.10] - 2024-12-23
### Added
- ODM API removal changes phase 1

## [1.0.9] - 2024-10-04
### Fixed
- PQMode Camel Case issue

## [1.0.8] - 2024-09-10
### Added
- Add Dolby Game Mode

## [1.0.7] - 2024-08-06
### Fixed
- AVOutput Cleanup

## [1.0.6] - 2024-07-11
### Fixed
- AspectRatio save issue

## [1.0.5] - 2024-03-19
### Fixed
- Header cleanup

## [1.0.4] - 2024-03-01
### Fixed
- Change to c style callback

## [1.0.3] - 2024-02-23
### Fixed
- DolbyMode index issue

## [1.0.2] - 2024-02-23
### Fixed
- BlackScreen issue after PictureReset

## [1.0.1] - 2024-02-22
### Added
- Add tvsettings hal pahse 2 change

## [1.0.0] - 2024-02-07
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
